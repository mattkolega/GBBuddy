cmake_minimum_required(VERSION 3.28...3.31)

project(
    GBBuddy
    VERSION 0.1
    DESCRIPTION "A modern Game Boy emulator"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
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
)
# Get SDL3 for multimedia functionality
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG preview-3.1.6
)
# Get tinyfiledialogs for native dialogs
FetchContent_Declare(
    tinyfiledialogs
    GIT_REPOSITORY https://git.code.sf.net/p/tinyfiledialogs/code
    GIT_TAG 29c1b354d75825209adf8cc1979c425885a64d32
)

FetchContent_MakeAvailable(fmt SDL3 tinyfiledialogs)

add_executable(gbbuddy
    src/main.cpp

    src/core/cpu.cpp
    src/core/mmu.cpp

    src/utils/dialog.cpp
)

add_library(tinyfiledialogs_lib STATIC ${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c)

target_link_libraries(gbbuddy PRIVATE
    fmt::fmt
    SDL3::SDL3
    tinyfiledialogs_lib
)

target_include_directories(gbbuddy PRIVATE
    ${tinyfiledialogs_SOURCE_DIR}
)