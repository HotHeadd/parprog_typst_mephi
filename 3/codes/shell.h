void shell_sort_parallel(int *array, int size, int threads) {
    for (int s = size / 2; s > 0; s /= 2) {
        #pragma omp parallel for num_threads(threads) shared(array, size, s) default(none)
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
