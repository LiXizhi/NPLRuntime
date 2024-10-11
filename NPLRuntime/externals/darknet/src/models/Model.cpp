#include "layers/Layer.h"
#include "utils/utils.h"

#include "Model.h"
namespace darknet
{
    Model::Model()
    {
        m_epochs          = 0;
        m_max_epochs      = 1000;
        m_batch           = 8;
        m_auto_stop_train = true;
        m_debug           = false;
        m_policy          = s_policy_constant;
        m_learning_rate   = 0.0025f;
        m_power           = 2.0f;
        m_gamma           = 1.0f;
        m_momentum        = 0.9f;
        m_decay           = 0.0005f;
        m_random          = true;
        m_augment         = true;
        m_jitter          = 0.1f; // jitter = 0.1 epochs >= 2000
        m_filp_horizontal = false;
        m_filp_vertical   = false;
        m_distort         = true;
        m_saturation      = 1.5f;
        m_exposure        = 1.5f;
        m_hue             = 0.1f;

        m_model_name = "default";
        m_costs.assign(10, FLT_MAX);

        m_root_directory = std::filesystem::current_path().string();
        m_net            = std::make_shared<Net>();
    }

    bool Model::LoadFromIniFile(const std::string &ini_path)
    {
        auto options_list = Options::Load(ini_path);
        if (options_list == nullptr || options_list->size() == 0) return false;
        if (m_root_directory.empty()) m_root_directory = std::filesystem::path(ini_path).parent_path().string();
        return Load(options_list);
    }

    bool Model::LoadFromIniText(const std::string &ini_text)
    {
        std::istringstream iss(ini_text);
        auto options_list = Options::Load(iss);
        if (options_list == nullptr || options_list->size() == 0) return false;
        return Load(options_list);
    }

    // 加载网络
    void Model::LoadNet()
    {
        if (GetNet()->IsLoaded()) return;
        auto model_path = GetModelPath();
        if (!std::filesystem::exists(model_path) || !LoadModelFromFile(model_path))
        {
            if (!std::filesystem::exists(m_net_path) || !GetNet()->Load(Options::Load(m_net_path)))
            {
                return;
            }
        }
    }

