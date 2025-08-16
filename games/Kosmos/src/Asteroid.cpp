#include "KosmosBase.h"
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
Asteroid::Asteroid(int dx, int dy, int dz, uint32_t seed)
    : dimX(dx), dimY(dy), dimZ(dz) {
    std::cerr << "[Asteroid] Allocating " << (dimX * dimY * dimZ) << " chunks..." << std::endl;
    chunks.reserve(dimX * dimY * dimZ);
    for (int z = 0; z < dimZ; ++z)
        for (int y = 0; y < dimY; ++y)
            for (int x = 0; x < dimX; ++x)
                chunks.emplace_back(std::make_unique<Chunk>(x, y, z));
    std::cerr << "[Asteroid] Generating asteroid voxels and meshes..." << std::endl;
    generate(seed);
    std::cerr << "[Asteroid] Generation complete." << std::endl;
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
    if (!chunk) {
        std::cerr << "[Asteroid] generateChunkMesh: null chunk at " << cx << "," << cy << "," << cz << std::endl;
        return;
    }
    auto mesh = std::make_unique<Mesh>();
    // Face data: {dx, dy, dz, nx, ny, nz, {4x (vx, vy, vz, u, v)}}
    static const float faceData[6][20] = {
        // -X
        {0,0,0, -1,0,0, 0,0,0, 0,1,0, 0,1,1, 0,0,1},
        // +X
        {1,0,0, 1,0,0, 1,0,0, 1,0,1, 1,1,1, 1,1,0},
        // -Y
        {0,0,0, 0,-1,0, 0,0,0, 1,0,0, 1,0,1, 0,0,1},
        // +Y
        {0,1,0, 0,1,0, 0,1,0, 0,1,1, 1,1,1, 1,1,0},
        // -Z
        {0,0,0, 0,0,-1, 0,0,0, 1,0,0, 1,1,0, 0,1,0},
        // +Z
        {0,0,1, 0,0,1, 0,0,1, 0,1,1, 1,1,1, 1,0,1}
    };
    // For each voxel in chunk, add faces if exposed
    for (int z = 0; z < CHUNK_SIZE; ++z)
    for (int y = 0; y < CHUNK_SIZE; ++y)
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        const Voxel& v = chunk->voxels[x][y][z];
        if (v.type == VoxelType::Empty) continue;
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
                // Add face (4 vertices, 2 triangles)
                const float* fd = faceData[face];
                float px = float(x), py = float(y), pz = float(z);
                float nx_ = fd[3], ny_ = fd[4], nz_ = fd[5];
                // Simple UVs: tile per VoxelType
                float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
                // Optionally: assign different UVs per VoxelType
                int base = int(v.type);
                float tileSize = 1.0f / 16.0f;
                u0 = (base % 16) * tileSize;
                v0 = (base / 16) * tileSize;
                u1 = u0 + tileSize;
                v1 = v0 + tileSize;
                // Vertex order: 0,1,2,3 (quad)
                size_t vertBase = mesh->vertices.size() / 8;
                for (int i = 0; i < 4; ++i) {
                    float vx = fd[6 + i*3 + 0];
                    float vy = fd[6 + i*3 + 1];
                    float vz = fd[6 + i*3 + 2];
                    float wx = px + vx;
                    float wy = py + vy;
                    float wz = pz + vz;
                    float u = (i == 0 || i == 3) ? u0 : u1;
                    float v = (i < 2) ? v0 : v1;
                    mesh->vertices.insert(mesh->vertices.end(), {wx, wy, wz, nx_, ny_, nz_, u, v});
                }
                // Indices (two triangles)
                mesh->indices.push_back((uint32_t)vertBase + 0);
                mesh->indices.push_back((uint32_t)vertBase + 1);
                mesh->indices.push_back((uint32_t)vertBase + 2);
                mesh->indices.push_back((uint32_t)vertBase + 2);
                mesh->indices.push_back((uint32_t)vertBase + 3);
                mesh->indices.push_back((uint32_t)vertBase + 0);
            }
        }
    }
    if (mesh->vertices.empty() || mesh->indices.empty()) {
        // No geometry for this chunk
        // std::cerr << "[Asteroid] Chunk (" << cx << "," << cy << "," << cz << ") is empty." << std::endl;
    } else {
        std::cerr << "[Asteroid] Chunk (" << cx << "," << cy << "," << cz << ") mesh: "
                  << (mesh->vertices.size() / 8) << " verts, " << mesh->indices.size() << " indices." << std::endl;
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
