
#include "activations.h"
namespace darknet
{
    static int s_activation_type_max = 0;
    static const int s_leaky         = s_activation_type_max++;
    static const int s_linear        = s_activation_type_max++;
    static const int s_logistic      = s_activation_type_max++;
    static const int s_relu          = s_activation_type_max++;
    static const int s_relu6         = s_activation_type_max++;
    static const int s_elu           = s_activation_type_max++;
    static const int s_selu          = s_activation_type_max++;

    int get_activation_by_name(const std::string &activation_name)
    {
        if (activation_name == "leaky")
            return s_leaky;
        else if (activation_name == "linear")
            return s_linear;
        else if (activation_name == "logistic")
            return s_logistic;
        else if (activation_name == "relu")
            return s_relu;
        else
            return s_linear;
    }

    void LeakyActivateArray(float *datas, int size)
    {
        // static inline float leaky_activate(float x) { return (x > 0) ? x : .1f * x; }
        for (int i = 0; i < size; i++)
        {
            datas[i] = datas[i] * (datas[i] > 0 ? 1 : 0.1f);
        }
    }

    void LinearActivateArray(float *datas, int size)
    {
        // static inline float linear_activate(float x) { return x; }
    }

    // (-FLT_MAX, FLT_MAX() => (0, 1)
    void LogisticActivateArray(float *datas, int size)
    {
        // static inline float logistic_activate(float x) { return 1.f / (1.f + expf(-x)); }
        for (int i = 0; i < size; i++)
        {
            datas[i] = 1.0f / (1.0f + std::exp(-datas[i]));
        }
    }

    void ReluActivateArray(float *datas, int size)
    {
        // static inline float relu_activate(float x) { return x * (x > 0); }
        for (int i = 0; i < size; i++)
        {
            datas[i] = datas[i] > 0 ? datas[i] : 0;
        }
    }

    void activate_array(float *datas, int size, int active_type)
    {
        if (active_type == s_leaky)
        {
            LeakyActivateArray(datas, size);
        }
        else if (active_type == s_linear)
        {
            LinearActivateArray(datas, size);
        }
        else if (active_type == s_logistic)
        {
            LogisticActivateArray(datas, size);
        }
        else if (active_type == s_relu)
        {
            ReluActivateArray(datas, size);
        }
        else
        {
            // auto active_function = s_activate_functions[active_type];
            // for (int i = 0; i < size; i++)
            // {
            //     datas[i] = active_function(datas[i]);
            // }
        }
    }

    void LeakyGradientArray(float *datas, int size, float *deltas)
    {
        // static inline float leaky_gradient(float x) { return (x > 0) ? 1 : .1f; }
        for (int i = 0; i < size; i++)
        {
            deltas[i] = deltas[i] * (datas[i] > 0 ? 1 : 0.1f);
        }
    }

    void LinearGradientArray(float *datas, int size, float *deltas)
    {
        // static inline float linear_gradient(float x) { return 1; }
    }

    void LogisticGradientArray(float *datas, int size, float *deltas)
    {
        // static inline float logistic_gradient(float x) { return (1 - x) * x; }
        for (int i = 0; i < size; i++)
        {
            deltas[i] = deltas[i] * datas[i] * (1 - datas[i]);
        }
    }

    void ReluGradientArray(float *datas, int size, float *deltas)
    {
        // static inline float relu_gradient(float x) { return (x > 0); }
        for (int i = 0; i < size; i++)
        {
            deltas[i] = datas[i] > 0 ? deltas[i] : 0;
        }
    }

    void gradient_array(float *datas, int size, int active_type, float *deltas)
    {
        if (active_type == s_leaky)
        {
            LeakyGradientArray(datas, size, deltas);
        }
        else if (active_type == s_linear)
        {
            LinearGradientArray(datas, size, deltas);
        }
        else if (active_type == s_logistic)
        {
            LogisticGradientArray(datas, size, deltas);
        }
        else if (active_type == s_relu)
        {
            ReluGradientArray(datas, size, deltas);
        }
        else
        {
            // auto gradient_function = s_gradient_functions[active_type];
            // for (int i = 0; i < size; i++)
            // {
            //     deltas[i] = deltas[i] * gradient_function(datas[i]);
            // }
        }
    }
}

// static ActiveFunctionType s_activate_functions[] = {
//     linear_activate,
//     logistic_activate,
//     relu_activate,
//     relu6_activate,
//     leaky_activate,
//     elu_activate,
//     selu_activate,
// };

