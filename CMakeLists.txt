cmake_minimum_required(VERSION 3.28...3.31)

project(
    GBBuddy
    VERSION 0.1
    DESCRIPTION "A modern Game Boy emulator"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

# Export compile_commands.json so intellisense works properly
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add CMake modules
include(FetchContent)

# Get fmt library for formatting console output
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.1.1
    GIT_SHALLOW 1
)
# Get SDL3 for multimedia functionality
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.0
    GIT_SHALLOW 1
)
# Get tinyfiledialogs for native dialogs
FetchContent_Declare(
    tinyfiledialogs
    GIT_REPOSITORY https://git.code.sf.net/p/tinyfiledialogs/code
    GIT_TAG master
    GIT_SHALLOW 1
)

FetchContent_MakeAvailable(fmt SDL3 tinyfiledialogs)

# tinyfiledialogs needs to be compiled before it can be used
add_library(tinyfiledialogs_lib STATIC ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c)

add_subdirectory(src/common)
add_subdirectory(src/core)

add_executable(gbbuddy
    src/main.cpp
)

target_link_libraries(gbbuddy
    PRIVATE
        common
        core
        SDL3::SDL3
)