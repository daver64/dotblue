// platform_x11.cpp
#if defined(__linux__) || defined(__FreeBSD__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glxext.h> // <-- Add this for glXCreateContextAttribsARB
//#include "DotBlue/wglext.h"
#include <unistd.h>
#include <atomic>
#include <iostream>

namespace DotBlue {
void run_window(std::atomic<bool>& running) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display\n";
        return;
    }

    Window root = DefaultRootWindow(display);

    GLint attributes[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo* vi = glXChooseVisual(display, 0, attributes);
    if (!vi) {
        std::cerr << "No appropriate visual found\n";
        return;
    }

    Colormap cmap = XCreateColormap(display, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    Window win = XCreateWindow(display, root, 0, 0, 800, 600, 0,
        vi->depth, InputOutput, vi->visual,
        CWColormap | CWEventMask, &swa);

    XStoreName(display, win, "OpenGL Window");
    XMapWindow(display, win);

    // Create legacy context first
    GLXContext legacyCtx = glXCreateContext(display, vi, nullptr, GL_TRUE);
    glXMakeCurrent(display, win, legacyCtx);

    // Get glXCreateContextAttribsARB
    typedef GLXContext(*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = nullptr;
    glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
        glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

    GLXContext modernCtx = nullptr;

    if (glXCreateContextAttribsARB) {
        int fbcount = 0;
        static int visual_attribs[] = {
            GLX_X_RENDERABLE    , True,
            GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
            GLX_RENDER_TYPE     , GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
            GLX_RED_SIZE        , 8,
            GLX_GREEN_SIZE      , 8,
            GLX_BLUE_SIZE       , 8,
            GLX_ALPHA_SIZE      , 8,
            GLX_DEPTH_SIZE      , 24,
            GLX_STENCIL_SIZE    , 8,
            GLX_DOUBLEBUFFER    , True,
            None
        };
        GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
        if (fbc && fbcount > 0) {
            int context_attribs[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                GLX_CONTEXT_MINOR_VERSION_ARB, 4,
                GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                None
            };
            modernCtx = glXCreateContextAttribsARB(display, fbc[0], nullptr, True, context_attribs);
            if (modernCtx) {
                glXMakeCurrent(display, None, nullptr);
                glXDestroyContext(display, legacyCtx);
                glXMakeCurrent(display, win, modernCtx);
            }
            XFree(fbc);
        }
    }

    while (running) {
        while (XPending(display)) {
            XEvent xev;
            XNextEvent(display, &xev);
            if (xev.type == ClientMessage || xev.type == DestroyNotify) {
                running = false;
            }
        }

        glViewport(0, 0, 800, 600);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glXSwapBuffers(display, win);
        usleep(16000);
    }

    glXMakeCurrent(display, None, nullptr);
    if (modernCtx) {
        glXDestroyContext(display, modernCtx);
    } else {
        glXDestroyContext(display, legacyCtx);
    }
    XDestroyWindow(display, win);
    XCloseDisplay(display);
}
}
#endif