    bool Model::Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list)
    {
        m_options_list = options_list;
        if (m_options_list == nullptr || m_options_list->size() == 0) return false;

        auto options = GetOptions("model");
        if (options == nullptr) return false;

        m_root_directory    = options->GetOption("root", m_root_directory.c_str());
        m_model_directory   = options->GetOption("models", m_model_directory.c_str());
        m_dataset_directory = options->GetOption("datasets", m_dataset_directory.c_str());
        m_model_path        = options->GetOption("model", m_model_path.c_str());
        m_dataset_path      = options->GetOption("dataset", m_dataset_path.c_str());
        m_net_path          = options->GetOption("net", m_net_path.c_str());

        auto root_path = std::filesystem::path(m_root_directory);
        if (m_model_directory.empty()) m_model_directory = (root_path / "models").string();
        if (m_dataset_directory.empty()) m_dataset_directory = (root_path / "datasets").string();
        if (m_model_path.empty() || !std::filesystem::path(m_model_path).is_absolute()) m_model_path = (std::filesystem::path(m_model_directory) / (m_model_path.empty() ? GetDefaultModelFileName() : m_model_path)).string();
        if (m_dataset_path.empty() || !std::filesystem::path(m_dataset_path).is_absolute()) m_dataset_path = (std::filesystem::path(m_dataset_directory) / (m_dataset_path.empty() ? "dataset.txt" : m_dataset_path)).string();
        if (m_net_path.empty()) m_net_path = (root_path / "net").string();

        // 创建相关目录
        if (!m_model_directory.empty() && !std::filesystem::exists(m_model_directory)) std::filesystem::create_directories(m_model_directory);
        if (!m_dataset_directory.empty() && !std::filesystem::exists(m_dataset_directory)) std::filesystem::create_directories(m_dataset_directory);

        // 获取训练器参数
        m_augment         = options->GetOption("augment", m_augment);
        m_random          = options->GetOption("random", m_random);
        m_jitter          = options->GetOption("jitter", m_jitter);
        m_filp_horizontal = options->GetOption("filp_horizontal", m_filp_horizontal);
        m_filp_vertical   = options->GetOption("filp_vertical", m_filp_vertical);
        m_hue             = options->GetOption("hue", m_hue);
        m_saturation      = options->GetOption("saturation", m_saturation);
        m_exposure        = options->GetOption("exposure", m_exposure);
        m_distort         = options->GetOption("distort", m_distort);
        m_batch           = options->GetOption("batch", m_batch);
        m_max_epochs      = options->GetOption("max_epochs", m_max_epochs);
        m_auto_stop_train = options->GetOption("auto_stop_train", m_auto_stop_train);
        m_debug           = options->GetOption("debug", m_debug);
        m_momentum        = options->GetOption("momentum", m_momentum);
        m_decay           = options->GetOption("decay", m_decay);
        m_learning_rate   = options->GetOption("learning_rate", m_learning_rate);
        m_power           = options->GetOption("power", m_power);
        m_gamma           = options->GetOption("gamma", m_gamma);
        m_policy          = GetPolicyByName(options->GetOption("policy", ""), m_policy);
        if (m_policy == s_policy_step)
        {
            m_steps.push_back(options->GetOption("step", 1));
            m_scales.push_back(options->GetOption("scale", 1.0f));
        }
        else if (m_policy == s_policy_steps)
        {
            options->ParseOption("steps", "", m_steps);
            options->ParseOption("scales", "", m_scales);
        }
        else if (m_policy == s_policy_sigmoid)
        {
            m_steps.push_back(options->GetOption("step", 1));
        }

        LoadNet();
        return true;
    }

    bool Model::LoadModelFromFile(const std::string &model_path)
    {
        std::ifstream ifs(model_path, std::ios::binary | std::ios::in);
        if (!ifs.is_open()) return false;
        return LoadModelFromFile(ifs);
    }

    bool Model::SaveModelToFile(const std::string &model_path)
    {
        std::ofstream ofs(model_path, std::ios::binary | std::ios::out);
        if (!ofs.is_open()) return false;
        return SaveModelToFile(ofs);
    }

    bool Model::LoadModelFromFile(std::istream &is)
    {
        return GetNet()->Load(is);
    }

    bool Model::SaveModelToFile(std::ostream &os)
    {
        return GetNet()->Save(os);
    }

    void Model::PrintModelInfo(std::ostream *os)
    {
        if (os == nullptr) os = &std::cout;
        auto net = GetNet();
        net->PrintNetInfo(os);

        *os << "width = " << net->GetWidth() << std::endl;
        *os << "height = " << net->GetHeight() << std::endl;
        *os << "batch_normalize = " << net->GetBatchNormalize() << std::endl;
        *os << "max_batches = " << GetMaxEpcohs() << std::endl;
        *os << "batch = " << GetBatch() << std::endl;
        *os << "learning_rate = " << GetLearningRate() << std::endl;
        *os << "momentum = " << GetMomentum() << std::endl;
        *os << "decay = " << GetDecay() << std::endl;
        *os << "policy = " << GetPolicy() << std::endl;
        *os << "jitter = " << GetJitter() << std::endl;
        *os << "saturation = " << GetSaturation() << std::endl;
        *os << "exposure = " << GetExposure() << std::endl;
        *os << "hue = " << GetHue() << std::endl;
        *os << "distort = " << IsDistort() << std::endl;
        *os << "filp_horizontal = " << IsFilpHorizontal() << std::endl;
        *os << "filp_vertical = " << IsFilpVertical() << std::endl;
        *os << "random = " << IsRandom() << std::endl;
        *os << "augment = " << IsAugment() << std::endl;
    }

    float Model::DoEpoch()
    {
        auto net      = GetNet();
        auto batch    = GetBatch();
        auto start_ts = get_timestamp();

        // 获取下一批数据
        if (!LoadNextBatchInput()) return 0.f;

        // 设置输入输出流
        net->SetInOSS(nullptr);
        // 设置输入数据
        net->SetInDatas(GetInDatas());
        net->SetInTruths(GetInTruths());

        // 前向网络
        net->Forward();
        // 计算损失
        auto cost = net->Loss();
        // 反向网络
        net->Backward();

        // 更新网络
        net->Update(GetCurrentLearningRate(), GetMomentum(), GetDecay(), GetEpochBatchSize());

        auto end_ts = get_timestamp();
        std::cout << "doepoch use times:" << (end_ts - start_ts) << " ms loss: " << cost << std::endl;
        return cost;
    }

    void Model::Train()
    {
        m_epochs             = 0;
        auto net             = GetNet();
        auto os              = GetTrainOS();
        auto max_epochs      = GetMaxEpcohs();
        auto auto_stop_train = IsAutoStopTrain();
        auto batch           = GetBatch();
        int costs_size       = m_costs.size();
        float last_avg_cost  = FLT_MAX;

        net->SetBatch(batch);
        PrintModelInfo(os);

        while (m_epochs < max_epochs)
        {
            auto start_ts = get_timestamp();

            // 获取下一批数据
            if (!LoadNextBatchInput()) return;
            // 设置输入输出流
            // net->SetInOSS(input->GetInOSS());

            // 设置输入数据
            net->SetInDatas(GetInDatas());
            net->SetInTruths(GetInTruths());

            // 前向网络
            net->Forward();
            // 计算损失
            auto cost = net->Loss();
            // 反向网络
            net->Backward();

            // 更新网络
            net->Update(GetCurrentLearningRate(), GetMomentum(), GetDecay(), GetEpochBatchSize());

            auto end_ts                    = get_timestamp();
            m_costs[m_epochs % costs_size] = cost;
            m_epochs++;

            *os << "epochs: " << m_epochs << " use times:" << (end_ts - start_ts) << " ms loss: " << cost << std::endl;
            if (os != &std::cout) std::cout << "epochs: " << m_epochs << " use times:" << (end_ts - start_ts) << " ms loss: " << cost << std::endl;

            if (auto_stop_train && (m_epochs % costs_size) == 0)
            {
                auto cur_avg_cost   = GetAverageCost();
                auto delta_avg_cost = std::abs(last_avg_cost - cur_avg_cost);
                *os << "per 10 epochs: cur_avg_cost = " << cur_avg_cost << " last_avg_cost = " << last_avg_cost << " delta_avg_cost = " << delta_avg_cost << std::endl;
                if (delta_avg_cost < 0.0001)
                {
                    *os << "auto stop train!!!" << std::endl;
                    break;
                }
                last_avg_cost = cur_avg_cost;
            }
        }

        auto model_path = GetModelPath();
        if (!model_path.empty() && !std::isnan(m_costs.back()))
        {
            SaveModelToFile(model_path);
        }
    }

    void Model::Test()
    {
        SetBatch(1);
        SetRandom(false);
        SetAugment(false);

        auto os     = GetTestOS();
        auto inputs = GetInputs();
        auto net    = GetNet();
        net->SetBatch(1);

        auto layer      = net->GetBackLayer();
        int inputs_size = inputs->size();
        for (int input_index = 0; input_index < inputs_size; input_index++)
        {
            if (!LoadNextBatchInput()) continue;

            auto start_ts = get_timestamp();

            *os << std::endl;
            *os << "test epochs = " << input_index << std::endl;
            // *os << "input data => " << dataset->at(input_index) << std::endl;

            // 设置输入数据
            net->SetInDatas(GetInDatas());
            net->SetInTruths(GetInTruths());

            // 前向网络
            net->Forward();

            // 打印预测
            layer->PrintPredicts(os);

            auto end_ts = get_timestamp();
            *os << "use times:" << (end_ts - start_ts) << "ms" << std::endl;
        }
    }

    std::vector<float> *Model::Predict(float *in_datas)
    {
        auto net = GetNet();
        net->SetBatch(1);
        net->SetInDatas(in_datas);
        net->SetInTruths(nullptr);
        net->SetInOSS(nullptr);
        net->Forward();
        return net->GetBackLayer()->GetPredicts();
    }

    float Model::GetAverageCost()
    {
        auto costs_size = m_costs.size();
        auto sum        = 0.f;
        for (auto i = 0; i < costs_size; i++)
        {
            sum += m_costs[i];
        }
        return sum / costs_size;
    }

    float Model::GetCurrentLearningRate()
    {
        auto epochs        = GetEpochs();
        auto max_epochs    = GetMaxEpcohs();
        auto policy        = GetPolicy();
        auto learning_rate = GetLearningRate();
        auto gamma         = GetGamma();
        auto power         = GetPower();
        auto scales        = GetScales();
        auto steps         = GetSteps();
        if (policy == s_policy_constant)
        {
            return learning_rate;
        }
        else if (policy == s_policy_step)
        {
            return learning_rate * std::pow(scales[0], epochs / steps[0]);
        }
        else if (policy == s_policy_steps)
        {
            float rate     = learning_rate;
            int steps_size = GetStepsSize();
            for (int i = 0; i < steps_size; ++i)
            {
                if (steps[i] > epochs) return rate;
                rate *= scales[i];
            }
            return rate;
        }
        else if (policy == s_policy_exp)
        {
            return learning_rate * std::pow(gamma, epochs);
        }
        else if (policy == s_policy_poly)
        {
            return learning_rate * std::pow(1 - (float)epochs / max_epochs, power);
        }
        else if (policy == s_policy_random)
        {
            return learning_rate * std::pow(random_float(0.0f, 1.0f), power);
        }
        else if (policy == s_policy_sigmoid)
        {
            return learning_rate * (1.0f / (1.0f + std::exp(gamma * (epochs - steps[0]))));
        }
        else
        {
            return learning_rate;
        }
    }

    std::ostream *Model::GetTrainOS()
    {
        if (!IsDebug()) return &std::cout;
        if (m_train_os != nullptr) return m_train_os.get();
        auto train_os_path = std::filesystem::path(GetRootDirectory()) / "train.txt";
        m_train_os         = std::make_shared<std::ofstream>(train_os_path);
        return m_train_os.get();
    }

    std::ostream *Model::GetTestOS()
    {
        if (!IsDebug()) return &std::cout;
        if (m_test_os != nullptr) return m_test_os.get();
        auto tests_os_path = std::filesystem::path(GetRootDirectory()) / "test.txt";
        m_test_os          = std::make_shared<std::ofstream>(tests_os_path);
        return m_test_os.get();
    }

    Options *Model::GetOptions(const std::string &name)
    {
        if (m_options_list == nullptr) return nullptr;

        for (int i = 0; i < m_options_list->size(); i++)
        {
            auto options = m_options_list->at(i);
            if (options->GetName() == name) return options.get();
        }

        return nullptr;
    }
}