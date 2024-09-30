#ifndef __DARKNET_POOL_LAYER_H__
#define __DARKNET_POOL_LAYER_H__

#include "Layer.h"
namespace darknet
{
    class MaxPoolLayer : public Layer
    {
    public:
        MaxPoolLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);

        virtual void Resize();
        virtual void Forward();
        virtual void Backward();

    protected:
        int m_size;
        int m_stride;
        int m_padding;

        std::vector<int> m_indexes;
    };
}
#endif