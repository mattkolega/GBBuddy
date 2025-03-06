# Needed for creating test cases
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0
    GIT_SHALLOW TRUE
)
# Needed for parsing JSON
FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG        v4.4.3
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    sm83tests
    GIT_REPOSITORY https://github.com/SingleStepTests/sm83
    GIT_TAG        main
)
FetchContent_MakeAvailable(Catch2 glaze sm83tests)

# Make Catch2 scripts discoverable
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# Add command to copy CPU test data to build dir
add_custom_target(copy_sm83_tests
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${sm83tests_SOURCE_DIR}/v1
    ${CMAKE_BINARY_DIR}/test/cputestdata
)

add_executable(gbbuddytest
    bitwisetest.cpp
    cputest.cpp
    mmutest.cpp
)

target_link_libraries(gbbuddytest
    PRIVATE
        common
        core
        Catch2::Catch2WithMain
        glaze::glaze
)

add_dependencies(gbbuddytest copy_sm83_tests)

include(CTest)
include(Catch)
catch_discover_tests(gbbuddytest)