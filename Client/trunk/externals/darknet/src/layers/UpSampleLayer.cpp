#include "UpSampleLayer.h"
namespace darknet
{
    /*
    [upsample]
    stride=2
    scale=1
    */

    UpSampleLayer::UpSampleLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options) : Layer(net, prev_layer, options)
    {
        m_name    = "upsample";
        m_type    = s_upsample;
        m_reverse = false;

        m_stride = options->GetOption("stride", 1);
        m_scale  = options->GetOption("scale", 1);

        if (m_stride < 0)
        {
            m_stride  = -m_stride;
            m_reverse = true;
        }

        m_out_width   = m_stride * m_in_width;
        m_out_height  = m_stride * m_in_height;
        m_out_filters = m_in_filters;

        auto batch_out_size = GetBatchOutSize();
        m_outputs.assign(batch_out_size, 0.f);
        m_deltas.assign(batch_out_size, 0.f);
    }

    void UpSampleLayer::Forward()
    {
        auto in_datas  = GetInDatas();
        auto out_datas = GetOutDatas();
        fill_cpu(GetBatchOutSize(), 0, out_datas, 1);
        if (m_reverse)
        {
            upsample_cpu(out_datas, m_out_width, m_out_height, m_out_filters, m_batch, m_stride, 0, m_scale, in_datas);
        }
        else
        {
            upsample_cpu(in_datas, m_in_width, m_in_height, m_in_filters, m_batch, m_stride, 1, m_scale, out_datas);
        }
    }

    void UpSampleLayer::Backward()
    {
        auto in_deltas  = GetInDeltas();
        auto out_deltas = GetOutDeltas();
        if (out_deltas == nullptr) return;
        if (m_reverse)
        {
            upsample_cpu(in_deltas, m_out_width, m_out_height, m_out_filters, m_batch, m_stride, 1, m_scale, out_deltas);
        }
        else
        {
            upsample_cpu(out_deltas, m_in_width, m_in_height, m_in_filters, m_batch, m_stride, 0, m_scale, in_deltas);
        }
    }
}