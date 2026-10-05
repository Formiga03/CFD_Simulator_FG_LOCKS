#include "matrix.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include <vector>

// Helper to fill test data
Matrixf create_random_matrix(int rows, int cols) {
    Matrixf m(rows, cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            m(r, c) = static_cast<float>(r * cols + c);
        }
    }
    return m;
}

TEST_CASE("Matrix Performance Benchmarks", "[benchmark]") {
    constexpr int N = 512;
    Matrixf mat = create_random_matrix(N, N);
    Matrixf reusable_out(N, N);

    // 1. Returning a newly created object keeps it alive
    BENCHMARK("transpose() - Allocates new matrix") {
        return mat.transpose();
    };

    // 2. For in-place/buffer mutations, return a pointer or reference to the buffer
    BENCHMARK("transpose_into() - Zero allocation, cache-tiled") {
        mat.transpose_into(reusable_out);
        return reusable_out.data.data();
    };

    // 3. Returning the extracted vector
    BENCHMARK("col(0) - Copies N elements to heap vector") {
        return mat.col(0);
    };

    // 4. Returning an accessed element from the view
    BENCHMARK("col_view(0) - Non-owning strided view") {
        auto view = mat.col_view(0);
        return view(N / 2);
    };

    // 5. Returning the buffer pointer
    std::vector<int> sub_indices(128);
    for (int i = 0; i < 128; ++i) sub_indices[i] = i * 2;
    Matrixf sub_buffer;

    BENCHMARK("submatrix_into() - 128x128 slice") {
        mat.submatrix_into(sub_indices, sub_buffer);
        return sub_buffer.data.data();
    };
}