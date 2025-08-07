#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"

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


#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#if defined(__linux__) || defined(__FreeBSD__)
#include "backends/imgui_impl_sdl2.h"
#elif defined(_WIN32) || defined(__CYGWIN__)
#include "backends/imgui_impl_win32.h"
#endif


#include <chrono>
#include <string>
#include <iostream>
#include <filesystem>

namespace DotBlue
{

    std::string gTimingInfo;
    GLFont glapp_default_font = {};
    unsigned int texid = 0;
    DotBlue::GLTextureAtlas *glapp_texture_atlas = nullptr;
    Mix_Chunk *sound = nullptr;
    Mix_Music *music = nullptr;
    DotBlue::GLShader *shader = nullptr;
    DotBlue::GLShader *texturedShader = nullptr;
    void InitApp()
    {

        vec3 position(1.0f,2.0f,3.0f);
        mat4 transform = Math::translate(mat4(1.0f), position);
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

        texturedShader = new DotBlue::GLShader();
        if (!texturedShader->loadFromFiles("../shaders/textured.vert", "../shaders/textured.frag"))
        {
            std::cerr << "Failed to load textured shaders!" << std::endl;
        }
        else
        {
            std::cerr << "Textured shaders loaded successfully!" << std::endl;
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
        delete shader;
        shader = nullptr;
        delete texturedShader;
        texturedShader = nullptr;
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

        // Set up viewport for modern OpenGL
        glViewport(0, 0, width, height);

        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Modern textured rendering
        texturedShader->bind();
        texturedShader->setVec2("u_resolution", (float)width, (float)height);
        texturedShader->setInt("u_texture", 0);
        glActiveTexture(GL_TEXTURE0);
        TexturedQuadShader(texid, 100.0f, 50.0f, 300.0f, 300.0f);
        TexturedQuadShader(texid, 350.0f, 50.0f, 550.0f, 250.0f);
        texturedShader->unbind();

        // Modern colored shape rendering
        shader->bind();
        shader->setVec2("u_resolution", (float)width, (float)height);
        GLLineShader(100.0f, 400.0f, 300.0f, 400.0f, 1.0f, 0.0f, 0.0f); // Red line
        GLLineShader(100.0f, 450.0f, 300.0f, 450.0f, 0.0f, 1.0f, 0.0f); // Green line
        GLTriangleShader(500.0f, 400.0f, 550.0f, 350.0f, 600.0f, 400.0f, 1.0f, 1.0f, 0.0f); // Yellow triangle
        GLRectangleShader(500.0f, 450.0f, 600.0f, 500.0f, 1.0f, 0.0f, 1.0f); // Magenta rectangle
        shader->unbind();
        
        // Draw textured atlas quad using modern rendering
        texturedShader->bind();
        texturedShader->setVec2("u_resolution", (float)width, (float)height);
        texturedShader->setInt("u_texture", 0);
        glActiveTexture(GL_TEXTURE0);
        
        glapp_texture_atlas->select(16);
        // Get the UV coordinates for the selected atlas image
        float u0, v0, u1, v1;
        glapp_texture_atlas->getSelectedUVs(u0, v0, u1, v1);
        TexturedQuadShaderUV(glapp_texture_atlas->getTextureID(), 400.0f, 50.0f, 528.0f, 178.0f, u0, v0, u1, v1);
        texturedShader->unbind();
        
        // Text rendering (GLPrintf still uses fixed-function, but it's for text which is more complex)
        // Set up matrices for text rendering only
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
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