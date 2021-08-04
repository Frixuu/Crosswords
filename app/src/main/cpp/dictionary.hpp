#ifndef CROSSWORD_HELPER_DICTIONARY_HPP
#define CROSSWORD_HELPER_DICTIONARY_HPP

#include <algorithm>
#include <atomic>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include "word_node.hpp"
#include "utils/utf8.hpp"
#include "utils/macros.hpp"
#include "utils/arena.hpp"

namespace crossword {

    using namespace ::crossword::collections;
    using namespace ::crossword::utils;

    /// Stores a set of words from a particular language in forms
    /// that allow for easy indexing.
    class Dictionary {

    private:

        std::unique_ptr<WordNode> forward_index;
        std::unique_ptr<arena<WordNode>> node_pool;
        std::unique_ptr<arena<map_chunk<WordNode>>> chunk_pool;
        std::unique_ptr<arena<std::string>> string_pool;

        /// Parses lines from a UTF-8 encoded buffer and adds them to the dictionary.
        /// @param buffer Pointer to the data buffer.
        /// @param start Index to start searching from.
        /// @param end Exclusive end index of buffer parsing.
        void load_from_buffer(const char *buffer, size_t start, size_t end) {

            // Stores contents of the current line so far
            std::vector<char> line_buffer;

            // Cache not to lookup first parent
            char last_first_byte = 'a';
            auto last_ancestor = forward_index->children
                .try_emplace('a', node_pool->alloc(), chunk_pool.get())
                .first.second;

            auto index = start;
            while (index < end) {
                // Read the buffer forward
                auto byte = buffer[index++];
                if (byte != '\n' && byte != '\r') {
                    line_buffer.emplace_back(byte);
                } else {

                    // CRLF sequences and multiple line breaks are valid,
                    // but since we control the input, we know that's pretty rare
                    if (LIKELY(!line_buffer.empty())) {

                        // Most of the words are short enough to be inlined
                        auto word_ptr = string_pool->alloc();
                        *word_ptr = std::string(line_buffer.begin(), line_buffer.end());
                        auto first_letter = to_lower(line_buffer[0]);
                        line_buffer.clear();

                        // The input is pre-sorted, so we usually get the cached parent
                        if (LIKELY(to_lower(line_buffer[0]) == last_first_byte)) {
                            last_ancestor->push_word(word_ptr, 1, node_pool.get(), chunk_pool.get());
                        } else {
                            // The first letter has changed,
                            // next words will use the new parent
                            last_first_byte = first_letter;
                            last_ancestor = forward_index->children
                                .try_emplace(first_letter, node_pool->alloc(), chunk_pool.get())
                                .first.second;
                            last_ancestor->push_word(word_ptr, 1, node_pool.get(), chunk_pool.get());
                        }
                    }
                }
            }

            // In case the buffer did not end with a new line,
            // push the remaining chars
            if (!line_buffer.empty()) {
                auto word_ptr = string_pool->alloc();
                *word_ptr = std::string(line_buffer.begin(), line_buffer.end());
                forward_index->push_word(word_ptr, 0, node_pool.get(), chunk_pool.get());
            }
        }

    public:

        /// Creates a new, empty Dictionary.
        Dictionary() :
            forward_index(std::make_unique<WordNode>()),
            node_pool(std::make_unique<arena<WordNode>>()),
            chunk_pool(std::make_unique<arena<map_chunk<WordNode>>>()),
            string_pool(std::make_unique<arena<std::string>>()) {
        }

        Dictionary(const Dictionary &) = delete;
        Dictionary(Dictionary &&) = delete;
        Dictionary &operator=(const Dictionary &) = delete;
        Dictionary &operator=(Dictionary &&) = delete;

        ~Dictionary() {
            forward_index.reset();
            node_pool.reset();
            chunk_pool.reset();
            string_pool.reset();
        }

        void find_words(std::vector<std::string> &vec,
                        const std::string &pattern,
                        int32_t limit,
                        const std::string &cursor) {

            forward_index->find_words(vec, pattern, 0, 0, limit, cursor);
        }

        void merge(Dictionary *other) {
            forward_index->merge(other->forward_index.get(), chunk_pool.get());
            node_pool->merge(other->node_pool.get());
            chunk_pool->merge(other->chunk_pool.get());
            string_pool->merge(other->string_pool.get());
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
        /// @param par_count How many threads will be spawned to parse the buffer?
        void load_from_buffer_par(const char *buffer, int length, int par_count) {
            std::vector<std::thread> loading_threads;
            std::vector<std::unique_ptr<Dictionary>> result_dictionaries;

            auto indices = std::make_unique<int[]>(par_count);
            indices[0] = 0;

            // Split the buffer into chunks
            for (int i = 1; i < par_count; i++) {

                // Since we do not know the word length distribution,
                // we start from the equal-sized segments
                int candidate = i * length / par_count;

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
            for (auto i = 0; i < par_count; i++) {
                int start = indices[i];
                int end = length;
                if (i < (par_count - 1)) {
                    end = indices[i + 1];
                }

                // Each thread gets their own dictionary,
                // because merging them is cheaper than locking
                // and making other threads' caches dirty
                auto dict = std::make_unique<Dictionary>();
                auto thread = std::thread(&Dictionary::load_from_buffer, dict.get(), buffer, start, end);
                loading_threads.emplace_back(std::move(thread));
                result_dictionaries.emplace_back(std::move(dict));
            }

            // Wait until parsing finishes
            for (auto &thread : loading_threads) {
                thread.join();
            }

            // Merge the results.
            // It's pretty cheap as long as the input was at-least k-sorted
            for (auto &dict : result_dictionaries) {
                merge(dict.get());
            }
        }
    };
}

#endif //CROSSWORD_HELPER_DICTIONARY_HPP
