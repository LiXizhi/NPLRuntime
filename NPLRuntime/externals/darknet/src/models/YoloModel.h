#ifndef __DARKNET_YOLO_MODEL_H__
#define __DARKNET_YOLO_MODEL_H__
#include "Model.h"
#include "layers/Layer.h"
namespace darknet
{
    class YoloModel : public Model
    {
    public:
        class Truth
        {
        public:
            Truth() : m_probability(0.f) { m_datas.resize(5); }
            inline void SetX(float x) { m_datas[0] = x; }
            inline float GetX() { return m_datas[0]; }
            inline void SetY(float y) { m_datas[1] = y; }
            inline float GetY() { return m_datas[1]; }
            inline void SetWidth(float width) { m_datas[2] = width; }
            inline float GetWidth() { return m_datas[2]; }
            inline void SetHeight(float height) { m_datas[3] = height; }
            inline float GetHeight() { return m_datas[3]; }
            inline void SetClassifyIndex(int classify) { m_datas[4] = classify; }
            inline int GetClassifyIndex() { return m_datas[4]; }
            inline void SetProbability(float probability) { m_probability = probability; }
            inline float GetProbability() { return m_probability; }
            inline void SetClassify(const std::string &classify) { m_classify = classify; }
            inline const std::string &GetClassify() { return m_classify; }

        protected:
            std::vector<float> m_datas;
            std::string m_classify;
            float m_probability;
        };

        using ModelDataItem = Model::DataItem;
        class YoloModelDataItem : public ModelDataItem
        {
        public:
            YoloModelDataItem(const std::string &path) : ModelDataItem(this), m_path(path) {}

            inline const std::string &GetPath() { return m_path; }
            inline std::vector<Truth> *GetTruths() { return &m_truths; }

        protected:
            std::string m_path;
            std::vector<Truth> m_truths;
        };

    public:
        YoloModel();

        // 加载模型
        virtual bool Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list);
        // 加载模型
        virtual bool LoadModelFromFile(std::istream &is);
        // 保存模型
        virtual bool SaveModelToFile(std::ostream &os);
        // 训练
        virtual void Train();
        // 测试
        virtual void Test();
        // 加载数据集
        virtual void LoadDataSet();
        // 加载下一批输入
        virtual bool LoadNextBatchInput();

    protected:
        inline std::unordered_map<std::string, std::shared_ptr<YoloModelDataItem>> *GetDataSet() { return &m_dataset; }
        inline void SetInputIndex(int data_index) { m_input_index = data_index; }
        inline int GetInputIndex() { return m_input_index; }

    protected:
        void LoadInputs(Options *options);
        void LoadInputs(std::istream &is);
        void LoadInputs(const std::string &text);
        bool LoadInput(const std::string &img_path, std::vector<Truth> truths, float *out_datas, int out_datas_size, float *out_truths, int out_truths_size);

    protected:
        // 训练数据
        int m_input_index;                                                             // 输入索引
        int m_cache_size;                                                              // 输入缓存大小
        std::unordered_map<std::string, std::weak_ptr<DataImage>> m_input_images;      // 输入图像
        std::deque<std::shared_ptr<DataImage>> m_cache_images;                         // 缓存图像
        std::unordered_map<std::string, std::shared_ptr<YoloModelDataItem>> m_dataset; // 输入集 image_path => YoloModelDataItem

    protected:
        std::unordered_map<std::string, int> m_classify_index_map;
        std::unordered_map<int, std::string> m_index_classify_map;

    public:
        int GetClassifySize()
        {
            return m_classify_index_map.size();
        }

        const std::string &GetIndexClassify(int index)
        {
            return m_index_classify_map[index];
        }

        int GetClassifyIndex(const std::string &classify)
        {
            auto it = m_classify_index_map.find(classify);
            return it == m_classify_index_map.end() ? -1 : it->second;
        }

        int CreateGetClassifyIndex(const std::string &classify)
        {
            auto it = m_classify_index_map.find(classify);
            if (it != m_classify_index_map.end()) return it->second;
            auto index = (int)(m_classify_index_map.size());
            m_classify_index_map.insert_or_assign(classify, index);
            m_index_classify_map.insert_or_assign(index, classify);
            return index;
        }

        void ClearClassifyIndex()
        {
            m_classify_index_map.clear();
            m_index_classify_map.clear();
        }

        void SaveClassifyIndex(std::ostream &os)
        {
            int size = m_classify_index_map.size();
            os.write((const char *)(&size), sizeof(size));
            for (auto it = m_classify_index_map.begin(); it != m_classify_index_map.end(); it++)
            {
                auto key     = it->first;
                auto value   = it->second;
                int key_size = key.size();
                os.write((const char *)(&key_size), sizeof(key_size));
                os.write(key.c_str(), key_size);
                os.write((const char *)(&value), sizeof(value));
            }
        }

        void LoadClassifyIndex(std::istream &is)
        {
            int size = 0;
            is.read((char *)(&size), sizeof(size));
            for (int i = 0; i < size; i++)
            {
                int key_size = 0;
                std::string key;
                is.read((char *)(&key_size), sizeof(key_size));
                key.resize(key_size);
                is.read((char *)(key.c_str()), key_size);

                int value = 0;
                is.read((char *)(&value), sizeof(value));

                m_classify_index_map[key]   = value;
                m_index_classify_map[value] = key;
            }
        }

    public:
        static float GetNetImageScale(int img_width, int img_height, int net_width, int net_height, int *offset_x, int *offset_y)
        {
            float scale_x = net_width * 1.0f / img_width;
            float scale_y = net_height * 1.0f / img_height;
            float scale   = scale_x > scale_y ? scale_y : scale_x;

            if (offset_x != nullptr) *offset_x = (net_width - scale * img_width);
            if (offset_y != nullptr) *offset_y = (net_height - scale * img_height);

            return scale;
        }

        static void ImageBoxToNetBox(float *img_box, float *net_box, int img_width, int img_height, int net_width, int net_height)
        {
            int offset_x = 0;
            int offset_y = 0;
            float scale  = GetNetImageScale(img_width, img_height, net_width, net_height, &offset_x, &offset_y);
            net_box[0]   = (img_box[0] * img_width * scale + offset_x * 0.5f) / net_width;
            net_box[1]   = (img_box[1] * img_height * scale + offset_y * 0.5f) / net_height;
            net_box[2]   = (img_box[2] * img_width * scale) / net_width;
            net_box[3]   = (img_box[3] * img_height * scale) / net_height;
        }

        static void NetBoxToImageBox(float *net_box, float *img_box, int img_width, int img_height, int net_width, int net_height)
        {
            int offset_x = 0;
            int offset_y = 0;
            float scale  = GetNetImageScale(img_width, img_height, net_width, net_height, &offset_x, &offset_y);
            img_box[0]   = (net_box[0] * net_width - offset_x * 0.5f) / scale / img_width;
            img_box[1]   = (net_box[1] * net_height - offset_y * 0.5f) / scale / img_height;
            img_box[2]   = net_box[2] * net_width / scale / img_width;
            img_box[3]   = net_box[3] * net_height / scale / img_height;
        }
    };
}
#endif