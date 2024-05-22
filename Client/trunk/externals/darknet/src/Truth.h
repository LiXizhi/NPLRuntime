#ifndef __TRUTH_H__
#define __TRUTH_H__

#include <fstream>
#include <memory>
#include <string>
#include <vector>

class Truth
{
public:
    Truth(int datas_size = 0) : m_probability(0.f) { m_datas.resize(datas_size); }
    virtual std::shared_ptr<Truth> Clone()
    {
        auto clone           = MakeShared();
        clone->m_datas       = m_datas;
        clone->m_probability = m_probability;
        return clone;
    }

    virtual bool Load(std::istream& is)
    {
        int size = m_datas.size();
        for (int i = 0; i < size; i++)
        {
            if (is.good())
            {
                is >> m_datas[i];
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    virtual void SetClassify(int classify) {}
    virtual int GetClassify() { return 0; }
    virtual void SetX(float x) {}
    virtual float GetX() { return 0.f; }
    virtual void SetY(float y) {}
    virtual float GetY() { return 0.f; }
    virtual void SetWidth(float width) {}
    virtual float GetWidth() { return 0.f; }
    virtual void SetHeight(float height) {}
    virtual float GetHeight() { return 0.f; }

protected:
    virtual std::shared_ptr<Truth> MakeShared() { return std::make_shared<Truth>(); }

public:
    inline float *GetDatas() { return m_datas.data(); }
    inline int GetSize() { return m_datas.size(); }
    inline void SetProbability(float probability) { m_probability = probability; }
    inline float GetProbability() { return m_probability; }

protected:
    std::vector<float> m_datas;
    float m_probability;
};

class YoloTruth : public Truth
{
public:
    YoloTruth() : Truth(5) {}

    virtual void SetX(float x) { m_datas[0] = x; }
    virtual float GetX() { return m_datas[0]; }
    virtual void SetY(float y) { m_datas[1] = y; }
    virtual float GetY() { return m_datas[1]; }
    virtual void SetWidth(float width) { m_datas[2] = width; }
    virtual float GetWidth() { return m_datas[2]; }
    virtual void SetHeight(float height) { m_datas[3] = height; }
    virtual float GetHeight() { return m_datas[3]; }
    virtual void SetClassify(int classify) { m_datas[4] = classify; }
    virtual int GetClassify() { return m_datas[4]; }

protected:
    virtual std::shared_ptr<Truth> MakeShared() { return std::make_shared<YoloTruth>(); }
};

#endif