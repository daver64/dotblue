#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"
#if defined(__linux__) || defined(__FreeBSD__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glxext.h>
const char *default_font_str = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
#elif defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
const char *default_font_str = "C:/Windows/Fonts/consola.ttf";
extern HDC glapp_hdc;
#endif
#include <DotBlue/GLPlatform.h>
#include <chrono>
#include <string>
#include <iostream>
#include <filesystem>
#if defined(WIN32) || defined(__CYGWIN__)
#include <SDL.h>
#include <SDL_mixer.h>
extern HWND hwnd;
#elif defined(__linux__) || defined(__FreeBSD__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#endif
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_sdl2.h"

namespace DotBlue
{

    std::string gTimingInfo;
    GLFont glapp_default_font = {};
    unsigned int texid = 0;
    DotBlue::GLTextureAtlas *glapp_texture_atlas = nullptr;
    Mix_Chunk *sound = nullptr;
    Mix_Music *music = nullptr;
    DotBlue::GLShader *shader = nullptr;
    void InitApp()
    {

        std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
        glapp_default_font = LoadFont(default_font_str);
        SetApplicationTitle("DotBlueTheBlue!!");
        texid = DotBlue::LoadPNGTexture("../bud.png");
        glapp_texture_atlas = new DotBlue::GLTextureAtlas("../mc.png", 16, 16);
        GLDisableTextureFiltering(glapp_texture_atlas->getTextureID());
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
            return;
        }
        else
        {
            std::cerr << "SDL_Init succeeded" << std::endl;
            // Initialize SDL2_mixer (stereo, 44.1kHz, 16-bit, 1024 byte chunks)
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
            {
                std::cerr << "Mix_OpenAudio failed: " << Mix_GetError() << std::endl;
                SDL_Quit();
                return;
            }
            else
            {
                std::cerr << "Mix_OpenAudio succeeded" << std::endl;
            }
            sound = Mix_LoadWAV("../stone1.wav");
            if (!sound)
            {
                std::cerr << "Mix_LoadWAV failed: " << Mix_GetError() << std::endl;
            }
            else
            {
                std::cerr << "Mix_LoadWAV succeeded" << std::endl;
                Mix_VolumeChunk(sound, MIX_MAX_VOLUME / 16);
                Mix_PlayChannel(-1, sound, 0);
            }
        }

        music = Mix_LoadMUS("../hal3.mp3");
        if (!music)
        {
            std::cerr << "Mix_LoadMUS failed: " << Mix_GetError() << std::endl;
        }
        else
        {
            std::cerr << "Mix_LoadMUS succeeded" << std::endl;
            Mix_VolumeMusic(MIX_MAX_VOLUME / 16);
            Mix_PlayMusic(music, 0); // 0 = play once, -1 = loop
        }

        shader = new DotBlue::GLShader();
        if (!shader->loadFromFiles("../shaders/passthrough.vert", "../shaders/passthrough.frag"))
        {
            std::cerr << "Failed to load shaders!" << std::endl;
        }
        else
        {
            std::cerr << "Shaders loaded successfully!" << std::endl;
        }
    }
    void ShutdownApp()
    {
        if (sound)
        {
            Mix_FreeChunk(sound);
            sound = nullptr;
        }
        Mix_CloseAudio();
        SDL_Quit();
        delete glapp_texture_atlas;
        glapp_texture_atlas = nullptr;
        if (texid)
            glDeleteTextures(1, &texid);
        texid = 0;
    }
#if defined(linux) || defined(__FreeBSD__)
    void HandleInput(SDL_Window *window)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event); 

        }
    }
#endif
    void HandleInput()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
        }
    }
    void UpdateAndRender()
    {
        //   auto start = std::chrono::high_resolution_clock::now();
        RGBA red{1.0, 0.0, 0.0, 1.0};
        RGBA green{0.0, 1.0, 0.0, 1.0};
        RGBA blue{0.0, 0.0, 1.0, 1.0};
        RGBA white{1.0, 1.0, 1.0, 1.0};
        int width = 800, height = 600; // You may want to make these dynamic

        // Set up orthographic projection for 2D text rendering
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1); // Top-left is (0,0)
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(white.r, white.g, white.b, white.a);
        TexturedQuad(texid, 100, 50, 300, 300);

        // Draw a quad from the texture atlas

        glapp_texture_atlas->select(16);
        glapp_texture_atlas->bind();
        glapp_texture_atlas->draw_quad(400, 50, 128, 128);
        // GLEnableTextureFiltering(glapp_texture_atlas->getTextureID());
        //  Now render text at pixel coordinates
        GLPrintf(glapp_default_font, 100, 100, green, "Hello DotBlue World!");

        // Each frame:
        ImGuiIO &io = ImGui::GetIO();
#if defined(_WIN32)
        RECT rect;
        GetClientRect(hwnd, &rect);
        io.DisplaySize.x = static_cast<float>(rect.right - rect.left);
        io.DisplaySize.y = static_cast<float>(rect.bottom - rect.top);

        ImGui_ImplWin32_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
#elif defined(__linux__) || defined(__FreeBSD__)
        // Set display size using your window variables (replace width/height if needed)
        io.DisplaySize.x = static_cast<float>(width);
        io.DisplaySize.y = static_cast<float>(height);

        ImGui_ImplOpenGL3_NewFrame();
        // If you use SDL2 for windowing, you may want ImGui_ImplSDL2_NewFrame(window);
#endif
        ImGui::NewFrame();

        ImGui::Begin("Hello, DotBlue!"); // No ImGuiWindowFlags_NoMove, NoResize
        ImGui::Text("Welcome to DotBlue!");
        if (ImGui::Button("Press Me"))
        {
            ImGui::Text("Button was pressed!");
        }
        static float sliderValue = 0.0f;
        ImGui::SliderFloat("Slider", &sliderValue, 0.0f, 1.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers (platform-specific)
        GLSwapBuffers();
        // auto end = std::chrono::high_resolution_clock::now();
        //  std::chrono::duration<double, std::milli> elapsed = end - start;

        // static int counter=0;
        //  counter++;
        // if (counter % 60 == 0) { // Update timing info every 60 frames
        //     gTimingInfo = "Frame time: " + std::to_string(static_cast<int>(elapsed.count())) + " ms";
        // }
    }
}