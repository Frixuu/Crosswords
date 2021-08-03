#ifndef CROSSWORD_HELPER_MMAP_HPP
#define CROSSWORD_HELPER_MMAP_HPP

#include <utility>
#ifdef _WIN32
#include <windows.h>
#include <sysinfoapi.h>
#include <memoryapi.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

namespace crossword::utils {

#ifdef _WIN32
    class mapping_handles_windows {
    private:
        HANDLE file;
        HANDLE mapping;
    public:
        mapping_handles_windows()
            : file(INVALID_HANDLE_VALUE), mapping(INVALID_HANDLE_VALUE) { }
        mapping_handles_windows(HANDLE file, HANDLE mapping)
            : file(file), mapping(mapping) { }
        mapping_handles_windows(const mapping_handles_windows&) = delete;
        mapping_handles_windows& operator=(const mapping_handles_windows&) = delete;
        mapping_handles_windows(mapping_handles_windows&&) = delete;
        mapping_handles_windows& operator=(mapping_handles_windows&& other) {
            if (mapping != INVALID_HANDLE_VALUE && mapping != nullptr) {
                ::CloseHandle(file);
                ::CloseHandle(mapping);
            }
            file = other.file;
            other.file = INVALID_HANDLE_VALUE;
            mapping = other.mapping;
            other.mapping = INVALID_HANDLE_VALUE;
            return *this;
        };
        ~mapping_handles_windows() {
            if (mapping != INVALID_HANDLE_VALUE && mapping != nullptr) {
                ::CloseHandle(file);
                ::CloseHandle(mapping);
            }
        }
    };
#else
    class mapping_handles_unix {
    private:
        int fd;
        off_t size;
        void* mapping;
    public:
        mapping_handles_unix() :fd(-1), size(-1), mapping(nullptr) { }
        mapping_handles_unix(int fd, void* mapping, off_t size)
            : fd(fd), size(size), mapping(mapping) { }
        mapping_handles_unix(const mapping_handles_unix&) = delete;
        mapping_handles_unix& operator=(const mapping_handles_unix&) = delete;
        mapping_handles_unix(mapping_handles_unix&&) = delete;
        mapping_handles_unix& operator=(mapping_handles_unix&& other) {
            if (fd >= 0) {
                ::munmap(mapping, size);
                ::close(fd);
            }
            fd = other.fd;
            other.fd = -1;
            size = other.size;
            other.size = -1;
            mapping = other.mapping;
            other.mapping = nullptr;
            return *this;
        }
        ~mapping_handles_unix() {
            if (fd >= 0) {
                ::munmap(mapping, size);
                ::close(fd);
            }
        }
    };
#endif

    class file_mapping {
    private:
#ifdef _WIN32
        mapping_handles_windows native_handles;
#else
        mapping_handles_unix native_handles;
#endif
    public:
        const char* data;
        uint64_t size;

#ifdef _WIN32
        file_mapping(LPCWSTR filename) : data(nullptr), size(-1) {
            auto file = ::CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, nullptr);
            if (file == INVALID_HANDLE_VALUE) {
                return;
            }
            
            LARGE_INTEGER file_size = { 0 };
            ::GetFileSizeEx(file, &file_size);
            size = static_cast<uint64_t>(file_size.QuadPart);

            auto hmap = ::CreateFileMappingW(file, NULL, PAGE_READONLY, 0, 0, nullptr);
            if (hmap == nullptr) {
                ::CloseHandle(file);
                return;
            }

            data = static_cast<const char*>(::MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 0));
            if (data == nullptr) {
                ::CloseHandle(hmap);
                ::CloseHandle(file);
                return;
            }

            native_handles = mapping_handles_windows(file, hmap);
        }
#else
        file_mapping(const char* filename) : data(nullptr), size(-1) {
            auto fd = ::open(filename, O_RDONLY);
            if (fd < 0) {
                return;
            }

            size = ::lseek(fd, 0, SEEK_END); 
            auto mapped_data = ::mmap(nullptr, size, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
            data = reinterpret_cast<char*>(mapped_data);
            native_handles = mapping_handles_unix(fd, mapped_data, size);
        }
#endif
    };
}

#endif // CROSSWORD_HELPER_MMAP_HPP