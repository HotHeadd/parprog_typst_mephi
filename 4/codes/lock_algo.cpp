#include <iostream>
#include <vector>
#include <random>
#include <omp.h>

int main() {
    const int NUM_BINS = 10;
    const long NUM_ITEMS = 10'000'000;
    std::vector<long> histogram(NUM_BINS, 0);

    std::vector<omp_lock_t> locks(NUM_BINS);
    for (int i = 0; i < NUM_BINS; ++i) omp_init_lock(&locks[i]);
    std::mt19937_64 rng;
    unsigned seed = static_cast<unsigned>(omp_get_thread_num()) ^ (unsigned)time(nullptr);
    rng.seed(seed);
    std::uniform_int_distribution<int> dist(0, NUM_BINS - 1);

    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (long i = 0; i < NUM_ITEMS; ++i) {
            int bin = dist(rng);

            omp_set_lock(&locks[bin]);
            ++histogram[bin];
            omp_unset_lock(&locks[bin]);
        }
    }

    for (int i = 0; i < NUM_BINS; ++i) omp_destroy_lock(&locks[i]);

    for (int i = 0; i < NUM_BINS; ++i) {
        std::cout << "bin[" << i << "] = " << histogram[i] << "\n";
    }

    long sum = 0;
    for (auto v : histogram) sum += v;
    std::cout << "total = " << sum << " (expected " << NUM_ITEMS << ")\n";
    return 0;
}
