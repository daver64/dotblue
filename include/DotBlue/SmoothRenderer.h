#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#ifndef DOTBLUE_API
#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef DOTBLUE_STATIC
#define DOTBLUE_API
#elif defined(DOTBLUE_EXPORTS)
#define DOTBLUE_API __declspec(dllexport)
#else
#define DOTBLUE_API __declspec(dllimport)
#endif
#else
#ifdef DOTBLUE_STATIC
#define DOTBLUE_API
#else
#define DOTBLUE_API __attribute__((visibility("default")))
#endif
#endif
#endif

namespace DotBlue
{
    class InputManager;
    class InputBindings;

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4251) // STL types in DLL interface
#endif

    class DOTBLUE_API SmoothRenderer
    {
    public:
        SmoothRenderer();
        ~SmoothRenderer();

        // Initialize the smooth renderer (must be called from main thread with active OpenGL context)
        bool Initialize();
        
        // Start the render system
        void Start();
        
        // Stop the render system and wait for it to finish
        void Stop();
        
        // Update render state from main thread (called during window events)
        void UpdateRenderState(float deltaTime, const InputManager& input, const InputBindings& bindings);
        
        // Signal that window size has changed
        void OnWindowResize(int width, int height);
        
        // Check if renderer is running
        bool IsRunning() const { return m_running.load(); }

    private:
        // Render thread main function (kept for potential future threading)
        void RenderThreadMain();
        
        // Create shared OpenGL context for render thread (kept for potential future threading)
        bool CreateRenderContext();
        
        // Cleanup render context
        void CleanupRenderContext();

        // Thread management (kept for potential future threading)
        std::atomic<bool> m_running;
        std::atomic<bool> m_shouldStop;
        std::unique_ptr<std::thread> m_renderThread;
        
        // Frame synchronization
        std::mutex m_stateMutex;
        std::condition_variable m_frameReady;
        std::atomic<bool> m_newFrameAvailable;
        
        // Render state (protected by mutex)
        struct RenderState
        {
            float deltaTime;
            int windowWidth;
            int windowHeight;
            bool inputStateValid;
            // Copy of input state for thread safety
            // We'll copy the relevant input data here
        } m_renderState;
        
        // Platform-specific context data
#if defined(_WIN32)
        void* m_sharedContext;  // HGLRC
        void* m_deviceContext;  // HDC
#elif defined(__linux__) || defined(__FreeBSD__)
        void* m_display;        // Display*
        void* m_sharedContext;  // GLXContext
        unsigned long m_window; // Window
#endif
        
        // Timing
        std::chrono::high_resolution_clock::time_point m_lastFrameTime;
        static constexpr double TARGET_FRAME_TIME_MS = 1000.0 / 60.0; // 60 FPS
    };

#ifdef _WIN32
#pragma warning(pop)
#endif

    // Global smooth renderer instance management
    DOTBLUE_API bool InitializeSmoothRenderer();
    DOTBLUE_API void ShutdownSmoothRenderer();
    DOTBLUE_API SmoothRenderer* GetSmoothRenderer();
    
    // Enhanced window run function that uses smooth renderer
    DOTBLUE_API void RunWindowSmooth(std::atomic<bool>& running);
}
