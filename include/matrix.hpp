#pragma once

#include <vector>
#include <complex>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>

/**
 * @struct column
 * @brief A lightweight, non-owning strided view representing a matrix column.
 * 
 * This structure provides zero-allocation, O(1) access to a column within a 
 * row-major matrix. By stepping across memory using the row stride, it avoids 
 * the need to copy elements into a new contiguous buffer.
 * 
 * @tparam T The numeric data type (e.g., float, std::complex<float>).
 */
template<typename T>
struct column {
    int col_length;  ///< Number of elements in the column (number of rows in the matrix).
    int row_length;  ///< The memory stride to the next element (number of columns in the matrix).
    T* mem_add;      ///< Pointer to the 0th element of this specific column.

    /**
     * @brief Constructs a strided column view.
     * 
     * @param c_l The total number of elements in the column.
     * @param r_l The stride length to jump to the next row's element.
     * @param col_beginning A raw pointer to the start of the column.
     */
    column(int c_l, int r_l, T* col_beginning) 
        : col_length(c_l), row_length(r_l), mem_add(col_beginning) {}

    // =========================================================
    // 1. FAST ACCESS (Use this in your tight physics loops)
    // =========================================================
    
    /**
     * @brief Accesses an element in the column without overhead (Write-enabled).
     * 
     * Uses assert() for bounds checking, which is entirely removed by the compiler 
     * in Release mode (-O3 -DNDEBUG) for maximum performance.
     * 
     * @param col_element The 0-based index down the column.
     * @return T& A mutable reference to the physical memory element.
     */
    T& operator()(int col_element) {
        assert(col_element >= 0 && col_element < col_length);
        return *(mem_add + (row_length * col_element));
    }    

    /**
     * @brief Accesses an element in the column without overhead (Read-only).
     */
    const T& operator()(int col_element) const {
        assert(col_element >= 0 && col_element < col_length);
        return *(mem_add + (row_length * col_element));
    } 

    // =========================================================
    // 2. SAFE ACCESS (Use this for testing/debugging)
    // =========================================================
    
    /**
     * @brief Accesses an element with guaranteed bounds checking (Write-enabled).
     * 
     * @param col_element The 0-based index down the column.
     * @return T& A mutable reference to the physical memory element.
     * @throws std::out_of_range If the index is out of bounds.
     */
    T& at(int col_element) {
        if (col_element < 0 || col_element >= col_length) {
            throw std::out_of_range("Column index out of bounds.");
        }
        return *(mem_add + (row_length * col_element));
    }

    /**
     * @brief Accesses an element with guaranteed bounds checking (Read-only).
     */
    const T& at(int col_element) const {
        if (col_element < 0 || col_element >= col_length) {
            throw std::out_of_range("Column index out of bounds.");
        }
        return *(mem_add + (row_length * col_element));
    }
};

/**
 * @struct Matrix
 * @brief A high-performance, contiguous-memory dense matrix structure.
 * 
 * This template structure is designed for High-Performance Computing (HPC) and 
 * seamless GPU migration. By enforcing a flat 1D std::vector backing, it guarantees 
 * strict row-major memory alignment. When instantiated with std::complex<float>, 
 * it is binary-compatible with CUDA's cuFloatComplex, allowing direct memory 
 * copies (cudaMemcpy) without marshalling overhead.
 * 
 * @tparam T The numeric data type (e.g., float, double, std::complex<float>).
 */
template <typename T>
struct Matrix {
    int rows;                  ///< Number of rows in the matrix.
    int cols;                  ///< Number of columns in the matrix.
    std::vector<T> data;       ///< Flat contiguous memory storage (row-major order).

    // =========================================================
    // 1. Constructors & Lifecycle
    // =========================================================

    /**
     * @brief Constructs a zero-initialized Matrix with specified dimensions.
     * 
     * @param r Number of rows.
     * @param c Number of columns.
     */
    Matrix(int r, int c) 
        : rows(r), cols(c), data(r * c, T(0)) {}
         
