
#include "blas.h"
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <memory>
namespace darknet
{
    void const_cpu(int N, float ALPHA, float *X, int INCX)
    {
        for (int i = 0; i < N; ++i) X[i * INCX] = ALPHA;
    }

    void mul_cpu(int N, float *X, int INCX, float *Y, int INCY)
    {
        for (int i = 0; i < N; ++i) Y[i * INCY] *= X[i * INCX];
    }

    void pow_cpu(int N, float ALPHA, float *X, int INCX, float *Y, int INCY)
    {
        for (int i = 0; i < N; ++i) Y[i * INCY] = std::pow(X[i * INCX], ALPHA);
    }
    // N 数量
    // APLHA 系数
    // X 输入向量
    // INCX 步长
    // Y 输出向量
    // INCY 步长
    // Y = APLHA * X + Y
    void axpy_cpu(int N, float ALPHA, float *X, int INCX, float *Y, int INCY)
    {
        for (int i = 0; i < N; ++i) Y[i * INCY] += ALPHA * X[i * INCX];
    }

    // X = APLHA * X
    void scal_cpu(int N, float ALPHA, float *X, int INCX)
    {
        for (int i = 0; i < N; ++i) X[i * INCX] *= ALPHA;
    }

    // X = APLHA * X + BETA
    void scal_add_cpu(int N, float ALPHA, float BETA, float *X, int INCX)
    {
        for (int i = 0; i < N; ++i) X[i * INCX] = X[i * INCX] * ALPHA + BETA;
    }

    void fill_cpu(int N, float ALPHA, float *X, int INCX)
    {
        if (INCX == 1 && ALPHA == 0 && N > 0)
        {
            memset(X, 0, N * sizeof(float));
        }
        else
        {
            for (int i = 0; i < N; ++i) X[i * INCX] = ALPHA;
        }
    }
    // Y = X
    void copy_cpu(int N, float *X, int INCX, float *Y, int INCY)
    {
        for (int i = 0; i < N; ++i) Y[i * INCY] = X[i * INCX];
    }
    // 求每张特征图的均值
    // x 输入
    // batch 批大小
    // filters 特征图个数
    // spatial 特征图大小
    // mean 均值
    void mean_cpu(float *x, int batch, int filters, int spatial, float *mean)
    {
        float scale = 1.f / (batch * spatial);
        for (int i = 0; i < filters; ++i)
        {
            mean[i] = 0;
            for (int j = 0; j < batch; ++j)
            {
                for (int k = 0; k < spatial; ++k)
                {
                    int index = j * filters * spatial + i * spatial + k;
                    mean[i] += x[index];
                }
            }
            mean[i] *= scale;
        }
    }

    // 求每张特征图的均值
    // x 输入
    // mean 均值
    // batch 批大小
    // filters 特征图个数
    // spatial 特征图大小
    // variance 方差
    void variance_cpu(float *x, float *mean, int batch, int filters, int spatial, float *variance)
    {
        float scale = 1.f / (batch * spatial);
        for (int i = 0; i < filters; ++i)
        {
            variance[i] = 0;
            for (int j = 0; j < batch; ++j)
            {
                for (int k = 0; k < spatial; ++k)
                {
                    int index = j * filters * spatial + i * spatial + k;
                    variance[i] += std::pow((x[index] - mean[i]), 2);
                }
            }
            variance[i] *= scale;
        }
    }

    // 归一化
    void normalize_cpu(float *x, float *mean, float *variance, int batch, int filters, int spatial)
    {
        for (int b = 0; b < batch; ++b)
        {
            for (int f = 0; f < filters; ++f)
            {
                for (int i = 0; i < spatial; ++i)
                {
                    int index = b * filters * spatial + f * spatial + i;
                    x[index]  = (x[index] - mean[f]) / (std::sqrt(variance[f]) + 0.000001f);
                }
            }
        }
    }

    void mean_delta_cpu(float *delta, float *variance, int batch, int filters, int spatial, float *mean_delta)
    {
        for (int i = 0; i < filters; ++i)
        {
            mean_delta[i] = 0;
            for (int j = 0; j < batch; ++j)
            {
                for (int k = 0; k < spatial; ++k)
                {
                    int index = j * filters * spatial + i * spatial + k;
                    mean_delta[i] += delta[index];
                }
            }
            mean_delta[i] *= (-1.f / std::sqrt(variance[i] + 0.00001f));
        }
    }

