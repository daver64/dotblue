#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef DOTBLUE_STATIC
#define DOTBLUE_API
#elif defined(DOTBLUE_EXPORTS)
#define DOTBLUE_API __declspec(dllexport)
#else
#define DOTBLUE_API __declspec(dllimport)
#endif
#else
#ifdef DOTBLUE_STATIC
#define DOTBLUE_API
#else
#define DOTBLUE_API __attribute__((visibility("default")))
#endif
#endif

#include "GLPlatform.h"
#include "Input.h"
#include <functional>

namespace DotBlue
{
    // Game callback types
    typedef std::function<bool()> GameInitCallback;
    typedef std::function<void(float)> GameUpdateCallback;
    typedef std::function<void()> GameRenderCallback;
    typedef std::function<void()> GameShutdownCallback;
    typedef std::function<void(const InputManager&, const InputBindings&)> GameInputCallback;

    // Game callback system
    DOTBLUE_API void SetGameCallbacks(
        GameInitCallback initCallback,
        GameUpdateCallback updateCallback, 
        GameRenderCallback renderCallback,
        GameShutdownCallback shutdownCallback,
        GameInputCallback inputCallback
    );
    
    DOTBLUE_API int RunGame(std::atomic<bool>& running);
    DOTBLUE_API int RunGameSmooth(std::atomic<bool>& running);
    
    // Test functions
    DOTBLUE_API void TestInputSystem();
}
