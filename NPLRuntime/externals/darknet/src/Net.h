
#ifndef __NET_H__
#define __NET_H__
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Options;
class Layer;
class Input;

class Net
{
public:
    Net();

    // 加载网络结构
    bool Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options);

    // 加载保存模型(网络结构和参数)
    bool Save(const std::string &model_path);
    bool Load(const std::string &model_path);

protected:
    bool Save(std::ostream &os);
    bool Load(std::istream &is);

public:
    void Forward();
    void Backward();
    void Update(float learning_rate, float momentum, float decay, int epoch_batch_size);
    float Loss();

    void PrintNetInfo(std::ostream *os = nullptr);
    void SetBatch(int batch);

public:
    inline std::vector<std::shared_ptr<Layer>> *GetLayers() { return &m_layers; }
    inline Layer *GetLayer(int index) { return (index >= 0 && index < m_layers.size()) ? m_layers[index].get() : nullptr; }
    inline int GetLayersSize() { return m_layers.size(); }
    inline float *GetWorkspace() { return m_workspace.data(); }
    inline int GetWidth() { return m_width; }
    inline int GetHeight() { return m_height; }
    inline int GetChannels() { return m_channels; }
    inline bool SetTrain(bool train) { return m_train = train; }
    inline bool IsTrain() { return m_train; }
    inline bool IsPredict() { return !m_train; }
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
protected:
    int m_width;
    int m_height;
    int m_channels;
    int m_batch;
    int m_truths_size; // 真实值大小
    int m_max_truths;  // 真实值最大数量
    bool m_batch_normalize;
    bool m_train;
    std::vector<std::shared_ptr<Layer>> m_layers;
    std::shared_ptr<std::vector<std::shared_ptr<Options>>> m_options;
    std::vector<float> m_workspace;
    std::shared_ptr<std::vector<std::ostream *>> m_in_oss;
    float *m_in_datas;
    float *m_in_truths;
    int m_max_workspace_size; // 最大工作空间大小
};

#endif