#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <limits.h>

#include <omp.h>
#include <limits.h>

int find_cancel(int *arr, int size, int target, int threads) {
    int index = -1;

    #pragma omp parallel num_threads(threads) default(none) \
                             shared(arr, size, target, index)
    {
        #pragma omp for
        for (int i = 0; i < size; ++i) {
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

int find_atomic(int *arr, int size, int target, int threads) {
    int index = -1;

    #pragma omp parallel num_threads(threads) default(none) \
                             shared(arr, size, target, index)
    {
        #pragma omp for
        for (int i = 0; i < size; ++i) {
            int current;
            #pragma omp atomic read
            current = index;
            if (current != -1) {
                continue;
            }

            if (arr[i] == target) {
                int prev;
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

int find_reduction_min(int *arr, int size, int target, int threads) {
    int index = INT_MAX;

    #pragma omp parallel for num_threads(threads) \
                             default(none) \
                             shared(arr, size, target) \
                             reduction(min:index)
    for (int i = 0; i < size; ++i) {
        if (arr[i] == target) {
            if (i < index) index = i;
        }
    }

    return (index == INT_MAX) ? -1 : index;
}

void getExperimentDots(
    int* array,
    int sz,
    int target,
    int threads,
    int repeats,
    FILE* file_res,
    FILE* file_speedup,
    FILE* file_eff,
    double* T1cancel,
    double* T1atomic,
    double* T1reduction)
{
    double timeCancel = 0;
    double timeAtomic = 0;
    double timeReduction = 0;
    double start, end;
    for (int i = 0; i < repeats; ++i) {
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

    for (int i = 0; i < repeats; ++i) {
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

    for (int i = 0; i < repeats; ++i) {
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
    double realSpeedupCancel = (*T1cancel) / timeCancel;
    double realSpeedupAtomic = (*T1atomic) / timeAtomic;
    double realSpeedupReduction = (*T1reduction) / timeReduction;
    if (file_res != NULL) {
        fprintf(file_res, "%d %lf %lf %lf\n", threads, timeReduction, timeCancel, timeAtomic);
    }
    if (file_speedup != NULL) {
        fprintf(file_speedup, "%d %lf %lf %lf %d\n", threads, realSpeedupReduction, realSpeedupCancel, realSpeedupAtomic, threads);
    }
    if (file_eff != NULL) {
        fprintf(file_eff, "%d %lf %lf %lf %d\n", threads, realSpeedupReduction / threads, realSpeedupCancel / threads, realSpeedupAtomic / threads, 1);
    }
}

int main(int argc, char** argv)
{
    srand(time(NULL));

    FILE* file_res = fopen("results/runtime", "w");
    fprintf(file_res, "3\n");
    fprintf(file_res, "ReductionAlg\n");
    fprintf(file_res, "CancelAlg\n");
    fprintf(file_res, "AtomicAlg\n");
    fprintf(file_res, "Кол-во потоков\n");
    fprintf(file_res, "Время выполнения, мс\n");
    fprintf(file_res, "none\n");

    FILE* file_speedup = fopen("results/speedup", "w");
    fprintf(file_speedup, "4\n");
    fprintf(file_speedup, "ReductionAlg\n");
    fprintf(file_speedup, "CancelAlg\n");
    fprintf(file_speedup, "AtomicAlg\n");
    fprintf(file_speedup, "Expected\n");
    fprintf(file_speedup, "Кол-во потоков\n");
    fprintf(file_speedup, "Ускорение\n");
    fprintf(file_speedup, "none\n");

    FILE* file_eff = fopen("results/efficiency", "w");
    fprintf(file_eff, "4\n");
    fprintf(file_eff, "ReductionAlg\n");
    fprintf(file_eff, "CancelAlg\n");
    fprintf(file_eff, "AtomicAlg\n");
    fprintf(file_eff, "Expected\n");
    fprintf(file_eff, "Кол-во потоков\n");
    fprintf(file_eff, "Эффективность\n");
    fprintf(file_eff, "none\n");


    FILE* file_det_eff_t = fopen("results/det_efficiency_t", "w");
    fprintf(file_det_eff_t, "4\n");
    fprintf(file_det_eff_t, "ReductionAlg\n");
    fprintf(file_det_eff_t, "CancelAlg\n");
    fprintf(file_det_eff_t, "AtomicAlg\n");
    fprintf(file_det_eff_t, "Expected\n");
    fprintf(file_det_eff_t, "Кол-во потоков\n");
    fprintf(file_det_eff_t, "Эффективность\n");
    fprintf(file_det_eff_t, "none\n");

    FILE* file_det_eff_f = fopen("results/det_efficiency_f", "w");
    fprintf(file_det_eff_f, "4\n");
    fprintf(file_det_eff_f, "ReductionAlg\n");
    fprintf(file_det_eff_f, "CancelAlg\n");
    fprintf(file_det_eff_f, "AtomicAlg\n");
    fprintf(file_det_eff_f, "Expected\n");
    fprintf(file_det_eff_f, "Кол-во потоков\n");
    fprintf(file_det_eff_f, "Эффективность\n");
    fprintf(file_det_eff_f, "none\n");

    int sz = pow(10, 7);
    int repeats = 20;
    int max_threads = 16;
    int sz_special = pow(10, 7);
    double T1reduction, T1atomic, T1cancel;
    for (int threads=1; threads <= max_threads; ++threads){
        int* array = (int*)malloc(sz*sizeof(int));
        int* determined_array = (int*)malloc(sz*sizeof(int));
        int* determined_array_mini = (int*)malloc(sz_special*sizeof(int));
        int num = rand()%1000000;

        for(int i=0; i<sz; i++) {
            array[i] = rand();
            determined_array[i] = num;
        }
        for(int i=0; i<sz_special; i++) {
            determined_array_mini[i] = rand();
        }
        determined_array_mini[10000] = num;

        int target = rand();
        int det_true = num;
        int det_false = rand();

        getExperimentDots(array, sz, target, threads, repeats, file_res, file_speedup, file_eff, &T1cancel, &T1atomic, &T1reduction);
        getExperimentDots(determined_array_mini, sz_special, det_true, threads, repeats, NULL, NULL, file_det_eff_t, &T1cancel, &T1atomic, &T1reduction);
        getExperimentDots(determined_array, sz, det_false, threads, repeats, NULL, NULL, file_det_eff_f, &T1cancel, &T1atomic, &T1reduction);

        free(array);
        free(determined_array);
        free(determined_array_mini);
        printf("finished %d\n", threads);
    }

    fclose(file_res);
    fclose(file_speedup);
    fclose(file_eff);
    fclose(file_det_eff_t);
    fclose(file_det_eff_f);
    return 0;
}
