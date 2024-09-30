#include "AttentionLayer.h"
namespace darknet
{
    AttentionLayer::AttentionLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options) : Layer(net, prev_layer, options)
    {
        m_name = "attention";
        m_type = s_attention;
    }

}