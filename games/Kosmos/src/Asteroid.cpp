#include "KosmosBase.h"
#include <cmath>
#include <algorithm>

Chunk::Chunk(int x, int y, int z) : chunkX(x), chunkY(y), chunkZ(z), mesh(nullptr)
{
    for (int i = 0; i < CHUNK_SIZE; ++i)
        for (int j = 0; j < CHUNK_SIZE; ++j)
            for (int k = 0; k < CHUNK_SIZE; ++k)
                voxels[i][j][k] = Voxel(VoxelType::Empty);
}

Voxel &Chunk::getVoxel(int x, int y, int z)
{
    return voxels[x][y][z];
}
void Chunk::setVoxel(int x, int y, int z, VoxelType type, uint8_t data)
{
    voxels[x][y][z] = Voxel(type, data);
}

Asteroid::Asteroid(int dx, int dy, int dz, uint32_t seed) : dimX(dx), dimY(dy), dimZ(dz)
{
    for (int cz = 0; cz < dz; ++cz)
        for (int cy = 0; cy < dy; ++cy)
            for (int cx = 0; cx < dx; ++cx)
                chunks.push_back(std::make_unique<Chunk>(cx, cy, cz));
    generate(seed);
}

Chunk *Asteroid::getChunk(int cx, int cy, int cz)
{
    if (cx < 0 || cy < 0 || cz < 0 || cx >= dimX || cy >= dimY || cz >= dimZ)
        return nullptr;
    return chunks[cx + cy * dimX + cz * dimX * dimY].get();
}

Voxel *Asteroid::getVoxel(int wx, int wy, int wz)
{
    int cx = wx / CHUNK_SIZE, cy = wy / CHUNK_SIZE, cz = wz / CHUNK_SIZE;
    int lx = wx % CHUNK_SIZE, ly = wy % CHUNK_SIZE, lz = wz % CHUNK_SIZE;
    Chunk *chunk = getChunk(cx, cy, cz);
    if (!chunk)
        return nullptr;
    return &chunk->getVoxel(lx, ly, lz);
}

void Asteroid::setVoxel(int wx, int wy, int wz, VoxelType type, uint8_t data)
{
    int cx = wx / CHUNK_SIZE, cy = wy / CHUNK_SIZE, cz = wz / CHUNK_SIZE;
    int lx = wx % CHUNK_SIZE, ly = wy % CHUNK_SIZE, lz = wz % CHUNK_SIZE;
    Chunk *chunk = getChunk(cx, cy, cz);
    if (!chunk)
        return;
    chunk->setVoxel(lx, ly, lz, type, data);
}

void Asteroid::generate(uint32_t seed)
{
    PerlinNoise noise(seed);
    int wxMax = dimX * CHUNK_SIZE, wyMax = dimY * CHUNK_SIZE, wzMax = dimZ * CHUNK_SIZE;
    glm::vec3 center(wxMax / 2.0f, wyMax / 2.0f, wzMax / 2.0f);
    float baseRadius = std::min({wxMax, wyMax, wzMax}) * 0.45f;
    for (int z = 0; z < wzMax; ++z)
    {
        for (int y = 0; y < wyMax; ++y)
        {
            for (int x = 0; x < wxMax; ++x)
            {
                glm::vec3 p(x, y, z);
                float r = glm::length(p - center);
                float n = noise.noise(x * 0.07f, y * 0.07f, z * 0.07f);
                float potato = baseRadius + n * (baseRadius * 0.25f);
                if (r < potato)
                    setVoxel(x, y, z, VoxelType::Stone);
            }
        }
    }
    for (int cz = 0; cz < dimZ; ++cz)
        for (int cy = 0; cy < dimY; ++cy)
            for (int cx = 0; cx < dimX; ++cx)
                generateChunkMesh(cx, cy, cz);
}

void Asteroid::generateChunkMesh(int cx, int cy, int cz)
{
    Chunk *chunk = getChunk(cx, cy, cz);
    if (!chunk)
        return;
    auto mesh = std::make_unique<Mesh>();
    static const float faceVerts[6][12] = {
        // -X (left): CCW from outside
        {0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0},
        // +X (right): CCW from outside
        {1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1},
        // +Z (front): CCW from outside
        {0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1},
        // -Z (back): CCW from outside
        {1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0},
        // +Y (top): CCW from outside
        {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0},
        // -Y (bottom): CCW from outside
        {0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1}};
    static const float faceNormals[6][3] = {
        {-1, 0, 0}, {1, 0, 0}, {0, 0, 1}, {0, 0, -1}, {0, 1, 0}, {0, -1, 0}};
    static const uint16_t quadIndices[6] = {0, 1, 2, 0, 2, 3};
    // For now, always use the first tile (index 0) for stone faces
    extern DotBlue::GLTextureAtlas *g_atlas_for_mesh; // must be set before calling generateChunkMesh
    // We'll select the tile for each face (for now, always 0)
    for (int z = 0; z < CHUNK_SIZE; ++z)
    {
        for (int y = 0; y < CHUNK_SIZE; ++y)
        {
            for (int x = 0; x < CHUNK_SIZE; ++x)
            {
                Voxel &v = chunk->voxels[x][y][z];
                if (v.type == VoxelType::Empty)
                    continue;
                for (int face = 0; face < 6; ++face)
                {
                    int nx = x + (int)faceNormals[face][0];
                    int ny = y + (int)faceNormals[face][1];
                    int nz = z + (int)faceNormals[face][2];
                    bool expose = true;
                    if (nx >= 0 && nx < CHUNK_SIZE && ny >= 0 && ny < CHUNK_SIZE && nz >= 0 && nz < CHUNK_SIZE)
                    {
                        if (chunk->voxels[nx][ny][nz].type != VoxelType::Empty)
                            expose = false;
                    }
                    else
                    {
                        int wx = x + chunk->chunkX * CHUNK_SIZE + (int)faceNormals[face][0];
                        int wy = y + chunk->chunkY * CHUNK_SIZE + (int)faceNormals[face][1];
                        int wz = z + chunk->chunkZ * CHUNK_SIZE + (int)faceNormals[face][2];
                        Voxel *nv = getVoxel(wx, wy, wz);
                        if (nv && nv->type != VoxelType::Empty)
                            expose = false;
                    }
                    if (expose)
                    {
                        float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
                        if (g_atlas_for_mesh)
                        {
                            g_atlas_for_mesh->select(0); // Always select tile 0 for now
                            g_atlas_for_mesh->getSelectedUVs(u0, v0, u1, v1);
                        }
                        size_t vertBase = mesh->vertices.size() / 8;
                        for (int i = 0; i < 4; ++i)
                        {
                            float vx = x + faceVerts[face][i * 3 + 0];
                            float vy = y + faceVerts[face][i * 3 + 1];
                            float vz = z + faceVerts[face][i * 3 + 2];
                            float nx = faceNormals[face][0];
                            float ny = faceNormals[face][1];
                            float nz = faceNormals[face][2];
                            float u = (i == 0 || i == 3) ? u0 : u1;
                            float v = (i < 2) ? v0 : v1;
                            mesh->vertices.insert(mesh->vertices.end(), {vx, vy, vz, nx, ny, nz, u, v});
                        }
                        for (int i = 0; i < 6; ++i)
                            mesh->indices.push_back(static_cast<uint16_t>(vertBase + quadIndices[i]));
                    }
                }
            }
        }
    }
    chunk->mesh = std::move(mesh);
}
