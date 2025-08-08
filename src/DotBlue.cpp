#include "DotBlue/DotBlue.h"
#include "DotBlue/MemChunkAllocator.h"
#include "DotBlue/ColourConsole.h"
#include "DotBlue/GLPlatform.h"
#include "DotBlue/ThreadedRenderer.h"
#include <iostream>
#include <cstring>

namespace DotBlue {

// Game callback storage
static GameInitCallback g_gameInit = nullptr;
static GameUpdateCallback g_gameUpdate = nullptr;
static GameRenderCallback g_gameRender = nullptr;
static GameShutdownCallback g_gameShutdown = nullptr;
static GameInputCallback g_gameInput = nullptr;

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

void SetGameCallbacks(
    GameInitCallback initCallback,
    GameUpdateCallback updateCallback, 
    GameRenderCallback renderCallback,
    GameShutdownCallback shutdownCallback,
    GameInputCallback inputCallback)
{
    g_gameInit = initCallback;
    g_gameUpdate = updateCallback;
    g_gameRender = renderCallback;
    g_gameShutdown = shutdownCallback;
    g_gameInput = inputCallback;
}

int RunGame(std::atomic<bool>& running)
{
    // This will create window, initialize OpenGL, call InitApp, etc.
    // The game callbacks will be called from within the main loop
    DotBlue::RunWindow(running);
    return 0;
}

int RunGameThreaded(std::atomic<bool>& running)
{
    // This will create window with threaded renderer
    DotBlue::RunWindowThreaded(running);
    return 0;
}

// Callback accessor functions (for internal use)
bool CallGameInit()
{
    if (g_gameInit) {
        return g_gameInit();
    }
    return true;
}

void CallGameUpdate(float deltaTime)
{
    if (g_gameUpdate) {
        g_gameUpdate(deltaTime);
    }
}

void CallGameRender()
{
    if (g_gameRender) {
        g_gameRender();
    }
}

void CallGameShutdown()
{
    if (g_gameShutdown) {
        g_gameShutdown();
    }
}

void CallGameInput(const InputManager& input, const InputBindings& bindings)
{
    if (g_gameInput) {
        g_gameInput(input, bindings);
    }
}



}
