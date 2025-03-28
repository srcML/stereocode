# @file CMakeLists.txt
# CMake files for the Stereocode project

# Supports any version of CMake >= specified
cmake_minimum_required(VERSION 3.17)

# Prevent in-source builds. However, it does not prevent CMakeFiles and CMakeCache.txt from being created (need to manually remove them)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    # Stop CMake immediately with an error
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory and run CMake from there.")
endif()

# Project information
project(Stereocode LANGUAGES CXX)

# Stereocode requires C++17 or higher
set(CMAKE_CXX_STANDARD 17)

# Use Release build type if a build type is not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Get a list of paths for the cpp and hpp files
file(GLOB STEREOCODE_SOURCE src/*.cpp src/*.hpp)

# The stereocode application (target)
add_executable(stereocode ${STEREOCODE_SOURCE})

# Copy CLI11 to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(stereocode PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)

# For srcML v1.1+
# Finds and use libsrcml + srcml.h
find_package(srcML QUIET) 

if (srcML_FOUND) 
    target_link_libraries(stereocode PRIVATE srcML::LibsrcML)
else() # For srcML v1.0
    # Find srcml.h
    if(WIN32)
        set(ROOT_DIR $ENV{SystemDrive}) # Windows could be on D:/ ... etc
        target_include_directories(stereocode PRIVATE "${ROOT_DIR}/Program Files/srcML/include")
    else()
        target_include_directories(stereocode PRIVATE "/usr/local/include")
    endif()

    # Search common paths
    find_library(LIBSRCML_LINK NAMES libsrcml.dylib libsrcml.1.dylib libsrcml.so.1 libsrcml.so libsrcml.lib)

    if (LIBSRCML_LINK)
        target_link_libraries(stereocode ${LIBSRCML_LINK})
    else()
        if(WIN32)
            find_library(SRCML_LIB libsrcml.lib PATHS "${ROOT_DIR}/Program Files/srcML/lib")
            target_link_libraries(stereocode PRIVATE ${SRCML_LIB})
        else()
            find_library(SRCML_LIB NAMES libsrcml.dylib libsrcml.1.dylib libsrcml.so.1 libsrcml.so PATHS "/usr/local/lib")
            target_link_libraries(stereocode PRIVATE ${SRCML_LIB})
        endif()
    endif()
endif()

# Turn on compiler warnings.
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Wpedantic) 
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/Wall /Wextra /Wpedantic)
endif()

enable_testing()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB TESTFILES ${CMAKE_CURRENT_BINARY_DIR}/test/*.xml)
list(FILTER TESTFILES EXCLUDE REGEX "BASE.xml|stereotypes.xml")

foreach(TEST_FILE ${TESTFILES})
    string(REGEX REPLACE "\\.xml$" "" TEST_FILE "${TEST_FILE}")
    get_filename_component(BASENAME ${TEST_FILE} NAME)

    add_test(
        NAME "${BASENAME}_test"
        COMMAND ${CMAKE_COMMAND}
            -DSTEREOCODE=$<TARGET_FILE:stereocode>
            -DTEST_FILE=${TEST_FILE}
            -P ${CMAKE_CURRENT_BINARY_DIR}/test/runtests.cmake
    )
endforeach()