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

set(DOTBLUE_SOURCES
    src/DotBlue.cpp
    src/MemChunkAllocator.cpp
    src/GLImage.cpp
    src/GLTextureAtlas.cpp
    src/GLShader.cpp
    src/GLMath.cpp
    src/Input.cpp
    src/GLPlatformWin32.cpp
    src/GLPlatformUnix.cpp
    src/GLPlatform.cpp
    src/GLPrintf.cpp
    src/imgui.cpp
    src/imgui_draw.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
    src/imgui_demo.cpp
    src/backends/imgui_impl_opengl3.cpp
)

if(WIN32 OR CYGWIN)
    list(APPEND DOTBLUE_SOURCES src/backends/imgui_impl_win32.cpp)
elseif(UNIX AND NOT APPLE)
    list(APPEND DOTBLUE_SOURCES src/backends/imgui_impl_sdl2.cpp)
endif()

add_library(DotBlue ${LIB_TYPE} ${DOTBLUE_SOURCES})

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
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)


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


# Use absolute path for glew32.lib
if(MSVC)
    # Set GLEW root directory
    set(GLEW_ROOT "C:/Users/daver/LocalApps/glew-2.1.0")

    # Add GLEW include directory
    include_directories(${GLEW_ROOT}/include)
    target_link_libraries(DotBlue PRIVATE
        "C:/Users/daver/LocalApps/glew-2.1.0/lib/Release/x64/glew32.lib"
    )
    
    # Set GLM root directory (Windows only - Linux uses system packages)
    set(GLM_ROOT "C:/Users/daver/LocalApps/glm-1.0.1")
    include_directories(${GLM_ROOT})
    
    set(SDL2_MIXER_ROOT "C:/Users/daver/LocalApps/SDL2_mixer-2.8.1")
    include_directories(${SDL2_MIXER_ROOT}/include)
    target_link_libraries(DotBlue PRIVATE "${SDL2_MIXER_ROOT}/lib/x64/SDL2_mixer.lib")
    target_link_libraries(DotBlueTest PRIVATE DotBlue "${SDL2_MIXER_ROOT}/lib/x64/SDL2_mixer.lib")
    
    set(SDL2_ROOT "C:/Users/daver/LocalApps/SDL2-2.32.6")
    include_directories(${SDL2_ROOT}/include)
    target_link_libraries(DotBlue PRIVATE "${SDL2_ROOT}/lib/x64/SDL2.lib")
    target_link_libraries(DotBlueTest PRIVATE DotBlue "${SDL2_ROOT}/lib/x64/SDL2.lib" "${SDL2_ROOT}/lib/x64/SDL2main.lib")
    
    include_directories("C:/Users/daver/source/dotblue/src")
    
    # Add GLM to test target (Windows only)
    if(BUILD_TESTS)
        target_include_directories(DotBlueTest PRIVATE "${GLM_ROOT}")
    endif()
    
elseif(UNIX AND NOT APPLE)
    # On Linux, GLM is typically installed via package manager
    # Check if GLM is available
    find_path(GLM_INCLUDE_DIR glm/glm.hpp
        PATHS /usr/include /usr/local/include
        DOC "GLM include directory")
    
    if(GLM_INCLUDE_DIR)
        message(STATUS "Found GLM: ${GLM_INCLUDE_DIR}")
        target_include_directories(DotBlue PUBLIC ${GLM_INCLUDE_DIR})
        if(BUILD_TESTS)
            target_include_directories(DotBlueTest PRIVATE ${GLM_INCLUDE_DIR})
        endif()
    else()
        message(WARNING "GLM not found. Install with: sudo apt-get install libglm-dev")
    endif()
endif()


