#ifndef __ROUTE_LAYER_H__
#define __ROUTE_LAYER_H__

#include "Layer.h"

class RouteLayer : public Layer
{
public:
    RouteLayer(Net *net, Layer *prev_layer, Options *options);

    virtual void Forward();
    virtual void Backward();

protected:
    std::vector<int> m_input_layers;
};
#endif