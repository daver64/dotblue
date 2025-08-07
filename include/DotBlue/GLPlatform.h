#pragma once
#include <atomic>
#include <string>
#include "stb_truetype.h"
#include "stb_image.h"
#if defined(__linux__) || defined(__FreeBSD__)
#include <SDL2/SDL.h>
#endif
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
        
        // Uniform setters
        void setFloat(const std::string &name, float value) const;
        void setVec2(const std::string &name, float x, float y) const;
        void setVec3(const std::string &name, float x, float y, float z) const;
        void setInt(const std::string &name, int value) const;

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
        
        // Get UV coordinates for selected image
        void getSelectedUVs(float& u0_out, float& v0_out, float& u1_out, float& v1_out) const {
            u0_out = u0; v0_out = v0; u1_out = u1; v1_out = v1;
        } 
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

    #if defined(linux) || defined(__FreeBSD__)
    void HandleInput(SDL_Window* window);
    #endif
    void HandleInput();
    void GLSwapBuffers();
    void GLSleep(int ms);
    GLFont LoadFont(const char *fontPath, float pixelHeight = 14.0f);
    void GLPrintf(const GLFont &font, float x, float y, const RGBA &color, const char *fmt, ...);
    float GetCharHeight(const GLFont &font, char c);
    float GetCharWidth(const GLFont &font, char c);
    void SetApplicationTitle(const std::string &title);
    unsigned int LoadPNGTexture(const std::string &filename);
    void GLDisableTextureFiltering(unsigned int textureID);
    void GLEnableTextureFiltering(unsigned int textureID);

    // Modern shader-compatible drawing functions
    void GLLineShader(float x0, float y0, float x1, float y1, float r, float g, float b);
    void GLTriangleShader(float x0, float y0, float x1, float y1, float x2, float y2, float r, float g, float b);
    void GLRectangleShader(float x0, float y0, float x1, float y1, float r, float g, float b);    // Modern textured drawing functions
    void TexturedQuadShader(unsigned int textureID, float x0, float y0, float x1, float y1);
    void TexturedQuadShaderUV(unsigned int textureID, float x0, float y0, float x1, float y1,
                             float u0, float v0, float u1, float v1);
    void TexturedTriangleShader(unsigned int textureID,
                               float x0, float y0, float u0, float v0,
                               float x1, float y1, float u1, float v1,
                               float x2, float y2, float u2, float v2);

} // namespace DotBlue