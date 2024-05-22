#ifndef __ATTENTION_LAYER_H__
#define __ATTENTION_LAYER_H__

#include "Layer.h"

class AttentionLayer : public Layer
{
public:
    AttentionLayer(Net *net, Layer *prev_layer, Options *options);
};

#endif