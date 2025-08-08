#define SDL_MAIN_HANDLED  // Prevent SDL from redefining main
#include "GameBase.h"
#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"

// Include OpenGL headers
#ifdef _WIN32
#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#elif defined(__linux__) || defined(__FreeBSD__)
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include <iostream>
#include <cmath>

class SpaceGame : public GameBase
{
private:
    unsigned int starTexture;
    float rotation;
    DotBlue::GLShader colorShader;
    DotBlue::GLShader textureShader;
    
public:
    bool Initialize() override 
    {
        std::cout << "Initializing Space Game..." << std::endl;
        
        // Load shaders
        if (!colorShader.loadFromFiles("shaders/passthrough.vert", "shaders/passthrough.frag")) {
            std::cout << "Failed to load color shader" << std::endl;
            return false;
        }
        std::cout << "Color shader loaded successfully" << std::endl;
        
        if (!textureShader.loadFromFiles("shaders/textured.vert", "shaders/textured.frag")) {
            std::cout << "Failed to load texture shader" << std::endl;
            return false;
        }
        std::cout << "Texture shader loaded successfully" << std::endl;
        
        // Try to load a texture (optional - we'll handle if it fails)
        starTexture = 0;
        try {
            starTexture = DotBlue::LoadPNGTexture("assets/star.png");
            std::cout << "Star texture loaded: " << starTexture << std::endl;
        } catch (...) {
            std::cout << "No star texture found, using primitives only" << std::endl;
        }
        
        rotation = 0.0f;
        return true;
    }
    
    void Update(float deltaTime) override 
    {
        // Rotate our demo objects
        rotation += deltaTime * 45.0f; // 45 degrees per second
        if (rotation > 360.0f) rotation -= 360.0f;
    }
    
    void Render() override 
    {
        // Don't clear here - DotBlue handles screen clearing
        // glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);
        
        // Get window size for shader uniform
        int width = 800, height = 600; // Default window size, you can get actual size from DotBlue
        
        // Bind color shader for primitive rendering
        colorShader.bind();
        colorShader.setVec2("u_resolution", (float)width, (float)height);
        
        // Draw some colorful shapes using DotBlue functions
        
        // Red triangle (moving)
        float triX = 100.0f + std::sin(rotation * 3.14159f / 180.0f) * 50.0f;
        DotBlue::GLTriangleShader(triX, 100.0f, triX + 50.0f, 150.0f, triX + 25.0f, 50.0f, 
                                  1.0f, 0.2f, 0.2f); // Red
        
        // Green rectangle
        DotBlue::GLRectangleShader(200.0f, 200.0f, 300.0f, 280.0f, 
                                   0.2f, 1.0f, 0.2f); // Green
        
        // Blue line (rotating)
        float lineLen = 80.0f;
        float radians = rotation * 3.14159f / 180.0f;
        float x1 = 400.0f + std::cos(radians) * lineLen;
        float y1 = 200.0f + std::sin(radians) * lineLen;
        DotBlue::GLLineShader(400.0f, 200.0f, x1, y1, 
                              0.2f, 0.2f, 1.0f); // Blue
        
        // White rectangle border
        DotBlue::GLRectangleShader(350.0f, 350.0f, 450.0f, 450.0f, 
                                   1.0f, 1.0f, 1.0f); // White
        
        // If we have a texture, draw it using texture shader
        if (starTexture > 0) {
            textureShader.bind();
            textureShader.setVec2("u_resolution", (float)width, (float)height);
            textureShader.setInt("u_texture", 0); // Set texture unit 0
            DotBlue::TexturedQuadShader(starTexture, 500.0f, 100.0f, 580.0f, 180.0f);
        }
        
       // std::cout << "Rendering space objects... rotation: " << rotation << std::endl;
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

// Use the convenience macro to create main function with smooth renderer
DOTBLUE_GAME_MAIN_SMOOTH(SpaceGame)
