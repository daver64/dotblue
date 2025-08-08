#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"
#include <chrono>

#if defined(__linux__) || defined(__FreeBSD__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glxext.h>
const char *default_font_str = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#elif defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL.h>
#include <SDL_mixer.h>
extern HWND hwnd;
const char *default_font_str = "C:/Windows/Fonts/consola.ttf";
extern HDC glapp_hdc;
#endif

#include "DotBlue/Input.h"

#include <chrono>
#include <string>
#include <iostream>
#include <filesystem>

namespace DotBlue
{

    std::string gTimingInfo;
    GLFont glapp_default_font = {};
    void InitApp()
    {
        std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

        // Initialize input system
        InitializeInput();

        // Load default font
        glapp_default_font = LoadFont(default_font_str);
        
        // Set application title
        SetApplicationTitle("DotBlue Engine");

        // Initialize SDL audio system
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        }
        else
        {
            // Initialize SDL2_mixer (stereo, 44.1kHz, 16-bit, 1024 byte chunks)
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
            {
                std::cerr << "Mix_OpenAudio failed: " << Mix_GetError() << std::endl;
                SDL_Quit();
            }
        }
        
        // Call game initialization if available
        DotBlue::CallGameInit();
    }

    void TestInputSystem()
    {
        std::cout << "DotBlue Input System Test" << std::endl;
        std::cout << "=========================" << std::endl;

        // Initialize the application

        // Get input references
        InputManager &input = GetInputManager();
        InputBindings &bindings = GetInputBindings();

        std::cout << "Input system initialized!" << std::endl;
        std::cout << "Connected controllers: " << input.getControllerCount() << std::endl;

        // Test some basic functionality
        std::cout << "\nTesting key bindings:" << std::endl;
        auto forwardKeys = bindings.getKeyBindings(Action::MOVE_FORWARD);
        std::cout << "MOVE_FORWARD bound to " << forwardKeys.size() << " keys" << std::endl;

        auto fireButtons = bindings.getMouseBindings(Action::FIRE_PRIMARY);
        std::cout << "FIRE_PRIMARY bound to " << fireButtons.size() << " mouse buttons" << std::endl;

        std::cout << "\nPress keys to test input (this is just initialization test)" << std::endl;
        std::cout << "In a real game, you would use this in your main loop" << std::endl;

        // Example of how you'd use it in a game loop:
        std::cout << "if (bindings.isActionPressed(Action::MOVE_FORWARD, input)) {" << std::endl;
        std::cout << "    movePlayerForward();" << std::endl;
        std::cout << "}" << std::endl;
    }
    void ShutdownApp()
    {
        // Shutdown input system
        ShutdownInput();

        // Clean up SDL audio
        Mix_CloseAudio();
        SDL_Quit();
    }
#if defined(linux) || defined(__FreeBSD__)
    void HandleInput(SDL_Window *window)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // Handle input for our input system
            if (g_inputManager)
            {
                g_inputManager->handleSDLEvent(event);
            }
        }
    }
#endif
    void HandleInput()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // Handle input for our input system
            if (g_inputManager)
            {
                g_inputManager->handleSDLEvent(event);
            }
        }
    }
    void UpdateAndRender()
    {
        // Calculate delta time
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Update input system
        UpdateInput();
        
        // Get input references for game callback
        InputManager& input = GetInputManager();
        InputBindings& bindings = GetInputBindings();
        
        // Call game input handling
        DotBlue::CallGameInput(input, bindings);
        
        // Call game update
        DotBlue::CallGameUpdate(deltaTime);

        int width = 800, height = 600; // You may want to make these dynamic

        // Set up viewport for modern OpenGL
        glViewport(0, 0, width, height);

        // Call game rendering
        DotBlue::CallGameRender();

        // Swap buffers (platform-specific)
        GLSwapBuffers();
    }
}