    /**
     * @brief Constructs a Matrix by taking ownership of a pre-existing 1D vector.
     * 
     * @param r Number of rows.
     * @param c Number of columns.
     * @param vct_dt The 1D vector representing the flattened matrix data.
     * @throws std::invalid_argument If the provided vector size does not equal r * c.
     */
    Matrix(int r, int c, const std::vector<T>& vct_dt) 
        : rows(r), cols(c), data(vct_dt) {
        if (vct_dt.size() != static_cast<size_t>(r * c)) {
            throw std::invalid_argument("Vector size does not match matrix dimensions.");
        }
    }

    /**
     * @brief Deep copy constructor.
     * 
     * @param Mat The source Matrix to copy.
     */
    Matrix(const Matrix<T>& Mat) 
        : rows(Mat.rows), cols(Mat.cols), data(Mat.data) {}

    /**
     * @brief Default constructor for delayed initialization.
     * 
     * Creates an empty 0x0 matrix. Primarily used when declaring reusable 
     * buffers in a zero-allocation "_into" architecture before resizing them.
     */
    Matrix() : rows(0), cols(0), data() {}

    // =========================================================
    // 2. Core Memory Management
    // =========================================================

    /**
     * @brief Physically zeroes out the underlying memory buffer.
     */
    void zero() {
        std::fill(data.begin(), data.end(), T(0));
    }

    /**
     * @brief Resizes the matrix geometry.
     * 
     * @param r The new number of rows.
     * @param c The new number of columns.
     */
    void resize(int r, int c) {
        rows = r;
        cols = c;
        if (data.size() != static_cast<size_t>(r * c)) {
            data.resize(r * c, T(0));
        }
    }

    void add_padding(int r_num, int c_num) {
        rows = r_num;
        cols = c_num;
        data.resize(r_num*c_num);
    }

    // =========================================================
    // 3. Coordinate Mapping & Element Access
    // =========================================================

    /**
     * @brief Maps 2D coordinates to a 1D flat memory index.
     * 
     * @param r The 0-based row index.
     * @param c The 0-based column index.
     * @return int The corresponding 1D index in the underlying data vector.
     */
    inline int idx(int r, int c) const {
        return r * cols + c;
    }

    /**
     * @brief 2D coordinate access operator (Write-enabled).
     */
    T& operator()(int row, int col) {
        return data[idx(row, col)];
    }

    /**
     * @brief 2D coordinate access operator (Read-only).
     */
    const T& operator()(int row, int col) const {
        return data[idx(row, col)];
    }

    /**
     * @brief Move Constructor (Zero Allocation Transfer)
     */
    Matrix(Matrix<T>&& other) noexcept 
        : rows(other.rows), cols(other.cols), data(std::move(other.data)) {
        other.rows = 0;
        other.cols = 0;
    }

    /**
     * @brief Move Assignment Operator (Zero Allocation Transfer)
     */
    Matrix<T>& operator=(Matrix<T>&& other) noexcept {
        if (this != &other) {
            rows = other.rows;
            cols = other.cols;
            data = std::move(other.data); // Steals the 1D flat vector instantly
            
            other.rows = 0;
            other.cols = 0;
        }
        return *this;
    }

    // =========================================================
    // 4. Memory Slicing & Views
    // =========================================================

    /**
     * @brief Extracts an entire row into a newly allocated vector.
     * 
     * @param row_order The 0-based index of the target row.
     * @return std::vector<T> A new vector containing the row data.
     * @throws std::invalid_argument If the requested row is out of bounds.
     */
    std::vector<T> row(int row_order) const {
        if (row_order < 0 || row_order >= rows) {
            throw std::invalid_argument("Row Order Number does not match matrix dimensions.");
        }

        int init_idx = idx(row_order, 0); 
        int end_idx = init_idx + cols; 

        return std::vector<T>(data.begin() + init_idx, data.begin() + end_idx);
    }

    /**
     * @brief Extracts an entire column into a newly allocated vector.
     * 
     * @param col_order The 0-based index of the target column.
     * @return std::vector<T> A new vector containing the column data.
     * @throws std::invalid_argument If the requested column is out of bounds.
     */
    std::vector<T> col(int col_order) const {
        if (col_order < 0 || col_order >= cols) {
            throw std::invalid_argument("Column Order Number does not match matrix dimensions.");
        }
        
        std::vector<T> res(rows);
        for (int ii = 0; ii < rows; ii++) {
            res[ii] = data[idx(ii, col_order)];
        }
        return res;
    }

