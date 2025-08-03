#include "DotBlue/DotBlue.h"
#include "DotBlue/MemChunkAllocator.h"
#include <iostream>
#include <cstring>
namespace DotBlue {

void Hello() {
#if defined(DOTBLUE_WINDOWS)
    std::cout << "Hello from Windows!" << std::endl;
#elif defined(DOTBLUE_LINUX)
    std::cout << "Hello from Linux!" << std::endl;
#elif defined(DOTBLUE_FREEBSD)
    std::cout << "Hello from FreeBSD!" << std::endl;
#else
    std::cout << "Hello from Unknown OS!" << std::endl;
#endif
}


int DB_Test() {
    DotBlue::MemChunkAllocator allocator("mapped_data.bin", 1024 * 1024, 64); // 1 MB, 64-byte chunks

    void* ptr = allocator.allocate();
    if (ptr) {
        std::strcpy((char*)ptr, "Hello, mapped memory!");
        std::cout << (char*)ptr << "\n";
        allocator.free(ptr);
    }

    return 0;
}

}
