#include "YoloModel.h"
namespace darknet
{
    YoloModel::YoloModel()
    {
    }

    bool YoloModel::Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list)
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

    bool YoloModel::LoadModelFromFile(std::istream &is)
    {
        if (!Model::LoadModelFromFile(is)) return false;
        std::string softmax_classify_index = GetNet()->GetOptions()->GetOption("softmax_classify_index", "");
        if (!softmax_classify_index.empty())
        {
            std::istringstream iss(softmax_classify_index);
            LoadClassifyIndex(iss);
        }

        return true;
    }

    bool YoloModel::SaveModelToFile(std::ostream &os)
    {
        std::ostringstream oss;
        SaveClassifyIndex(oss);
        GetNet()->GetOptions()->SetOption("softmax_classify_index", oss.str());
        return Model::SaveModelToFile(os);
    }

    void YoloModel::Train()
    {
        // 加载数据集
        LoadDataSet();

        // 设置真实值大小
        GetNet()->SetTruthsSize(5);
        GetNet()->SetMaxTruths(10);
        GetNet()->SetBatch(GetBatch());

        // 开始训练
        Model::Train();
    }

    void YoloModel::Test()
    {
        // 加载数据集
        LoadDataSet();

        // 开始测试
        Model::Test();
    }

    void YoloModel::LoadDataSet()
    {
        // 加载数据
        LoadInputs(GetOptions("dataset"));

        // 数据集配置文件
        if (!m_dataset_path.empty())
        {
            std::ifstream ifs(m_dataset_path);
            if (ifs.is_open()) LoadInputs(ifs);
        }

        auto dataset = GetDataSet();
        auto inputs  = GetInputs();

        inputs->clear();
        for (auto it = dataset->begin(); it != dataset->end(); it++)
        {
            inputs->push_back(it->second);

            auto truths      = it->second->ToType<YoloModelDataItem>()->GetTruths();
            auto truths_size = truths->size();
            for (auto i = 0; i < truths_size; i++)
            {
                auto truth = truths->data() + i;
                truth->SetClassifyIndex(CreateGetClassifyIndex(truth->GetClassify()));
            }
        }
    }

    bool YoloModel::LoadNextBatchInput()
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
            auto dataitem = inputs->at(data_index)->ToType<YoloModelDataItem>();

            // 加载采样数据
            if (!LoadInput(dataitem->GetPath(), *(dataitem->GetTruths()), in_datas, in_datas_size, in_truths, in_truths_size))
            {
                std::cout << "invalid input data => " << dataitem->GetPath() << std::endl;
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

    void YoloModel::LoadInputs(const std::string &text)
    {
        std::istringstream is(text);
        LoadInputs(is);
    }

    void YoloModel::LoadInputs(Options *options)
    {
        if (options == nullptr) return;
        auto input_lines = options->GetLines();
        for (auto it = input_lines->begin(); it != input_lines->end(); it++)
        {
            std::istringstream is(*it);
            LoadInputs(is);
        }
    }

    void YoloModel::LoadInputs(std::istream &is)
    {
        std::string line;
        std::string x;
        std::string y;
        std::string width;
        std::string height;
        std::string classify;
        std::string path;
        Truth truth;

        auto dataset = GetDataSet();
        while (is.good() && std::getline(is, line))
        {
            line = string_trim(line);
            if (line.empty()) continue;

            std::istringstream is(line);
            is >> path;
            auto dataitem = std::make_shared<YoloModelDataItem>(path);
            auto truths = dataitem->GetTruths();
            while (is.good())
            {
                is >> x >> y >> width >> height >> classify >> path;
                if (x.empty() || y.empty() || width.empty() || height.empty() || classify.empty()) break;
                truth.SetX(std::stof(x));
                truth.SetY(std::stof(y));
                truth.SetWidth(std::stof(width));
                truth.SetHeight(std::stof(height));
                truth.SetClassify(classify);

                if (truth.GetX() <= 0.f || truth.GetY() <= 0.f || truth.GetWidth() <= 0.f || truth.GetHeight() <= 0.f) break;
                truths->push_back(truth);
            }

            if (path.empty() || truths->empty())
            {
                std::cout << "invalid data item => " << line << std::endl;
                continue;
            }

            dataset->insert_or_assign(path, dataitem);
        }
    }

    bool YoloModel::LoadInput(const std::string &img_path, std::vector<YoloModel::Truth> truths, float *out_datas, int out_datas_size, float *out_truths, int out_truths_size)
    {
        std::shared_ptr<DataImage> input_image = nullptr;
        auto it                                = m_input_images.find(img_path);

        // 加载图像
        if (it == m_input_images.end() || it->second.expired())
        {
            input_image = std::make_shared<DataImage>();
            if (!input_image->Load(img_path))
            {
                std::cout << "invalid image path: " << img_path << std::endl;
                return false;
            }

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

        // 图像抖动
        auto jitter      = GetJitter();
        auto is_augment  = IsAugment();
        auto truths_size = truths.size();
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

            for (int i = 0; i < truths_size; i++)
            {
                auto &truth = truths[i];
                truth.SetX(std::max(0.f, (truth.GetX() * ow - pleft) / swidth));
                truth.SetY(std::max(0.f, (truth.GetY() * oh - ptop) / sheight));
                truth.SetWidth(std::max(0.f, (truth.GetWidth() * ow) / swidth));
                truth.SetHeight(std::max(0.f, (truth.GetHeight() * oh) / sheight));
            }
        }

        // 图像缩放填充至网络输入尺寸
        auto net_width  = GetNetWidth();
        auto net_height = GetNetHeight();
        auto img_width  = input_image->GetWidth();
        auto img_height = input_image->GetHeight();
        input_image     = input_image->ScaleFill(net_width, net_height);

        for (int i = 0; i < truths_size; i++)
        {
            auto &truth   = truths[i];
            float boxs[4] = {truth.GetX(), truth.GetY(), truth.GetWidth(), truth.GetHeight()};
            ImageBoxToNetBox(boxs, boxs, img_width, img_height, net_width, net_height);
            truth.SetX(boxs[0]);
            truth.SetY(boxs[1]);
            truth.SetWidth(boxs[2]);
            truth.SetHeight(boxs[3]);
        }

        // 翻转图像
        auto is_flip_horizontal = is_augment && (random_int() % 2) == 0;
        auto is_flip_vertical   = is_augment && (random_int() % 2) == 0;
        if (is_flip_horizontal && IsFilpHorizontal())
        {
            input_image->FlipHorizontal();
            for (int i = 0; i < truths_size; i++)
            {
                auto &truth = truths[i];
                truth.SetX(1.0f - truth.GetX());
            }
        }
        if (is_flip_vertical && IsFilpVertical())
        {
            input_image->FlipVertical();
            for (int i = 0; i < truths_size; i++)
            {
                auto &truth = truths[i];
                truth.SetY(1.0f - truth.GetY());
            }
        }

        // 模糊图像
        if (IsDistort() && is_augment) input_image->RandomDistort(m_hue, m_saturation, m_exposure);

        // 填充数据
        memcpy(out_datas, input_image->GetData(), out_datas_size * sizeof(float));
        for (int i = 0; i < truths_size; i++)
        {
            auto &truth   = truths[i];
            *out_truths++ = truth.GetX();
            *out_truths++ = truth.GetY();
            *out_truths++ = truth.GetWidth();
            *out_truths++ = truth.GetHeight();
            *out_truths++ = truth.GetClassifyIndex();
        }
        return true;
    }
}