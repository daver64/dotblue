#include "GameBase.h"
#include <chrono>

GameBase::GameBase() 
    : m_running(false)
    , m_deltaTime(0.0f)
    , m_inputManager(nullptr)
    , m_inputBindings(nullptr)
{
}

GameBase::~GameBase()
{
}

int GameBase::Run()
{
    // Initialize DotBlue systems
    DotBlue::InitApp();
    
    // Get input system references
    m_inputManager = &DotBlue::GetInputManager();
    m_inputBindings = &DotBlue::GetInputBindings();
    
    // Initialize game-specific logic
    if (!Initialize()) {
        return -1;
    }
    
    m_running = true;
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    // Main game loop
    while (m_running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        m_deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Handle input
        m_inputManager->update();
        HandleInput(*m_inputManager, *m_inputBindings);
        
        // Update game logic
        Update(m_deltaTime);
        
        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Render();
        DotBlue::SwapBuffers(); // You'll need to add this to DotBlue
        
        // Handle window events
        if (DotBlue::ShouldClose()) { // You'll need to add this to DotBlue
            m_running = false;
        }
    }
    
    // Cleanup
    Shutdown();
    DotBlue::ShutdownApp();
    
    return 0;
}
