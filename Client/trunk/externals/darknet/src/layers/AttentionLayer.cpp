#include "AttentionLayer.h"

AttentionLayer::AttentionLayer(Net *net, Layer *prev_layer, Options *options): Layer(net, prev_layer, options)
{   
    m_name = "attention";
    m_type = s_attention;
}

