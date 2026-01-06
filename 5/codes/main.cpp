#include <mpi.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <random>
#include <cstdint>
#include <climits>
#include <limits>
#include <iomanip>
#include <cmath>

namespace fs = std::filesystem;

static const int repeats = 10;
static const int32_t count = 3*std::pow(10, 7);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int size = 0;
    int rank = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int base_count = count / size;
    int remainder = count % size;
    int local_count = base_count + (rank < remainder ? 1 : 0);

    std::random_device dev;
    std::mt19937 gen(dev());
    std::uniform_int_distribution<int> dist(0, 0x7FFFFFFF);

    double total_time = 0.0;

    for (int r = 0; r < repeats; ++r) {
        std::unique_ptr<int[]> local_array = std::make_unique<int[]>(local_count);
        int local_max = INT_MIN;
        int global_max = INT_MIN;

        if (rank == 0) {
            std::vector<int> global_array(static_cast<size_t>(count));
            for (int32_t i = 0; i < count; ++i) {
                global_array[i] = dist(gen);
            }

            for (int i = 0; i < local_count; ++i) local_array[i] = global_array[i];

            int32_t current_position = local_count;
            for (int dest = 1; dest < size; ++dest) {
                int dest_count = base_count + (dest < remainder ? 1 : 0);
                if (dest_count > 0) {
                    MPI_Send(global_array.data() + current_position, dest_count,
                             MPI_INT, dest, 0, MPI_COMM_WORLD);
                    current_position += dest_count;
                }
            }
        } else {
            if (local_count > 0) {
                MPI_Recv(local_array.get(), local_count, MPI_INT, 0, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double start = MPI_Wtime();

        for (int i = 0; i < local_count; ++i) {
            if (local_array[i] > local_max) local_max = local_array[i];
        }

        MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

        double end = MPI_Wtime();

        if (rank == 0) total_time += (end - start);
    }

    if (rank == 0) {
        double avg_seconds = total_time / static_cast<double>(repeats);
        double avg_ms = avg_seconds * 1000.0;
        std::string fname = "results/mpi_time";
        bool need_header_time = (!fs::exists(fname) || fs::file_size(fname) == 0);
        {
            std::ofstream file(fname, std::ios::app);
            if (!file) {
                std::cerr << "Cannot open " << fname << " for appending\n";
            } else {
                if (need_header_time) {
                    file << 1 << "\n";
                    file << count << "\n";
                    file << "Кол-во процессов\n";
                    file << "Время выполнения, мс\n";
                    file << "none\n";
                }
                file << size << " " << std::fixed << std::setprecision(6) << avg_ms << "\n";
            }
        }

        double baseline_ms = std::numeric_limits<double>::quiet_NaN();
        std::string baseline_fname = "results/mpi_baseline";
        if (size == 1) {
            std::ofstream base(baseline_fname);
            if (base) {
                base << std::fixed << std::setprecision(6) << avg_ms << "\n";
                baseline_ms = avg_ms;
            } else {
                std::cerr << "Cannot write baseline file " << baseline_fname << "\n";
            }
        } else {
            if (fs::exists(baseline_fname) && fs::file_size(baseline_fname) > 0) {
                std::ifstream base(baseline_fname);
                if (base) {
                    base >> baseline_ms;
                }
            }
        }

        std::string sname = "results/mpi_speedup";
        bool need_header_sp = (!fs::exists(sname) || fs::file_size(sname) == 0);
        {
            std::ofstream sp(sname, std::ios::app);
            if (!sp) {
                std::cerr << "Cannot open " << sname << " for appending\n";
            } else {
                if (need_header_sp) {
                    sp << 2 << "\n";
                    sp << "Реальное ускорение\n";
                    sp << "Ожидаемое ускорение\n";
                    sp << "Кол-во процессов\n";
                    sp << "Ускорение\n";
                    sp << "none\n";
                }
                double realSpeedup = std::numeric_limits<double>::quiet_NaN();
                double theoreticalSpeedup = static_cast<double>(size);
                if (std::isfinite(baseline_ms) && avg_ms > 0.0) {
                    realSpeedup = baseline_ms / avg_ms;
                }
                sp << size << " " << std::fixed << std::setprecision(6)
                   << realSpeedup << " " << theoreticalSpeedup << "\n";
            }
        }

        std::string efname = "results/mpi_efficiency";
        bool need_header_ef = (!fs::exists(efname) || fs::file_size(efname) == 0);
        {
            std::ofstream ef(efname, std::ios::app);
            if (!ef) {
                std::cerr << "Cannot open " << efname << " for appending\n";
            } else {
                if (need_header_ef) {
                    ef << 2 << "\n";
                    ef << "Реальная эффективность\n";
                    ef << "Ожидаемая эффективность\n";
                    ef << "Кол-во процессов\n";
                    ef << "Эффективность\n";
                    ef << "none\n";
                }
                double realSpeedup = std::numeric_limits<double>::quiet_NaN();
                if (std::isfinite(baseline_ms) && avg_ms > 0.0) {
                    realSpeedup = baseline_ms / avg_ms;
                }
                double realEfficiency = std::numeric_limits<double>::quiet_NaN();
                if (std::isfinite(realSpeedup)) realEfficiency = realSpeedup / static_cast<double>(size);
                double theoreticalEfficiency = 1.0;
                ef << size << " " << std::fixed << std::setprecision(6)
                   << realEfficiency << " " << theoreticalEfficiency << "\n";
            }
        }

        std::cout << "MPI measurement finished. processes=" << size
                  << " avg_time=" << avg_ms << " ms\n";
        if (std::isfinite(baseline_ms)) {
            std::cout << "Baseline T1 (ms) = " << baseline_ms << "\n";
        } else {
            std::cout << "Baseline T1 not found. Run with 1 process first to create results/mpi_baseline.\n";
        }
        std::cout << "Appended results to results/mpi_time, results/mpi_speedup, results/mpi_efficiency\n";
    }

    MPI_Finalize();
    return 0;
}
