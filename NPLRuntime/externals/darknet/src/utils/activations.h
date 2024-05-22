#ifndef __ACTIVATIONS_H__
#define __ACTIVATIONS_H__

#include <memory>
#include <cmath>
#include <functional>
#include <string>

void activate_array(float *datas, int size, int active_type);
void gradient_array(float *datas, int size, int active_type, float* deltas);
int get_activation_by_name(const std::string& activation_name);

#endif