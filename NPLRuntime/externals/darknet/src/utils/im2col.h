#ifndef __DARKNET_IM2COL_H__
#define __DARKNET_IM2COL_H__
namespace darknet
{
    void im2col(const float *data_im, const int channels, const int height, const int width, const int kernel_h, const int kernel_w, const int pad_h, const int pad_w, const int stride_h, const int stride_w, const int output_h, const int output_w, float *data_col);
    void col2im(const float *data_col, const int channels, const int height, const int width, const int kernel_h, const int kernel_w, const int pad_h, const int pad_w, const int stride_h, const int stride_w, const int output_h, const int output_w, float *data_im);
}
#endif