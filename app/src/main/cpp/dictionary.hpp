#ifndef CROSSWORD_HELPER_DICTIONARY_HPP
#define CROSSWORD_HELPER_DICTIONARY_HPP

#include <map>
#include <string>
#include <thread>
#include <vector>
#include "word_node.hpp"
#include "utils/utf8.hpp"
#include "utils/macros.hpp"
#include "utils/memory_pool.hpp"

namespace crossword {

    /// Stores a set of words from a particular language in forms
    /// that allow for easy indexing.
    class Dictionary {

    private:

        std::unique_ptr<WordNode> forward_index;
        std::unique_ptr<utils::memory_pool<WordNode>> node_pool;

        /// Parses lines from a UTF-8 encoded buffer and adds them to the dictionary.
        /// @param buffer Pointer to the data buffer.
        /// @param start Index to start searching from.
        /// @param end Exclusive end index of buffer parsing.
        void load_from_buffer(const char *buffer, size_t start, size_t end) {
            std::vector<char> line_buffer;

            // Cache not to lookup first parent
            char last_first_byte = 'a';
            auto last_ancestor = forward_index->children
                .try_emplace('a', node_pool->alloc())
                .first.second;

            auto index = start;
            while (index < end) {
                auto byte = buffer[index++];
                if (byte != '\n' && byte != '\r') {
                    line_buffer.emplace_back(byte);
                } else {
                    // CRLF sequences and multiple line breaks are valid
                    if (LIKELY(!line_buffer.empty())) {
                        auto word = std::string(line_buffer.begin(), line_buffer.end());
                        auto first_letter = utils::to_lower(line_buffer[0]);
                        line_buffer.clear();
                        if (LIKELY(utils::to_lower(line_buffer[0]) == last_first_byte)) {
                            last_ancestor->push_word(std::move(word), 1, node_pool.get());
                        } else {
                            last_first_byte = first_letter;
                            last_ancestor = forward_index->children
                                .try_emplace(first_letter, node_pool->alloc())
                                .first.second;
                            last_ancestor->push_word(std::move(word), 1, node_pool.get());
                        }
                    }
                }
            }

            // In case the buffer did not end with a new line,
            // push the remaining chars
            if (!line_buffer.empty()) {
                auto word = std::string(line_buffer.begin(), line_buffer.end());
                forward_index->push_word(std::move(word), 0, node_pool.get());
            }
        }

    public:

        /// Creates a new, empty Dictionary.
        Dictionary() :
            forward_index(std::make_unique<WordNode>()),
            node_pool(std::make_unique<utils::memory_pool<WordNode>>()) {
        }

        Dictionary(const Dictionary &) = delete;
        Dictionary(Dictionary &&) = delete;
        Dictionary &operator=(const Dictionary &) = delete;
        Dictionary &operator=(Dictionary &&) = delete;

        ~Dictionary() {
            forward_index.reset();
            node_pool.reset();
        }

        void find_words(std::vector<std::string> &vec,
                        const std::string &pattern,
                        int32_t limit,
                        const std::string &cursor) {

            forward_index->find_words(vec, pattern, 0, 0, limit, cursor);
        }

        void merge(Dictionary *other) {
            forward_index->merge(other->forward_index.get());
            node_pool->merge_pools(other->node_pool.get());
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

            // Split the buffer into chunks, one for every thread
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

            for (auto i = 0; i < par_count; i++) {
                int start = indices[i];
                int end = length;
                if (i < (par_count - 1)) {
                    end = indices[i + 1];
                }

                auto dict = std::make_unique<Dictionary>();
                auto thread = std::thread(&Dictionary::load_from_buffer, dict.get(), buffer, start, end);
                loading_threads.emplace_back(std::move(thread));
                result_dictionaries.emplace_back(std::move(dict));
            }

            // Wait until parsing finishes
            for (auto &thread : loading_threads) {
                thread.join();
            }

            for (auto &dict : result_dictionaries) {
                // std::cout
                //     << " - " << dict->calculate_size() << " words, "
                //     << dict->count_nodes() << " nodes" << std::endl;
                merge(dict.get());
            }

            // std::cout << "======================" << std::endl
            //          << " - " << calculate_size() << " words, "
            //         << count_nodes() << " nodes" << std::endl;
        }
    };
}

#endif //CROSSWORD_HELPER_DICTIONARY_HPP
