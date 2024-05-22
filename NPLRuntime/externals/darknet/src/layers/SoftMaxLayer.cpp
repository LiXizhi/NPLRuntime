#include "SoftMaxLayer.h"
#include "Net.h"
#include "Options.h"
#include "blas.h"
#include "utils.h"

SoftMaxLayer::SoftMaxLayer(Net *net, Layer *prev_layer, Options *options) : Layer(net, prev_layer, options)
{
    m_name = "softmax";
    m_type = s_softmax;

    m_temperature = options->GetOption("temperature", 1.0f);

    m_out_width   = 1;
    m_out_height  = 1;
    m_out_filters = GetInSize();
}

void SoftMaxLayer::Resize()
{
    Layer::Resize();

    auto batch_out_size = GetBatchOutSize();
    m_losses.assign(batch_out_size, 0.0f);
}

void SoftMaxLayer::Forward()
{
    auto in_datas  = GetInDatas();
    auto out_datas = GetOutDatas();
    auto in_size   = GetInSize();
    auto batch     = GetBatch();
    softmax_cpu(in_datas, in_size, batch, out_datas, m_temperature);
}

float SoftMaxLayer::Loss()
{
    auto in_size = GetInSize();
    auto batch   = GetBatch();
    softmax_cross_entropy_cpu(batch, in_size, GetOutDatas(), GetNet()->GetInTruths(), GetInDeltas(), GetLossesData());
    m_cost = sum_array(GetLossesData(), GetLossesSize()) / batch;
    return m_cost;
}

void SoftMaxLayer::Backward()
{
    auto prev_layer = GetPrevLayer();
    if (prev_layer == nullptr) return;
    axpy_cpu(GetBatchInSize(), 1, GetInDeltas(), 1, prev_layer->GetInDeltas(), 1);

    DebugBackward();
}

void SoftMaxLayer::DebugBackward()
{
    if (!GetNet()->IsTrain()) return;
    auto out_datas = GetOutDatas();
    auto in_size   = GetInSize();
    auto batch     = GetBatch();
    auto in_truths = GetNet()->GetInTruths();
    auto in_deltas = GetInDeltas();
    for (int batch = 0; batch < m_batch; batch++)
    {
        auto os = GetInputOS(batch);
        if (os != nullptr)
        {
            *os << "layer = softmax action = backward" << std::endl;
            DebugDatas(os, out_datas + batch * in_size, in_size, "predicts");
            DebugDatas(os, in_deltas + batch * in_size, in_size, "in_deltas");
        }
    }
}

// std::vector<float> *SoftMaxLayer::GetPredicts()
// {
//     static std::vector<float> s_predicts;
//     s_predicts.clear();

//     auto out_datas = GetOutDatas();
//     auto out_size  = GetOutSize();
//     for (int b = 0; b < m_batch; b++)
//     {
//         auto max_value = 0.f;
//         auto max_index = 0;
//         auto outputs   = out_datas + b * out_size;
//         for (int i = 0; i < out_size; i++)
//         {
//             if (max_value < outputs[i])
//             {
//                 max_value = outputs[i];
//                 max_index = i;
//             }
//         }
//         s_predicts.push_back(max_index);
//         s_predicts.push_back(max_value);
//     }
//     return &s_predicts;
// }

void SoftMaxLayer::PrintPredicts(std::ostream *os)
{
    if (os == nullptr) return;

    *os << "softmax predict:" << std::endl;

    auto out_datas = GetOutDatas();
    auto out_size  = GetOutSize();

    for (int b = 0; b < m_batch; b++)
    {
        auto max_value = 0.f;
        auto max_index = 0;
        auto outputs   = out_datas + b * out_size;
        if (m_batch > 1) *os << "data index = " << b << std::endl;
        for (int i = 0; i < out_size; i++)
        {
            *os << "classid = " << i << " prob = " << outputs[i] << " " << std::endl;
            // *os << i << " " << outputs[i] << std::endl;

            if (max_value < outputs[i])
            {
                max_value = outputs[i];
                max_index = i;
            }
        }
        *os << "result: " << max_index << " " << max_value << std::endl;
    }
}
