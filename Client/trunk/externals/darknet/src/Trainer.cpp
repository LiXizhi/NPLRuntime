#include "Trainer.h"
#include "DataSet.h"
#include "Layer.h"
#include "Net.h"
#include "utils.h"

Trainer::Trainer()
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

    m_default_model_filename = "model.lastest";
    m_costs.assign(10, FLT_MAX);

    m_root_path = std::filesystem::current_path().string();
    m_net       = std::make_shared<Net>();
}

bool Trainer::LoadFromIniFile(const std::string &ini_path)
{
    auto options_list = Options::Load(ini_path);
    if (options_list == nullptr || options_list->size() == 0) return false;
    m_root_path = std::filesystem::path(ini_path).parent_path().string();
    return Load(options_list->at(0).get());
}

bool Trainer::LoadFromIniText(const std::string &ini_text)
{
    std::istringstream iss(ini_text);
    auto options_list = Options::Load(iss);
    if (options_list == nullptr || options_list->size() == 0) return false;
    return Load(options_list->at(0).get());
}

bool Trainer::Load(Options *options)
{
    m_root_path    = options->GetOption("root", m_root_path.c_str());
    m_net_path     = options->GetOption("net", "");
    m_models_path  = options->GetOption("models", "");
    m_dataset_path = options->GetOption("dataset", "");
    m_debugs_path  = options->GetOption("debugs", "");
    m_train_path   = options->GetOption("train", "");
    m_test_path    = options->GetOption("test", "");

    auto root_path = std::filesystem::path(m_root_path);
    if (m_models_path.empty()) m_models_path = (root_path / "models").string();
    if (m_debugs_path.empty()) m_debugs_path = (root_path / "debugs").string();
    if (m_net_path.empty()) m_net_path = (root_path / "net").string();
    if (!m_dataset_path.empty() && !std::filesystem::path(m_dataset_path).is_absolute()) m_dataset_path = (root_path / m_dataset_path).string();

    // 创建相关目录
    if (!m_models_path.empty() && !std::filesystem::exists(m_models_path)) std::filesystem::create_directories(m_models_path);
    if (!m_debugs_path.empty() && !std::filesystem::exists(m_debugs_path)) std::filesystem::create_directories(m_debugs_path);

    // 加载网络
    auto model_path = std::filesystem::path(m_models_path) / m_default_model_filename;
    if (!std::filesystem::exists(model_path) || !m_net->Load(model_path.string()))
    {
        if (!std::filesystem::exists(m_net_path) || !m_net->Load(Options::Load(m_net_path)))
        {
            return false;
        }
    }

    // 解析真实值
    ParseNetInfo();

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

    if (!m_dataset_path.empty()) m_dataset = LoadDataSet(m_dataset_path);
    m_net->SetBatch(m_batch);
    return true;
}

bool Trainer::LoadModelFromFile(const std::string &model_path)
{
    return m_net->Load(model_path);
}

bool Trainer::SaveModelToFile(const std::string &model_path)
{
    return m_net->Save(model_path);
}

void Trainer::ParseNetInfo()
{
    auto layer      = m_net->GetBackLayer();
    auto layer_type = layer->GetType();
    if (layer_type == Layer::s_yolo)
    {
        // clang-format off
        m_truth_creator =  []() -> std::shared_ptr<Truth> { return std::make_shared<YoloTruth>(); };
        // clang-format on
    }
    else
    {
        auto truths_size = layer->GetOutSize();
        // clang-format off
        m_truth_creator =  [truths_size]() -> std::shared_ptr<Truth> { return std::make_shared<Truth>(truths_size); };
        // clang-format on
    }

    // clang-format off
    m_dataitem_creator = []() -> std::shared_ptr<DataItem> { return std::make_shared<DataItem>(); };
    // clang-format on
}

void Trainer::PrintTrainerInfo(std::ostream *os)
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

float Trainer::DoEpoch()
{
    auto net     = GetNet();
    auto batch   = GetBatch();
    auto dataset = GetDataSet();
    if (dataset == nullptr) return 0.f;

    net->SetTrain(true);
    net->SetBatch(batch);
    // 获取下一批数据
    auto input = dataset->GetNextBatchInput(batch);
    if (input == nullptr) return 0.f;

    // 设置输入输出流
    net->SetInOSS(input->GetInOSS());
    // 设置输入数据
    net->SetInDatas(input->GetDatas());
    net->SetInTruths(input->GetTruths());

    // 前向网络
    net->Forward();
    // 计算损失
    auto cost = net->Loss();
    // 反向网络
    net->Backward();

    // 更新网络
    net->Update(GetCurrentLearningRate(), GetMomentum(), GetDecay(), GetEpochBatchSize());

    return cost;
}

