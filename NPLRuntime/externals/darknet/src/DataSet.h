#ifndef __DATASET_H__
#define __DATASET_H__
#include <deque>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "DataItem.h"
#include "Image.h"
#include "Truth.h"

class Net;

class DataSet
{
public:
    class BatchInput
    {
    public:
        BatchInput(int datas_size, int truths_size)
        {
            m_datas.resize(datas_size);
            m_truths.resize(truths_size);
        }

        inline float *GetTruths() { return m_truths.data(); }
        inline float *GetDatas() { return m_datas.data(); }
        inline DataItem *GetDataItem(int index) { return index < 0 || index > m_data_items.size() ? nullptr : m_data_items[index].get(); }
        inline std::vector<std::shared_ptr<DataItem>> *GetDataItems() { return &m_data_items; }

    public:
        std::shared_ptr<std::vector<std::ostream *>> GetInOSS()
        {
            auto oss = std::make_shared<std::vector<std::ostream *>>();
            int size = m_data_items.size();
            for (int i = 0; i < size; i++)
            {
                oss->push_back(m_data_items[i]->GetOStream());
            }
            return oss;
        }

    protected:
        std::vector<float> m_datas;
        std::vector<float> m_truths;
        std::vector<std::shared_ptr<DataItem>> m_data_items;
    };

public:
    DataSet();

    void Load(std::istream &is);
    std::shared_ptr<BatchInput> GetNextBatchInput(int batch);

public:
    inline std::vector<std::shared_ptr<DataItem>> *GetDataItems() { return &m_data_items; }
    inline int GetDatasSize() { return m_width * m_height * m_channels; }
    inline void SetTruthCreator(std::function<std::shared_ptr<Truth>()> truth_creator) { m_truth_creator = truth_creator; }
    inline void SetDataItemCreator(std::function<std::shared_ptr<DataItem>()> dataitem_creator) { m_dataitem_creator = dataitem_creator; }

    inline void SetWidth(int width) { m_width = width; }
    inline int GetWidth() { return m_width; }
    inline void SetHeight(int height) { m_height = height; }
    inline int GetHeight() { return m_height; }
    inline void SetChannels(int channels) { m_channels = channels; }
    inline int GetChannels() { return m_channels; }
    inline void SetMaxTruthsSize(int max_truths_size) { m_max_truths_size = max_truths_size; }
    inline int GetMaxTruthsSize() { return m_max_truths_size; }

    inline void SetDebug(bool debug) { m_debug = debug; }
    inline bool IsDebug() { return m_debug; }
    inline void SetDebugsPath(const std::string &debugs_path) { m_debugs_path = debugs_path; }
    inline const std::string &GetDebugsPath() { return m_debugs_path; }
    inline void SetInputIndex(int data_index) { m_input_index = data_index; }
    inline int GetInputIndex() { return m_input_index; }

    inline void SetRandom(bool random) { m_random = random; }
    inline bool IsRandom() { return m_random; }
    inline void SetAugment(bool augment) { m_augment = augment; }
    inline bool IsAugment() { return m_augment; }

    inline void SetJitter(float jitter) { m_jitter = jitter; }
    inline float GetJitter() { return m_jitter; }
    inline void SetFilpHorizontal(bool filp_horizontal) { m_filp_horizontal = filp_horizontal; }
    inline bool IsFilpHorizontal() { return m_filp_horizontal; }
    inline void SetFilpVertical(bool filp_vertical) { m_filp_vertical = filp_vertical; }
    inline bool IsFilpVertical() { return m_filp_vertical; }
    inline void SetDistort(bool distort) { m_distort = distort; }
    inline bool IsDistort() { return m_distort; }
    inline void SetHue(float hue) { m_hue = hue; }
    inline float GetHue() { return m_hue; }
    inline void SetSaturation(float saturation) { m_saturation = saturation; }
    inline float GetSaturation() { return m_saturation; }
    inline void SetExposure(float exposure) { m_exposure = exposure; }
    inline float GetExposure() { return m_exposure; }

public:
    static float GetNetImageScale(int img_width, int img_height, int net_width, int net_height, int *offset_x = nullptr, int *offset_y = nullptr);
    static void ImageBoxToNetBox(float *img_box, float *net_box, int img_width, int img_height, int net_width, int net_height);
    static void NetBoxToImageBox(float *net_box, float *img_box, int img_width, int img_height, int net_width, int net_height);
    static std::shared_ptr<Image> LoadNetImage(const std::string &in_img_path, int in_net_width, int in_net_height, int *out_img_width = nullptr, int *out_img_height = nullptr);
    static std::shared_ptr<Image> LoadNetImage(const unsigned char *in_datas, const int in_size, int in_net_width, int in_net_height, int *out_img_width = nullptr, int *out_img_height = nullptr);
    static void LoadLines(std::istream &in_is, std::vector<std::string> *out_lines, std::function<void(std::string &)> line_handler = nullptr);

protected:
    std::shared_ptr<DataItem> LoadDataItem(int data_item_index);
    std::shared_ptr<DataItem> AugmentImageDataItem(std::shared_ptr<DataItem> in_data_item);

protected:
    int m_width;               // 数据宽
    int m_height;              // 数据高
    int m_channels;            // 数据通道
    int m_max_truths_size;     // 最大真实值大小
    int m_input_index;         // 输入索引
    bool m_debug;              // 是否调试
    std::string m_debugs_path; // 调试路径

    std::function<std::shared_ptr<Truth>()> m_truth_creator;
    std::function<std::shared_ptr<DataItem>()> m_dataitem_creator;
    std::vector<std::shared_ptr<DataItem>> m_data_items;
    std::deque<std::shared_ptr<DataItem>> m_cache_data_items;

    bool m_random;          // 是否随机
    bool m_augment;         // 是否增强
    float m_jitter;         // 抖动
    bool m_filp_horizontal; // 是否水平翻转
    bool m_filp_vertical;   // 是否垂直翻转
    bool m_distort;         // 是否变形图像
    float m_saturation;     // 饱和度
    float m_exposure;       // 曝光
    float m_hue;            // 色度
};
#endif