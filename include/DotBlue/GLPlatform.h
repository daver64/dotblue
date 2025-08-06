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

    class GLShader
    {
    public:
        GLShader();
        ~GLShader();

        bool load(const std::string &vertexSrc, const std::string &fragmentSrc);
        bool loadFromFiles(const std::string &vertexPath, const std::string &fragmentPath);
        void bind() const;
        void unbind() const;
        unsigned int getProgram() const { return programID; }

    private:
        unsigned int programID;
        unsigned int compileShader(unsigned int type, const std::string &src);
        void deleteProgram();
    };

    class GLTextureAtlas
    {
    public:
        GLTextureAtlas(const std::string &pngPath, int imgWidth, int imgHeight);
        ~GLTextureAtlas();

        void select(int index);                                   // Select image by index (0-based, left-to-right, top-to-bottom)
        void bind() const;                                        // Bind the atlas texture
        void draw_quad(float x, float y, float w, float h) const; // Draw selected image at (x, y) with size (w, h)

        int getImageCount() const { return rows * cols; }
        unsigned int getTextureID() const { return textureID; } 
    private:
        unsigned int textureID;
        int atlasWidth, atlasHeight;
        int imgWidth, imgHeight;
        int rows, cols;
        int selectedIndex;
        float u0, v0, u1, v1; // UVs for selected image
    };

    void InitApp();
    void ShutdownApp();
    void RunWindow(std::atomic<bool> &running);
    void UpdateAndRender();
    void HandleInput();
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
    void GLDisableTextureFiltering(unsigned int textureID);
    void GLEnableTextureFiltering(unsigned int textureID);

} // namespace DotBlue