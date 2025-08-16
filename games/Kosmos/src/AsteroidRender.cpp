// Simple shader sources for lit textured voxels
#include "KosmosBase.h"
#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <iostream>
static const char *voxelVertShader = R"(
#version 130
uniform mat4 u_mvp;
uniform vec3 u_chunkOffset;
in vec3 a_pos;
in vec3 a_normal;
in vec2 a_uv;
out vec3 v_normal;
out vec2 v_uv;
void main() {
    gl_Position = u_mvp * vec4(a_pos + u_chunkOffset, 1.0);
    v_normal = a_normal;
    v_uv = a_uv;
}

)";
static const char *voxelFragShader = R"(
#version 130
uniform sampler2D u_tex;
uniform vec3 u_lightDir;
uniform float u_ambient;
in vec3 v_normal;
in vec2 v_uv;
out vec4 fragColor;
void main() {
    vec3 normal = normalize(v_normal);
    float diff = max(dot(normal, normalize(u_lightDir)), 0.0);
    float lighting = u_ambient + (1.0 - u_ambient) * diff;
    vec4 tex = texture(u_tex, v_uv);
    fragColor = vec4(tex.rgb * lighting, tex.a);
}
)";
// Helper: Upload mesh to VBO/VAO
struct GLMesh
{
    GLuint vao = 0, vbo = 0, ebo = 0;
    size_t indexCount = 0;
    void upload(const Mesh &mesh)
    {
        if (mesh.vertices.empty() || mesh.indices.empty())
        {
            // std::cerr << "[GLMesh] Skipping upload: empty mesh." << std::endl;
            indexCount = 0;
            return;
        }
        if (!vao)
            glGenVertexArrays(1, &vao);
        if (!vbo)
            glGenBuffers(1, &vbo);
        if (!ebo)
            glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);
    // Set up vertex attributes: pos(3), normal(3), uv(2) = 8 floats per vertex
    glEnableVertexAttribArray(0); // a_pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); // a_normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); // a_uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    indexCount = mesh.indices.size();
    // std::cerr << "[GLMesh] Uploaded mesh: " << (mesh.vertices.size() / 8) << " verts, " << mesh.indices.size() << " indices." << std::endl;
    }
};

// Implementation of AsteroidRender::render
void AsteroidRender::render(const Asteroid& asteroid, const DotBlue::GLTextureAtlas& atlas, const glm::dmat4& viewProj, const glm::vec3& lightDir)
{
    // ...existing code...
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    static bool glInited = false;
    static DotBlue::GLShader voxelShader;
    static std::vector<GLMesh> glMeshes;

    if (!glInited)
    {
        // Use DotBlue GLShader to load from source strings
        bool loaded = voxelShader.load(voxelVertShader, voxelFragShader);
        if (!loaded)
        {
            std::cerr << "[AsteroidRender] ERROR: Shader failed to load!" << std::endl;
            return;
        }
        glInited = true;
    }

    // Upload all chunk meshes
    glMeshes.clear();
    static bool printedDebug = false;
    for (const auto &chunkPtr : asteroid.chunks)
    {
        GLMesh glmesh;
        if (chunkPtr->mesh) {
            glmesh.upload(*chunkPtr->mesh);
            // Print debug info for the first non-empty mesh
            if (!printedDebug && !chunkPtr->mesh->vertices.empty() && !chunkPtr->mesh->indices.empty()) {
                printedDebug = true;
                size_t vcount = chunkPtr->mesh->vertices.size() / 8;
                size_t icount = chunkPtr->mesh->indices.size();
                printf("[AsteroidRender] Debug: First mesh: %zu vertices, %zu indices\n", vcount, icount);
                for (size_t i = 0; i < std::min(vcount, size_t(8)); ++i) {
                    float x = chunkPtr->mesh->vertices[i*8+0];
                    float y = chunkPtr->mesh->vertices[i*8+1];
                    float z = chunkPtr->mesh->vertices[i*8+2];
                    float nx = chunkPtr->mesh->vertices[i*8+3];
                    float ny = chunkPtr->mesh->vertices[i*8+4];
                    float nz = chunkPtr->mesh->vertices[i*8+5];
                    float u = chunkPtr->mesh->vertices[i*8+6];
                    float v = chunkPtr->mesh->vertices[i*8+7];
                    printf("  Vertex %zu: pos(%.2f, %.2f, %.2f) normal(%.2f, %.2f, %.2f) uv(%.2f, %.2f)\n", i, x, y, z, nx, ny, nz, u, v);
                }
                for (size_t i = 0; i < std::min(icount, size_t(12)); i += 3) {
                    printf("  Triangle %zu: %u, %u, %u\n", i/3, chunkPtr->mesh->indices[i], chunkPtr->mesh->indices[i+1], chunkPtr->mesh->indices[i+2]);
                }
            }
        }
        glMeshes.push_back(glmesh);
    }

    voxelShader.bind();
    voxelShader.setMat4("u_mvp", glm::mat4(viewProj));
    voxelShader.setVec3("u_lightDir", lightDir.x, lightDir.y, lightDir.z);
    voxelShader.setFloat("u_ambient", 0.3125f);
        atlas.bind();
    voxelShader.setInt("u_tex", 0);

    // Draw all chunk meshes
    for (size_t chunkIdx = 0; chunkIdx < asteroid.chunks.size(); ++chunkIdx)
    {
        const Chunk &chunk = *asteroid.chunks[chunkIdx];
        GLMesh &glmesh = glMeshes[chunkIdx];
        if (glmesh.indexCount == 0)
            continue;
        float offsetX = float(chunk.chunkX * CHUNK_SIZE);
        float offsetY = float(chunk.chunkY * CHUNK_SIZE);
        float offsetZ = float(chunk.chunkZ * CHUNK_SIZE);
        voxelShader.setVec3("u_chunkOffset", offsetX, offsetY, offsetZ);
        glBindVertexArray(glmesh.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(glmesh.indexCount), GL_UNSIGNED_INT, 0);
    }
}
