#include "gen.h"
#include "shell.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <optional>
#include <omp.h>
#include <filesystem>

namespace fs = std::filesystem;

using TResults = std::vector<std::vector<double>>;
using TResultLine = std::vector<double>;
using TData = std::vector<std::vector<std::unique_ptr<int32_t[]>>>;
using TSeries = std::vector<std::unique_ptr<int32_t[]>>;

int32_t elems = pow(10, 6);
int32_t arrays = 100;
int threads = 12;

TData   randomData, // full, 1%
        fullySortedData, // sorted, reverted
        partiallySortedData, // 25, 50, 75
        locallySortedData; // 10, 1000, 100000

void prepareData(
    double random_coef,
    std::optional<bool> is_reverse,
    double part_coef,
    int32_t block_size
)
{
    static TMyGenerator gen(elems);
    TSeries seriesRandom, seriesFully, seriesPartial, seriesLocally;
    for (int i = 0; i < arrays; ++i) {
        std::cout << i << std::endl;
        seriesRandom.push_back(std::move(gen.GenRandom(random_coef)));
        if (is_reverse.has_value()) {
            seriesFully.push_back(std::move(gen.GenPartiallySorted(1, is_reverse.value())));
        }
        seriesPartial.push_back(std::move(gen.GenPartiallySorted(part_coef, false)));
        seriesLocally.push_back(std::move(gen.GenLocallySorted(block_size)));
    }
    if (is_reverse.has_value()) {
        fullySortedData.push_back(std::move(seriesFully));
    }
    randomData.push_back(std::move(seriesRandom));
    partiallySortedData.push_back(std::move(seriesPartial));
    locallySortedData.push_back(std::move(seriesLocally));
}

TResults process(TData& data) {
    TResults results;
    static std::unique_ptr<int32_t[]> copy = std::make_unique<int32_t[]>(elems);
    for (const auto& series : data){
        TResultLine line;
        for (int t=1; t <= threads; ++t) {
            double sum = 0;
            for (const auto& array : series) {
                std::copy(array.get(), array.get() + elems, copy.get());
                double s = omp_get_wtime();
                shell_sort_parallel(copy.get(), elems, t);
                double e = omp_get_wtime();
                sum += e - s;
            }
            sum /= arrays;
            sum *= 1000; // to ms
            line.push_back(sum);
            std::cout << "Finished " << t << std::endl;
        }
        results.push_back(line);
    }
    return results;
}

void prepare_headers(
    std::ofstream& fileRand,
    std::ofstream& fileRandSpeed,
    std::ofstream& fileRandEff,
    std::ofstream& fileFully,
    std::ofstream& filePartial,
    std::ofstream& fileLocal,
    std::ofstream& fileTogether,
    std::ofstream& fileTogetherSpeed,
    std::ofstream& fileTogetherEff
) {
    fileRand << "3\n" << "100%\n" << "10%\n" << "1%\n" << "Кол-во потоков\n" << "Время выполнения\n" << "none\n";
    fileRandSpeed << "3\n" << "100%\n" << "10%\n" << "1%\n" << "Кол-во потоков\n" << "Ускорение\n" << "none\n";
    fileRandEff << "3\n" << "100%\n" << "10%\n" << "1%\n" << "Кол-во потоков\n" << "Эффективность\n" << "none\n";


    fileFully << "2\n" << "sorted\n" << "reversed\n" << "Кол-во потоков\n" << "Время выполнения\n" << "none\n";
    filePartial << "3\n" << "25%\n" << "50%\n" << "75%\n" << "Кол-во потоков\n" << "Время выполнения\n" << "none\n";
    fileLocal << "3\n" << "10\n" << "1000\n" << "100000\n" << "Кол-во потоков\n" << "Время выполнения\n" << "none\n";

    fileTogether << "3\n" << "Сорт. в обратном порядке\n" << "Частично отсортированные (50%)\n" << "Локально отсортированные (1000)\n" << "Кол-во потоков\n" << "Время выполнения\n" << "none\n";
    fileTogetherSpeed << "3\n" << "Сорт. в обратном порядке\n" << "Частично отсортированные (50%)\n" << "Локально отсортированные (1000)\n" << "Кол-во потоков\n" << "Ускорение\n" << "none\n";
    fileTogetherEff << "3\n" << "Сорт. в обратном порядке\n" << "Частично отсортированные (50%)\n" << "Локально отсортированные (1000)\n" << "Кол-во потоков\n" << "Эффективность\n" << "none\n";
}

