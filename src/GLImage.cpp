#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"
#ifdef _WIN32

#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <atomic>
#include <iostream>
#undef UNICODE
#undef _UNICODE
#elif defined(__linux__) || defined(__FreeBSD__)
#include <GL/glew.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <unistd.h>
#include <atomic>
#include <iostream>
#endif

#include <vector>
#include <fstream>
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <utility>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "DotBlue/stb_image.h"
namespace DotBlue {
unsigned int LoadPNGTexture(const std::string &filename)
{
    int width, height, channels;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 4); // Force RGBA
    if (!data)
    {
        std::cerr << "Failed to load PNG: " << filename << std::endl;
        return 0;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
    std::cout << "Texture id of " << filename << ": " << texID << std::endl;
    return texID;
}
void TexturedQuad(unsigned int textureID, float x0, float y0, float x1, float y1)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x0, y0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x1, y0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x1, y1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x0, y1);
    glEnd();
}

void TexturedTriangle(unsigned int textureID,
                      float x0, float y0, float u0, float v0,
                      float x1, float y1, float u1, float v1,
                      float x2, float y2, float u2, float v2)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBegin(GL_TRIANGLES);
    glTexCoord2f(u0, v0);
    glVertex2f(x0, y0);
    glTexCoord2f(u1, v1);
    glVertex2f(x1, y1);
    glTexCoord2f(u2, v2);
    glVertex2f(x2, y2);
    glEnd();
}

void GLDisableTextureFiltering(unsigned int textureID) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void GLEnableTextureFiltering(unsigned int textureID) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void GLLine(float x0, float y0, float x1, float y1) {
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();
}

void GLTriangle(float x0, float y0, float x1, float y1, float x2, float y2) {
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void GLRectangle(float x0, float y0, float x1, float y1) {
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

// Modern shader-compatible versions with efficient buffer reuse
static unsigned int lineVAO = 0, lineVBO = 0;
static unsigned int triangleVAO = 0, triangleVBO = 0;  
static unsigned int quadVAO = 0, quadVBO = 0, quadEBO = 0;
static unsigned int texturedQuadVAO = 0, texturedQuadVBO = 0, texturedQuadEBO = 0;
static unsigned int texturedTriangleVAO = 0, texturedTriangleVBO = 0;

static void initLineBuffers() {
    if (lineVAO == 0) {
        glGenVertexArrays(1, &lineVAO);
        glGenBuffers(1, &lineVBO);
        
        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
}

static void initTriangleBuffers() {
    if (triangleVAO == 0) {
        glGenVertexArrays(1, &triangleVAO);
        glGenBuffers(1, &triangleVBO);
        
        glBindVertexArray(triangleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
        glBufferData(GL_ARRAY_BUFFER, 3 * 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
}

static void initQuadBuffers() {
    if (quadVAO == 0) {
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);
        
        unsigned int indices[] = {
            0, 1, 2,  // First triangle
            2, 3, 0   // Second triangle
        };
        
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
}

static void initTexturedQuadBuffers() {
    if (texturedQuadVAO == 0) {
        glGenVertexArrays(1, &texturedQuadVAO);
        glGenBuffers(1, &texturedQuadVBO);
        glGenBuffers(1, &texturedQuadEBO);
        
        unsigned int indices[] = {
            0, 1, 2,  // First triangle
            2, 3, 0   // Second triangle
        };
        
        glBindVertexArray(texturedQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, texturedQuadVBO);
        // Vertex format: x, y, z, u, v (position + texture coords)
        glBufferData(GL_ARRAY_BUFFER, 4 * 5 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texturedQuadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        // Position attribute (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Texture coordinate attribute (location = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
}

static void initTexturedTriangleBuffers() {
    if (texturedTriangleVAO == 0) {
        glGenVertexArrays(1, &texturedTriangleVAO);
        glGenBuffers(1, &texturedTriangleVBO);
        
        glBindVertexArray(texturedTriangleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, texturedTriangleVBO);
        // Vertex format: x, y, z, u, v (position + texture coords)
        glBufferData(GL_ARRAY_BUFFER, 3 * 5 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        
        // Position attribute (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Texture coordinate attribute (location = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
}

void GLLineShader(float x0, float y0, float x1, float y1, float r, float g, float b) {
    initLineBuffers();
    
    float vertices[] = {
        x0, y0, 0.0f, r, g, b,
        x1, y1, 0.0f, r, g, b
    };

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_LINES, 0, 2);
}

void GLTriangleShader(float x0, float y0, float x1, float y1, float x2, float y2, float r, float g, float b) {
    initTriangleBuffers();
    
    float vertices[] = {
        x0, y0, 0.0f, r, g, b,
        x1, y1, 0.0f, r, g, b,
        x2, y2, 0.0f, r, g, b
    };

    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLRectangleShader(float x0, float y0, float x1, float y1, float r, float g, float b) {
    initQuadBuffers();
    
    float vertices[] = {
        x0, y0, 0.0f, r, g, b,  // Bottom-left
        x1, y0, 0.0f, r, g, b,  // Bottom-right
        x1, y1, 0.0f, r, g, b,  // Top-right
        x0, y1, 0.0f, r, g, b   // Top-left
    };

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// Modern textured drawing functions
void TexturedQuadShader(unsigned int textureID, float x0, float y0, float x1, float y1) {
    initTexturedQuadBuffers();
    
    float vertices[] = {
        x0, y0, 0.0f, 0.0f, 0.0f,  // Bottom-left: pos(x,y,z) + tex(u,v)
        x1, y0, 0.0f, 1.0f, 0.0f,  // Bottom-right
        x1, y1, 0.0f, 1.0f, 1.0f,  // Top-right
        x0, y1, 0.0f, 0.0f, 1.0f   // Top-left
    };

    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(texturedQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, texturedQuadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void TexturedTriangleShader(unsigned int textureID,
                           float x0, float y0, float u0, float v0,
                           float x1, float y1, float u1, float v1,
                           float x2, float y2, float u2, float v2) {
    initTexturedTriangleBuffers();
    
    float vertices[] = {
        x0, y0, 0.0f, u0, v0,
        x1, y1, 0.0f, u1, v1,
        x2, y2, 0.0f, u2, v2
    };

    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(texturedTriangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, texturedTriangleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
}