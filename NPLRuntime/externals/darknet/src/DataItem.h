#ifndef __DATA_ITEM_H__
#define __DATA_ITEM_H__

#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Image.h"
#include "Truth.h"

class DataItem
{
public:
    DataItem() : m_loaded(false), m_image(nullptr) {}

    virtual std::shared_ptr<DataItem> Clone()
    {
        auto clone = MakeShared();
        CopyTo(clone.get());
        return clone;
    }

    virtual bool Load()
    {
        if (m_loaded) return true;
        if (!LoadDatas(m_in_datas)) return false;
        if (!LoadTruths(m_in_truths)) return false;
        m_loaded = true;
        return true;
    }

    virtual void Unload()
    {
        m_loaded = false;
        m_datas.clear();
        m_datas.shrink_to_fit();
        m_truths.clear();
        m_truths.shrink_to_fit();
        m_image = nullptr;
    }

    virtual bool LoadDatas(const std::string &datas)
    {
        int datas_size = datas.size();
        if (datas_size > 4)
        {
            auto suffix = datas.substr(datas_size - 4, 4);
            if (suffix == ".jpg" || suffix == ".png")
            {
                auto image = std::make_shared<Image>();
                if (!image->Load(datas)) return false;
                m_image = image;
                return true;
            }
        }

        std::istringstream iss(datas);
        float data = 0.f;
        while (iss.good())
        {
            iss >> data;
            m_datas.push_back(data);
        }

        return m_datas.size() > 0;
    }

    virtual bool LoadTruths(const std::string &truths)
    {
        std::istringstream iss(truths);
        while (iss.good())
        {
            std::shared_ptr<Truth> truth = m_truth_creator != nullptr ? m_truth_creator() : std::make_shared<Truth>();
            if (!truth->Load(iss)) return false;
            m_truths.push_back(truth);
        }

        return m_truths.size() > 0;
    }

    virtual void FillDatas(float *out_datas, int out_datas_size)
    {
        if (m_image != nullptr)
        {
            memcpy(out_datas, m_image->GetData(), out_datas_size);
        }
        else
        {
            memcpy(out_datas, m_datas.data(), out_datas_size);
        }
    }

    virtual void FillTruths(float *out_truths, int out_truths_size)
    {
        auto truths_size = m_truths.size();
        for (int i = 0; i < truths_size; i++)
        {
            auto datas = m_truths[i]->GetDatas();
            auto size  = m_truths[i]->GetSize();
            if (size <= out_truths_size)
            {
                for (int j = 0; j < size; j++)
                {
                    *out_truths++ = *datas++;
                }
                out_truths_size -= size;
            }
        }
    }

protected:
    virtual std::shared_ptr<DataItem> MakeShared() { return std::make_shared<DataItem>(); }

public:
    virtual void SetImage(std::shared_ptr<Image> image) { m_image = image; }
    virtual std::shared_ptr<Image> GetImage() { return m_image; }
    inline void SetInDatas(const std::string &in_datas) { m_in_datas = in_datas; }
    inline const std::string &GetInDatas() { return m_in_datas; }
    inline void SetInTruths(const std::string &in_truths) { m_in_truths = in_truths; }
    inline const std::string &GetInTruths() { return m_in_truths; }
    inline std::vector<std::shared_ptr<Truth>> *GetTruths() { return &m_truths; }
    inline void SetTruthCreator(std::function<std::shared_ptr<Truth>()> truth_creator) { m_truth_creator = truth_creator; }

protected:
    void CopyTo(DataItem *data_item)
    {
        data_item->m_in_datas  = m_in_datas;
        data_item->m_in_truths = m_in_truths;
        data_item->m_datas     = m_datas;
        data_item->m_image     = m_image == nullptr ? nullptr : m_image->Clone();
        int truths_size        = m_truths.size();
        for (int i = 0; i < truths_size; i++)
        {
            data_item->m_truths.push_back(m_truths[i]->Clone());
        }

        data_item->m_ofs = m_ofs;
    }

protected:
    bool m_loaded;
    std::string m_in_datas;
    std::string m_in_truths;
    std::vector<float> m_datas;
    std::shared_ptr<Image> m_image;
    std::vector<std::shared_ptr<Truth>> m_truths;
    std::function<std::shared_ptr<Truth>()> m_truth_creator;

    // debug use
public:
    inline void SetOStream(std::shared_ptr<std::ofstream> ofs) { m_ofs = ofs; }
    inline std::ofstream *GetOStream() { return (m_ofs != nullptr && m_ofs->good()) ? m_ofs.get() : nullptr; }

protected:
    std::shared_ptr<std::ofstream> m_ofs;
};

#endif