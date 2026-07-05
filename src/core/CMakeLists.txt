add_library(core STATIC
    bus.cpp
    cartridge.cpp
    cpu.cpp
    gameboy.cpp

    mappers/mapper.cpp
    mappers/nombc.cpp
)

target_link_libraries(core
    PRIVATE
        project_sanitizers
        project_warnings
        common
        fmt::fmt
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
