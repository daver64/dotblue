// platform_x11.cpp
#if defined(__linux__) || defined(__FreeBSD__)
#include <GL/glew.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glxext.h>
#include <unistd.h>
#include <sys/select.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <DotBlue/GLPlatform.h>
#include <DotBlue/Input.h>

// Global X11 variables (accessible by ThreadedRenderer.cpp)
Display *display = nullptr;
Window win = 0;
GLXContext modernCtx = nullptr;

// X11 event callback for client applications (like ImGui)
static DotBlue::X11EventCallback g_x11EventCallback = nullptr;

namespace DotBlue
{
    void SetX11EventCallback(X11EventCallback callback)
    {
        g_x11EventCallback = callback;
    }
    void GLSwapBuffers()
    {
        glXSwapBuffers(display, win);
    }
    void GLSleep(int ms)
    {
        usleep(ms * 1000);
    }

    void RunWindow(std::atomic<bool> &running)
    {
        const double targetFrameTime = 1000.0 / 60.0; // 60 FPS target (ms)
        display = XOpenDisplay(nullptr);
        if (!display)
        {
            std::cerr << "Cannot open X display\n";
            return;
        }

        int screen = DefaultScreen(display);
        Window root = DefaultRootWindow(display);

        // Try to get a modern FBConfig
        int fbcount = 0;
        static int visual_attribs[] = {
            GLX_X_RENDERABLE, True,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            GLX_DOUBLEBUFFER, True,
            None};
        GLXFBConfig *fbc = glXChooseFBConfig(display, screen, visual_attribs, &fbcount);

        XVisualInfo *vi = nullptr;
        win = 0;
        Colormap cmap;
        XSetWindowAttributes swa;

        if (fbc && fbcount > 0)
        {
            vi = glXGetVisualFromFBConfig(display, fbc[0]);
            cmap = XCreateColormap(display, root, vi->visual, AllocNone);
            swa.colormap = cmap;
            swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask
                | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
            win = XCreateWindow(display, root, 0, 0, 800, 600, 0,
                                vi->depth, InputOutput, vi->visual,
                                CWColormap | CWEventMask, &swa);
        }
        else
        {
            // Fallback to legacy visual
            GLint attributes[] = {
                GLX_RGBA,
                GLX_DEPTH_SIZE, 24,
                GLX_DOUBLEBUFFER,
                None};
            vi = glXChooseVisual(display, screen, attributes);
            if (!vi)
            {
                std::cerr << "No appropriate visual found\n";
                XCloseDisplay(display);
                return;
            }
            cmap = XCreateColormap(display, root, vi->visual, AllocNone);
            swa.colormap = cmap;
            swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask
                | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
            win = XCreateWindow(display, root, 0, 0, 800, 600, 0,
                                vi->depth, InputOutput, vi->visual,
                                CWColormap | CWEventMask, &swa);
        }

        XStoreName(display, win, "OpenGL Window");
        XMapWindow(display, win);

        // Set size hints
        XSizeHints *size_hints = XAllocSizeHints();
        size_hints->flags = PPosition | PSize | PMinSize;
        size_hints->min_width = 200;
        size_hints->min_height = 150;
        XSetNormalHints(display, win, size_hints);
        XFree(size_hints);

        // Create legacy context first
        GLXContext legacyCtx = glXCreateContext(display, vi, nullptr, GL_TRUE);
        glXMakeCurrent(display, win, legacyCtx);

        // Get glXCreateContextAttribsARB
        typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC)(Display *, GLXFBConfig, GLXContext, Bool, const int *);
        PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = nullptr;
        glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");

        modernCtx = nullptr;

        if (glXCreateContextAttribsARB && fbc && fbcount > 0)
        {
            int majorVersions[] = {4, 4, 4, 4, 3, 3};
            int minorVersions[] = {4, 3, 2, 1, 3, 0};
            for (int i = 0; i < 6; ++i)
            {
                int context_attribs[] = {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, majorVersions[i],
                    GLX_CONTEXT_MINOR_VERSION_ARB, minorVersions[i],
                    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, // <--- Use COMPATIBILITY
                    None};
                modernCtx = glXCreateContextAttribsARB(display, fbc[0], nullptr, True, context_attribs);
                if (modernCtx)
                {
                    glXMakeCurrent(display, None, nullptr);
                    glXDestroyContext(display, legacyCtx);
                    glXMakeCurrent(display, win, modernCtx);
                    std::cout << "Created OpenGL context version " << majorVersions[i] << "." << minorVersions[i] << std::endl;
                    break;
                }
            }
        }
        
        // Initialize GLEW after OpenGL context is created and current
        if (glewInit() != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW!" << std::endl;
            running = false;
            return;
        }
        else
        {
            std::cerr << "GLEW initialized successfully" << std::endl;
        }
        
