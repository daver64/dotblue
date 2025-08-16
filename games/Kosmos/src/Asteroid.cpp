#include "KosmosBase.h"
#include <cstdint>
#include <memory>
#include <cstdio>
// Implementation for Asteroid::setVoxel
void Asteroid::setVoxel(int wx, int wy, int wz, VoxelType type, uint8_t data) {
    int cx = wx / CHUNK_SIZE, cy = wy / CHUNK_SIZE, cz = wz / CHUNK_SIZE;
    int lx = wx % CHUNK_SIZE, ly = wy % CHUNK_SIZE, lz = wz % CHUNK_SIZE;
    Chunk* chunk = getChunk(cx, cy, cz);
    if (chunk) chunk->setVoxel(lx, ly, lz, type, data);
}
#include <cmath>
#include <iostream>
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

constexpr int DEFAULT_CHUNKS_X = 2;
constexpr int DEFAULT_CHUNKS_Y = 2;
constexpr int DEFAULT_CHUNKS_Z = 2;

Asteroid::Asteroid(int dx, int dy, int dz, uint32_t seed)
    : dimX(dx), dimY(dy), dimZ(dz)
{
    int chunkCount = dx * dy * dz;
    chunks.reserve(chunkCount);
    for (int cz = 0; cz < dz; ++cz) {
        for (int cy = 0; cy < dy; ++cy) {
            for (int cx = 0; cx < dx; ++cx) {
                chunks.push_back(std::make_unique<Chunk>(cx, cy, cz));
            }
        }
    }
    generate(seed);
}

Chunk* Asteroid::getChunk(int cx, int cy, int cz) {
    if (cx < 0 || cy < 0 || cz < 0 || cx >= dimX || cy >= dimY || cz >= dimZ) return nullptr;
    int idx = cz * dimY * dimX + cy * dimX + cx;
    return chunks[idx].get();
}

Voxel* Asteroid::getVoxel(int wx, int wy, int wz) {
    int cx = wx / CHUNK_SIZE, cy = wy / CHUNK_SIZE, cz = wz / CHUNK_SIZE;
    int lx = wx % CHUNK_SIZE, ly = wy % CHUNK_SIZE, lz = wz % CHUNK_SIZE;
    Chunk* chunk = getChunk(cx, cy, cz);
    if (!chunk) return nullptr;
    return &chunk->voxels[lx][ly][lz];
}

void Asteroid::generate(uint32_t seed) {
    PerlinNoise noise(seed);
    int wxMax = dimX * CHUNK_SIZE;
    int wyMax = dimY * CHUNK_SIZE;
    int wzMax = dimZ * CHUNK_SIZE;
    float cx = wxMax / 2.0f, cy = wyMax / 2.0f, cz = wzMax / 2.0f;
    float radius = std::min({wxMax, wyMax, wzMax}) * 0.22f;
    for (int wz = 0; wz < wzMax; ++wz) {
        for (int wy = 0; wy < wyMax; ++wy) {
            for (int wx = 0; wx < wxMax; ++wx) {
                float dx = wx - cx, dy = wy - cy, dz = wz - cz;
                float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
                float n = noise.noise(wx * 0.08f, wy * 0.08f, wz * 0.08f);
                float threshold = radius + n * 6.0f;
                if (dist < threshold) {
                    VoxelType t = VoxelType::Stone;
                    if (n > 0.35f) t = VoxelType::Iron;
                    else if (n < -0.35f) t = VoxelType::Ice;
                    setVoxel(wx, wy, wz, t, 0);
                } else {
                    setVoxel(wx, wy, wz, VoxelType::Empty, 0);
                }
            }
        }
    }
    // Generate meshes for all chunks
    for (int cz = 0; cz < dimZ; ++cz)
        for (int cy = 0; cy < dimY; ++cy)
            for (int cx = 0; cx < dimX; ++cx)
                generateChunkMesh(cx, cy, cz);
}

