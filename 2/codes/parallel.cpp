#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <limits.h>
#include <random>

int32_t find_cancel(int32_t *arr, int32_t size, int32_t target, int32_t threads) {
    int32_t index = -1;

    #pragma omp parallel num_threads(threads) default(none) \
                             shared(arr, size, target, index)
    {
        #pragma omp for
        for (int32_t i = 0; i < size; ++i) {
            if (arr[i] == target) {
                #pragma omp critical
                {
                    if (index == -1)
                        index = i;
                }
                #pragma omp cancel for
            }

            #pragma omp cancellation point for
        }
    }
    return index;
}

int32_t find_atomic(int32_t *arr, int32_t size, int32_t target, int32_t threads) {
    int32_t index = -1;

    #pragma omp parallel num_threads(threads) default(none) \
                             shared(arr, size, target, index)
    {
        #pragma omp for
        for (int32_t i = 0; i < size; ++i) {
            int32_t current;
            #pragma omp atomic read
            current = index;
            if (current != -1) {
                continue;
            }

            if (arr[i] == target) {
                int32_t prev;
                #pragma omp atomic capture
                {
                    prev = index;
                    index = (prev == -1) ? i : prev;
                }
            }
        }
    }

    return index;
}

int32_t find_reduction_min(int32_t *arr, int32_t size, int32_t target, int32_t threads) {
    int32_t index = INT_MAX;

    #pragma omp parallel for num_threads(threads) \
                             default(none) \
                             shared(arr, size, target) \
                             reduction(min:index)
    for (int32_t i = 0; i < size; ++i) {
        if (arr[i] == target) {
            if (i < index) index = i;
        }
    }

    return (index == INT_MAX) ? -1 : index;
}

int32_t find_dirty(int32_t *arr, int32_t size, int32_t target, int32_t threads) {
    int32_t index = -1;

    #pragma omp parallel for num_threads(threads) \
                             default(none) \
                             shared(arr, size, target, index)
    for (int32_t i = 0; i < size; ++i) {
        if (arr[i] == target) {
            index = i;

            i = size;
        }
    }

    return index;
}

void getExperimentDots(
    int32_t* array,
    int32_t sz,
    int32_t target,
    int32_t threads,
    int32_t repeats,
    FILE* file_res,
    FILE* file_speedup,
    FILE* file_eff,
    double* T1cancel,
    double* T1atomic,
    double* T1reduction,
    double* T1dirty)
{
    double timeCancel = 0;
    double timeAtomic = 0;
    double timeReduction = 0;
    double timeDirty = 0;
    double start, end;
    for (int32_t i = 0; i < repeats; ++i) {
        start = omp_get_wtime();
        find_cancel(array, sz, target, threads);
        end = omp_get_wtime();
        timeCancel += end - start;
    }
    timeCancel /= repeats;
    timeCancel *= 1000;
    if (threads == 1) {
        *T1cancel = timeCancel;
    }

    for (int32_t i = 0; i < repeats; ++i) {
        start = omp_get_wtime();
        find_atomic(array, sz, target, threads);
        end = omp_get_wtime();
        timeAtomic += end - start;
    }
    timeAtomic /= repeats;
    timeAtomic *= 1000;
    if (threads == 1) {
        *T1atomic = timeAtomic;
    }

    for (int32_t i = 0; i < repeats; ++i) {
        start = omp_get_wtime();
        find_reduction_min(array, sz, target, threads);
        end = omp_get_wtime();
        timeReduction += end - start;
    }
    timeReduction /= repeats;
    timeReduction *= 1000;
    if (threads == 1) {
        *T1reduction = timeReduction;
    }

    for (int32_t i = 0; i < repeats; ++i) {
        start = omp_get_wtime();
        find_dirty(array, sz, target, threads);
        end = omp_get_wtime();
        timeDirty += end - start;
    }
    timeDirty /= repeats;
    timeDirty *= 1000;
    if (threads == 1) {
        *T1dirty = timeDirty;
    }

    double realSpeedupCancel = (*T1cancel) / timeCancel;
    double realSpeedupAtomic = (*T1atomic) / timeAtomic;
    double realSpeedupReduction = (*T1reduction) / timeReduction;
    double realSpeedupDirty = (*T1dirty) / timeDirty;
    if (file_res != NULL) {
        fprintf(file_res, "%d %lf %lf %lf %lf\n", threads, timeReduction, timeCancel, timeAtomic, timeDirty);
    }
    if (file_speedup != NULL) {
        fprintf(file_speedup, "%d %lf %lf %lf %lf %d\n", threads, realSpeedupReduction, realSpeedupCancel, realSpeedupAtomic, realSpeedupDirty, threads);
    }
    if (file_eff != NULL) {
        fprintf(file_eff, "%d %lf %lf %lf %lf %d\n", threads, realSpeedupReduction / threads, realSpeedupCancel / threads, realSpeedupAtomic / threads, realSpeedupDirty/threads, 1);
    }
}

