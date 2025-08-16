
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "DotBlue/GLPlatform.h"

struct Mesh
{
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
};

// #include <vector>
// #include <memory>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include "DotBlue/GLPlatform.h"

class Camera
{
public:
    glm::dvec3 position, target, up;
    double yaw, pitch, distance;
    Camera();
    void updateFromInput(float deltaTime);
    glm::dmat4 getViewMatrix() const;
};
#include <vector>
#include <cstdint>
#include <random>

class PerlinNoise
{
public:
    PerlinNoise(uint32_t seed = 0);

    float noise(float x) const;
    float noise(float x, float y) const;
    float noise(float x, float y, float z) const;
    float noise(float x, float y, float z, float w) const;

private:
    std::vector<int> p;
    static float fade(float t);
    static float lerp(float a, float b, float t);
    static float grad(int hash, float x);
    static float grad(int hash, float x, float y);
    static float grad(int hash, float x, float y, float z);
    static float grad(int hash, float x, float y, float z, float w);
};
#pragma once
#include <DotBlue/DotBlue.h>
#include <atomic>

// Forward declarations
namespace DotBlue
{
    class InputManager;
    class InputBindings;
}

class KosmosBase
{
public:
    KosmosBase();
    virtual ~KosmosBase();

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
    virtual void HandleInput(const DotBlue::InputManager &input, const DotBlue::InputBindings &bindings) = 0;

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

class IniFile
{
public:
    bool load(const std::string &filename);
    bool save(const std::string &filename) const;

    std::string getString(const std::string &section, const std::string &key, const std::string &def = "") const;
    int getInt(const std::string &section, const std::string &key, int def = 0) const;
    float getFloat(const std::string &section, const std::string &key, float def = 0.0f) const;

    void setString(const std::string &section, const std::string &key, const std::string &value);
    void setInt(const std::string &section, const std::string &key, int value);
    void setFloat(const std::string &section, const std::string &key, float value);

private:
    std::map<std::string, std::map<std::string, std::string>> data;
    static void trim(std::string &s);
};
enum class VoxelType : uint8_t
{
    Empty,
    Stone,
    Iron,
    Ice,
};

struct Voxel
{
    VoxelType type;
    uint8_t data;
    Voxel() : type(VoxelType::Empty), data(0) {}
    Voxel(VoxelType t, uint8_t d = 0) : type(t), data(d) {}
};

constexpr int CHUNK_SIZE = 16;
class Chunk
{
public:
    Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    int chunkX, chunkY, chunkZ;
    std::unique_ptr<Mesh> mesh;
    Chunk(int x, int y, int z);
    Voxel &getVoxel(int x, int y, int z);
    void setVoxel(int x, int y, int z, VoxelType type, uint8_t data = 0);
};

constexpr int MIN_CHUNKS = 4;
constexpr int MAX_CHUNKS = 32;
class Asteroid
{
public:
    int dimX, dimY, dimZ;
    std::vector<std::unique_ptr<Chunk>> chunks;
    Asteroid(int dx, int dy, int dz, uint32_t seed);
    Chunk *getChunk(int cx, int cy, int cz);
    Voxel *getVoxel(int wx, int wy, int wz);
    void setVoxel(int wx, int wy, int wz, VoxelType type, uint8_t data = 0);
    void generate(uint32_t seed);
    void generateChunkMesh(int cx, int cy, int cz);
};
class AsteroidRender
{
public:
    // Renders the asteroid using per-chunk meshes, GLTextureAtlas, and lighting
    static void render(const Asteroid &asteroid, const DotBlue::GLTextureAtlas &atlas, const glm::dmat4 &viewProj, const glm::vec3 &lightDir);
};

// FileSystem utility for config directory
#include <string>
class FileSystem
{
public:
    static std::string getKosmosConfigDir();
};
// Convenience macro for main function
#define DOTBLUE_GAME_MAIN(GameClass) \
    int main()                       \
    {                                \
        GameClass game;              \
        return game.Run();           \
    }

// Smooth version that prevents window-move freezing
#define DOTBLUE_GAME_MAIN_SMOOTH(GameClass) \
    int main()                              \
    {                                       \
        GameClass game;                     \
        return game.RunSmooth();            \
    }
