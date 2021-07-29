#ifndef CROSSWORD_HELPER_NATIVE_LIB_HPP
#define CROSSWORD_HELPER_NATIVE_LIB_HPP

#include <android/asset_manager_jni.h>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include "word_node.hpp"
#include "string_utils.hpp"

namespace crossword {

    /// Stores a set of words from a particular language in forms
    /// that allow for easy indexing.
    class Dictionary {

    private:

        std::unique_ptr<WordNode> forward_index;

        /// Parses lines from a UTF-8 encoded buffer and adds them to the dictionary.
        /// @param buffer Pointer to the data buffer.
        /// @param start Index to start searching from.
        /// @param end Exclusive end index of buffer parsing.
        void load_from_buffer(const char *buffer, int start, int end) {
            std::vector<char> line_buffer;
            auto index = start;
            auto bytes_till_codepoint_end = 0;
            while (index < end) {
                auto byte = buffer[index++];
                if (bytes_till_codepoint_end > 0) {
                    bytes_till_codepoint_end--;
                    line_buffer.emplace_back(byte);
                } else {
                    if (byte != '\n' && byte != '\r') {
                        line_buffer.emplace_back(byte);
                        bytes_till_codepoint_end = utils::codepoint_size(byte) - 1;
                    } else {
                        if (!line_buffer.empty()) {
                            auto word = std::string(line_buffer.begin(), line_buffer.end());
                            line_buffer.clear();
                            forward_index->push_word(std::move(word), 0);
                        }
                    }
                }
            }

            // In case the buffer did not end with a new line,
            // push the remaining chars
            if (!line_buffer.empty()) {
                auto word = std::string(line_buffer.begin(), line_buffer.end());
                forward_index->push_word(std::move(word), 0);
            }
        }

    public:

        /// Creates a new, empty Dictionary.
        Dictionary() {
            forward_index = std::make_unique<WordNode>();
        }

        ~Dictionary() {
            forward_index.reset();
        }

        void find_words(std::vector<std::string> &vec, const std::string &pattern) {
            forward_index->find_words(vec, pattern, 0, -1, std::string());
        }

        /// Puts words in this dictionary, using multiple threads.
        /// @param buffer Buffer to UTF-8 data
        /// @param length Length of the buffer
        /// @param par_count How many threads will be spawned to parse the buffer?
        void load_from_buffer_par(const char *buffer, int length, int par_count) {
            std::vector<std::thread> loadingThreads;

            auto indices = new int[par_count];
            indices[0] = 0;

            // Split the buffer into par_count equal-sized segments,
            // then align the segment size to the nearest line break
            for (int i = 1; i < par_count; i++) {
                int candidate = i * length / par_count;
                while (candidate < length) {
                    auto thisByte = buffer[candidate++];
                    auto nextByte = buffer[candidate];
                    // TODO: Handle UTF-8 codepoints
                    if ((thisByte == '\n' || thisByte == '\r') &&
                        nextByte != '\n' && nextByte != '\r') {
                        indices[i] = candidate + 1;
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

                auto thread = std::thread(&Dictionary::load_from_buffer, this, buffer, start, end);
                loadingThreads.emplace_back(std::move(thread));
            }

            // Wait until parsing finishes
            for (auto &thread : loadingThreads) {
                thread.join();
            }
        }
    };
}

#endif //CROSSWORD_HELPER_NATIVE_LIB_HPP