    /**
     * @brief Extracts a column directly into a pre-allocated vector.
     * 
     * @param col_order The 0-based index of the column to extract.
     * @param out_vec [OUTPUT] Reference to an existing vector to be overwritten. 
     * @throws std::invalid_argument If the requested column is out of bounds.
     */
    void col_into(int col_order, std::vector<T>& out_vec) const {
        if (col_order < 0 || col_order >= cols) {
            throw std::invalid_argument("Column Order Number does not match matrix dimensions.");
        }
    
        if (out_vec.size() != static_cast<size_t>(rows)) {
            out_vec.resize(rows); 
        }

        for (int ii = 0; ii < rows; ii++) {
            out_vec[ii] = data[idx(ii, col_order)];
        }
    }

    /**
     * @brief Returns a zero-allocation, strided view of a specific column.
     * 
     * @param col_order The 0-based index of the requested column.
     * @return column<T> A non-owning strided view of the column.
     * @throws std::invalid_argument If the requested column is out of bounds.
     */
    column<T> col_view(int col_order) {
        if (col_order < 0 || col_order >= cols) {
            throw std::invalid_argument("Column Order Number does not match matrix dimensions.");
        }
        return column<T>(rows, cols, data.data() + col_order);
    }


    /**
     * @brief Extracts an arbitrary submatrix into a pre-allocated buffer.
     *
     * Equivalent to NumPy's @c A[np.ix_(row_idx, col_idx)] : the result has one
     * row per entry of @p row_idx and one column per entry of @p col_idx, with
     * element @f$(i,j)@f$ taken from @f$A(\text{row\_idx}[i],\ \text{col\_idx}[j])@f$.
     * Indices may repeat and need not be sorted.
     *
     * @param[in]  row_idx Row indices to keep, in the order they should appear.
     * @param[in]  col_idx Column indices to keep, in the order they should appear.
     * @param[out] out     Reusable buffer, resized to row_idx.size() x col_idx.size().
     * @throws std::out_of_range If any index falls outside the matrix.
     *
     * @note Rows are copied contiguously from the source, so the inner loop is
     *       cache-friendly whenever @p col_idx is ascending.
     */
    void submatrix_into(const std::vector<int>& row_idx,
                        const std::vector<int>& col_idx,
                        Matrix<T>& out) const
    {
        const int nr = static_cast<int>(row_idx.size());
        const int nc = static_cast<int>(col_idx.size());
        out.resize(nr, nc);

        for (int i = 0; i < nr; ++i) {

            const int r = row_idx[i];
            if (r < 0 || r >= rows) {
                throw std::out_of_range("submatrix_into: row index out of bounds.");
            }

            const T* src = data.data() + static_cast<size_t>(r) * cols;
            T*       dst = out.data.data() + static_cast<size_t>(i) * nc;

            for (int j = 0; j < nc; ++j) {
                const int c = col_idx[j];
                if (c < 0 || c >= cols) {
                    throw std::out_of_range("submatrix_into: column index out of bounds.");
                }
                dst[j] = src[c];
            }
        }
    }

    /**
     * @brief Extracts a symmetric submatrix into a pre-allocated buffer.
     *
     * Equivalent to @c A[np.ix_(indices, indices)] : the same index list is applied
     * to both axes. This is the form used to restrict a correlation matrix to a
     * subsystem, @f$C_{A} = C[\mathcal{A}, \mathcal{A}]@f$.
     *
     * @param[in]  indices Subsystem site indices.
     * @param[out] out     Reusable buffer, resized to indices.size() x indices.size().
     * @throws std::out_of_range If any index falls outside the matrix.
     */
    void submatrix_into(const std::vector<int>& indices, Matrix<T>& out) const {
        submatrix_into(indices, indices, out);
    }

