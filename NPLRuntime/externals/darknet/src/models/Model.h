#ifndef __DARKNET_MODEL_H__
#define __DARKNET_MODEL_H__

#include <any>
#include <cfloat>
#include <deque>
#include <filesystem>
#include <functional>
#include <memory>

#include "cores/DataImage.h"
#include "cores/Net.h"
#include "cores/Options.h"
namespace darknet
{
    class Model
    {
    public:
        class DataItem
        {
        private:
            template <typename T>
            static void *GetType_s()
            {
                static T *s_type_instance = nullptr;
                return (void *)(&s_type_instance);
            }

        public:
            template <typename T>
            DataItem(T *) : m_type(GetType_s<T>()) {}

            template <typename T>
            T *ToType() { return this->GetType() == GetType_s<T>() ? (static_cast<T *>(this)) : (nullptr); }

        public:
            void *GetType() { return m_type; }

        protected:
            template <typename T>
            void SetType(T *) { m_type = GetType_s<T>(); }

        private:
            void *m_type;
        };

    public:
        Model();

        bool LoadFromIniFile(const std::string &ini_path);
        bool LoadFromIniText(const std::string &ini_text);
        bool LoadModelFromFile(const std::string &model_path);
        bool SaveModelToFile(const std::string &model_path);

        void PrintModelInfo(std::ostream *os = nullptr);

    public:
        virtual void LoadNet();
        virtual bool Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list);
        virtual bool LoadModelFromFile(std::istream &is);
        virtual bool SaveModelToFile(std::ostream &os);
        virtual std::vector<float> *Predict(float *in_datas);
        virtual void Train();
        virtual void Test();
        virtual float DoEpoch();
        virtual void ClearDataSet() {}
        virtual void LoadDataSet() {}
        virtual void SaveDataSet() {}
        virtual bool LoadNextBatchInput() { return false; }

    protected:
        float GetAverageCost();
        float GetCurrentLearningRate();
        std::ostream *GetTrainOS();
        std::ostream *GetTestOS();
        Options *GetOptions(const std::string &name);

    public:
        inline Net *GetNet() { return m_net.get(); }
        inline int GetNetWidth() { return m_net->GetWidth(); }
        inline int GetNetHeight() { return m_net->GetHeight(); }
        inline int GetNetChannels() { return m_net->GetChannels(); }
        inline int GetNetInDatasSize() { return m_net->GetWidth() * m_net->GetHeight() * m_net->GetChannels(); }
        inline int GetNetInTruthsSize() { return m_net->GetTruthsSize() * m_net->GetMaxTruths(); }
        std::vector<std::shared_ptr<DataItem>> *GetInputs() { return &m_inputs; };
        inline float *GetInDatas() { return m_in_datas.data(); }
        inline void SetInDatasSize(int size) { m_in_datas.resize(size); }
        inline void SetInTruthsSize(int size) { m_in_truths.resize(size); }
        inline float *GetInTruths() { return m_in_truths.data(); }
        inline void SetRootDirectory(const std::string &root_directory) { m_root_directory = root_directory; }
        inline const std::string &GetRootDirectory() { return m_root_directory; }
        inline std::string &GetModelDirectory() { return m_model_directory; }
        inline void SetModelDirectory(const std::string &model_directory) { m_model_directory = model_directory; }
        inline std::string &GetDataSetDirectory() { return m_dataset_directory; }
        inline void SetDataSetDirectory(const std::string &dataset_directory) { m_dataset_directory = dataset_directory; }
        inline const std::string &GetDataSetPath() { return m_dataset_path; }
        inline void SetDataSetPath(const std::string &dataset_path) { m_dataset_path = dataset_path; }

