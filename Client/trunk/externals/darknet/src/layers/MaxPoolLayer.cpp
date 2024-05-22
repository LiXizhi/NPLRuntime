#include "MaxPoolLayer.h"
#include "Options.h"

MaxPoolLayer::MaxPoolLayer(Net *net, Layer *prev_layer, Options *options) : Layer(net, prev_layer, options)
{
    m_name    = "max_pool";
    m_type    = s_maxpool;
    m_size    = options->GetOption("size", 2);
    m_stride  = options->GetOption("stride", m_size);
    m_padding = options->GetOption("padding", m_size - 1);

    m_out_width   = (m_in_width + m_padding - m_size) / m_stride + 1;
    m_out_height  = (m_in_height + m_padding - m_size) / m_stride + 1;
    m_out_filters = m_in_filters;

    auto batch_out_size = GetBatchOutSize();
    m_outputs.assign(batch_out_size, 0.f);
    m_deltas.assign(batch_out_size, 0.f);
    m_indexes.assign(batch_out_size, 0);
}

void MaxPoolLayer::Resize()
{
    Layer::Resize();

    auto batch_out_size = GetBatchOutSize();
    m_indexes.assign(batch_out_size, 0);
}

void MaxPoolLayer::Forward()
{
    int w_offset = -m_padding / 2;
    int h_offset = -m_padding / 2;

    int h = m_out_height;
    int w = m_out_width;
    int c = m_out_filters;

    auto in_datas  = GetInDatas();
    auto out_datas = GetOutDatas();
    auto indexes   = m_indexes.data();
    for (int b = 0; b < m_batch; ++b)
    {
        for (int k = 0; k < c; ++k)
        {
            for (int i = 0; i < h; ++i)
            {
                for (int j = 0; j < w; ++j)
                {
                    int out_index = j + w * (i + h * (k + c * b));
                    float max     = -FLT_MAX;
                    int max_i     = -1;
                    for (int n = 0; n < m_size; ++n)
                    {
                        for (int m = 0; m < m_size; ++m)
                        {
                            int cur_h  = h_offset + i * m_stride + n;
                            int cur_w  = w_offset + j * m_stride + m;
                            int index  = cur_w + m_in_width * (cur_h + m_in_height * (k + b * m_in_filters));
                            bool valid = cur_h >= 0 && cur_h < m_in_height && cur_w >= 0 && cur_w < m_in_width;
                            float val  = valid ? in_datas[index] : -FLT_MAX;
                            max_i      = (val > max) ? index : max_i;
                            max        = (val > max) ? val : max;
                        }
                    }
                    out_datas[out_index] = max;
                    indexes[out_index]   = max_i;
                }
            }
        }
    }
}

void MaxPoolLayer::Backward()
{
    auto out_deltas = GetOutDeltas();
    if (out_deltas == nullptr) return;

    auto batch_out_size = GetBatchOutSize();
    auto in_deltas      = GetInDeltas();
    auto indexes        = m_indexes.data();

    for (int i = 0; i < batch_out_size; ++i)
    {
        int index = indexes[i];
        out_deltas[index] += in_deltas[i];
    }
}