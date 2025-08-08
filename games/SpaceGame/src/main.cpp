#define SDL_MAIN_HANDLED  // Prevent SDL from redefining main
#include "GameBase.h"
#include <iostream>

class SpaceGameGame : public GameBase
{
public:
    bool Initialize() override 
    {
        std::cout << "Initializing Space Game..." << std::endl;
        return true;
    }
    
    void Update(float deltaTime) override 
    {
        // Update game logic here
    }
    
    void Render() override 
    {
        // For now, just a simple render
        std::cout << "Rendering frame..." << std::endl;
    }
    
    void HandleInput(const DotBlue::InputManager& input, const DotBlue::InputBindings& bindings) override 
    {
        // Simple input handling - we'll implement this later
        // For now just check if we should quit (this will be implemented by DotBlue)
    }
    
    void Shutdown() override 
    {
        std::cout << "Shutting down Space Game..." << std::endl;
    }
};

// Use the convenience macro to create main function
DOTBLUE_GAME_MAIN(SpaceGameGame)
