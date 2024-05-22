#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "utils.h"

long long get_timestamp()
{
    auto now       = std::chrono::high_resolution_clock::now();
    auto timestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    return timestamp;
}

int random_int()
{
    return std::rand();
}

int random_int(int min_value, int max_value, int random_value)
{
    if (min_value > max_value)
    {
        auto tmp_value = min_value;
        min_value      = max_value;
        max_value      = tmp_value;
    }
    return min_value + random_value % (max_value - min_value + 1);
}

int random_int(int min_value, int max_value)
{
    return random_int(min_value, max_value, random_int());
}

float random_float()
{
    return (float)std::rand() / (float)RAND_MAX;
}

float random_float(float min_value, float max_value, float random_value)
{
    if (min_value > max_value)
    {
        auto tmp_value = max_value;
        max_value      = min_value;
        min_value      = tmp_value;
    }

    return min_value + random_value * (max_value - min_value);
}

float random_float(float min_value, float max_value)
{
    return random_float(min_value, max_value, random_float());
}

float random_scale(float s)
{
    float scale = random_float(1.0f, s);
    if (random_int() % 2 == 0) return scale;
    return 1.0f / scale;
}

float sum_array(float *a, int n)
{
    float sum = 0.f;
    for (int i = 0; i < n; i++)
    {
        sum += a[i];
    }
    return sum;
}

float mag_array(float *a, int n)
{
    float sum = 0;
    for (int i = 0; i < n; ++i)
    {
        sum += a[i] * a[i];
    }
    return std::sqrt(sum);
}

// std::string to_binary_string(void *data, int size)
// {
//     unsigned char *p = (unsigned char *)data;
//     std::string str;

//     str.resize(size * 2);
//     char *s = str.data();

//     while (size > 0)
//     {
//         unsigned char c = *p++;
//         *s++            = (c & 0xf) + 'A';
//         *s++            = (c >> 4) + 'A';
//     }

//     return str;
// }

// void *from_binary_string(std::string str)
// {
//     static std::vector<unsigned char> s_buffer;
//     s_buffer.resize(str.size() / 2);

//     auto p    = s_buffer.data();
//     auto s    = str.data();
//     auto size = str.size();

//     while (size > 0)
//     {
//         unsigned char c1 = *s++;
//         unsigned char c2 = *s++;
//         *p++             = (c1 - 'A') | ((c2 - 'A') << 4);
//     }

//     return s_buffer.data();
// }


