#pragma once
#include <memory>
#include <algorithm>

struct Matrix {
    size_t Rows;
    size_t Columns;
    std::shared_ptr<double[]> ptr;

    Matrix(size_t rows, size_t cols) : Rows(rows), Columns(cols) {
        ptr = std::shared_ptr<double[]>(new double[rows * cols], std::default_delete<double[]>());
    }

    double& at(size_t i, size_t j) {
        return ptr[i * Columns + j];
    }

    const double& at(size_t i, size_t j) const {
        return ptr[i * Columns + j];
    }

    void fill(double value) {
        std::fill(ptr.get(), ptr.get() + Rows * Columns, value);
    }
};
