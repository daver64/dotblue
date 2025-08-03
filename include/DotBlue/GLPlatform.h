#pragma once
#include <atomic>
#include <string>
#include "stb_truetype.h"
#include "stb_image.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
namespace DotBlue
{
    struct GLFont
    {
        unsigned int textureID;
        int width, height;
        stbtt_bakedchar cdata[96]; // ASCII 32..126
    };
    struct RGBA
    {
        float r, g, b, a;
    };
    void InitApp();
    void RunWindow(std::atomic<bool> &running);
    void UpdateAndRender();
    void GLSwapBuffers();
    void GLSleep(int ms);
    GLFont LoadFont(const char *fontPath, float pixelHeight = 14.0f);
    void GLPrintf(const GLFont &font, float x, float y, const RGBA &color, const char *fmt, ...);
    float GetCharHeight(const GLFont &font, char c);
    float GetCharWidth(const GLFont &font, char c);
    void SetApplicationTitle(const std::string &title);
    unsigned int LoadPNGTexture(const std::string &filename);
    void TexturedQuad(unsigned int textureID, float x0, float y0, float x1, float y1);
    void TexturedTriangle(unsigned int textureID,
                          float x0, float y0, float u0, float v0,
                          float x1, float y1, float u1, float v1,
                          float x2, float y2, float u2, float v2);

} // namespace DotBlue