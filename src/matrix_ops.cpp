/**
 * @file matrix_ops.cpp
 * @brief Core linear algebra operations for MIPT QuantumSim.
 * 
 * This module provides high-performance wrappers around OpenBLAS and LAPACKE.
 * It features both standard (allocating) functions for initialization and 
 * highly optimized, zero-allocation "in-place" functions for hot simulation loops.
 */

#include "matrix_ops.hpp"
#include <cblas.h>
#include <lapacke.h>
#include <stdexcept>
#include <complex>
#include <utility>
#include <algorithm> 

// ================================================================================
// Matrix Multiplication
// ================================================================================

/**
 * @brief Standard matrix multiplication (C = A * B).
 * 
 * Allocates and returns a new MatrixCd. Ideal for initialization or 
 * single-use calculations outside of hot loops.
 */
MatrixCd multiply(const MatrixCd& A, const MatrixCd& B) {
    // Inner dimensions must match for valid matrix multiplication
    if (A.cols != B.rows) {
        throw std::invalid_argument("Matrix dimensions do not match for multiplication.");
    }

    // Heap allocation: Creates a new matrix for the result
    MatrixCd C(A.rows, B.cols);

    // BLAS scaling formula: C = alpha * (A * B) + beta * C
    // To achieve pure C = A * B, we set alpha = 1.0 and beta = 0.0
    std::complex<double> alpha(1.0f, 0.0f);
    std::complex<double> beta(0.0f, 0.0f);

    cblas_zgemm(
        CblasRowMajor,       // Use 1D flat array mapping
        CblasNoTrans,        // Matrix A is not transposed
        CblasNoTrans,        // Matrix B is not transposed
        A.rows, B.cols, A.cols,
        &alpha,
        A.data.data(), A.cols,
        B.data.data(), B.cols,
        &beta,
        C.data.data(), C.cols
    );

    return C;
}

/**
 * @brief HPC in-place matrix multiplication (C_result = A * B).
 * 
 * Overwrites the memory of C_result. Prevents expensive OS-level memory 
 * allocations during time-evolution loops, maximizing CPU cache efficiency.
 */
void multiply_into(const MatrixCd& A, const MatrixCd& B, MatrixCd& C_result) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Matrix dimensions do not match for multiplication.");
    }

    // Safety fallback: Resize C_result only if dimensions are incorrect.
    // In a properly structured hot loop, this branch is never taken.
    if (C_result.rows != A.rows || C_result.cols != B.cols) {
        C_result = MatrixCd(A.rows, B.cols);
    }

    std::complex<double> alpha(1.0f, 0.0f);
    std::complex<double> beta(0.0f, 0.0f); // beta = 0.0 forces total memory overwrite of C_result

    cblas_zgemm(
        CblasRowMajor, CblasNoTrans, CblasNoTrans,
        A.rows, B.cols, A.cols,
        &alpha,
        A.data.data(), A.cols,
        B.data.data(), B.cols,
        &beta,
        C_result.data.data(), C_result.cols
    );
}

/**
 * @brief Evolves the correlation matrix in time (C_result = U0 * C0 * U0_dagger).
 * 
 * Performs the double matrix multiplication required for unitary time evolution. 
 * Requires a pre-allocated auxiliary buffer to hold the intermediate state, 
 * guaranteeing zero OS-level memory allocations during the Monte Carlo loop.
 * 
 * @param U0         The unitary time-evolution matrix.
 * @param C0         The initial correlation matrix (or current state).
 * @param U0_dagger  The adjoint (conjugate transpose) of the unitary matrix.
 * @param aux_buffer A pre-allocated temporary matrix of size (rows x cols).
 * @param C_result   [OUTPUT] The evolved correlation matrix.
 */
void evolve_correlation_matrix_into(const MatrixCd& U0, 
                                           const MatrixCd& C0, 
                                           const MatrixCd& U0_dagger, 
                                           MatrixCd& aux_buffer, 
                                           MatrixCd& C_result) {
    
    // Step 1: aux_buffer = U0 * C0
    // Computes the left side of the time evolution
    multiply_into(U0, C0, aux_buffer);

    // Step 2: C_result = aux_buffer * U0_dagger
    // Computes the right side and writes directly into the final output matrix
    multiply_into(aux_buffer, U0_dagger, C_result);
}


// ================================================================================
// Matrix Diagonalization
// ================================================================================
/**
 * @brief Diagonalizes a Hermitian matrix.
 * 
 * Allocates new memory for the eigenvalues and eigenvectors.
 * 
 * @note LAPACK physically overwrites the input matrix to save memory. 
 * This function handles the deep copy automatically to preserve the original.
 */
std::pair<std::vector<double>, MatrixCd> diagonalize(const MatrixCd& H) {
    if (H.rows != H.cols) {
        throw std::invalid_argument("Matrix must be square to be diagonalized.");
    }
    
    int N = H.rows;
    
    std::vector<double> eigenvalues(N);
    MatrixCd eigenvectors = H; // Triggers deep copy of the original Hamiltonian

    // LAPACKE_zheev: double-precision complex Hermitian eigenvalues
    int info = LAPACKE_zheev(
        LAPACK_ROW_MAJOR,
        'V', // eigenvalues + eigenvectors
        'U', // upper triangle
        N,
        reinterpret_cast<lapack_complex_double*>(eigenvectors.data.data()), N,
        eigenvalues.data()
    );

    if (info > 0) throw std::runtime_error("LAPACKE_cheev failed to converge.");
    if (info < 0) throw std::invalid_argument("LAPACKE_cheev parameter error.");

    return std::make_pair(eigenvalues, eigenvectors);
}

