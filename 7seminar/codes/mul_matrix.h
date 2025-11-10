#include "mtx.h"
int mul_matrix(const Matrix* matr_1,const Matrix* matr_2, Matrix* res_m, int threads){
    if(matr_1->Columns != matr_2->Rows) return 1;
    if(res_m->Rows != matr_1->Rows) return 1;
    if(res_m->Columns != matr_2->Columns) return 1;

    #pragma omp parallel for shared(res_m, matr_1, matr_2) default(none) num_threads(threads)
    for(size_t i = 0; i < res_m->Rows; i++){
        for(size_t j = 0; j < res_m->Columns;j++){
            double sum = 0;
            for(size_t p = 0; p < matr_1->Columns; p++){
                sum += matr_1->ptr[i * matr_1->Columns + p] * matr_2->ptr[p * matr_2->Columns + j];
            }
            res_m->ptr[i * res_m->Columns + j] = sum;
        }
    }
    return 0;
}

int mul_matrix_seq(const Matrix* matr_1,const Matrix* matr_2, Matrix* res_m){
    if(matr_1->Columns != matr_2->Rows) return 1;
    if(res_m->Rows != matr_1->Rows) return 1;
    if(res_m->Columns != matr_2->Columns) return 1;

    for(size_t i = 0; i < res_m->Rows; i++){
        for(size_t j = 0; j < res_m->Columns;j++){
            double sum = 0;
            for(size_t p = 0; p < matr_1->Columns; p++){
                sum += matr_1->ptr[i * matr_1->Columns + p] * matr_2->ptr[p * matr_2->Columns + j];
            }
            res_m->ptr[i * res_m->Columns + j] = sum;
        }
    }
    return 0;
}
