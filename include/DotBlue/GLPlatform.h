#pragma once
#include <atomic>
#include <string>
#include "stb_truetype.h"
#include "stb_image.h"

// Include DOTBLUE_API definition
#ifndef DOTBLUE_API
#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef DOTBLUE_STATIC
#define DOTBLUE_API
#elif defined(DOTBLUE_EXPORTS)
#define DOTBLUE_API __declspec(dllexport)
#else
#define DOTBLUE_API __declspec(dllimport)
#endif
#else
#ifdef DOTBLUE_STATIC
#define DOTBLUE_API
#else
#define DOTBLUE_API __attribute__((visibility("default")))
#endif
#endif
#endif

// GLM Math Library
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#if defined(__linux__) || defined(__FreeBSD__)
#include <SDL2/SDL.h>
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
namespace DotBlue
{
    // GLM type aliases for convenience
    using Vec2 = glm::vec2;
    using Vec3 = glm::vec3;
    using Vec4 = glm::vec4;
    using Mat3 = glm::mat3;
    using Mat4 = glm::mat4;
    using Quat = glm::quat;
    
    struct GLFont
    {
        unsigned int textureID;
        int width, height;
        stbtt_bakedchar cdata[96]; // ASCII 32..126
    };
    struct RGBA
    {
        float r, g, b, a;
        
        // Constructor for convenience
        RGBA(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
        
        // Convert to Vec4
        Vec4 toVec4() const { return Vec4(r, g, b, a); }
        Vec3 toVec3() const { return Vec3(r, g, b); }
    };

    class GLShader
    {
    public:
        DOTBLUE_API GLShader();
        DOTBLUE_API ~GLShader();

        DOTBLUE_API bool load(const std::string &vertexSrc, const std::string &fragmentSrc);
        DOTBLUE_API bool loadFromFiles(const std::string &vertexPath, const std::string &fragmentPath);
        DOTBLUE_API void bind() const;
        DOTBLUE_API void unbind() const;
        unsigned int getProgram() const { return programID; }
        
        // Uniform setters
        DOTBLUE_API void setFloat(const std::string &name, float value) const;
        DOTBLUE_API void setVec2(const std::string &name, float x, float y) const;
        DOTBLUE_API void setVec3(const std::string &name, float x, float y, float z) const;
        DOTBLUE_API void setInt(const std::string &name, int value) const;
        
        // GLM-friendly uniform setters
        DOTBLUE_API void setVec2(const std::string &name, const Vec2 &value) const;
        DOTBLUE_API void setVec3(const std::string &name, const Vec3 &value) const;
        DOTBLUE_API void setVec4(const std::string &name, const Vec4 &value) const;
        DOTBLUE_API void setMat3(const std::string &name, const Mat3 &matrix) const;
        DOTBLUE_API void setMat4(const std::string &name, const Mat4 &matrix) const;

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
    DOTBLUE_API void GLSleep(int ms);
    DOTBLUE_API GLFont LoadFont(const char *fontPath, float pixelHeight = 14.0f);
    DOTBLUE_API void GLPrintf(const GLFont &font, float x, float y, const RGBA &color, const char *fmt, ...);
    DOTBLUE_API float GetCharHeight(const GLFont &font, char c);
    DOTBLUE_API float GetCharWidth(const GLFont &font, char c);
    DOTBLUE_API void SetApplicationTitle(const std::string &title);
    DOTBLUE_API unsigned int LoadPNGTexture(const std::string &filename);
    DOTBLUE_API void GLDisableTextureFiltering(unsigned int textureID);
    DOTBLUE_API void GLEnableTextureFiltering(unsigned int textureID);

    // Modern shader-compatible drawing functions
    DOTBLUE_API void GLLineShader(float x0, float y0, float x1, float y1, float r, float g, float b);
    DOTBLUE_API void GLTriangleShader(float x0, float y0, float x1, float y1, float x2, float y2, float r, float g, float b);
    DOTBLUE_API void GLRectangleShader(float x0, float y0, float x1, float y1, float r, float g, float b);    // Modern textured drawing functions
    DOTBLUE_API void TexturedQuadShader(unsigned int textureID, float x0, float y0, float x1, float y1);
    DOTBLUE_API void TexturedQuadShaderUV(unsigned int textureID, float x0, float y0, float x1, float y1,
                             float u0, float v0, float u1, float v1);
    DOTBLUE_API void TexturedTriangleShader(unsigned int textureID,
                               float x0, float y0, float u0, float v0,
                               float x1, float y1, float u1, float v1,
                               float x2, float y2, float u2, float v2);

    // 3D Math convenience functions
    namespace Math {
        // Create common transformation matrices
        Mat4 perspective(float fov, float aspect, float near, float far);
        Mat4 ortho(float left, float right, float bottom, float top, float near, float far);
        Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
        
        // Transform operations
        Mat4 translate(const Mat4& matrix, const Vec3& translation);
        Mat4 rotate(const Mat4& matrix, float angle, const Vec3& axis);
        Mat4 scale(const Mat4& matrix, const Vec3& scaling);
        
        // Common constants
        const float PI = 3.14159265359f;
        const float DEG_TO_RAD = PI / 180.0f;
        const float RAD_TO_DEG = 180.0f / PI;
        
        // Utility functions
        float radians(float degrees);
        float degrees(float radians);
    }
    
    // Forward declarations
    class InputManager;
    class InputBindings;
    
    // Game callback access (for internal use)
    bool CallGameInit();
    void CallGameUpdate(float deltaTime);
    void CallGameRender();
    void CallGameShutdown();
    void CallGameInput(const InputManager& input, const InputBindings& bindings);
    
    // Window message callback system (for games that need to handle window messages like ImGui)
#ifdef _WIN32
    typedef long (*WindowMessageCallback)(void* hwnd, unsigned int uMsg, unsigned long long wParam, long long lParam);
    DOTBLUE_API void SetWindowMessageCallback(WindowMessageCallback callback);
    DOTBLUE_API void* GetWindowHandle();
#endif

    // X11 event callback system (for Linux games that need to handle X11 events like ImGui)
#if defined(__linux__) || defined(__FreeBSD__)
    typedef void (*X11EventCallback)(void* xevent);
    DOTBLUE_API void SetX11EventCallback(X11EventCallback callback);
#endif

} // namespace DotBlue