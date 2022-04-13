#ifndef CROSSWORD_HELPER_MISSING_LETTERS_HPP
#define CROSSWORD_HELPER_MISSING_LETTERS_HPP

#include "../memory/arena.hpp"
#include "../utils/android.hpp"
#include "../utils/utf8.hpp"
#include "../word_node.hpp"
#include "word_index.hpp"

#include <algorithm>
#include <iterator>
#include <thread>

namespace crossword::indexing {

    using namespace ::crossword::collections;
    using namespace ::crossword::utils;
    using ::crossword::memory::Arena;

    /// The 'missing letters' index stores words in a way
    /// that enables fast lookup of words that have some of the letters missing.
    class MissingLettersIndex final : public WordIndex {
    private:
        std::unique_ptr<WordNode> root;
        std::unique_ptr<Arena<WordNode>> arena_node;
        std::unique_ptr<Arena<MapChunk<uint8_t, WordNode*>>> arena_map_chunk;
        std::unique_ptr<Arena<std::u8string, false>> arena_string;

        /// Adds the word to the index.
        /// @details It is assumed that the word pointer belongs to this indexes' arena_string.
        inline void add(std::u8string* word_ptr) {
            root->push_word(word_ptr, 0, arena_node.get(), arena_map_chunk.get());
        }

    public:

        MissingLettersIndex() :
            root(std::make_unique<WordNode>()),
            arena_node(std::make_unique<Arena<WordNode>>()),
            arena_map_chunk(std::make_unique<Arena<MapChunk<uint8_t, WordNode*>>>()),
            arena_string(std::make_unique<Arena<std::u8string, false>>()) {}

        ~MissingLettersIndex() = default;

        /// Tries to merge this index with another index.
        /// @returns True if the merge was successful.
        /// Merge can be unsuccessful if the other index is not a missing letters index
        /// or not enough memory is available.
        /// @details No matter the result, the other index is assumed to be in an invalid state.
        virtual bool merge(WordIndex* other) override {
            auto other_index = dynamic_cast<MissingLettersIndex*>(other);
            if (other_index == nullptr) {
                return false;
            }

            root->merge(other_index->root.get(), arena_map_chunk.get());
            arena_node->merge(other_index->arena_node.get());
            arena_map_chunk->merge(other_index->arena_map_chunk.get());
            arena_string->merge(other_index->arena_string.get());

            return true;
        }

        /// Returns the set of words that match the provided pattern.
        /// The pattern is assumed to be a string of UTF-8 characters,
        /// where a dot . (0x2E) is considered to be any character.
        /// @result The set of words that match the pattern.
        /// @param input The pattern to match.
        /// @param max_results The maximum number of results to return.
        virtual std::vector<std::u8string> lookup(const std::u8string& input,
                                                  const size_t max_results) const override {
            std::vector<std::u8string> results;
            root->find_words(results, input, 0, 0, max_results);
            return results;
        }

        /// Parses lines from a UTF-8 encoded buffer and adds them to the index.
        /// @param buffer Pointer to the data buffer.
        /// @param start Index to start searching from.
        /// @param end Exclusive end index of buffer parsing.
        virtual void load_from_buffer(const uint8_t* buffer,
                                      const size_t start,
                                      const size_t end) override {
            // Stores contents of the current line so far
            std::vector<uint8_t> line_buffer;
            line_buffer.reserve(128);

            android::log::tag("load_from_buffer").i("Parsing %d bytes", end - start);

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
                        auto word_ptr = arena_string->alloc();
                        *word_ptr = std::u8string(line_buffer.begin(), line_buffer.end());
                        line_buffer.clear();
                        add(word_ptr);
                    }
                }
            }

            // In case the buffer did not end with a new line,
            // push the remaining chars
            if (!line_buffer.empty()) {
                auto word_ptr = arena_string->alloc();
                *word_ptr = std::u8string(line_buffer.begin(), line_buffer.end());
                add(word_ptr);
            }
        }

        virtual void load_from_buffer_parallel(const uint8_t* buffer,
                                               const int length,
                                               const int factor) override {
            load_from_buffer_parallel_impl<MissingLettersIndex>(buffer, length, factor);
        }
    };
}

#endif // CROSSWORD_HELPER_MISSING_LETTERS_HPP
