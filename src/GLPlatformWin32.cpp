// platform_win32.cpp
#ifdef _WIN32
#include <DotBlue/DotBlue.h>
#include <DotBlue/GLPlatform.h>
#include <windows.h>
#include <gl/GL.h>
#include <DotBlue/wglext.h>
#include <atomic>
#include <iostream>
#include <vector>
#include <string>
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
HDC glapp_hdc;
void GLSwapBuffers()
{
    SwapBuffers(glapp_hdc);
}
void GLSleep(int ms)
{

}
void RunWindow(std::atomic<bool>& running) 
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

    // Load WGL extension functions
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
        (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

    HGLRC modernContext = nullptr;

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
            // Only set pixel format if it hasn't been set yet
            // SetPixelFormat(hdc, formats[0], &pfd); // REMOVE this line!
        }

        // Try to create the highest available context (4.4 down to 3.3)
        int majorVersions[] = {4, 4, 4, 4, 3, 3};
        int minorVersions[] = {4, 3, 2, 1, 3, 0};
        for (int i = 0; i < 6; ++i) {
            const int contextAttribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 4,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, // <--- Use COMPATIBILITY
                0
            };
            modernContext = wglCreateContextAttribsARB(hdc, 0, contextAttribs);
            if (modernContext) {
                wglMakeCurrent(nullptr, nullptr);
                wglDeleteContext(tempContext);
                wglMakeCurrent(hdc, modernContext);
                std::cout << "Created OpenGL context version " << majorVersions[i] << "." << minorVersions[i] << std::endl;
                break;
            }
        }
    }

    InitApp();
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

        DotBlue::UpdateAndRender();
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

void SetApplicationTitle(const std::string& title) {
    HWND hwnd = GetActiveWindow();
    if (hwnd) {
        SetWindowTextA(hwnd, title.c_str());
    }
}

}
#endif
