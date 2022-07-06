# @file CMakeLists.txt
# 
# CMake files for the Stereocode project

# Can be run by any version of CMake >= this
cmake_minimum_required(VERSION 3.17)

# project info
project(Stereocode)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Use Release build type if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

file(GLOB STEREOCODE_SOURCE *.cpp *.hpp)

# stereocode application
add_executable(stereocode ${STEREOCODE_SOURCE})

# assumes srcml.h may be in this directory
target_include_directories(stereocode PRIVATE /usr/local/include)

# Download CLI11 into directory named external
set(CMAKE_EXTERNAL_SOURCE_DIR ${CMAKE_BINARY_DIR}/external)
file(DOWNLOAD https://github.com/CLIUtils/CLI11/releases/download/v1.9.1/CLI11.hpp ${CMAKE_EXTERNAL_SOURCE_DIR}/CLI11.hpp)
target_include_directories(stereocode PRIVATE ${CMAKE_EXTERNAL_SOURCE_DIR})

# libsrcml
find_library(LIBSRCML_LINK NAMES libsrcml libsrcml.1 libsrcml.1.dylib libsrcml.so.1 libsrcml.so)
target_link_directories(stereocode PRIVATE /usr/local/lib)
target_link_libraries(stereocode ${LIBSRCML_LINK})

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

enable_testing()
FILE(COPY test.sh DESTINATION .)
add_test(NAME AxisModelBase_test COMMAND test.sh $<TARGET_FILE:stereocode> ${CMAKE_SOURCE_DIR}/tests/AxisModelBase.xml)
add_test(NAME AxisModelLinear_test COMMAND test.sh $<TARGET_FILE:stereocode> ${CMAKE_SOURCE_DIR}/tests/AxisModelLinear.xml)
add_test(NAME AxisModelLog_test COMMAND test.sh $<TARGET_FILE:stereocode> ${CMAKE_SOURCE_DIR}/tests/AxisModelLog.xml)
add_test(NAME AxisTick_test COMMAND test.sh $<TARGET_FILE:stereocode> ${CMAKE_SOURCE_DIR}/tests/AxisTick.xml)
add_test(NAME Range_test COMMAND test.sh $<TARGET_FILE:stereocode> ${CMAKE_SOURCE_DIR}/tests/Range.xml)
