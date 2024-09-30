#ifndef __DARKNET_DROP_OUT_LAYER_H__
#define __DARKNET_DROP_OUT_LAYER_H__

#include "Layer.h"
namespace darknet
{
    class DropOutLayer : public Layer
    {
    public:
        DropOutLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);

        virtual void Resize();
        virtual void Forward();
        virtual void Backward();

    protected:
        float m_probability;
        float m_scale;
        std::vector<float> m_randoms;
    };
}
#endif