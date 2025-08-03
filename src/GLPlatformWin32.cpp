// platform_win32.cpp
#ifdef _WIN32
#include <DotBlue/DotBlue.h>
#include <windows.h>
#include <gl/GL.h>
#include "DotBlue/wglext.h"
#include <atomic>
#include <iostream>
#include <vector>
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

// WGL extension function pointers
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
typedef BOOL(WINAPI* PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int*, const FLOAT*, UINT, int*, UINT*);

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

    // Create legacy context first
    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);

    // Load WGL extension functions
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
        (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

    HGLRC modernContext = nullptr;
    int pixelFormat = format;

    if (wglCreateContextAttribsARB && wglChoosePixelFormatARB) {
        // Specify attributes for modern pixel format
        const int pixelAttribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 24,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0
        };
        UINT numFormats;
        int formats[1];
        if (wglChoosePixelFormatARB(hdc, pixelAttribs, nullptr, 1, formats, &numFormats) && numFormats > 0) {
            pixelFormat = formats[0];
            SetPixelFormat(hdc, pixelFormat, &pfd);
        }

        // Specify context attributes for OpenGL 4.4
        const int contextAttribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 4,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        modernContext = wglCreateContextAttribsARB(hdc, 0, contextAttribs);
        if (modernContext) {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(tempContext);
            wglMakeCurrent(hdc, modernContext);
        }
    }

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
    if (modernContext) {
        wglDeleteContext(modernContext);
    } else {
        wglDeleteContext(tempContext);
    }
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
}
}
#endif
