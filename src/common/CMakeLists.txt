add_library(common STATIC
    bitwise.h
    dialog.cpp
    dialog.h
    logger.h
)

target_link_libraries(common
    INTERFACE
        fmt::fmt
    PRIVATE
        tinyfiledialogs_lib
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)