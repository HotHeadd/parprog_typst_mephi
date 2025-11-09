#include "external.h"
#include "mul_matrix.h"
#include "gen.h"
#include "data.h"

double MeasureFuncSquare(int func_num, Matrix& a, Matrix& b, Matrix& c, int t, int blocks) {
    double sumtime = 0;
    double start, end;
    for (int i=0; i < repeats; ++i) {
        switch (func_num)
        {
        case 0:
            start = omp_get_wtime();
            FoxParallelMultiplication(a.ptr.get(), b.ptr.get(), c.ptr.get(), a.Rows, t);
            end = omp_get_wtime();
            break;
        case 1:
            start = omp_get_wtime();
            cannon_par(a.ptr.get(), b.ptr.get(), c.ptr.get(), a.Rows, blocks);
            end = omp_get_wtime();
            break;
        case 2:
            start = omp_get_wtime();
            mul_matrix(&a, &b, &c, t);
            end = omp_get_wtime();
            break;
        }
        sumtime += end - start;
    }
    sumtime /= repeats;
    sumtime *= 1000;
    return sumtime;
}

void TestForSquare(TGraphDataSq& squareData, Matrix& a, Matrix& b, Matrix& c) {
    for (const auto& [t, block] : threadsAndBlockSize) {
        std::cout << "finished " << t << " threads" << std::endl;
        squareData << t << " ";
        for (int func_num = 0; func_num < 3; ++func_num) {
            c.fill(0);
            squareData << MeasureFuncSquare(func_num, a, b, c, t, block) << " ";
        }
        squareData << "\n";
    }
}

struct Dataset {
    Matrix a;
    Matrix b;
    Matrix c;
};

void TestForRect(TGraphDataRec& recData, std::vector<Dataset>& data) {
    double start, end;
    for (int t = 1; t <= threads; ++t) {
        recData << t << " ";
        for (auto& dataset : data) {
            double sumtime = 0;
            for (int i=0; i < repeats; ++i) {
                dataset.c.fill(0);
                start = omp_get_wtime();
                mul_matrix(&dataset.a, &dataset.b, &dataset.c, t);
                end = omp_get_wtime();
                sumtime += end - start;
            }
            sumtime /= repeats;
            sumtime *= 1000;
            recData << sumtime << " ";
        }
        recData << "\n";
    }
}

int main() {
    init_vectors();
    for (auto& squareData: allSizesSq) {
        Matrix a = GenerateRandom(squareData.rows, squareData.cols);
        Matrix b = GenerateRandom(squareData.rows, squareData.cols);
        Matrix c(squareData.rows, squareData.cols);
        TestForSquare(squareData, a, b, c);
        std::cout << "finished " << squareData.rows << std::endl;
    }
    std::cout << "finished square" << std::endl;
    {// identity
        Matrix a = GenerateRandom(identitySq.rows, identitySq.cols);
        Matrix b = GenerateIdentity(identitySq.rows);
        Matrix c(identitySq.rows, identitySq.cols);
        TestForSquare(identitySq, a, b, c);
    }
    std::cout << "finished identity" << std::endl;
    {// sparse (75% values)
        Matrix a = GenerateRandom(sparseSq.rows, sparseSq.cols, 0.75);
        Matrix b = GenerateSparse(sparseSq.rows, sparseSq.cols, 0.75);
        Matrix c(sparseSq.rows, sparseSq.cols);
        TestForSquare(sparseSq, a, b, c);
    }
    std::cout << "finished sparse" << std::endl;
    {// sparse (25% values)
        Matrix a = GenerateRandom(sparseLowSq.rows, sparseLowSq.cols, 0.25);
        Matrix b = GenerateSparse(sparseLowSq.rows, sparseLowSq.cols, 0.25);
        Matrix c(sparseLowSq.rows, sparseLowSq.cols);
        TestForSquare(sparseLowSq, a, b, c);
    }
    std::cout << "finished sparce low" << std::endl;
    {// bigRandı
        Matrix a = GenerateRandom(randomSq.rows, randomSq.cols, -10000000.0, 10000000.0);
        Matrix b = GenerateRandom(randomSq.rows, randomSq.cols, -10000000.0, 10000000.0);
        Matrix c(randomSq.rows, randomSq.cols);
        TestForSquare(randomSq, a, b, c);
    }
    std::cout << "finished bigrand" << std::endl;
    // rectangles
    for (auto& recData: allRecVec) {
        std::vector<Dataset> data;
        for (const auto& [rows, cols] : recData.mtxs) {
            data.emplace_back(
                GenerateRandom(rows, cols),
                GenerateRandom(cols, rows),
                Matrix(rows, rows)
            );
        }
        TestForRect(recData, data);
        std::cout << "finished " << recData.mtxs[0].first << std::endl;
    }
    std::cout << "finished rect" << std::endl;
    // sparsed rectangles
    std::vector<Dataset> data;
    for (const auto& [rows, cols] : sparseRec.mtxs) {
        data.emplace_back(
            GenerateRandom(rows, cols),
            GenerateRandom(cols, rows),
            Matrix(rows, rows)
        );
    }
    TestForRect(sparseRec, data);
}
