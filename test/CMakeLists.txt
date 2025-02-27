FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
  GIT_SHALLOW 1
)
FetchContent_MakeAvailable(Catch2)

# Make Catch2 scripts discoverable
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(gbbuddytest
    mmutest.cpp
)

target_link_libraries(gbbuddytest
    PRIVATE
        common
        core
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(gbbuddytest)