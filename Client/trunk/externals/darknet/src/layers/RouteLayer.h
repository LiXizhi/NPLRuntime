#ifndef __DARKNET_ROUTE_LAYER_H__
#define __DARKNET_ROUTE_LAYER_H__

#include "Layer.h"
namespace darknet
{
    class RouteLayer : public Layer
    {
    public:
        RouteLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);

        virtual void Forward();
        virtual void Backward();

    protected:
        std::vector<int> m_input_layers;
    };
}
#endif