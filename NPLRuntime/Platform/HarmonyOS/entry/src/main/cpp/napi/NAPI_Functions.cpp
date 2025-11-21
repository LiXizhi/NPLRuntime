/**
 * NAPI Functions Implementation
 * 提供给ArkTS调用的Native接口实现
 */

#include "NAPI_Functions.h"
#include "NAPIHelper.h"
#include "ParaEngineHelper.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "ParaEngine/Core/ParaEngine.h"
#include "ParaEngineSettings.h"
#include "LuaNapiBridge.h"
#include "../AppDelegate.h"
#include <hilog/log.h>
#include <rawfile/raw_file_manager.h>
#include <native_window/external_window.h>

// 重新定义LOG_TAG以避免冲突
#undef LOG_TAG
#define LOG_TAG "ParaEngine"

namespace ParaEngine {

// 基础引擎接口
napi_value NAPI_SetSurface(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createUndefined(env);
    }
    
    OH_LOG_INFO(LOG_APP, "NAPI_SetSurface called");
    
    // TODO: 设置渲染表面
    // ParaEngineHelper::setSurface(surface);
    
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_SetAssetManager(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createUndefined(env);
    }
    
    OH_LOG_INFO(LOG_APP, "NAPI_SetAssetManager called");
    
    // TODO: 设置资源管理器
    // ParaEngineHelper::setAssetManager(assetManager);
    
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_InitParaEngine(napi_env env, napi_callback_info info) {
    OH_LOG_INFO(LOG_APP, "NAPI_InitParaEngine called");
    
    try {
        // 1. 初始化ParaEngineHelper
        ParaEngineHelper* helper = ParaEngineHelper::getInstance();
        if (!helper->init(env, nullptr)) {
            OH_LOG_ERROR(LOG_APP, "Failed to initialize ParaEngineHelper");
            return NAPIHelper::createUndefined(env);
        }
        
        // 2. 获取AppDelegate实例并初始化
        AppDelegate* appDelegate = AppDelegate::getInstance();
        if (!appDelegate->applicationDidFinishLaunching()) {
            OH_LOG_ERROR(LOG_APP, "Failed to initialize AppDelegate");
            return NAPIHelper::createUndefined(env);
        }
        
        OH_LOG_INFO(LOG_APP, "ParaEngine initialized successfully");
        return NAPIHelper::createUndefined(env);
        
    } catch (const std::exception& e) {
        OH_LOG_ERROR(LOG_APP, "Exception during ParaEngine initialization: %s", e.what());
        return NAPIHelper::createUndefined(env);
    } catch (...) {
        OH_LOG_ERROR(LOG_APP, "Unknown exception during ParaEngine initialization");
        return NAPIHelper::createUndefined(env);
    }
}

napi_value NAPI_CleanupParaEngine(napi_env env, napi_callback_info info) {
    OH_LOG_INFO(LOG_APP, "NAPI_CleanupParaEngine called");
    
    // TODO: 清理ParaEngine
    // ParaEngineHelper::cleanupParaEngine();
    
    return NAPIHelper::createUndefined(env);
}

// 渲染生命周期接口
napi_value NAPI_OnSurfaceCreated(napi_env env, napi_callback_info info) {
    OH_LOG_INFO(LOG_APP, "NAPI_OnSurfaceCreated called");
    
    // TODO: 处理渲染表面创建
    // ParaEngineHelper::onSurfaceCreated();
    
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_OnSurfaceChanged(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createUndefined(env);
    }
    
    int32_t width = NAPIHelper::getInt32(env, args[0]);
    int32_t height = NAPIHelper::getInt32(env, args[1]);
    
    OH_LOG_INFO(LOG_APP, "NAPI_OnSurfaceChanged called: %d x %d", width, height);
    
    // TODO: 处理渲染表面大小变化
    // ParaEngineHelper::onSurfaceChanged(width, height);
    
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_OnDrawFrame(napi_env env, napi_callback_info info) {
    // TODO: 处理帧绘制
    // ParaEngineHelper::onDrawFrame();
    
    return NAPIHelper::createUndefined(env);
}

// 输入事件接口
napi_value NAPI_OnTouch(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createUndefined(env);
    }
    
    int32_t action = NAPIHelper::getInt32(env, args[0]);
    // TODO: 处理触摸事件
    
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_OnKey(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createUndefined(env);
    }
    
    int32_t action = NAPIHelper::getInt32(env, args[0]);
    int32_t keyCode = NAPIHelper::getInt32(env, args[1]);
    
    return NAPIHelper::createUndefined(env);
}

// 应用生命周期接口
napi_value NAPI_OnPause(napi_env env, napi_callback_info info) {
    OH_LOG_INFO(LOG_APP, "NAPI_OnPause called");
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_OnResume(napi_env env, napi_callback_info info) {
    OH_LOG_INFO(LOG_APP, "NAPI_OnResume called");
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_OnDestroy(napi_env env, napi_callback_info info) {
    OH_LOG_INFO(LOG_APP, "NAPI_OnDestroy called");
    return NAPIHelper::createUndefined(env);
}

// 消息传递接口
napi_value NAPI_SendMessage(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createUndefined(env);
    }
    
    std::string message = NAPIHelper::getString(env, args[0]);
    OH_LOG_INFO(LOG_APP, "NAPI_SendMessage: %s", message.c_str());
    
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_CallLuaFunction(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createUndefined(env);
    }
    
    std::string functionName = NAPIHelper::getString(env, args[0]);
    std::string params = NAPIHelper::getString(env, args[1]);
    
    // TODO: 调用Lua函数
    // LuaNapiBridge::callLuaFunction(functionName, params);
    
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_RegisterArkTSCallback(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];

    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return NAPIHelper::createUndefined(env);
    }

    ParaEngineHelper* helper = ParaEngineHelper::getInstance();
    helper->init(env, args[0]);

    std::string languageName = helper->getCurrentLanguage();

    ParaEngine::LanguageType ret = ParaEngine::LanguageType::ENGLISH;
    if (languageName == "zh") {
        ret = ParaEngine::LanguageType::CHINESE;
    } else if (languageName == "en") {
        ret = ParaEngine::LanguageType::ENGLISH;
    } else if (languageName == "fr") {
        ret = ParaEngine::LanguageType::FRENCH;
    } else if (languageName == "it") {
        ret = ParaEngine::LanguageType::ITALIAN;
    } else if (languageName == "de") {
        ret = ParaEngine::LanguageType::GERMAN;
    } else if (languageName == "es") {
        ret = ParaEngine::LanguageType::SPANISH;
    } else if (languageName == "ru") {
        ret = ParaEngine::LanguageType::RUSSIAN;
    } else if (languageName == "nl") {
        ret = ParaEngine::LanguageType::DUTCH;
    } else if (languageName == "ko") {
        ret = ParaEngine::LanguageType::KOREAN;
    } else if (languageName == "ja") {
        ret = ParaEngine::LanguageType::JAPANESE;
    } else if (languageName == "hu") {
        ret = ParaEngine::LanguageType::HUNGARIAN;
    } else if (languageName == "pt") {
        ret = ParaEngine::LanguageType::PORTUGUESE;
    } else if (languageName == "ar") {
        ret = ParaEngine::LanguageType::ARABIC;
    } else if (languageName == "nb") {
        ret = ParaEngine::LanguageType::NORWEGIAN;
    } else if (languageName == "pl") {
        ret = ParaEngine::LanguageType::POLISH;
    } else if (languageName == "tr") {
        ret = ParaEngine::LanguageType::TURKISH;
    } else if (languageName == "uk") {
        ret = ParaEngine::LanguageType::UKRAINIAN;
    } else if (languageName == "ro") {
        ret = ParaEngine::LanguageType::ROMANIAN;
    } else if (languageName == "bg") {
        ret = ParaEngine::LanguageType::BULGARIAN;
    }

    ParaEngineSettings::GetSingleton().SetCurrentLanguage(ret);

    return NAPIHelper::createUndefined(env);
}

// 文件操作接口
napi_value NAPI_ReadAssetFile(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createUndefined(env);
    }
    
