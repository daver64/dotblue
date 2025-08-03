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
if(BUILD_TESTS)
    add_executable(DotBlueTest test/main.cpp)
    target_link_libraries(DotBlueTest PRIVATE DotBlue)
endif()
