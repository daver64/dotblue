cmake_minimum_required(VERSION 3.12)
project(SpaceGame VERSION 1.0 LANGUAGES CXX)

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
    src/GameBase.cpp
)

# ImGui source files
set(IMGUI_SOURCES
    ../../src/imgui.cpp
    ../../src/imgui_draw.cpp
    ../../src/imgui_tables.cpp
    ../../src/imgui_widgets.cpp
    ../../src/imgui_demo.cpp
    ../../src/backends/imgui_impl_opengl3.cpp
)

# Add platform-specific ImGui backend
if(WIN32)
    list(APPEND IMGUI_SOURCES ../../src/backends/imgui_impl_win32.cpp)
elseif(UNIX AND NOT APPLE)
    list(APPEND IMGUI_SOURCES ../../src/backends/imgui_impl_sdl2.cpp)
endif()

# Create the game executable
add_executable(SpaceGame ${GAME_SOURCES} ${IMGUI_SOURCES})

# Include DotBlue headers
target_include_directories(SpaceGame PRIVATE ${DOTBLUE_DIR}/include)

# Include ImGui headers
target_include_directories(SpaceGame PRIVATE ${DOTBLUE_DIR}/src)

# Include GLM headers (Windows only)
if(WIN32)
    target_include_directories(SpaceGame PRIVATE ${GLM_ROOT})
    target_include_directories(SpaceGame PRIVATE ${SDL2_ROOT}/include)
    target_include_directories(SpaceGame PRIVATE ${SDL2_MIXER_ROOT}/include)
    
    # Add GLEW include directory
    target_include_directories(SpaceGame PRIVATE "C:/Users/daver/LocalApps/glew-2.1.0/include")
endif()

# Link with DotBlue library directly
if(WIN32)
    target_link_libraries(SpaceGame PRIVATE 
        ${DOTBLUE_BUILD_DIR}/DotBlue.lib
    )
else()
    target_link_libraries(SpaceGame PRIVATE 
        ${DOTBLUE_BUILD_DIR}/libDotBlue.so
    )
endif()

# Add SDL2 libraries on Windows
if(WIN32)
    target_link_libraries(SpaceGame PRIVATE 
        ${SDL2_ROOT}/lib/x64/SDL2.lib
        ${SDL2_ROOT}/lib/x64/SDL2main.lib
        "C:/Users/daver/LocalApps/glew-2.1.0/lib/Release/x64/glew32.lib"
        opengl32.lib
    )
else()
    # On Linux, link with system SDL2 libraries
    target_link_libraries(SpaceGame PRIVATE 
        SDL2
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
    add_custom_command(TARGET SpaceGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DOTBLUE_BUILD_DIR}/DotBlue.dll"
        $<TARGET_FILE_DIR:SpaceGame>)
        
    # Copy other required DLLs from their source locations
    add_custom_command(TARGET SpaceGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/Users/daver/LocalApps/glew-2.1.0/bin/Release/x64/glew32.dll"
        "C:/Users/daver/LocalApps/SDL2-2.32.6/VisualC/x64/Release/SDL2.dll"
        "C:/Users/daver/LocalApps/SDL2_mixer-2.8.1/lib/x64/SDL2_mixer.dll"
        $<TARGET_FILE_DIR:SpaceGame>)
endif()

# Set working directory for Visual Studio
set_property(TARGET SpaceGame PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
