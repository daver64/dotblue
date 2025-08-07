#pragma once

#if defined(__linux__) || defined(__FreeBSD__)
#include <SDL2/SDL.h>
#elif defined(_WIN32) || defined(__CYGWIN__)
#include <SDL.h>
#endif

#include <vector>
#include <map>
#include <memory>
#include <cstring>
#include "GLPlatform.h"

namespace DotBlue {

    // Forward declarations
    class InputManager;
    class InputBindings;

    // Game action enumeration - extend this for your specific games
    enum class Action {
        // Movement
        MOVE_FORWARD,
        MOVE_BACKWARD,
        MOVE_LEFT,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN,
        
        // Rotation
        TURN_LEFT,
        TURN_RIGHT,
        PITCH_UP,
        PITCH_DOWN,
        ROLL_LEFT,
        ROLL_RIGHT,
        
        // Combat
        FIRE_PRIMARY,
        FIRE_SECONDARY,
        RELOAD,
        
        // Utility
        JUMP,
        CROUCH,
        USE_ITEM,
        TOGGLE_MAP,
        PAUSE,
        
        // UI
        MENU_UP,
        MENU_DOWN,
        MENU_LEFT,
        MENU_RIGHT,
        MENU_SELECT,
        MENU_CANCEL,
        
        ACTION_COUNT // Keep this last
    };

    // Gamepad state structure
    struct GamepadState {
        bool connected = false;
        float leftStickX = 0.0f, leftStickY = 0.0f;
        float rightStickX = 0.0f, rightStickY = 0.0f;
        float leftTrigger = 0.0f, rightTrigger = 0.0f;
        bool buttons[SDL_CONTROLLER_BUTTON_MAX] = {false};
        bool previousButtons[SDL_CONTROLLER_BUTTON_MAX] = {false};
        
        void updatePreviousState() {
            memcpy(previousButtons, buttons, sizeof(buttons));
        }
    };

    // Input smoothing utility
    class InputSmoother {
    private:
        Vec2 smoothedValue = Vec2(0.0f, 0.0f);
        float smoothingFactor = 0.1f;
        
    public:
        InputSmoother(float factor = 0.1f) : smoothingFactor(factor) {}
        
        Vec2 smooth(const Vec2& rawInput) {
            smoothedValue.x = lerp(smoothedValue.x, rawInput.x, smoothingFactor);
            smoothedValue.y = lerp(smoothedValue.y, rawInput.y, smoothingFactor);
            return smoothedValue;
        }
        
        void setSmoothingFactor(float factor) { smoothingFactor = factor; }
        void reset() { smoothedValue = Vec2(0.0f, 0.0f); }
        
    private:
        float lerp(float a, float b, float t) {
            return a + t * (b - a);
        }
    };

    // Main input manager class
    class InputManager {
    private:
        // Keyboard state
        const Uint8* keyboardState = nullptr;
        Uint8 previousKeyboardState[SDL_NUM_SCANCODES];
        
        // Mouse state
        int mouseX = 0, mouseY = 0;
        int previousMouseX = 0, previousMouseY = 0;
        Uint32 mouseButtons = 0;
        Uint32 previousMouseButtons = 0;
        int mouseWheelX = 0, mouseWheelY = 0;
        
        // Controller state
        std::vector<SDL_GameController*> controllers;
        std::vector<GamepadState> gamepadStates;
        
        // Input smoothing
        std::map<int, InputSmoother> stickSmoothers;
        
        // Settings
        float deadZone = 0.15f;
        float mouseSensitivity = 1.0f;
        
    public:
        InputManager();
        ~InputManager();
        
        // Core update function - call this every frame
        void update();
        
        // Keyboard input
        bool isKeyPressed(SDL_Scancode key) const;
        bool isKeyJustPressed(SDL_Scancode key) const;
        bool isKeyJustReleased(SDL_Scancode key) const;
        
        // Mouse input
        bool isMouseButtonPressed(int button) const;
        bool isMouseButtonJustPressed(int button) const;
        bool isMouseButtonJustReleased(int button) const;
        Vec2 getMousePosition() const;
        Vec2 getMouseDelta() const;
        Vec2 getMouseWheel() const;
        void setMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }
        
        // Controller input
        int getControllerCount() const;
        bool isControllerConnected(int controller) const;
        bool isControllerButtonPressed(int controller, SDL_GameControllerButton button) const;
        bool isControllerButtonJustPressed(int controller, SDL_GameControllerButton button) const;
        bool isControllerButtonJustReleased(int controller, SDL_GameControllerButton button) const;
        
        // Analog input
        Vec2 getControllerLeftStick(int controller, bool smooth = true) const;
        Vec2 getControllerRightStick(int controller, bool smooth = true) const;
        float getControllerLeftTrigger(int controller) const;
        float getControllerRightTrigger(int controller) const;
        
        // Settings
        void setDeadZone(float zone) { deadZone = zone; }
        float getDeadZone() const { return deadZone; }
        
        // Controller management
        void addController(int deviceIndex);
        void removeController(int instanceID);
        
        // Event handling (call this in your SDL event loop)
        void handleSDLEvent(const SDL_Event& event);
        
    private:
        Vec2 applyDeadZone(float x, float y) const;
        void initializeControllers();
        void updateControllers();
    };

    // Input binding system
    class InputBindings {
    private:
        std::map<Action, std::vector<SDL_Scancode>> keyBindings;
        std::map<Action, std::vector<SDL_GameControllerButton>> buttonBindings;
        std::map<Action, std::vector<int>> mouseBindings; // Mouse button bindings
        
    public:
        // Binding management
        void bindKey(Action action, SDL_Scancode key);
        void bindButton(Action action, SDL_GameControllerButton button);
        void bindMouseButton(Action action, int mouseButton);
        
        void clearBindings(Action action);
        void clearAllBindings();
        
        // Action queries
        bool isActionPressed(Action action, const InputManager& input, int controller = 0) const;
        bool isActionJustPressed(Action action, const InputManager& input, int controller = 0) const;
        bool isActionJustReleased(Action action, const InputManager& input, int controller = 0) const;
        
        // Analog input for actions (useful for movement)
        float getActionValue(Action action, const InputManager& input, int controller = 0) const;
        Vec2 getActionVector(Action xAction, Action yAction, const InputManager& input, int controller = 0) const;
        
        // Configuration
        void loadDefaultBindings();
        void saveBindings(const std::string& filename) const;
        void loadBindings(const std::string& filename);
        
        // Utility
        std::vector<SDL_Scancode> getKeyBindings(Action action) const;
        std::vector<SDL_GameControllerButton> getButtonBindings(Action action) const;
        std::vector<int> getMouseBindings(Action action) const;
    };

    // Global input instances
    extern std::unique_ptr<InputManager> g_inputManager;
    extern std::unique_ptr<InputBindings> g_inputBindings;

    // Convenience functions
    void InitializeInput();
    void ShutdownInput();
    void UpdateInput();
    InputManager& GetInputManager();
    InputBindings& GetInputBindings();

} // namespace DotBlue
