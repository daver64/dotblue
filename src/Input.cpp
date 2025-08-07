#include "DotBlue/Input.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

namespace DotBlue {

    // Global input instances
    std::unique_ptr<InputManager> g_inputManager = nullptr;
    std::unique_ptr<InputBindings> g_inputBindings = nullptr;

    // InputManager Implementation
    InputManager::InputManager() {
        // Initialize keyboard state arrays
        memset(previousKeyboardState, 0, sizeof(previousKeyboardState));
        
        // Initialize SDL game controller subsystem
        if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
            std::cerr << "Failed to initialize SDL Game Controller subsystem: " << SDL_GetError() << std::endl;
        }
        
        initializeControllers();
    }

    InputManager::~InputManager() {
        // Close all controllers
        for (auto controller : controllers) {
            if (controller) {
                SDL_GameControllerClose(controller);
            }
        }
        controllers.clear();
        gamepadStates.clear();
    }

    void InputManager::update() {
        // Store previous keyboard state
        if (keyboardState) {
            memcpy(previousKeyboardState, keyboardState, SDL_NUM_SCANCODES);
        }
        
        // Get current keyboard state
        keyboardState = SDL_GetKeyboardState(nullptr);
        
        // Store previous mouse state
        previousMouseX = mouseX;
        previousMouseY = mouseY;
        previousMouseButtons = mouseButtons;
        
        // Get current mouse state
        mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
        
        // Reset mouse wheel (it's event-based)
        mouseWheelX = 0;
        mouseWheelY = 0;
        
        // Update controllers
        updateControllers();
    }

    void InputManager::initializeControllers() {
        // Scan for connected controllers
        int numJoysticks = SDL_NumJoysticks();
        for (int i = 0; i < numJoysticks; ++i) {
            if (SDL_IsGameController(i)) {
                addController(i);
            }
        }
    }

    void InputManager::updateControllers() {
        for (size_t i = 0; i < controllers.size(); ++i) {
            if (!controllers[i] || !gamepadStates[i].connected) continue;
            
            GamepadState& state = gamepadStates[i];
            SDL_GameController* controller = controllers[i];
            
            // Update previous button state
            state.updatePreviousState();
            
            // Update button states
            for (int btn = 0; btn < SDL_CONTROLLER_BUTTON_MAX; ++btn) {
                state.buttons[btn] = SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)btn);
            }
            
            // Update analog inputs
            state.leftStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
            state.leftStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
            state.rightStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
            state.rightStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;
            state.leftTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
            state.rightTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
        }
    }

    void InputManager::addController(int deviceIndex) {
        SDL_GameController* controller = SDL_GameControllerOpen(deviceIndex);
        if (controller) {
            controllers.push_back(controller);
            gamepadStates.push_back(GamepadState());
            gamepadStates.back().connected = true;
            
            // Initialize smoother for this controller
            stickSmoothers[static_cast<int>(controllers.size() - 1)] = InputSmoother(0.1f);
            
            std::cout << "Controller connected: " << SDL_GameControllerName(controller) << std::endl;
        }
    }

    void InputManager::removeController(int instanceID) {
        for (size_t i = 0; i < controllers.size(); ++i) {
            if (controllers[i] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllers[i])) == instanceID) {
                SDL_GameControllerClose(controllers[i]);
                controllers.erase(controllers.begin() + i);
                gamepadStates.erase(gamepadStates.begin() + i);
                stickSmoothers.erase(static_cast<int>(i));
                std::cout << "Controller disconnected" << std::endl;
                break;
            }
        }
    }

    void InputManager::handleSDLEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_MOUSEWHEEL:
                mouseWheelX = event.wheel.x;
                mouseWheelY = event.wheel.y;
                break;
                
            case SDL_CONTROLLERDEVICEADDED:
                addController(event.cdevice.which);
                break;
                
            case SDL_CONTROLLERDEVICEREMOVED:
                removeController(event.cdevice.which);
                break;
        }
    }

    // Keyboard input methods
    bool InputManager::isKeyPressed(SDL_Scancode key) const {
        return keyboardState && keyboardState[key];
    }

    bool InputManager::isKeyJustPressed(SDL_Scancode key) const {
        return keyboardState && keyboardState[key] && !previousKeyboardState[key];
    }

    bool InputManager::isKeyJustReleased(SDL_Scancode key) const {
        return keyboardState && !keyboardState[key] && previousKeyboardState[key];
    }

    // Mouse input methods
    bool InputManager::isMouseButtonPressed(int button) const {
        return (mouseButtons & SDL_BUTTON(button)) != 0;
    }

    bool InputManager::isMouseButtonJustPressed(int button) const {
        return (mouseButtons & SDL_BUTTON(button)) && !(previousMouseButtons & SDL_BUTTON(button));
    }

    bool InputManager::isMouseButtonJustReleased(int button) const {
        return !(mouseButtons & SDL_BUTTON(button)) && (previousMouseButtons & SDL_BUTTON(button));
    }

    Vec2 InputManager::getMousePosition() const {
        return Vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
    }

    Vec2 InputManager::getMouseDelta() const {
        return Vec2(static_cast<float>(mouseX - previousMouseX), 
                   static_cast<float>(mouseY - previousMouseY)) * mouseSensitivity;
    }

    Vec2 InputManager::getMouseWheel() const {
        return Vec2(static_cast<float>(mouseWheelX), static_cast<float>(mouseWheelY));
    }

    // Controller input methods
    int InputManager::getControllerCount() const {
        return static_cast<int>(controllers.size());
    }

    bool InputManager::isControllerConnected(int controller) const {
        return controller >= 0 && controller < static_cast<int>(gamepadStates.size()) && 
               gamepadStates[controller].connected;
    }

    bool InputManager::isControllerButtonPressed(int controller, SDL_GameControllerButton button) const {
        if (!isControllerConnected(controller) || button >= SDL_CONTROLLER_BUTTON_MAX) return false;
        return gamepadStates[controller].buttons[button];
    }

    bool InputManager::isControllerButtonJustPressed(int controller, SDL_GameControllerButton button) const {
        if (!isControllerConnected(controller) || button >= SDL_CONTROLLER_BUTTON_MAX) return false;
        return gamepadStates[controller].buttons[button] && 
               !gamepadStates[controller].previousButtons[button];
    }

    bool InputManager::isControllerButtonJustReleased(int controller, SDL_GameControllerButton button) const {
        if (!isControllerConnected(controller) || button >= SDL_CONTROLLER_BUTTON_MAX) return false;
        return !gamepadStates[controller].buttons[button] && 
               gamepadStates[controller].previousButtons[button];
    }

    Vec2 InputManager::applyDeadZone(float x, float y) const {
        float magnitude = std::sqrt(x * x + y * y);
        if (magnitude < deadZone) {
            return Vec2(0.0f, 0.0f);
        }
        
        // Scale to remove dead zone
        float scale = (magnitude - deadZone) / (1.0f - deadZone);
        return Vec2(x, y) * (scale / magnitude);
    }

    Vec2 InputManager::getControllerLeftStick(int controller, bool smooth) const {
        if (!isControllerConnected(controller)) return Vec2(0.0f, 0.0f);
        
        const GamepadState& state = gamepadStates[controller];
        Vec2 stick = applyDeadZone(state.leftStickX, state.leftStickY);
        
        if (smooth && stickSmoothers.count(controller)) {
            return const_cast<InputManager*>(this)->stickSmoothers[controller].smooth(stick);
        }
        
        return stick;
    }

    Vec2 InputManager::getControllerRightStick(int controller, bool smooth) const {
        if (!isControllerConnected(controller)) return Vec2(0.0f, 0.0f);
        
        const GamepadState& state = gamepadStates[controller];
        Vec2 stick = applyDeadZone(state.rightStickX, state.rightStickY);
        
        if (smooth && stickSmoothers.count(controller)) {
            return const_cast<InputManager*>(this)->stickSmoothers[controller].smooth(stick);
        }
        
        return stick;
    }

    float InputManager::getControllerLeftTrigger(int controller) const {
        if (!isControllerConnected(controller)) return 0.0f;
        return gamepadStates[controller].leftTrigger;
    }

    float InputManager::getControllerRightTrigger(int controller) const {
        if (!isControllerConnected(controller)) return 0.0f;
        return gamepadStates[controller].rightTrigger;
    }

    // InputBindings Implementation
    void InputBindings::bindKey(Action action, SDL_Scancode key) {
        keyBindings[action].push_back(key);
    }

    void InputBindings::bindButton(Action action, SDL_GameControllerButton button) {
        buttonBindings[action].push_back(button);
    }

    void InputBindings::bindMouseButton(Action action, int mouseButton) {
        mouseBindings[action].push_back(mouseButton);
    }

    void InputBindings::clearBindings(Action action) {
        keyBindings[action].clear();
        buttonBindings[action].clear();
        mouseBindings[action].clear();
    }

    void InputBindings::clearAllBindings() {
        keyBindings.clear();
        buttonBindings.clear();
        mouseBindings.clear();
    }

    bool InputBindings::isActionPressed(Action action, const InputManager& input, int controller) const {
        // Check keyboard bindings
        if (keyBindings.count(action)) {
            for (SDL_Scancode key : keyBindings.at(action)) {
                if (input.isKeyPressed(key)) return true;
            }
        }
        
        // Check mouse bindings
        if (mouseBindings.count(action)) {
            for (int button : mouseBindings.at(action)) {
                if (input.isMouseButtonPressed(button)) return true;
            }
        }
        
        // Check controller bindings
        if (buttonBindings.count(action)) {
            for (SDL_GameControllerButton button : buttonBindings.at(action)) {
                if (input.isControllerButtonPressed(controller, button)) return true;
            }
        }
        
        return false;
    }

    bool InputBindings::isActionJustPressed(Action action, const InputManager& input, int controller) const {
        // Check keyboard bindings
        if (keyBindings.count(action)) {
            for (SDL_Scancode key : keyBindings.at(action)) {
                if (input.isKeyJustPressed(key)) return true;
            }
        }
        
        // Check mouse bindings
        if (mouseBindings.count(action)) {
            for (int button : mouseBindings.at(action)) {
                if (input.isMouseButtonJustPressed(button)) return true;
            }
        }
        
        // Check controller bindings
        if (buttonBindings.count(action)) {
            for (SDL_GameControllerButton button : buttonBindings.at(action)) {
                if (input.isControllerButtonJustPressed(controller, button)) return true;
            }
        }
        
        return false;
    }

    bool InputBindings::isActionJustReleased(Action action, const InputManager& input, int controller) const {
        // Check keyboard bindings
        if (keyBindings.count(action)) {
            for (SDL_Scancode key : keyBindings.at(action)) {
                if (input.isKeyJustReleased(key)) return true;
            }
        }
        
        // Check mouse bindings
        if (mouseBindings.count(action)) {
            for (int button : mouseBindings.at(action)) {
                if (input.isMouseButtonJustReleased(button)) return true;
            }
        }
        
        // Check controller bindings
        if (buttonBindings.count(action)) {
            for (SDL_GameControllerButton button : buttonBindings.at(action)) {
                if (input.isControllerButtonJustReleased(controller, button)) return true;
            }
        }
        
        return false;
    }

    float InputBindings::getActionValue(Action action, const InputManager& input, int controller) const {
        // For digital inputs, return 1.0f if pressed, 0.0f if not
        return isActionPressed(action, input, controller) ? 1.0f : 0.0f;
    }

    Vec2 InputBindings::getActionVector(Action xAction, Action yAction, const InputManager& input, int controller) const {
        float x = getActionValue(xAction, input, controller);
        float y = getActionValue(yAction, input, controller);
        return Vec2(x, y);
    }

    void InputBindings::loadDefaultBindings() {
        clearAllBindings();
        
        // Movement bindings
        bindKey(Action::MOVE_FORWARD, SDL_SCANCODE_W);
        bindKey(Action::MOVE_BACKWARD, SDL_SCANCODE_S);
        bindKey(Action::MOVE_LEFT, SDL_SCANCODE_A);
        bindKey(Action::MOVE_RIGHT, SDL_SCANCODE_D);
        bindKey(Action::MOVE_UP, SDL_SCANCODE_SPACE);
        bindKey(Action::MOVE_DOWN, SDL_SCANCODE_LCTRL);
        
        // Rotation bindings
        bindKey(Action::TURN_LEFT, SDL_SCANCODE_LEFT);
        bindKey(Action::TURN_RIGHT, SDL_SCANCODE_RIGHT);
        bindKey(Action::PITCH_UP, SDL_SCANCODE_UP);
        bindKey(Action::PITCH_DOWN, SDL_SCANCODE_DOWN);
        bindKey(Action::ROLL_LEFT, SDL_SCANCODE_Q);
        bindKey(Action::ROLL_RIGHT, SDL_SCANCODE_E);
        
        // Combat bindings
        bindMouseButton(Action::FIRE_PRIMARY, SDL_BUTTON_LEFT);
        bindMouseButton(Action::FIRE_SECONDARY, SDL_BUTTON_RIGHT);
        bindKey(Action::RELOAD, SDL_SCANCODE_R);
        
        // Utility bindings
        bindKey(Action::JUMP, SDL_SCANCODE_SPACE);
        bindKey(Action::CROUCH, SDL_SCANCODE_LCTRL);
        bindKey(Action::USE_ITEM, SDL_SCANCODE_F);
        bindKey(Action::TOGGLE_MAP, SDL_SCANCODE_M);
        bindKey(Action::PAUSE, SDL_SCANCODE_ESCAPE);
        
        // UI bindings
        bindKey(Action::MENU_UP, SDL_SCANCODE_UP);
        bindKey(Action::MENU_DOWN, SDL_SCANCODE_DOWN);
        bindKey(Action::MENU_LEFT, SDL_SCANCODE_LEFT);
        bindKey(Action::MENU_RIGHT, SDL_SCANCODE_RIGHT);
        bindKey(Action::MENU_SELECT, SDL_SCANCODE_RETURN);
        bindKey(Action::MENU_CANCEL, SDL_SCANCODE_ESCAPE);
        
        // Controller bindings
        bindButton(Action::MOVE_FORWARD, SDL_CONTROLLER_BUTTON_DPAD_UP);
        bindButton(Action::MOVE_BACKWARD, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        bindButton(Action::MOVE_LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        bindButton(Action::MOVE_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        bindButton(Action::JUMP, SDL_CONTROLLER_BUTTON_A);
        bindButton(Action::FIRE_PRIMARY, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        bindButton(Action::FIRE_SECONDARY, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        bindButton(Action::PAUSE, SDL_CONTROLLER_BUTTON_START);
        bindButton(Action::MENU_SELECT, SDL_CONTROLLER_BUTTON_A);
        bindButton(Action::MENU_CANCEL, SDL_CONTROLLER_BUTTON_B);
    }

    std::vector<SDL_Scancode> InputBindings::getKeyBindings(Action action) const {
        if (keyBindings.count(action)) {
            return keyBindings.at(action);
        }
        return {};
    }

    std::vector<SDL_GameControllerButton> InputBindings::getButtonBindings(Action action) const {
        if (buttonBindings.count(action)) {
            return buttonBindings.at(action);
        }
        return {};
    }

    std::vector<int> InputBindings::getMouseBindings(Action action) const {
        if (mouseBindings.count(action)) {
            return mouseBindings.at(action);
        }
        return {};
    }

    // Global convenience functions
    void InitializeInput() {
        g_inputManager = std::make_unique<InputManager>();
        g_inputBindings = std::make_unique<InputBindings>();
        g_inputBindings->loadDefaultBindings();
    }

    void ShutdownInput() {
        g_inputManager.reset();
        g_inputBindings.reset();
    }

    void UpdateInput() {
        if (g_inputManager) {
            g_inputManager->update();
        }
    }

    InputManager& GetInputManager() {
        return *g_inputManager;
    }

    InputBindings& GetInputBindings() {
        return *g_inputBindings;
    }

} // namespace DotBlue
