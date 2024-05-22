#ifndef __SOFTMAX_LAYER_H__
#define __SOFTMAX_LAYER_H__

#include "Layer.h"

class SoftMaxLayer : public Layer
{
public:
    SoftMaxLayer(Net* net, Layer* prev_layer, Options *options);

    virtual void Resize();
    virtual void Forward();
    virtual void Backward();
    virtual float Loss();

    virtual void PrintPredicts(std::ostream* ofs);

protected:
    virtual void DebugBackward();

protected:
    float m_temperature; // 温度 越大结果越平均
};

#endif