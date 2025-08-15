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
    
    // Smooth game loop (prevents window-move freezing)
    int RunSmooth();
    
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


// INI file handler for config
#include <string>
#include <map>

class IniFile {
public:
    bool load(const std::string& filename);
    bool save(const std::string& filename) const;

    std::string getString(const std::string& section, const std::string& key, const std::string& def = "") const;
    int getInt(const std::string& section, const std::string& key, int def = 0) const;
    float getFloat(const std::string& section, const std::string& key, float def = 0.0f) const;

    void setString(const std::string& section, const std::string& key, const std::string& value);
    void setInt(const std::string& section, const std::string& key, int value);
    void setFloat(const std::string& section, const std::string& key, float value);

private:
    std::map<std::string, std::map<std::string, std::string>> data;
    static void trim(std::string& s);
};


// FileSystem utility for config directory
#include <string>
class FileSystem {
public:
    static std::string getKosmosConfigDir();
};

// Convenience macro for main function
#define DOTBLUE_GAME_MAIN(GameClass) \
int main() \
{ \
    GameClass game; \
    return game.Run(); \
}

// Smooth version that prevents window-move freezing
#define DOTBLUE_GAME_MAIN_SMOOTH(GameClass) \
int main() \
{ \
    GameClass game; \
    return game.RunSmooth(); \
}
