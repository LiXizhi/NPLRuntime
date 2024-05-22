#ifndef __CONNECTED_LAYER_H__
#define __CONNECTED_LAYER_H__

#include "Layer.h"

class ConnectedLayer : public Layer
{
public:
    ConnectedLayer(Net *net, Layer *prev_layer, Options *options);

    virtual void Forward();
    virtual void Backward();
    virtual float Loss();
    virtual void PrintPredicts(std::ostream* os);
protected:
    virtual void DebugBackward();
    virtual void DebugUpdate();

protected:
    int m_activation;
};
#endif