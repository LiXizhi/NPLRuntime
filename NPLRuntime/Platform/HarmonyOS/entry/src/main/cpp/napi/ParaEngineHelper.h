//-----------------------------------------------------------------------------
// ParaEngineHelper.h - HarmonyOS NAPI Version
// Authors: big
// CreateDate: 2025.8.4
//-----------------------------------------------------------------------------

#pragma once

#include "NAPIHelper.h"
#include <string>

namespace ParaEngine {

    /**
     * ParaEngine与HarmonyOS ArkTS交互的助手类
     * 对应Android版本的ParaEngineHelper
     */
    class ParaEngineHelper {
    public:
        static ParaEngineHelper* getInstance();
        
        // 初始化和清理
        bool init(napi_env env, napi_value arktsObject);
        void cleanup();
        
        // 调用ArkTS方法
        bool callArkTSMethod(const std::string& methodName, const std::vector<napi_value>& args, napi_value* result = nullptr);
        bool callArkTSMethod(const std::string& methodName, const std::string& params);
        
        // 平台功能
        std::string getPlatformBridge();
        void saveImageToGallery(const std::string& imageData);
        void openEditBox(const std::string& placeholder, const std::string& defaultText, int maxLength, bool multiline);
        void showWebView(const std::string& url);
        void hideWebView();
        void setScreenOrientation(int orientation);
        void exitApplication();
        bool hasPermission(const std::string& permission);
        std::string getLauncherData();
        void onAgreeUserPrivacy();
        bool getUsbMode();
        void logMessage(const std::string& level, const std::string& tag, const std::string& message);
        
        // 设备信息
        std::string getDeviceInfo();
        std::string getStoragePath();
        std::string getResourcePath();
        
        // 蓝牙功能
        bool isBluetoothAvailable();
        void enableBluetooth();
        void disableBluetooth();
        void startBluetoothScan();
        void stopBluetoothScan();
        std::string getPairedDevices();
        
        // 屏幕录制
        void startScreenRecording();
        void stopScreenRecording();
        void pauseScreenRecording();
        void resumeScreenRecording();
        bool getScreenRecordingStatus();
        
        // 文件操作
        std::string readAssetFile(const std::string& fileName);
        bool writeFile(const std::string& fileName, const std::string& content);
        bool fileExists(const std::string& fileName);
        
        // 网络状态
        bool isNetworkAvailable();
        std::string getNetworkType();

        // 语言相关
        std::string getCurrentLanguage();
        
        // 从ArkTS回调到Native的方法
        static void onEditBoxResult(const std::string& callbackId, const std::string& result);
        static void onWebViewEvent(const std::string& event, const std::string& data);
        static void onPermissionResult(const std::string& permission, bool granted);
        static void onBluetoothEvent(const std::string& event, const std::string& data);
        static void onScreenRecordEvent(const std::string& event, const std::string& data);
        
    private:
        ParaEngineHelper();
        ~ParaEngineHelper();
        
        static ParaEngineHelper* s_instance;
        napi_env m_env;
        napi_ref m_arktsObjectRef;
        bool m_initialized;
        
        // 辅助方法
        napi_value createArgsArray(const std::vector<std::string>& stringArgs);
        std::string callArkTSMethodWithStringResult(const std::string& methodName, const std::vector<std::string>& args);
        bool callArkTSMethodWithBoolResult(const std::string& methodName, const std::vector<std::string>& args);
    };

} // namespace ParaEngine

// NAPI导出函数声明
extern "C" {
    // 初始化函数
    napi_value Init(napi_env env, napi_value exports);
    
    // 核心功能导出
    napi_value CallLuaFunction(napi_env env, napi_callback_info info);
    napi_value GetPlatformBridge(napi_env env, napi_callback_info info);
    napi_value SaveImageToGallery(napi_env env, napi_callback_info info);
    napi_value OpenEditBox(napi_env env, napi_callback_info info);
    napi_value ShowWebView(napi_env env, napi_callback_info info);
    napi_value HideWebView(napi_env env, napi_callback_info info);
    napi_value SetScreenOrientation(napi_env env, napi_callback_info info);
    napi_value ExitApplication(napi_env env, napi_callback_info info);
    napi_value HasPermission(napi_env env, napi_callback_info info);
    napi_value GetLauncherData(napi_env env, napi_callback_info info);
    napi_value OnAgreeUserPrivacy(napi_env env, napi_callback_info info);
    napi_value GetUsbMode(napi_env env, napi_callback_info info);
    napi_value LogMessage(napi_env env, napi_callback_info info);
    
    // 设备信息导出
    napi_value GetDeviceInfo(napi_env env, napi_callback_info info);
    napi_value GetStoragePath(napi_env env, napi_callback_info info);
    napi_value GetResourcePath(napi_env env, napi_callback_info info);
    
    // 蓝牙功能导出
    napi_value IsBluetoothAvailable(napi_env env, napi_callback_info info);
    napi_value EnableBluetooth(napi_env env, napi_callback_info info);
    napi_value DisableBluetooth(napi_env env, napi_callback_info info);
    napi_value StartBluetoothScan(napi_env env, napi_callback_info info);
    napi_value StopBluetoothScan(napi_env env, napi_callback_info info);
    napi_value GetPairedDevices(napi_env env, napi_callback_info info);
    
    // 屏幕录制导出
    napi_value StartScreenRecording(napi_env env, napi_callback_info info);
    napi_value StopScreenRecording(napi_env env, napi_callback_info info);
    napi_value PauseScreenRecording(napi_env env, napi_callback_info info);
    napi_value ResumeScreenRecording(napi_env env, napi_callback_info info);
    napi_value GetScreenRecordingStatus(napi_env env, napi_callback_info info);
    
    // 文件操作导出
    napi_value ReadAssetFile(napi_env env, napi_callback_info info);
    napi_value WriteFile(napi_env env, napi_callback_info info);
    napi_value FileExists(napi_env env, napi_callback_info info);
    
    // 网络状态导出
    napi_value IsNetworkAvailable(napi_env env, napi_callback_info info);
    napi_value GetNetworkType(napi_env env, napi_callback_info info);
    
    // 回调处理导出
    napi_value OnEditBoxResult(napi_env env, napi_callback_info info);
    napi_value OnWebViewEvent(napi_env env, napi_callback_info info);
    napi_value OnPermissionResult(napi_env env, napi_callback_info info);
    napi_value OnBluetoothEvent(napi_env env, napi_callback_info info);
    napi_value OnScreenRecordEvent(napi_env env, napi_callback_info info);
}
