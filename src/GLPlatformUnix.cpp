// platform_x11.cpp
#if defined(__linux__) || defined(__FreeBSD__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
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

    GLXContext glc = glXCreateContext(display, vi, nullptr, GL_TRUE);
    glXMakeCurrent(display, win, glc);

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
    glXDestroyContext(display, glc);
    XDestroyWindow(display, win);
    XCloseDisplay(display);
}
}

#endif
