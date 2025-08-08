#pragma once
#include <DotBlue/DotBlue.h>
#include <atomic>

// Forward declarations
namespace DotBlue {
    class InputManager;
    class InputBindings;
}

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
    
    // Input handling - using the callback signature
    virtual void HandleInput(const DotBlue::InputManager& input, const DotBlue::InputBindings& bindings) = 0;
    
    // Game state
    void Stop() { m_running = false; }
    bool IsRunning() const { return m_running; }
    
protected:
    std::atomic<bool> m_running;
    float m_deltaTime;
};

// Convenience macro for main function
#define DOTBLUE_GAME_MAIN(GameClass) \
int main() \
{ \
    GameClass game; \
    return game.Run(); \
}
