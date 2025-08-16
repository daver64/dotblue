#define SDL_MAIN_HANDLED // Prevent SDL from redefining main
#include "KosmosBase.h"
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
static long HandleWindowMessage(void *hwnd, unsigned int msg, unsigned long long wParam, long long lParam);
#elif defined(__linux__) || defined(__FreeBSD__)
// Forward declaration of X11 event handler
static void HandleX11Event(void *xevent);
#endif

#include <chrono>
#include <thread>


class Kosmos : public KosmosBase
{
private:
    Asteroid* asteroid = nullptr;
    AsteroidRender* asteroidRenderer = nullptr;
    DotBlue::GLTextureAtlas* atlas = nullptr;
    DotBlue::GLCamera camera;
    bool showKosmosUI;

public:
    Kosmos() { std::cerr << "[Kosmos] Constructor called." << std::endl; }
    ~Kosmos() {
        delete asteroid;
        delete asteroidRenderer;
        delete atlas;
        std::cerr << "[Kosmos] Destructor called." << std::endl;
    }

    bool Initialize() override
    {
        std::cerr << "Initializing Kosmos..." << std::endl;
#ifdef _WIN32
        DotBlue::SetWindowMessageCallback(HandleWindowMessage);
#elif defined(__linux__) || defined(__FreeBSD__)
        DotBlue::SetX11EventCallback(HandleX11Event);
#endif
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
#ifdef _WIN32
        HWND hwnd = static_cast<HWND>(DotBlue::GetWindowHandle());
        ImGui_ImplWin32_Init(hwnd);
#endif
        ImGui_ImplOpenGL3_Init("#version 130");
        showKosmosUI = true;

        // Create asteroid at world origin
        asteroid = new Asteroid(8, 8, 8, 42); // 8x8x8 chunks (128^3 voxels), seed=42

        // Place camera at (0,0,-32) looking at center (0,0,0)
        camera.setPosition(glm::dvec3(0.0, 0.0, -32.0));
        camera.setTarget(glm::dvec3(0.0, 0.0, 0.0));
        camera.setUp(glm::dvec3(0.0, 1.0, 0.0));
        camera.setFOV(60.0);
        camera.setAspect(16.0 / 9.0);
        camera.setNearFar(0.1, 1000.0);

        // Load texture atlas (mc.png, 16x16 tiles)
        atlas = new DotBlue::GLTextureAtlas("mc.png", 16, 16);

        // AsteroidRender is just a static class, no need to instantiate
        return true;
    }

    void Update(float deltaTime) override
    {
        // Future: camera movement, input, etc.
    }

    void Render() override
    {
        // Clear the screen for 3D rendering
        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        // Render asteroid with texture atlas and lighting
        glm::dmat4 view = camera.getViewMatrix();
        glm::dmat4 proj = glm::perspective(glm::radians(camera.getFOV()), camera.getAspect(), camera.getNear(), camera.getFar());
        glm::dmat4 viewProj = proj * view;
        glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
        AsteroidRender::render(*asteroid, *atlas, viewProj, lightDir);

       // RenderUI();
    }

