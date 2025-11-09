#include <cstdlib>
#include <cmath>
#include <omp.h>
#include "mtx.h"

// === КЛАССИЧЕСКИЙ АЛГОРИТМ ФОКСА ТОЛЬКО ДЛЯ ПОЛНЫХ КВАДРАТОВ === by mr. Semen
void FoxParallelMultiplication(double* A, double* B, double* C, int Size, int NumThreads) {
    int q = (int)sqrt((double)NumThreads);

    // Проверяем, является ли число потоков полным квадратом и делит ли размер на q
    if (q * q == NumThreads && Size % q == 0 && q > 0) {
        int BlockSize = Size / q;

#pragma omp parallel num_threads(NumThreads)
        {
            int tid = omp_get_thread_num();
            int row = tid / q;
            int col = tid % q;

            double* Cblock = (double*)calloc((size_t)BlockSize * BlockSize, sizeof(double));
            double* TempA = (double*)malloc((size_t)BlockSize * BlockSize * sizeof(double));
            double* TempB = (double*)malloc((size_t)BlockSize * BlockSize * sizeof(double));

            for (int iter = 0; iter < q; iter++) {
                int pivot = (row + iter) % q;

                // Копирование блока A
                for (int bi = 0; bi < BlockSize; bi++) {
                    for (int bj = 0; bj < BlockSize; bj++) {
                        int gi = row * BlockSize + bi;
                        int gj = pivot * BlockSize + bj;
                        TempA[bi * BlockSize + bj] = A[gi * Size + gj];
                    }
                }

                // Копирование блока B
                for (int bi = 0; bi < BlockSize; bi++) {
                    for (int bj = 0; bj < BlockSize; bj++) {
                        int gi = pivot * BlockSize + bi;
                        int gj = col * BlockSize + bj;
                        TempB[bi * BlockSize + bj] = B[gi * Size + gj];
                    }
                }

                // Умножение блоков
                for (int i = 0; i < BlockSize; i++) {
                    for (int k = 0; k < BlockSize; k++) {
                        double a_val = TempA[i * BlockSize + k];
                        for (int j = 0; j < BlockSize; j++) {
                            Cblock[i * BlockSize + j] += a_val * TempB[k * BlockSize + j];
                        }
                    }
                }
            }

            // Запись результата
            for (int bi = 0; bi < BlockSize; bi++) {
                for (int bj = 0; bj < BlockSize; bj++) {
                    int gi = row * BlockSize + bi;
                    int gj = col * BlockSize + bj;
                    C[gi * Size + gj] = Cblock[bi * BlockSize + bj];
                }
            }

            free(Cblock);
            free(TempA);
            free(TempB);
        }
    }
}

// Алгоритм Кэннона (параллельная версия) by ms. Nadya
void cannon_par(double *A, double *B, double *C, int n, int blocks_per_line) {
    int threads = blocks_per_line * blocks_per_line;

    #pragma omp parallel for collapse(2) num_threads(threads) default(none) shared(A, B, C, n, blocks_per_line)
    for (int block_i = 0; block_i < blocks_per_line; block_i++) {
        for (int block_j = 0; block_j < blocks_per_line; block_j++) {

            int block_size = n / blocks_per_line;
            double* local_C = (double*)calloc(block_size * block_size, sizeof(double));

            for (int step = 0; step < blocks_per_line; step++) {

                // Сдвиги: начальный + текущий шаг
                int shifted_A_j = (block_j + block_i + step) % blocks_per_line;
                int shifted_B_i = (block_i + block_j + step) % blocks_per_line;

                int start_i_A = block_i * block_size;
                int start_j_A = shifted_A_j * block_size;

                int start_i_B = shifted_B_i * block_size;
                int start_j_B = block_j * block_size;

                int start_i_C = start_i_A;
                int start_j_C = start_j_B;

                // Умножение блока A×B и накопление в C
                for (int i = 0; i < block_size; i++) {
                    for (int j = 0; j < block_size; j++) {
                        double sum = 0.0;
                        for (int k = 0; k < block_size; k++) {
                            int idx_A = (start_i_A + i) * n + (start_j_A + k);
                            int idx_B = (start_i_B + k) * n + (start_j_B + j);
                            sum += A[idx_A] * B[idx_B];
                        }

                        int idx_C = (start_i_C + i) * n + (start_j_C + j);
                        // Обновляем C с учетом суммы по этому k-блоку
                        #pragma omp atomic
                        C[idx_C] += sum;
                    }
                }
            }
        }
    }
}
