#ifndef __DARKNET_CONNECTED_LAYER_H__
#define __DARKNET_CONNECTED_LAYER_H__

#include "Layer.h"
namespace darknet
{
    class ConnectedLayer : public Layer
    {
    public:
        ConnectedLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);

        virtual void Forward();
        virtual void Backward();
        virtual float Loss();
        virtual void PrintPredicts(std::ostream *os);

    protected:
        virtual void DebugBackward();
        virtual void DebugUpdate();

    protected:
        int m_activation;
    };
}
#endif