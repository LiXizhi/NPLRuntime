
#ifndef __DARKNET_NET_H__
#define __DARKNET_NET_H__
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
namespace darknet
{
    class Options;
    class Layer;
    class Input;

    class Net
    {
    public:
        Net();

        // 创建一个网络层
        std::shared_ptr<Layer> MakeLayer(Layer *prev_layer, std::shared_ptr<Options> layer_options);
        // 加载网络结构
        bool Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list);

        // 加载保存模型(网络结构和参数)
        bool Save(const std::string &model_path);
        bool Load(const std::string &model_path);

        // 加载保存模型(网络结构和参数)
        bool Save(std::ostream &os);
        bool Load(std::istream &is);

    public:
        // 网络前向传播
        void Forward();
        // 网络方向传播
        void Backward();
        // 更新网络
        void Update(float learning_rate, float momentum, float decay, int epoch_batch_size);
        // 获取网络当前损失
        float Loss();
        // 打印网络信息
        void PrintNetInfo(std::ostream *os = nullptr);
        // 设置网络训练批大小
        void SetBatch(int batch);

    public:
        inline bool IsLoaded() { return !m_layers.empty(); }
        inline std::shared_ptr<Options> GetOptions() { return m_options; }
        inline std::vector<std::shared_ptr<Layer>> *GetLayers() { return &m_layers; }
        inline Layer *GetLayer(int index) { return (index >= 0 && index < m_layers.size()) ? m_layers[index].get() : nullptr; }
        inline int GetLayersSize() { return m_layers.size(); }
        inline float *GetWorkspace() { return m_workspace.data(); }
        inline int GetWidth() { return m_width; }
        inline int GetHeight() { return m_height; }
        inline int GetChannels() { return m_channels; }
        inline Layer *GetFrontLayer() { return m_layers.empty() ? nullptr : m_layers.front().get(); }
        inline Layer *GetBackLayer() { return m_layers.empty() ? nullptr : m_layers.back().get(); }
        inline void SetInDatas(float *in_datas) { m_in_datas = in_datas; }
        inline float *GetInDatas() { return m_in_datas; }
        inline void SetInTruths(float *in_truths) { m_in_truths = in_truths; }
        inline float *GetInTruths() { return m_in_truths; }
        inline std::shared_ptr<std::vector<std::ostream *>> GetInOSS() { return m_in_oss; }
        inline void SetInOSS(std::shared_ptr<std::vector<std::ostream *>> in_oss) { m_in_oss = in_oss; }
        inline bool GetBatchNormalize() { return m_batch_normalize; }
        inline int GetMaxTruthsSize() { return m_max_truths * m_truths_size; } // 总真实值大小  m_max_truths * m_truths_size
        inline int GetMaxTruths() { return m_max_truths; }                     // 真实值最大数量
        inline int GetTruthsSize() { return m_truths_size; }                   // 单个真实值大小
        inline void SetMaxTruths(int max_truths) { m_max_truths = max_truths; }
        inline void SetTruthsSize(int truths_size) { m_truths_size = truths_size; }

    protected:
        int m_width;                                           // 输入数据宽度
        int m_height;                                          // 输入数据高度
        int m_channels;                                        // 输入数据深度
        int m_batch;                                           // 单次训练采样大小
        int m_truths_size;                                     // 单个真实值大小
        int m_max_truths;                                      // 真实值最大数量
        bool m_batch_normalize;                                // 是否使用BatchNormalize
        std::vector<std::shared_ptr<Layer>> m_layers;          // 网络层
        std::shared_ptr<Options> m_options;                    // 网络结构参数
        std::vector<float> m_workspace;                        // 网络层工作空间 缓冲区
        float *m_in_datas;                                     // 输入数据
        float *m_in_truths;                                    // 输入真实值
        std::shared_ptr<std::vector<std::ostream *>> m_in_oss; // 日志输出流
    };
}
#endif