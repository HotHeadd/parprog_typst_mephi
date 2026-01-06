#include <omp.h>

void shell_sort_parallel(int *array, int size, int threads, int schedule_type, int chunk) {
    omp_sched_t kind;
    switch(schedule_type) {
        case 0: kind = omp_sched_static; break;
        case 1: kind = omp_sched_dynamic; break;
        case 2: kind = omp_sched_guided; break;
        case 3: kind = omp_sched_auto; break;
        default: kind = omp_sched_static; break;
    }
    omp_set_schedule(kind, chunk);

    for (int s = size / 2; s > 0; s /= 2) {
        #pragma omp parallel for num_threads(threads) schedule(runtime) shared(array, size, s) default(none)
        for (int offset = 0; offset < s; ++offset) {
            for (int i = offset + s; i < size; i += s) {
                for (int j = i - s; j >= 0 && array[j] > array[j + s]; j -= s) {
                    int temp = array[j];
                    array[j] = array[j + s];
                    array[j + s] = temp;
                }
            }
        }
    }
}
