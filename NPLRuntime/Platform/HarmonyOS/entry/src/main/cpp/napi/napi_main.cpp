//-----------------------------------------------------------------------------
// napi_main.cpp - HarmonyOS NAPI Version
// Authors: LanZhiHong, big
// CreateDate: 2019.7.16
// ModifyDate: 2025.8.4 (Migrated to HarmonyOS NAPI)
//-----------------------------------------------------------------------------

#include "NAPIHelper.h"
#include <hilog/log.h>
#include <node_api.h>

// HarmonyOS日志标签
#define LOG_TAG "ParaEngine"

extern "C" {
    // NAPI模块注册函数
    static napi_value Init(napi_env env, napi_value exports);
    
    // NAPI模块描述
    static napi_module napi_module_desc = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = Init,
        .nm_modname = "entry",
        .nm_priv = nullptr,
        .reserved = {nullptr},
    };
    
    // 模块注册
    extern "C" __attribute__((constructor)) void RegisterModule() {
        napi_module_register(&napi_module_desc);
    }
}

namespace ParaEngine {
    
    // 导出的NAPI函数声明
    extern napi_value NAPI_SetSurface(napi_env env, napi_callback_info info);
    extern napi_value NAPI_SetAssetManager(napi_env env, napi_callback_info info);
    extern napi_value NAPI_InitParaEngine(napi_env env, napi_callback_info info);
    extern napi_value NAPI_CleanupParaEngine(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OnSurfaceCreated(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OnSurfaceChanged(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OnDrawFrame(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OnTouch(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OnKey(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OnPause(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OnResume(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OnDestroy(napi_env env, napi_callback_info info);
    extern napi_value NAPI_SendMessage(napi_env env, napi_callback_info info);
    extern napi_value NAPI_ReadAssetFile(napi_env env, napi_callback_info info);
    extern napi_value NAPI_WriteFile(napi_env env, napi_callback_info info);
    extern napi_value NAPI_FileExists(napi_env env, napi_callback_info info);
    extern napi_value NAPI_GetDeviceInfo(napi_env env, napi_callback_info info);
    extern napi_value NAPI_GetStoragePath(napi_env env, napi_callback_info info);
    extern napi_value NAPI_GetResourcePath(napi_env env, napi_callback_info info);
    extern napi_value NAPI_CallLuaFunction(napi_env env, napi_callback_info info);
    extern napi_value NAPI_RegisterArkTSCallback(napi_env env, napi_callback_info info);
    extern napi_value NAPI_ShowWebView(napi_env env, napi_callback_info info);
    extern napi_value NAPI_HideWebView(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OpenEditBox(napi_env env, napi_callback_info info);
    extern napi_value NAPI_SetScreenOrientation(napi_env env, napi_callback_info info);
    extern napi_value NAPI_ExitApplication(napi_env env, napi_callback_info info);
    extern napi_value NAPI_HasPermission(napi_env env, napi_callback_info info);
    extern napi_value NAPI_SaveImageToGallery(napi_env env, napi_callback_info info);
    extern napi_value NAPI_StartScreenRecording(napi_env env, napi_callback_info info);
    extern napi_value NAPI_StopScreenRecording(napi_env env, napi_callback_info info);
    extern napi_value NAPI_PauseScreenRecording(napi_env env, napi_callback_info info);
    extern napi_value NAPI_ResumeScreenRecording(napi_env env, napi_callback_info info);
    extern napi_value NAPI_GetScreenRecordingStatus(napi_env env, napi_callback_info info);
    extern napi_value NAPI_IsBluetoothAvailable(napi_env env, napi_callback_info info);
    extern napi_value NAPI_EnableBluetooth(napi_env env, napi_callback_info info);
    extern napi_value NAPI_DisableBluetooth(napi_env env, napi_callback_info info);
    extern napi_value NAPI_StartBluetoothScan(napi_env env, napi_callback_info info);
    extern napi_value NAPI_StopBluetoothScan(napi_env env, napi_callback_info info);
    extern napi_value NAPI_GetPairedDevices(napi_env env, napi_callback_info info);
    extern napi_value NAPI_IsNetworkAvailable(napi_env env, napi_callback_info info);
    extern napi_value NAPI_GetNetworkType(napi_env env, napi_callback_info info);
    extern napi_value NAPI_LogMessage(napi_env env, napi_callback_info info);
    extern napi_value NAPI_GetLauncherData(napi_env env, napi_callback_info info);
    extern napi_value NAPI_OnAgreeUserPrivacy(napi_env env, napi_callback_info info);
    extern napi_value NAPI_GetUsbMode(napi_env env, napi_callback_info info);
}

// NAPI模块初始化函数
static napi_value Init(napi_env env, napi_value exports) {
    OH_LOG_INFO(LOG_APP, "NAPI Module Init begin");
    
    // NAPIHelper不需要显式初始化
    
    // 定义要导出的方法
    napi_property_descriptor desc[] = {
        // 核心引擎方法
        {"setSurface", nullptr, ParaEngine::NAPI_SetSurface, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setAssetManager", nullptr, ParaEngine::NAPI_SetAssetManager, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"initParaEngine", nullptr, ParaEngine::NAPI_InitParaEngine, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"cleanupParaEngine", nullptr, ParaEngine::NAPI_CleanupParaEngine, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 渲染相关方法
        {"onSurfaceCreated", nullptr, ParaEngine::NAPI_OnSurfaceCreated, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onSurfaceChanged", nullptr, ParaEngine::NAPI_OnSurfaceChanged, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onDrawFrame", nullptr, ParaEngine::NAPI_OnDrawFrame, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 输入事件方法
        {"onTouch", nullptr, ParaEngine::NAPI_OnTouch, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onKey", nullptr, ParaEngine::NAPI_OnKey, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 生命周期方法
        {"onPause", nullptr, ParaEngine::NAPI_OnPause, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onResume", nullptr, ParaEngine::NAPI_OnResume, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onDestroy", nullptr, ParaEngine::NAPI_OnDestroy, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 通信方法
        {"sendMessage", nullptr, ParaEngine::NAPI_SendMessage, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"callLuaFunction", nullptr, ParaEngine::NAPI_CallLuaFunction, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"registerArkTSCallback", nullptr, ParaEngine::NAPI_RegisterArkTSCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 文件操作方法
        {"readAssetFile", nullptr, ParaEngine::NAPI_ReadAssetFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"writeFile", nullptr, ParaEngine::NAPI_WriteFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"fileExists", nullptr, ParaEngine::NAPI_FileExists, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 设备信息方法
        {"getDeviceInfo", nullptr, ParaEngine::NAPI_GetDeviceInfo, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getStoragePath", nullptr, ParaEngine::NAPI_GetStoragePath, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getResourcePath", nullptr, ParaEngine::NAPI_GetResourcePath, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // UI相关方法
        {"showWebView", nullptr, ParaEngine::NAPI_ShowWebView, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"hideWebView", nullptr, ParaEngine::NAPI_HideWebView, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"openEditBox", nullptr, ParaEngine::NAPI_OpenEditBox, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setScreenOrientation", nullptr, ParaEngine::NAPI_SetScreenOrientation, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"exitApplication", nullptr, ParaEngine::NAPI_ExitApplication, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 权限和安全方法
        {"hasPermission", nullptr, ParaEngine::NAPI_HasPermission, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"saveImageToGallery", nullptr, ParaEngine::NAPI_SaveImageToGallery, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 屏幕录制方法
        {"startScreenRecording", nullptr, ParaEngine::NAPI_StartScreenRecording, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"stopScreenRecording", nullptr, ParaEngine::NAPI_StopScreenRecording, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"pauseScreenRecording", nullptr, ParaEngine::NAPI_PauseScreenRecording, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"resumeScreenRecording", nullptr, ParaEngine::NAPI_ResumeScreenRecording, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getScreenRecordingStatus", nullptr, ParaEngine::NAPI_GetScreenRecordingStatus, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 蓝牙方法
        {"isBluetoothAvailable", nullptr, ParaEngine::NAPI_IsBluetoothAvailable, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"enableBluetooth", nullptr, ParaEngine::NAPI_EnableBluetooth, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"disableBluetooth", nullptr, ParaEngine::NAPI_DisableBluetooth, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"startBluetoothScan", nullptr, ParaEngine::NAPI_StartBluetoothScan, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"stopBluetoothScan", nullptr, ParaEngine::NAPI_StopBluetoothScan, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getPairedDevices", nullptr, ParaEngine::NAPI_GetPairedDevices, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 网络方法
        {"isNetworkAvailable", nullptr, ParaEngine::NAPI_IsNetworkAvailable, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getNetworkType", nullptr, ParaEngine::NAPI_GetNetworkType, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 日志和调试方法
        {"logMessage", nullptr, ParaEngine::NAPI_LogMessage, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        // 启动数据和隐私方法
        {"getLauncherData", nullptr, ParaEngine::NAPI_GetLauncherData, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"onAgreeUserPrivacy", nullptr, ParaEngine::NAPI_OnAgreeUserPrivacy, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getUsbMode", nullptr, ParaEngine::NAPI_GetUsbMode, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    
    // 定义导出的方法到exports对象
    napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    if (status != napi_ok) {
        OH_LOG_ERROR(LOG_APP, "Failed to define NAPI properties");
        return nullptr;
    }
    
    OH_LOG_INFO(LOG_APP, "NAPI Module Init end");
    return exports;
}
