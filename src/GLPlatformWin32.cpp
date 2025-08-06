// platform_win32.cpp
#ifdef _WIN32
#include <DotBlue/DotBlue.h>
#include <DotBlue/GLPlatform.h>
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true; // ImGui handled the message

    if (uMsg == WM_CLOSE || uMsg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
HWND hwnd;
namespace DotBlue
{
    
    HDC glapp_hdc;
    void GLSwapBuffers()
    {
        SwapBuffers(glapp_hdc);
    }
    void GLSleep(int ms)
    {
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
        ImGui_ImplOpenGL3_Init("#version 130");
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

}
#endif
