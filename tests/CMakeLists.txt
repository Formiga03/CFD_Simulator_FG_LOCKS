# Download Catch2 via FetchContent (isolated to tests only)
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Standard Unit Test Runner
add_executable(utest_matrix utest_matrix.cpp)

target_link_libraries(utest_matrix PRIVATE
    matrix_lib
    Catch2::Catch2WithMain
)

# Speed Test Runner
add_executable(stest_matrix stest_matrix.cpp)

target_link_libraries(stest_matrix PRIVATE 
    matrix_lib 
    Catch2::Catch2WithMain
)

# Integrate with CTest test discovery
include(Catch)
catch_discover_tests(utest_matrix)