#ifndef CROSSWORD_HELPER_WORD_NODE_HPP
#define CROSSWORD_HELPER_WORD_NODE_HPP

#include "collections/chunked_map.hpp"
#include "memory/arena.hpp"
#include "utils/android.hpp"
#include "utils/utf8.hpp"

#include <map>
#include <string>

namespace crossword {

    using ::crossword::collections::ChunkedMap;
    using ::crossword::collections::MapChunk;
    using ::crossword::memory::Arena;
    using namespace ::crossword::utils;
    using namespace ::crossword::utils::android;

    /// A node in an index representing set of strings.
    struct WordNode {
    public:
        std::string* valid_word;
        ChunkedMap<uint8_t, WordNode*> children;

        /// Creates a new WordNode representing an invalid word.
        constexpr WordNode() : valid_word(nullptr) {}

        WordNode(const WordNode& other) = delete;
        WordNode& operator=(const WordNode& other) = delete;

        /// Determines whether this node represents a valid word.
        /// This only makes sense in context of a particular tree index.
        constexpr inline bool valid() noexcept {
            return valid_word != nullptr;
        }

        /// Does this node have any children nodes?
        constexpr inline bool has_children() noexcept {
            return !children.empty();
        }

        size_t calculate_size() noexcept {
            size_t count = 0;
            if (valid()) {
                count += 1;
            }
            if (has_children()) {
                for (const auto& entry : children) {
                    count += entry.second->calculate_size();
                }
            }
            return count;
        }

        size_t count_nodes() noexcept {
            size_t count = 1;
            if (has_children()) {
                for (const auto& entry : children) {
                    count += entry.second->count_nodes();
                }
            }
            return count;
        }

        /// Pushes a word deep down the index.
        /// @param str Word being pushed into the index.
        /// @param index Current index depth.
        bool push_word(std::string* str,
                       const size_t index,
                       Arena<WordNode>* node_arena,
                       Arena<MapChunk<uint8_t, WordNode*>>* chunk_arena) {
            // Check the length of the word (depth of the index)
            auto word_length = str->length();

            if (index == word_length) {
                valid_word = str;
                return true;
            }

            if (index < word_length) {
                auto key = str->at(index);
                if (utils::codepoint_is_one_byte(key)) {
                    key = utils::to_lower(key);
                }

                auto new_child = node_arena->alloc();
                auto [entry, inserted] = children.find_or_insert(key, new_child, chunk_arena);
                auto [_key, node] = entry;

                // No string assignment happened, bump the arena pointer back
                if (!inserted) {
                    node_arena->dealloc_last();
                }

                // Is the next node a target for the word to stay?
                if (index + 1 == word_length) {
                    node->valid_word = str;
                    return true;
                } else {
                    // Whatever, just push it forward
                    return node->push_word(str, index + 1, node_arena, chunk_arena);
                }
            }

            log::tag("push_word").w("Missed a word: %s", str->c_str());
            return false;
        }

        /// Find words matching a provided pattern.
        /// All matching words will be added to the passed vector.
        /// If limit > 0, only n words will be added.
        /// If a cursor value is provided, assuming children are sorted,
        /// starts search from that cursor value (TODO).
        void find_words(std::vector<std::string>& vec,
                        const std::string& pattern,
                        const size_t index,
                        const int32_t point_offset,
                        const int32_t limit) {
            // The pattern matched a wildcard and parent was a multi-byte character
            if (point_offset > 0) {
                for (const auto& [_key, child] : children) {
                    // The wildcard is a single character, do not increment index
                    child->find_words(vec, pattern, index, point_offset - 1, limit);
                }
                return;
            }

            // The result vector is full
            if (limit <= static_cast<int>(vec.size())) {
                return;
            }

            // We have reached the end of the pattern!
            // If this node represents a valid word, add it to the result vector
            if (index == pattern.length()) {
                if (valid()) {
                    auto word_copy = *valid_word;
                    vec.push_back(word_copy);
                }
                return;
            }

            // Perhaps we have went too far?
            if (index > pattern.length()) [[unlikely]] {
                log::tag("find_words").w("Missed node ending pattern %s", pattern.c_str());
                return;
            }

            // No children? We're done
            if (!has_children()) {
                return;
            }

            auto ch = static_cast<uint8_t>(pattern.at(index));
            if (ch == '.') {
                for (const auto& [key, child] : children) {
                    int offset = 0;
                    if (!utils::codepoint_is_continuation(key))
                        offset = utils::codepoint_size(key) - 1;

                    child->find_words(vec, pattern, index + 1, offset, limit);
                }
                return;
            }

            auto result = children.find(ch);
            if (result != children.end()) {
                auto [_key, child] = result.get_element();
                child->find_words(vec, pattern, index + 1, 0, limit);
            }

            auto ch_lower = utils::to_lower(ch);
            if (ch == ch_lower) {
                return;
            }

            auto result_lower = children.find(ch_lower);
            if (result_lower != children.end()) {
                auto [_key, child] = result_lower.get_element();
                child->find_words(vec, pattern, index + 1, 0, limit);
            }
        }

        /// Merges another node with this node.
        /// Assume the other node always represents the same place in an index as this one.
        /// @param other The other node to merge with this one.
        /// @param chunk_arena The arena to allocate new map elements with.
        void merge(WordNode* other, Arena<MapChunk<uint8_t, WordNode*>>* chunk_arena) {
            if (other->valid()) {
                valid_word = other->valid_word;
            }

            // The other node does not have children? Nothing else to merge
            if (!other->has_children()) {
                return;
            }

            // This node does not have children? Just move the other node's ones
            if (!has_children()) {
                children = std::move(other->children);
                return;
            }

            // Both nodes have children? It gets a bit more complicated.
            // First, iterate over the other node's children
            for (auto [key, other_child] : other->children) {
                auto result = children.find(key);
                if (result == children.end()) {
                    // The other node has a child that this node does not have? Save it
                    // (ignore the result, we are sure an insertion will happen)
                    children.find_or_insert(key, other_child, chunk_arena);
                } else {
                    // If both nodes exist, merge them by recursion
                    auto this_child = result.get_element().second;
                    this_child->merge(other_child, chunk_arena);
                }
            }
        }
    };
}

#endif // CROSSWORD_HELPER_WORD_NODE_HPP
