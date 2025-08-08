#include <DotBlue/DotBlue.h>
#include <DotBlue/GLPlatform.h>
#include <DotBlue/SmoothRenderer.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <cmath>
#include <iostream>

#include "GameBase.h"

// Include ImGui headers
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#if defined(__linux__) || defined(__FreeBSD__)
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif

class SpaceGame : public GameBase
{
private:
    DotBlue::GLShader colorShader;
    DotBlue::GLShader textureShader;
    GLuint starTexture;
    float rotation;
    
    // ImGui context
    ImGuiContext* imgui_context;
    bool imgui_initialized;

public:
    SpaceGame() : rotation(0.0f), imgui_context(nullptr), imgui_initialized(false) {}

    bool Initialize() override 
    {
        std::cout << "Initializing Space Game..." << std::endl;
        
        // Skip ImGui initialization entirely to avoid STB crashes
        // The Linux system STB library has compatibility issues with ImGui font atlas building
        imgui_initialized = false;
        std::cout << "ImGui disabled due to STB library compatibility issues on Linux" << std::endl;
        
        // Load shaders
        if (!colorShader.loadFromFiles("shaders/passthrough.vert", "shaders/passthrough.frag")) {
            std::cerr << "Failed to load color shader" << std::endl;
            return false;
        }
        std::cout << "Color shader loaded successfully" << std::endl;
        
        if (!textureShader.loadFromFiles("shaders/textured.vert", "shaders/textured.frag")) {
            std::cerr << "Failed to load texture shader" << std::endl;
            return false;
        }
        std::cout << "Texture shader loaded successfully" << std::endl;
        
        // Load star texture
        starTexture = DotBlue::LoadPNGTexture("assets/star.png");
        if (starTexture == 0) {
            std::cerr << "Failed to load star texture" << std::endl;
            return false;
        }
        std::cout << "Star texture loaded: " << starTexture << std::endl;
        
        rotation = 0.0f;
        return true;
    }
    
    void Update(float deltaTime) override 
    {
        // Rotate our demo objects
        rotation += deltaTime * 45.0f; // 45 degrees per second
        if (rotation > 360.0f) rotation -= 360.0f;
        
        std::cout << "Rendering frame... rotation: " << rotation << std::endl;
    }
    
    void Render() override 
    {
        // Set up OpenGL state
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);  // Dark blue background
        glClear(GL_COLOR_BUFFER_BIT);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Get window size for shader uniform
        int width = 800, height = 600; // Default window size
        
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
        
        // Draw rotating stars
        if (starTexture > 0) {
            textureShader.bind();
            textureShader.setVec2("u_resolution", (float)width, (float)height);
            textureShader.setInt("u_texture", 0); // Set texture unit 0
            
            float starSize = 32.0f;
            for (int i = 0; i < 5; i++) {
                float angle = (rotation + i * 72.0f) * 3.14159f / 180.0f; // Stars every 72 degrees
                float radius = 150.0f;
                float starX = 400.0f + std::cos(angle) * radius - starSize/2;
                float starY = 300.0f + std::sin(angle) * radius - starSize/2;
                
                DotBlue::TexturedQuadShader(starTexture, starX, starY, starX + starSize, starY + starSize);
            }
        }
        
        // Render ImGui (if properly initialized) but skip problematic parts
        if (imgui_initialized && imgui_context) {
            ImGui::SetCurrentContext(imgui_context);
            
            // Start the ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
            
            // Simple ImGui window without text (to avoid font issues)
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Status")) {
                // Just colored boxes instead of text to avoid font rendering
                ImGui::ColorButton("##red", ImVec4(1,0,0,1));
                ImGui::SameLine();
                ImGui::ColorButton("##green", ImVec4(0,1,0,1));
                ImGui::SameLine();
                ImGui::ColorButton("##blue", ImVec4(0,0,1,1));
            }
            ImGui::End();
            
            // Render ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }
    
    void Shutdown() override {
        if (imgui_initialized) {
            ImGui_ImplOpenGL3_Shutdown();
            if (imgui_context) {
                ImGui::DestroyContext(imgui_context);
                imgui_context = nullptr;
            }
            imgui_initialized = false;
        }
    }
    
    void HandleInput(const DotBlue::InputManager& input, const DotBlue::InputBindings& bindings) override 
    {
        // Simple input handling
    }
    
    ~SpaceGame() {
        Shutdown();
    }
};

// Forward declaration
static void HandleX11Event(void* xevent);

// Global pointer for event handling
static SpaceGame* g_game = nullptr;

// Use the convenience macro to create main function with smooth renderer
DOTBLUE_GAME_MAIN_SMOOTH(SpaceGame)

#if defined(__linux__) || defined(__FreeBSD__)
// X11 event handler for ImGui input
static void HandleX11Event(void* xevent)
{
    if (!g_game) return;
    
    XEvent* xev = static_cast<XEvent*>(xevent);
    
    // Only handle basic events, avoid complex ImGui interactions that might cause issues
    switch (xev->type)
    {
        case MotionNotify:
            // Basic mouse tracking without ImGui for now
            break;
            
        case ButtonPress:
        case ButtonRelease:
            // Basic mouse button handling without ImGui for now
            break;
            
        case KeyPress:
        case KeyRelease:
            // Basic keyboard handling without ImGui for now
            break;
    }
}
#endif
