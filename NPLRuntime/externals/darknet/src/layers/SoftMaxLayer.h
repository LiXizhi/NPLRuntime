#ifndef __DARKNET_SOFTMAX_LAYER_H__
#define __DARKNET_SOFTMAX_LAYER_H__

#include "Layer.h"
namespace darknet
{
    class SoftMaxLayer : public Layer
    {
    public:
        SoftMaxLayer(Net *net, Layer *prev_layer, std::shared_ptr<Options> options);

        virtual void Resize();
        virtual void Forward();
        virtual void Backward();
        virtual float Loss();

        virtual void PrintPredicts(std::ostream *ofs);

    protected:
        virtual void DebugBackward();

    protected:
        float m_temperature; // 温度 越大结果越平均
    };
}
#endif