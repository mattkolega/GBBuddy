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

add_executable(gbbuddy
    src/main.cpp
)