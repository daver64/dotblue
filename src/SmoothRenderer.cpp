#include "DotBlue/SmoothRenderer.h"
#include "DotBlue/GLPlatform.h"
#include "DotBlue/DotBlue.h"
#include "DotBlue/Input.h"
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#include "DotBlue/wglext.h"
extern HWND hwnd;  // Declared in GLPlatformWin32.cpp
extern HDC hdc;
extern HGLRC modernContext;
#elif defined(__linux__) || defined(__FreeBSD__)
#include <GL/glx.h>
#include <X11/Xlib.h>
extern Display* display;  // Declared in GLPlatformUnix.cpp
extern Window win;
extern GLXContext modernCtx;
#endif

namespace DotBlue
{
    // Global smooth renderer instance
    static SmoothRenderer* g_smoothRenderer = nullptr;

    SmoothRenderer::SmoothRenderer()
        : m_running(false)
        , m_shouldStop(false)
        , m_renderThread(nullptr)
        , m_newFrameAvailable(false)
        , m_sharedContext(nullptr)
#if defined(_WIN32)
        , m_deviceContext(nullptr)
#elif defined(__linux__) || defined(__FreeBSD__)
        , m_display(nullptr)
        , m_window(0)
#endif
    {
        m_renderState.deltaTime = 1.0f / 60.0f;
        m_renderState.windowWidth = 800;
        m_renderState.windowHeight = 600;
        m_renderState.inputStateValid = false;
    }

    SmoothRenderer::~SmoothRenderer()
    {
        Stop();
        CleanupRenderContext();
    }

    bool SmoothRenderer::Initialize()
    {
        std::cout << "Initializing threaded renderer..." << std::endl;
        
        if (!CreateRenderContext())
        {
            std::cerr << "Failed to create shared OpenGL context for render thread" << std::endl;
            return false;
        }
        
        std::cout << "Threaded renderer initialized successfully" << std::endl;
        return true;
    }

    void SmoothRenderer::Start()
    {
        if (m_running.load())
        {
            std::cout << "Render thread already running" << std::endl;
            return;
        }

        std::cout << "Starting render thread..." << std::endl;
        m_running = true;
        m_shouldStop = false;
        m_newFrameAvailable = false;
        m_lastFrameTime = std::chrono::high_resolution_clock::now();
        
        m_renderThread = std::make_unique<std::thread>(&SmoothRenderer::RenderThreadMain, this);
        std::cout << "Render thread started" << std::endl;
    }

    void SmoothRenderer::Stop()
    {
        if (!m_running.load())
        {
            return;
        }

        std::cout << "Stopping render thread..." << std::endl;
        m_shouldStop = true;
        
        // Wake up the render thread
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_newFrameAvailable = true;
        }
        m_frameReady.notify_one();
        
        if (m_renderThread && m_renderThread->joinable())
        {
            m_renderThread->join();
        }
        
