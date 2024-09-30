#include "gemm.h"
namespace darknet
{
    // 通用矩阵运算
    // TA A 矩阵是否转置
    // TB B 矩阵是否转置
    // M C矩阵行数
    // N C矩阵列数
    // K A矩阵列数 B矩阵行数
    // A A矩阵
    // lda A矩阵列数
    // B B矩阵
    // ldb B矩阵列数
    // C C矩阵
    // ldc C矩阵列数
    // ALPHA 矩阵乘法系数
    // BETA C矩阵系数
    void gemm(int TA, int TB, int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float BETA, float *C, int ldc)
    {
        if (BETA != 1)
        {
            for (int i = 0; i < M; ++i)
            {
                for (int j = 0; j < N; ++j)
                {
                    C[i * ldc + j] *= BETA;
                }
            }
        }

        if (!TA && !TB)
            gemm_nn(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
        else if (TA && !TB)
            gemm_tn(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
        else if (!TA && TB)
            gemm_nt(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
        else
            gemm_tt(M, N, K, ALPHA, A, lda, B, ldb, C, ldc);
    }

    void gemm_nn(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc)
    {
        for (int i = 0; i < M; ++i)
        {
            for (int k = 0; k < K; ++k)
            {
                float A_PART = ALPHA * A[i * lda + k];
                for (int j = 0; j < N; ++j)
                {
                    C[i * ldc + j] += A_PART * B[k * ldb + j];
                }
            }
        }
    }

    void gemm_tn(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc)
    {
        for (int i = 0; i < M; ++i)
        {
            for (int k = 0; k < K; ++k)
            {
                float A_PART = ALPHA * A[k * lda + i];
                for (int j = 0; j < N; ++j)
                {
                    C[i * ldc + j] += A_PART * B[k * ldb + j];
                }
            }
        }
    }

    void gemm_nt(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc)
    {
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                float sum = 0;
                for (int k = 0; k < K; ++k)
                {
                    sum += A[i * lda + k] * B[j * ldb + k];
                }
                C[i * ldc + j] += ALPHA * sum;
            }
        }
    }

    void gemm_tt(int M, int N, int K, float ALPHA, float *A, int lda, float *B, int ldb, float *C, int ldc)
    {
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                float sum = 0;
                for (int k = 0; k < K; ++k)
                {
                    sum += A[k * lda + i] * B[j * ldb + k];
                }
                C[i * ldc + j] += ALPHA * sum;
            }
        }
    }
}