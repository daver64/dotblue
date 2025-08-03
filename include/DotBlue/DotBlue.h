#pragma once

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
#include "stb_truetype.h"
namespace DotBlue {

struct GLFont {
    unsigned int textureID;
    int width, height;
    stbtt_bakedchar cdata[96]; // ASCII 32..126
};

    DOTBLUE_API void Hello();
    DOTBLUE_API int DB_Test();
    DOTBLUE_API int Console_Test();
    DOTBLUE_API int GL_Test();
}
