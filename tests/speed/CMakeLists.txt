add_executable(stest_matrix
    stest_matrix.cpp
)

target_link_libraries(stest_matrix PRIVATE
    matrix_lib
    Catch2::Catch2WithMain
)

# Optional: Add a custom target so you can type `cmake --build build-test --target run_benchmarks`
add_custom_target(run_benchmarks
    COMMAND stest_matrix
    DEPENDS stest_matrix
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Executing Catch2 performance benchmarks..."
)