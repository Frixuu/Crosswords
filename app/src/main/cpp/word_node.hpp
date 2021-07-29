#ifndef CROSSWORD_HELPER_WORD_NODE_HPP
#define CROSSWORD_HELPER_WORD_NODE_HPP

#include <map>
#include <string>

namespace crossword {

    /// A node in an index representing set of strings.
    struct WordNode {

        std::string valid_word;
        std::unique_ptr<std::map<char, std::unique_ptr<WordNode>>> children;

        /// Creates a new WordNode.
        /// The node will be representing a valid word.
        WordNode(const std::string &&word) {
            valid_word = std::move(word);
        }

        /// Creates a new WordNode representing an invalid word.
        WordNode() : WordNode(std::string()) {
        }

        ~WordNode() {
            children.reset();
        }

        /// Determines whether this node represents a valid word.
        /// This only makes sense in context of a particular tree index.
        bool valid() noexcept {
            return !valid_word.empty();
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
                // TODO: Handle UTF-8
                auto key = tolower(str.at(index));
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
                        int32_t index,
                        size_t limit,
                        const std::string &cursor) {

            if (!children || index > pattern.length()) {
                return;
            } else if (limit > 0 && limit < vec.size()) {
                return;
            } else if (index == pattern.length()) {
                if (valid()) {
                    auto wordCopy = valid_word;
                    vec.emplace_back(std::move(wordCopy));
                }
                return;
            } else {
                auto ch = pattern.at(index);
                if (ch == '.') {
                    for (const auto &entry : *children) {
                        entry.second->find_words(vec, pattern, index + 1, limit, cursor);
                    }
                } else {
                    auto result = children->find(ch);
                    if (result != children->end()) {
                        result->second->find_words(vec, pattern, index + 1, limit, cursor);
                    }
                }
            }
        }
    };
}

#endif //CROSSWORD_HELPER_WORD_NODE_HPP
