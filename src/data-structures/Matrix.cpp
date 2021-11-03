#include "Matrix.hpp"

Matrix::Matrix(int rows, int cols) : rows(rows), cols(cols) {
    data = new int[rows*cols];
    for(int i = 0; i < rows*cols; i++) data[i] = 0;
}

Matrix::Matrix(int n) : Matrix(n, n) {}

Matrix::Matrix(const Matrix& mat) : rows(mat.rows), cols(mat.cols) {
    data = new int[rows*cols];
    for(int i = 0; i < rows*cols; i++) data[i] = mat.data[i];
}

Matrix::Matrix(Matrix&& mat) : rows(mat.rows), cols(mat.cols), data(mat.data) {
    mat.data = nullptr;
}

void clear(Matrix& mat) {
    for(int i = 0; i < mat.numRows(); i++) {
        for(int j = 0; j < mat.numCols(); j++) {
            mat[i][j] = 0;
        }
    }
}

void printMatrix(std::ostream& out, const Matrix& m) {
    for(int i = 0; i < m.numRows(); ++i) {
        for(int j = 0; j < m.numCols(); ++j) {
            out << m[i][j] << '\t';
        }
        out << std::endl;
    }
}