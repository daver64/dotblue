#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"
#include <iostream>
#include <cstring>

namespace DotBlue {

// Game callback storage
static GameInitCallback g_gameInit = nullptr;
static GameUpdateCallback g_gameUpdate = nullptr;
static GameRenderCallback g_gameRender = nullptr;
static GameShutdownCallback g_gameShutdown = nullptr;
static GameInputCallback g_gameInput = nullptr;

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

int RunGame(std::atomic<bool>& running)
{
    // This will create window, initialize OpenGL, call InitApp, etc.
    // The game callbacks will be called from within the main loop
    RunWindow(running);
    return 0;
}

int RunGameSmooth(std::atomic<bool>& running)
{
    // This will create window with smooth renderer
    RunWindowSmooth(running);
    return 0;
}

}