    void RenderUI()
    {
        int width = 0, height = 0;
        DotBlue::GetRenderWindowSize(width, height);
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize.x = (float)width;
        io.DisplaySize.y = (float)height;
#ifdef _WIN32
        ImGui_ImplWin32_NewFrame();
#endif
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        if (showKosmosUI)
        {
            ImGui::Begin("Kosmos UI", &showKosmosUI);
            ImGui::Text("Asteroid at (0,0,0), camera at (0,0,-32)");
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void HandleInput(const DotBlue::InputManager &input, const DotBlue::InputBindings &bindings) override
    {
        // Implement camera controls/input here in the future
    }

    void Shutdown() override
    {
        std::cerr << "[Kosmos] Shutdown() called." << std::endl;
        std::cerr.flush();
        ImGui_ImplOpenGL3_Shutdown();
#ifdef _WIN32
        ImGui_ImplWin32_Shutdown();
#endif
        ImGui::DestroyContext();
        std::cerr << "[Kosmos] Shutdown() completed." << std::endl;
        std::cerr.flush();
    }
};

#ifdef _WIN32
// Window message handler for ImGui input
static long HandleWindowMessage(void *hwnd, unsigned int msg, unsigned long long wParam, long long lParam)
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
static void HandleX11Event(void *xevent)
{
    XEvent *xev = static_cast<XEvent *>(xevent);
    ImGuiIO &io = ImGui::GetIO();

    switch (xev->type)
    {
    case MotionNotify:
        io.MousePos = ImVec2((float)xev->xmotion.x, (float)xev->xmotion.y);
        break;

    case ButtonPress:
        if (xev->xbutton.button == Button1)
            io.MouseDown[0] = true; // Left mouse
        if (xev->xbutton.button == Button3)
            io.MouseDown[1] = true; // Right mouse
        if (xev->xbutton.button == Button2)
            io.MouseDown[2] = true; // Middle mouse
        break;

    case ButtonRelease:
        if (xev->xbutton.button == Button1)
            io.MouseDown[0] = false;
        if (xev->xbutton.button == Button3)
            io.MouseDown[1] = false;
        if (xev->xbutton.button == Button2)
            io.MouseDown[2] = false;
        break;

    case KeyPress:
    {
        KeySym keysym = XLookupKeysym(&xev->xkey, 0);

        // Map X11 keys to ImGui keys (ImGui 1.90+ modern API)
        ImGuiKey imgui_key = ImGuiKey_None;
        if (keysym >= XK_a && keysym <= XK_z)
        {
            imgui_key = (ImGuiKey)(ImGuiKey_A + (keysym - XK_a));
        }
        else if (keysym >= XK_0 && keysym <= XK_9)
        {
            imgui_key = (ImGuiKey)(ImGuiKey_0 + (keysym - XK_0));
        }
        else
        {
            // Map some common special keys
            switch (keysym)
            {
            case XK_space:
                imgui_key = ImGuiKey_Space;
                break;
            case XK_Return:
                imgui_key = ImGuiKey_Enter;
                break;
            case XK_Escape:
                imgui_key = ImGuiKey_Escape;
                break;
            case XK_BackSpace:
                imgui_key = ImGuiKey_Backspace;
                break;
            case XK_Tab:
                imgui_key = ImGuiKey_Tab;
                break;
            case XK_Left:
                imgui_key = ImGuiKey_LeftArrow;
                break;
            case XK_Right:
                imgui_key = ImGuiKey_RightArrow;
                break;
            case XK_Up:
                imgui_key = ImGuiKey_UpArrow;
                break;
            case XK_Down:
                imgui_key = ImGuiKey_DownArrow;
                break;
            default:
                break;
            }
        }

        if (imgui_key != ImGuiKey_None)
        {
            io.AddKeyEvent(imgui_key, true);
        }

        // Handle character input for printable characters
        if (keysym >= 32 && keysym <= 126)
        {
            io.AddInputCharacter((unsigned int)keysym);
        }
        break;
    }

    case KeyRelease:
    {
        KeySym keysym = XLookupKeysym(&xev->xkey, 0);

        // Map X11 keys to ImGui keys (same mapping as KeyPress)
        ImGuiKey imgui_key = ImGuiKey_None;
        if (keysym >= XK_a && keysym <= XK_z)
        {
            imgui_key = (ImGuiKey)(ImGuiKey_A + (keysym - XK_a));
        }
        else if (keysym >= XK_0 && keysym <= XK_9)
        {
            imgui_key = (ImGuiKey)(ImGuiKey_0 + (keysym - XK_0));
        }
        else
        {
            switch (keysym)
            {
            case XK_space:
                imgui_key = ImGuiKey_Space;
                break;
            case XK_Return:
                imgui_key = ImGuiKey_Enter;
                break;
            case XK_Escape:
                imgui_key = ImGuiKey_Escape;
                break;
            case XK_BackSpace:
                imgui_key = ImGuiKey_Backspace;
                break;
            case XK_Tab:
                imgui_key = ImGuiKey_Tab;
                break;
            case XK_Left:
                imgui_key = ImGuiKey_LeftArrow;
                break;
            case XK_Right:
                imgui_key = ImGuiKey_RightArrow;
                break;
            case XK_Up:
                imgui_key = ImGuiKey_UpArrow;
                break;
            case XK_Down:
                imgui_key = ImGuiKey_DownArrow;
                break;
            default:
                break;
            }
        }

        if (imgui_key != ImGuiKey_None)
        {
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
DOTBLUE_GAME_MAIN_SMOOTH(Kosmos)
