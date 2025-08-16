#include "KosmosBase.h"
// Implementation for Asteroid::setVoxel
void Asteroid::setVoxel(int wx, int wy, int wz, VoxelType type, uint8_t data) {
    int cx = wx / CHUNK_SIZE, cy = wy / CHUNK_SIZE, cz = wz / CHUNK_SIZE;
    int lx = wx % CHUNK_SIZE, ly = wy % CHUNK_SIZE, lz = wz % CHUNK_SIZE;
    Chunk* chunk = getChunk(cx, cy, cz);
    if (chunk) chunk->setVoxel(lx, ly, lz, type, data);
}
#include <cmath>

// --- Chunk Implementation ---
Chunk::Chunk(int x, int y, int z) : chunkX(x), chunkY(y), chunkZ(z), mesh(nullptr) {
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
        float nx = static_cast<float>(dx / len);
        float ny = static_cast<float>(dy / len);
        float nz = static_cast<float>(dz / len);
        float noise = shapeNoise.noise(nx * static_cast<float>(freq), ny * static_cast<float>(freq), nz * static_cast<float>(freq));
        double surface = baseRadius + noise * amplitude;
        if (r < surface) {
            // Material distribution
            float oreVal = oreNoise.noise(gx * 0.07f, gy * 0.07f, gz * 0.07f);
            float volVal = volatileNoise.noise(gx * 0.09f, gy * 0.09f, gz * 0.09f);
            VoxelType t = VoxelType::Stone;
            if (oreVal > 0.55f) t = VoxelType::Iron;
            else if (volVal > 0.55f) t = VoxelType::Ice;
            setVoxel(gx, gy, gz, t);
        } else {
            setVoxel(gx, gy, gz, VoxelType::Empty);
        }
    }
    // Generate mesh for all chunks
    for (int cz = 0; cz < dimZ; ++cz)
        for (int cy = 0; cy < dimY; ++cy)
            for (int cx = 0; cx < dimX; ++cx)
                generateChunkMesh(cx, cy, cz);
}
void Asteroid::generateChunkMesh(int cx, int cy, int cz) {
    Chunk* chunk = getChunk(cx, cy, cz);
    if (!chunk) return;
    auto mesh = std::make_unique<Mesh>();
    // For each voxel in chunk, add faces if exposed
    for (int z = 0; z < CHUNK_SIZE; ++z)
    for (int y = 0; y < CHUNK_SIZE; ++y)
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        const Voxel& v = chunk->voxels[x][y][z];
        if (v.type == VoxelType::Empty) continue;
        // For each face direction, check neighbor
        for (int face = 0; face < 6; ++face) {
            int nx = x, ny = y, nz = z;
            if (face == 0) nx--;
            if (face == 1) nx++;
            if (face == 2) ny--;
            if (face == 3) ny++;
            if (face == 4) nz--;
            if (face == 5) nz++;
            bool exposed = false;
            if (nx < 0 || ny < 0 || nz < 0 || nx >= CHUNK_SIZE || ny >= CHUNK_SIZE || nz >= CHUNK_SIZE) {
                // Neighbor is out of chunk, check world
                int wx = chunk->chunkX * CHUNK_SIZE + x;
                int wy = chunk->chunkY * CHUNK_SIZE + y;
                int wz = chunk->chunkZ * CHUNK_SIZE + z;
                int nwx = wx, nwy = wy, nwz = wz;
                if (face == 0) nwx--;
                if (face == 1) nwx++;
                if (face == 2) nwy--;
                if (face == 3) nwy++;
                if (face == 4) nwz--;
                if (face == 5) nwz++;
                Voxel* neighbor = getVoxel(nwx, nwy, nwz);
                if (!neighbor || neighbor->type == VoxelType::Empty) exposed = true;
            } else {
                if (chunk->voxels[nx][ny][nz].type == VoxelType::Empty) exposed = true;
            }
            if (exposed) {
                // Add face vertices, normals, UVs to mesh->vertices
                // Add indices to mesh->indices
                // (User must implement actual face generation)
            }
        }
    }
    chunk->mesh = std::move(mesh);
}
void Asteroid::render(const DotBlue::GLTextureAtlas& atlas, const glm::dmat4& viewProj) {
    // For each chunk, render its mesh
    for (const auto& chunkPtr : chunks) {
        const Chunk& chunk = *chunkPtr;
        if (!chunk.mesh) continue;
        // Bind texture, set up shader, upload mesh data, draw
        // (User must implement actual rendering)
    }
}
