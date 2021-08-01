#ifndef CROSSWORD_HELPER_CHUNKED_MAP_HPP
#define CROSSWORD_HELPER_CHUNKED_MAP_HPP

#include <algorithm>
#include <iterator>
#include <memory>

namespace crossword::collections {

    template <typename V>
    struct map_chunk {
        char key1, key2, key3;
        std::unique_ptr<V> value1, value2, value3;

        map_chunk() {
            key1 = key2 = key3 = 0;
            value1 = nullptr;
            value2 = nullptr;
            value3 = nullptr;
        };

        map_chunk(const map_chunk& other) = delete;
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

        typedef std::pair<char, std::unique_ptr<V>*> element_type;

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
            auto chunk = &map->chunks[chunk_index];
            switch (index_inner) {
                case 0:
                    element->first = chunk->key1;
                    element->second = &chunk->value1;
                    break;
                case 1:
                    element->first = chunk->key2;
                    element->second = &chunk->value2;
                    break;
                case 2:
                    element->first = chunk->key3;
                    element->second = &chunk->value3;
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

        element_type operator->() {
            return get_element();
        }

        std::unique_ptr<element_type> operator *() {
            auto ptr = std::make_unique<element_type>();
            fill_element(ptr.get());
            return ptr;
        }

    };

    template <typename V>
    class chunked_map {
        
        typedef chunked_map_iterator<V> iterator;

    public:

        std::unique_ptr<map_chunk<V>[]> chunks;
        int16_t size;
        int16_t capacity;

    private:

        void resize_by_one() {
            auto old_chunk_count = capacity / 3;
            auto new_chunks = std::make_unique<map_chunk<V>[]>(old_chunk_count + 1);
            if (capacity > 0) {
                for (auto i = 0; i < old_chunk_count; ++i) {
                    auto old_chunk = &chunks[i];
                    new_chunks[i].key1 = old_chunk->key1;
                    new_chunks[i].key2 = old_chunk->key2;
                    new_chunks[i].key3 = old_chunk->key3;
                    new_chunks[i].value1.swap(old_chunk->value1);
                    new_chunks[i].value2.swap(old_chunk->value2);
                    new_chunks[i].value3.swap(old_chunk->value3);
                }
            }
            chunks.swap(new_chunks);
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

        iterator find(char key) {
            auto it = begin();
            for (; it != end(); ++it) {
                if (it.get_element().first == key) {
                    break;
                }
            }
            return it;
        }

        std::pair<std::pair<char, std::unique_ptr<V>*>, bool> try_emplace(
            char key,
            std::unique_ptr<V>&& value) {

            auto it = find(key);
            if (it != end()) {
                return std::make_pair(it.get_element(), false);
            }

            if (size == capacity) {
                resize_by_one();
            }

            auto chunk_index = size / 3;
            auto index_inner = size - (chunk_index * 3);
            auto chunk = &chunks[chunk_index];
            switch (index_inner) {
                case 0:
                    chunk->key1 = key;
                    chunk->value1 = std::move(value);
                    break;
                case 1:
                    chunk->key2 = key;
                    chunk->value2 = std::move(value);
                    break;
                case 2:
                    chunk->key3 = key;
                    chunk->value3 = std::move(value);
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