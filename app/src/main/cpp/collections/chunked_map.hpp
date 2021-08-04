#ifndef CROSSWORD_HELPER_CHUNKED_MAP_HPP
#define CROSSWORD_HELPER_CHUNKED_MAP_HPP

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include "../utils/memory_pool.hpp"

namespace crossword::collections {

    template <typename V>
    struct map_chunk {

        using pointer_type = V*;

        char key1, key2, key3;
        pointer_type value1, value2, value3;

        map_chunk() {
            key1 = key2 = key3 = 0;
            value1 = nullptr;
            value2 = nullptr;
            value3 = nullptr;
        };
    };

    template <typename V>
    class chunked_map;

    template <typename V>
    class chunked_map_iterator {

    protected:

        chunked_map<V> *map;
        int16_t index;

    public:

        using iterator_category = std::random_access_iterator_tag;
        using difference_type = int16_t;
        using element_type = std::pair<char, V*>;

        chunked_map_iterator(chunked_map<V> *map, int16_t index) : map(map), index(index) { }
        chunked_map_iterator(const chunked_map_iterator &other) = default;
        ~chunked_map_iterator() {}

        operator bool() const {
            return index >= 0 && index < map->size;
        }

        bool operator ==(const chunked_map_iterator &other) const {
            return map == other.map && index == other.index;
        }

        chunked_map_iterator& operator +=(const difference_type &delta) {
            index += delta;
            return *this;
        }

        chunked_map_iterator& operator -=(const difference_type &delta) {
            index -= delta;
            return *this;
        }

        chunked_map_iterator& operator ++() {
            ++index;
            return *this;
        }

        chunked_map_iterator& operator --() {
            --index;
            return *this;
        }

        chunked_map_iterator operator ++(int) {
            auto temp(*this);
            ++index;
            return temp;
        }

        chunked_map_iterator operator --(int) {
            auto temp(*this);
            --index;
            return temp;
        }

        chunked_map_iterator operator +(const difference_type &delta) {
            auto old_index = index;
            index += delta;
            auto temp(*this);
            index = old_index;
            return temp;
        }

        chunked_map_iterator operator -(const difference_type &delta) {
            auto old_index = index;
            index -= delta;
            auto temp(*this);
            index = old_index;
            return temp;
        }

        void fill_element(element_type *element) {
            auto chunk_index = index / 3;
            auto index_inner = index - (chunk_index * 3);
            map_chunk<V> *chunk = &map->chunks[chunk_index];
            switch (index_inner) {
                case 0:
                    element->first = chunk->key1;
                    element->second = chunk->value1;
                    break;
                case 1:
                    element->first = chunk->key2;
                    element->second = chunk->value2;
                    break;
                case 2:
                    element->first = chunk->key3;
                    element->second = chunk->value3;
                    break;
                default:
                    break;
            }
        }

        element_type get_element() {
            element_type pair = std::make_pair((char) 0, nullptr);
            fill_element(&pair);
            return pair;
        }

        element_type operator*() {
            return get_element();
        }
    };

    template <typename V>
    class chunked_map {
        
        using iterator = chunked_map_iterator<V>;

    public:

        map_chunk<V>* chunks;
        int16_t size;
        int16_t capacity;

    private:

        void resize_by_one(crossword::utils::memory_pool<map_chunk<V>>* pool) {
            auto old_chunk_count = capacity / 3;
            auto new_chunks = pool->alloc(old_chunk_count + 1);
            if (capacity > 0) {
                for (auto i = 0; i < old_chunk_count; ++i) {
                    new_chunks[i] = chunks[i];
                }
            }
            chunks = new_chunks;
            capacity += 3;
        }

    public:

        chunked_map() : chunks(nullptr), size(0), capacity(0) {
        }

        bool empty() const noexcept {
            return size == 0;
        }

        iterator begin() {
            return iterator(this, 0);
        }

        iterator end() {
            return iterator(this, size);
        }

        inline iterator find_naive(char key) {
            auto it = begin();
            for (; it != end(); ++it) {
                if (it.get_element().first == key) {
                    break;
                }
            }
            return it;
        }

        inline iterator find_chunk_linear(char key) {
            int16_t chunk_index = 0;
            int16_t index_inner = 0;
            int16_t full_index = 0;
            int16_t allocated_chunks = capacity / 3;
            while (chunk_index < allocated_chunks) {
                auto chunk = &chunks[chunk_index];
                if (chunk->key1 == key) {
                    index_inner = 0;
                    break;
                } else if (chunk->key2 == key) {
                    index_inner = 1;
                    break;
                } else if (chunk->key3 == key) {
                    index_inner = 2;
                    break;
                }
                ++chunk_index;
            }

            full_index = std::min(static_cast<int16_t>(chunk_index * 3 + index_inner), size);
            return iterator(this, full_index);
        }

        iterator find(char key) {
            return find_chunk_linear(key);
        }

        std::pair<std::pair<char, V*>, bool> try_emplace(
            char key,
            V* value,
            crossword::utils::memory_pool<map_chunk<V>>* pool) {

            auto it = find(key);
            if (it != end()) {
                return std::make_pair(it.get_element(), false);
            }

            if (size == capacity) {
                resize_by_one(pool);
            }

            auto chunk_index = size / 3;
            auto index_inner = size - (chunk_index * 3);
            auto chunk = &chunks[chunk_index];
            switch (index_inner) {
                case 0:
                    chunk->key1 = key;
                    chunk->value1 = value;
                    break;
                case 1:
                    chunk->key2 = key;
                    chunk->value2 = value;
                    break;
                case 2:
                    chunk->key3 = key;
                    chunk->value3 = value;
                    break;
                default:
                    break;
            }

            size++;
            return std::make_pair(it.get_element(), true);
        }
    };
}

#endif // CROSSWORD_HELPER_CHUNKED_MAP_HPP