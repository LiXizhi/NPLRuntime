#include "ShortCutLayer.h"
#include "Net.h"
#include "Options.h"
#include "activations.h"
#include "blas.h"

/*
[shortcut]
from=-4
alpha=1
beta=1
activation=linear
*/

ShortCutLayer::ShortCutLayer(Net *net, Layer *prev_layer, Options *options) : Layer(net, prev_layer, options)
{
    m_name = "shortcut";
    m_type = s_shortcut;

    m_alpha       = options->GetOption("alpha", 1.0f);
    m_beta        = options->GetOption("beta", 1.0f);
    m_activation  = get_activation_by_name(options->GetOption("activation", "leaky"));
    int from      = options->GetOption("from", -2);
    m_input_layer = from < 0 ? (from + GetLayerIndex()) : from;
    if (net->GetLayer(m_input_layer) == nullptr)
    {
        std::cerr << "shortcut layer index = " << GetLayerIndex() << " invalid from = " << from << std::endl;
        m_out_width   = 0;
        m_out_height  = 0;
        m_out_filters = 0;
    }
    else
    {
        m_out_width   = m_in_width;
        m_out_height  = m_in_height;
        m_out_filters = m_in_filters;
    }

    auto batch_out_size = GetBatchOutSize();
    m_outputs.assign(batch_out_size, 0.f);
    m_deltas.assign(batch_out_size, 0.f);
}

void ShortCutLayer::Forward()
{
    auto in_datas       = GetInDatas();
    auto out_datas      = GetOutDatas();
    auto batch_out_size = GetBatchOutSize();
    auto layer          = GetNet()->GetLayer(m_input_layer);

    copy_cpu(batch_out_size, in_datas, 1, out_datas, 1);
    shortcut_cpu(m_batch, layer->GetOutWidth(), layer->GetOutHeight(), layer->GetOutFilters(), layer->GetOutDatas(), m_out_width, m_out_height, m_out_filters, m_alpha, m_beta, out_datas);
    activate_array(out_datas, batch_out_size, m_activation);
}

void ShortCutLayer::Backward()
{
    auto out_datas      = GetOutDatas();
    auto in_deltas      = GetInDeltas();
    auto batch_out_size = GetBatchOutSize();
    auto out_deltas     = GetOutDeltas();
    auto layer          = GetNet()->GetLayer(m_input_layer);

    gradient_array(out_datas, batch_out_size, m_activation, in_deltas);
    axpy_cpu(batch_out_size, m_alpha, in_deltas, 1, out_deltas, 1);
    // 加减忽略 乘法互换 out_datas = alpha * out_datas + beta * in_datas => out_datas = beta * in_datas  => out_deltas = beta * in_deltas
    shortcut_cpu(m_batch, m_out_width, m_out_height, m_out_filters, in_deltas, layer->GetOutWidth(), layer->GetOutHeight(), layer->GetOutFilters(), 1, m_beta, layer->GetInDeltas());
}