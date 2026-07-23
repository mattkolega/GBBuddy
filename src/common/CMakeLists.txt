add_library(common STATIC
    fs.cpp
)

target_link_libraries(common
    PRIVATE
        project_sanitizers
        project_warnings
        tinyfiledialogs_lib
    PUBLIC
        fmt::fmt
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
