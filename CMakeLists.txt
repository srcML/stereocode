# @file CMakeLists.txt
# 
# CMake files for the Stereocode project

# Can be run by any version of CMake >= this
cmake_minimum_required(VERSION 3.18)

# project info
project(Stereocode)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Use Release build type if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# stereocode application
add_executable(stereocode Stereocode.cpp)

# assumes srcml.h may be in this directory
target_include_directories(stereocode PRIVATE /usr/local/include)

# Download CLI11 into directory named external
set(CMAKE_EXTERNAL_SOURCE_DIR ${CMAKE_BINARY_DIR}/external)
file(DOWNLOAD https://github.com/CLIUtils/CLI11/releases/download/v1.9.1/CLI11.hpp ${CMAKE_EXTERNAL_SOURCE_DIR}/CLI11.hpp)
target_include_directories(stereocode PRIVATE ${CMAKE_EXTERNAL_SOURCE_DIR})

# libsrcml
target_link_directories(stereocode PRIVATE /usr/local/lib)
target_link_libraries(stereocode libsrcml.a)

# Turn on warnings
add_compile_options(
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-pedantic>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-ansi>

    $<$<CXX_COMPILER_ID:MSVC>:/Wall>
)

# Hello run command
add_custom_target(run
        COMMENT "Run hello"
        COMMAND ./hello
        DEPENDS hello
        USES_TERMINAL
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
