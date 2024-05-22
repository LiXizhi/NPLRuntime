// NPLOCE 也是使用stb库, 造成重复函数定义, 且版本不一致.
#if defined(__EMSCRIPTEN__) || defined(DNPLRUNTIME_OCE)
#include "stb_image.h"
#include "stb_image_write.h"
#else
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif
#endif

#include "Image.h"
#include "utils.h"

Image::Image(int width, int height, int channel)
{
    m_width   = width;
    m_height  = height;
    m_channel = channel;
    m_data.resize(width * height * channel);
}

bool Image::Load(const std::string &filepath, const int target_channel)
{
    int width = 0, height = 0, channel = 0;
    unsigned char *pixels = stbi_load(filepath.c_str(), &width, &height, &channel, target_channel);

    if (pixels == nullptr)
    {
        std::cerr << "Cannot load image: " << filepath << std::endl;
        std::cerr << "STB Reason: " << stbi_failure_reason() << std::endl;
        return false;
    }

    if (target_channel != 0) channel = target_channel;

    m_width   = width;
    m_height  = height;
    m_channel = channel;
    m_data.resize(width * height * channel);
    float *data = m_data.data();
    for (int k = 0; k < channel; ++k)
    {
        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                int dst_index   = i + width * j + width * height * k;
                int src_index   = k + channel * i + channel * width * j;
                data[dst_index] = (float)(pixels[src_index] / 255.0f);
            }
        }
    }

    stbi_image_free(pixels);
    return true;
}

bool Image::Load(const unsigned char *buffer, const int size, const int target_channel)
{
    int width = 0, height = 0, channel = 0;
    unsigned char *pixels = stbi_load_from_memory(buffer, size, &width, &height, &channel, target_channel);

    if (pixels == nullptr)
    {
        std::cerr << "STB Reason: " << stbi_failure_reason() << std::endl;
        return false;
    }

    if (target_channel != 0) channel = target_channel;

    m_width   = width;
    m_height  = height;
    m_channel = channel;
    m_data.resize(width * height * channel);
    float *data = m_data.data();
    for (int k = 0; k < channel; ++k)
    {
        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < width; ++i)
            {
                int dst_index   = i + width * j + width * height * k;
                int src_index   = k + channel * i + channel * width * j;
                data[dst_index] = (float)(pixels[src_index] / 255.0f);
            }
        }
    }

    stbi_image_free(pixels);
    return true;
}

bool Image::Save(const std::string &filepath)
{
    std::vector<unsigned char> pixels;
    pixels.resize(m_width * m_height * m_channel);

    const int stride = m_width * m_height;
    for (int y = 0; y < m_height; y++)
    {
        for (int x = 0; x < m_width; x++)
        {
            int src_index = y * m_width + x;
            int dst_index = src_index * m_channel;
            for (int c = 0; c < m_channel; c++)
            {
                pixels[dst_index + c] = m_data[src_index + c * stride] * 255;
            }
        }
    }

    std::string ext_name = std::filesystem::path(filepath).extension().string();
    if (ext_name == ".jpg" || ext_name == ".jpeg")
    {
        return 0 != stbi_write_jpg(filepath.c_str(), m_width, m_height, m_channel, pixels.data(), 100);
    }
    else if (ext_name == ".bmp")
    {
        return 0 != stbi_write_bmp(filepath.c_str(), m_width, m_height, m_channel, pixels.data());
    }
    else // if (ext_name == ".png")
    {
        return 0 != stbi_write_png(filepath.c_str(), m_width, m_height, m_channel, pixels.data(), m_width * m_channel);
    }
}

float Image::GetPixel(int x, int y, int c)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height || c < 0 || c >= m_channel) return 0.f;
    return m_data[x + y * m_width + c * m_width * m_height];
}

void Image::SetPixel(int x, int y, int c, float pixel)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height || c < 0 || c >= m_channel) return;
    m_data[x + y * m_width + c * m_width * m_height] = pixel;
}

void Image::AddPixel(int x, int y, int c, float pixel)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height || c < 0 || c >= m_channel) return;
    m_data[x + y * m_width + c * m_width * m_height] += pixel;
}

