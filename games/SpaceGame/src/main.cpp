#define SDL_MAIN_HANDLED  // Prevent SDL from redefining main
#include "GameBase.h"
#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"

// Include ImGui headers
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#ifdef _WIN32
#include <windows.h>
#include "backends/imgui_impl_win32.h"
// Forward declare the ImGui Win32 handler (copied from imgui_impl_win32.h)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#elif defined(__linux__) || defined(__FreeBSD__)
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif

// Include OpenGL headers
#ifdef _WIN32
#include <GL/glew.h>
#include <GL/gl.h>
#elif defined(__linux__) || defined(__FreeBSD__)
#include <GL/glew.h>
#include <GL/gl.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif

#include <iostream>
#include <cmath>

#ifdef _WIN32
// Forward declaration of window message handler
static long HandleWindowMessage(void* hwnd, unsigned int msg, unsigned long long wParam, long long lParam);
#elif defined(__linux__) || defined(__FreeBSD__)
// Forward declaration of X11 event handler
static void HandleX11Event(void* xevent);
#endif

class SpaceGame : public GameBase
{
private:
    unsigned int starTexture;
    float rotation;
    DotBlue::GLShader colorShader;
    DotBlue::GLShader textureShader;
    DotBlue::GLFont gameFont;
    bool showSpaceGameUI;
    
public:
    bool Initialize() override 
    {
        std::cout << "Initializing Space Game..." << std::endl;
        
#ifdef _WIN32
        // Set up window message callback for ImGui input handling
        DotBlue::SetWindowMessageCallback(HandleWindowMessage);
#elif defined(__linux__) || defined(__FreeBSD__)
        // Set up X11 event callback for ImGui input handling
        DotBlue::SetX11EventCallback(HandleX11Event);
#endif
        
        // Initialize ImGui - it's our responsibility since DotBlue no longer does it
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        ImGui::StyleColorsDark();
        
        // Initialize ImGui backends
#ifdef _WIN32
        // Get the window handle from DotBlue
        HWND hwnd = static_cast<HWND>(DotBlue::GetWindowHandle());
        ImGui_ImplWin32_Init(hwnd);
#endif
        ImGui_ImplOpenGL3_Init("#version 130");
        
        showSpaceGameUI = true;
        
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
        
        // Load font for GLPrintf
#ifdef _WIN32
        gameFont = DotBlue::LoadFont("C:/Windows/Fonts/consola.ttf", 16.0f);
#elif defined(__linux__) || defined(__FreeBSD__)
        gameFont = DotBlue::LoadFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 16.0f);
#endif
        std::cout << "Game font loaded successfully" << std::endl;
        
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
        // Set up OpenGL state - this is now the game's responsibility
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);  // Dark blue background
        glClear(GL_COLOR_BUFFER_BIT);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
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
        
    
        // ImGui rendering
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = (float)width;
        io.DisplaySize.y = (float)height;
        
#ifdef _WIN32
        ImGui_ImplWin32_NewFrame();
#endif
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        
        // SpaceGame UI
        if (showSpaceGameUI) {
            ImGui::Begin("Space Game Control", &showSpaceGameUI);
            ImGui::Text("Welcome to Space Game!");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                       1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::SliderFloat("Rotation Speed", &rotation, 0.0f, 360.0f);
            if (ImGui::Button("Reset Rotation")) {
                rotation = 0.0f;
            }
            ImGui::End();
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // Test GLPrintf functionality with yellow text at (20, 20)
        // Do this AFTER all shader-based rendering to avoid state conflicts
        // Set up OpenGL state for legacy rendering (GLPrintf uses immediate mode)
        glUseProgram(0); // Disable shaders to use fixed function pipeline
        
        // Set up orthographic projection for 2D text rendering
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, (double)width, (double)height, 0.0, -1.0, 1.0); // Top-left origin
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // Ensure proper state for text rendering
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        
        DotBlue::RGBA yellowColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow color
        DotBlue::GLPrintf(gameFont, 20.0f, 20.0f, yellowColor, "GLPrintf Test: Rotation %.1f degrees", rotation);
        
        // Restore matrices
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        
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
        
        // Cleanup ImGui since we initialized it
        ImGui_ImplOpenGL3_Shutdown();
#ifdef _WIN32
        ImGui_ImplWin32_Shutdown();
#endif
        ImGui::DestroyContext();
    }
};

