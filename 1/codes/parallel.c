#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <limits.h>

int parallel(int* array, size_t size, size_t threads) {
    int max = INT32_MIN;
    #pragma omp parallel num_threads(threads) shared(array, size) reduction(max: max) default(none)
    {
        #pragma omp for
        for(int i=0; i<size; i++)
        {
            if(array[i] > max) { max = array[i]; };
        }
    }
    return max;
}

int main(int argc, char** argv)
{
    srand(time(NULL));

    FILE* file_res = fopen("results/parallel", "w");
    fprintf(file_res, "1\n");
    fprintf(file_res, "Параллельный алгоритм\n");
    fprintf(file_res, "Кол-во потоков\n");
    fprintf(file_res, "Время выполнения, мс\n");
    fprintf(file_res, "none\n");

    FILE* file_speedup = fopen("results/speedup", "w");
    fprintf(file_speedup, "2\n");
    fprintf(file_speedup, "Реальное ускорение\n");
    fprintf(file_speedup, "Ожидаемое ускорение\n");
    fprintf(file_speedup, "Кол-во потоков\n");
    fprintf(file_speedup, "Ускорение\n");
    fprintf(file_speedup, "none\n");

    FILE* file_eff = fopen("results/efficiency", "w");
    fprintf(file_eff, "2\n");
    fprintf(file_eff, "Реальная эффективность\n");
    fprintf(file_eff, "Ожидаемая эффективность\n");
    fprintf(file_eff, "Кол-во потоков\n");
    fprintf(file_eff, "Эффективность\n");
    fprintf(file_eff, "none\n");

    int sz = 3*pow(10, 7);
    int repeats = 10;
    int max_threads = 16;
    double T1;

    for (int threads=1; threads <= max_threads; ++threads){
        int* array = (int*)malloc(sz*sizeof(int));
        for(int i=0; i<sz; i++) { array[i] = rand(); }

        double time_sum = 0;
        double start, end;
        for (int i = 0; i < repeats; ++i) {
            start = omp_get_wtime();
            parallel(array, sz, threads);
            end = omp_get_wtime();
            time_sum += end - start;
        }
        time_sum /= 10;
        time_sum *= 1000; // в миллисекунды
        if (threads == 1) {
            T1 = time_sum;
        }
        double realSpeedup = T1 / time_sum;
        fprintf(file_res, "%d %lf\n", threads, time_sum);
        fprintf(file_speedup, "%d %lf %d\n", threads, realSpeedup, threads);
        fprintf(file_eff, "%d %lf %d\n", threads, realSpeedup / threads, 1);
        free(array);
        printf("finished %d\n", threads);
    }

    fclose(file_res);
    fclose(file_speedup);
    fclose(file_eff);
    return 0;
}
