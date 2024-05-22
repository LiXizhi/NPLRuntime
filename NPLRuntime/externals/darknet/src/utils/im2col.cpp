#include <memory>
#include <cstring>
#include "im2col.h"

inline static int is_a_ge_zero_and_a_lt_b(int a, int b) { return (unsigned int)(a) < (unsigned int)(b); }

// https://github.com/BVLC/caffe/blob/master/src/caffe/util/im2col.cpp
// 将输入数据转换为卷积核大小的特征图矩阵 卷积核矩阵的每个元素就一个特征图
void im2col(const float *data_im, const int channels, const int height, const int width, const int kernel_h, const int kernel_w, const int pad_h, const int pad_w, const int stride_h, const int stride_w, const int output_h, const int output_w, float *data_col)
{
    const int channel_size = height * width;
    int channel, kernel_row, kernel_col, output_rows, output_col;

    // 遍历每张特征图
    for (channel = channels; channel--; data_im += channel_size)
    {
        // 遍历卷积核
        for (kernel_row = 0; kernel_row < kernel_h; kernel_row++)
        {
            for (kernel_col = 0; kernel_col < kernel_w; kernel_col++)
            {
                // 计算卷积核中的点(kernel_row, kernel_col)在特征图上的所有位置
                int input_row = -pad_h + kernel_row;
                for (output_rows = output_h; output_rows; output_rows--)
                {
                    if (!is_a_ge_zero_and_a_lt_b(input_row, height))
                    {
                        for (output_col = output_w; output_col; output_col--)
                        {
                            *(data_col++) = 0;
                        }
                    }
                    else
                    {
                        int input_col = -pad_w + kernel_col;
                        for (output_col = output_w; output_col; output_col--)
                        {
                            if (is_a_ge_zero_and_a_lt_b(input_col, width))
                            {
                                *(data_col++) = data_im[input_row * width + input_col];
                            }
                            else
                            {
                                *(data_col++) = 0;
                            }
                            input_col += stride_w;
                        }
                    }
                    input_row += stride_h;
                }
            }
        }
    }

    // return s_data_cols.data();
}


// https://github.com/BVLC/caffe/blob/master/src/caffe/util/im2col.cpp
void col2im(const float *data_col, const int channels, const int height, const int width, const int kernel_h, const int kernel_w, const int pad_h, const int pad_w, const int stride_h, const int stride_w, const int output_h, const int output_w, float *data_im)
{
    memset(data_im, 0, sizeof(float) * height * width * channels);

    const int channel_size = height * width;
    int channel, kernel_row, kernel_col, output_rows, output_col;

    for (channel = channels; channel--; data_im += channel_size)
    {
        for (kernel_row = 0; kernel_row < kernel_h; kernel_row++)
        {
            for (kernel_col = 0; kernel_col < kernel_w; kernel_col++)
            {
                int input_row = -pad_h + kernel_row;
                for (output_rows = output_h; output_rows; output_rows--)
                {
                    if (!is_a_ge_zero_and_a_lt_b(input_row, height))
                    {
                        data_col += output_w;
                    }
                    else
                    {
                        int input_col = -pad_w + kernel_col;
                        for (output_col = output_w; output_col; output_col--)
                        {
                            if (is_a_ge_zero_and_a_lt_b(input_col, width))
                            {
                                data_im[input_row * width + input_col] += *data_col;
                            }
                            data_col++;
                            input_col += stride_w;
                        }
                    }
                    input_row += stride_h;
                }
            }
        }
    }
}
