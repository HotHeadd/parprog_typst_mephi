#include "prime.h"
#include <mpi.h>
#include <omp.h>

#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int mpi_rank = 0, mpi_size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    const int total_cores   = 4;
    const int total_threads = 8;
    const int expected_mpi_processes = total_cores;
    int omp_threads = std::max(1, total_threads / std::max(1, total_cores));

    if (mpi_rank == 0 && mpi_size != expected_mpi_processes) {
        std::fprintf(stderr, "Warning: Expected %d MPI processes for this configuration, got %d\n",
                     expected_mpi_processes, mpi_size);
    }

    omp_set_num_threads(omp_threads);
    int omp_max_threads = omp_get_max_threads();

    int64_t Nstart = 1000000000000LL;
    int64_t Nend   = 1000000001000LL;
    if (mpi_rank == 0) {
        if (argc >= 3) {
            Nstart = std::stoll(argv[1]);
            Nend   = std::stoll(argv[2]);
        }
    }

    MPI_Bcast(&Nstart, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Nend,   1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    int64_t total_numbers = Nend - Nstart + 1;
    if (total_numbers <= 0) {
        if (mpi_rank == 0) std::fprintf(stderr, "Empty range: [%lld, %lld]\n", Nstart, Nend);
        MPI_Finalize();
        return 1;
    }

    double proc_start = MPI_Wtime();

    int64_t first_odd = (Nstart % 2 == 1) ? Nstart : Nstart + 1;
    if (first_odd > Nend) first_odd = Nend + 1;

    int64_t local_first = first_odd + 2LL * mpi_rank;
    int64_t local_count = 0;
    if (local_first <= Nend)
        local_count = ((Nend - local_first) / 2) / mpi_size + 1;

    std::vector<std::vector<int64_t>> thread_bins(omp_max_threads);
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        auto &bin = thread_bins[tid];
        #pragma omp for schedule(dynamic, 100)
        for (int64_t i = 0; i < local_count; ++i) {
            int64_t num = local_first + i * 2LL * mpi_size;
            if (num <= Nend && is_prime(num)) {
                bin.push_back(num);
            }
        }
    }


    size_t found = 0;
    for (auto &b : thread_bins) found += b.size();
    std::vector<int64_t> local_primes;
    local_primes.reserve(found);
    for (auto &b : thread_bins) {
        local_primes.insert(local_primes.end(), b.begin(), b.end());
    }

    double proc_end = MPI_Wtime();
    double proc_time = proc_end - proc_start;
    int found_int = local_primes.size();

    std::vector<double> procs_times;
    std::vector<int> counts;
    if (mpi_rank == 0) {
        procs_times.resize(mpi_size);
        counts.resize(mpi_size);
    }

    MPI_Gather(&proc_time, 1, MPI_DOUBLE,
               (mpi_rank == 0) ? procs_times.data() : nullptr, 1, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    MPI_Gather(&found_int, 1, MPI_INT,
               (mpi_rank == 0) ? counts.data() : nullptr, 1, MPI_INT,
               0, MPI_COMM_WORLD);

    std::vector<int> displs;
    std::vector<int64_t> result_primes;
    int total_found = 0;
    if (mpi_rank == 0) {
        displs.resize(mpi_size);
        int pos = 0;
        for (int i = 0; i < mpi_size; ++i) {
            displs[i] = pos;
            pos += counts[i];
        }
        total_found = pos;
        result_primes.resize(total_found);
    }

    MPI_Gatherv(
        (found_int > 0) ? local_primes.data() : nullptr,
        found_int, MPI_LONG_LONG,
        (mpi_rank == 0) ? result_primes.data() : nullptr,
        (mpi_rank == 0) ? counts.data() : nullptr,
        (mpi_rank == 0) ? displs.data() : nullptr,
        MPI_LONG_LONG,
        0, MPI_COMM_WORLD
    );

    if (mpi_rank == 0) {
        double global_end = MPI_Wtime();
        double max_proc = *std::max_element(procs_times.begin(), procs_times.end());
        double global_time = max_proc;

        double sequential_time = 0.0;
        std::ifstream seq_file("results/sequential_results.txt");
        if (seq_file) {
            std::string line;
            while (std::getline(seq_file, line)) {
                double t;
                if (sscanf(line.c_str(), "Время выполнения: %lf сек", &t) == 1) {
                    sequential_time = t;
                    break;
                }
            }
        }

        double speedup = (sequential_time > 0.0) ? (sequential_time / global_time) : 0.0;
        double efficiency = 0.0;
        if (global_time > 0.0) {
            efficiency = speedup / (mpi_size * omp_threads);
        }

        std::ofstream fout("results/parallel_results_cyc.txt");
        if (fout) {
            fout << "ЭКСПЕРИМЕНТАЛЬНЫЕ ДАННЫЕ:\n";
            fout << "Количество MPI-процессов: " << mpi_size << "\n";
            fout << "Потоков на процесс (OpenMP): " << omp_threads << "\n\n";

            fout << "Время последовательного алгоритма: " << sequential_time << " сек\n";
            fout << "Оценочное время параллельного алгоритма (max proc): " << global_time << " сек\n";
            fout << "Ускорение (Speedup): " << speedup << "\n";
            fout << "Эффективность (Efficiency): " << efficiency << "\n\n";

            fout << "ВРЕМЯ РАБОТЫ ПРОЦЕССОВ:\n";
            for (int i = 0; i < mpi_size; ++i) {
                fout << "Процесс " << i << ": " << procs_times[i]
                     << " сек, простых чисел: " << counts[i] << "\n";
            }
        }

        std::printf("Parallel (max proc): %.6f sec, speedup: %.2f, efficiency: %.1f%%, total_primes: %d\n",
                    global_time, speedup, efficiency, total_found);
    }

    MPI_Finalize();
    return 0;
}
