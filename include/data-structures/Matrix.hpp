#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "Point.hpp"
#include <iostream>

class Matrix {
public:
    Matrix(int rows, int cols);
    Matrix(int n); // for square matrices
    Matrix(const Matrix& mat); // copy constructor
    Matrix(Matrix&& mat); // move constructor

    // disable Matrix assignment
    Matrix& operator=(const Matrix&) = delete;
    Matrix& operator=(Matrix&&) = delete;

    ~Matrix() {
        delete[] data;
    }

    int numRows() const { return rows; }
    int numCols() const { return cols; }

    // these access operators do not perform bounds checking
    int* operator[](int i) { return data + (i*cols); }
    const int* operator[](int i) const { return data + (i*cols); }
    int& operator[](const Point& p) { return (*this)[p.y][p.x]; }
    int operator[](const Point& p) const { return (*this)[p.y][p.x]; }

private:
    int* data;
    int rows, cols;
};

void clear(Matrix& mat);

void printMatrix(std::ostream& out, const Matrix& m);

#endif