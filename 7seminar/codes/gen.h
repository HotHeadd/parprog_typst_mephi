#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include "mtx.h"


Matrix GenerateRandom(size_t rows, size_t cols, double from = -10, double to = 10) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> value(from, to);

    Matrix m(rows, cols);
    for (size_t i = 0; i < rows * cols; ++i)
        m.ptr[i] = value(gen);
    return m;
}

Matrix GenerateIdentity(size_t size) {
    Matrix m(size, size);
    m.fill(0.0);
    for (size_t i = 0; i < size; ++i)
        m.at(i, i) = 1.0;
    return m;
}

Matrix GenerateSparse(size_t rows, size_t cols, double density = 0.05,
                       double from = -10, double to = 10) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    std::uniform_real_distribution<double> value(from, to);

    Matrix m(rows, cols);
    for (size_t i = 0; i < rows * cols; ++i)
        m.ptr[i] = (prob(gen) < density) ? value(gen) : 0.0;
    return m;
}
