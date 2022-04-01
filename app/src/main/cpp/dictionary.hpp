#ifndef CROSSWORD_HELPER_DICTIONARY_HPP
#define CROSSWORD_HELPER_DICTIONARY_HPP

#include "memory/arena.hpp"
#include "utils/utf8.hpp"
#include "word_node.hpp"

#include <algorithm>
#include <atomic>
#include <map>
#include <string>
#include <thread>
#include <vector>

namespace crossword {

    using namespace ::crossword::collections;
    using namespace ::crossword::utils;
    using ::crossword::memory::Arena;

    /// Stores a set of words from a particular language in forms
    /// that allow for easy indexing.
    class Dictionary {
    private:
        std::unique_ptr<WordNode> forward_index;
        std::unique_ptr<Arena<WordNode>> node_arena;
        std::unique_ptr<Arena<MapChunk<uint8_t, WordNode*>>> index_map_chunk_arena;
        std::unique_ptr<Arena<std::string, false>> string_arena;

        /// Parses lines from a UTF-8 encoded buffer and adds them to the dictionary.
        /// @param buffer Pointer to the data buffer.
        /// @param start Index to start searching from.
        /// @param end Exclusive end index of buffer parsing.
        void load_from_buffer(const char* buffer, const size_t start, const size_t end) {
            // Stores contents of the current line so far
            std::vector<char> line_buffer;

            // Cache not to lookup first parent
            char prev_first_byte = 'a';
            auto prev_ancestor
                = forward_index->children
                      .find_or_insert('a', node_arena->alloc(), index_map_chunk_arena.get())
                      .entry.second;

            auto index = start;
            while (index < end) {
                // Read the buffer forward
                auto byte = buffer[index++];
                if (byte != '\n' && byte != '\r') {
                    line_buffer.push_back(byte);
                } else {
                    // CRLF sequences and multiple line breaks are valid,
                    // but since we control the input, we know that's pretty rare
                    if (!line_buffer.empty()) [[likely]] {
                        // Most of the words are short enough to be inlined
                        auto word_ptr = string_arena->alloc();
                        *word_ptr = std::string(line_buffer.begin(), line_buffer.end());
                        auto first_byte = to_lower(line_buffer[0]);
                        line_buffer.clear();

                        // The input is pre-sorted, so we usually get the cached parent
                        if (first_byte == prev_first_byte) [[likely]] {
                            prev_ancestor->push_word(word_ptr, 1, node_arena.get(),
                                                     index_map_chunk_arena.get());
                        } else {
                            // The first letter has changed,
                            // next words will use the new parent
                            prev_first_byte = first_byte;
                            prev_ancestor = forward_index->children
                                                .find_or_insert(first_byte, node_arena->alloc(),
                                                                index_map_chunk_arena.get())
                                                .entry.second;
                            prev_ancestor->push_word(word_ptr, 1, node_arena.get(),
                                                     index_map_chunk_arena.get());
                        }
                    }
                }
            }

            // In case the buffer did not end with a new line,
            // push the remaining chars
            if (!line_buffer.empty()) {
                auto word_ptr = string_arena->alloc();
                *word_ptr = std::string(line_buffer.begin(), line_buffer.end());
                forward_index->push_word(word_ptr, 0, node_arena.get(),
                                         index_map_chunk_arena.get());
            }
        }

    public:
        /// Creates a new, empty Dictionary.
        Dictionary() :
            forward_index(std::make_unique<WordNode>()),
            node_arena(std::make_unique<Arena<WordNode>>()),
            index_map_chunk_arena(std::make_unique<Arena<MapChunk<uint8_t, WordNode*>>>()),
            string_arena(std::make_unique<Arena<std::string, false>>()) {}

        Dictionary(const Dictionary&) = delete;
        Dictionary(Dictionary&&) = delete;
        Dictionary& operator=(const Dictionary&) = delete;
        Dictionary& operator=(Dictionary&&) = delete;

        ~Dictionary() {
            forward_index.reset();
            node_arena.reset();
            index_map_chunk_arena.reset();
            string_arena.reset();
        }

        void find_words(std::vector<std::string>& vec,
                        const std::string& pattern,
                        int32_t limit,
                        const std::string& cursor) {
            forward_index->find_words(vec, pattern, 0, 0, limit, cursor);
        }

        void merge(Dictionary* other) {
            forward_index->merge(other->forward_index.get(), index_map_chunk_arena.get());
            node_arena->merge(other->node_arena.get());
            index_map_chunk_arena->merge(other->index_map_chunk_arena.get());
            string_arena->merge(other->string_arena.get());
        }

        size_t calculate_size() {
            return forward_index->calculate_size();
        }

        size_t count_nodes() {
            return forward_index->count_nodes();
        }

        /// Puts words in this dictionary, using multiple threads.
        /// @param buffer Buffer to UTF-8 data
        /// @param length Length of the buffer
        /// @param thread_count How many threads will be spawned to parse the buffer?
        void load_from_buffer_par(const char* buffer, const int length, const int thread_count) {
            std::vector<std::thread> threads;
            std::vector<std::unique_ptr<Dictionary>> result_dictionaries;

            auto indices = std::make_unique<int[]>(thread_count);
            indices[0] = 0;

            // Split the buffer into chunks
            for (int i = 1; i < thread_count; i++) {
                // Since we do not know the word length distribution,
                // we start from the equal-sized segments
                int candidate = i * length / thread_count;

                // CR and LF cannot be in later bytes of the codepoint,
                // so break on any of them
                while (candidate < length) {
                    auto curr_byte = buffer[candidate++];
                    if (curr_byte == '\n' || curr_byte == '\r') {
                        indices[i] = candidate;
                        break;
                    }
                }
            }

            // Spawn a thread for every chunk
            for (auto i = 0; i < thread_count; i++) {
                int start = indices[i];
                int end = length;
                if (i < (thread_count - 1)) {
                    end = indices[i + 1];
                }

                // Each thread gets their own dictionary,
                // because merging them should be cheaper than locking
                // and making other threads' caches dirty
                auto dict = std::make_unique<Dictionary>();
                threads.emplace_back(&Dictionary::load_from_buffer, dict.get(), buffer, start, end);
                result_dictionaries.push_back(std::move(dict));
            }

            // Wait until parsing finishes
            for (auto& thread : threads) {
                thread.join();
            }

            // Merge the results.
            // It's pretty cheap as long as the input was at-least k-sorted
            for (auto& dict : result_dictionaries) {
                merge(dict.get());
            }
        }
    };
}

#endif // CROSSWORD_HELPER_DICTIONARY_HPP