    /**
     * @brief Returns an arbitrary submatrix as a new object.
     *
     * @warning Triggers a heap allocation. In tight loops use submatrix_into().
     * @param[in] row_idx Row indices to keep.
     * @param[in] col_idx Column indices to keep.
     * @return Matrix<T> A newly allocated submatrix.
     */
    Matrix<T> submatrix(const std::vector<int>& row_idx,
                        const std::vector<int>& col_idx) const
    {
        Matrix<T> res;                          // delayed initialization
        this->submatrix_into(row_idx, col_idx, res);
        return res;
    }

    /**
     * @brief Returns a symmetric submatrix as a new object.
     *
     * @warning Triggers a heap allocation. In tight loops use submatrix_into().
     * @param[in] indices Subsystem site indices, applied to both axes.
     * @return Matrix<T> A newly allocated submatrix.
     */
    Matrix<T> submatrix(const std::vector<int>& indices) const {
        Matrix<T> res;
        this->submatrix_into(indices, indices, res);
        return res;
    }

    // =========================================================
    // 5. Mathematical Operations
    // =========================================================

    /**
     * @brief Computes the trace of the matrix (sum of the main diagonal).
     * 
     * @return T The computed trace value.
     */
    T trace() const {
        T trace_sum = T(0);
        int min_dim = std::min(rows, cols);
        
        for (int i = 0; i < min_dim; ++i) {
            trace_sum += data[idx(i, i)];
        }
        return trace_sum;
    }

    /**
     * @brief Populates a specified diagonal with a uniform value.
     * 
     * @param order_of_diag 0 for main diagonal, >0 for superdiagonals, <0 for subdiagonals.
     * @param diag_val The uniform value to assign to the target diagonal.
     */
    void diag(int order_of_diag, T diag_val) {
        int offset = std::abs(order_of_diag);
        if (offset >= rows && offset >= cols) return; 

        if (order_of_diag >= 0) {
            int num_diag_elements = std::min(rows, cols - offset);
            for (int ii = 0; ii < num_diag_elements; ++ii) {
                data[idx(ii, ii + offset)] = diag_val;
            }
        } else {
            int num_diag_elements = std::min(rows - offset, cols);
            for (int ii = 0; ii < num_diag_elements; ++ii) {
                data[idx(ii + offset, ii)] = diag_val;
            }
        }   
    }

/**
     * @brief Populates the main diagonal of the matrix using a provided vector.
     * 
     * @param diag_vct A vector containing the diagonal elements. Passed by const 
     *                 reference to avoid expensive memory copies.
     */
    void diag_into(const std::vector<T>& diag_vct) {
        
        // Safety check: Ensure we don't read past the end of the vector OR the matrix
        int min_val = std::min({rows, cols, static_cast<int>(diag_vct.size())});
        
        for (int ii = 0; ii < min_val; ++ii) {
            data[idx(ii, ii)] = diag_vct[ii];
        }
    }

    /**
     * @brief Populates the main diagonal with the complex exponential e^(i * theta).
     * 
     * @param diag_vct A vector of real phases (theta). Passed by const reference.
     */
    void diag_exp_into(const std::vector<float>& diag_vct) {
        
        // Safety check: Ensure we don't read past the end of the vector OR the matrix
        int min_val = std::min({static_cast<int>(rows), static_cast<int>(cols), static_cast<int>(diag_vct.size())});
        
        for (int ii = 0; ii < min_val; ++ii) {
            // std::polar(magnitude, phase) computes exactly magnitude * e^(i * phase)
            data[idx(ii, ii)] = static_cast<T>(std::polar(1.0f, diag_vct[ii]));
        }
    }

    /**
     * @brief Computes the transpose of the matrix using a zero-allocation buffer.
     * 
     * Uses Cache Blocking (Tiling) to prevent cache thrashing during the transpose, 
     * ensuring maximum L1 cache efficiency for large matrices.
     * 
     * @param out [OUTPUT] Reusable matrix buffer to store the transposed result.
     */
    void transpose_into(Matrix<T>& out) const {
        out.resize(cols, rows); 
        
        // 32x32 blocks fit perfectly into modern L1 CPU caches
        constexpr int BLOCK_SIZE = 32;

        for (int r = 0; r < rows; r += BLOCK_SIZE) {
            for (int c = 0; c < cols; c += BLOCK_SIZE) {
                
                int r_max = std::min(r + BLOCK_SIZE, rows);
                int c_max = std::min(c + BLOCK_SIZE, cols);
                
                for (int rr = r; rr < r_max; ++rr) {
                    for (int cc = c; cc < c_max; ++cc) {
                        out(cc, rr) = (*this)(rr, cc);
                    }
                }
            }
        }
    }