void Image::RGBToHSV()
{
    assert(m_channel == 3);
    for (int j = 0; j < m_height; ++j)
    {
        for (int i = 0; i < m_width; ++i)
        {
            float r     = GetPixel(i, j, 0);
            float g     = GetPixel(i, j, 1);
            float b     = GetPixel(i, j, 2);
            float max   = three_way_max(r, g, b);
            float min   = three_way_min(r, g, b);
            float delta = max - min;
            float h     = 0;
            float s     = 0;
            float v     = max;
            if (max == 0)
            {
                s = 0;
                h = 0;
            }
            else
            {
                s = delta / max;
                if (r == max)
                {
                    h = (g - b) / delta;
                }
                else if (g == max)
                {
                    h = 2 + (b - r) / delta;
                }
                else
                {
                    h = 4 + (r - g) / delta;
                }
                if (h < 0) h += 6;
                h = h / 6.;
            }
            SetPixel(i, j, 0, h);
            SetPixel(i, j, 1, s);
            SetPixel(i, j, 2, v);
        }
    }
}

void Image::HSVToRGB()
{
    assert(m_channel == 3);
    for (int j = 0; j < m_height; ++j)
    {
        for (int i = 0; i < m_width; ++i)
        {
            float h = 6 * GetPixel(i, j, 0);
            float s = GetPixel(i, j, 1);
            float v = GetPixel(i, j, 2);
            float r = 0.f;
            float g = 0.f;
            float b = 0.f;
            if (s == 0)
            {
                r = g = b = v;
            }
            else
            {
                int index = std::floor(h);
                float f   = h - index;
                float p   = v * (1 - s);
                float q   = v * (1 - s * f);
                float t   = v * (1 - s * (1 - f));
                if (index == 0)
                {
                    r = v;
                    g = t;
                    b = p;
                }
                else if (index == 1)
                {
                    r = q;
                    g = v;
                    b = p;
                }
                else if (index == 2)
                {
                    r = p;
                    g = v;
                    b = t;
                }
                else if (index == 3)
                {
                    r = p;
                    g = q;
                    b = v;
                }
                else if (index == 4)
                {
                    r = t;
                    g = p;
                    b = v;
                }
                else
                {
                    r = v;
                    g = p;
                    b = q;
                }
            }
            SetPixel(i, j, 0, r);
            SetPixel(i, j, 1, g);
            SetPixel(i, j, 2, b);
        }
    }
}

void Image::ScaleChannel(int c, float v)
{
    auto datas = m_data.data();
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            auto index   = x + y * m_width + c * m_width * m_height;
            datas[index] = datas[index] * v;
        }
    }
}

void Image::Constrain()
{
    int size   = m_width * m_height * m_channel;
    auto datas = m_data.data();
    for (int i = 0; i < size; ++i)
    {
        if (datas[i] < 0) datas[i] = 0;
        if (datas[i] > 1) datas[i] = 1;
    }
}

void Image::Distort(float hue, float saturation, float exposure)
{
    RGBToHSV();
    ScaleChannel(1, saturation);
    ScaleChannel(2, exposure);
    int size   = m_width * m_height;
    auto datas = m_data.data();
    for (int i = 0; i < size; ++i)
    {
        m_data[i] = m_data[i] + hue;
        if (datas[i] > 1) datas[i] -= 1;
        if (datas[i] < 0) datas[i] += 1;
    }
    HSVToRGB();
    Constrain();
}

void Image::RandomDistort(float hue, float saturation, float exposure)
{
    float dhue = random_float(-hue, hue);
    float dsat = random_scale(saturation);
    float dexp = random_scale(exposure);
    Distort(dhue, dsat, dexp);
}

void Image::FlipHorizontal()
{
    auto datas = m_data.data();
    for (int c = 0; c < m_channel; ++c)
    {
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width / 2; ++x)
            {
                int index    = c * m_width * m_height + y * m_width + x;
                int flip     = c * m_width * m_height + y * m_width + m_width - x - 1;
                float swap   = datas[flip];
                datas[flip]  = datas[index];
                datas[index] = swap;
            }
        }
    }
}

void Image::FlipVertical()
{
    auto datas = m_data.data();
    for (int c = 0; c < m_channel; ++c)
    {
        for (int y = 0; y < m_height / 2; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                int index    = c * m_width * m_height + y * m_width + x;
                int flip     = c * m_width * m_height + (m_height - 1 - y) * m_width + x;
                float swap   = datas[flip];
                datas[flip]  = datas[index];
                datas[index] = swap;
            }
        }
    }
}

