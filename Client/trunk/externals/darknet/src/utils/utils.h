#ifndef __DARKNET_UTILS_H__
#define __DARKNET_UTILS_H__

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <codecvt>
#include <locale>
#include <string>

namespace darknet
{
    long long get_timestamp();

    int random_int();
    int random_int(int min_value, int max_value, int random_value);
    int random_int(int min_value, int max_value);
    float random_float();
    float random_float(float min_value, float max_value, float random_value);
    float random_float(float min_value, float max_value);
    float random_scale(float s);

    float sum_array(float *a, int n);
    float mag_array(float *a, int n);

    std::string string_trim(const std::string &text);

    std::wstring Utf8ToWString(const std::string &text);
    std::string WStringToUtf8(const std::wstring &text);
    std::string WStringToString(const std::wstring &text, const std::string &locale = "zh-CN");
    std::wstring StringToWString(const std::string &text, const std::string &locale = "zh-CN");
    std::string StringToUtf8(const std::string &text, const std::string &locale = "zh-CN");
    std::string Utf8ToString(const std::string &text, const std::string &locale = "zh-CN");
}

#endif