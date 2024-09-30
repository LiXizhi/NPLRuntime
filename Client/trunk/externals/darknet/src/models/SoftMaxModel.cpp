#include "SoftMaxModel.h"
namespace darknet
{
    SoftMaxModel::SoftMaxModel()
    {
        m_input_index = 0;
        m_cache_size  = 64;
    }

    bool SoftMaxModel::Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list)
    {
        if (!Model::Load(options_list)) return false;

        auto names_options = GetOptions("names");
        if (names_options != nullptr)
        {
            auto options = names_options->GetOptions();
            for (auto it = options->begin(); it != options->end(); it++)
            {
                auto classify                  = it->first;
                auto index                     = std::stoi(it->second);
                m_classify_index_map[classify] = index;
                m_index_classify_map[index]    = classify;
            }
        }
        return true;
    }

    bool SoftMaxModel::LoadModelFromFile(std::istream &is)
    {
        if (!Model::LoadModelFromFile(is)) return false;
        LoadClassifyIndex();
        return true;
    }

    bool SoftMaxModel::SaveModelToFile(std::ostream &os)
    {
        SaveClassifyIndex();
        return Model::SaveModelToFile(os);
    }

    void SoftMaxModel::LoadClassifyIndex()
    {
        std::string softmax_classify_index = GetNet()->GetOptions()->GetOption("softmax_classify_index", "");
        if (!softmax_classify_index.empty())
        {
            std::istringstream iss(softmax_classify_index);
            LoadClassifyIndex(iss);
        }
    }

    void SoftMaxModel::SaveClassifyIndex()
    {
        std::ostringstream oss;
        SaveClassifyIndex(oss);
        GetNet()->GetOptions()->SetOption("softmax_classify_index", oss.str());
    }

    void SoftMaxModel::Train()
    {
        // 加载网络
        LoadNet();

        // 加载数据集
        LoadDataSet();

        // 刷新网络
        RefreshNet();

        // 开始训练
        Model::Train();

        // 保存数据集
        SaveDataSet();
    }

    void SoftMaxModel::Test()
    {
        // 加载数据集
        LoadDataSet();

        // 开始测试
        Model::Test();
    }

    void SoftMaxModel::RefreshNet()
    {
        // 设置真实值大小
        GetNet()->SetTruthsSize(GetClassifySize());
        GetNet()->SetMaxTruths(1);
        GetNet()->SetBatch(GetBatch());

        auto layers        = GetNet()->GetLayers();
        auto classify_size = GetClassifySize();

        if (layers->size() > 0 && layers->back()->GetType() == Layer::s_softmax && layers->back()->GetOutSize() == classify_size) return;

        // 移除softmax层
        std::shared_ptr<Options> softmax_layer_options = nullptr;
        if (layers->size() > 0 && layers->back()->GetType() == Layer::s_softmax)
        {
            softmax_layer_options = layers->back()->GetOptions();
            layers->pop_back();
        }
        else
        {
            softmax_layer_options = std::make_shared<Options>("softmax");
        }

        // 连接层
        std::shared_ptr<Options> connected_layer_options = nullptr;
        if (layers->size() > 0 && layers->back()->GetType() == Layer::s_connected)
        {
            connected_layer_options = layers->back()->GetOptions();
            connected_layer_options->SetOption("output", classify_size);
            layers->pop_back();
        }
        else
        {
            connected_layer_options = std::make_shared<Options>("connected");
            connected_layer_options->SetOption("output", classify_size);
            connected_layer_options->SetOption("activation", "linear");
        }

        auto connected_layer = GetNet()->MakeLayer(GetNet()->GetBackLayer(), connected_layer_options);
        layers->push_back(connected_layer);

        auto softmax_layer = GetNet()->MakeLayer(GetNet()->GetBackLayer(), softmax_layer_options);
        layers->push_back(softmax_layer);
    }

    void SoftMaxModel::ClearDataSet()
    {
        GetDataSet()->clear();
    }

    void SoftMaxModel::LoadDataSet()
    {
        // 清空分类索引
        ClearClassifyIndex();

        // 加载分类索引
        LoadClassifyIndex();

        // 加载数据集可选项
        LoadInputs(GetOptions("dataset"));

        // 数据集配置文件
        if (!m_dataset_path.empty() && std::filesystem::exists(m_dataset_path))
        {
            std::ifstream ifs(m_dataset_path);
            if (ifs.is_open()) LoadInputs(ifs);
        }

        auto dataset = GetDataSet();
        auto inputs  = GetInputs();

        inputs->clear();
        for (auto it = dataset->begin(); it != dataset->end(); it++)
        {
            auto dataitem = it->second;
            inputs->push_back(dataitem);
            CreateGetClassifyIndex(dataitem->GetClassify());
        }
    }

    void SoftMaxModel::SaveDataSet()
    {
        auto dataset_path = GetDataSetPath();
        if (dataset_path.empty())
        {
            auto dataset_directory = GetDataSetDirectory();
            if (dataset_directory.empty()) return;
            dataset_path = dataset_directory + "/dataset.txt";
            SetDataSetPath(dataset_path);
        }

        auto dataset = GetDataSet();
        std::ofstream ofs(dataset_path, std::ios::out);
        for (auto it = dataset->begin(); it != dataset->end(); it++)
        {
            auto dataitem = it->second;
            ofs << dataitem->GetPath() << " " << dataitem->GetClassify() << std::endl;
        }
    }

    bool SoftMaxModel::LoadNextBatchInput()
    {
        auto batch_input_index = 0;
        auto input_index       = GetInputIndex();
        auto inputs            = GetInputs();
        auto input_size        = GetInputs()->size();
        auto batch             = GetBatch();
        auto in_datas_size     = GetNetInDatasSize();
        auto in_truths_size    = GetNetInTruthsSize();

        // 分配内存
        SetInDatasSize(in_datas_size * batch);
        SetInTruthsSize(in_truths_size * batch);

        auto in_datas  = GetInDatas();
        auto in_truths = GetInTruths();
        auto is_radom  = IsRandom();

        std::unordered_set<int> invalid_data_index_set;
        while (batch_input_index < batch && invalid_data_index_set.size() < input_size)
        {
            auto data_index = is_radom ? random_int(0, input_size - 1) : (input_index++ % input_size);
            if (invalid_data_index_set.find(data_index) != invalid_data_index_set.end()) continue;

            // 获取采样项
            auto input = inputs->at(data_index)->ToType<SoftMaxModelDataItem>();

            // 加载采样数据
            if (!LoadInput(input->GetPath(), input->GetClassify(), in_datas, in_datas_size, in_truths, in_truths_size))
            {
                std::cout << "invalid input data => " << input->GetPath() << std::endl;
                invalid_data_index_set.insert(data_index);
                continue;
            }

            in_datas += in_datas_size;
            in_truths += in_truths_size;
            batch_input_index++;
        }
        SetInputIndex(GetInputIndex() + batch);
        return true;
    }

    int SoftMaxModel::Predict(float *in_datas, float *predict_probability)
    {
        // auto start_time = get_timestamp();
        auto results   = Model::Predict(in_datas);
        auto size      = results->size();
        auto max_index = 0;
        auto max_value = results->at(0);

        for (int i = 0; i < size; i++)
        {
            if (results->at(i) > max_value)
            {
                max_index = i;
                max_value = results->at(i);
            }
            // std::cout << i << " " << results->at(i) << std::endl;
        }

        if (predict_probability != nullptr) *predict_probability = max_value;

        // auto end_time = get_timestamp();
        // std::cout << "predict use time: " << (end_time - start_time) << "ms" << ", classify index: " << max_index << ", probability: " << max_value << std::endl;
        return max_index;
    }

    int SoftMaxModel::Predict(std::shared_ptr<DataImage> input_image, float *predict_probability)
    {
        input_image = HandleInputImage(input_image);
        if (input_image == nullptr) return -1;
        input_image = (input_image->GetWidth() == GetNetWidth() && input_image->GetHeight() == GetNetHeight()) ? input_image : input_image->ScaleFill(GetNetWidth(), GetNetHeight());
        return Predict(input_image->GetData(), predict_probability);
    }

    int SoftMaxModel::Predict(const std::string &img_path, float *predict_probability)
    {
        return Predict(LoadInputImage(img_path), predict_probability);
    }

    int SoftMaxModel::Predict(const unsigned char *pixels, const int size, float *predict_probability)
    {
        auto img = std::make_shared<DataImage>();
        if (!img->Load(pixels, size)) return -1;
        return Predict(img, predict_probability);
    }

    void SoftMaxModel::LoadInputs(const std::string &text)
    {
        std::istringstream is(text);
        LoadInputs(is);
    }

    void SoftMaxModel::LoadInputs(Options *options)
    {
        if (options == nullptr) return;
        auto input_lines = options->GetLines();
        for (auto it = input_lines->begin(); it != input_lines->end(); it++)
        {
            std::istringstream is(*it);
            LoadInputs(is);
        }
    }

    void SoftMaxModel::LoadInputs(std::istream &is)
    {
        std::string line;
        std::string path;
        std::string classify;
        auto dataset = GetDataSet();
        while (is.good() && std::getline(is, line))
        {
            line = string_trim(line);
            if (line.empty()) continue;
            std::istringstream is(line);
            is >> path >> classify;
            if (classify.empty() || path.empty()) continue;
            dataset->insert_or_assign(path, std::make_shared<SoftMaxModelDataItem>(path, classify));
        }
    }

    std::shared_ptr<DataImage> SoftMaxModel::LoadInputImage(const std::string &input_path)
    {
        auto input_image = std::make_shared<DataImage>();
        if (input_image->Load(input_path)) return input_image;
        std::cout << "invalid image path: " << input_path << std::endl;
        return nullptr;
    }

    bool SoftMaxModel::LoadInput(const std::string &img_path, const std::string &classify, float *out_datas, int out_datas_size, float *out_truths, int out_truths_size)
    {
        auto classify_index = GetClassifyIndex(classify);
        if (classify_index < 0)
        {
            std::cout << "invliad classify index: " << classify << std::endl;
            return false;
        }

        std::shared_ptr<DataImage> input_image = nullptr;
        auto it                                = m_input_images.find(img_path);

        // 加载图像
        if (it == m_input_images.end() || it->second.expired())
        {
            input_image = LoadInputImage(img_path);
            if (input_image == nullptr) return false;

            // 处理图像
            input_image = HandleInputImage(input_image);

            // 缓存图像
            m_input_images.insert_or_assign(img_path, input_image);
            m_cache_images.push_back(input_image);
            if (m_cache_images.size() > m_cache_size) m_cache_images.pop_front();
        }
        else
        {
            input_image = it->second.lock();
        }

        // 图像克隆
        input_image = input_image->Clone();

        // 填充数据
        return LoadInput(input_image, classify_index, out_datas, out_datas_size, out_truths, out_truths_size);
    }

    bool SoftMaxModel::LoadInput(std::shared_ptr<DataImage> input_image, int classify_index, float *out_datas, int out_datas_size, float *out_truths, int out_truths_size)
    {
        // 图像抖动
        auto jitter     = GetJitter();
        auto is_augment = IsAugment();
        if (jitter != 0 && is_augment)
        {
            auto ow = input_image->GetWidth();
            auto oh = input_image->GetHeight();

            int dw = ow * jitter;
            int dh = oh * jitter;

            int pleft  = random_int(-dw, dw);
            int pright = random_int(-dw, dw);
            int ptop   = random_int(-dh, dh);
            int pbot   = random_int(-dh, dh);

            int swidth  = ow - pleft - pright;
            int sheight = oh - ptop - pbot;

            input_image = input_image->GetSubImage(pleft, ptop, swidth, sheight);
        }

        // 图像缩放填充至网络输入尺寸
        auto net_width  = GetNetWidth();
        auto net_height = GetNetHeight();
        auto img_width  = input_image->GetWidth();
        auto img_height = input_image->GetHeight();
        if (net_width != img_width || net_height != img_height) input_image = input_image->ScaleFill(net_width, net_height);

        // 翻转图像
        auto is_flip_horizontal = is_augment && (random_int() % 2) == 0;
        auto is_flip_vertical   = is_augment && (random_int() % 2) == 0;
        if (is_flip_horizontal && IsFilpHorizontal())
        {
            input_image->FlipHorizontal();
        }
        if (is_flip_vertical && IsFilpVertical())
        {
            input_image->FlipVertical();
        }

        // 模糊图像
        if (IsDistort() && is_augment) input_image->RandomDistort(m_hue, m_saturation, m_exposure);

        // 填充数据
        memcpy(out_datas, input_image->GetData(), out_datas_size * sizeof(float));
        memset(out_truths, 0, out_truths_size * sizeof(float));
        out_truths[classify_index] = 1;

        return true;
    }
}