        inline int GetEpochBatchSize() { return m_batch; }
        inline void SetEpochs(int epochs) { m_epochs = epochs; }
        inline int GetEpochs() { return m_epochs; }
        inline void SetMaxEpcohs(int max_epochs) { m_max_epochs = max_epochs; }
        inline int GetMaxEpcohs() { return m_max_epochs; }
        inline void SetBatch(int batch) { m_batch = batch; }
        inline int GetBatch() { return m_batch; }
        inline void SetMomentum(float momentum) { m_momentum = momentum; }
        inline float GetMomentum() { return m_momentum; }
        inline void SetDecay(float decay) { m_decay = decay; }
        inline float GetDecay() { return m_decay; }
        inline void SetLearningRate(float learning_rate) { m_learning_rate = learning_rate; }
        inline float GetLearningRate() { return m_learning_rate; }
        inline int GetPolicy() { return m_policy; }
        inline float GetGamma() { return m_gamma; }
        inline float GetPower() { return m_power; }
        inline int *GetSteps() { return m_steps.data(); }
        inline int GetStepsSize() { return m_steps.size(); }
        inline float *GetScales() { return m_scales.data(); }
        inline int GetScalesSize() { return m_scales.size(); }

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

        inline void SetAutoStopTrain(bool auto_stop_train) { m_auto_stop_train = auto_stop_train; }
        inline bool IsAutoStopTrain() { return m_auto_stop_train; }
        inline void SetDebug(bool debug) { m_debug = debug; }
        inline bool IsDebug() { return m_debug; }
        inline void SetModelPath(const std::string &model_path) { m_model_path = model_path; }
        inline const std::string &GetModelPath() { return m_model_path; }
        inline const std::string &GetModelName() { return m_model_name; }
        inline const std::string GetDefaultModelFileName() { return m_model_name + ".model.lastest"; }

    protected:
        std::shared_ptr<Net> m_net;
        std::shared_ptr<std::vector<std::shared_ptr<Options>>> m_options_list;
        std::vector<std::shared_ptr<DataItem>> m_inputs;
        std::shared_ptr<std::ofstream> m_train_os;
        std::shared_ptr<std::ofstream> m_test_os;
        std::vector<float> m_in_datas;        // 输入数据
        std::vector<float> m_in_truths;       // 输入真实值
        bool m_debug;                         // 是否调试
        bool m_auto_stop_train;               // 是否自动停止训练
        std::vector<float> m_costs;           // 网络成本
        std::string m_model_name;             // 模型名称
        std::string m_root_directory;         // 根目录
        std::string m_net_path;               // 网络路径
        std::string m_dataset_directory;      // 数据集目录
        std::string m_dataset_path;           // 数据集路径
        std::string m_model_directory;        // 模型目录
        std::string m_model_path;             // 模型路径

        int m_epochs;          // 当前迭代多少次
        int m_max_epochs;      // 最大训练轮数
        int m_batch;           // 训练批次
        float m_learning_rate; // 学习率
        float m_momentum;      // 增强
        float m_decay;         // 衰减

        int m_policy;                // 学习率策略
        float m_power;               // 学习率指数
        float m_gamma;               // 学习率指数
        std::vector<int> m_steps;    // 学习率调整点
        std::vector<float> m_scales; // 学习率调整点

        bool m_random;          // 是否随机
        bool m_augment;         // 是否增强
        float m_jitter;         // 抖动
        bool m_filp_horizontal; // 是否水平翻转
        bool m_filp_vertical;   // 是否垂直翻转
        bool m_distort;         // 是否变形图像
        float m_saturation;     // 饱和度
        float m_exposure;       // 曝光
        float m_hue;            // 色度
    public:
        static const int s_policy_constant = 0;
        static const int s_policy_step     = 1;
        static const int s_policy_exp      = 2;
        static const int s_policy_poly     = 3;
        static const int s_policy_sigmoid  = 4;
        static const int s_policy_steps    = 5;
        static const int s_policy_random   = 6;

        static const int GetPolicyByName(const std::string &policy, const int default_policy = s_policy_constant)
        {
            if (policy == "random") return s_policy_random;
            if (policy == "poly") return s_policy_poly;
            if (policy == "constant") return s_policy_constant;
            if (policy == "step") return s_policy_step;
            if (policy == "exp") return s_policy_exp;
            if (policy == "sigmoid") return s_policy_sigmoid;
            if (policy == "steps") return s_policy_steps;
            return default_policy;
        }
    };
}
#endif