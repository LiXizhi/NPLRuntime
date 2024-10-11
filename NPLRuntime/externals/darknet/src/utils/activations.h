#ifndef __DARKNET_ACTIVATIONS_H__
#define __DARKNET_ACTIVATIONS_H__

#include <cmath>
#include <functional>
#include <memory>
#include <string>
namespace darknet
{
    void activate_array(float *datas, int size, int active_type);
    void gradient_array(float *datas, int size, int active_type, float *deltas);
    int get_activation_by_name(const std::string &activation_name);
}
#endif