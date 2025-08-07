// Example usage of DotBlue Input System
#include "DotBlue/DotBlue.h"

void ExampleInputUsage() {
    using namespace DotBlue;
    
    // Get reference to input systems
    InputManager& input = GetInputManager();
    InputBindings& bindings = GetInputBindings();
    
    // Example 1: Direct input queries
    if (input.isKeyPressed(SDL_SCANCODE_W)) {
        std::cout << "W key is held down" << std::endl;
    }
    
    if (input.isKeyJustPressed(SDL_SCANCODE_SPACE)) {
        std::cout << "Space was just pressed!" << std::endl;
    }
    
    // Example 2: Mouse input
    Vec2 mousePos = input.getMousePosition();
    Vec2 mouseDelta = input.getMouseDelta();
    
    if (input.isMouseButtonPressed(SDL_BUTTON_LEFT)) {
        std::cout << "Left mouse button held at: " << mousePos.x << ", " << mousePos.y << std::endl;
    }
    
    // Example 3: Controller input
    if (input.getControllerCount() > 0) {
        Vec2 leftStick = input.getControllerLeftStick(0);
        Vec2 rightStick = input.getControllerRightStick(0);
        
        if (leftStick.x != 0.0f || leftStick.y != 0.0f) {
            std::cout << "Left stick: " << leftStick.x << ", " << leftStick.y << std::endl;
        }
        
        if (input.isControllerButtonJustPressed(0, SDL_CONTROLLER_BUTTON_A)) {
            std::cout << "Controller A button pressed!" << std::endl;
        }
    }
    
    // Example 4: Action-based input (recommended)
    if (bindings.isActionPressed(Action::MOVE_FORWARD, input)) {
        std::cout << "Moving forward!" << std::endl;
    }
    
    if (bindings.isActionJustPressed(Action::FIRE_PRIMARY, input)) {
        std::cout << "Firing primary weapon!" << std::endl;
    }
    
    // Example 5: Movement vector from multiple actions
    Vec2 movement(0.0f, 0.0f);
    if (bindings.isActionPressed(Action::MOVE_RIGHT, input)) movement.x += 1.0f;
    if (bindings.isActionPressed(Action::MOVE_LEFT, input)) movement.x -= 1.0f;
    if (bindings.isActionPressed(Action::MOVE_FORWARD, input)) movement.y += 1.0f;
    if (bindings.isActionPressed(Action::MOVE_BACKWARD, input)) movement.y -= 1.0f;
    
    // Normalize diagonal movement
    if (movement.x != 0.0f && movement.y != 0.0f) {
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        movement.x /= length;
        movement.y /= length;
    }
    
    // Use movement vector for player/camera movement
    if (movement.x != 0.0f || movement.y != 0.0f) {
        std::cout << "Movement vector: " << movement.x << ", " << movement.y << std::endl;
    }
}

// Example for a simple game loop with input
void ExampleGameLoop() {
    using namespace DotBlue;
    
    // Game state
    Vec3 playerPosition(0.0f, 0.0f, 0.0f);
    Vec3 cameraRotation(0.0f, 0.0f, 0.0f);
    bool gameRunning = true;
    
    InputManager& input = GetInputManager();
    InputBindings& bindings = GetInputBindings();
    
    while (gameRunning) {
        // Handle SDL events (this would be in your main loop)
        HandleInput();
        
        // Update input system (this would be in UpdateAndRender)
        UpdateInput();
        
        // Check for quit
        if (bindings.isActionJustPressed(Action::PAUSE, input) || 
            input.isKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
            gameRunning = false;
        }
        
        // Player movement
        Vec3 movement(0.0f, 0.0f, 0.0f);
        float speed = 5.0f; // units per second
        
        if (bindings.isActionPressed(Action::MOVE_FORWARD, input)) movement.z += speed;
        if (bindings.isActionPressed(Action::MOVE_BACKWARD, input)) movement.z -= speed;
        if (bindings.isActionPressed(Action::MOVE_LEFT, input)) movement.x -= speed;
        if (bindings.isActionPressed(Action::MOVE_RIGHT, input)) movement.x += speed;
        if (bindings.isActionPressed(Action::MOVE_UP, input)) movement.y += speed;
        if (bindings.isActionPressed(Action::MOVE_DOWN, input)) movement.y -= speed;
        
        // Apply movement (you'd multiply by deltaTime in a real game)
        playerPosition += movement * 0.016f; // Assuming 60 FPS
        
        // Camera rotation from mouse
        Vec2 mouseDelta = input.getMouseDelta();
        float sensitivity = 0.001f;
        cameraRotation.x += mouseDelta.y * sensitivity; // Pitch
        cameraRotation.y += mouseDelta.x * sensitivity; // Yaw
        
        // Or from controller
        if (input.getControllerCount() > 0) {
            Vec2 rightStick = input.getControllerRightStick(0);
            cameraRotation.x += rightStick.y * sensitivity * 10.0f;
            cameraRotation.y += rightStick.x * sensitivity * 10.0f;
        }
        
        // Combat
        if (bindings.isActionJustPressed(Action::FIRE_PRIMARY, input)) {
            std::cout << "Firing primary weapon!" << std::endl;
            // FireWeapon();
        }
        
        if (bindings.isActionJustPressed(Action::FIRE_SECONDARY, input)) {
            std::cout << "Firing secondary weapon!" << std::endl;
            // FireSecondaryWeapon();
        }
        
        // Other game logic...
        
        // Render
        UpdateAndRender();
    }
}

