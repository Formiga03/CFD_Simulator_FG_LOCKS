
#ADDED FOR TESTS DIRECTORY
cmake_minimum_required(VERSION 3.24)
project(MatrixProject
    VERSION 0.0.1
    LANGUAGES CXX
)

# 1. Enforce modern C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 2. Define your production library target
# Since Matrix<T> is template-based, use an INTERFACE library
add_library(matrix_lib INTERFACE)

# Expose the include/ folder to anything linking against matrix_lib
target_include_directories(matrix_lib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 3. Handle testing conditionally
# include(CTest) automatically provides the BUILD_TESTING option (ON by default)
include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()