/**
 * @brief HPC in-place diagonalization.
 * 
 * Overwrites pre-allocated `eigenvalues` and `eigenvectors` objects. 
 * Bypasses heap allocation for maximum performance in iterative Monte Carlo algorithms.
 */
 void diagonalize_into(const MatrixCd& H, std::vector<double>& eigenvalues, MatrixCd& eigenvectors) {
    try {
        if (H.rows != H.cols) {
            throw std::invalid_argument("Matrix must be square to be diagonalized.");
        }
        
        int N = H.rows;

        if (eigenvalues.size() != static_cast<size_t>(N)) {
            eigenvalues.resize(N);
        }
        if (eigenvectors.rows != N || eigenvectors.cols != N) {
            eigenvectors = MatrixCd(N, N); 
        }

        std::copy(H.data.begin(), H.data.end(), eigenvectors.data.begin());

        int info = LAPACKE_zheev(
            LAPACK_ROW_MAJOR, 'V', 'U', N,
            reinterpret_cast<lapack_complex_double*>(eigenvectors.data.data()), N,
            eigenvalues.data()
        );

        if (info > 0) throw std::runtime_error("LAPACKE_cheev failed to converge.");
        // We append the INFO code here so you know exactly which parameter LAPACKE rejected
        if (info < 0) throw std::invalid_argument("LAPACKE_cheev parameter error. INFO CODE = " + std::to_string(info));

    } catch (const std::exception& e) {
        #pragma omp critical
        {
            std::cerr << "\n[CRASH IN LAPACKE MATH]: " << e.what() << "\n";
        }
        throw;
    }
}
/**
 * @brief Computes the Hermitian Adjoint (Conjugate Transpose) of a matrix.
 * 
 * Performance Optimization:
 * -----------------------
 * Uses Cache Blocking (Tiling) to prevent cache thrashing during the transpose.
 * By processing the matrix in 32x32 chunks, both the read and write operations 
 * stay strictly within the CPU's L1 cache, yielding massive speedups for large matrices.
 * 
 * @param A The input matrix.
 * @param A_adj [OUTPUT] Reusable buffer for the resulting adjoint matrix.
 */
void adjoint_into(const MatrixCd& A, MatrixCd& A_adj) {
    
    A_adj.resize(A.cols, A.rows); 

    // L1 Cache lines usually hold 64 bytes. A 32x32 block of std::complex<double> (8 bytes each)
    // is 8KB, which easily fits inside modern 32KB or 64KB L1 caches.
    constexpr int BLOCK_SIZE = 32;

    // Outer loops iterate over the blocks
    for (int r = 0; r < A.rows; r += BLOCK_SIZE) {
        for (int c = 0; c < A.cols; c += BLOCK_SIZE) {
            
            // Handle the boundary edges if the matrix size isn't a perfect multiple of 32
            int r_max = std::min(r + BLOCK_SIZE, A.rows);
            int c_max = std::min(c + BLOCK_SIZE, A.cols);
            
            // Inner loops iterate within the specific L1 cache block
            for (int rr = r; rr < r_max; ++rr) {
                for (int cc = c; cc < c_max; ++cc) {
                    A_adj(cc, rr) = std::conj(A(rr, cc));
                }
            }
            
        }
    }
}

// ================================================================================
// Utilities
// ================================================================================

/**
 * @brief Generates a matrix populated with uniform random complex values.
 * 
 * Uses a flat 1D memory traversal to maintain continuous cache access lines.
 * Default min/max boundaries are declared in the header file.
 */
MatrixCd create_random_matrix(int rows, int cols, std::mt19937& rng, double min_val, double max_val) {
    MatrixCd M(rows, cols);
    std::uniform_real_distribution<double> dist(min_val, max_val);
    
    int total_elements = rows * cols;
    
    // Flat memory traversal is significantly faster than nested 2D loops
    for (int i = 0; i < total_elements; ++i) {
        double real_part = dist(rng);
        double imag_part = dist(rng);
        M.data[i] = std::complex<double>(real_part, imag_part);
    }
    
    return M;
}

/**
 * @brief Extracts a submatrix based on a list of selected indices.
 */
MatrixCd slice_idx(const MatrixCd& mat, const std::vector<int>& idx_selected) {

    int num_idx = idx_selected.size();
    MatrixCd out_mat(num_idx, num_idx);

    for (int ii = 0; ii < num_idx; ii++) {
        int ii_idx = idx_selected[ii]; 
        
        // jj starts at ii + 1 to handle the upper triangle only
        for (int jj = ii + 1; jj < num_idx; jj++) { // Fixed num_idxM typo
            int jj_idx = idx_selected[jj]; 

            // WRITE to the new coordinates (ii, jj)
            // READ from the original coordinates (ii_idx, jj_idx) inside mat.data
            out_mat(ii, jj) = mat(ii_idx, jj_idx);
            out_mat(jj, ii) = mat(jj_idx, ii_idx);
        }
        
        // Handle the diagonal separately
        out_mat(ii, ii) = mat(ii_idx, ii_idx);
    }

    return out_mat;
}