void Image::DrawRectangle(int x, int y, int width, int height, float r, float g, float b, float a)
{
    int stride    = m_width * m_height;
    float rgba[4] = {r, g, b, a};
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int ix = i + x;
            int iy = j + y;
            if (ix < 0 || ix >= m_width || iy < 0 || iy >= m_height) continue;
            if (i != 0 && i != (width - 1) && j != 0 && j != (height - 1)) continue;
            int index = iy * m_width + ix;
            for (int k = 0; k < m_channel; k++)
            {
                m_data[index + k * stride] = rgba[k];
            }
        }
    }
}

void Image::FillRectangle(int x, int y, int width, int height, float r, float g, float b, float a)
{
    int stride    = m_width * m_height;
    float rgba[4] = {r, g, b, a};
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            int ix = i + x;
            int iy = j + y;
            if (ix < 0 || ix >= m_width || iy < 0 || iy >= m_height) continue;
            int index = iy * m_width + ix;
            for (int k = 0; k < m_channel; k++)
            {
                m_data[index + k * stride] = rgba[k];
            }
        }
    }
}

std::shared_ptr<Image> Image::Clone()
{
    auto clone = std::make_shared<Image>(m_width, m_height, m_channel);
    memcpy(clone->GetData(), GetData(), m_width * m_height * m_channel * sizeof(float));
    return clone;
}

std::shared_ptr<Image> Image::Scale(float scale_x, float scale_y)
{
    auto src_width  = m_width;
    auto src_height = m_height;
    auto src_data   = GetData();

    int dst_width  = src_width * scale_x;
    int dst_height = src_height * scale_y;
    auto image     = std::make_shared<Image>(dst_width, dst_height, m_channel);
    auto dst_data  = image->GetData();

    if (src_width == dst_width && src_height == dst_height)
    {
        memcpy(dst_data, src_data, dst_width * dst_height * m_channel * sizeof(float));
        return image;
    }

    float d_scale_x = 1.0f / scale_x;
    float d_scale_y = 1.0f / scale_y;
    for (int k = 0; k < m_channel; k++)
    {
        int src_offset = k * src_width * src_height;
        int dst_offset = k * dst_width * dst_height;
        for (int j = 0; j < dst_height; j++)
        {
            for (int i = 0; i < dst_width; i++)
            {
                int dst_x           = i;
                int dst_y           = j;
                int src_x           = i * d_scale_x;
                int src_y           = j * d_scale_y;
                int src_index       = src_offset + src_y * src_width + src_x;
                int dst_index       = dst_offset + dst_y * dst_width + dst_x;
                dst_data[dst_index] = src_data[src_index];
            }
        }
    }

    return image;
}

std::shared_ptr<Image> Image::Fill(int width, int height, int default_value)
{
    auto src_width  = m_width;
    auto src_height = m_height;
    auto src_data   = GetData();

    int dst_width  = width;
    int dst_height = height;
    auto image     = std::make_shared<Image>(dst_width, dst_height, m_channel);
    auto dst_data  = image->GetData();

    if (src_width == dst_width && src_height == dst_height)
    {
        memcpy(dst_data, src_data, dst_width * dst_height * m_channel * sizeof(float));
        return image;
    }

    int offset_x = (dst_width - src_width) / 2;
    int offset_y = (dst_height - src_height) / 2;
    for (int k = 0; k < m_channel; k++)
    {
        int src_offset = k * src_width * src_height;
        int dst_offset = k * dst_width * dst_height;
        for (int j = 0; j < dst_height; j++)
        {
            for (int i = 0; i < dst_width; i++)
            {
                int dst_x     = i;
                int dst_y     = j;
                int src_x     = i - offset_x;
                int src_y     = j - offset_y;
                int src_index = src_offset + src_y * src_width + src_x;
                int dst_index = dst_offset + dst_y * dst_width + dst_x;
                if (src_x < 0 || src_x >= src_width || src_y < 0 || src_y >= src_height)
                {
                    if (default_value == -1)
                    {
                        int src_index       = src_offset + constrain(src_y, 0, src_height - 1) * src_width + constrain(src_x, 0, src_width - 1);
                        dst_data[dst_index] = src_data[src_index];
                    }
                    else
                    {
                        dst_data[dst_index] = default_value;
                    }
                }
                else
                {
                    dst_data[dst_index] = src_data[src_index];
                }
            }
        }
    }

    return image;
}

