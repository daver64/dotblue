cmake_minimum_required(VERSION 3.12)
project(Kosmos VERSION 1.0 LANGUAGES CXX)

# Use C++17 (same as DotBlue)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find DotBlue library
# Use absolute path to DotBlue build directory
if(WIN32)
    set(DOTBLUE_DIR "C:/Users/daver/source/dotblue" CACHE PATH "Path to DotBlue source")
else()
    set(DOTBLUE_DIR "/home/dave/source/dotblue" CACHE PATH "Path to DotBlue source")
endif()
set(DOTBLUE_BUILD_DIR "${DOTBLUE_DIR}/build" CACHE PATH "Path to DotBlue build")

# Set GLM path (Windows only)
if(WIN32)
    set(GLM_ROOT "C:/Users/daver/LocalApps/glm-1.0.1")
    set(SDL2_ROOT "C:/Users/daver/LocalApps/SDL2-2.32.6")
    set(SDL2_MIXER_ROOT "C:/Users/daver/LocalApps/SDL2_mixer-2.8.1")
endif()

# Game source files
set(GAME_SOURCES
    src/main.cpp
    src/KosmosBase.cpp
    src/IniFile.cpp
    src/FileSystem.cpp
    src/Perlin.cpp
    src/Asteroid.cpp
    src/AsteroidRender.cpp
)

# Set ImGui path - use local copy for cross-platform consistency
if(WIN32)
    set(IMGUI_DIR "C:/Users/daver/LocalApps/imgui" CACHE PATH "Path to ImGui")
else()
    set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../imgui" CACHE PATH "Path to ImGui")
endif()

# ImGui source files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Add platform-specific ImGui backend
if(WIN32)
    list(APPEND IMGUI_SOURCES ${IMGUI_DIR}/backends/imgui_impl_win32.cpp)
endif()

# Create executable
add_executable(Kosmos ${GAME_SOURCES} ${IMGUI_SOURCES})

# Include DotBlue headers
target_include_directories(Kosmos PRIVATE ${DOTBLUE_DIR}/include)

# Include ImGui headers
target_include_directories(Kosmos PRIVATE ${IMGUI_DIR})
target_include_directories(Kosmos PRIVATE ${IMGUI_DIR}/backends)


# Include SDL2 headers (platform-specific)
if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    target_include_directories(Kosmos PRIVATE ${SDL2_INCLUDE_DIRS})
else()
    target_include_directories(Kosmos PRIVATE ${GLM_ROOT})
    target_include_directories(Kosmos PRIVATE ${SDL2_ROOT}/include)
    target_include_directories(Kosmos PRIVATE ${SDL2_MIXER_ROOT}/include)
    # Add GLEW include directory
    target_include_directories(Kosmos PRIVATE "C:/Users/daver/LocalApps/glew-2.1.0/include")
endif()

# Link with libraries
if(WIN32)
    target_link_libraries(Kosmos PRIVATE 
        ${DOTBLUE_BUILD_DIR}/DotBlue.lib
        ${SDL2_ROOT}/lib/x64/SDL2.lib
        ${SDL2_ROOT}/lib/x64/SDL2main.lib
        "C:/Users/daver/LocalApps/glew-2.1.0/lib/Release/x64/glew32.lib"
        opengl32.lib
    )
else()
    # On Linux, link with system SDL2 libraries and local ImGui
    target_link_libraries(Kosmos PRIVATE 
        ${DOTBLUE_BUILD_DIR}/libDotBlue.so
        ${SDL2_LIBRARIES}
        SDL2_mixer
        GL
        GLEW
        X11
    )
endif()

# Copy assets to build directory (only if assets directory exists)
if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    file(COPY assets/ DESTINATION ${CMAKE_BINARY_DIR}/assets/)
endif()

# Copy shaders to build directory
if(EXISTS "${CMAKE_SOURCE_DIR}/shaders")
    file(COPY shaders/ DESTINATION ${CMAKE_BINARY_DIR}/shaders/)
endif()

# Platform-specific settings
if(WIN32)
    # Copy DotBlue DLL to output directory on Windows
    add_custom_command(TARGET Kosmos POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DOTBLUE_BUILD_DIR}/DotBlue.dll"
        $<TARGET_FILE_DIR:Kosmos>)
        
    # Copy other required DLLs from their source locations
    add_custom_command(TARGET Kosmos POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Users/daver/LocalApps/glew-2.1.0/bin/Release/x64/glew32.dll"
        "C:/Users/daver/LocalApps/SDL2-2.32.6/VisualC/x64/Release/SDL2.dll"
        "C:/Users/daver/LocalApps/SDL2_mixer-2.8.1/lib/x64/SDL2_mixer.dll"
        $<TARGET_FILE_DIR:Kosmos>)
endif()

# Set working directory for Visual Studio
set_property(TARGET Kosmos PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