void test_delim_graph()
{
    TData& data = randomData;

    static std::unique_ptr<int32_t[]> copy = std::make_unique<int32_t[]>(elems);
    const double eps = 1e-9;

    struct GraphFile {
        std::string filename;
        int threads;
    };

    std::vector<GraphFile> files = {
        {"results/delim_graph_1thread", 1},
        {"results/delim_graph_8threads", 8}
    };

    for (const auto& gf : files) {
        std::ofstream file(gf.filename);
        file << "1\n";
        file << "Время выполнения для разынх delim\n";
        file << "delim*10\n";
        file << "Время выполнения, мс\n";
        file << "none\n";

        double delim = 1.1;
        while (delim <= 3.0 + eps) {
            double sum = 0;

            for (const auto& series : data) {
                for (const auto& array : series) {
                    std::copy(array.get(), array.get() + elems, copy.get());
                    double s = omp_get_wtime();
                    shell_sort_parallel(copy.get(), elems, gf.threads, delim);
                    double e = omp_get_wtime();
                    sum += e - s;
                }
            }

            sum /= arrays;
            sum *= 1000; // ms

            file << delim*10 << " " << sum << "\n";
            std::cout << "Finished delim=" << delim << " threads=" << gf.threads << " time=" << sum << "ms\n";

            delim += 0.1;
            delim = std::round(delim * 10.0) / 10.0;
        }
    }
}


int main() {
    std::cout << "preparing data" << std::endl;
    prepareData(1, false, 0.25, 10);
    prepareData(0.1, true, 0.50, 1000);
    prepareData(0.01, std::nullopt, 0.75, 100000);

    std::cout << "calculating" << std::endl;
    TResults resultsRandom = process(randomData);
    TResults resultsFullySorted = process(fullySortedData);
    TResults resultsPartiallySorted = process(partiallySortedData);
    TResults resultsLocallySorted = process(locallySortedData);

    std::ofstream fileRand("results/random");
    std::ofstream fileRandSpeed("results/rand_speed");
    std::ofstream fileRandEff("results/rand_eff");

    std::ofstream fileFully("results/fully");
    std::ofstream filePartial("results/part");
    std::ofstream fileLocal("results/local");

    std::ofstream fileTogether("results/together");
    std::ofstream fileTogetherSpeed("results/together_speed");
    std::ofstream fileTogetherEff("results/together_eff");
    prepare_headers(fileRand, fileRandSpeed, fileRandEff, fileFully, filePartial, fileLocal, fileTogether, fileTogetherSpeed, fileTogetherEff);
    for (int t = 0; t < threads; ++t) {
        fileRand << t + 1 << " ";
        fileRandSpeed << t + 1 << " ";
        fileRandEff << t + 1 << " ";
        fileFully << t + 1 << " ";
        filePartial << t + 1 << " ";
        fileLocal << t + 1 << " ";
        fileTogether << t + 1 << " ";
        fileTogetherSpeed << t + 1 << " ";
        fileTogetherEff << t + 1 << " ";

        for (const auto& arr : resultsRandom) {
            fileRand << arr[t] << " ";
            fileRandSpeed << arr[0] / arr[t] << " ";
            fileRandEff << arr[0] / arr[t] / (t + 1) << " ";
        }
        fileRand << "\n";
        fileRandSpeed << "\n";
        fileRandEff << "\n";

        for (const auto& arr : resultsFullySorted) {
            fileFully << arr[t] << " ";
        }
        fileFully << "\n";
        for (const auto& arr : resultsPartiallySorted) {
            filePartial << arr[t] << " ";
        }
        filePartial << "\n";
        for (const auto& arr : resultsLocallySorted) {
            fileLocal << arr[t] << " ";
        }
        fileLocal << "\n";

        fileTogether << resultsFullySorted[1][t] << " " << resultsPartiallySorted[1][t] << " " << resultsLocallySorted[1][t] << "\n";
        fileTogetherSpeed << resultsFullySorted[1][0] / resultsFullySorted[1][t]
              << " " << resultsPartiallySorted[1][0] / resultsPartiallySorted[1][t]
              << " " << resultsLocallySorted[1][0] / resultsLocallySorted[1][t] << "\n";
        fileTogetherEff << resultsFullySorted[1][0] / resultsFullySorted[1][t] / (t + 1)
              << " " << resultsPartiallySorted[1][0] / resultsPartiallySorted[1][t] / (t + 1)
              << " " << resultsLocallySorted[1][0] / resultsLocallySorted[1][t] / (t + 1)
              << "\n";
    }

    test_delim_graph();
}
