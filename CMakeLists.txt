# @file CMakeLists.txt
# CMake files for the Stereocode project

# Supports any version of CMake >= specified
cmake_minimum_required(VERSION 3.28)

# If the user has not explicitly set a compiler, try to find Clang.
if(NOT CMAKE_CXX_COMPILER)
    find_program(CLANG_CXX_PATH NAMES clang++ clang-cl)
    if(CLANG_CXX_PATH)
        set(CMAKE_CXX_COMPILER "${CLANG_CXX_PATH}")
        message(STATUS "User did not specify a compiler. Defaulting to Clang: ${CLANG_CXX_PATH}")
    endif()
endif()

# Project information
project(Stereocode LANGUAGES CXX)

# Prevent in-source builds. 
# However, it does not prevent the CMakeFiles folder and CMakeCache.txt from being created (need to manually remove them)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory and run CMake from there.")
endif()

# Stereocode requires C++17 or higher
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use Release build type if a build type is not specified
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE)
endif()

# Get a list of paths for the cpp and hpp files
file(GLOB_RECURSE STEREOCODE_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

# The stereocode application (target)
add_executable(stereocode ${STEREOCODE_SOURCES})

# Include directory (Quotes for paths that contain a space)
target_include_directories(stereocode PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# For srcML v1.1+
# Finds and use libsrcml + srcml.h
find_package(srcML QUIET) 

if (srcML_FOUND)
    message(STATUS "Found srcML package version: ${srcML_VERSION}")
    target_link_libraries(stereocode PRIVATE srcML::LibsrcML)
else() # Manual search for headers and libs
    message(STATUS "srcML config not found. Attempting manual search for libs...")
    set(ROOT_DIR $ENV{SystemDrive}) # Windows could be on D:/ ... etc
    find_path(SRCML_INCLUDE_DIR 
        NAMES srcml.h 
        HINTS "${ROOT_DIR}/srcML/include" "/usr/local/include"
    )
    find_library(SRCML_LIB 
        NAMES libsrcml
        HINTS "${ROOT_DIR}/srcML/lib" "/usr/local/lib"
    )
    if(SRCML_LIB AND SRCML_INCLUDE_DIR)
        target_include_directories(stereocode PRIVATE ${SRCML_INCLUDE_DIR})
        target_link_libraries(stereocode PRIVATE ${SRCML_LIB})
        message(STATUS "Found srcML library at: ${SRCML_LIB}")
    else()
        message(FATAL_ERROR "Could not find srcML. Please install it or set SRCML_LIB and SRCML_INCLUDE_DIR.")
    endif()
endif()

# Turn on compiler warnings
target_compile_options(stereocode PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wshadow -Wunused>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wshadow-all -Wunused>
)

enable_testing()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Find test files
file(GLOB TESTFILES ${CMAKE_CURRENT_BINARY_DIR}/test/*.xml)
list(FILTER TESTFILES EXCLUDE REGEX "BASE.xml|stereotypes.xml")

foreach(TEST_FILE_PATH ${TESTFILES})
    # 1. Get the base name for the test label (e.g., "Cpp")
    get_filename_component(TEST_NAME ${TEST_FILE_PATH} NAME_WE)
    
    # 2. Get the full path WITHOUT extension for the script (e.g., "build/test/Cpp")
    #    The script will append .xml, .BASE.xml, etc.
    string(REGEX REPLACE "\\.xml$" "" TEST_FILE_NO_EXT "${TEST_FILE_PATH}")

    add_test(
        NAME "${TEST_NAME}_test"
        COMMAND ${CMAKE_COMMAND}
            -DSTEREOCODE=$<TARGET_FILE:stereocode>
            -DTEST_FILE=${TEST_FILE_NO_EXT}   # <--- Passing the stripped path
            -P ${CMAKE_CURRENT_BINARY_DIR}/test/runtests.cmake
    )
endforeach()