    /**
     * @brief Computes and returns the transpose of the matrix.
     * 
     * @warning Triggers a heap allocation. For heavy loops, use transpose_into().
     * @return Matrix<T> A newly allocated transposed matrix.
     */
    Matrix<T> transpose() const {
        Matrix<T> res;               // Triggers delayed initialization
        this->transpose_into(res);   // Utilizes the optimized tiled logic
        return res;
    }

    /**
     * @brief Modifies the matrix in-place, replacing every element with its complex conjugate.
     * 
     * For purely real matrices (e.g., Matrix<float>), this operation mathematically 
     * does nothing. The static_cast ensures that the C++ compiler safely resolves 
     * std::conj(float) back to a standard float without throwing a type-mismatch error.
     */
    void conjugate_in_place() {
        int total_elements = rows * cols;
        for (int i = 0; i < total_elements; ++i) {
            data[i] = static_cast<T>(std::conj(data[i]));
        }
    }

    /**
     * @brief Returns a newly allocated matrix where every element is complex conjugated.
     * 
     * @return Matrix<T> A new conjugated matrix.
     */
    Matrix<T> conjugate() const {
        Matrix<T> res(*this);       // Utilize the existing deep-copy constructor
        res.conjugate_in_place();   // Conjugate the copy
        return res;
    }

    // =========================================================
    // 6. Input/Output & Utilities
    // =========================================================
  
    /**
     * @brief Overloads the stream insertion operator for formatted terminal output.
     */
    friend std::ostream& operator<<(std::ostream& os, const Matrix<T>& matrix) {
        int total_elements = matrix.rows * matrix.cols;
        
        // 1. Save the stream's current formatting state
        std::streamsize old_prec = os.precision();
        std::ios_base::fmtflags old_flags = os.flags();
        
        // 2. Lock in high precision specifically for matrix debugging
        os << std::fixed << std::setprecision(6);
        
        for (int i = 0; i < total_elements; ++i) {
            if (i % matrix.cols == 0) os << "[ ";
            
            os << std::setw(12) << matrix.data[i] << " ";
            
            if ((i + 1) % matrix.cols == 0) os << "]\n";
        }
        
        // 3. Restore the original formatting state so other prints aren't affected
        os.flags(old_flags);
        os.precision(old_prec);
        
        return os;
    }

    /**
     * @brief Copy assignment operator.
     * 
     * Safely copies dimensions and memory from another matrix. 
     * Supports chained assignment (A = B = C).
     */
    Matrix<T>& operator=(const Matrix<T>& other) {
        // 1. Self-assignment guard: prevents crashing if someone does 'A = A;'
        if (this == &other) {
            return *this;
        }

        // 2. Copy the dimensions
        rows = other.rows;
        cols = other.cols;

        // 3. Copy the actual data
        // std::vector's own assignment operator automatically handles checking 
        // capacity, resizing if necessary, and copying the physical memory block.
        data = other.data;

        // 4. Return a reference to this object to allow chained assignments
        return *this;
    }

};

// ==============================================================================
// TYPE ALIASES
// ==============================================================================
// These standard type definitions allow legacy code to drop in the templated 
// matrix without requiring widespread refactoring.

using Matrixf  = Matrix<float>;                  ///< Alias for a real-valued floating-point matrix
using MatrixCf = Matrix<std::complex<float>>;    ///< Alias for a complex-valued floating-point matrix
using Matrixd  = Matrix<double>;                 ///< Alias for a real-valued double-precision matrix
using MatrixCd = Matrix<std::complex<double>>;   ///< Alias for a complex-valued double-precision matrix