void Asteroid::generateChunkMesh(int cx, int cy, int cz) {
    Chunk* chunk = getChunk(cx, cy, cz);
    if (!chunk) return;
    auto mesh = std::make_unique<Mesh>();
    // Corrected face vertex order (CCW as seen from outside)
    static const float faceVerts[6][12] = {
        // -X (left)
        {0,0,1, 0,1,1, 0,1,0, 0,0,0},
        // +X (right)
        {1,0,0, 1,1,0, 1,1,1, 1,0,1},
        // -Y (bottom)
        {0,0,0, 1,0,0, 1,0,1, 0,0,1},
        // +Y (top)
        {0,1,1, 1,1,1, 1,1,0, 0,1,0},
        // -Z (back)
        {1,0,0, 0,0,0, 0,1,0, 1,1,0},
        // +Z (front)
        {0,0,1, 1,0,1, 1,1,1, 0,1,1}
    };
    static const uint32_t quadIndices[6] = {0, 1, 2, 0, 2, 3};
    static const float faceNormals[6][3] = {
        {-1,0,0}, {1,0,0}, {0,-1,0}, {0,1,0}, {0,0,-1}, {0,0,1}
    };
    int faceCount = 0, voxelCount = 0;
    for (int z = 0; z < CHUNK_SIZE; ++z) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                const Voxel& v = chunk->voxels[x][y][z];
                if (v.type == VoxelType::Empty) continue;
                ++voxelCount;
                for (int face = 0; face < 6; ++face) {
                    int nx = x, ny = y, nz = z;
                    if (face == 0) nx--;
                    if (face == 1) nx++;
                    if (face == 2) ny--;
                    if (face == 3) ny++;
                    if (face == 4) nz--;
                    if (face == 5) nz++;
                    bool neighborSolid = false;
                    if (nx < 0 || ny < 0 || nz < 0 || nx >= CHUNK_SIZE || ny >= CHUNK_SIZE || nz >= CHUNK_SIZE) {
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
                        int ncx = nwx / CHUNK_SIZE, ncy = nwy / CHUNK_SIZE, ncz = nwz / CHUNK_SIZE;
                        int nlx = nwx % CHUNK_SIZE, nly = nwy % CHUNK_SIZE, nlz = nwz % CHUNK_SIZE;
                        if (nlx < 0) nlx += CHUNK_SIZE;
                        if (nly < 0) nly += CHUNK_SIZE;
                        if (nlz < 0) nlz += CHUNK_SIZE;
                        Chunk* neighborChunk = getChunk(ncx, ncy, ncz);
                        if (neighborChunk && nlx >= 0 && nlx < CHUNK_SIZE && nly >= 0 && nly < CHUNK_SIZE && nlz >= 0 && nlz < CHUNK_SIZE) {
                            neighborSolid = (neighborChunk->voxels[nlx][nly][nlz].type != VoxelType::Empty);
                        }
                    } else {
                        neighborSolid = (chunk->voxels[nx][ny][nz].type != VoxelType::Empty);
                    }
                    if (!neighborSolid) {
                        const float* fd = faceVerts[face];
                        const float* normal = faceNormals[face];
                        float px = float(x), py = float(y), pz = float(z);
                        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                        int base = int(v.type);
                        float tileSize = 1.0f / 16.0f;
                        u0 = (base % 16) * tileSize;
                        v0 = (base / 16) * tileSize;
                        u1 = u0 + tileSize;
                        v1 = v0 + tileSize;
                        size_t vertBase = mesh->vertices.size() / 8;
                        for (int i = 0; i < 4; ++i) {
                            float wx = px + fd[i*3+0];
                            float wy = py + fd[i*3+1];
                            float wz = pz + fd[i*3+2];
                            float u = (i == 0 || i == 3) ? u0 : u1;
                            float v = (i < 2) ? v0 : v1;
                            mesh->vertices.insert(mesh->vertices.end(), {wx, wy, wz, normal[0], normal[1], normal[2], u, v});
                        }
                        for (int i = 0; i < 6; ++i) {
                            mesh->indices.push_back(static_cast<uint32_t>(vertBase + quadIndices[i]));
                        }
                        ++faceCount;
                    }
                }
            }
        }
    }
    chunk->mesh = std::move(mesh);
    printf("[Asteroid] Chunk (%d,%d,%d): %d voxels, %d faces\n", cx, cy, cz, voxelCount, faceCount);
}