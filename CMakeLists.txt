# @file CMakeLists.txt
# CMake files for the Stereocode project

# Supports any version of CMake >= specified
cmake_minimum_required(VERSION 3.17)

# Project information
project(Stereocode LANGUAGES CXX)

# Stereocode requires C++17 or higher
set(CMAKE_CXX_STANDARD 17)

# Use Release build type if a build type is not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Get a list of paths for the cpp and hpp files
file(GLOB STEREOCODE_SOURCE *.cpp *.hpp)

# The stereocode application (target)
add_executable(stereocode ${STEREOCODE_SOURCE})

# Check if the CLI11 exists
set(CLI11_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(CMAKE_EXTERNAL_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/external)
if(EXISTS ${CLI11_DIR})
    # If it exists, copy the 'external' directory to the binary directory.
    file(COPY ${CLI11_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
    # If it doesn't exist, download the file.
    file(DOWNLOAD https://github.com/CLIUtils/CLI11/releases/download/v2.4.1/CLI11.hpp ${CMAKE_EXTERNAL_SOURCE_DIR}/CLI11.hpp)
endif()
target_include_directories(stereocode PRIVATE ${CMAKE_EXTERNAL_SOURCE_DIR})

# Find and use libsrcml
find_package(srcML REQUIRED)
target_link_libraries(stereocode PRIVATE srcML::LibsrcML)

# Turn on compiler warnings.
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Wpedantic) 
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/Wall /Wextra /Wpedantic)
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