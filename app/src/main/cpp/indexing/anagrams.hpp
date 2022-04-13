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
        virtual void load_from_buffer(const uint8_t* buffer,
                                      const size_t start,
                                      const size_t end) override {
            // TODO: implement
        }

        virtual void load_from_buffer_parallel(const uint8_t* buffer,
                                               const int length,
                                               const int factor) override {
            load_from_buffer_parallel_impl<AnagramIndex>(buffer, length, factor);
        }
    };
}

#endif // CROSSWORD_HELPER_ANAGRAMS_HPP
