// platform_win32.cpp
#ifdef _WIN32
#include <DotBlue/DotBlue.h>
#include <DotBlue/GLPlatform.h>
#include <DotBlue/SmoothRenderer.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <DotBlue/wglext.h>
#include <atomic>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

#undef UNICODE
#undef _UNICODE
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_win32.h"
#include <thread>
#include <atomic>
#pragma comment(lib, "opengl32.lib")
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// External declarations for global OpenGL context
extern HWND hwnd;
extern HDC hdc;
extern HGLRC modernContext;

// Thread-based backup renderer for window moves
std::atomic<bool> g_useThreadBackup{false};
std::thread g_backupRenderThread;
std::atomic<bool> g_backupThreadRunning{false};

void BackupRenderLoop();

// Global timer callback wrapper
void CALLBACK GlobalTimerRenderProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

// Global flag to track if we're in a size/move operation
static bool g_inSizeMove = false;
static UINT_PTR g_moveTimer = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true; // ImGui handled the message

    switch (uMsg)
    {
    case WM_ENTERSIZEMOVE:
        g_inSizeMove = true;
        std::cout << "Entering size/move mode - starting thread backup" << std::endl;
        // Start backup render thread
        if (!g_backupThreadRunning.load()) {
            g_useThreadBackup = true;
            g_backupRenderThread = std::thread(BackupRenderLoop);
        }
        return 0;
    case WM_EXITSIZEMOVE:
        g_inSizeMove = false;
        std::cout << "Exiting size/move mode - stopping thread backup" << std::endl;
        // Stop backup render thread
        if (g_backupThreadRunning.load()) {
            g_useThreadBackup = false;
            if (g_backupRenderThread.joinable()) {
                g_backupRenderThread.join();
            }
        }
        return 0;
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Global timer callback wrapper
void CALLBACK GlobalTimerRenderProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) { // Print every 60 frames (once per second at 60 FPS)
        std::cout << "Timer backup rendering active (frame " << frameCount << ")" << std::endl;
    }
    
    // Make the context current
    wglMakeCurrent(hdc, modernContext);
    
    // Calculate delta time
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    // Update input system
    DotBlue::UpdateInput();
    DotBlue::InputManager& input = DotBlue::GetInputManager();
    DotBlue::InputBindings& bindings = DotBlue::GetInputBindings();

    // Call game input and update
    DotBlue::CallGameInput(input, bindings);
    DotBlue::CallGameUpdate(deltaTime);

    // Set up viewport
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    glViewport(0, 0, width, height);

    // Default clear color
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Call game rendering
    DotBlue::CallGameRender();

    // ImGui rendering
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize.x = static_cast<float>(width);
    io.DisplaySize.y = static_cast<float>(height);

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // Default ImGui demo
    ImGui::Begin("DotBlue Engine (Timer Backup)");
    ImGui::Text("Welcome to DotBlue!");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
               1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Text("Window Move Mode Active");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    SwapBuffers(hdc);
}

