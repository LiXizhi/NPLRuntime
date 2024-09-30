#ifndef __DARKNET_UP_SAMPLE_LAYER_H__
#define __DARKNET_UP_SAMPLE_LAYER_H__

#include "Layer.h"
namespace darknet
{
    class UpSampleLayer : public Layer
    {
    public:
        UpSampleLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);

        virtual void Forward();
        virtual void Backward();

    protected:
        int m_stride;
        float m_scale;
        bool m_reverse;
    };
}
#endif