#include "GameBase.h"
#include <DotBlue/Input.h>

GameBase::GameBase() 
    : m_running(false)
    , m_deltaTime(0.0f)
{
}

GameBase::~GameBase()
{
}

int GameBase::Run()
{
    // Set up callbacks for this game instance
    DotBlue::SetGameCallbacks(
        [this]() -> bool { return this->Initialize(); },
        [this](float dt) { this->m_deltaTime = dt; this->Update(dt); },
        [this]() { this->Render(); },
        [this]() { this->Shutdown(); },
        [this](const DotBlue::InputManager& input, const DotBlue::InputBindings& bindings) { 
            this->HandleInput(input, bindings); 
        }
    );
    
    // Run the game using DotBlue's window system
    m_running = true;
    return DotBlue::RunGame(m_running);
}

int GameBase::RunSmooth()
{
    // Set up callbacks for this game instance
    DotBlue::SetGameCallbacks(
        [this]() -> bool { return this->Initialize(); },
        [this](float dt) { this->m_deltaTime = dt; this->Update(dt); },
        [this]() { this->Render(); },
        [this]() { this->Shutdown(); },
        [this](const DotBlue::InputManager& input, const DotBlue::InputBindings& bindings) { 
            this->HandleInput(input, bindings); 
        }
    );
    
    // Run the game using DotBlue's smooth window system
    m_running = true;
    return DotBlue::RunGameSmooth(m_running);
}
