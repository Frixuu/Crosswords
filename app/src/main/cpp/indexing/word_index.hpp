#ifndef CROSSWORD_HELPER_WORD_INDEX_HPP
#define CROSSWORD_HELPER_WORD_INDEX_HPP

#include "../utils/android.hpp"

#include <concepts>
#include <string>
#include <thread>
#include <vector>

namespace crossword::indexing {

    /// A word index implements a specific algorithm and data structure
    /// for storing words and retrieving them depending on the input.
    /// @details For example, a "rhyme" index would reverse the words before storing them
    /// to make the retrieval operation faster.
    class WordIndex;

    /// This is a class that derives from WordIndex.
    template <class T>
    concept Index = std::is_base_of_v<WordIndex, T>;

    class WordIndex {
    protected:

        template <Index T>
        void load_from_buffer_parallel_impl(const uint8_t* buffer,
                                            const int length,
                                            const int factor) {
            auto thread_count = std::clamp(factor, 1, 32);
            std::vector<std::thread> threads;
            std::vector<std::unique_ptr<T>> partial_indexes;
            auto logger = utils::android::log::tag("WordIndex");

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
                auto index = std::make_unique<T>();
                threads.emplace_back(&WordIndex::load_from_buffer, index.get(), buffer, start, end);
                partial_indexes.push_back(std::move(index));
            }

            // Wait until parsing finishes
            for (auto& thread : threads) {
                thread.join();
            }

            logger.i("Successfully loaded index on %d threads", thread_count);

            // Merge the results.
            // It's pretty cheap as long as the input was at-least k-sorted
            for (auto& index : partial_indexes) {
                this->merge(index.get());
            }

            logger.i("Successfully merged %d indexes", thread_count);
        }

    public:

        virtual ~WordIndex(){};

        /// Tries to merge this index with another index.
        /// @returns True if the merge was successful, false otherwise.
        /// Merge might fail because the indexes are incompatible
        /// or because enough memory is not available.
        /// @details It is assumed that after a merge (no matter if successful or not),
        /// the passed index is no longer usable.
        virtual bool merge(WordIndex* other) = 0;

        /// Looks up matching words in an index.
        /// What exactly is considered a match is up to the implementation.
        /// @returns A vector of matching words.
        /// @param input The word to look up.
        /// @param max_results The maximum number of results to return.
        virtual std::vector<std::u8string> lookup(const std::u8string& input,
                                                  const size_t max_results) const = 0;

        /// Reads the provided buffer and adds the contents to this index.
        /// @param buffer The UTF8 buffer to read from.
        /// @param start Index to start searching from.
        /// @param end Exclusive end index of buffer parsing.
        virtual void load_from_buffer(const uint8_t* buffer,
                                      const size_t start,
                                      const size_t end) = 0;

        virtual void load_from_buffer_parallel(const uint8_t* buffer,
                                               const int length,
                                               const int factor) = 0;
    };
}

#endif // CROSSWORD_HELPER_WORD_INDEX_HPP
