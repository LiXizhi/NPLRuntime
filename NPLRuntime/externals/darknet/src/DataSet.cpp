#include "DataSet.h"
#include "Layer.h"
#include "Net.h"
#include "Options.h"
#include "utils.h"

DataSet::DataSet()
{
    m_width           = 0;
    m_height          = 0;
    m_channels        = 0;
    m_max_truths_size = 0;
    m_input_index     = 0;
    m_debug           = false;
    m_random          = false;
    m_augment         = false;
    m_jitter          = 0.f;
    m_filp_horizontal = false;
    m_filp_vertical   = false;
    m_distort         = false;
    m_saturation      = 0.f;
    m_exposure        = 0.f;
    m_hue             = 0.f;

    m_cache_data_items.resize(32, nullptr);
}

std::shared_ptr<DataSet::BatchInput> DataSet::GetNextBatchInput(int batch)
{
    auto input_item_index = 0;
    auto input_index      = GetInputIndex();
    auto data_items_size  = GetDataItems()->size();
    auto datas_size       = GetDatasSize();
    auto truths_size      = GetMaxTruthsSize();
    auto batch_input      = std::make_shared<BatchInput>(datas_size * batch, truths_size * batch);
    auto input_data       = batch_input->GetDatas();
    auto truth_data       = batch_input->GetTruths();
    auto is_radom         = IsRandom();

    std::unordered_set<int> invalid_data_index_set;
    while (input_item_index < batch && invalid_data_index_set.size() < data_items_size)
    {
        auto data_item_index = is_radom ? random_int(0, data_items_size - 1) : (input_index++ % data_items_size);
        if (invalid_data_index_set.find(data_item_index) != invalid_data_index_set.end()) continue;

        // 加载数据项
        auto data_item = LoadDataItem(data_item_index);
        if (data_item == nullptr)
        {
            std::cerr << "无效数据项: " << data_item_index << std::endl;
            invalid_data_index_set.insert(data_item_index);
            continue;
        }

        // 增强数据项
        if (data_item->GetImage() != nullptr) data_item = AugmentImageDataItem(data_item);

        // 输出调试信息
        auto os = data_item->GetOStream();
        if (os != nullptr)
        {
            *os << std::endl;
            *os << "input index: " << (GetInputIndex() + input_item_index) << std::endl;
        }

        // 拷贝图像数据到输入对象
        data_item->FillDatas(input_data, datas_size);
        data_item->FillTruths(truth_data, truths_size);

        batch_input->GetDataItems()->push_back(data_item);
        input_data += datas_size;
        truth_data += truths_size;
        input_item_index++;
    }
    SetInputIndex(GetInputIndex() + batch);
    return batch == input_item_index ? batch_input : nullptr;
}

std::shared_ptr<DataItem> DataSet::LoadDataItem(int data_item_index)
{
    auto data_item = m_data_items[data_item_index];
    if (!data_item->Load()) return nullptr;
    if (m_cache_data_items.end() == std::find(m_cache_data_items.begin(), m_cache_data_items.end(), data_item))
    {
        auto cache_data_item = m_cache_data_items.front();
        if (cache_data_item != nullptr) cache_data_item->Unload();
        m_cache_data_items.push_back(data_item);
        m_cache_data_items.pop_front();
    }

    // 打开调试输出
    if (IsDebug() && data_item->GetOStream() == nullptr)
    {
        data_item->SetOStream(std::make_shared<std::ofstream>(std::filesystem::path(m_debugs_path) / (std::to_string(data_item_index) + ".txt")));
        auto ofs = data_item->GetOStream();
        if (ofs != nullptr)
        {
            *ofs << "in_datas: " << data_item->GetInDatas() << std::endl;
            *ofs << "in_truths: " << data_item->GetInTruths() << std::endl;
        }
    }
    return data_item;
}

