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

void sequential_exp() {
    std::vector<int> seq_sizes = {
        32, 142, 252, 363, 473,
        584, 694, 804, 914, 1024
    };

    std::ofstream file("results/seq_res");

    file << "1\n" << "Последовательная реализация\n" << "Кол-во строк в кв. матр.\n" << "Время выполнения, мс\n" << "none\n";

    double T1;
    double start, end;
    for (int size : seq_sizes) {
        Matrix a = GenerateRandom(size, size);
        Matrix b = GenerateRandom(size, size);
        Matrix c(size, size);

        double sum_time = 0;
        for (int i=0; i < repeats; ++i) {
            c.fill(0.0);
            start = omp_get_wtime();
            mul_matrix_seq(&a, &b, &c);
            end = omp_get_wtime();
            sum_time += end - start;
        }
        sum_time /= repeats;
        sum_time *= 1000;
        file << size << " " << sum_time << "\n";
    }
}

void multithread_base_exp() {
    size_t size = 1024;
    std::ofstream file("results/mtt_res");
    std::ofstream file_speed("results/mtt_speed");
    std::ofstream file_eff("results/mtt_eff");

    file << "1\n" << "Параллельная реализация\n" << "Кол-во потоков\n" << "Время выполнения, мс\n" << "none\n";
    file_speed << "1\n" << "Параллельная реализация\n" << "Кол-во потоков\n" << "Ускорение\n" << "none\n";
    file_eff << "1\n" << "Параллельная реализация\n" << "Кол-во потоков\n" << "Эффективность\n" << "none\n";

    Matrix a = GenerateRandom(size, size);
    Matrix b = GenerateRandom(size, size);
    Matrix c(size, size);

    double T1;
    double start, end;
    for (int t=1; t <= threads; ++t) {
        double sum_time = 0;
        for (int i=0; i < repeats; ++i) {
            c.fill(0.0);
            start = omp_get_wtime();
            mul_matrix(&a, &b, &c, t);
            end = omp_get_wtime();
            sum_time += end - start;
        }
        sum_time /= repeats;
        sum_time *= 1000;
        if (t == 1) {
            T1 = sum_time;
        }
        file << t << " " << sum_time << "\n";
        file_speed << t << " " << T1 / sum_time << "\n";
        file_eff << t << " " << T1/sum_time/t << "\n";
    }
}

int main() {
    sequential_exp();
    std::cout << "finished seq " << std::endl;
    multithread_base_exp();
    std::cout << "finished mtt" << std::endl;
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
    std::cout << "finished sparse low" << std::endl;
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
