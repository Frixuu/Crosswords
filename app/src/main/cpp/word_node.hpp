#ifndef CROSSWORD_HELPER_WORD_NODE_HPP
#define CROSSWORD_HELPER_WORD_NODE_HPP

#include <map>
#include <string>
#include "collections/chunked_map.hpp"
#include "utils/utf8.hpp"
#include "utils/macros.hpp"
#include "utils/memory_pool.hpp"

namespace crossword {

    /// A node in an index representing set of strings.
    struct WordNode {

        std::string* valid_word;
        collections::chunked_map<WordNode> children;

        /// Creates a new WordNode representing an invalid word.
        WordNode() : valid_word(nullptr) {
        }

        WordNode(const WordNode&& other) = delete;
        WordNode& operator=(const WordNode& other) = delete;
        WordNode(WordNode&& other) = delete;
        WordNode& operator=(WordNode&& other) = delete;

        /// Determines whether this node represents a valid word.
        /// This only makes sense in context of a particular tree index.
        bool valid() const noexcept {
            return valid_word != nullptr;
        }

        bool has_children() const noexcept {
            return !children.empty();
        }

        size_t calculate_size() noexcept {
            size_t count = 0;
            if (valid()) {
                count += 1;
            }
            if (has_children()) {
                for (const auto &entry : children) {
                    count += entry.second->calculate_size();
                }
            }
            return count;
        }

        size_t count_nodes() noexcept {
            size_t count = 1;
            if (has_children()) {
                for (const auto &entry : children) {
                    count += entry.second->count_nodes();
                }
            }
            return count;
        }

        /// Pushes a word deep down the index.
        /// @param str Word being pushed into the index.
        /// @param index Current index depth.
        bool push_word(std::string* str,
                       size_t index,
                       utils::memory_pool<WordNode> *node_pool,
                       utils::memory_pool<collections::map_chunk<WordNode>> *chunk_pool) {

            auto word_length = str->length();
            if (index < word_length) {

                auto key = str->at(index);
                if (utils::codepoint_is_one_byte(key)) {
                    key = utils::to_lower(key);
                }

                auto [entry, inserted] = children.try_emplace(key, node_pool->alloc(), chunk_pool);
                if (!inserted) {
                    node_pool->dealloc_last();
                }

                if (index < (word_length - 1)) {
                    return entry.second->push_word(str, index + 1, node_pool, chunk_pool);
                } else {
                    entry.second->valid_word = str;
                }
            } else if (LIKELY(index == word_length)) {
                valid_word = str;
            } else {
                return false;
            }

            return true;
        }

        /// Find words matching a provided pattern.
        /// All matching words will be added to the passed vector.
        /// If limit > 0, only n words will be added.
        /// If a cursor value is provided, assuming children are sorted,
        /// starts search from that cursor value (TODO).
        void find_words(std::vector<std::string> &vec,
                        const std::string &pattern,
                        size_t index,
                        int32_t point_offset,
                        int32_t limit,
                        const std::string &cursor) {

            // The pattern matched a wildcard and parent was a multi-byte character
            if (point_offset > 0) {
                auto offset = point_offset - 1;
                for (auto entry : children) {
                    // The wildcard is a single character, do not increment index
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
                    auto word_copy = *valid_word;
                    vec.emplace_back(std::move(word_copy));
                }
                return;
            }

            if (has_children() && index < pattern.length()) {
                auto ch = pattern.at(index);
                if (ch == '.') {
                    for (const auto &entry : children) {
                        auto key = entry.first;
                        auto offset = 0;
                        if (LIKELY(!utils::codepoint_is_continuation(key))) {
                            offset = utils::codepoint_size(key) - 1;
                        }

                        entry.second->find_words(vec, pattern, index + 1, offset, limit, cursor);
                    }
                } else {
                    auto result = children.find(ch);
                    if (result != children.end()) {
                        result.get_element().second->find_words(vec, pattern, index + 1, 0, limit, cursor);
                    }
                    auto ch_lower = utils::to_lower(ch);
                    if (ch != ch_lower) {
                        auto result_lower = children.find(ch_lower);
                        if (result_lower != children.end()) {
                            result_lower.get_element().second->find_words(vec, pattern, index + 1, 0, limit, cursor);
                        }
                    }
                }
            }
        }

        void merge(WordNode *other,
                   utils::memory_pool<collections::map_chunk<WordNode>> *chunk_pool) {

            if (other->valid()) {
                valid_word = other->valid_word;
            }

            if (other->has_children()) {
                if (!has_children()) {
                    children = std::move(other->children);
                } else {
                    for (auto other_child : other->children) {
                        auto result = children.find(other_child.first);
                        if (result == children.end()) {
                            children.try_emplace(other_child.first, other_child.second, chunk_pool);
                        } else {
                            auto this_node = result.get_element().second;
                            auto other_node = other_child.second;
                            this_node->merge(other_node, chunk_pool);
                        }
                    }
                }
            }
        }
    };
}

#endif //CROSSWORD_HELPER_WORD_NODE_HPP