std::shared_ptr<DataItem> DataSet::AugmentImageDataItem(std::shared_ptr<DataItem> in_data_item)
{
    auto data_item   = in_data_item->Clone();
    auto truths      = data_item->GetTruths();
    auto truths_size = truths->size();
    auto img         = data_item->GetImage();
    auto is_augment  = IsAugment();

    // jitter
    if (m_jitter != 0 && is_augment)
    {
        auto ow = img->GetWidth();
        auto oh = img->GetHeight();

        int dw = ow * m_jitter;
        int dh = oh * m_jitter;

        int pleft  = random_int(-dw, dw);
        int pright = random_int(-dw, dw);
        int ptop   = random_int(-dh, dh);
        int pbot   = random_int(-dh, dh);

        int swidth  = ow - pleft - pright;
        int sheight = oh - ptop - pbot;

        img = img->GetSubImage(pleft, ptop, swidth, sheight);

        for (int i = 0; i < truths_size; i++)
        {
            auto truth = truths->at(i);
            truth->SetX(std::max(0.f, (truth->GetX() * ow - pleft) / swidth));
            truth->SetY(std::max(0.f, (truth->GetY() * oh - ptop) / sheight));
            truth->SetWidth(std::max(0.f, (truth->GetWidth() * ow) / swidth));
            truth->SetHeight(std::max(0.f, (truth->GetHeight() * oh) / sheight));
        }
    }

    // resize
    auto net_width  = GetWidth();
    auto net_height = GetHeight();
    auto img_width  = img->GetWidth();
    auto img_height = img->GetHeight();
    img             = img->ScaleFill(net_width, net_height);
    assert(img->GetWidth() == net_width && img->GetHeight() == net_height);

    for (int i = 0; i < truths_size; i++)
    {
        auto truth    = truths->at(i);
        float boxs[4] = {truth->GetX(), truth->GetY(), truth->GetWidth(), truth->GetHeight()};
        ImageBoxToNetBox(boxs, boxs, img_width, img_height, net_width, net_height);
        truth->SetX(boxs[0]);
        truth->SetY(boxs[1]);
        truth->SetWidth(boxs[2]);
        truth->SetHeight(boxs[3]);
    }

    // flip
    auto is_flip_horizontal = (random_int() % 2) == 0;
    auto is_flip_vertical   = (random_int() % 2) == 0;
    if (m_filp_horizontal && is_flip_horizontal && is_augment)
    {
        img->FlipHorizontal();
        for (int i = 0; i < truths_size; i++)
        {
            auto truth = truths->at(i);
            truth->SetX(1.0f - truth->GetX());
        }
    }
    if (m_filp_vertical && is_flip_vertical && is_augment)
    {
        img->FlipVertical();
        for (int i = 0; i < truths_size; i++)
        {
            auto truth = truths->at(i);
            truth->SetY(1.0f - truth->GetY());
        }
    }

    // distort
    if (m_distort && is_augment) img->RandomDistort(m_hue, m_saturation, m_exposure);

    // 设置增强后的图像
    data_item->SetImage(img);
    return data_item;
}

float DataSet::GetNetImageScale(int img_width, int img_height, int net_width, int net_height, int *offset_x, int *offset_y)
{
    float scale_x = net_width * 1.0f / img_width;
    float scale_y = net_height * 1.0f / img_height;
    float scale   = scale_x > scale_y ? scale_y : scale_x;

    if (offset_x != nullptr) *offset_x = (net_width - scale * img_width);
    if (offset_y != nullptr) *offset_y = (net_height - scale * img_height);

    return scale;
}

void DataSet::ImageBoxToNetBox(float *img_box, float *net_box, int img_width, int img_height, int net_width, int net_height)
{
    int offset_x = 0;
    int offset_y = 0;
    float scale  = GetNetImageScale(img_width, img_height, net_width, net_height, &offset_x, &offset_y);
    net_box[0]   = (img_box[0] * img_width * scale + offset_x * 0.5f) / net_width;
    net_box[1]   = (img_box[1] * img_height * scale + offset_y * 0.5f) / net_height;
    net_box[2]   = (img_box[2] * img_width * scale) / net_width;
    net_box[3]   = (img_box[3] * img_height * scale) / net_height;
}

