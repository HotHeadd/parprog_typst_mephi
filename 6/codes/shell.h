#include <iostream>

void shell_sort(int *array, int size) {
    for (int s = size / 512; s > 0; s /= 2) {
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

void merge_arrays(int *result, int **arrays, int num_arrays, int *sizes, int N) {
    int *indexes = (int *)calloc(num_arrays, sizeof(int));
    for (int pos = 0; pos < N; pos++) {
        int min_val = INT_MAX;
        int min_idx = -1;
        for (int i = 0; i < num_arrays; i++) {
            if (indexes[i] >= sizes[i])
                continue;
            int temp = arrays[i][indexes[i]];
            if (temp < min_val) {
                min_val = temp;
                min_idx = i;
            }
        }

        if (min_idx != -1) {
            result[pos] = min_val;
            indexes[min_idx]++;
        }
    }
    free(indexes);
}
