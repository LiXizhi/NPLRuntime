#include "TransformerLayer.h"
namespace darknet
{
    TransformerLayer::TransformerLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options) : Layer(net, prev_layer, options)
    {
        m_name = "transformer";
        m_type = s_attention;
    }

}