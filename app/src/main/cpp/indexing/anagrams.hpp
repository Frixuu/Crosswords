#ifndef CROSSWORD_HELPER_ANAGRAMS_HPP
#define CROSSWORD_HELPER_ANAGRAMS_HPP

#include "word_index.hpp"
#include "../utils/android.hpp"

#include <thread>

namespace crossword::indexing {

    /// The anagram index stores words in a way
    /// that allows for fast lookup of word anagrams.
    class AnagramIndex final : public WordIndex {
    public:
        AnagramIndex() = default;
        ~AnagramIndex() = default;

        /// Tries to merge this index with another index.
        /// @returns True if the merge was successful.
        /// Merge can be unsuccessful if the other index is not an anagram index
        /// or not enough memory is available.
        /// @details No matter the result, the other index is assumed to be in an invalid state.
        virtual bool merge(WordIndex* other) override {
            auto other_index = dynamic_cast<AnagramIndex*>(other);
            if (other_index == nullptr) {
                return false;
            }

            // TODO: implement

            return true;
        }

        /// Returns the set of words that are anagrams of the given word.
        /// @returns Anagrams of the given word.
        /// @param input Word to find anagrams of.
        /// @param max_results Maximum number of results to return.
        virtual std::vector<std::u8string> lookup(const std::u8string& input,
                                                  const size_t max_results) const override {
            return {u8"TODO"};
        }

        /// Parses lines from a UTF-8 encoded buffer and adds them to the index.
        /// @param buffer Pointer to the data buffer.
        /// @param start Index to start searching from.
        /// @param end Exclusive end index of buffer parsing.
        void load_from_buffer(const uint8_t* buffer, const size_t start, const size_t end) {
            // TODO: implement
        }

        /// Puts words in this dictionary, using multiple threads.
        /// @param buffer Buffer to UTF-8 data
        /// @param length Length of the buffer
        /// @param factor How many threads will be spawned to parse the buffer?
        void load_from_buffer_parallel(const uint8_t* buffer, const int length, const int factor) {
            auto thread_count = std::clamp(factor, 1, 32);
            std::vector<std::thread> threads;
            std::vector<std::unique_ptr<AnagramIndex>> result_dictionaries;
            auto logger = utils::android::log::tag("MissingLettersIndex");

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

            logger.i("Split buffer to %d chunks", thread_count);

            // Spawn a thread for every chunk
            for (auto i = 0; i < thread_count; i++) {
                int start = indices[i];
                int end = length;
                if (i < (thread_count - 1)) {
                    end = indices[i + 1];
                }

                // Each thread gets their own partial index,
                // because merging them should be cheaper than locking
                // and making other threads' caches dirty
                auto index = std::make_unique<AnagramIndex>();
                threads.emplace_back(&AnagramIndex::load_from_buffer, index.get(), buffer,
                                     start, end);
                result_dictionaries.push_back(std::move(index));
            }

            // Wait until parsing finishes
            for (auto& thread : threads) {
                thread.join();
            }

            logger.i("Successfully loaded index on %d threads", thread_count);

            // Merge the results.
            // It's pretty cheap as long as the input was at-least k-sorted
            for (auto& index : result_dictionaries) {
                this->merge(index.get());
            }

            logger.i("Successfully merged %d indexes", thread_count);
        }
    };
}

#endif // CROSSWORD_HELPER_ANAGRAMS_HPP
