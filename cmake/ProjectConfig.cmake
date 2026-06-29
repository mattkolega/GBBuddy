# Project configuration
# Settings/options to be applied across targets

# Create symlink for compile_commands so IDE can find it more easily

if(CMAKE_EXPORT_COMPILE_COMMANDS AND NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    file(
        CREATE_LINK
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
        SYMBOLIC
    )
endif()

# Add option to enable address and undefined behaviour sanitisation
# Targets must be linked with `project_sanitizers` to be affected

option(SANITIZE "Compile with ASan/UBSan (MSVC only supports ASan)" OFF)

add_library(project_sanitizers INTERFACE)
if(SANITIZE)
    if(MSVC)
        target_compile_options(project_sanitizers INTERFACE /fsanitize=address)
    else()
        target_compile_options(project_sanitizers INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(project_sanitizers INTERFACE -fsanitize=address,undefined)
    endif()
endif()

# Enable extra warnings
# Targets must be linked with `project_warnings` to be affected

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Wno-gnu-anonymous-struct>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

# Enable IPO/LTO optimisations for Release builds if supported by compiler

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
else()
    message(WARNING "IPO/LTO not supported: ${ipo_error}")
endif()
