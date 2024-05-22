#include "ConnectedLayer.h"
#include "Net.h"
#include "Options.h"
#include "activations.h"
#include "blas.h"
#include "gemm.h"
#include "utils.h"

#include <cmath>

ConnectedLayer::ConnectedLayer(Net *net, Layer *prev_layer, Options *options) : Layer(net, prev_layer, options)
{
    m_name        = "connnected";
    m_type        = s_connected;
    m_out_width   = 1;
    m_out_height  = 1;
    m_out_filters = options->GetOption("output", 1);
    m_activation  = get_activation_by_name(options->GetOption("activation", "linear"));

    auto in_size = GetInSize();
    m_weights.assign(m_out_filters * in_size, 0.0f);
    m_weights_deltas.assign(m_out_filters * in_size, 0.0f);
    m_biases.assign(m_out_filters, 0.0f);
    m_biases_deltas.assign(m_out_filters, 0.0f);

    float scale  = std::sqrt(2.0f / in_size);
    int nweights = m_weights.size();
    for (int i = 0; i < nweights; i++)
    {
        m_weights[i] = scale * random_float(-1, 1);
    }

    InitBatchNormalize();
}

void ConnectedLayer::Forward()
{
    auto out_datas = GetOutDatas();
    auto out_size  = GetOutSize();
    auto biases    = GetBiasesData();

    auto m = m_batch;
    auto k = GetInSize();
    auto n = out_size;
    auto a = GetInDatas();     // m * k
    auto b = GetWeightsData(); // n * k
    auto c = out_datas;        // m * n

    // 矩阵运算获取特征图数据
    gemm(0, 1, m, n, k, 1, a, k, b, k, 1, c, n);

    // 前向偏置
    if (IsBatchNormalize())
    {
        ForwardBatchNormalizeLayer();
    }
    else
    {
        ForwardAddBiases(out_datas, biases, m_batch, m_out_filters, m_out_width * m_out_height);
    }

    // 输出激活
    activate_array(out_datas, GetBatchOutSize(), m_activation);
}

float ConnectedLayer::Loss()
{
    // 计算deltas
    auto out_datas = GetOutDatas();
    auto in_deltas = GetInDeltas();
    auto in_truths = GetNet()->GetInTruths();
    auto losses    = GetLossesData();
    for (int b = 0; b < m_batch; b++)
    {
        auto os    = GetInputOS(b);
        auto index = b * m_out_filters;
        for (int i = 0; i < m_out_filters; i++)
        {
            in_deltas[index + i] = in_truths[index + i] - out_datas[index + i];
            losses[index + i]    = in_deltas[index + i];
            if (os != nullptr)
            {
                *os << "connected layer Loss => predict = " << out_datas[index + i] << " truth = " << in_truths[index + i] << " delta = " << in_deltas[index + i] << std::endl;
            }
        }
    }
    m_cost = sum_array(GetLossesData(), GetLossesSize()) / m_batch;
    return m_cost;
}

void ConnectedLayer::Backward()
{
    auto in_datas       = GetInDatas();
    auto out_datas      = GetOutDatas();
    auto biases         = GetBiasesData();
    auto in_deltas      = GetInDeltas();
    auto in_size        = GetInSize();
    auto out_size       = GetOutSize();
    auto batch_out_size = GetBatchOutSize();

    // 梯度增量
    gradient_array(out_datas, batch_out_size, m_activation, in_deltas);

    // 反向偏置
    if (IsBatchNormalize())
    {
        BackwardBatchNormalizeLayer();
    }
    else
    {
        BackwardAddBiases(m_biases_deltas.data(), in_deltas, m_batch, m_out_filters, m_out_width * m_out_height);
    }

    auto m   = m_batch;
    auto k   = in_size;
    auto n   = out_size;
    float *a = in_deltas;                        // m * n
    float *b = in_datas;                         // m * k
    float *c = GetWeightsDeltasData();           // n * k
    gemm(1, 0, n, k, m, 1, a, n, b, k, 1, c, k); // 计算权重增量

    auto out_deltas = GetOutDeltas();
    if (out_deltas != nullptr)
    {
        a = in_deltas;                               // m * n
        b = GetWeightsData();                        // n * k
        c = out_deltas;                              // m * k
        gemm(0, 0, m, k, n, 1, a, n, b, k, 1, c, k); // 计算增量
    }

    // DebugBackward();
}

void ConnectedLayer::PrintPredicts(std::ostream *os)
{
    *os << "layer = connnected action = predict" << std::endl;
    DebugDatas(os, m_outputs.data(), m_outputs.size(), "predicts");
}

void ConnectedLayer::DebugBackward()
{
    if (!GetNet()->IsTrain()) return;

    auto out_datas      = GetOutDatas();
    auto in_datas       = GetInDatas();
    auto in_size        = GetInSize();
    auto batch          = GetBatch();
    auto in_deltas      = GetInDeltas();
    auto biases_deltas  = GetBiasesDeltasData();
    auto weights_deltas = GetWeightsDeltasData();
    for (int batch = 0; batch < m_batch; batch++)
    {
        auto os = GetInputOS(batch);
        if (os != nullptr)
        {
            *os << "layer = connnected action = backward" << std::endl;
            // DebugDatas(os, out_datas + batch * in_size, in_size, m_out_filters, "output");
            DebugDatas(os, in_deltas, m_out_filters, "in_deltas");
            DebugDatas(os, in_datas, in_size, "in_datas");
            DebugDatas(os, weights_deltas, in_size, m_out_filters, "weights_deltas");
            DebugDatas(os, biases_deltas, m_out_filters, "biases_deltas");
        }
        in_deltas += batch * m_out_filters;
        in_datas += batch * in_size;
        weights_deltas += batch * in_size;
        biases_deltas += batch * m_out_filters;
    }
}

void ConnectedLayer::DebugUpdate()
{
    // auto biases  = GetBiasesData();
    // auto weights = GetWeightsData();
    // auto in_size = GetInSize();
    // for (int batch = 0; batch < m_batch; batch++)
    // {
    //     auto os = GetInputOFS(batch);
    //     if (os != nullptr)
    //     {
    //         *os << "layer = connnected action = update" << std::endl;
    //         DebugDatas(os, biases, m_out_filters, "biases");
    //         DebugDatas(os, weights, in_size, m_out_filters, "weights");
    //     }
    //     biases += m_out_filters;
    //     weights += batch * m_out_filters * m_out_width * m_out_height;
    // }
}