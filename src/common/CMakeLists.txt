add_library(common STATIC
    dialog.cpp
)

target_link_libraries(common
    PUBLIC
        fmt::fmt  # Public because it's included in logger.h which is used by other targets
    PRIVATE
        tinyfiledialogs_lib
)

target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    PRIVATE
        ${tinyfiledialogs_SOURCE_DIR}
)
