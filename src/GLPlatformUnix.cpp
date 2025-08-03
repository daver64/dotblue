// platform_x11.cpp
#if defined(__linux__) || defined(__FreeBSD__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glxext.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include <DotBlue/GLPlatform.h>
namespace DotBlue {
Display* display = nullptr;
Window win = 0;
void GLSwapBuffers()
{
    glXSwapBuffers(display, win);
}
void GLSleep(int ms)
{
    usleep(ms * 1000);
}

void RunWindow(std::atomic<bool>& running) {
    display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open X display\n";
        return;
    }

    int screen = DefaultScreen(display);
    Window root = DefaultRootWindow(display);

    // Try to get a modern FBConfig
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
    GLXFBConfig* fbc = glXChooseFBConfig(display, screen, visual_attribs, &fbcount);

    XVisualInfo* vi = nullptr;
    win = 0;
    Colormap cmap;
    XSetWindowAttributes swa;

    if (fbc && fbcount > 0) {
        vi = glXGetVisualFromFBConfig(display, fbc[0]);
        cmap = XCreateColormap(display, root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
        win = XCreateWindow(display, root, 0, 0, 800, 600, 0,
            vi->depth, InputOutput, vi->visual,
            CWColormap | CWEventMask, &swa);
    } else {
        // Fallback to legacy visual
        GLint attributes[] = {
            GLX_RGBA,
            GLX_DEPTH_SIZE, 24,
            GLX_DOUBLEBUFFER,
            None
        };
        vi = glXChooseVisual(display, screen, attributes);
        if (!vi) {
            std::cerr << "No appropriate visual found\n";
            XCloseDisplay(display);
            return;
        }
        cmap = XCreateColormap(display, root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
        win = XCreateWindow(display, root, 0, 0, 800, 600, 0,
            vi->depth, InputOutput, vi->visual,
            CWColormap | CWEventMask, &swa);
    }

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

    if (glXCreateContextAttribsARB && fbc && fbcount > 0) {
        int majorVersions[] = {4, 4, 4, 4, 3, 3};
        int minorVersions[] = {4, 3, 2, 1, 3, 0};
        for (int i = 0; i < 6; ++i) {
            int context_attribs[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, majorVersions[i],
                GLX_CONTEXT_MINOR_VERSION_ARB, minorVersions[i],
                GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, // <--- Use COMPATIBILITY
                None
            };
            modernCtx = glXCreateContextAttribsARB(display, fbc[0], nullptr, True, context_attribs);
            if (modernCtx) {
                glXMakeCurrent(display, None, nullptr);
                glXDestroyContext(display, legacyCtx);
                glXMakeCurrent(display, win, modernCtx);
                std::cout << "Created OpenGL context version " << majorVersions[i] << "." << minorVersions[i] << std::endl;
                break;
            }
        }
    }
    DotBlue::InitApp();
    // Main loop
    while (running) {
        while (XPending(display)) {
            XEvent xev;
            XNextEvent(display, &xev);
            if (xev.type == ClientMessage || xev.type == DestroyNotify) {
                running = false;
            }
        }
        DotBlue::UpdateAndRender();
    }

    glXMakeCurrent(display, None, nullptr);
    if (modernCtx) {
        glXDestroyContext(display, modernCtx);
    } else {
        glXDestroyContext(display, legacyCtx);
    }
    XDestroyWindow(display, win);
    XFreeColormap(display, cmap);
    XFree(vi);
    if (fbc) XFree(fbc);
    XCloseDisplay(display);
}
}
#endif