// Backup render thread that runs independently during window moves
void BackupRenderLoop()
{
    g_backupThreadRunning = true;
    
    while (g_useThreadBackup.load()) {
        if (g_inSizeMove) {
            // Create a separate OpenGL context for this thread
            HGLRC backupContext = wglCreateContext(hdc);
            if (backupContext) {
                wglMakeCurrent(hdc, backupContext);
                
                // Calculate delta time
                static auto lastTime = std::chrono::high_resolution_clock::now();
                auto currentTime = std::chrono::high_resolution_clock::now();
                float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
                lastTime = currentTime;

                // Update input system
                DotBlue::UpdateInput();
                DotBlue::InputManager& input = DotBlue::GetInputManager();
                DotBlue::InputBindings& bindings = DotBlue::GetInputBindings();

                // Call game input and update
                DotBlue::CallGameInput(input, bindings);
                DotBlue::CallGameUpdate(deltaTime);

                // Set up viewport
                RECT rect;
                GetClientRect(hwnd, &rect);
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;
                glViewport(0, 0, width, height);

                // Default clear color
                glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                // Call game rendering
                DotBlue::CallGameRender();

                // ImGui rendering
                ImGuiIO &io = ImGui::GetIO();
                io.DisplaySize.x = static_cast<float>(width);
                io.DisplaySize.y = static_cast<float>(height);

                ImGui_ImplWin32_NewFrame();
                ImGui_ImplOpenGL3_NewFrame();
                ImGui::NewFrame();

                // Default ImGui demo
                ImGui::Begin("DotBlue Engine (Thread Backup)");
                ImGui::Text("Welcome to DotBlue!");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                           1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::Text("Thread Backup Mode Active");
                ImGui::End();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                // Swap buffers
                SwapBuffers(hdc);
                
                wglMakeCurrent(nullptr, nullptr);
                wglDeleteContext(backupContext);
            }
        }
        
        // Target 60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    g_backupThreadRunning = false;
}

HWND hwnd;
HDC hdc;
HGLRC modernContext;
HGLRC tempContext;
namespace DotBlue
{
    
    HDC glapp_hdc;
    void GLSwapBuffers()
    {
        SwapBuffers(glapp_hdc);
    }
    void GLSleep(int ms)
    {
        Sleep(ms);
    }
    void RunWindow(std::atomic<bool> &running)
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = "GLWindowClass";

        RegisterClass(&wc);

        hwnd = CreateWindowEx(
            0, "GLWindowClass", "OpenGL Window",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            nullptr, nullptr, wc.hInstance, nullptr);

        HDC hdc = GetDC(hwnd);
        glapp_hdc = hdc;
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;

        int format = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, format, &pfd);

        // Create legacy context first
        HGLRC tempContext = wglCreateContext(hdc);
        wglMakeCurrent(hdc, tempContext);

        typedef HGLRC(WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

        HGLRC modernContext = nullptr;

        if (wglCreateContextAttribsARB)
        {
            // Try to create the highest available context (4.4 down to 3.3)
            int majorVersions[] = {4, 4, 4, 4, 3, 3};
            int minorVersions[] = {4, 3, 2, 1, 3, 0};
            for (int i = 0; i < 6; ++i)
            {
                const int contextAttribs[] = {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, majorVersions[i],
                    WGL_CONTEXT_MINOR_VERSION_ARB, minorVersions[i],
                    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                    0};
                modernContext = wglCreateContextAttribsARB(hdc, 0, contextAttribs);
                if (modernContext)
                {
                    wglMakeCurrent(nullptr, nullptr);
                    wglDeleteContext(tempContext);
                    wglMakeCurrent(hdc, modernContext);
                    std::cout << "Created OpenGL context version " << majorVersions[i] << "." << minorVersions[i] << std::endl;
                    break;
                }
            }
        }

        if (glewInit() != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW!" << std::endl;
            exit(1);
        }
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hwnd); // hwnd is your window handle
        ImGui_ImplOpenGL3_Init("#version 400");
        DotBlue::InitApp();

        // Manual frame timing setup
        const double targetFrameTime = 1000.0 / 60.0; // 60 FPS target (ms)

        while (running)
        {
            auto frameStart = std::chrono::high_resolution_clock::now();

            MSG msg;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    running = false;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            DotBlue::HandleInput();
            DotBlue::UpdateAndRender();

            auto frameEnd = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
            if (elapsed < targetFrameTime)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(targetFrameTime - elapsed)));
            }
        }
        DotBlue::ShutdownApp();
        wglMakeCurrent(nullptr, nullptr);
        if (modernContext)
        {
            wglDeleteContext(modernContext);
        }
        else
        {
            wglDeleteContext(tempContext);
        }
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
    }

    void SetApplicationTitle(const std::string &title)
    {
        HWND hwnd = GetActiveWindow();
        if (hwnd)
        {
            SetWindowTextA(hwnd, title.c_str());
        }
    }

    // Timer-based rendering to prevent window move freezing
    void CALLBACK TimerRenderProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
    {
        // Make the context current
        wglMakeCurrent(hdc, modernContext);
        
        // Calculate delta time
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Update input system
        UpdateInput();
        InputManager& input = GetInputManager();
        InputBindings& bindings = GetInputBindings();

        // Call game input and update
        DotBlue::CallGameInput(input, bindings);
        DotBlue::CallGameUpdate(deltaTime);

        // Set up viewport
        RECT rect;
        GetClientRect(hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        glViewport(0, 0, width, height);

        // Default clear color
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Call game rendering
        DotBlue::CallGameRender();

        // ImGui rendering
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize.x = static_cast<float>(width);
        io.DisplaySize.y = static_cast<float>(height);

        ImGui_ImplWin32_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Default ImGui demo
        ImGui::Begin("DotBlue Engine (Timer-based)");
        ImGui::Text("Welcome to DotBlue!");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                   1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        SwapBuffers(hdc);
    }

    void RunWindowSmooth(std::atomic<bool> &running)
    {
        // Create window (copied from existing RunWindow function)
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = "GLWindowClassThreaded";

        RegisterClass(&wc);

        hwnd = CreateWindowEx(
            0, "GLWindowClassThreaded", "DotBlue Engine (Timer-based)",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            nullptr, nullptr, wc.hInstance, nullptr);

        hdc = GetDC(hwnd);
        glapp_hdc = hdc;

        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;

        int pixelFormat = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, pixelFormat, &pfd);

        // Create OpenGL context
        tempContext = wglCreateContext(hdc);
        wglMakeCurrent(hdc, tempContext);

        // Try to create a modern context
        modernContext = nullptr;
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

        if (wglCreateContextAttribsARB)
        {
            const int contextAttribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 4,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                0
            };
            modernContext = wglCreateContextAttribsARB(hdc, 0, contextAttribs);
            if (modernContext)
            {
                wglMakeCurrent(nullptr, nullptr);
                wglDeleteContext(tempContext);
                wglMakeCurrent(hdc, modernContext);
                std::cout << "Created modern OpenGL 4.4 context" << std::endl;
            }
        }

        if (!modernContext)
        {
            modernContext = tempContext;
            std::cout << "Using legacy OpenGL context" << std::endl;
        }

        if (glewInit() != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW!" << std::endl;
            exit(1);
        }

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplOpenGL3_Init("#version 400");
        DotBlue::InitApp();

        std::cout << "Starting high-precision rendering loop..." << std::endl;

        // High-precision timing setup
        LARGE_INTEGER frequency, lastTime, currentTime;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastTime);
        
        const double targetFPS = 60.0;
        const double frameTime = 1.0 / targetFPS;
        double accumulator = 0.0;

        // Main game loop with high-precision timing
        while (running)
        {
            // Process Windows messages (non-blocking)
            MSG msg;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            
            if (!running) break;

            // High-precision timing
            QueryPerformanceCounter(&currentTime);
            double deltaTime = (double)(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
            lastTime = currentTime;
            
            accumulator += deltaTime;
            
            // Render at target framerate (but skip if in size/move - timer handles it)
            if (accumulator >= frameTime && !g_inSizeMove)
            {
                // Make the context current and render frame
                wglMakeCurrent(hdc, modernContext);
                
                // Calculate delta time
                static auto lastFrameTime = std::chrono::high_resolution_clock::now();
                auto currentFrameTime = std::chrono::high_resolution_clock::now();
                float gameDeltaTime = std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();
                lastFrameTime = currentFrameTime;

                // Update input system
                UpdateInput();
                InputManager& input = GetInputManager();
                InputBindings& bindings = GetInputBindings();

                // Call game input and update
                DotBlue::CallGameInput(input, bindings);
                DotBlue::CallGameUpdate(gameDeltaTime);

                // Set up viewport
                RECT rect;
                GetClientRect(hwnd, &rect);
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;
                glViewport(0, 0, width, height);

                // Default clear color
                glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                // Call game rendering
                DotBlue::CallGameRender();

                // ImGui rendering
                ImGuiIO &io = ImGui::GetIO();
                io.DisplaySize.x = static_cast<float>(width);
                io.DisplaySize.y = static_cast<float>(height);

                ImGui_ImplWin32_NewFrame();
                ImGui_ImplOpenGL3_NewFrame();
                ImGui::NewFrame();

                // Default ImGui demo with FPS display
                ImGui::Begin("DotBlue Engine (High-Precision)");
                ImGui::Text("Welcome to DotBlue!");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                           1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::Text("Target: %.1f FPS", targetFPS);
                ImGui::End();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                // Swap buffers
                SwapBuffers(hdc);
                
                accumulator = 0.0;
            }
            else
            {
                // Sleep for a very short time to prevent excessive CPU usage
                Sleep(1);
            }
        }

        std::cout << "Stopping high-precision renderer..." << std::endl;

        DotBlue::ShutdownApp();
        wglMakeCurrent(nullptr, nullptr);
        if (modernContext)
        {
            wglDeleteContext(modernContext);
        }
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
    }

}
#endif
