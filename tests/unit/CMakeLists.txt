add_executable(unit_tests
    utest_matrix.cpp
    # Add future test files here
)

target_link_libraries(unit_tests PRIVATE
    matrix_lib
    Catch2::Catch2WithMain
)

# Automatically registers individual TEST_CASE entries with CTest
catch_discover_tests(unit_tests)