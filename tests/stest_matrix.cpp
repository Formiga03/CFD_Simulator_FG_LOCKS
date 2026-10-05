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

TEST_CASE("Matrix Performance Benchmarks", "[!benchmark]") {
    // 512x512 matrix (~1 MB data, exceeds L1/L2 cache sizes to stress memory)
    constexpr int N = 512;
    Matrixf mat = create_random_matrix(N, N);
    Matrixf reusable_out(N, N);

    // -------------------------------------------------------------
    // Benchmark 1: Transpose Allocation vs Reusable Buffer
    // -------------------------------------------------------------
    BENCHMARK("transpose() - Allocates new matrix") {
        auto result = mat.transpose();
        // Prevent compiler from optimizing out the unused result
        Catch::Benchmark::doNotOptimizeAway(result.data.data());
        return result;
    };

    BENCHMARK("transpose_into() - Zero allocation, cache-tiled") {
        mat.transpose_into(reusable_out);
        Catch::Benchmark::doNotOptimizeAway(reusable_out.data.data());
    };

    // -------------------------------------------------------------
    // Benchmark 2: Column Copy vs Zero-Allocation Column View
    // -------------------------------------------------------------
    BENCHMARK("col(0) - Copies N elements to heap vector") {
        std::vector<float> copied = mat.col(0);
        Catch::Benchmark::doNotOptimizeAway(copied.data());
        return copied;
    };

    BENCHMARK("col_view(0) - Non-owning strided view") {
        auto view = mat.col_view(0);
        // Access an element to simulate real loop usage
        float sample = view(N / 2);
        Catch::Benchmark::doNotOptimizeAway(sample);
        return view;
    };

    // -------------------------------------------------------------
    // Benchmark 3: Submatrix Extraction
    // -------------------------------------------------------------
    std::vector<int> sub_indices(128);
    for (int i = 0; i < 128; ++i) sub_indices[i] = i * 2;
    Matrixf sub_buffer;

    BENCHMARK("submatrix_into() - 128x128 slice") {
        mat.submatrix_into(sub_indices, sub_buffer);
        Catch::Benchmark::doNotOptimizeAway(sub_buffer.data.data());
    };
}