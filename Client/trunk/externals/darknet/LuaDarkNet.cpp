﻿#include "LuaDarkNet.h"

#if defined(_WIN32)
#define EXPORT_API __declspec(dllexport)
#define IMPORT_API __declspec(dllimport)
#elif defined(__linux__) || defined(__APPLE__)
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define EXPORT_API __attribute__((visibility("default")))
#else
#define EXPORT_API
#endif
#else
#pragma error "Unsupported platform or compiler!"
#endif

// 初始化并打开库
extern "C" EXPORT_API int luaopen_LuaDarkNet(lua_State *L)
{
    LuaDarkNet::Register(L);
    return 0;
}

namespace LuaDarkNet
{
    void Register(lua_State *L)
    {
        luabridge::getGlobalNamespace(L)
            .beginNamespace("LuaDarkNet")
            .beginClass<LuaWakeWordModel>("LuaWakeWordModel")
            .addConstructor<void (*)(void)>()
            .addFunction("LoadModelFromFile", &LuaWakeWordModel::LoadModelFromFile)
            .addFunction("Predict", &LuaWakeWordModel::Predict)
            .endClass()
            .endNamespace();
    }
}