void Trainer::Train()
{
    m_epochs             = 0;
    auto net             = GetNet();
    auto dataset         = GetTrainDataSet();
    auto os              = GetTrainOS();
    auto max_epochs      = GetMaxEpcohs();
    auto auto_stop_train = IsAutoStopTrain();
    auto batch           = GetBatch();
    int costs_size       = m_costs.size();
    float last_avg_cost  = FLT_MAX;

    net->SetTrain(true);
    net->SetBatch(batch);
    PrintTrainerInfo(os);

    while (m_epochs < max_epochs)
    {
        auto start_ts = get_timestamp();

        // 获取下一批数据
        auto input = dataset->GetNextBatchInput(batch);
        if (input == nullptr) return;
        // 设置输入输出流
        net->SetInOSS(input->GetInOSS());

        // 设置输入数据
        net->SetInDatas(input->GetDatas());
        net->SetInTruths(input->GetTruths());

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

    auto models_directory = std::filesystem::path(GetModelsPath());
    if (!models_directory.empty()) net->Save((models_directory / m_default_model_filename).string());
}

void Trainer::Test()
{
    auto net     = GetNet();
    auto dataset = GetTestDataSet();
    net->SetTrain(true);
    net->SetBatch(1);
    std::ostream *ofs = GetTestOS();
    if (ofs == nullptr) ofs = &std::cout;

    auto layer          = net->GetBackLayer();
    int data_items_size = dataset->GetDataItems()->size();
    for (int i = 0; i < data_items_size; i++)
    {
        auto start_ts = get_timestamp();

        // 获取下一批数据
        auto input = dataset->GetNextBatchInput(1);
        if (input == nullptr) return;
        auto data_item = input->GetDataItem(0);
        *ofs << std::endl;
        *ofs << "test epochs = " << i << std::endl;
        *ofs << "in datas = " << data_item->GetInDatas() << std::endl;
        *ofs << "in truths = " << data_item->GetInTruths() << std::endl;

        // 设置输入数据
        net->SetInDatas(input->GetDatas());
        net->SetInTruths(input->GetTruths());

        // 前向网络
        net->Forward();

        // 打印预测
        layer->PrintPredicts(ofs);

        auto end_ts = get_timestamp();
        *ofs << "use times:" << (end_ts - start_ts) << "ms" << std::endl;
    }
}

std::vector<float> *Trainer::Predict(float *img_datas, int img_width, int img_height)
{
    auto net = GetNet();
    net->SetTrain(false);
    net->SetBatch(1);
    net->SetInDatas(img_datas);
    net->SetInTruths(nullptr);
    net->Forward();
    auto net_width  = net->GetWidth();
    auto net_height = net->GetHeight();
    auto layer      = net->GetBackLayer();
    auto results    = layer->GetPredicts();
    if (layer->GetType() != Layer::s_yolo) return results;
    auto size  = results->size() / 6;
    auto datas = results->data();

    for (int i = 0; i < size; i++)
    {
        DataSet::NetBoxToImageBox(datas, datas, img_width, img_height, net_width, net_height);
        datas[0] = datas[0] * img_width;
        datas[1] = datas[1] * img_height;
        datas[2] = datas[2] * img_width;
        datas[3] = datas[3] * img_height;
        datas += 6;
    }
    return results;
}

std::vector<float> *Trainer::Predict(const std::string &img_path)
{
    int img_width   = 0;
    int img_height  = 0;
    auto net        = GetNet();
    auto net_width  = net->GetWidth();
    auto net_height = net->GetHeight();
    auto img        = DataSet::LoadNetImage(img_path, net_width, net_height, &img_width, &img_height);
    if (img == nullptr) return nullptr;
    return Predict(img->GetData(), img_width, img_height);
}

std::vector<float> *Trainer::Predict(const unsigned char *pixels, const int size)
{
    int img_width   = 0;
    int img_height  = 0;
    auto net        = GetNet();
    auto net_width  = net->GetWidth();
    auto net_height = net->GetHeight();
    auto img        = DataSet::LoadNetImage(pixels, size, net_width, net_height, &img_width, &img_height);
    if (img == nullptr) return nullptr;
    return Predict(img->GetData(), img_width, img_height);
}

float Trainer::GetAverageCost()
{
    auto costs_size = m_costs.size();
    auto sum        = 0.f;
    for (auto i = 0; i < costs_size; i++)
    {
        sum += m_costs[i];
    }
    return sum / costs_size;
}

float Trainer::GetCurrentLearningRate()
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

std::shared_ptr<DataSet> Trainer::LoadDataSet(const std::string &dataset_path)
{
    auto net     = GetNet();
    auto dataset = std::make_shared<DataSet>();
    dataset->SetRandom(IsRandom());
    dataset->SetAugment(IsAugment());
    dataset->SetJitter(GetJitter());
    dataset->SetFilpHorizontal(IsFilpHorizontal());
    dataset->SetFilpVertical(IsFilpVertical());
    dataset->SetDistort(IsDistort());
    dataset->SetHue(GetHue());
    dataset->SetSaturation(GetSaturation());
    dataset->SetExposure(GetExposure());
    dataset->SetDebug(IsDebug());
    dataset->SetDebugsPath(GetDebugsPath());
    dataset->SetWidth(net->GetWidth());
    dataset->SetHeight(net->GetHeight());
    dataset->SetChannels(net->GetChannels());
    dataset->SetMaxTruthsSize(net->GetMaxTruthsSize());
    dataset->SetTruthCreator(GetTruthCreator());
    dataset->SetDataItemCreator(GetDataItemCreator());

    auto ifs = std::ifstream(dataset_path);
    if (ifs.is_open()) dataset->Load(ifs);
    return dataset;
}

std::shared_ptr<DataSet> Trainer::GetTrainDataSet()
{
    auto net     = GetNet();
    auto dataset = std::make_shared<DataSet>();
    dataset->SetRandom(IsRandom());
    dataset->SetAugment(IsAugment());
    dataset->SetJitter(GetJitter());
    dataset->SetFilpHorizontal(IsFilpHorizontal());
    dataset->SetFilpVertical(IsFilpVertical());
    dataset->SetDistort(IsDistort());
    dataset->SetHue(GetHue());
    dataset->SetSaturation(GetSaturation());
    dataset->SetExposure(GetExposure());
    dataset->SetDebug(IsDebug());
    dataset->SetDebugsPath(GetDebugsPath());
    dataset->SetWidth(net->GetWidth());
    dataset->SetHeight(net->GetHeight());
    dataset->SetChannels(net->GetChannels());
    dataset->SetMaxTruthsSize(net->GetMaxTruthsSize());
    dataset->SetTruthCreator(GetTruthCreator());
    dataset->SetDataItemCreator(GetDataItemCreator());

    auto ifs = std::ifstream(m_train_path.empty() ? m_dataset_path : m_train_path);
    dataset->Load(ifs);
    return dataset;
}

std::shared_ptr<DataSet> Trainer::GetTestDataSet()
{
    auto net     = GetNet();
    auto dataset = std::make_shared<DataSet>();
    dataset->SetRandom(false);
    dataset->SetAugment(false);
    dataset->SetDebug(false);
    dataset->SetWidth(net->GetWidth());
    dataset->SetHeight(net->GetHeight());
    dataset->SetChannels(net->GetChannels());
    dataset->SetMaxTruthsSize(net->GetMaxTruthsSize());
    dataset->SetTruthCreator(GetTruthCreator());
    dataset->SetDataItemCreator(GetDataItemCreator());

    auto ifs = std::ifstream(m_test_path);
    dataset->Load(ifs);
    return dataset;
}

std::ostream *Trainer::GetTrainOS()
{
    if (m_debugs_path.empty()) return &std::cout;
    if (m_train_os != nullptr) return m_train_os.get();
    auto train_os_path = std::filesystem::path(m_debugs_path) / "train.txt";
    m_train_os         = std::make_shared<std::ofstream>(train_os_path);
    return m_train_os.get();
}

std::ostream *Trainer::GetTestOS()
{
    if (m_debugs_path.empty()) return &std::cout;
    if (m_test_os != nullptr) return m_test_os.get();
    auto tests_os_path = std::filesystem::path(m_debugs_path) / "test.txt";
    m_test_os          = std::make_shared<std::ofstream>(tests_os_path);
    return m_test_os.get();
}