    std::string fileName = NAPIHelper::getString(env, args[0]);
    
    // TODO: 读取资源文件
    std::string content = "";
    
    return NAPIHelper::createString(env, content);
}

napi_value NAPI_WriteFile(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createBoolean(env, false);
    }
    
    std::string fileName = NAPIHelper::getString(env, args[0]);
    std::string content = NAPIHelper::getString(env, args[1]);
    
    // TODO: 写入文件
    bool success = false;
    
    return NAPIHelper::createBoolean(env, success);
}

napi_value NAPI_FileExists(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createBoolean(env, false);
    }
    
    std::string fileName = NAPIHelper::getString(env, args[0]);
    
    // TODO: 检查文件是否存在
    bool exists = false;
    
    return NAPIHelper::createBoolean(env, exists);
}

// 系统信息接口
napi_value NAPI_GetDeviceInfo(napi_env env, napi_callback_info info) {
    return NAPIHelper::createString(env, "{}");
}

napi_value NAPI_GetStoragePath(napi_env env, napi_callback_info info) {
    return NAPIHelper::createString(env, "/data/storage");
}

napi_value NAPI_GetResourcePath(napi_env env, napi_callback_info info) {
    return NAPIHelper::createString(env, "/data/resources");
}

// UI接口
napi_value NAPI_ShowWebView(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_HideWebView(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_OpenEditBox(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_SetScreenOrientation(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_ExitApplication(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

// 权限和功能接口
napi_value NAPI_HasPermission(napi_env env, napi_callback_info info) {
    return NAPIHelper::createBoolean(env, false);
}

napi_value NAPI_SaveImageToGallery(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

// 屏幕录制接口
napi_value NAPI_StartScreenRecording(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_StopScreenRecording(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_PauseScreenRecording(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_ResumeScreenRecording(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_GetScreenRecordingStatus(napi_env env, napi_callback_info info) {
    return NAPIHelper::createBoolean(env, false);
}

// 蓝牙接口
napi_value NAPI_IsBluetoothAvailable(napi_env env, napi_callback_info info) {
    return NAPIHelper::createBoolean(env, false);
}

napi_value NAPI_EnableBluetooth(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_DisableBluetooth(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_StartBluetoothScan(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_StopBluetoothScan(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_GetPairedDevices(napi_env env, napi_callback_info info) {
    return NAPIHelper::createString(env, "[]");
}

// 网络接口
napi_value NAPI_IsNetworkAvailable(napi_env env, napi_callback_info info) {
    return NAPIHelper::createBoolean(env, true);
}

napi_value NAPI_GetNetworkType(napi_env env, napi_callback_info info) {
    return NAPIHelper::createString(env, "wifi");
}

// 日志接口
napi_value NAPI_LogMessage(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return NAPIHelper::createUndefined(env);
    }
    
    std::string level = NAPIHelper::getString(env, args[0]);
    std::string tag = NAPIHelper::getString(env, args[1]);
    std::string message = NAPIHelper::getString(env, args[2]);
    
    OH_LOG_INFO(LOG_APP, "[%s][%s] %s", level.c_str(), tag.c_str(), message.c_str());
    
    return NAPIHelper::createUndefined(env);
}

// 其他接口
napi_value NAPI_GetLauncherData(napi_env env, napi_callback_info info) {
    return NAPIHelper::createString(env, "{}");
}

napi_value NAPI_OnAgreeUserPrivacy(napi_env env, napi_callback_info info) {
    return NAPIHelper::createUndefined(env);
}

napi_value NAPI_GetUsbMode(napi_env env, napi_callback_info info) {
    return NAPIHelper::createBoolean(env, false);
}

} // namespace ParaEngine