        m_renderThread.reset();
        m_running = false;
        std::cout << "Render thread stopped" << std::endl;
    }

    void SmoothRenderer::UpdateRenderState(float deltaTime, const InputManager& input, const InputBindings& bindings)
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        
        m_renderState.deltaTime = deltaTime;
        m_renderState.inputStateValid = true;
        
        // Copy any necessary input state here for thread safety
        // For now, we'll just pass the deltaTime and let the game callbacks handle input on main thread
        
        m_newFrameAvailable = true;
        m_frameReady.notify_one();
    }

    void SmoothRenderer::OnWindowResize(int width, int height)
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        m_renderState.windowWidth = width;
        m_renderState.windowHeight = height;
    }

    bool SmoothRenderer::CreateRenderContext()
    {
#if defined(_WIN32)
        if (!hwnd || !hdc || !modernContext)
        {
            std::cerr << "Main OpenGL context not available" << std::endl;
            return false;
        }

        m_deviceContext = hdc;

        // Create a shared context
        m_sharedContext = wglCreateContext((HDC)m_deviceContext);
        if (!m_sharedContext)
        {
            std::cerr << "Failed to create shared OpenGL context" << std::endl;
            return false;
        }

        // Share lists with the main context
        if (!wglShareLists(modernContext, (HGLRC)m_sharedContext))
        {
            std::cerr << "Failed to share OpenGL contexts" << std::endl;
            wglDeleteContext((HGLRC)m_sharedContext);
            m_sharedContext = nullptr;
            return false;
        }

        std::cout << "Created shared OpenGL context for Windows" << std::endl;
        return true;

#elif defined(__linux__) || defined(__FreeBSD__)
        if (!display || !win || !modernCtx)
        {
            std::cerr << "Main OpenGL context not available" << std::endl;
            return false;
        }

        m_display = display;
        m_window = win;

        // Create a shared context
        int screen = DefaultScreen((Display*)m_display);
        XVisualInfo* vi = glXChooseVisual((Display*)m_display, screen, nullptr);
        if (!vi)
        {
            std::cerr << "Failed to choose visual for shared context" << std::endl;
            return false;
        }

        m_sharedContext = glXCreateContext((Display*)m_display, vi, modernCtx, GL_TRUE);
        XFree(vi);

        if (!m_sharedContext)
        {
            std::cerr << "Failed to create shared OpenGL context" << std::endl;
            return false;
        }

        std::cout << "Created shared OpenGL context for Linux" << std::endl;
        return true;
#endif
    }

    void SmoothRenderer::CleanupRenderContext()
    {
        if (m_sharedContext)
        {
#if defined(_WIN32)
            wglDeleteContext((HGLRC)m_sharedContext);
#elif defined(__linux__) || defined(__FreeBSD__)
            glXDestroyContext((Display*)m_display, (GLXContext)m_sharedContext);
#endif
            m_sharedContext = nullptr;
        }
    }

    void SmoothRenderer::RenderThreadMain()
    {
        std::cout << "Render thread started executing" << std::endl;

        // Make the shared context current on this thread
#if defined(_WIN32)
        if (!wglMakeCurrent((HDC)m_deviceContext, (HGLRC)m_sharedContext))
        {
            std::cerr << "Failed to make shared context current on render thread" << std::endl;
            return;
        }
#elif defined(__linux__) || defined(__FreeBSD__)
        if (!glXMakeCurrent((Display*)m_display, m_window, (GLXContext)m_sharedContext))
        {
            std::cerr << "Failed to make shared context current on render thread" << std::endl;
            return;
        }
#endif

        std::cout << "Render thread OpenGL context activated" << std::endl;

        // Main render loop - run continuously at 60 FPS
        while (!m_shouldStop.load())
        {
            auto frameStart = std::chrono::high_resolution_clock::now();
            
            // Copy render state for this frame
            RenderState currentState;
            {
                std::lock_guard<std::mutex> lock(m_stateMutex);
                currentState = m_renderState;
            }

            // Set up viewport
            glViewport(0, 0, currentState.windowWidth, currentState.windowHeight);

            // Call game rendering (this should only do OpenGL calls, no ImGui)
            CallGameRender();

            // Swap buffers (this is thread-safe)
#if defined(_WIN32)
            SwapBuffers((HDC)m_deviceContext);
#elif defined(__linux__) || defined(__FreeBSD__)
            glXSwapBuffers((Display*)m_display, m_window);
#endif

            // Frame timing
            auto frameEnd = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(frameEnd - frameStart).count();
            
            if (elapsed < TARGET_FRAME_TIME_MS)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(TARGET_FRAME_TIME_MS - elapsed)));
            }
        }

        // Cleanup - make context not current
#if defined(_WIN32)
        wglMakeCurrent(nullptr, nullptr);
#elif defined(__linux__) || defined(__FreeBSD__)
        glXMakeCurrent((Display*)m_display, None, nullptr);
#endif

        std::cout << "Render thread finished" << std::endl;
    }

    // Global functions
    bool InitializeSmoothRenderer()
    {
        if (g_smoothRenderer)
        {
            std::cout << "Threaded renderer already initialized" << std::endl;
            return true;
        }

        g_smoothRenderer = new SmoothRenderer();
        if (!g_smoothRenderer->Initialize())
        {
            delete g_smoothRenderer;
            g_smoothRenderer = nullptr;
            return false;
        }

        return true;
    }

    void ShutdownSmoothRenderer()
    {
        if (g_smoothRenderer)
        {
            delete g_smoothRenderer;
            g_smoothRenderer = nullptr;
        }
    }

    SmoothRenderer* GetSmoothRenderer()
    {
        return g_smoothRenderer;
    }
}
