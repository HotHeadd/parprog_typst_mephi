#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <limits.h>

int sequential(int* array, size_t size) {
    int max = INT32_MIN;
    for(int i=0; i<size; i++)
    {
        if(array[i] > max) { max = array[i]; };
    }
    return max;
}

int main(int argc, char** argv)
{
    srand(time(NULL));

    FILE* file = fopen("results/sequential_extra", "w");
    fprintf(file, "1\n");
    fprintf(file, "Последовательный алгоритм\n");
    fprintf(file, "Кол-во элементов\n");
    fprintf(file, "Время выполнения, мс\n");
    fprintf(file, "linear\n");
    int sz = 82*pow(10, 6);
    int end_sz = 91*pow(10, 6);
    int step = 9*pow(10, 4);
    int repeats = 100;

    while (sz <= end_sz) {
        int* array = (int*)malloc(sz*sizeof(int));
        for(int i=0; i<sz; i++) { array[i] = rand(); }

        double time_sum = 0;
        double start, end;
        for (int i = 0; i < repeats; ++i) {
            start = omp_get_wtime();
            sequential(array, sz);
            end = omp_get_wtime();
            time_sum += end - start;
        }
        time_sum /= repeats;
        time_sum *= 1000; // в миллисекунды
        fprintf(file, "%d %lf\n", sz, time_sum);
        sz += step;
        free(array);
        printf("finished %d\n", sz);
    }

    fclose(file);
    return 0;
}
