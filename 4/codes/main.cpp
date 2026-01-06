#include "gen.h"
#include "shell.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
#include <cmath>
#include <omp.h>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

using TSeries = std::vector<std::unique_ptr<int32_t[]>>;

static const int32_t elems = std::pow(10, 6);
static const int32_t arrays = 100;
static const int max_threads = 8;
static const std::vector<int> chunks = {1, 10, 100, 1000};

std::string schedule_name(int schedule_type) {
    switch (schedule_type) {
        case 0: return "results/sched_static";
        case 1: return "results/sched_dynamic";
        case 2: return "results/sched_guided";
        case 3: return "results/sched_auto";
        default: return "results/sched_unknown";
    }
}

int main() {
    fs::create_directories("results");

    std::cout << "Generating random data (single set, coef=1.0), arrays = " << arrays << ", elems = " << elems << std::endl;

    TSeries randomSeries;
    randomSeries.reserve(arrays);
    {
        static TMyGenerator gen(elems);
        for (int i = 0; i < arrays; ++i) {
            randomSeries.push_back(std::move(gen.GenRandom(1.0)));
            if ((i+1) % 10 == 0) std::cout << "generated " << (i+1) << " / " << arrays << std::endl;
        }
    }

    std::unique_ptr<int32_t[]> copy = std::make_unique<int32_t[]>(elems);

    for (int sched = 0; sched < 4; ++sched) {
        std::string fname = schedule_name(sched);
        std::ofstream file(fname);
        if (!file) {
            std::cerr << "Cannot open " << fname << " for writing\n";
            continue;
        }

        file << chunks.size() << "\n";
        for (int c : chunks) file << c << "\n";
        file << "Кол-во потоков\n";
        file << "Время выполнения, мс\n";
        file << "none\n";

        std::cout << "Measuring schedule " << sched << " -> " << fname << std::endl;

        for (int t = 1; t <= max_threads; ++t) {
            file << t << " ";

            for (size_t ci = 0; ci < chunks.size(); ++ci) {
                int chunk = chunks[ci];

                double total = 0.0;
                for (const auto &arr : randomSeries) {
                    std::copy(arr.get(), arr.get() + elems, copy.get());

                    double s = omp_get_wtime();
                    shell_sort_parallel(copy.get(), elems, t, sched, chunk);
                    double e = omp_get_wtime();
                    total += (e - s);
                }

                double avg_ms = (total / static_cast<double>(arrays)) * 1000.0;
                file << avg_ms;
                if (ci + 1 < chunks.size()) file << " ";

                std::cout << "sched=" << sched << " threads=" << t << " chunk=" << chunk
                          << " time=" << avg_ms << " ms" << std::endl;
            }

            file << "\n";
        }

        file.close();
    }

    std::cout << "Done. Files: sched_static, sched_dynamic, sched_guided, sched_auto in ./results/\n";
    return 0;
}
