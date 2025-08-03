#include "DotBlue/MemChunkAllocator.h"
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

namespace DotBlue {
MemChunkAllocator::MemChunkAllocator(const std::string& filePath, std::size_t totalSize, std::size_t chunkSize)
    : _totalSize(totalSize), _chunkSize(chunkSize)
{
    if (chunkSize == 0 || totalSize == 0 || totalSize < chunkSize)
        throw std::invalid_argument("Invalid chunk or total size");

    mapFile(filePath, totalSize);
    _totalChunks = totalSize / chunkSize;
    _freeList.resize(_totalChunks, true);
}

MemChunkAllocator::~MemChunkAllocator() {
    unmapFile();
}

void* MemChunkAllocator::allocate() {
    for (std::size_t i = 0; i < _totalChunks; ++i) {
        if (_freeList[i]) {
            _freeList[i] = false;
            return static_cast<uint8_t*>(_base) + i * _chunkSize;
        }
    }
    return nullptr; // Out of memory
}

void MemChunkAllocator::free(void* ptr) {
    if (!ptr) return;

   // auto offset = static_cast<uint8_t*>(ptr) - static_cast<uint8_t*>(_base);
    auto offset = static_cast<std::size_t>(static_cast<uint8_t*>(ptr) - static_cast<uint8_t*>(_base));
    if (offset % _chunkSize != 0 || offset >= _totalSize)
        throw std::invalid_argument("Invalid pointer");

    std::size_t index = offset / _chunkSize;
    _freeList[index] = true;
}

void MemChunkAllocator::mapFile(const std::string& filePath, std::size_t size) {
#ifdef _WIN32
    _fileHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE,
                              0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (_fileHandle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open file");

    LARGE_INTEGER liSize;
    liSize.QuadPart = static_cast<LONGLONG>(size);
    SetFilePointerEx(_fileHandle, liSize, nullptr, FILE_BEGIN);
    SetEndOfFile(_fileHandle);

    _mappingHandle = CreateFileMappingA(_fileHandle, nullptr, PAGE_READWRITE, 0, 0, nullptr);
    if (!_mappingHandle)
        throw std::runtime_error("Failed to create file mapping");

    _base = MapViewOfFile(_mappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!_base)
        throw std::runtime_error("Failed to map view of file");

#else
    _fd = open(filePath.c_str(), O_RDWR | O_CREAT, 0666);
    if (_fd < 0)
        throw std::runtime_error("Failed to open file");

    if (ftruncate(_fd, size) != 0)
        throw std::runtime_error("Failed to resize file");

    _base = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
    if (_base == MAP_FAILED)
        throw std::runtime_error("Failed to mmap file");
#endif
}

void MemChunkAllocator::unmapFile() {
    if (_base) {
#ifdef _WIN32
        UnmapViewOfFile(_base);
        if (_mappingHandle) CloseHandle(_mappingHandle);
        if (_fileHandle) CloseHandle(_fileHandle);
#else
        munmap(_base, _totalSize);
        if (_fd >= 0) close(_fd);
#endif
    }
    _base = nullptr;
}

} // namespace DotBlue
