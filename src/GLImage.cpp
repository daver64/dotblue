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
}