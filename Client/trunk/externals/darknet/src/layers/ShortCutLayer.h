#ifndef __SHORT_CUT_LAYER_H__
#define __SHORT_CUT_LAYER_H__

#include "Layer.h"

class ShortCutLayer : public Layer
{
public:
    ShortCutLayer(Net *net, Layer *prev_layer, Options *options);

    virtual void Forward();
    virtual void Backward();

protected:
    int m_activation;
    int m_input_layer;
    float m_alpha;
    float m_beta;
};
#endif