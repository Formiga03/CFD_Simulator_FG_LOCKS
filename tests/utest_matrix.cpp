#include "matrix.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <complex>
#include <vector>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// Generates a helper incremental matrix
template <typename T>
Matrix<T> make_incremental_matrix(int rows, int cols)
{

	Matrix<T> m(rows, cols);
	T val = T{1};

	for (int r = 0; r < rows; ++r)
	{
		for(int c = 0; c < cols; ++c)
		{
			m(r,c) = val;
			val += T{1};
		}
	}

	return m;
}

TEST_CASE("Matrix Lifecycle", "[matrix][lifecycle]")
{
	SECTION("Default constructor 0x0 empty matrix")
	{
		Matrixf m;
		CHECK(m.rows == 0);
		CHECK(m.cols == 0);
		CHECK(m.data.empty());
	}

	SECTION("Dimension consctructor initalises elements to zero")
	{
		Matrixf m(3,4);
		CHECK(m.rows == 3);
		CHECK(m.cols == 4);
		CHECK(m.data.size() == 12);
		for (float val: m.data)
		{
			CHECK(val == 0.0f);
		}
	}

	SECTION("Vector constructor size checks")
	{
		std::vector<float> valid_data = {1.0f,2.0f,3.0f,4.0f,5.0f,6.0f};
		REQUIRE_NOTHROW(Matrixf(2, 3, valid_data));

		std::vector<float> invalid_data = {1.0f, 2.0f};
		REQUIRE_THROWS_AS(Matrixf(2, 3, invalid_data), std::invalid_argument);

	}

	SECTION("Deep Copy Isolation")
	{
		auto m = make_incremental_matrix<float>(2,2);
		Matrixf m_copy = m;

		m_copy(0,0) = 999.0f;

		CHECK_THAT(m(0, 0), WithinAbs(1.0f,1e-6f));
		CHECK_THAT(m_copy(0, 0), WithinAbs(999.0f, 1e-6f));
	}
	
	SECTION("Move transfer ownership leaves valid source")
	{
		auto m = make_incremental_matrix<float>(3,3);
		Matrixf m_target = std::move(m);

		CHECK(m_target.rows == 3);
		CHECK(m_target.cols == 3);
		CHECK(m_target.data.size() == 9);

		CHECK(m.rows == 0);
		CHECK(m.cols == 0);
		CHECK(m.data.empty());
	}
}

TEST_CASE("Column view operations and aliasing", "[matrix][view]") {
    // 3 rows, 4 columns:
    // [  1,  2,  3,  4 ]
    // [  5,  6,  7,  8 ]
    // [  9, 10, 11, 12 ]
    auto mat = make_incremental_matrix<float>(3, 4);

    SECTION("View reads correct strided elements") {
        column<float> col1 = mat.col_view(1); // Second column: {2, 6, 10}

        REQUIRE(col1.col_length == 3);
        REQUIRE(col1.row_length == 4);

        CHECK_THAT(col1(0), WithinAbs(2.0f, 1e-6f));
        CHECK_THAT(col1(1), WithinAbs(6.0f, 1e-6f));
        CHECK_THAT(col1(2), WithinAbs(10.0f, 1e-6f));
    }

    SECTION("Mutating column view mutates underlying matrix memory (Zero-Copy)") {
        column<float> col2 = mat.col_view(2);
        col2(1) = 42.0f; // Modifies mat(1, 2)

        CHECK_THAT(mat(1, 2), WithinAbs(42.0f, 1e-6f));
    }

    SECTION("Safe access .at() enforces bounds checking") {
        column<float> col0 = mat.col_view(0);

        CHECK_NOTHROW(col0.at(0));
        CHECK_NOTHROW(col0.at(2));
        CHECK_THROWS_AS(col0.at(-1), std::out_of_range);
        CHECK_THROWS_AS(col0.at(3), std::out_of_range);
    }

    SECTION("Requesting column view out of bounds throws") {
        CHECK_THROWS_AS(mat.col_view(-1), std::invalid_argument);
        CHECK_THROWS_AS(mat.col_view(4), std::invalid_argument);
    }
}