        DotBlue::InitApp();
        // Main loop
        while (running)
        {
            auto frameStart = std::chrono::high_resolution_clock::now();

            while (XPending(display))
            {
                XEvent xev;
                XNextEvent(display, &xev);
                
                // Forward event to client application (for ImGui, etc.)
                if (g_x11EventCallback)
                {
                    g_x11EventCallback(&xev);
                }
                
                if (xev.type == ClientMessage || xev.type == DestroyNotify)
                {
                    running = false;
                }
                // Basic event handling - client applications can extend this
            }
            //DotBlue::HandleInput(win);
            DotBlue::UpdateAndRender();

            auto frameEnd = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
            if (elapsed < targetFrameTime)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(targetFrameTime - elapsed)));
            }
        }
        DotBlue::ShutdownApp();
        glXMakeCurrent(display, None, nullptr);
        if (modernCtx)
        {
            glXDestroyContext(display, modernCtx);
        }
        else
        {
            glXDestroyContext(display, legacyCtx);
        }
        XDestroyWindow(display, win);
        XFreeColormap(display, cmap);
        XFree(vi);
        if (fbc)
            XFree(fbc);
        XCloseDisplay(display);
    }

    void SetApplicationTitle(const std::string &title)
    {
        if (display && win)
        {
            XStoreName(display, win, title.c_str());
            XFlush(display);
        }
    }

    // Render function for Linux timer-based approach
    void PerformRender()
    {
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
        XWindowAttributes gwa;
        XGetWindowAttributes(display, win, &gwa);
        int width = gwa.width;
        int height = gwa.height;
        glViewport(0, 0, width, height);

        // Call game rendering
        DotBlue::CallGameRender();

        // Swap buffers
        glXSwapBuffers(display, win);
    }

    void RunWindowSmooth(std::atomic<bool> &running)
    {
        const double targetFrameTime = 1000.0 / 60.0; // 60 FPS target (ms)
        display = XOpenDisplay(nullptr);
        if (!display)
        {
            std::cerr << "Cannot open X display\n";
            return;
        }

        int screen = DefaultScreen(display);
        Window root = DefaultRootWindow(display);

        // Create window (similar to existing implementation)
        int fbcount = 0;
        static int visual_attribs[] = {
            GLX_X_RENDERABLE, True,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            GLX_DOUBLEBUFFER, True,
            None
        };

        GLXFBConfig *fbc = glXChooseFBConfig(display, screen, visual_attribs, &fbcount);
        if (!fbc)
        {
            std::cerr << "Failed to retrieve a framebuffer config\n";
            XCloseDisplay(display);
            return;
        }

        XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[0]);
        Colormap cmap = XCreateColormap(display, root, vi->visual, AllocNone);

        XSetWindowAttributes swa;
        swa.colormap = cmap;
        swa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

        win = XCreateWindow(display, root, 0, 0, 800, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
        XMapWindow(display, win);
        XStoreName(display, win, "DotBlue Engine (Timer-based Linux)");

        // Set up delete window protocol
        Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display, win, &wmDeleteMessage, 1);

        // Create OpenGL context
        GLXContext legacyCtx = glXCreateContext(display, vi, nullptr, GL_TRUE);
        glXMakeCurrent(display, win, legacyCtx);

        // Try to create modern context
        PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = 
            (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");

        modernCtx = nullptr;
        if (glXCreateContextAttribsARB)
        {
            int majorVersions[] = {4, 3, 3};
            int minorVersions[] = {4, 3, 0};
            
            for (int i = 0; i < 3; i++)
            {
                int context_attribs[] = {
                    GLX_CONTEXT_MAJOR_VERSION_ARB, majorVersions[i],
                    GLX_CONTEXT_MINOR_VERSION_ARB, minorVersions[i],
                    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                    None
                };
                modernCtx = glXCreateContextAttribsARB(display, fbc[0], nullptr, True, context_attribs);
                if (modernCtx)
                {
                    glXMakeCurrent(display, None, nullptr);
                    glXDestroyContext(display, legacyCtx);
                    glXMakeCurrent(display, win, modernCtx);
                    std::cout << "Created OpenGL context version " << majorVersions[i] << "." << minorVersions[i] << std::endl;
                    break;
                }
            }
        }

        if (glewInit() != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW!" << std::endl;
            running = false;
            return;
        }
        
        DotBlue::InitApp();

        std::cout << "Starting timer-based rendering loop (Linux)..." << std::endl;

        // Get X11 connection file descriptor for select()
        int x11_fd = ConnectionNumber(display);
        
        // Main loop with timeout-based rendering
        while (running)
        {
            auto frameStart = std::chrono::high_resolution_clock::now();

            // Check for X11 events with timeout
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(x11_fd, &fds);
            
            // Set timeout with some variation for more natural timing
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 20000; // 20ms = ~50 FPS base, allowing for natural variation
            
            int result = select(x11_fd + 1, &fds, nullptr, nullptr, &timeout);
            
            if (result > 0 && FD_ISSET(x11_fd, &fds))
            {
                // Process all available X11 events
                while (XPending(display))
                {
                    XEvent xev;
                    XNextEvent(display, &xev);
                    
                    // Forward event to client application (for ImGui, etc.)
                    if (g_x11EventCallback)
                    {
                        g_x11EventCallback(&xev);
                    }
                    
                    if (xev.type == ClientMessage || xev.type == DestroyNotify)
                    {
                        running = false;
                        break;
                    }
                    
                    // Basic event handling - client applications can extend this
                }
            }
            
            if (!running)
                break;

            // Always render, regardless of whether events occurred
            PerformRender();

            // Frame timing - allow natural variation like Windows
            auto frameEnd = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
            
            // Only sleep if we're running significantly faster than target (give some headroom)
            double minFrameTime = targetFrameTime * 0.8; // Allow some natural variation
            if (elapsed < minFrameTime)
            {
                // Use a shorter sleep to allow more natural frame rate fluctuation
                int sleepTime = (int)((minFrameTime - elapsed) * 0.7); // Less aggressive sleep
                if (sleepTime > 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
                }
            }
        }

        std::cout << "Stopping timer-based renderer (Linux)..." << std::endl;

        DotBlue::ShutdownApp();
        glXMakeCurrent(display, None, nullptr);
        if (modernCtx)
        {
            glXDestroyContext(display, modernCtx);
        }
        else
        {
            glXDestroyContext(display, legacyCtx);
        }
        XDestroyWindow(display, win);
        XFreeColormap(display, cmap);
        XFree(vi);
        if (fbc)
            XFree(fbc);
        XCloseDisplay(display);
    }

}
#endif
