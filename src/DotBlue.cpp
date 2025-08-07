#include "DotBlue/DotBlue.h"
#include "DotBlue/MemChunkAllocator.h"
#include "DotBlue/ColourConsole.h"
#include "DotBlue/GLPlatform.h"
#include <iostream>
#include <cstring>

namespace DotBlue {

void Hello() 
{
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


int DB_Test() 
{
    DotBlue::MemChunkAllocator allocator("mapped_data.bin", 1024 * 1024, 64); // 1 MB, 64-byte chunks

    void* ptr = allocator.allocate();
    if (ptr) {
        std::strcpy((char*)ptr, "Hello, mapped memory!");
        std::cout << (char*)ptr << "\n";
        allocator.free(ptr);
    }

    return 0;
}

int Console_Test()
{
    DotBlue::ColourConsole::resize(100, 40);
    DotBlue::ColourConsole::clear();
    DotBlue::ColourConsole::hideCursor();

    DotBlue::ColourConsole::setColour(DotBlue::ConsoleColour::BrightGreen);
    DotBlue::ColourConsole::gotoxy(10, 5);
    std::cout << "Hello in colour at (10,5)!";

    int x, y;
    ColourConsole::getxy(x, y);
    DotBlue::ColourConsole::reset();
    DotBlue::ColourConsole::gotoxy(0, y + 2);
    std::cout << "Cursor was at: (" << x << "," << y << ")" << std::endl;

    DotBlue::ColourConsole::showCursor();
    return 0;   
}

int GL_Test() 
{
    std::atomic<bool> running(true);
    DotBlue::RunWindow(running);
    return 0;   
}



}
