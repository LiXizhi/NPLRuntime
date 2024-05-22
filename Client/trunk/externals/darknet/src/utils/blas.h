
#ifndef __BLAS_H__
#define __BLAS_H__

void const_cpu(int N, float ALPHA, float *X, int INCX);
void mul_cpu(int N, float *X, int INCX, float *Y, int INCY);
void pow_cpu(int N, float ALPHA, float *X, int INCX, float *Y, int INCY);
void axpy_cpu(int N, float ALPHA, float *X, int INCX, float *Y, int INCY);
void scal_cpu(int N, float ALPHA, float *X, int INCX);
void scal_add_cpu(int N, float ALPHA, float BETA, float *X, int INCX);
void fill_cpu(int N, float ALPHA, float *X, int INCX);
void copy_cpu(int N, float *X, int INCX, float *Y, int INCY);
void mean_cpu(float *x, int batch, int filters, int spatial, float *mean);
void variance_cpu(float *x, float *mean, int batch, int filters, int spatial, float *variance);
void normalize_cpu(float *x, float *mean, float *variance, int batch, int filters, int spatial);
void mean_delta_cpu(float *delta, float *variance, int batch, int filters, int spatial, float *mean_delta);
void variance_delta_cpu(float *x, float *delta, float *mean, float *variance, int batch, int filters, int spatial, float *variance_delta);
void normalize_delta_cpu(float *x, float *mean, float *variance, float *mean_delta, float *variance_delta, int batch, int filters, int spatial, float *delta);

void softmax_cpu(float *input, int classes, int batch, float *output, float temperature);
void softmax_cross_entropy_cpu(int batch, int classes, float *pred, float *truth, float *delta, float *error);
void shortcut_cpu(int batch, int w1, int h1, int c1, float *add, int w2, int h2, int c2, float s1, float s2, float *out);
void upsample_cpu(float *in, int w, int h, int c, int batch, int stride, int forward, float scale, float *out);

#endif