// platform_win32.cpp
#ifdef _WIN32
#include <DotBlue/DotBlue.h>
#include <DotBlue/GLPlatform.h>
#include <DotBlue/ThreadedRenderer.h>
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
#pragma comment(lib, "opengl32.lib")
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Global flag to track if we're in a size/move operation
static bool g_inSizeMove = false;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true; // ImGui handled the message

    switch (uMsg)
    {
    case WM_ENTERSIZEMOVE:
        g_inSizeMove = true;
        return 0;
    case WM_EXITSIZEMOVE:
        g_inSizeMove = false;
        return 0;
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
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

    void RunWindowThreaded(std::atomic<bool> &running)
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

        std::cout << "Starting timer-based rendering loop..." << std::endl;

        // Set up a timer for 60 FPS rendering (16.67ms intervals)
        UINT_PTR timerId = SetTimer(hwnd, 1, 16, TimerRenderProc);
        if (!timerId)
        {
            std::cerr << "Failed to create render timer!" << std::endl;
            running = false;
            return;
        }

        // Main message loop - this now only handles messages, rendering is timer-driven
        while (running)
        {
            MSG msg;
            BOOL result = GetMessage(&msg, nullptr, 0, 0);
            
            if (result == 0) // WM_QUIT
            {
                running = false;
                break;
            }
            else if (result == -1) // Error
            {
                std::cerr << "GetMessage error" << std::endl;
                break;
            }
            
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Cleanup timer
        KillTimer(hwnd, timerId);

        std::cout << "Stopping timer-based renderer..." << std::endl;

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
