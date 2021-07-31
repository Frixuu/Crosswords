#ifndef CROSSWORD_HELPER_WORD_NODE_HPP
#define CROSSWORD_HELPER_WORD_NODE_HPP

#include <map>
#include <string>
#include "string_utils.hpp"

namespace crossword {

    /// A node in an index representing set of strings.
    struct WordNode {

        std::string valid_word;
        std::unique_ptr<std::map<char, std::unique_ptr<WordNode>>> children;

        /// Creates a new WordNode.
        /// The node will be representing a valid word.
        explicit WordNode(const std::string &&word) : valid_word(std::move(word)) {
        }

        /// Creates a new WordNode representing an invalid word.
        WordNode() {
        }

        /// Determines whether this node represents a valid word.
        /// This only makes sense in context of a particular tree index.
        bool valid() const noexcept {
            return !valid_word.empty();
        }

        size_t calculate_size() noexcept {
            size_t count = 0;
            if (valid()) {
                count += 1;
            }
            if (children) {
                for (const auto &entry : *children) {
                    count += entry.second->calculate_size();
                }
            }
            return count;
        }

        /// Pushes a word deep down the index.
        /// @param str Word being pushed into the index.
        /// @param index Current index depth.
        bool push_word(const std::string &&str, size_t index) {

            // If this node does not have children at this point, create the collection
            if (!children) {
                children = std::make_unique<std::map<char, std::unique_ptr<WordNode>>>();
            }

            auto word_length = str.length();
            if (index < word_length) {

                auto key = str.at(index);
                if (utils::codepoint_is_one_byte(key)) {
                    key = tolower(key);
                }

                auto [entry, _] = children->try_emplace(key, std::make_unique<WordNode>());
                if (index < (word_length - 1)) {
                    return entry->second->push_word(std::move(str), index + 1);
                } else {
                    entry->second->valid_word = std::move(str);
                }
            } else if (index == word_length) {
                valid_word = std::move(str);
            } else {
                return false;
            }

            return true;
        }

        /// Find words matching a provided pattern.
        /// All matching words will be added to the passed vector.
        /// If limit > 0, only n words will be added.
        /// If a cursor value is provided and children are sorted,
        /// starts search from that cursor value.
        void find_words(std::vector<std::string> &vec,
                        const std::string &pattern,
                        size_t index,
                        int32_t point_offset,
                        int32_t limit,
                        const std::string &cursor) {

            // The pattern matched a wildcard and parent was a multi-byte character
            if (point_offset > 0) {
                auto offset = point_offset - 1;
                for (const auto &entry : *children) {
                    entry.second->find_words(vec, pattern, index, offset, limit, cursor);
                }
                return;
            }

            // The result vector is full
            if (limit > 0 && limit <= static_cast<int>(vec.size())) {
                return;
            }

            if (index == pattern.length()) {
                if (valid()) {
                    auto word_copy = valid_word;
                    vec.emplace_back(std::move(word_copy));
                }
                return;
            }

            if (children && index <= pattern.length()) {
                auto ch = pattern.at(index);
                if (ch == '.') {
                    for (const auto &entry : *children) {
                        auto key = entry.first;

                        auto offset = 0;
                        if (!utils::codepoint_is_continuation(key)) {
                            offset = utils::codepoint_size(key) - 1;
                        }

                        entry.second->find_words(vec, pattern, index + 1, offset, limit, cursor);
                    }
                } else {
                    auto result = children->find(ch);
                    if (result != children->end()) {
                        result->second->find_words(vec, pattern, index + 1, 0, limit, cursor);
                    }
                    auto ch_lower = tolower(ch);
                    if (ch != ch_lower) {
                        auto result = children->find(ch_lower);
                        if (result != children->end()) {
                            result->second->find_words(vec, pattern, index + 1, 0, limit, cursor);
                        }
                    }
                }
            }
        }

        void merge(WordNode *other) {

            if (other->valid()) {
                valid_word = std::move(other->valid_word);
            }

            if (other->children) {
                if (!children) {
                    children = std::move(other->children);
                } else {
                    std::vector<std::pair<char, std::unique_ptr<WordNode>>> buffer;
                    for (auto &other_child : *other->children) {
                        auto result = children->find(other_child.first);
                        if (result == children->end()) {
                            buffer.emplace_back(std::move(other_child));
                        } else {
                            result->second->merge(other_child.second.get());
                        }
                    }
                    for (auto &pair : buffer) {
                        children->try_emplace(pair.first, std::move(pair.second));
                    }
                }
            }
        }
    };
}

#endif //CROSSWORD_HELPER_WORD_NODE_HPP
