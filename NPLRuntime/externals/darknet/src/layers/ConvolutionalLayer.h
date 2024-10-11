#ifndef __DARKNET_CONVOLUTIONAL_LAYER_H__
#define __DARKNET_CONVOLUTIONAL_LAYER_H__

#include "Layer.h"
namespace darknet
{
    class ConvolutionalLayer : public Layer
    {
    public:
        ConvolutionalLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);

        virtual void Forward();
        virtual void Backward();

        virtual int GetWorkspaceSize() { return m_in_filters * m_size * m_size * m_out_width * m_out_height; } // k * n

    public:
        inline void SetSize(int size) { m_size = size; }
        inline void SetStride(int stride) { m_stride = stride; }
        inline void SetPadding(int padding) { m_padding = padding; }
        inline int GetKernelSize() { return m_in_filters * m_size * m_size; }
        inline int GetSize() { return m_size; }
        inline int GetStride() { return m_stride; }

    protected:
        virtual void DebugForward() {}
        virtual void DebugBackward() {}
        virtual void DebugUpdate();

    protected:
        inline static int get_out_width(int in_width, int padding_left, int padding_right, int kernel_width, int stride) { return (in_width + padding_left + padding_right - kernel_width) / stride + 1; }
        inline static int get_out_height(int in_height, int padding_top, int padding_bottom, int kernel_height, int stride) { return (in_height + padding_top + padding_bottom - kernel_height) / stride + 1; }

    protected:
        int m_size;
        int m_stride;
        int m_padding;
        int m_activation;
    };
}
#endif