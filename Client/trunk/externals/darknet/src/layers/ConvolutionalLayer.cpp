#include "activations.h"
#include "blas.h"
#include "gemm.h"
#include "im2col.h"
#include "utils.h"

#include "ConvolutionalLayer.h"
#include "Net.h"
#include "Options.h"

ConvolutionalLayer::ConvolutionalLayer(Net *net, Layer *prev_layer, Options *options) : Layer(net, prev_layer, options)
{
    m_name        = "convolutional";
    m_type        = s_convolutional;
    m_size        = options->GetOption("size", 3);
    m_stride      = options->GetOption("stride", 1);
    m_padding     = options->GetOption("padding", 1);
    m_out_filters = options->GetOption("filters", 16);
    m_activation  = get_activation_by_name(options->GetOption("activation", "leaky"));
    m_out_width   = get_out_width(m_in_width, m_padding, m_padding, m_size, m_stride);
    m_out_height  = get_out_height(m_in_height, m_padding, m_padding, m_size, m_stride);

    auto kernel_size = GetKernelSize();
    m_weights.assign(m_out_filters * kernel_size, 0.0f);
    m_weights_deltas.assign(m_out_filters * kernel_size, 0.0f);
    m_biases.assign(m_out_filters, 0.0f);
    m_biases_deltas.assign(m_out_filters, 0.0f);

    float scale  = std::sqrt(1.0f / kernel_size);
    int nweights = m_weights.size();
    for (int i = 0; i < nweights; i++)
    {
        m_weights[i] = scale * random_float(-1, 1);
    }

    InitBatchNormalize();
}

void ConvolutionalLayer::Forward()
{
    auto in_datas  = GetInDatas();
    auto out_datas = GetOutDatas();
    auto workspace = GetNet()->GetWorkspace();

    auto weights_datas = GetWeightsData();
    auto in_size       = GetInSize();
    auto out_size      = GetOutSize();
    auto m             = GetOutImageCount();
    auto n             = GetOutImageSize();
    auto k             = GetKernelSize();
    for (int i = 0; i < m_batch; i++)
    {
        auto im = in_datas + i * in_size;
        auto a  = weights_datas;            // m * k   weights
        auto b  = workspace;                // k * n   in_data
        auto c  = out_datas + i * out_size; // m * n   out_data

        if (m_size == 1 && m_stride == 1)
        {
            b = im;
        }
        else
        {
            im2col(im, m_in_filters, m_in_height, m_in_width, m_size, m_size, m_padding, m_padding, m_stride, m_stride, m_out_height, m_out_width, b); // 提取卷积核特征图 k * n
        }

        gemm(0, 0, m, n, k, 1, a, k, b, n, 1, c, n); // 计算输出特征图数据
    }

    // 加上偏执项
    if (IsBatchNormalize())
    {
        ForwardBatchNormalizeLayer();
    }
    else
    {
        ForwardAddBiases(out_datas, GetBiasesData(), m_batch, m_out_filters, m_out_width * m_out_height);
    }

    // 激活函数
    activate_array(out_datas, GetBatchOutSize(), m_activation);
}

void ConvolutionalLayer::Backward()
{
    auto in_datas  = GetInDatas();
    auto in_deltas = GetInDeltas();
    auto workspace = GetNet()->GetWorkspace();

    // 激活函数的导数 * 输入的梯度 = 输出的梯度
    gradient_array(GetOutDatas(), GetBatchOutSize(), m_activation, in_deltas);

    // 梯度偏置求导得偏置增量  偏置不影响deltas  乘除忽略加减
    if (IsBatchNormalize())
    {
        BackwardBatchNormalizeLayer();
    }
    else
    {
        BackwardAddBiases(m_biases_deltas.data(), in_deltas, m_batch, m_out_filters, m_out_width * m_out_height);
    }

    auto in_size  = GetInSize();
    auto out_size = GetOutSize();
    auto m        = GetOutImageCount();
    auto n        = GetOutImageSize();
    auto k        = GetKernelSize();
    for (int i = 0; i < m_batch; i++)
    {
        auto im = in_datas + i * in_size;
        auto a  = in_deltas + i * out_size; // m * n  in_deltas
        auto b  = workspace;                // k * n  in_data
        auto c  = GetWeightsDeltasData();   // m * k  out_data_deltas  => weights_deltas

        if (m_size == 1 && m_stride == 1)
        {
            b = im;
        }
        else
        {
            im2col(im, m_in_filters, m_in_height, m_in_width, m_size, m_size, m_padding, m_padding, m_stride, m_stride, m_out_height, m_out_width, b); // 提取卷积核特征图 k * n
        }

        gemm(0, 1, m, k, n, 1, a, n, b, n, 1, c, k); // 计算权重增量

        // 输出 in_deltas
        auto prev_layer = GetPrevLayer();
        if (prev_layer != nullptr)
        {
            a = GetWeightsData();         // m * k  weights
            b = in_deltas + i * out_size; // m * n  in_deltas
            c = workspace;                // k * n  out_deltas

            gemm(1, 0, k, n, m, 1, a, k, b, n, 0, c, n); // 计算增量

            // k * n -> prev_layer m * n
            col2im(c, m_in_filters, m_in_height, m_in_width, m_size, m_size, m_padding, m_padding, m_stride, m_stride, m_out_height, m_out_width, prev_layer->GetInDeltas() + i * prev_layer->GetOutSize());
        }
    }
}

void ConvolutionalLayer::DebugUpdate()
{
    // auto biases_deltas    = GetBiasesDeltasData();
    // auto weights_deltas   = GetWeightsDeltasData();
    // auto biases = GetBiasesData();
    // auto weights = GetWeightsData();
    // for (int batch = 0; batch < m_batch; batch++)
    // {
    //     auto ofs = GetInputOFS(batch);
    //     if (ofs != nullptr)
    //     {
    //         *ofs << "layer = " << m_name << " action = update" << std::endl;
    //         for (int i = 0; i < m_out_filters; i++)
    //         {
    //             *ofs << "filter = " << i << " bias = " << biases[i] << std::endl;
    //             DebugDatas(ofs, weights + i * m_out_width * m_out_height, m_out_width, m_out_height, "weights");
    //         }
    //     }
    //     biases += m_out_filters;
    //     weights += batch * m_out_filters * m_out_width * m_out_height;
    // }
}