// static ActiveFunctionType s_gradient_functions[] = {
//     linear_gradient,
//     logistic_gradient,
//     relu_gradient,
//     relu6_gradient,
//     leaky_gradient,
//     elu_gradient,
//     selu_gradient,
// };

// using ActiveFunctionType   = float (*)(float);
// using GradientFunctionType = float (*)(float);

// static inline float stair_activate(float x)
// {
//     int n = floorf(x);
//     if (n % 2 == 0)
//         return floorf(x / 2.f);
//     else
//         return (x - n) + floorf(x / 2.f);
// }
// static inline float hardtan_activate(float x)
// {
//     if (x < -1)
//         return -1;
//     if (x > 1)
//         return 1;
//     return x;
// }
// static inline float linear_activate(float x) { return x; }
// static inline float logistic_activate(float x) { return 1.f / (1.f + expf(-x)); }
// static inline float loggy_activate(float x) { return 2.f / (1.f + expf(-x)) - 1; }
// static inline float relu_activate(float x) { return x * (x > 0); }
// static inline float relu6_activate(float x) { return x < 0 ? 0 : (x > 6 ? 6 : x); }
// static inline float elu_activate(float x) { return (x >= 0) * x + (x < 0) * (expf(x) - 1); }
// static inline float selu_activate(float x) { return (x >= 0) * 1.0507f * x + (x < 0) * 1.0507f * 1.6732f * (expf(x) - 1); }
// static inline float relie_activate(float x) { return (x > 0) ? x : .01f * x; }
// static inline float ramp_activate(float x) { return x * (x > 0) + .1f * x; }
// static inline float leaky_activate(float x) { return (x > 0) ? x : .1f * x; }
// // static inline float tanh_activate(float x){return (expf(2*x)-1)/(expf(2*x)+1);}
// static inline float tanh_activate(float x) { return (2 / (1 + expf(-2 * x)) - 1); }
// static inline float gelu_activate(float x) { return (0.5 * x * (1 + tanhf(0.797885 * x + 0.035677 * powf(x, 3)))); }
// static inline float softplus_activate(float x, float threshold)
// {
//     if (x > threshold)
//         return x; // too large
//     else if (x < -threshold)
//         return expf(x); // too small
//     return logf(expf(x) + 1);
// }
// static inline float plse_activate(float x)
// {
//     if (x < -4)
//         return .01f * (x + 4);
//     if (x > 4)
//         return .01f * (x - 4) + 1;
//     return .125f * x + .5f;
// }

// static inline float lhtan_activate(float x)
// {
//     if (x < 0)
//         return .001f * x;
//     if (x > 1)
//         return .001f * (x - 1) + 1;
//     return x;
// }
// static inline float lhtan_gradient(float x)
// {
//     if (x > 0 && x < 1)
//         return 1;
//     return .001f;
// }

// static inline float hardtan_gradient(float x)
// {
//     if (x > -1 && x < 1)
//         return 1;
//     return 0;
// }
// static inline float linear_gradient(float x) { return 1; }
// static inline float logistic_gradient(float x) { return (1 - x) * x; }
// static inline float loggy_gradient(float x)
// {
//     float y = (x + 1.f) / 2.f;
//     return 2 * (1 - y) * y;
// }
// static inline float stair_gradient(float x)
// {
//     if (floor(x) == x)
//         return 0;
//     return 1.0f;
// }
// static inline float relu_gradient(float x) { return (x > 0); }
// static inline float relu6_gradient(float x) { return (x > 0 && x < 6); }
// static inline float elu_gradient(float x) { return (x >= 0) + (x < 0) * (x + 1); }
// static inline float selu_gradient(float x) { return (x >= 0) * 1.0507f + (x < 0) * (x + 1.0507f * 1.6732f); }
// static inline float relie_gradient(float x) { return (x > 0) ? 1 : .01f; }
// static inline float ramp_gradient(float x) { return (x > 0) + .1f; }
// static inline float leaky_gradient(float x) { return (x > 0) ? 1 : .1f; }
// static inline float tanh_gradient(float x) { return 1 - x * x; }

// static inline float sech(float x) { return 2 / (expf(x) + expf(-x)); }
// static inline float gelu_gradient(float x)
// {
//     const float x3 = powf(x, 3);
//     return 0.5 * tanhf(0.0356774 * x3 + 0.797885 * x) + (0.0535161 * x3 + 0.398942 * x) * powf(sech(0.0356774 * x3 + 0.797885 * x), 2) + 0.5;
// }
// static inline float plse_gradient(float x) { return (x < 0 || x > 1) ? .01f : .125f; }