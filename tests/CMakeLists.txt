# Fetch Catch2 once for all test categories
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Catch module needed for catch_discover_tests
include(Catch)

# Conditionally delegate to subdirectories
if(BUILD_TESTING)
    add_subdirectory(unit)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(speed)
endif()