#pragma once
#include <DotBlue/DotBlue.h>
#include <DotBlue/GLPlatform.h>
#include <DotBlue/Input.h>
#include <atomic>
#include <memory>

class GameBase
{
public:
    GameBase();
    virtual ~GameBase();
    
    // Main game loop
    int Run();
    
    // Override these in your game
    virtual bool Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Shutdown() = 0;
    
    // Input handling
    virtual void HandleInput(const InputManager& input, const InputBindings& bindings) = 0;
    
    // Game state
    void Stop() { m_running = false; }
    bool IsRunning() const { return m_running; }
    
protected:
    std::atomic<bool> m_running;
    float m_deltaTime;
    
    // Access to DotBlue systems
    InputManager* m_inputManager;
    InputBindings* m_inputBindings;
};

// Convenience macro for main function
#define DOTBLUE_GAME_MAIN(GameClass) \
int main() \
{ \
    GameClass game; \
    return game.Run(); \
}
