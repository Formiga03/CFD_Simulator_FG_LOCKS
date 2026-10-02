/**
 * @file matrix_ops.hpp
 * @brief Core linear algebra operations for MIPT QuantumSim.
 * 
 * Declarations for BLAS/LAPACKE matrix wrappers. Includes standard 
 * allocating functions and high-performance in-place operations.
 */

#pragma once

#include <vector>
#include <complex>
#include <utility>
#include <random>

// Assuming MatrixCd is defined here based on your main.cpp includes
#include "matrix.hpp" 

// ================================================================================
// Matrix Multiplication
// ================================================================================

/**
 * @brief Standard matrix multiplication (C = A * B). Allocates new memory.
 */
MatrixCd multiply(const MatrixCd& A, const MatrixCd& B);

/**
 * @brief HPC in-place matrix multiplication (C_result = A * B).
 */
void multiply_into(const MatrixCd& A, const MatrixCd& B, MatrixCd& C_result);

/**
 * @brief Evolves the correlation matrix in time (C_result = U0 * C0 * U0_dagger).
 */
void evolve_correlation_matrix_into(const MatrixCd& U0, 
                                    const MatrixCd& C0, 
                                    const MatrixCd& U0_dagger, 
                                    MatrixCd& aux_buffer, 
                                    MatrixCd& C_result);

// ================================================================================
// Matrix Diagonalization
// ================================================================================

/**
 * @brief Diagonalizes a Hermitian matrix. Allocates new memory.
 */
std::pair<std::vector<double>, MatrixCd> diagonalize(const MatrixCd& H);

/**
 * @brief HPC in-place diagonalization. Bypasses heap allocation.
 */
void diagonalize_into(const MatrixCd& H, std::vector<double>& eigenvalues, MatrixCd& eigenvectors);

/**
 * @brief Computes the Hermitian Adjoint (Conjugate Transpose) into a pre-allocated buffer.
 */
void adjoint_into(const MatrixCd& A, MatrixCd& A_adj);

// ================================================================================
// Utilities
// ================================================================================

/**
 * @brief Generates a matrix populated with uniform random complex values.
 * 
 * Default boundaries are set here as required by C++ standard.
 */
MatrixCd create_random_matrix(int rows, int cols, std::mt19937& rng, double min_val = -1.0f, double max_val = 1.0f);

/**
 * @brief Extracts a submatrix based on a list of selected indices.
 */
MatrixCd slice_idx(const MatrixCd& mat, const std::vector<int>& idx_selected);