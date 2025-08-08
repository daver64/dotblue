cmake_minimum_required(VERSION 3.12)
project(PlatformerGame VERSION 1.0 LANGUAGES CXX)

# Use C++17 (same as DotBlue)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find DotBlue library
# Adjust this path to point to your DotBlue installation
set(DotBlue_DIR "../dotblue" CACHE PATH "Path to DotBlue library")
find_package(DotBlue REQUIRED PATHS ${DotBlue_DIR})

# Game source files
set(GAME_SOURCES
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/Level.cpp
    src/GameLogic.cpp
)

# Create the game executable
add_executable(PlatformerGame ${GAME_SOURCES})

# Link with DotBlue
target_link_libraries(PlatformerGame PRIVATE DotBlue::DotBlue)

# Include DotBlue headers
target_include_directories(PlatformerGame PRIVATE ${DOTBLUE_INCLUDE_DIRS})

# Copy assets to build directory
file(COPY assets/ DESTINATION ${CMAKE_BINARY_DIR}/assets/)

# Platform-specific settings
if(WIN32)
    # Copy DLLs to output directory on Windows
    add_custom_command(TARGET PlatformerGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DotBlue_DIR}/build/Release/DotBlue.dll"
        $<TARGET_FILE_DIR:PlatformerGame>)
        
    # Copy other required DLLs (you might want to automate this)
    add_custom_command(TARGET PlatformerGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DotBlue_DIR}/build/Release/glew32.dll"
        "${DotBlue_DIR}/build/Release/SDL2.dll"
        "${DotBlue_DIR}/build/Release/SDL2_mixer.dll"
        $<TARGET_FILE_DIR:PlatformerGame>)
endif()

# Set working directory for Visual Studio
set_property(TARGET PlatformerGame PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
