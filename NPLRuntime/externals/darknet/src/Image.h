#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

class Image
{
public:
    Image(int width = 0, int height = 0, int channel = 0);

    bool Load(const std::string &filepath, const int target_channel = 3);
    bool Load(const unsigned char* buffer, const int size, const int target_channel = 3);
    bool Save(const std::string &filepath);

    float GetPixel(int x, int y, int c);
    void SetPixel(int x, int y, int c, float pixel);
    void AddPixel(int x, int y, int c, float pixel);

    void RGBToHSV();
    void HSVToRGB();
    void ScaleChannel(int c, float v);
    void Constrain();
    void Distort(float hue, float saturation, float exposure);
    void RandomDistort(float hue = 0.1f, float saturation = 1.5f, float exposure = 1.5f);
    void FlipHorizontal();
    void FlipVertical();

    void DrawRectangle(int x, int y, int width, int height, float r, float g, float b, float a = 1.0f);
    void FillRectangle(int x, int y, int width, int height, float r, float g, float b, float a = 1.0f);

    std::shared_ptr<Image> Clone();
    std::shared_ptr<Image> Scale(float scale_x, float scale_y);
    std::shared_ptr<Image> Fill(float scale_x, float scale_y) { return Fill((int)(scale_x * GetWidth()), (int)(scale_y * GetHeight()), 0); }
    std::shared_ptr<Image> Fill(int width, int height, int default_value = 0);
    std::shared_ptr<Image> ScaleFill(int width, int height);
    std::shared_ptr<Image> GetSubImage(int x, int y, int width, int height);

public:
    static void Test();

public:
    inline float *GetData() { return m_data.data(); }
    inline int GetWidth() { return m_width; }
    inline int GetHeight() { return m_height; }
    inline int GetChannel() { return m_channel; }
    inline int SetWidth(int width) { m_width = width; }
    inline int SetHeight(int height) { m_height = height; }
    inline int SetChannel(int channel) { m_channel = channel; }

protected:
    static inline float three_way_max(float a, float b, float c) { return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c); }
    static inline float three_way_min(float a, float b, float c) { return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c); }
    static inline int constrain(int v, int min, int max) { return v < min ? min : (v > max ? max : v); }
    static inline float constrain(float v, float min, float max) { return v < min ? min : (v > max ? max : v); }

protected:
    int m_width;
    int m_height;
    int m_channel;
    std::vector<float> m_data;
};

#endif