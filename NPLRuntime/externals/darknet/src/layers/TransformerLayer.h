#ifndef __DARKNET_TRANSFORMER_LAYER_H__
#define __DARKNET_TRANSFORMER_LAYER_H__

#include "Layer.h"
namespace darknet
{
    class TransformerLayer : public Layer
    {
    public:
        TransformerLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);

    protected:
        std::vector<float> m_weight_token_embeddings;    // weight token embeddings
        std::vector<float> m_weight_position_embeddings; // weight position embeddings
    };
}
#endif