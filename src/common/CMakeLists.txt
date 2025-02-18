add_library(common STATIC
    dialog.cpp
)

target_link_libraries(common
    PRIVATE
        fmt::fmt
        tinyfiledialogs_lib
)

target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    PRIVATE
        ${tinyfiledialogs_SOURCE_DIR}
)
