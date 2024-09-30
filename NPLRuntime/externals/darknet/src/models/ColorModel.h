#ifndef __DARKNET_COLOR_MODEL_H__
#define __DARKNET_COLOR_MODEL_H__
#include <stdint.h>

#include "SoftMaxModel.h"
namespace darknet
{
    class ColorModel : public SoftMaxModel
    {
    public:
        ColorModel();
        virtual bool Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list);
        virtual void LoadNet();
        virtual void LoadDataSet();
        virtual void SaveDataSet();
        virtual bool LoadNextBatchInput();

    public:
        bool LoadModelFromFile(const std::string &model_path) { return SoftMaxModel::LoadModelFromFile(model_path); }
        bool SaveModelToFile(const std::string &model_path) { return SoftMaxModel::SaveModelToFile(model_path); }
        int Predict(unsigned char r, unsigned char g, unsigned char b);

    public:
        inline std::unordered_map<int, int> *GetDataSet() { return &m_dataset; }
        inline std::unordered_map<int, int> *GetRGB565DataSet() { return &m_rgb565_dataset; }
        inline bool IsEnableRGB565() { return m_enable_rgb565; }
        inline void SetEnableRGB565(bool enable_rgb565) { m_enable_rgb565 = enable_rgb565; }
        inline int GetBlackClassifyIndex() { return GetClassifyIndex(R"(黑色)"); }
        inline int GetWhiteClassifyIndex() { return GetClassifyIndex(R"(白色)"); }
        inline int GetRedClassifyIndex() { return GetClassifyIndex(R"(红色)"); }
        inline int GetGreenClassifyIndex() { return GetClassifyIndex(R"(绿色)"); }
        inline int GetBlueClassifyIndex() { return GetClassifyIndex(R"(蓝色)"); }
        inline int GetOrangeClassifyIndex() { return GetClassifyIndex(R"(橙色)"); }
        inline int GetYellowClassifyIndex() { return GetClassifyIndex(R"(黄色)"); }
        inline int GetCyanClassifyIndex() { return GetClassifyIndex(R"(青色)"); }
        inline int GetPurpleClassifyIndex() { return GetClassifyIndex(R"(紫色)"); }
        inline std::string GetDataSetColorsPath() { return m_dataset_colors_path.empty() ? (GetDataSetDirectory() + "/dataset.colors") : m_dataset_colors_path; }
        inline void SetDataSetColorsPath(const std::string dataset_colors_path) { m_dataset_colors_path = dataset_colors_path; }
    protected:
        std::unordered_map<int, int> m_dataset;
        std::unordered_map<int, int> m_rgb565_dataset;
        std::string m_dataset_colors_path;
        bool m_enable_rgb565;
        std::vector<int> m_inputs;

    public:
        static int GetColorIndex(unsigned char r, unsigned char g, unsigned char b)
        {
            unsigned char color[4] = {r, g, b, 0};
            return *((int *)color);
        }

        static unsigned short RGB888ToRGB565(unsigned char r, unsigned char g, unsigned char b)
        {
            return (r >> 3) << 11 | (g >> 2) << 5 | (b >> 3);
        }
    };
}
#endif