#ifdef _WIN32
// Window message handler for ImGui input
static long HandleWindowMessage(void* hwnd, unsigned int msg, unsigned long long wParam, long long lParam)
{
    // Cast to proper Windows types
    HWND hWnd = static_cast<HWND>(hwnd);
    UINT uMsg = static_cast<UINT>(msg);
    WPARAM wp = static_cast<WPARAM>(wParam);
    LPARAM lp = static_cast<LPARAM>(lParam);
    
    // Let ImGui handle the message
    LRESULT result = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wp, lp);
    
    // Return the result directly as a long
    return static_cast<long>(result);
}
#elif defined(__linux__) || defined(__FreeBSD__)
// X11 event handler for ImGui input
static void HandleX11Event(void* xevent)
{
    XEvent* xev = static_cast<XEvent*>(xevent);
    ImGuiIO& io = ImGui::GetIO();
    
    switch (xev->type)
    {
        case MotionNotify:
            io.MousePos = ImVec2((float)xev->xmotion.x, (float)xev->xmotion.y);
            break;
            
        case ButtonPress:
            if (xev->xbutton.button == Button1) io.MouseDown[0] = true;  // Left mouse
            if (xev->xbutton.button == Button3) io.MouseDown[1] = true;  // Right mouse
            if (xev->xbutton.button == Button2) io.MouseDown[2] = true;  // Middle mouse
            break;
            
        case ButtonRelease:
            if (xev->xbutton.button == Button1) io.MouseDown[0] = false;
            if (xev->xbutton.button == Button3) io.MouseDown[1] = false;
            if (xev->xbutton.button == Button2) io.MouseDown[2] = false;
            break;
            
        case KeyPress:
        {
            KeySym keysym = XLookupKeysym(&xev->xkey, 0);
            
            // Map X11 keys to ImGui keys (ImGui 1.90+ modern API)
            ImGuiKey imgui_key = ImGuiKey_None;
            if (keysym >= XK_a && keysym <= XK_z) {
                imgui_key = (ImGuiKey)(ImGuiKey_A + (keysym - XK_a));
            } else if (keysym >= XK_0 && keysym <= XK_9) {
                imgui_key = (ImGuiKey)(ImGuiKey_0 + (keysym - XK_0));
            } else {
                // Map some common special keys
                switch (keysym) {
                    case XK_space: imgui_key = ImGuiKey_Space; break;
                    case XK_Return: imgui_key = ImGuiKey_Enter; break;
                    case XK_Escape: imgui_key = ImGuiKey_Escape; break;
                    case XK_BackSpace: imgui_key = ImGuiKey_Backspace; break;
                    case XK_Tab: imgui_key = ImGuiKey_Tab; break;
                    case XK_Left: imgui_key = ImGuiKey_LeftArrow; break;
                    case XK_Right: imgui_key = ImGuiKey_RightArrow; break;
                    case XK_Up: imgui_key = ImGuiKey_UpArrow; break;
                    case XK_Down: imgui_key = ImGuiKey_DownArrow; break;
                    default: break;
                }
            }
            
            if (imgui_key != ImGuiKey_None) {
                io.AddKeyEvent(imgui_key, true);
            }
            
            // Handle character input for printable characters
            if (keysym >= 32 && keysym <= 126) {
                io.AddInputCharacter((unsigned int)keysym);
            }
            break;
        }
        
        case KeyRelease:
        {
            KeySym keysym = XLookupKeysym(&xev->xkey, 0);
            
            // Map X11 keys to ImGui keys (same mapping as KeyPress)
            ImGuiKey imgui_key = ImGuiKey_None;
            if (keysym >= XK_a && keysym <= XK_z) {
                imgui_key = (ImGuiKey)(ImGuiKey_A + (keysym - XK_a));
            } else if (keysym >= XK_0 && keysym <= XK_9) {
                imgui_key = (ImGuiKey)(ImGuiKey_0 + (keysym - XK_0));
            } else {
                switch (keysym) {
                    case XK_space: imgui_key = ImGuiKey_Space; break;
                    case XK_Return: imgui_key = ImGuiKey_Enter; break;
                    case XK_Escape: imgui_key = ImGuiKey_Escape; break;
                    case XK_BackSpace: imgui_key = ImGuiKey_Backspace; break;
                    case XK_Tab: imgui_key = ImGuiKey_Tab; break;
                    case XK_Left: imgui_key = ImGuiKey_LeftArrow; break;
                    case XK_Right: imgui_key = ImGuiKey_RightArrow; break;
                    case XK_Up: imgui_key = ImGuiKey_UpArrow; break;
                    case XK_Down: imgui_key = ImGuiKey_DownArrow; break;
                    default: break;
                }
            }
            
            if (imgui_key != ImGuiKey_None) {
                io.AddKeyEvent(imgui_key, false);
            }
            break;
        }
        
        case FocusIn:
            io.AddFocusEvent(true);
            break;
            
        case FocusOut:
            io.AddFocusEvent(false);
            break;
    }
}
#endif

// Use the convenience macro to create main function with smooth renderer
DOTBLUE_GAME_MAIN_SMOOTH(SpaceGame)
