#include "DropOutLayer.h"
namespace darknet
{
    DropOutLayer::DropOutLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options) : Layer(net, prev_layer, options)
    {
        m_name        = "dropout";
        m_type        = s_dropout;
        m_probability = options->GetOption("probability", 0.5f);
        m_scale       = 1.f / (1.f - m_probability);

        m_out_width   = m_in_width;
        m_out_height  = m_in_height;
        m_out_filters = m_in_filters;

        m_randoms.assign(GetBatchOutSize(), 0.f);
    }

    void DropOutLayer::Resize()
    {
        Layer::Resize();
        m_randoms.assign(GetBatchOutSize(), 0.f);
    }

    void DropOutLayer::Forward()
    {
        auto in_datas       = GetInDatas();
        auto out_datas      = GetOutDatas();
        auto batch_out_size = GetBatchOutSize();
        auto randoms        = m_randoms.data();

        for (int i = 0; i < batch_out_size; ++i)
        {
            float r    = random_float(0.f, 1.f);
            randoms[i] = r;
            if (r < m_probability)
            {
                out_datas[i] = 0;
            }
            else
            {
                out_datas[i] = m_scale * in_datas[i];
            }
        }
    }

    void DropOutLayer::Backward()
    {
        auto in_deltas  = GetInDeltas();
        auto out_deltas = GetOutDeltas();
        if (out_deltas == nullptr) return;
        auto batch_out_size = GetBatchOutSize();
        auto randoms        = m_randoms.data();
        for (int i = 0; i < batch_out_size; ++i)
        {
            float r = randoms[i];
            if (r < m_probability)
            {
                out_deltas[i] = 0;
            }
            else
            {
                out_deltas[i] = m_scale * in_deltas[i];
            }
        }
    }
}