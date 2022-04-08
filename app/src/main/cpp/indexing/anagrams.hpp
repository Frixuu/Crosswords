#ifndef CROSSWORD_HELPER_ANAGRAMS_HPP
#define CROSSWORD_HELPER_ANAGRAMS_HPP

#include "word_index.hpp"

namespace crossword::indexing {

    /// The anagram index stores words in a way
    /// that allows for fast lookup of word anagrams.
    class AnagramIndex final : public WordIndex {
    public:
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
            return {"TODO"};
        }
    };
}

#endif // CROSSWORD_HELPER_ANAGRAMS_HPP