TEST_CASE("Submatrix extraction and slicing", "[matrix][slice]") {
    auto mat = make_incremental_matrix<float>(4, 4);

    SECTION("Arbitrary and repeated index submatrix") {
        // Pick rows [2, 0] and cols [3, 1]
        std::vector<int> r_idx{2, 0};
        std::vector<int> c_idx{3, 1};

        Matrixf sub = mat.submatrix(r_idx, c_idx);

        REQUIRE(sub.rows == 2);
        REQUIRE(sub.cols == 2);
        CHECK_THAT(sub(0, 0), WithinAbs(mat(2, 3), 1e-6f));
        CHECK_THAT(sub(0, 1), WithinAbs(mat(2, 1), 1e-6f));
        CHECK_THAT(sub(1, 0), WithinAbs(mat(0, 3), 1e-6f));
        CHECK_THAT(sub(1, 1), WithinAbs(mat(0, 1), 1e-6f));
    }

    SECTION("Submatrix out-of-range index throws") {
        Matrixf out;
        CHECK_THROWS_AS(mat.submatrix_into({0, 4}, {1, 2}, out), std::out_of_range);
        CHECK_THROWS_AS(mat.submatrix_into({0, 1}, {-1, 2}, out), std::out_of_range);
    }
}

TEST_CASE("Cache-Blocked Transpose correctly handles tile edges", "[matrix][math]") {
    // 35x67 crosses the 32x32 boundary and tests non-square remainder tiles
    const int R = 35;
    const int C = 67;
    auto original = make_incremental_matrix<double>(R, C);

    Matrixd transposed = original.transpose();

    REQUIRE(transposed.rows == C);
    REQUIRE(transposed.cols == R);

    bool matches = true;
    for (int r = 0; r < R; ++r) {
        for (int c = 0; c < C; ++c) {
            if (transposed(c, r) != original(r, c)) {
                matches = false;
                break;
            }
        }
    }
    CHECK(matches);
}

TEST_CASE("Matrix trace and diagonal operations", "[matrix][math]") {
    SECTION("Trace on square and rectangular matrices") {
        // Square: diag elements are 1, 5, 9 -> sum = 15
        auto square = make_incremental_matrix<float>(3, 3);
        CHECK_THAT(square.trace(), WithinAbs(15.0f, 1e-6f));

        // Rectangular tall (4x2): min_dim = 2 -> 1 + 4 = 5
        auto tall = make_incremental_matrix<float>(4, 2);
        CHECK_THAT(tall.trace(), WithinAbs(5.0f, 1e-6f));
    }

    SECTION("Superdiagonal and subdiagonal populations") {
        Matrixf m(4, 4);
        m.diag(1, 7.0f);  // Superdiagonal: (0,1), (1,2), (2,3)
        m.diag(-2, 3.0f); // Subdiagonal:   (2,0), (3,1)

        CHECK_THAT(m(0, 1), WithinAbs(7.0f, 1e-6f));
        CHECK_THAT(m(1, 2), WithinAbs(7.0f, 1e-6f));
        CHECK_THAT(m(2, 3), WithinAbs(7.0f, 1e-6f));
        CHECK_THAT(m(2, 0), WithinAbs(3.0f, 1e-6f));
        CHECK_THAT(m(3, 1), WithinAbs(3.0f, 1e-6f));

        // Other elements remain untouched
        CHECK_THAT(m(0, 0), WithinAbs(0.0f, 1e-6f));
    }
}

TEST_CASE("Complex number support (MatrixCf)", "[matrix][complex]") {
    MatrixCf cmat(2, 2);
    cmat(0, 0) = std::complex<float>(3.0f, 4.0f);
    cmat(0, 1) = std::complex<float>(1.0f, -2.0f);
    cmat(1, 0) = std::complex<float>(0.0f, 5.0f);
    cmat(1, 1) = std::complex<float>(-7.0f, 0.0f);

    SECTION("Conjugate in place flips imaginary components") {
        cmat.conjugate_in_place();

        CHECK_THAT(cmat(0, 0).real(), WithinAbs(3.0f, 1e-6f));
        CHECK_THAT(cmat(0, 0).imag(), WithinAbs(-4.0f, 1e-6f));

        CHECK_THAT(cmat(0, 1).real(), WithinAbs(1.0f, 1e-6f));
        CHECK_THAT(cmat(0, 1).imag(), WithinAbs(2.0f, 1e-6f));
    }

    SECTION("diag_exp_into generates unit complex phasors (Euler's formula)") {
        MatrixCf phase_mat(2, 2);
        std::vector<float> phases{0.0f, 3.14159265f / 2.0f}; // 0 and pi/2

        phase_mat.diag_exp_into(phases);

        // e^(i * 0) = 1 + 0i
        CHECK_THAT(phase_mat(0, 0).real(), WithinAbs(1.0f, 1e-5f));
        CHECK_THAT(phase_mat(0, 0).imag(), WithinAbs(0.0f, 1e-5f));

        // e^(i * pi/2) = 0 + 1i
        CHECK_THAT(phase_mat(1, 1).real(), WithinAbs(0.0f, 1e-5f));
        CHECK_THAT(phase_mat(1, 1).imag(), WithinAbs(1.0f, 1e-5f));
    }
}