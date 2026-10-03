#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <complex>
#include <vector>

#include "include/matrix.hpp"

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// Generates a helper incremental matrix
template <typename T>
Matrix<T> make_incremental_matrix(int rows, int cols)
{

	Matrix<T> m(rows, cols);

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


