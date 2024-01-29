# @file CMakeLists.txt
# CMake files for the Stereocode project

# Supports any version of CMake >= specified.
cmake_minimum_required(VERSION 3.17)

# Project information.
project(Stereocode LANGUAGES CXX)

# C++17 standard (Not enforced).
set(CMAKE_CXX_STANDARD 17)

# Use Debug build type if a build type is not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set the optimization level to the highest
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        add_compile_options(-O3)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/Ox)
    endif()
endif()

# Get a list of paths for the cpp and hpp files.
file(GLOB STEREOCODE_SOURCE *.cpp *.hpp)

# The stereocode application (target).
add_executable(stereocode ${STEREOCODE_SOURCE})

# Download the CLI11 parser into directory named "external".
set(CMAKE_EXTERNAL_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external) # Creates folder if doesn't exists.
file(DOWNLOAD https://github.com/CLIUtils/CLI11/releases/download/v2.3.2/CLI11.hpp ${CMAKE_EXTERNAL_SOURCE_DIR}/CLI11.hpp) # Name of file is needed since content of file itself are being downloaded.
target_include_directories(stereocode PRIVATE ${CMAKE_EXTERNAL_SOURCE_DIR})

# Assumes srcml.h may be in this directory
if(WIN32)
    target_include_directories(stereocode PRIVATE "C:/Program Files/srcML/include/") # Replace with your include path on Windows
else()
    target_include_directories(stereocode PRIVATE /usr/local/include)
endif()


# Find and use libsrcml
find_package(srcML REQUIRED)
target_link_libraries(stereocode PRIVATE srcML::LibsrcML)

# Turn on compiler warnings.

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Wpedantic) 
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/Wall)
endif()

enable_testing()
file(COPY tests DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB TESTFILES ${CMAKE_CURRENT_BINARY_DIR}/tests/*.xml)
list(FILTER TESTFILES EXCLUDE REGEX "BASE.xml|stereotypes.xml|runtests.cmake")
foreach(TEST ${TESTFILES})
    message(STATUS "Add test case ${TEST}")
    get_filename_component(DIRPART ${TEST} DIRECTORY)
    get_filename_component(BASENAME ${TEST} NAME)
    string(REGEX REPLACE "\\.xml$" "" BASENAME ${BASENAME})
    set(FULL_PATH_WITHOUT_EXTENSION "${DIRPART}/${BASENAME}")
    # The -D option creates variables and pass them with the -P option to the runtests.cmake.
    add_test(NAME "${BASENAME}_test" COMMAND ${CMAKE_COMMAND} -DSTEREOCODE=$<TARGET_FILE:stereocode> -DTEST_FILE=${FULL_PATH_WITHOUT_EXTENSION} -P tests/runtests.cmake)
endforeach()
