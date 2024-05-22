#ifndef __POOL_LAYER_H__
#define __POOL_LAYER_H__

#include "Layer.h"

class MaxPoolLayer : public Layer
{
public:
    MaxPoolLayer(Net *net, Layer *prev_layer, Options *options);

    virtual void Resize();
    virtual void Forward();
    virtual void Backward();

protected:
    int m_size;
    int m_stride;
    int m_padding;

    std::vector<int> m_indexes;
};
#endif