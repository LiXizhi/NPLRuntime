#include "ColorModel.h"
namespace darknet
{
    ColorModel::ColorModel()
    {
        m_batch           = 128;
        m_max_epochs      = 10000;
        m_auto_stop_train = false;
        m_model_name      = "colors";
        m_enable_rgb565   = false;

        CreateGetClassifyIndex(R"(黑色)");
        CreateGetClassifyIndex(R"(白色)");
        CreateGetClassifyIndex(R"(红色)");
        CreateGetClassifyIndex(R"(绿色)");
        CreateGetClassifyIndex(R"(蓝色)");
        CreateGetClassifyIndex(R"(橙色)");
        CreateGetClassifyIndex(R"(黄色)");
        CreateGetClassifyIndex(R"(青色)");
        CreateGetClassifyIndex(R"(紫色)");
    }

    bool ColorModel::Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list)
    {
        auto ok      = SoftMaxModel::Load(options_list);
        auto options = GetOptions("model");
        if (options != nullptr)
        {
            m_dataset_colors_path = options->GetOption("dataset_colors", m_dataset_colors_path.c_str());
        }
        return ok;
    }

    void ColorModel::LoadNet()
    {
        SoftMaxModel::LoadNet();
        if (GetNet()->IsLoaded()) return;

        auto net     = GetNet();
        auto options = std::make_shared<std::vector<std::shared_ptr<Options>>>();
        auto option  = std::make_shared<Options>("net");
        option->SetOption("width", 1);
        option->SetOption("height", 1);
        option->SetOption("channels", 3);
        options->push_back(option);

        option = std::make_shared<Options>("connected");
        option->SetOption("output", 32);
        option->SetOption("activation", "leaky");
        options->push_back(option);

        option = std::make_shared<Options>("connected");
        option->SetOption("output", 64);
        option->SetOption("activation", "leaky");
        options->push_back(option);

        option = std::make_shared<Options>("connected");
        option->SetOption("output", 32);
        option->SetOption("activation", "leaky");
        options->push_back(option);

        option = std::make_shared<Options>("connected");
        option->SetOption("output", 7);
        option->SetOption("activation", "linear");
        options->push_back(option);

        option = std::make_shared<Options>("softmax");
        options->push_back(option);

        net->Load(options);
    }

    void ColorModel::LoadDataSet()
    {
        SoftMaxModel::LoadDataSet();

        auto dataset = SoftMaxModel::GetDataSet();
        for (auto it = dataset->begin(); it != dataset->end(); it++)
        {
            auto path     = it->first;
            auto classify = it->second->GetClassify();
            auto img      = std::make_shared<DataImage>();
            if (!img->Load(path))
            {
                std::cout << "invalid data item => " << path << std::endl;
                continue;
            }

            auto classify_index = CreateGetClassifyIndex(classify);

            auto img_width    = img->GetWidth();
            auto img_height   = img->GetHeight();
            auto img_channels = img->GetChannel();
            auto img_datas    = img->GetData();
            for (int j = 0; j < img_height; j++)
            {
                for (int i = 0; i < img_width; i++)
                {
                    auto r           = img->GetPixel(i, j, 0) * 0xff;
                    auto g           = img->GetPixel(i, j, 1) * 0xff;
                    auto b           = img->GetPixel(i, j, 2) * 0xff;
                    auto color_index = GetColorIndex(r, g, b);
                    auto it          = m_dataset.find(color_index);
                    if (it == m_dataset.end())
                    {
                        m_dataset[color_index] = classify_index;
                        m_inputs.push_back(color_index);
                    }
                    else
                    {
                        if (it->second != classify_index)
                        {
                            // std::cout << "color index " << color_index << " classify index " << classify_index << " is not equal " << it->second << std::endl;
                            m_dataset[color_index] = classify_index;
                        }
                    }
                    if (m_enable_rgb565)
                    {
                        m_rgb565_dataset.insert_or_assign(RGB888ToRGB565(r, g, b), classify_index);
                    }
                }
            }
        }

        auto colors_path = GetDataSetColorsPath();
        std::ifstream ifs(colors_path);
        if (ifs.good())
        {
            std::string line;
            while (std::getline(ifs, line))
            {
                std::stringstream ss(line);
                int color_index = 0;
                ss >> color_index;
                std::string classify;
                ss >> classify;
                int classify_index = CreateGetClassifyIndex(classify);
                if (m_dataset.find(color_index) == m_dataset.end()) m_inputs.push_back(color_index);
                m_dataset[color_index] = classify_index;
            }
        }
    }

    void ColorModel::SaveDataSet()
    {
        auto colors_path = GetDataSetColorsPath();
        std::ofstream ofs(colors_path);
        int size = m_dataset.size();
        for (auto it = m_dataset.begin(); it != m_dataset.end(); it++)
        {
            ofs << it->first << " " << GetIndexClassify(it->second) << std::endl;
        }
    }

    bool ColorModel::LoadNextBatchInput()
    {
        auto net            = GetNet();
        auto batch          = GetBatch();
        auto in_datas_size  = GetNetInDatasSize();
        auto in_truths_size = GetNetInTruthsSize();

        assert(in_datas_size == 3);

        // 分配内存
        SetInDatasSize(in_datas_size * batch);
        SetInTruthsSize(in_truths_size * batch);

        auto in_datas  = GetInDatas();
        auto in_truths = GetInTruths();
        auto size      = m_inputs.size();
        for (int i = 0; i < batch; i++)
        {
            int index           = random_int(0, size - 1);
            auto color_index    = m_inputs[index];
            auto classify_index = m_dataset[color_index];
            for (int j = 0; j < in_datas_size; j++)
            {
                in_datas[i * in_datas_size + j] = *(((unsigned char *)(&color_index)) + j) / 255.0f;
            }
            for (int j = 0; j < in_truths_size; j++)
            {
                in_truths[i * in_truths_size + j] = j == classify_index ? 1.0f : 0.0f;
            }
        }

        return true;
    }

    int ColorModel::Predict(unsigned char r, unsigned char g, unsigned char b)
    {
        auto color_index = GetColorIndex(r, g, b);
        auto dataset     = GetDataSet();
        auto it          = dataset->find(color_index);
        if (it != dataset->end()) return it->second;

        if (m_enable_rgb565)
        {
            auto rgb565_dataset = GetRGB565DataSet();
            auto it             = rgb565_dataset->find(color_index);
            if (it != rgb565_dataset->end()) return it->second;
        }

        float in_datas[3] = {r / 255.0f, g / 255.0f, b / 255.0f};
        return SoftMaxModel::Predict(in_datas, nullptr);
    }
}