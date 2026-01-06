#include <iostream>
#include <omp.h>

int main() {
    std::cout << "OpenMP version macro: " << _OPENMP << std::endl;

    int year = _OPENMP / 100;
    int month = _OPENMP % 100;
    std::cout << "OpenMP standard date: " << year << "-" << month << std::endl;

    std::cout << "Number of processors: " << omp_get_num_procs() << std::endl;
    std::cout << "Max threads: " << omp_get_max_threads() << std::endl;

    std::cout << "Dynamic threads enabled: "
              << omp_get_dynamic() << std::endl;

    std::cout << "Timer resolution: "
              << omp_get_wtick() << " seconds" << std::endl;

    std::cout << "Nested parallelism enabled: "
              << omp_get_nested() << std::endl;

    std::cout << "Max active levels: "
              << omp_get_max_active_levels() << std::endl;

    omp_sched_t sched;
    int chunk;
    omp_get_schedule(&sched, &chunk);

    std::cout << "Schedule type: ";
    switch (sched) {
        case omp_sched_static:     std::cout << "static"; break;
        case omp_sched_dynamic:    std::cout << "dynamic"; break;
        case omp_sched_guided:     std::cout << "guided"; break;
        case omp_sched_auto:       std::cout << "auto"; break;
        default:                   std::cout << "unknown"; break;
    }
    std::cout << "\nChunk size: " << chunk << std::endl;

    return 0;
}
