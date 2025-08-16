#include "KosmosBase.h"
//#include "Perlin.cpp" // Or #include "Perlin.h" if you split header/impl
#include <cmath>

// --- Chunk Implementation ---
Chunk::Chunk(int x, int y, int z) : chunkX(x), chunkY(y), chunkZ(z) {
    for (int i = 0; i < CHUNK_SIZE; ++i)
        for (int j = 0; j < CHUNK_SIZE; ++j)
            for (int k = 0; k < CHUNK_SIZE; ++k)
                voxels[i][j][k] = Voxel();
}
Voxel& Chunk::getVoxel(int x, int y, int z) {
    return voxels[x][y][z];
}
void Chunk::setVoxel(int x, int y, int z, VoxelType type, uint8_t data) {
    voxels[x][y][z] = Voxel(type, data);
}

// --- Asteroid Implementation ---
Asteroid::Asteroid(int dx, int dy, int dz, uint32_t seed)
    : dimX(dx), dimY(dy), dimZ(dz) {
    chunks.reserve(dimX * dimY * dimZ);
    for (int z = 0; z < dimZ; ++z)
        for (int y = 0; y < dimY; ++y)
            for (int x = 0; x < dimX; ++x)
                chunks.emplace_back(std::make_unique<Chunk>(x, y, z));
    generate(seed);
}
Chunk* Asteroid::getChunk(int cx, int cy, int cz) {
    if (cx < 0 || cy < 0 || cz < 0 || cx >= dimX || cy >= dimY || cz >= dimZ) return nullptr;
    return chunks[cx + cy * dimX + cz * dimX * dimY].get();
}
Voxel* Asteroid::getVoxel(int wx, int wy, int wz) {
    int cx = wx / CHUNK_SIZE, cy = wy / CHUNK_SIZE, cz = wz / CHUNK_SIZE;
    int lx = wx % CHUNK_SIZE, ly = wy % CHUNK_SIZE, lz = wz % CHUNK_SIZE;
    Chunk* chunk = getChunk(cx, cy, cz);
    if (!chunk) return nullptr;
    return &chunk->getVoxel(lx, ly, lz);
}
void Asteroid::generate(uint32_t seed) {
    PerlinNoise shapeNoise(seed);
    PerlinNoise oreNoise(seed + 101);
    PerlinNoise volatileNoise(seed + 202);
    // Center in voxel space
    double cx = dimX * CHUNK_SIZE / 2.0;
    double cy = dimY * CHUNK_SIZE / 2.0;
    double cz = dimZ * CHUNK_SIZE / 2.0;
    double baseRadius = 0.45 * std::min({dimX, dimY, dimZ}) * CHUNK_SIZE;
    double amplitude = 0.18 * baseRadius;
    double freq = 0.08;
    for (int gz = 0; gz < dimZ * CHUNK_SIZE; ++gz)
    for (int gy = 0; gy < dimY * CHUNK_SIZE; ++gy)
    for (int gx = 0; gx < dimX * CHUNK_SIZE; ++gx) {
        double dx = gx - cx, dy = gy - cy, dz = gz - cz;
        double r = std::sqrt(dx*dx + dy*dy + dz*dz);
        double len = std::max(1e-6, std::sqrt(dx*dx + dy*dy + dz*dz));
        double nx = dx / len, ny = dy / len, nz = dz / len;
        double noise = shapeNoise.noise(nx * freq, ny * freq, nz * freq);
        double surface = baseRadius + noise * amplitude;
        if (r < surface) {
            // Material distribution
            double oreVal = oreNoise.noise(gx * 0.07, gy * 0.07, gz * 0.07);
            double volVal = volatileNoise.noise(gx * 0.09, gy * 0.09, gz * 0.09);
            VoxelType t = VoxelType::Stone;
            if (oreVal > 0.55) t = VoxelType::Iron;
            else if (volVal > 0.55) t = VoxelType::Ice;
            setVoxel(gx, gy, gz, t);
        } else {
            setVoxel(gx, gy, gz, VoxelType::Empty);
        }
    }
}
void Asteroid::setVoxel(int wx, int wy, int wz, VoxelType type, uint8_t data) {
    int cx = wx / CHUNK_SIZE, cy = wy / CHUNK_SIZE, cz = wz / CHUNK_SIZE;
    int lx = wx % CHUNK_SIZE, ly = wy % CHUNK_SIZE, lz = wz % CHUNK_SIZE;
    Chunk* chunk = getChunk(cx, cy, cz);
    if (chunk) chunk->setVoxel(lx, ly, lz, type, data);
}
void Asteroid::render(const GLTextureAtlas& atlas, const glm::dmat4& viewProj) {
    // For each chunk and voxel, render solid voxels using the atlas
    for (const auto& chunkPtr : chunks) {
        const Chunk& chunk = *chunkPtr;
        for (int z = 0; z < CHUNK_SIZE; ++z)
        for (int y = 0; y < CHUNK_SIZE; ++y)
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            const Voxel& v = chunk.voxels[x][y][z];
            if (v.type == VoxelType::Empty) continue;
            // Set atlas index based on voxel type
            int atlasIdx = 0;
            if (v.type == VoxelType::Stone) atlasIdx = 0;
            else if (v.type == VoxelType::Iron) atlasIdx = 1;
            else if (v.type == VoxelType::Ice) atlasIdx = 2;
            atlas.select(atlasIdx);
            // Compute world position
            glm::dvec3 pos = glm::dvec3(
                (chunk.chunkX * CHUNK_SIZE + x),
                (chunk.chunkY * CHUNK_SIZE + y),
                (chunk.chunkZ * CHUNK_SIZE + z)
            );
            // Model matrix for this voxel
            glm::dmat4 model = glm::translate(glm::dmat4(1.0), pos);
            glm::dmat4 mvp = viewProj * model;
            // Bind texture and draw cube (user must implement drawCube)
            // drawCube(mvp, atlas.getTextureID());
        }
    }
}

// --- Camera Implementation ---
Camera::Camera() : position(0,0,100), target(0,0,0), up(0,1,0), yaw(0), pitch(0), distance(100) {}
void Camera::updateFromInput(float deltaTime) {
    // User must implement: update yaw/pitch/distance from mouse/keys, then update position
}
glm::dmat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, up);
}
