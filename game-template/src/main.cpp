#include "GameBase.h"
#include <iostream>

class PlatformerGame : public GameBase
{
public:
    bool Initialize() override 
    {
        std::cout << "Initializing Platformer Game..." << std::endl;
        
        // Set up input bindings
        m_inputBindings->bindKey(Action::MOVE_LEFT, SDLK_a);
        m_inputBindings->bindKey(Action::MOVE_RIGHT, SDLK_d);
        m_inputBindings->bindKey(Action::JUMP, SDLK_SPACE);
        m_inputBindings->bindKey(Action::QUIT_GAME, SDLK_ESCAPE);
        
        // Load game assets
        // m_playerTexture = DotBlue::LoadTexture("assets/player.png");
        // m_backgroundTexture = DotBlue::LoadTexture("assets/background.png");
        
        return true;
    }
    
    void Update(float deltaTime) override 
    {
        // Update game logic here
        // m_player.Update(deltaTime);
        // m_level.Update(deltaTime);
    }
    
    void Render() override 
    {
        // Render game objects here
        // DotBlue::DrawTexture(m_backgroundTexture, 0, 0);
        // m_player.Render();
        // m_level.Render();
        
        // For now, just clear to blue
        glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
    }
    
    void HandleInput(const InputManager& input, const InputBindings& bindings) override 
    {
        if (bindings.isActionPressed(Action::QUIT_GAME, input)) {
            Stop();
        }
        
        if (bindings.isActionPressed(Action::MOVE_LEFT, input)) {
            // m_player.MoveLeft();
            std::cout << "Moving left" << std::endl;
        }
        
        if (bindings.isActionPressed(Action::MOVE_RIGHT, input)) {
            // m_player.MoveRight();  
            std::cout << "Moving right" << std::endl;
        }
        
        if (bindings.isActionPressed(Action::JUMP, input)) {
            // m_player.Jump();
            std::cout << "Jumping" << std::endl;
        }
    }
    
    void Shutdown() override 
    {
        std::cout << "Shutting down Platformer Game..." << std::endl;
        // Clean up game resources
    }
    
private:
    // Game-specific members
    // Player m_player;
    // Level m_level;
    // uint32_t m_playerTexture;
    // uint32_t m_backgroundTexture;
};

// Use the convenience macro to create main function
DOTBLUE_GAME_MAIN(PlatformerGame)
