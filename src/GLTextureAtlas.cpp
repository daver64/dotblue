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
#include "DotBlue/stb_image.h"

namespace DotBlue
{

    GLTextureAtlas::GLTextureAtlas(const std::string &pngPath, int imgW, int imgH)
        : textureID(0), imgWidth(imgW), imgHeight(imgH), selectedIndex(0)
    {
        // Load PNG and get atlas size
        textureID = LoadPNGTexture(pngPath);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &atlasWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &atlasHeight);

        cols = atlasWidth / imgWidth;
        rows = atlasHeight / imgHeight;
        printf("[GLTextureAtlas] atlasWidth=%d atlasHeight=%d imgWidth=%d imgHeight=%d cols=%d rows=%d\n",
            atlasWidth, atlasHeight, imgWidth, imgHeight, cols, rows);
        select(0); // Default to first image
    }

    GLTextureAtlas::~GLTextureAtlas()
    {
        if (textureID)
            glDeleteTextures(1, &textureID);
    }

    void GLTextureAtlas::select(int index)
    {
    selectedIndex = index;
    int col = index % cols;
    int row = index / cols;
    u0 = (float)(col * imgWidth) / atlasWidth;
    v0 = (float)(row * imgHeight) / atlasHeight;
    u1 = (float)((col + 1) * imgWidth) / atlasWidth;
    v1 = (float)((row + 1) * imgHeight) / atlasHeight;
    }

    void GLTextureAtlas::bind() const
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
    }

    void GLTextureAtlas::draw_quad(float x, float y, float w, float h) const
    {
        // Use modern shader-based rendering instead of legacy immediate mode
        TexturedQuadShaderUV(textureID, x, y, x + w, y + h, u0, v0, u1, v1);
    }

}