#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace DotBlue {
class MemChunkAllocator {
public:
    MemChunkAllocator(const std::string& filePath, std::size_t totalSize, std::size_t chunkSize);
    ~MemChunkAllocator();

    void* allocate();
    void free(void* ptr);
    void* baseAddress() const { return _base; }
    std::size_t chunkSize() const { return _chunkSize; }
    std::size_t totalChunks() const { return _totalChunks; }

private:
    void* _base = nullptr;
    std::size_t _totalSize = 0;
    std::size_t _chunkSize = 0;
    std::size_t _totalChunks = 0;

    std::vector<bool> _freeList;

#ifdef _WIN32
    void* _fileHandle = nullptr;
    void* _mappingHandle = nullptr;
#else
    int _fd = -1;
#endif

    void mapFile(const std::string& filePath, std::size_t size);
    void unmapFile();
};

} // namespace DotBlue
