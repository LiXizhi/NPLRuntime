#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "utils.h"
namespace darknet
{
    // 字符编码转换使用
    template <class Facet>
    class usable_facet : public Facet
    {
    public:
        template <class... Args>
        usable_facet(Args &&...args) : Facet(std::forward<Args>(args)...) {}
        ~usable_facet() {}
    };
    typedef usable_facet<std::codecvt_byname<wchar_t, char, std::mbstate_t>> codecvt_wstring;

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

    std::string string_trim(const std::string &text)
    {
        // 查找第一个非空白字符的位置
        size_t start = text.find_first_not_of(" \t\n\r");
        // 如果字符串全为空白字符，则返回空字符串
        if (start == std::string::npos) return "";
        // 查找最后一个非空白字符的位置
        size_t end = text.find_last_not_of(" \t\n\r");
        // 提取非空白字符部分
        return text.substr(start, end - start + 1);
    }

    std::wstring Utf8ToWString(const std::string &text)
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> codecvt;
        return codecvt.from_bytes(text);
    }

    // WString 转 utf8
    std::string WStringToUtf8(const std::wstring &text)
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> codecvt;
        return codecvt.to_bytes(text);
    }

    std::string WStringToString(const std::wstring &text, const std::string &locale)
    {
        static std::wstring_convert<codecvt_wstring> codecvt(new codecvt_wstring(locale));
        return codecvt.to_bytes(text);
    }

    // 转本地字符
    std::wstring StringToWString(const std::string &text, const std::string &locale)
    {
        static std::wstring_convert<codecvt_wstring> codecvt(new codecvt_wstring(locale));
        return codecvt.from_bytes(text);
    }

    // 本地字符转utf8
    std::string StringToUtf8(const std::string &text, const std::string &locale)
    {
        return WStringToUtf8(StringToWString(text, locale));
    }

    // 本地字符转utf8
    std::string Utf8ToString(const std::string &text, const std::string &locale)
    {
        return WStringToString(Utf8ToWString(text));
    }
}