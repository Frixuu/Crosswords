#ifndef CROSSWORD_HELPER_ARENA_HPP
#define CROSSWORD_HELPER_ARENA_HPP

#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include "macros.hpp"

namespace crossword::utils {

    template<typename T>
    class arena_segment {

    private:

        std::unique_ptr<T[]> data;
        size_t size;
        size_t used;

    public:

        /// Creates a new segment that can hold n objects of type T.
        explicit arena_segment(size_t size) : size(size), used(0) {
            data = std::make_unique<T[]>(size);
        }

        arena_segment(arena_segment&& other) noexcept : size(other.size), used(other.used) {
            data.swap(other.data);
        }

        /// Returns true if all the allocated slots are taken.
        inline bool full() const noexcept {
            return used == size;
        }

        /// Returns true if there are at least n free slots in this segment.
        inline bool can_allocate(size_t n) const noexcept {
            return used + n <= size;
        }

        /// Allocates a single object.
        /// This method does NOT check whether the segment is full.
        T* alloc() {
            auto x = data.get() + used;
            ++used;
            return x;
        }

        /// Allocates an array of n objects.
        /// This method does NOT check whether the segment is full.
        T* alloc(size_t n) {
            auto x = data.get() + used;
            used += n;
            return x;
        }

        void dealloc_last() {
            --used;
        }
    };

    template<typename T>
    class arena {

    private:

        const size_t typical_size = 8192;

        std::vector<arena_segment<T>> segments;
        size_t current_segment;

        /// Creates a new segment with default size.
        inline void push_new_segment() {
            push_new_segment(0);
        }

        /// Creates a new segment that can hold at least n items.
        void push_new_segment(size_t min_size) {
            segments.emplace_back(std::move(arena_segment<T>(std::max(min_size, typical_size))));
        }

        /// Moves an existing segment and attaches it to this arena.
        void push_existing_segment(arena_segment<T>&& segment) {
            segments.emplace_back(std::move(segment));
        }

    public:

        /// Creates a new arena allocator.
        arena() : current_segment(0) {
            push_new_segment();
        }

        /// Moves all segments belonging to some other arena to this arena.
        void merge(arena<T> *other) {
            auto it = std::make_move_iterator(other->segments.begin());
            auto end = std::make_move_iterator(other->segments.end());
            while (it != end) {
                push_existing_segment(std::move(*it));
                ++it;
            }
        }

        /// Allocates a single object.
        T* alloc() {
            auto segment = &segments[current_segment];

            // Warning: We know that the segments are merged
            // when no more objects get allocated in the arena,
            // but if that is no longer the case,
            // replace the `if` with `while`
            if (UNLIKELY(segment->full())) {
                push_new_segment();
                ++current_segment;
                segment = &segments[current_segment];
            }

            return segment->alloc();
        }

        /// Allocates an array of n objects.
        /// Returns a pointer to the first element of this array.
        T* alloc(size_t n) {
            auto segment = &segments[current_segment];

            // Ditto
            if (UNLIKELY(!segment->can_allocate(n))) {
                push_new_segment(n);
                ++current_segment;
                segment = &segments[current_segment];
            }

            return segment->alloc(n);
        }

        /// Deallocates the last allocated object
        /// (decrements the internal pointer by one).
        void dealloc_last() {
            auto segment = &segments[current_segment];
            segment->dealloc_last();
        }
    };
}

#endif // CROSSWORD_HELPER_ARENA_HPP