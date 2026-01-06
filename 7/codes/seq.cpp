#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>

#include "prime.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " Nstart Nend\n";
        return 1;
    }

    int64_t Nstart = std::stoll(argv[1]);
    int64_t Nend   = std::stoll(argv[2]);

    if (Nend < Nstart) {
        std::cerr << "Error: Nend must be >= Nstart\n";
        return 1;
    }

    const int64_t total_size = Nend - Nstart + 1;
    std::vector<int64_t> result_primes;
    result_primes.reserve(total_size);

    auto t0 = std::chrono::high_resolution_clock::now();

    for (int64_t num = Nstart; num <= Nend; num++) {
        if (is_prime(num)) {
            result_primes.push_back(num);
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dt = t1 - t0;
    double total_time = dt.count();

    {
        std::ofstream fout("results/sequential_results.txt");
        if (!fout) {
            std::cerr << "Error: cannot open sequential_results.txt\n";
            return 1;
        }
        fout << "Время выполнения: " << total_time << " сек\n";
    }

    std::cout << "Sequential: " << total_time
              << " sec, primes: " << result_primes.size() << "\n";

    return 0;
}
