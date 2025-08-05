cmake_minimum_required(VERSION 3.12)
project(DotBlue VERSION 1.0 LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_STATIC_LIB "Build DotBlue as a static library" OFF)
option(BUILD_TESTS "Build test executable" ON)

# Determine lib type
if(BUILD_STATIC_LIB)
    set(LIB_TYPE STATIC)
else()
    set(LIB_TYPE SHARED)
endif()

# Define the library
add_library(DotBlue ${LIB_TYPE}
    src/DotBlue.cpp
    src/MemChunkAllocator.cpp
    src/GLImage.cpp
    src/GLTextureAtlas.cpp
    src/GLShader.cpp
    src/GLPlatformWin32.cpp
    src/GLPlatformUnix.cpp
    src/GLPlatform.cpp
    src/GLPrintf.cpp
)

# Export symbols (for shared)
target_compile_definitions(DotBlue PRIVATE
    $<$<BOOL:${BUILD_STATIC_LIB}>:DOTBLUE_STATIC>
    $<$<AND:$<NOT:$<BOOL:${BUILD_STATIC_LIB}>>,$<BOOL:${WIN32}>>:DOTBLUE_EXPORTS>
)

# Platform macros
target_compile_definitions(DotBlue PRIVATE
    $<$<PLATFORM_ID:Windows>:DOTBLUE_WINDOWS>
    $<$<PLATFORM_ID:Linux>:DOTBLUE_LINUX>
    $<$<PLATFORM_ID:FreeBSD>:DOTBLUE_FREEBSD>
)

# Public headers
target_include_directories(DotBlue PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Optional test app
# Optional test app
if(BUILD_TESTS)
    add_executable(DotBlueTest test/main.cpp)
    target_include_directories(DotBlueTest PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
  if(MSVC)
      target_link_libraries(DotBlueTest PRIVATE DotBlue "C:/Users/daver/LocalApps/glew-2.1.0/lib/Release/x64/glew32.lib")
        target_compile_definitions(DotBlue PRIVATE _CRT_SECURE_NO_WARNINGS)
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(DotBlueTest PRIVATE
            DotBlue
            GL
            GLEW
            X11
            SDL2
            SDL2_mixer
            pthread
            dl)
        target_link_libraries(DotBlue PRIVATE
            GL
            GLEW
            X11
            SDL2
            SDL2_mixer
            pthread
            dl)
    endif()
endif()

# Set GLEW root directory
set(GLEW_ROOT "C:/Users/daver/LocalApps/glew-2.1.0")

# Add GLEW include directory
include_directories(${GLEW_ROOT}/include)

# Use absolute path for glew32.lib
if(MSVC)
    target_link_libraries(DotBlue PRIVATE
        "C:/Users/daver/LocalApps/glew-2.1.0/lib/Release/x64/glew32.lib"
    )
endif()

set(SDL2_MIXER_ROOT "C:/Users/daver/LocalApps/SDL2_mixer-2.8.1")
include_directories(${SDL2_MIXER_ROOT}/include)

if(MSVC)
    target_link_libraries(DotBlue PRIVATE "${SDL2_MIXER_ROOT}/lib/x64/SDL2_mixer.lib")
    target_link_libraries(DotBlueTest PRIVATE DotBlue "${SDL2_MIXER_ROOT}/lib/x64/SDL2_mixer.lib")
endif()

# Set SDL2 root directory (use SDL2, not SDL3)
set(SDL2_ROOT "C:/Users/daver/LocalApps/SDL2-2.32.6")
include_directories(${SDL2_ROOT}/include)

if(MSVC)
    target_link_libraries(DotBlue PRIVATE "${SDL2_ROOT}/lib/x64/SDL2.lib")
    target_link_libraries(DotBlueTest PRIVATE DotBlue "${SDL2_ROOT}/lib/x64/SDL2.lib")
endif()
