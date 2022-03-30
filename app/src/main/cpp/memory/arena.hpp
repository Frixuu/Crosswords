#ifndef CROSSWORD_HELPER_ARENA_HPP
#define CROSSWORD_HELPER_ARENA_HPP

#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

namespace crossword::memory {

    /// @brief Segment for an arena allocator.
    /// @tparam T Type of the object this segment can allocate.
    /// @tparam value_init Whether the allocated objects will be value- or default-initialized.
    template <typename T, bool value_init = true>
    class ArenaSegment {
    private:
        std::unique_ptr<T[]> data;
        size_t size;
        size_t used;

    public:
        /// Creates a new segment that can hold n objects of type T.
        explicit ArenaSegment(size_t size) : size(size), used(0) {
            if constexpr (value_init) {
                data = std::make_unique<T[]>(size);
            } else {
                // TODO: Replace with make_unique_for_overwrite when Clang supports it
                data = std::unique_ptr<T[]>(new T[size]);
            }
        }

        ArenaSegment(ArenaSegment&& other) noexcept : size(other.size), used(other.used) {
            data.swap(other.data);
        }

        ArenaSegment& operator=(ArenaSegment&& other) noexcept {
            std::swap(size, other.size);
            std::swap(used, other.used);
            data.swap(other.data);
            return *this;
        }

        ArenaSegment(const ArenaSegment&) = delete;
        ArenaSegment& operator=(const ArenaSegment&) = delete;

        /// Returns true if no objects have been allocated in this segment yet.
        constexpr inline bool empty() noexcept {
            return used == 0;
        }

        /// Returns true if all the allocated slots are taken.
        constexpr inline bool full() noexcept {
            return used == size;
        }

        /// Returns true if there are at least n free slots in this segment.
        constexpr inline bool can_allocate(size_t n) noexcept {
            return used + n <= size;
        }

        /// Allocates a single object.
        /// This method does NOT check whether the segment is full.
        inline T* alloc() {
            return alloc(1);
        }

        /// Allocates an array of n objects.
        /// This method does NOT check whether the segment is full.
        inline T* alloc(size_t n) {
            auto x = data.get() + used;
            used += n;
            return x;
        }

        /// If this segment is not empty, decrements the segment counter by 1.
        /// THIS IS UNSAFE as it invalidates the last returned pointer. Use with care.
        inline void dealloc_last() noexcept {
            if (!empty()) [[likely]] {
                --used;
            }
        }
    };

    static_assert(sizeof(ArenaSegment<int>) == 3 * sizeof(void*),
                  "ArenaSegment has to be 3 pointers in size");

    template <typename T, bool value_init = true>
    class Arena {
    private:
        const size_t min_size = 512;
        const size_t typical_size = 16384;

        std::vector<ArenaSegment<T, value_init>> segments;
        size_t current_segment;

        /// Creates a new segment with default size.
        inline void push_new_segment() {
            push_new_segment(typical_size);
        }

        /// Creates a new segment that can hold at least n items.
        inline void push_new_segment(size_t segment_size) {
            segments.emplace_back(std::max(segment_size, min_size));
        }

        /// Moves an existing segment and attaches it to this Arena.
        inline void push_existing_segment(ArenaSegment<T, value_init>&& segment) {
            segments.push_back(std::move(segment));
        }

    public:
        /// Creates a new Arena allocator.
        Arena() : current_segment(0) {
            push_new_segment();
        }

        /// Moves all segments belonging to some other Arena to this Arena.
        void merge(Arena<T, value_init>* other) {
            auto it = std::make_move_iterator(other->segments.begin());
            auto end = std::make_move_iterator(other->segments.end());
            while (it != end) {
                push_existing_segment(std::move(*it));
                ++it;
            }
        }

        /// Allocates a single object.
        inline T* alloc() {
            return alloc(1);
        }

        /// Allocates a contiguous array of n objects.
        /// Returns a pointer to the first element of this array.
        T* alloc(size_t n) {
            auto segment = &segments.at(current_segment);

            while (!segment->can_allocate(n)) [[unlikely]] {
                ++current_segment;

                if (current_segment == segments.size()) [[likely]] {
                    push_new_segment(std::max(n, typical_size));
                }

                segment = &segments.at(current_segment);
            }

            return segment->alloc(n);
        }

        /// Deallocates the last allocated object
        /// (decrements the internal pointer by one).
        inline void dealloc_last() noexcept {
            auto segment = &segments.at(current_segment);
            segment->dealloc_last();
        }
    };
}

#endif // CROSSWORD_HELPER_ARENA_HPP
