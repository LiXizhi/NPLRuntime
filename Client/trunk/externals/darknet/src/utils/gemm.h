#ifndef __GEMM_H__
#define __GEMM_H__

void gemm(int TA, int TB, int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float BETA, float *C, int ldc);
void gemm_nn(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc);
void gemm_tn(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc);
void gemm_nt(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc);
void gemm_tt(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc);

#endif