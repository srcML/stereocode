# @file CMakeLists.txt
# CMake files for the Stereocode project

# Supports any version of CMake >= specified, with the policies of the newest version named
cmake_minimum_required(VERSION 3.20...3.28)

# If the user has not set a compiler, with CMAKE_CXX_COMPILER or the CXX environment variable, try to find Clang.
if(NOT CMAKE_CXX_COMPILER AND NOT DEFINED ENV{CXX})
    find_program(CLANG_CXX_PATH NAMES clang++ clang-cl)
    if(CLANG_CXX_PATH)
        set(CMAKE_CXX_COMPILER "${CLANG_CXX_PATH}")
        message(STATUS "User did not specify a compiler. Defaulting to Clang: ${CLANG_CXX_PATH}")
    endif()
endif()

# Project information
project(Stereocode VERSION 2.0.0 LANGUAGES CXX)

# Prevent in-source builds.
# However, it does not prevent the CMakeFiles folder and CMakeCache.txt from being created (need to manually remove them)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory and run CMake from there.")
endif()

# Stereocode requires C++17 or higher, without compiler extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use Release build type if a build type is not specified
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Get a list of paths for the cpp and hpp files, which the build checks again for added or removed files
file(GLOB_RECURSE STEREOCODE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)
list(REMOVE_ITEM STEREOCODE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/stereocode.cpp")

# The analysis library (target) holds every source file but the main function, so the program and the unit tests link the same code
add_library(stereocode_lib STATIC ${STEREOCODE_SOURCES})

# Include directories (Quotes for paths that contain a space)
target_include_directories(stereocode_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/src")

# The stereocode application (target)
add_executable(stereocode src/stereocode.cpp)
target_compile_definitions(stereocode PRIVATE STEREOCODE_VERSION="${PROJECT_VERSION}")
target_link_libraries(stereocode PRIVATE stereocode_lib)

# For srcML v1.1+
# Finds and use libsrcml + srcml.h
find_package(srcML QUIET)

if (srcML_FOUND)
    message(STATUS "Found srcML package version: ${srcML_VERSION}")
    target_link_libraries(stereocode_lib PUBLIC srcML::LibsrcML)
else() # Manual search for headers and libs
    message(STATUS "srcML config not found. Attempting manual search for libs...")
    set(ROOT_DIR $ENV{SystemDrive}) # Windows could be on D:/ ... etc
    find_path(SRCML_INCLUDE_DIR
        NAMES srcml.h
        HINTS "${ROOT_DIR}/srcML/include" "/usr/local/include" "/opt/homebrew/include"
    )
    # find_library adds the lib prefix on Linux and macOS, so only the Windows library needs the name libsrcml
    find_library(SRCML_LIB
        NAMES srcml libsrcml
        HINTS "${ROOT_DIR}/srcML/lib" "/usr/local/lib" "/opt/homebrew/lib"
    )
    if(SRCML_LIB AND SRCML_INCLUDE_DIR)
        target_include_directories(stereocode_lib PUBLIC ${SRCML_INCLUDE_DIR})
        target_link_libraries(stereocode_lib PUBLIC ${SRCML_LIB})
        message(STATUS "Found srcML library at: ${SRCML_LIB}")
    else()
        message(FATAL_ERROR "Could not find srcML. Please install it or set SRCML_LIB and SRCML_INCLUDE_DIR.")
    endif()
endif()

# libxml2 checks the srcML input and runs the queries of the analysis (libsrcml already loads the same library)
find_package(LibXml2 REQUIRED)
target_link_libraries(stereocode_lib PUBLIC LibXml2::LibXml2)

# The worker threads need the thread library, which glibc before 2.34 keeps apart
find_package(Threads REQUIRED)
target_link_libraries(stereocode_lib PUBLIC Threads::Threads)

# Turn on compiler warnings, which test/CMakeLists.txt also gives the programs that do not link the library
set(STEREOCODE_WARNINGS
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wshadow -Wunused -Wno-trigraphs>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wshadow-all -Wunused -Wno-trigraphs>
)
target_compile_options(stereocode_lib PUBLIC ${STEREOCODE_WARNINGS})

# AddressSanitizer and UndefinedBehaviorSanitizer turn leaks and undefined behavior into test failures
option(STEREOCODE_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer (GCC or Clang)" OFF)
if(STEREOCODE_SANITIZE)
    target_compile_options(stereocode_lib PUBLIC -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    target_link_options(stereocode_lib PUBLIC -fsanitize=address,undefined)
    # With sanitizers, GCC falsely reports uninitialized members inside the std::regex of libstdc++
    target_compile_options(stereocode_lib PUBLIC $<$<CXX_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>)
endif()

enable_testing()

# test/CMakeLists.txt registers every test
add_subdirectory(test)
