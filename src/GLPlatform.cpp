#include "DotBlue/DotBlue.h"
#include "DotBlue/GLPlatform.h"
#if defined(__linux__) || defined(__FreeBSD__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glxext.h>
const char* default_font_str="/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
#elif defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
const char* default_font_str="C:/Windows/Fonts/consola.ttf";
extern HDC glapp_hdc;
#endif
#include <DotBlue/GLPlatform.h>
#include <chrono>
#include <string>
#include <iostream>

namespace DotBlue
{
    
    std::string gTimingInfo;
    GLFont glapp_default_font = {};
    unsigned int texid=0;
    DotBlue::GLTextureAtlas* glapp_texture_atlas = nullptr;
    void InitApp()
    {
        glapp_default_font = LoadFont(default_font_str);
        SetApplicationTitle("DotBlueTheBlue!!");
        texid=DotBlue::LoadPNGTexture("../bud.png");
        glapp_texture_atlas = new DotBlue::GLTextureAtlas("../mc.png", 16, 16);
        GLDisableTextureFiltering(glapp_texture_atlas->getTextureID());
    }
    void UpdateAndRender()
    {
     //   auto start = std::chrono::high_resolution_clock::now();
        RGBA red{1.0,0.0,0.0,1.0};
        RGBA green{0.0,1.0,0.0,1.0};
        RGBA blue{0.0,0.0,1.0,1.0};
        RGBA white{1.0,1.0,1.0,1.0};
        int width = 800, height = 600; // You may want to make these dynamic

        // Set up orthographic projection for 2D text rendering
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1); // Top-left is (0,0)
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(white.r, white.g, white.b, white.a);
        TexturedQuad(texid, 100, 50, 300, 300);

        // Draw a quad from the texture atlas
        
        glapp_texture_atlas->select(16);
        glapp_texture_atlas->bind();
        glapp_texture_atlas->draw_quad(400, 50, 128, 128);
        //GLEnableTextureFiltering(glapp_texture_atlas->getTextureID());
        // Now render text at pixel coordinates
        GLPrintf(glapp_default_font, 100, 100, green, "Hello DotBlue World!");
        GLSwapBuffers();
  
       // auto end = std::chrono::high_resolution_clock::now();
      //  std::chrono::duration<double, std::milli> elapsed = end - start;

       // static int counter=0;
      //  counter++;
       // if (counter % 60 == 0) { // Update timing info every 60 frames
       //     gTimingInfo = "Frame time: " + std::to_string(static_cast<int>(elapsed.count())) + " ms";
       // }

    }
}