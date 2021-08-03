#ifndef CROSSWORD_HELPER_MEMORY_POOL_HPP
#define CROSSWORD_HELPER_MEMORY_POOL_HPP

#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include "macros.hpp"

namespace crossword::utils {

    template<typename T>
    struct memory_pool_segment {
        std::unique_ptr<T[]> data;
        size_t size;
        size_t used;

        memory_pool_segment(size_t size) : size(size), used(0) {
            data = std::make_unique<T[]>(size);
        }

        memory_pool_segment(memory_pool_segment&& other) noexcept : size(other.size), used(other.used) {
            data.swap(other.data);
        }

        bool full() const noexcept {
            return used == size;
        }

        T* alloc() {
            auto x = data.get() + used;
            ++used;
            return x;
        }

        T* alloc(size_t n) {
            if (UNLIKELY(used + n > size)) {
                return nullptr;
            }
            auto x = data.get() + used;
            used += n;
            return x;
        }

        void dealloc_last() {
            --used;
        }
    };

    template<typename T>
    class memory_pool {

    private:

        const size_t typical_size = 8192;
        std::vector<memory_pool_segment<T>> segments;
        size_t current_segment;

        inline void push_new_segment() {
            push_new_segment(0);
        }

        void push_new_segment(size_t min_size) {
            segments.emplace_back(std::move(memory_pool_segment<T>(std::max(min_size, typical_size))));
        }

        void push_existing_segment(memory_pool_segment<T>&& segment) {
            segments.emplace_back(std::move(segment));
        }

    public:

        memory_pool() : current_segment(0) {
            push_new_segment();
        }

        void merge_pools(memory_pool<T>* other) {
            auto it = std::make_move_iterator(other->segments.begin());
            auto end = std::make_move_iterator(other->segments.end());
            while (it != end) {
                push_existing_segment(std::move(*it));
                ++it;
            }
        }

        T* alloc() {
            auto segment = &segments[current_segment];

            if (UNLIKELY(segment->full())) {
                push_new_segment();
                ++current_segment;
                segment = &segments[current_segment];
            }

            return segment->alloc();
        }

        T* alloc(size_t n) {
            auto segment = &segments[current_segment];
            auto ptr = segment->alloc(n);
            
            if (UNLIKELY(ptr == nullptr)) {
                push_new_segment(n);
                ++current_segment;
                segment = &segments[current_segment];
                ptr = segment->alloc(n);
            }

            return ptr;
        }

        void dealloc_last() {
            auto segment = &segments[current_segment];
            segment->dealloc_last();
        }
    };
}

#endif // CROSSWORD_HELPER_MEMORY_POOL_HPP