// Example custom bindings setup
void SetupCustomBindings() {
    using namespace DotBlue;
    
    InputBindings& bindings = GetInputBindings();
    
    // Clear existing bindings for an action
    bindings.clearBindings(Action::MOVE_FORWARD);
    
    // Add custom bindings
    bindings.bindKey(Action::MOVE_FORWARD, SDL_SCANCODE_UP);
    bindings.bindKey(Action::MOVE_FORWARD, SDL_SCANCODE_W);
    bindings.bindButton(Action::MOVE_FORWARD, SDL_CONTROLLER_BUTTON_DPAD_UP);
    
    // Set up WASD + arrow keys for movement
    bindings.bindKey(Action::MOVE_LEFT, SDL_SCANCODE_A);
    bindings.bindKey(Action::MOVE_LEFT, SDL_SCANCODE_LEFT);
    bindings.bindKey(Action::MOVE_RIGHT, SDL_SCANCODE_D);
    bindings.bindKey(Action::MOVE_RIGHT, SDL_SCANCODE_RIGHT);
    bindings.bindKey(Action::MOVE_BACKWARD, SDL_SCANCODE_S);
    bindings.bindKey(Action::MOVE_BACKWARD, SDL_SCANCODE_DOWN);
    
    // Combat bindings
    bindings.bindMouseButton(Action::FIRE_PRIMARY, SDL_BUTTON_LEFT);
    bindings.bindMouseButton(Action::FIRE_SECONDARY, SDL_BUTTON_RIGHT);
    bindings.bindKey(Action::RELOAD, SDL_SCANCODE_R);
    
    std::cout << "Custom input bindings configured!" << std::endl;
}

// Example for different input contexts (like switching between gameplay and menu)
enum class InputContext {
    GAMEPLAY,
    MENU,
    PAUSED
};

void HandleContextualInput(InputContext context) {
    using namespace DotBlue;
    
    InputManager& input = GetInputManager();
    InputBindings& bindings = GetInputBindings();
    
    switch (context) {
        case InputContext::GAMEPLAY:
            // Handle gameplay input (movement, combat, etc.)
            ExampleGameLoop();
            break;
            
        case InputContext::MENU:
            // Handle menu navigation
            if (bindings.isActionJustPressed(Action::MENU_UP, input)) {
                std::cout << "Menu: Move up" << std::endl;
            }
            if (bindings.isActionJustPressed(Action::MENU_DOWN, input)) {
                std::cout << "Menu: Move down" << std::endl;
            }
            if (bindings.isActionJustPressed(Action::MENU_SELECT, input)) {
                std::cout << "Menu: Select item" << std::endl;
            }
            if (bindings.isActionJustPressed(Action::MENU_CANCEL, input)) {
                std::cout << "Menu: Cancel/Back" << std::endl;
            }
            break;
            
        case InputContext::PAUSED:
            // Only handle pause/unpause
            if (bindings.isActionJustPressed(Action::PAUSE, input)) {
                std::cout << "Unpausing game" << std::endl;
            }
            break;
    }
}
