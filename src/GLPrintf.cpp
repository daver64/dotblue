// platform_win32.cpp
#define STB_TRUETYPE_IMPLEMENTATION
#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"
#ifdef _WIN32

#include <windows.h>
#include <gl/GL.h>
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
namespace DotBlue
{

    GLFont LoadFont(const char *fontPath, float pixelHeight)
    {
        GLFont font = {};

        std::ifstream file(fontPath, std::ios::binary);
        std::vector<unsigned char> ttfBuffer((std::istreambuf_iterator<char>(file)), {});

        if (ttfBuffer.empty()) {
            std::cerr << "Failed to load font file: " << fontPath << std::endl;
            return font; // Return empty font, nothing will render
        }

        const int texWidth = 512, texHeight = 512;
        std::vector<unsigned char> bitmap(texWidth * texHeight);

        int bakeResult = stbtt_BakeFontBitmap(
            ttfBuffer.data(), 0, pixelHeight,
            bitmap.data(), texWidth, texHeight,
            32, 96, font.cdata);

        if (bakeResult <= 0) {
            std::cerr << "stbtt_BakeFontBitmap failed for font: " << fontPath << std::endl;
            return font;
        }

        font.width = texWidth;
        font.height = texHeight;

        glGenTextures(1, &font.textureID);
        glBindTexture(GL_TEXTURE_2D, font.textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texWidth, texHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        std::cout << "Loaded font from " << fontPath << std::endl;
        std::cout << "Font texture ID: " << font.textureID << std::endl;
        std::cout << "Bitmap sample: " << (int)bitmap[0] << ", " << (int)bitmap[1] << std::endl;
        std::cout << "Font buffer size: " << ttfBuffer.size() << std::endl;
        std::cout << "Bake result: " << bakeResult << std::endl;
        return font;
    }

    void GLPrintf(const GLFont &font, float x, float y,
                  const RGBA &color, const char *fmt, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, font.textureID);
        glColor4f(color.r, color.g, color.b, color.a);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glBegin(GL_QUADS);
        const char *text = buffer;
        while (*text)
        {
            if (*text >= 32 && *text < 128)
            {
                stbtt_aligned_quad q;
                stbtt_GetBakedQuad(font.cdata, font.width, font.height, *text - 32, &x, &y, &q, 1);
                glTexCoord2f(q.s0, q.t0);
                glVertex2f(q.x0, q.y0);
                glTexCoord2f(q.s1, q.t0);
                glVertex2f(q.x1, q.y0);
                glTexCoord2f(q.s1, q.t1);
                glVertex2f(q.x1, q.y1);
                glTexCoord2f(q.s0, q.t1);
                glVertex2f(q.x0, q.y1);
            }
            ++text;
        }
        glEnd();
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) std::cout << "OpenGL error: " << err << std::endl;
    }

    void PrintOpenGLInfo()
    {
        std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    }
}