void DataSet::NetBoxToImageBox(float *net_box, float *img_box, int img_width, int img_height, int net_width, int net_height)
{
    int offset_x = 0;
    int offset_y = 0;
    float scale  = GetNetImageScale(img_width, img_height, net_width, net_height, &offset_x, &offset_y);
    img_box[0]   = (net_box[0] * net_width - offset_x * 0.5f) / scale / img_width;
    img_box[1]   = (net_box[1] * net_height - offset_y * 0.5f) / scale / img_height;
    img_box[2]   = net_box[2] * net_width / scale / img_width;
    img_box[3]   = net_box[3] * net_height / scale / img_height;
}

std::shared_ptr<Image> DataSet::LoadNetImage(const std::string &in_img_path, int in_net_width, int in_net_height, int *out_img_width, int *out_img_height)
{
    auto img = std::make_shared<Image>();
    if (!img->Load(in_img_path))
    {
        std::cerr << "文件路径无效: " << in_img_path << std::endl;
        return nullptr;
    }

    auto img_width  = img->GetWidth();
    auto img_height = img->GetHeight();

    if (out_img_width != nullptr) *out_img_width = img_width;
    if (out_img_height != nullptr) *out_img_height = img_height;

    if (in_net_width == img_width && in_net_height == img_height) return img;

    float scale           = GetNetImageScale(img_width, img_height, in_net_width, in_net_height, nullptr, nullptr);
    auto scale_img        = img->Scale(scale, scale);
    auto scale_img_width  = scale_img->GetWidth();
    auto scale_img_height = scale_img->GetHeight();

    if (in_net_width == scale_img_width && in_net_height == scale_img_height) return scale_img;

    return scale_img->Fill(in_net_width, in_net_height);
}

std::shared_ptr<Image> DataSet::LoadNetImage(const unsigned char *in_datas, const int in_size, int in_net_width, int in_net_height, int *out_img_width, int *out_img_height)
{
    auto img = std::make_shared<Image>();
    if (!img->Load(in_datas, in_size)) return nullptr;

    auto img_width  = img->GetWidth();
    auto img_height = img->GetHeight();

    if (out_img_width != nullptr) *out_img_width = img_width;
    if (out_img_height != nullptr) *out_img_height = img_height;

    if (in_net_width == img_width && in_net_height == img_height) return img;

    float scale           = GetNetImageScale(img_width, img_height, in_net_width, in_net_height, nullptr, nullptr);
    auto scale_img        = img->Scale(scale, scale);
    auto scale_img_width  = scale_img->GetWidth();
    auto scale_img_height = scale_img->GetHeight();

    if (in_net_width == scale_img_width && in_net_height == scale_img_height) return scale_img;

    return scale_img->Fill(in_net_width, in_net_height);
}

void DataSet::LoadLines(std::istream &in_is, std::vector<std::string> *out_lines, std::function<void(std::string &)> line_handler)
{
    if (line_handler == nullptr)
    {
        line_handler = [](std::string &line)
        {
            // clang-format off
            line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char c) -> bool { return std::isspace(c); }), line.end());
            // clang-format on
        };
    }

    std::string line;
    while (in_is.good() && std::getline(in_is, line))
    {
        line_handler(line);
        if (line.empty() || line.front() == '#' || line.front() == ';') continue;
        out_lines->push_back(line);
    }
}

void DataSet::Load(std::istream &is)
{
    std::vector<std::string> lines;

    // clang-format off
    LoadLines(is, &lines, [](std::string &line) {
        auto begin = line.begin();
        auto end = line.end() -1;
        while (begin <= end)
        {
            if (std::isspace(*begin)) begin++;
            else if (std::isspace(*end)) end--;
            else break;
        } 
        line = begin <= end ? std::string(begin, end + 1) : std::string("");
    });
    // clang-format on

    int size = lines.size() / 2;
    for (int i = 0; i < size; i++)
    {
        auto data_item = m_dataitem_creator == nullptr ? std::make_shared<DataItem>() : m_dataitem_creator();
        data_item->SetInDatas(lines[2 * i]);
        data_item->SetInTruths(lines[2 * i + 1]);
        data_item->SetTruthCreator(m_truth_creator);
        m_data_items.push_back(data_item);
    }
}
