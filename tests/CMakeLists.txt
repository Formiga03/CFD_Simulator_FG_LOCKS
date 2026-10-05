# 1. Download Catch2 via FetchContent (isolated to tests only)
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# 2. Define the test executable
add_executable(utest_matrix utest_matrix.cpp)

# 3. Link against your core library (defined in the root) and Catch2
target_link_libraries(utest_matrix PRIVATE
    matrix_lib
    Catch2::Catch2WithMain
)

# 4. Integrate with CTest test discovery
include(Catch)
catch_discover_tests(utest_matrix)