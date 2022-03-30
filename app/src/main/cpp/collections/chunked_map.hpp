#ifndef CROSSWORD_HELPER_CHUNKED_MAP_HPP
#define CROSSWORD_HELPER_CHUNKED_MAP_HPP

#include "../memory/arena.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>

namespace crossword::collections {

    using ::crossword::memory::Arena;

    template <typename K, typename V>
    struct MapChunk {
        K key1, key2, key3;
        V value1, value2, value3;
    };

    static_assert(sizeof(MapChunk<uint8_t, void*>) == 4 * sizeof(void*),
                  "MapChunk of char keys must be 4 pointers in size");

    template <typename K, typename V>
    class ChunkedMap;

    template <typename K, typename V>
    class ChunkedMapIterator {
    protected:
        ChunkedMap<K, V>* map;
        int16_t index;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = int16_t;

        ChunkedMapIterator(ChunkedMap<K, V>* map, int16_t index) : map(map), index(index) {}
        ChunkedMapIterator(const ChunkedMapIterator& other) = default;
        ~ChunkedMapIterator() {}

        operator bool() const {
            return index >= 0 && index < map->size;
        }

        bool operator==(const ChunkedMapIterator& other) const {
            return map == other.map && index == other.index;
        }

        ChunkedMapIterator& operator+=(const difference_type& delta) {
            index += delta;
            return *this;
        }

        ChunkedMapIterator& operator-=(const difference_type& delta) {
            index -= delta;
            return *this;
        }

        ChunkedMapIterator& operator++() {
            ++index;
            return *this;
        }

        ChunkedMapIterator& operator--() {
            --index;
            return *this;
        }

        ChunkedMapIterator operator++(int) {
            auto temp(*this);
            ++index;
            return temp;
        }

        ChunkedMapIterator operator--(int) {
            auto temp(*this);
            --index;
            return temp;
        }

        ChunkedMapIterator operator+(const difference_type& delta) {
            auto old_index = index;
            index += delta;
            auto temp(*this);
            index = old_index;
            return temp;
        }

        ChunkedMapIterator operator-(const difference_type& delta) {
            auto old_index = index;
            index -= delta;
            auto temp(*this);
            index = old_index;
            return temp;
        }

        void fill_element(std::pair<K, V>* element) {
            auto chunk_index = index / 3;
            auto index_inner = index - (chunk_index * 3);
            MapChunk<K, V>* chunk = &map->chunks[chunk_index];
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

        std::pair<K, V> get_element() {
            auto pair = std::make_pair(static_cast<K>(0), static_cast<V>(0));
            fill_element(&pair);
            return pair;
        }

        std::pair<K, V> operator*() {
            return get_element();
        }
    };

    static_assert(sizeof(ChunkedMapIterator<uint8_t, void*>) == 2 * sizeof(void*),
                  "ChunkedMapIterator of char keys must be 2 pointers in size");

    template <typename K, typename V>
    class ChunkedMap {
        using iterator = ChunkedMapIterator<K, V>;

    public:
        /// Map of chunks in this map.
        MapChunk<K, V>* chunks;
        /// How many chunks are currently allocated?
        int16_t allocated_chunks;
        /// How many elements are currently in this map?
        int16_t size;

    private:
        /// Resizes this map by one chunk.
        /// This reallocates the chunks, making the pointers to them invalid.
        void resize_by_one(Arena<MapChunk<K, V>>* arena) {
            auto new_chunks = arena->alloc(allocated_chunks + 1);
            if (allocated_chunks > 0) {
                for (auto i = 0; i < allocated_chunks; ++i) {
                    new_chunks[i] = chunks[i];
                }
            }
            chunks = new_chunks;
            allocated_chunks += 1;
        }

    public:
        ChunkedMap() : chunks(nullptr), allocated_chunks(0), size(0) {}

        /// How many elements can be stored in the map without additional allocation?
        inline int16_t capacity() const noexcept {
            return allocated_chunks * 3;
        }

        /// Checks if the map does not contain any elements.
        bool empty() const noexcept {
            return size == 0;
        }

        /// Creates an iterator that points to the first pair of the map.
        /// If the map is empty, equals to end().
        iterator begin() {
            return iterator(this, 0);
        }

        /// Creates an iterator that points to an invalid element
        /// after the last element of the map.
        iterator end() {
            return iterator(this, size);
        }

        /// Tries to find provided key, using linear search.
        inline iterator find_chunk_linear(K key) {
            int16_t chunk_index = 0;
            int16_t index_inner = 0;

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

            return iterator(this,
                            std::min(static_cast<int16_t>(chunk_index * 3 + index_inner), size));
        }

        /// Tries to find a provided key in the map.
        iterator find(K key) {
            return find_chunk_linear(key);
        }

        struct InsertResult {
            std::pair<K, V> entry;
            bool inserted;
        };

        InsertResult try_insert(K key, V value, Arena<MapChunk<K, V>>* arena) {
            auto it = find(key);
            if (it != end()) {
                return {it.get_element(), false};
            }

            if (size == capacity()) {
                resize_by_one(arena);
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
            return {it.get_element(), true};
        }
    };

    static_assert(sizeof(ChunkedMap<uint8_t, void*>) <= 2 * sizeof(void*),
                  "ChunkedMap must not be larger than 2 pointers in size");
}

#endif // CROSSWORD_HELPER_CHUNKED_MAP_HPP
