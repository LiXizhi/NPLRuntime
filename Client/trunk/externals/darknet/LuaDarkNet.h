

#ifndef __LUA_DARKNET_H__
#define __LUA_DARKNET_H__
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/Map.h"
#include "LuaBridge/UnorderedMap.h"
#include "LuaBridge/Vector.h"

#include "DarkNet.h"
#include "apps/WakeWordModel.h"

namespace LuaDarkNet
{
    class LuaWakeWordModel
    {
    public:
        bool LoadModelFromFile(const std::string &model_path) { return GetModel()->LoadModelFromFile(model_path); }

        std::string Predict(const std::string &data)
        {
            float probability = 0;
            auto classify_index = GetModel()->Predict((short *)data.data(), data.size() / sizeof(short), &probability);
            if (probability < 0.8)
                return "none";
            return GetModel()->GetIndexClassify(classify_index);
        }

    public:
        inline WakeWordModel *GetModel() { return &m_model; }

    protected:
        WakeWordModel m_model;
    };

    // LuaDarkNet() { m_trainer = std::make_shared<Trainer>(); }

    // bool LoadModelFromFile(const std::string &model_path)
    // {
    //     return m_trainer->LoadModelFromFile(model_path);
    // }

    // bool LoadFromIniFile(const std::string &ini_path)
    // {
    //     return m_trainer->LoadFromIniFile(ini_path);
    // }

    // bool LoadFromIniText(const std::string &ini_text)
    // {
    //     return m_trainer->LoadFromIniText(ini_text);
    // }

    // bool LoadConfig(const std::string &ini_path, bool is_ini_path)
    // {
    //     if (is_ini_path)
    //     {
    //         return LoadFromIniFile(ini_path);
    //     }
    //     else
    //     {
    //         return LoadFromIniText(ini_path);
    //     }
    // }

    // bool Load(const std::string &model_path)
    // {
    //     return LoadModelFromFile(model_path);
    // }

    // bool Save(const std::string &model_path)
    // {
    //     return m_trainer->SaveModelToFile(model_path);
    // }

    // void Train(const std::string &ini_path, bool is_ini_path)
    // {
    //     if (is_ini_path)
    //     {
    //         m_trainer->LoadFromIniFile(ini_path);
    //     }
    //     else
    //     {
    //         m_trainer->LoadFromIniText(ini_path);
    //     }
    //     m_trainer->Train();
    // }

    // void Test(const std::string &ini_path, bool is_ini_path)
    // {
    //     if (is_ini_path)
    //     {
    //         m_trainer->LoadFromIniFile(ini_path);
    //     }
    //     else
    //     {
    //         m_trainer->LoadFromIniText(ini_path);
    //     }
    //     m_trainer->Train();
    // }

    // std::vector<float> Predict(const std::string &img_path, bool is_img_path)
    // {
    //     if (is_img_path)
    //     {
    //         return *m_trainer->Predict(img_path);
    //     }
    //     else
    //     {
    //         return *m_trainer->Predict((const unsigned char *)(img_path.data()), img_path.size());
    //     }
    // }

    // std::vector<float> PredictSubImage(const std::string &img_path, bool is_img_path, int x, int y, int w, int h)
    // {
    //     auto img = std::make_shared<Image>();
    //     if (is_img_path)
    //     {
    //         if (!img->Load(img_path))
    //             return std::vector<float>();
    //     }
    //     else
    //     {
    //         if (!img->Load((const unsigned char *)(img_path.data()), img_path.size()))
    //             return std::vector<float>();
    //     }
    //     img = img->GetSubImage(x, y, w, h);
    //     img = img->ScaleFill(m_trainer->GetNet()->GetWidth(), m_trainer->GetNet()->GetHeight());
    //     return *m_trainer->Predict(img->GetData(), img->GetWidth(), img->GetHeight());
    // }

    // double DoEpoch()
    // {
    //     return m_trainer->DoEpoch();
    // }

    static void Register(lua_State *L);
};
#endif