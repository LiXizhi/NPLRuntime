#include "RouteLayer.h"
#include "Net.h"
#include "Options.h"
#include "blas.h"

/*
[route]
layers = -4
[route]
layers = -1, 8
*/
RouteLayer::RouteLayer(Net *net, Layer *prev_layer, Options *options) : Layer(net, prev_layer, options)
{
    m_name = "route";
    m_type = s_route;

    std::vector<int> layers;
    options->ParseOption("layers", "", layers);

    int layer_index = GetLayerIndex();
    int layers_size = layers.size();
    for (int i = 0; i < layers_size; i++)
    {
        int layer_index = layers[i] < 0 ? (layer_index + layers[i]) : layers[i];
        auto layer      = net->GetLayer(layer_index);
        if (layer == nullptr)
        {
            std::cerr << "route layer index = " << layer_index << " invalid layer = " << layers[i] << std::endl;
            continue;
        }
        auto layer_out_width   = layer->GetOutWidth();
        auto layer_out_height  = layer->GetOutHeight();
        auto layer_out_filters = layer->GetOutFilters();

        if (m_out_filters == 0)
        {
            m_out_width   = layer_out_width;
            m_out_height  = layer_out_height;
            m_out_filters = layer_out_filters;
            m_input_layers.push_back(layer_index);
        }
        else
        {
            if (m_out_width && layer_out_width && m_out_height == layer_out_height)
            {
                m_out_filters += layer_out_filters;
                m_input_layers.push_back(layer_index);
            }
            else
            {
                std::cerr << "route layer index = " << layer_index << " route layer width and height must be the same!!!" << std::endl;
            }
        }
    }

    m_in_width   = m_out_width;
    m_in_height  = m_out_height;
    m_in_filters = m_out_filters;

    auto batch_out_size = GetBatchOutSize();
    m_outputs.assign(batch_out_size, 0.f);
    m_deltas.assign(batch_out_size, 0.f);
}

void RouteLayer::Forward()
{
    int input_layer_offset = 0;
    int intput_layers_size = m_input_layers.size();
    auto out_datas         = GetOutDatas();
    auto out_size          = GetOutSize();
    for (int i = 0; i < intput_layers_size; ++i)
    {
        auto index    = m_input_layers[i];
        auto layer    = GetNet()->GetLayer(index);
        auto in_datas = layer->GetOutDatas();
        int in_size   = layer->GetOutSize();
        for (int j = 0; j < m_batch; ++j)
        {
            copy_cpu(in_size, in_datas + j * in_size, 1, out_datas + j * out_size + input_layer_offset, 1);
        }
        input_layer_offset += in_size;
    }
}

void RouteLayer::Backward()
{
    int input_layer_offset = 0;
    int intput_layers_size = m_input_layers.size();
    auto in_deltas         = GetInDeltas();
    auto out_size          = GetOutSize();
    for (int i = 0; i < intput_layers_size; ++i)
    {
        auto index     = m_input_layers[i];
        auto layer     = GetNet()->GetLayer(index);
        auto in_deltas = layer->GetInDeltas();
        auto in_size   = layer->GetOutSize();
        for (int j = 0; j < m_batch; ++j)
        {
            axpy_cpu(in_size, 1, in_deltas + j * out_size + input_layer_offset, 1, in_deltas + j * in_size, 1);
        }
        input_layer_offset += in_size;
    }
}