    void variance_delta_cpu(float *x, float *delta, float *mean, float *variance, int batch, int filters, int spatial, float *variance_delta)
    {
        for (int i = 0; i < filters; ++i)
        {
            variance_delta[i] = 0;
            for (int j = 0; j < batch; ++j)
            {
                for (int k = 0; k < spatial; ++k)
                {
                    int index = j * filters * spatial + i * spatial + k;
                    variance_delta[i] += delta[index] * (x[index] - mean[i]);
                }
            }
            variance_delta[i] *= -0.5f * std::pow(variance[i] + 0.00001f, (float)(-3.0f / 2.0f));
        }
    }

    void normalize_delta_cpu(float *x, float *mean, float *variance, float *mean_delta, float *variance_delta, int batch, int filters, int spatial, float *delta)
    {
        for (int j = 0; j < batch; ++j)
        {
            for (int f = 0; f < filters; ++f)
            {
                for (int k = 0; k < spatial; ++k)
                {
                    int index    = j * filters * spatial + f * spatial + k;
                    delta[index] = delta[index] * 1.0f / (std::sqrt(variance[f] + 0.00001f)) + variance_delta[f] * 2.0f * (x[index] - mean[f]) / (spatial * batch) + mean_delta[f] / (spatial * batch);
                }
            }
        }
    }

    // softmax 分类函数
    void softmax(float *input, int classes, float *output, float temperature)
    {
        float sum     = 0;
        float largest = -FLT_MAX;

        // 求输入最大值
        for (int i = 0; i < classes; ++i)
        {
            if (input[i] > largest) largest = input[i];
        }

        // 求softmax 输入数据归一化(0, 1] 并得到归一化后的和
        for (int i = 0; i < classes; ++i)
        {
            float e = std::exp(input[i] / temperature - largest / temperature);
            sum += e;
            output[i] = e;
        }

        // 归一化后值变百分比
        for (int i = 0; i < classes; ++i)
        {
            output[i] /= sum;
        }
    }

    // input 输入数据起始地址
    // classes 分类数
    // batch 网络单次训练样本数量
    // output 输出数据起始地址
    void softmax_cpu(float *input, int classes, int batch, float *output, float temperature)
    {
        for (int b = 0; b < batch; ++b)
        {
            softmax(input + b * classes, classes, output + b * classes, temperature);
        }
    }

    // softmax cross-entropy loss 交叉熵损失
    void softmax_cross_entropy_cpu(int batch, int classes, float *pred, float *truth, float *delta, float *error)
    {
        int size = batch * classes;
        for (int i = 0; i < size; ++i)
        {
            float t  = truth[i];
            float p  = pred[i];
            error[i] = (t) ? -std::log(p) : 0;
            delta[i] = t - p;
        }
    }

    void shortcut_cpu(int batch, int w1, int h1, int c1, float *add, int w2, int h2, int c2, float s1, float s2, float *out)
    {
        int stride = w1 / w2;
        int sample = w2 / w1;
        assert(stride == h1 / h2);
        assert(sample == h2 / h1);
        if (stride < 1) stride = 1;
        if (sample < 1) sample = 1;
        int minw = (w1 < w2) ? w1 : w2;
        int minh = (h1 < h2) ? h1 : h2;
        int minc = (c1 < c2) ? c1 : c2;

        for (int b = 0; b < batch; ++b)
        {
            for (int k = 0; k < minc; ++k)
            {
                for (int j = 0; j < minh; ++j)
                {
                    for (int i = 0; i < minw; ++i)
                    {
                        int out_index  = i * sample + w2 * (j * sample + h2 * (k + c2 * b));
                        int add_index  = i * stride + w1 * (j * stride + h1 * (k + c1 * b));
                        out[out_index] = s1 * out[out_index] + s2 * add[add_index];
                    }
                }
            }
        }
    }

    void upsample_cpu(float *in, int w, int h, int c, int batch, int stride, int forward, float scale, float *out)
    {
        int sh = stride * h;
        int sw = stride * w;
        for (int b = 0; b < batch; ++b)
        {
            for (int k = 0; k < c; ++k)
            {
                for (int j = 0; j < sh; ++j)
                {
                    for (int i = 0; i < sw; ++i)
                    {
                        int in_index  = b * w * h * c + k * w * h + (j / stride) * w + i / stride;
                        int out_index = b * w * h * c * stride * stride + k * w * h * stride * stride + j * w * stride + i;
                        if (forward)
                            out[out_index] = scale * in[in_index];
                        else
                            in[in_index] += scale * out[out_index];
                    }
                }
            }
        }
    }
}