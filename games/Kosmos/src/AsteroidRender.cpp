// ...existing code...
#include "KosmosBase.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cstdio>
struct GLMesh
{
    GLuint vao = 0, vbo = 0, ebo = 0;
    size_t indexCount = 0;
    void upload(const Mesh &mesh)
    {
        const char *glVer = (const char *)glGetString(GL_VERSION);
        if (!glVer)
        {
            std::cerr << "[GLMesh::upload] OpenGL context not current or not initialized!" << std::endl;
            indexCount = 0;
            return;
        }
        if (mesh.vertices.empty() || mesh.indices.empty())
        {
            indexCount = 0;
            return;
        }
        if (!vao)
            glGenVertexArrays(1, &vao);
        if (!vbo)
            glGenBuffers(1, &vbo);
        if (!ebo)
            glGenBuffers(1, &ebo);
        if (vao == 0 || vbo == 0 || ebo == 0)
        {
            std::cerr << "[GLMesh::upload] Failed to create VAO/VBO/EBO! vao=" << vao << " vbo=" << vbo << " ebo=" << ebo << std::endl;
            indexCount = 0;
            return;
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint16_t), mesh.indices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); // pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1); // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2); // uv
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        indexCount = mesh.indices.size();
        glBindVertexArray(0); // Unbind VAO to prevent state leakage
    }
    void destroy()
    {
        if (vbo)
        {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
        if (ebo)
        {
            glDeleteBuffers(1, &ebo);
            ebo = 0;
        }
        if (vao)
        {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
        indexCount = 0;
    }
    ~GLMesh() { destroy(); }
};

// Minimal shader sources
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

void AsteroidRender::render(const Asteroid &asteroid, const DotBlue::GLTextureAtlas &atlas, const glm::dmat4 &viewProj, const glm::vec3 &lightDir)
{
    static std::vector<GLMesh> glMeshes;
    static DotBlue::GLShader shader;
    static bool shaderLoaded = false;
    if (!shaderLoaded)
    {
        shaderLoaded = shader.load(voxelVertShader, voxelFragShader);
        if (!shaderLoaded)
        {
            std::cerr << "[AsteroidRender] Shader failed to load!" << std::endl;
            return;
        }
    }
    if (glMeshes.size() != asteroid.chunks.size())
    {
        for (auto &m : glMeshes)
            m.destroy();
        glMeshes.clear();
        glMeshes.resize(asteroid.chunks.size());
        for (size_t i = 0; i < asteroid.chunks.size(); ++i)
        {
            if (asteroid.chunks[i]->mesh)
                glMeshes[i].upload(*asteroid.chunks[i]->mesh);
        }
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    shader.bind();
    shader.setMat4("u_mvp", glm::mat4(viewProj));
    shader.setVec3("u_lightDir", lightDir.x, lightDir.y, lightDir.z);
    shader.setFloat("u_ambient", 0.45f);
    atlas.bind();
    shader.setInt("u_tex", 0);
    for (size_t i = 0; i < asteroid.chunks.size(); ++i)
    {
        const Chunk &chunk = *asteroid.chunks[i];
        const GLMesh &mesh = glMeshes[i];
        if (mesh.indexCount == 0 || mesh.vao == 0 || mesh.vbo == 0 || mesh.ebo == 0)
            continue;
        float offsetX = float(chunk.chunkX * CHUNK_SIZE);
        float offsetY = float(chunk.chunkY * CHUNK_SIZE);
        float offsetZ = float(chunk.chunkZ * CHUNK_SIZE);
        shader.setVec3("u_chunkOffset", offsetX, offsetY, offsetZ);
        if (i == 11)
        {
            const Chunk &debugChunk = *asteroid.chunks[i];
            if (debugChunk.mesh)
            {
            }
        }
        if (mesh.vao == 0 || mesh.vbo == 0 || mesh.ebo == 0)
        {
            continue;
        }
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indexCount), GL_UNSIGNED_SHORT, 0);
    }
    // Unbind shader and texture to avoid affecting subsequent rendering
    shader.unbind();
    glBindTexture(GL_TEXTURE_2D, 0);
}
