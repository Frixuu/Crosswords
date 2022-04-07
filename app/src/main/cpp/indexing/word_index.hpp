#ifndef CROSSWORD_HELPER_WORD_INDEX_HPP
#define CROSSWORD_HELPER_WORD_INDEX_HPP

#include <string>
#include <vector>

namespace crossword::indexing {

    /// A word index implements a specific algorithm and data structure
    /// for storing words and retrieving them depending on the input.
    /// @details For example, a "rhyme" index would reverse the words before storing them
    /// to make the retrieval operation faster.
    class WordIndex {
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
        virtual std::vector<std::string> lookup(const std::string& input,
                                                const size_t max_results) const = 0;
    };
}

#endif // CROSSWORD_HELPER_WORD_INDEX_HPP
