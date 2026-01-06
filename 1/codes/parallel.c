#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <limits.h>

int parallel(int* array, size_t size, size_t threads, size_t chunk)
{
    int max = INT32_MIN;

    #pragma omp parallel num_threads(threads) \
        shared(array, size, chunk) \
        reduction(max:max) \
        default(none)
    {
        #pragma omp for schedule(guided, chunk)
        for (size_t i = 0; i < size; ++i) {
            if (array[i] > max) {
                max = array[i];
            }
        }
    }

    return max;
}

int main(void)
{
    srand(time(NULL));

    FILE* file_res = fopen("results/parallel_dops", "w");

    int chunks[4] = {1, 10, 100, 1000};

    /* заголовок */
    fprintf(file_res, "4\n");
    for (int i = 0; i < 4; ++i)
        fprintf(file_res, "%d\n", chunks[i]);

    fprintf(file_res, "Кол-во потоков\n");
    fprintf(file_res, "Ускорение\n");
    fprintf(file_res, "none\n");

    int sz = 3 * pow(10, 7);
    int repeats = 100;
    int max_threads = 8;

    double T1[4] = {0};

    for (int threads = 1; threads <= max_threads; ++threads) {

        int* array = malloc(sz * sizeof(int));
        for (int i = 0; i < sz; ++i)
            array[i] = rand();

        double speedup[4];

        for (int c = 0; c < 4; ++c) {
            double time_sum = 0.0;

            for (int r = 0; r < repeats; ++r) {
                double start = omp_get_wtime();
                parallel(array, sz, threads, chunks[c]);
                double end = omp_get_wtime();
                time_sum += end - start;
            }

            time_sum /= repeats;

            if (threads == 1)
                T1[c] = time_sum;

            speedup[c] = T1[c] / time_sum;
        }

        /* строка ускорений */
        fprintf(file_res, "%d", threads);
        for (int c = 0; c < 4; ++c)
            fprintf(file_res, " %lf", speedup[c]);
        fprintf(file_res, "\n");

        free(array);
        printf("finished %d\n", threads);
    }

    fclose(file_res);
    return 0;
}
