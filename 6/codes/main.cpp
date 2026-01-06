#include "shell.h"
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <random>
#include <vector>
#include <climits>
#include <cmath>
#include <iomanip>
#include <ctime>

namespace fs = std::filesystem;

static const int num_arrays = 20;
static const int size = std::pow(10, 6);

std::random_device dev;
std::mt19937 gen(dev());
std::uniform_int_distribution<int> dist(0, 1000000);

std::unique_ptr<int[]> gen_random_array(int size) {
    std::unique_ptr<int[]> array = std::make_unique<int[]>(size);
    for (int i = 0; i < size; ++i)
        array[i] = dist(gen);
    return array;
}

int main(int argc, char** argv) {
    int rank, processes;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    double total_time_mpi = 0.0;

    std::vector<std::unique_ptr<int[]>> all_arrays;
    all_arrays.reserve(num_arrays);
    for (int i = 0; i < num_arrays; ++i)
        all_arrays.push_back(gen_random_array(size));

    std::unique_ptr<int[]> sizes_arr = std::make_unique<int[]>(processes);
    std::unique_ptr<int[]> offsets_arr = std::make_unique<int[]>(processes);
    int base_chunk = size / processes;
    int rem = size % processes;
    for (int i = 0; i < processes; i++) {
        sizes_arr[i] = base_chunk + (i < rem ? 1 : 0);
        offsets_arr[i] = (i == 0) ? 0 : offsets_arr[i - 1] + sizes_arr[i - 1];
    }

    int local_n = sizes_arr[rank];
    std::unique_ptr<int[]> local_arr = std::make_unique<int[]>(local_n);

    std::unique_ptr<int[]> mpi_merge_buf = nullptr;
    if (rank == 0)
        mpi_merge_buf = std::make_unique<int[]>(size);

    for (int array_index = 0; array_index < num_arrays; ++array_index) {
        std::unique_ptr<int[]> global_arr = nullptr;
        if (rank == 0) {
            global_arr = std::make_unique<int[]>(size);
            memcpy(global_arr.get(), all_arrays[array_index].get(), size * sizeof(int));
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();

        MPI_Scatterv(
            global_arr.get(), sizes_arr.get(), offsets_arr.get(), MPI_INT,
            local_arr.get(), local_n, MPI_INT,
            0, MPI_COMM_WORLD
        );
        shell_sort(local_arr.get(), local_n);

        MPI_Gatherv(
            local_arr.get(), local_n, MPI_INT,
            mpi_merge_buf.get(), sizes_arr.get(), offsets_arr.get(),
            MPI_INT, 0, MPI_COMM_WORLD
        );

        if (rank == 0) {
            std::vector<int*> subarrays(processes);
            for (int p = 0; p < processes; ++p)
                subarrays[p] = mpi_merge_buf.get() + offsets_arr[p];
            merge_arrays(mpi_merge_buf.get(), subarrays.data(), processes, sizes_arr.get(), size);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        if (rank == 0) {
            double end = MPI_Wtime();
            total_time_mpi += (end - start);
            for (int i=0; i<size-1; ++i) {
                if (mpi_merge_buf[i] > mpi_merge_buf[i+1]) {
                    std::cout << "NOT SORTED" << std::endl;
                }
            }
        }
    }


    if (rank == 0) {
        double avg_time_mpi = total_time_mpi / num_arrays;
        double avg_mpi_ms = avg_time_mpi * 1000.0;

        double time_one_mpi = 0.0;
        std::string baseline_fname = "results/baseline_512";

        if (processes == 1) {
            std::ofstream base(baseline_fname);
            if (base) {
                base << std::fixed << std::setprecision(6) << avg_mpi_ms << "\n";
                time_one_mpi = avg_mpi_ms;
            } else {
                std::cerr << "Cannot write baseline file " << baseline_fname << "\n";
            }
        } else {
            if (fs::exists(baseline_fname) && fs::file_size(baseline_fname) > 0) {
                std::ifstream base(baseline_fname);
                if (base) {
                    base >> time_one_mpi;
                }
            }
        }

        double real_speedup_mpi = time_one_mpi / avg_mpi_ms;

        double efficiency_mpi = real_speedup_mpi / processes;

        std::string fname_time = "results/time_512";
        bool need_header_time = (!fs::exists(fname_time) || fs::file_size(fname_time) == 0);
        {
            std::ofstream f(fname_time, std::ios::app);
            if (need_header_time) {
                f << 1 << "\n";
                f << "MPI\n";
                f << "Размер массива\n";
                f << "Время выполнения, мс\n";
                f << "none\n";
            }
            f << processes << " " << avg_mpi_ms << "\n";
        }

        std::string fname_sp = "results/speedup_512";
        bool need_header_sp = (!fs::exists(fname_sp) || fs::file_size(fname_sp) == 0);
        {
            std::ofstream f(fname_sp, std::ios::app);
            if (need_header_sp) {
                f << 1 << "\n";
                f << "MPI\n";
                f << "Размер массива\n";
                f << "Ускорение\n";
                f << "none\n";
            }
            f << processes << " " << real_speedup_mpi << "\n";
        }

        std::string fname_ef = "results/efficiency_512";
        bool need_header_ef = (!fs::exists(fname_ef) || fs::file_size(fname_ef) == 0);
        {
            std::ofstream f(fname_ef, std::ios::app);
            if (need_header_ef) {
                f << 1 << "\n";
                f << "MPI\n";
                f << "Размер массива\n";
                f << "Эффективность\n";
                f << "none\n";
            }
            f << processes << " " << efficiency_mpi << "\n";
        }

        std::cout << "Finished. Processes=" << processes
                  << " MPI_avg_time=" << avg_mpi_ms << " ms\n";
    }

    MPI_Finalize();
    return 0;
}
