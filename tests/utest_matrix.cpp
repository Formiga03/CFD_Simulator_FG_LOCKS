#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <complex>
#include <vector>

#include "matrix.hpp"

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
