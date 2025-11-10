#include <vector>
#include <fstream>
#include <memory>
#include <string>
#include <filesystem>
#include <utility>
#include <iostream>

std::vector<std::pair<int, int>> threadsAndBlockSize = {
    {1, 1},
    {4, 2},
    {9, 3},
    {16, 4},
    {25, 5}
};

int threads = 32;
int repeats = 20;

struct TGraphDataSq {
    size_t rows;
    size_t cols;
    std::string filename;
    std::unique_ptr<std::ofstream> file;

    TGraphDataSq(size_t rows_, size_t cols_, const std::string& filename)
        : rows(rows_), cols(cols_), filename(filename)
    {
        std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
        file = std::make_unique<std::ofstream>(filename);
        if (!file->is_open()) {
            std::cerr << "Ошибка: не удалось открыть файл " << filename << "\n";
            return;
        }
        *file << "3\n"
              << "Fox\n"
              << "Cannon\n"
              << "Networks\n"
              << "Кол-во потоков\n"
              << "Время выполнения, мс\n"
              << "none\n";
    }

    TGraphDataSq(const TGraphDataSq&) = delete;
    TGraphDataSq& operator=(const TGraphDataSq&) = delete;
    TGraphDataSq(TGraphDataSq&&) noexcept = default;
    TGraphDataSq& operator=(TGraphDataSq&&) noexcept = default;

    template <typename T>
    TGraphDataSq& operator<<(const T& value) {
        if (file && file->is_open()) {
            *file << value;
        }
        return *this;
    }
};

struct TGraphDataRec {
    std::vector<std::pair<size_t, size_t>> mtxs;
    std::string filename;
    std::unique_ptr<std::ofstream> file;

    TGraphDataRec(const std::vector<std::pair<size_t, size_t>>& mtxs_, const std::string& filename_)
        : mtxs(mtxs_), filename(filename_)
    {
        std::filesystem::create_directories(std::filesystem::path(filename_).parent_path());
        file = std::make_unique<std::ofstream>(filename_);
        if (!file->is_open()) {
            std::cerr << "Ошибка: не удалось открыть файл " << filename_ << "\n";
            return;
        }
        *file << mtxs.size() << "\n";
        for (const auto& mtx : mtxs) {
            *file << mtx.first << "x" << mtx.second << "\n";
        }
        *file << "Кол-во потоков\n"
              << "Время выполнения, мс\n"
              << "none\n";
    }

    TGraphDataRec(const TGraphDataRec&) = delete;
    TGraphDataRec& operator=(const TGraphDataRec&) = delete;
    TGraphDataRec(TGraphDataRec&&) noexcept = default;
    TGraphDataRec& operator=(TGraphDataRec&&) noexcept = default;

    template <typename T>
    TGraphDataRec& operator<<(const T& value) {
        if (file && file->is_open()) {
            *file << value;
        }
        return *this;
    }
};

std::vector<TGraphDataSq> allSizesSq;
std::vector<TGraphDataRec> allRecVec;
void init_vectors() {
    allSizesSq.emplace_back(60, 60, "results/sq1_small");
    allSizesSq.emplace_back(480, 480, "results/sq2_middle");
    allSizesSq.emplace_back(1020, 1020, "results/sq3_large");
    allSizesSq.emplace_back(2040 , 2040, "results/sq4_mega");

    allRecVec.emplace_back(
        std::vector<std::pair<size_t, size_t>>{ {1024, 64}, {64, 1024}, {256, 256} },
        "results/rec1_small"
    );
    allRecVec.emplace_back(
        std::vector<std::pair<size_t, size_t>>{ {2048, 124}, {124, 2048}, {512, 512} },
        "results/rec2_big"
    );
}

TGraphDataSq identitySq   = {1020, 1020, "results/sq5_identity"};
TGraphDataSq sparseSq     = {1020, 1020, "results/sq6_sparse"};
TGraphDataSq sparseLowSq  = {1020, 1020, "results/sq7_sparse_low"};
TGraphDataSq randomSq     = {1020, 1020, "results/sq8_bigrandom"};

TGraphDataRec sparseRec = {
    {
        {64, 1024},
        {1024, 64},
        {512, 512}
    },
    "results/rec3_sparse"
};
