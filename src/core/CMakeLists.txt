add_library(core STATIC
    cartridge.cpp
    cpu.cpp
    gameboy.cpp
    mmu.cpp

    mappers/mapper.cpp
    mappers/nombc.cpp
)

target_link_libraries(core
    PRIVATE
        common
        fmt::fmt
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

