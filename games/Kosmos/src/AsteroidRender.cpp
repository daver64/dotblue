#include "KosmosBase.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <iostream>
// Simple shader sources for lit textured voxels
static const char* voxelVertShader = R"(
#version 130
uniform mat4 u_mvp;
in vec3 a_pos;
in vec3 a_normal;
in vec2 a_uv;
out vec3 v_normal;
out vec2 v_uv;
void main() {
    gl_Position = u_mvp * vec4(a_pos, 1.0);
    v_normal = a_normal;
    v_uv = a_uv;
}

)";
static const char* voxelFragShader = R"(
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

// Use DotBlue GLShader for shader management
#include "DotBlue/GLPlatform.h"

// Helper: Upload mesh to VBO/VAO
struct GLMesh {
    GLuint vao = 0, vbo = 0, ebo = 0;
    size_t indexCount = 0;
    void upload(const Mesh& mesh) {
        if (mesh.vertices.empty() || mesh.indices.empty()) {
            std::cerr << "[GLMesh] Skipping upload: empty mesh." << std::endl;
            indexCount = 0;
            return;
        }
        if (!vao) glGenVertexArrays(1, &vao);
        if (!vbo) glGenBuffers(1, &vbo);
        if (!ebo) glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);
        indexCount = mesh.indices.size();
        std::cerr << "[GLMesh] Uploaded mesh: " << (mesh.vertices.size() / 8) << " verts, " << mesh.indices.size() << " indices." << std::endl;
    }
    void setupAttributes(GLuint prog) {
        glBindVertexArray(vao);
        GLint posLoc = glGetAttribLocation(prog, "a_pos");
        GLint normLoc = glGetAttribLocation(prog, "a_normal");
        GLint uvLoc = glGetAttribLocation(prog, "a_uv");
        if (posLoc == -1 || normLoc == -1 || uvLoc == -1) {
            std::cerr << "[GLMesh] ERROR: Attribute location(s) not found: a_pos=" << posLoc << ", a_normal=" << normLoc << ", a_uv=" << uvLoc << std::endl;
            return;
        }
        glEnableVertexAttribArray(posLoc);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(normLoc);
        glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(uvLoc);
        glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    }
    void draw() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, 0);
    }
};

// Store one GLMesh per chunk (static for now)
static std::vector<GLMesh> glMeshes;
static DotBlue::GLShader voxelShader;
static bool glInited = false;

void AsteroidRender::render(const Asteroid& asteroid, const DotBlue::GLTextureAtlas& atlas, const glm::dmat4& viewProj, const glm::vec3& lightDir) {
   // std::cerr << "[AsteroidRender] Begin render. Chunks: " << asteroid.chunks.size() << std::endl;
    if (!glInited) {
        std::cerr << "[AsteroidRender] Initializing shader..." << std::endl;
        // Use DotBlue GLShader to load from source strings
        voxelShader.load(voxelVertShader, voxelFragShader);
        glInited = true;
    }
    // Upload all chunk meshes if not already done
    if (glMeshes.size() != asteroid.chunks.size()) {
        std::cerr << "[AsteroidRender] Uploading chunk meshes..." << std::endl;
        glMeshes.clear();
        for (const auto& chunkPtr : asteroid.chunks) {
            GLMesh glmesh;
            if (chunkPtr->mesh) glmesh.upload(*chunkPtr->mesh);
            glMeshes.push_back(glmesh);
        }
    }
    voxelShader.bind();
    // Set uniforms using DotBlue GLShader API
    voxelShader.setMat4("u_mvp", viewProj);
    voxelShader.setVec3("u_lightDir", lightDir.x, lightDir.y, lightDir.z);
    voxelShader.setFloat("u_ambient", 0.25f); // Ambient term (dark background)
    // Bind texture
    atlas.bind();
    glActiveTexture(GL_TEXTURE0);
    voxelShader.setInt("u_tex", 0);
    // Draw all chunk meshes
    GLuint prog = voxelShader.getProgram();
    int rendered = 0;
    for (auto& glmesh : glMeshes) {
        if (glmesh.indexCount == 0) continue;
        glmesh.setupAttributes(prog);
        glmesh.draw();
        rendered++;
    }
   // std::cerr << "[AsteroidRender] Rendered " << rendered << " chunk meshes." << std::endl;
    voxelShader.unbind();
}
