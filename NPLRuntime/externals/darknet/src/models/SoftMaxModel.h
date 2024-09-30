#ifndef __DARKNET_SOFTMAX_MODEL_H__
#define __DARKNET_SOFTMAX_MODEL_H__

#include "Model.h"

#include "layers/Layer.h"
namespace darknet
{
    class SoftMaxModel : public Model
    {
    public:
        using ModelDataItem = Model::DataItem;
        class SoftMaxModelDataItem : public ModelDataItem
        {
        public:
            template <typename T = void *>
            SoftMaxModelDataItem(const std::string &path, const std::string &classify) : ModelDataItem(this), m_path(path), m_classify(classify) {}

            inline const std::string &GetPath() { return m_path; }
            inline const std::string &GetClassify() { return m_classify; }

        protected:
            std::string m_path;
            std::string m_classify;
        };

    public:
        SoftMaxModel();
        // 加载网络
        virtual void LoadNet() { Model::LoadNet(); }
        // 刷新网络
        virtual void RefreshNet();
        // 加载模型
        virtual bool Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list);
        // 加载模型
        virtual bool LoadModelFromFile(std::istream &is);
        // 保存模型
        virtual bool SaveModelToFile(std::ostream &os);
        // 加载保存分类索引
        virtual void LoadClassifyIndex();
        virtual void SaveClassifyIndex();
        // 训练
        virtual void Train();
        // 测试
        virtual void Test();
        // 加载数据集
        virtual void ClearDataSet();
        virtual void LoadDataSet();
        virtual void SaveDataSet();
        // 加载下一批输入
        virtual bool LoadNextBatchInput();

        // 加载输入图像
        virtual std::shared_ptr<DataImage> LoadInputImage(const std::string &input_path);
        // 预处理输入图像
        virtual std::shared_ptr<DataImage> HandleInputImage(std::shared_ptr<DataImage> input_image) { return input_image; }

    public:
        bool LoadModelFromFile(const std::string &model_path) { return Model::LoadModelFromFile(model_path); }
        bool SaveModelToFile(const std::string &model_path) { return Model::SaveModelToFile(model_path); }
        int Predict(float *in_datas, float *predict_probability);
        int Predict(const std::string &img_path, float *predict_probability);
        int Predict(const unsigned char *pixels, const int size, float *predict_probability);
        int Predict(std::shared_ptr<DataImage> input_image, float *predict_probability);

    public:
        inline std::unordered_map<std::string, std::shared_ptr<SoftMaxModelDataItem>> *GetDataSet() { return &m_dataset; }
        inline void SetInputIndex(int data_index) { m_input_index = data_index; }
        inline int GetInputIndex() { return m_input_index; }
        inline void SetCacheSize(int cache_size) { m_cache_size = cache_size; }
        inline int GetCacheSize() { return m_cache_size; }
        inline void SetPathClassifyItem(const std::string &path, const std::string &classify) { m_dataset.insert_or_assign(path, std::make_shared<SoftMaxModelDataItem>(path, classify)); }
        inline void UnsetPathClassifyItem(const std::string &path) { m_dataset.erase(path); }

    protected:
        void LoadInputs(Options *options);
        void LoadInputs(std::istream &is);
        void LoadInputs(const std::string &text);
        bool LoadInput(const std::string &img_path, const std::string &classify, float *out_datas, int out_datas_size, float *out_truths, int out_truths_size);
        bool LoadInput(std::shared_ptr<DataImage> input_image, int classify_index, float *out_datas, int out_datas_size, float *out_truths, int out_truths_size);

    private:
        // 训练数据
        int m_input_index;                                                                // 输入索引
        int m_cache_size;                                                                 // 输入缓存大小
        std::unordered_map<std::string, std::weak_ptr<DataImage>> m_input_images;         // 输入图像
        std::deque<std::shared_ptr<DataImage>> m_cache_images;                            // 缓存图像
        std::unordered_map<std::string, std::shared_ptr<SoftMaxModelDataItem>> m_dataset; // 数据集
    protected:
        std::unordered_map<std::string, int> m_classify_index_map;
        std::unordered_map<int, std::string> m_index_classify_map;

    public:
        std::vector<std::string> GetClassifyList()
        {
            std::vector<std::string> classify_list;
            for (auto &item : m_classify_index_map)
            {
                classify_list.push_back(item.first);
            }
            return std::move(classify_list);
        }

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
            if (!is.good()) return;

            int size = 0;
            is.read((char *)(&size), sizeof(size));
            for (int i = 0; i < size && is.good(); i++)
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
    };
}
#endif