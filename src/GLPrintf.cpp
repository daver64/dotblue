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
#else if defined(__linux__) || defined(__FreeBSD__)
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
namespace DotBlue {


GLFont LoadFont(const char* fontPath, float pixelHeight = 24.0f) {
    GLFont font = {};

    std::ifstream file(fontPath, std::ios::binary);
    std::vector<unsigned char> ttfBuffer((std::istreambuf_iterator<char>(file)), {});

    const int texWidth = 512, texHeight = 512;
    std::vector<unsigned char> bitmap(texWidth * texHeight);

    stbtt_BakeFontBitmap(
        ttfBuffer.data(), 0, pixelHeight,
        bitmap.data(), texWidth, texHeight,
        32, 96, font.cdata
    );

    font.width = texWidth;
    font.height = texHeight;

    glGenTextures(1, &font.textureID);
    glBindTexture(GL_TEXTURE_2D, font.textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texWidth, texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return font;
}

void GLPrintf(const GLFont& font, float x, float y, const char* text, float r, float g, float b) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font.textureID);
    glColor3f(r, g, b);

    glBegin(GL_QUADS);
    while (*text) {
        if (*text >= 32 && *text < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font.cdata, font.width, font.height, *text - 32, &x, &y, &q, 1);
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
        }
        ++text;
    }
    glEnd();
}
}