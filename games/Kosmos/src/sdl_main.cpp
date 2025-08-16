#include <cmath>
#include <thread>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "KosmosBase.h"
// File deleted as per user request.
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"

// Kosmos class definition (SDL version)
class Kosmos : public KosmosBase
{
private:
    Asteroid *asteroid = nullptr;
    AsteroidRender *asteroidRenderer = nullptr;
    DotBlue::GLTextureAtlas *atlas = nullptr;
    DotBlue::GLCamera camera;
    bool showKosmosUI;
public:
    double camYaw = 0.0, camPitch = 0.0;
    Kosmos() { std::cerr << "[Kosmos] Constructor called." << std::endl; }
    ~Kosmos()
    {
        delete asteroid;
        delete asteroidRenderer;
        delete atlas;
        std::cerr << "[Kosmos] Destructor called." << std::endl;
    }

    void Render() override
    {
        // Minimal implementation to satisfy abstract base class
    }

    bool Initialize() override
    {
        std::cerr << "Initializing Kosmos..." << std::endl;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 330");
        showKosmosUI = true;

        // Load texture atlas (mc.png, 16x16 tiles)
        atlas = new DotBlue::GLTextureAtlas("../assets/mc.png", 16, 16);
        g_atlas_for_mesh = atlas;
        atlas->select(0); // Select the first tile for all faces

        // Create asteroid at world origin
        asteroid = new Asteroid(8, 8, 8, 42); // 8x8x8 chunks (128^3 voxels), seed=42

        // Place camera at (0,0,-80) looking at center (0,0,0) for a better view
        camera.setPosition(glm::dvec3(0.0, 0.0, -80.0));
        camYaw = 0.0;
        camPitch = 0.0;
        camera.setTarget(glm::dvec3(0.0, 0.0, 0.0));
        camera.setUp(glm::dvec3(0.0, 1.0, 0.0));
        camera.setFOV(70.0); // Slightly wider FOV
        camera.setAspect(16.0 / 9.0);
        camera.setNearFar(0.1, 1000.0);

        return true;
    }

    void Update(float deltaTime, int mouseDX, int mouseDY, const Uint8* state) // pass SDL input
    {
        double speed = 40.0 * deltaTime;
        double mouseSensitivity = 0.15;
        camYaw += mouseDX * mouseSensitivity;
        camPitch -= mouseDY * mouseSensitivity;
        if (camPitch > 89.0) camPitch = 89.0;
        if (camPitch < -89.0) camPitch = -89.0;
        double yawRad = glm::radians(camYaw);
        double pitchRad = glm::radians(camPitch);
        glm::dvec3 forward(
            cos(pitchRad) * cos(yawRad),
            sin(pitchRad),
            cos(pitchRad) * sin(yawRad));
        glm::dvec3 worldUp(0, 1, 0);
        glm::dvec3 right = glm::normalize(glm::cross(forward, worldUp));
        bool moved = false;
        glm::dvec3 camPos = camera.getPosition();
        auto canMoveTo = [&](const glm::dvec3 &pos)
        {
            int wx = int(std::round(pos.x));
            int wy = int(std::round(pos.y));
            int wz = int(std::round(pos.z));
            Voxel *v = asteroid->getVoxel(wx, wy, wz);
            return (!v || v->type == VoxelType::Empty);
        };
        if (state[SDL_SCANCODE_W]) {
            glm::dvec3 newPos = camPos + forward * speed;
            if (canMoveTo(newPos)) { camPos = newPos; moved = true; }
        }
        if (state[SDL_SCANCODE_S]) {
            glm::dvec3 newPos = camPos - forward * speed;
            if (canMoveTo(newPos)) { camPos = newPos; moved = true; }
        }
        if (state[SDL_SCANCODE_A]) {
            glm::dvec3 newPos = camPos - right * speed;
            if (canMoveTo(newPos)) { camPos = newPos; moved = true; }
        }
        if (state[SDL_SCANCODE_D]) {
            glm::dvec3 newPos = camPos + right * speed;
            if (canMoveTo(newPos)) { camPos = newPos; moved = true; }
        }
        if (state[SDL_SCANCODE_UP]) {
            glm::dvec3 newPos = camPos + worldUp * speed;
            if (canMoveTo(newPos)) { camPos = newPos; moved = true; }
        }
        if (state[SDL_SCANCODE_DOWN]) {
            glm::dvec3 newPos = camPos - worldUp * speed;
            if (canMoveTo(newPos)) { camPos = newPos; moved = true; }
        }
        if (moved) {
            camera.setPosition(camPos);
        }
        // After movement, always recalculate forward and set target
        forward = glm::dvec3(
            cos(pitchRad) * cos(yawRad),
            sin(pitchRad),
            cos(pitchRad) * sin(yawRad));
        camera.setTarget(camera.getPosition() + forward);
        camera.setUp(worldUp);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glm::dmat4 view = camera.getViewMatrix();
        glm::dmat4 proj = glm::perspective(glm::radians(camera.getFOV()), camera.getAspect(), camera.getNear(), camera.getFar());
        glm::dmat4 viewProj = proj * view;
        glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
        AsteroidRender::render(*asteroid, *atlas, viewProj, lightDir);
    }

    void Shutdown() override
    {
        std::cerr << "[Kosmos] Shutdown() called." << std::endl;
        std::cerr.flush();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        std::cerr << "[Kosmos] Shutdown() completed." << std::endl;
        std::cerr.flush();
    }
};

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window* window = SDL_CreateWindow("Kosmos", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "glewInit failed" << std::endl;
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");
    // Your game setup
    Kosmos game;
    game.Initialize();
    bool running = true;
    SDL_SetRelativeMouseMode(SDL_TRUE);
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }
        int mouseDX, mouseDY;
        SDL_GetRelativeMouseState(&mouseDX, &mouseDY);
        const Uint8* state = SDL_GetKeyboardState(NULL);
        // Update game logic with SDL input
        game.Update(1.0f / 60.0f, mouseDX, mouseDY, state); // Replace with real deltaTime if desired
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        // Render game
        game.Render();
        // Render ImGui
        ImGui::Render();
        glViewport(0, 0, 1280, 720);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