std::shared_ptr<Image> Image::ScaleFill(int width, int height)
{
    // 缩放至最小宽高 不变形图像
    float scale_x  = width * 1.0f / GetWidth();
    float scale_y  = height * 1.0f / GetHeight();
    float scale    = scale_x > scale_y ? scale_y : scale_x;
    auto scale_img = Scale(scale, scale);

    // 填充到最大宽高 不变形图像
    float fill_scale_x = width * 1.0f / scale_img->GetWidth();
    float fill_scale_y = height * 1.0f / scale_img->GetHeight();
    auto fill_img      = scale_img->Fill(width, height);
    return fill_img;
}

std::shared_ptr<Image> Image::GetSubImage(int x, int y, int width, int height)
{
    auto src_width  = m_width;
    auto src_height = m_height;
    auto src_data   = GetData();

    auto dst_width  = width;
    auto dst_height = height;
    auto image      = std::make_shared<Image>(dst_width, dst_height, m_channel);
    auto dst_data   = image->GetData();

    if (src_width == dst_width && src_height == dst_height)
    {
        memcpy(dst_data, src_data, dst_width * dst_height * m_channel * sizeof(float));
        return image;
    }

    for (int k = 0; k < m_channel; k++)
    {
        int src_offset = k * src_width * src_height;
        int dst_offset = k * dst_width * dst_height;
        for (int j = 0; j < dst_height; j++)
        {
            for (int i = 0; i < dst_width; i++)
            {
                int dst_x           = i;
                int dst_y           = j;
                int src_x           = constrain(x + i, 0, src_width - 1);
                int src_y           = constrain(y + j, 0, src_height - 1);
                int src_index       = src_offset + src_y * src_width + src_x;
                int dst_index       = dst_offset + dst_y * dst_width + dst_x;
                dst_data[dst_index] = src_data[src_index];
            }
        }
    }

    return image;
}

void Image::Test()
{
    std::string in_path = "d:/workspace/cpp/cpp_darknet/data/yolo/images/0001.jpg";
    auto in_img         = std::make_shared<Image>();
    in_img->Load(in_path);

    float in_truths[] = {0.406583, 0.493744, 0.281532, 0.319069};

    int net_width  = 96;
    int net_height = 96;
    int img_width  = in_img->GetWidth();
    int img_height = in_img->GetHeight();

    int in_x = in_truths[0] * img_width;
    int in_y = in_truths[1] * img_height;
    int in_w = in_truths[2] * img_width;
    int in_h = in_truths[3] * img_height;

    float scale_x  = net_width * 1.0f / img_width;
    float scale_y  = net_height * 1.0f / img_height;
    float scale    = scale_x > scale_y ? scale_y : scale_x;
    auto scale_img = in_img->Scale(scale, scale);

    int offset_x = (net_width - scale * img_width);
    int offset_y = (net_height - scale * img_height);
    auto out_img = scale_img->Fill(net_width, net_height);

    int out_x = in_x * scale + offset_x * 0.5f;
    int out_y = in_y * scale + offset_y * 0.5f;
    int out_w = in_w * scale;
    int out_h = in_h * scale;

    float out_x_scale = out_x * 1.0f / net_width;
    float out_y_scale = out_y * 1.0f / net_height;
    float out_w_scale = out_w * 1.0f / net_width;
    float out_h_scale = out_h * 1.0f / net_height;

    std::cout << out_x_scale << "," << out_y_scale << "," << out_w_scale << "," << out_h_scale << std::endl;

    int in_out_x = (out_x - offset_x * 0.5f) / scale;
    int in_out_y = (out_y - offset_y * 0.5f) / scale;
    int in_out_w = out_w / scale;
    int in_out_h = out_h / scale;

    auto out_path = std::filesystem::path(in_path);
    out_img->FillRectangle((out_x - out_w / 2), (out_y - out_h / 2), out_w, out_h, 1, 0, 0);
    out_img->Save((out_path.parent_path().parent_path() / "tmps" / out_path.filename()).string());
}
