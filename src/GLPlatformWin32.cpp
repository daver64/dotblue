// platform_win32.cpp
#ifdef _WIN32
#include <DotBlue/DotBlue.h>
#include <windows.h>
#include <gl/GL.h>
#include <atomic>
#include <iostream>
#undef UNICODE
#undef _UNICODE

#pragma comment(lib, "opengl32.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_CLOSE || uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

namespace DotBlue {

void run_window(std::atomic<bool>& running) 
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName =  "GLWindowClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, "GLWindowClass", "OpenGL Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, wc.hInstance, nullptr);

    HDC hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    int format = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, format, &pfd);

    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);

    // Load WGL extensions to get OpenGL 3.3 context (omitted for brevity)
    // For this minimal example, we keep legacy context

    while (running) 
    {
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

        glViewport(0, 0, 800, 600);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SwapBuffers(hdc);
        Sleep(16);
    }

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tempContext);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
}
}
#endif
