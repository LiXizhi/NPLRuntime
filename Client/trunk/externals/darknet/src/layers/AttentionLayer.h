#ifndef __DARKNET_ATTENTION_LAYER_H__
#define __DARKNET_ATTENTION_LAYER_H__

#include "Layer.h"
namespace darknet
{
    class AttentionLayer : public Layer
    {
    public:
        AttentionLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);
    };
}
#endif