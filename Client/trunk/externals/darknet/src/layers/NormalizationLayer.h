#ifndef __NORMALIZATION_LAYER_H__
#define __NORMALIZATION_LAYER_H__
#include "Layer.h"

class NormalizationLayer : public Layer
{
public:
    NormalizationLayer(Net *net, Layer *prev_layer, Options *options);

    virtual void Resize();
    virtual void Forward();
    virtual void Backward();

protected:
    int m_normalization_type;

    // s_normalization_default
    std::vector<float> m_means;
    std::vector<float> m_variances;

    // s_normalization_channel_shared
    std::vector<float> m_squared;
    std::vector<float> m_norms;
    float m_alpha;
    float m_beta;
    float m_kappa;
    int m_size;

protected:
    static const int s_normalization_default        = 0;
    static const int s_normalization_channel_shared = 1;
    static const int s_normalization_spatial        = 2;
};

#endif