int32_t main(int32_t argc, char** argv)
{
    srand(time(NULL));

    FILE* file_res = fopen("results/runtime_array", "w");
    fprintf(file_res, "4\n");
    fprintf(file_res, "ReductionAlg\n");
    fprintf(file_res, "CancelAlg\n");
    fprintf(file_res, "AtomicAlg\n");
    fprintf(file_res, "DirtyAlg\n");
    fprintf(file_res, "Кол-во потоков\n");
    fprintf(file_res, "Время выполнения, мс\n");
    fprintf(file_res, "none\n");

    FILE* file_speedup = fopen("results/speedup_array", "w");
    fprintf(file_speedup, "5\n");
    fprintf(file_speedup, "ReductionAlg\n");
    fprintf(file_speedup, "CancelAlg\n");
    fprintf(file_speedup, "AtomicAlg\n");
    fprintf(file_speedup, "DirtyAlg\n");
    fprintf(file_speedup, "Expected\n");
    fprintf(file_speedup, "Кол-во потоков\n");
    fprintf(file_speedup, "Ускорение\n");
    fprintf(file_speedup, "none\n");

    FILE* file_eff = fopen("results/efficiency_array", "w");
    fprintf(file_eff, "5\n");
    fprintf(file_eff, "ReductionAlg\n");
    fprintf(file_eff, "CancelAlg\n");
    fprintf(file_eff, "AtomicAlg\n");
    fprintf(file_eff, "DirtyAlg\n");
    fprintf(file_eff, "Expected\n");
    fprintf(file_eff, "Кол-во потоков\n");
    fprintf(file_eff, "Эффективность\n");
    fprintf(file_eff, "none\n");

    FILE* file_res_close = fopen("results/runtime_close", "w");
    fprintf(file_res_close, "4\n");
    fprintf(file_res_close, "ReductionAlg\n");
    fprintf(file_res_close, "CancelAlg\n");
    fprintf(file_res_close, "AtomicAlg\n");
    fprintf(file_res_close, "DirtyAlg\n");
    fprintf(file_res_close, "Кол-во потоков\n");
    fprintf(file_res_close, "Время выполнения, мс\n");
    fprintf(file_res_close, "none\n");

    FILE* file_speedup_close = fopen("results/speedup_close", "w");
    fprintf(file_speedup_close, "5\n");
    fprintf(file_speedup_close, "ReductionAlg\n");
    fprintf(file_speedup_close, "CancelAlg\n");
    fprintf(file_speedup_close, "AtomicAlg\n");
    fprintf(file_speedup_close, "DirtyAlg\n");
    fprintf(file_speedup_close, "Expected\n");
    fprintf(file_speedup_close, "Кол-во потоков\n");
    fprintf(file_speedup_close, "Ускорение\n");
    fprintf(file_speedup_close, "none\n");

    FILE* file_eff_close = fopen("results/efficiency_close", "w");
    fprintf(file_eff_close, "5\n");
    fprintf(file_eff_close, "ReductionAlg\n");
    fprintf(file_eff_close, "CancelAlg\n");
    fprintf(file_eff_close, "AtomicAlg\n");
    fprintf(file_eff_close, "DirtyAlg\n");
    fprintf(file_eff_close, "Expected\n");
    fprintf(file_eff_close, "Кол-во потоков\n");
    fprintf(file_eff_close, "Эффективность\n");
    fprintf(file_eff_close, "none\n");

    int32_t sz = pow(10, 7);
    int32_t sz_close = pow(10, 6);
    int32_t target_frequency = 0.005 * sz_close;
    int32_t repeats = 100;
    int32_t max_threads = 12;
    double T1reduction, T1atomic, T1cancel, T1dirty;
    double T1reduction_close, T1atomic_close, T1cancel_close, T1dirty_close;

    int32_t* array = (int32_t*)malloc(sz*sizeof(int32_t));
    int32_t* array_close = (int32_t*)malloc(sz_close*sizeof(int32_t));

    std::mt19937_64 rng(123);
    std::uniform_int_distribution<size_t> idx(0, sz_close-1);
    std::uniform_int_distribution<int32_t> target_random(0, 10000000);
    for (int32_t threads=1; threads <= max_threads; ++threads) {
        int32_t target = target_random(rng);
        int32_t not_target = target - 1;
        for(int32_t i=0; i<sz; i++) {
            array[i] = not_target;
        }
        for(int32_t i=0; i<sz_close; i++) {
            array_close[i] = not_target;
        }

        for (int i = 0; i < target_frequency; ++i) {
            array_close[idx(rng)] = target;
        }

        getExperimentDots(array, sz, target, threads, repeats, file_res, file_speedup, file_eff, &T1cancel, &T1atomic, &T1reduction, &T1dirty);
        getExperimentDots(array_close, sz_close, target, threads, repeats, file_res_close, file_speedup_close, file_eff_close, &T1cancel_close, &T1atomic_close, &T1reduction_close, &T1dirty_close);

        printf("finished %d\n", threads);
    }

    free(array);
    free(array_close);
    fclose(file_res);
    fclose(file_speedup);
    fclose(file_eff);
    return 0;
}
