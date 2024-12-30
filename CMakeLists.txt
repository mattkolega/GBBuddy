cmake_minimum_required(VERSION 3.28...3.31)

project(
    GBBuddy
    VERSION 0.1
    DESCRIPTION "A modern Game Boy emulator"
    LANGUAGES CXX
)

# Set standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Turn off compiler extensions
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
FetchContent_MakeAvailable(fmt)

add_executable(gbbuddy
    src/main.cpp
)

# Link libraries to executable
target_link_libraries(gbbuddy
    PRIVATE
    fmt::fmt
)