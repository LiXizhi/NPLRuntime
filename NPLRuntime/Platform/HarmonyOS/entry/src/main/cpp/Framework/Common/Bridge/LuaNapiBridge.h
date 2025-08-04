//-----------------------------------------------------------------------------
// LuaNapiBridge.h - HarmonyOS NAPI Version
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2025.8.4 (Migrated to HarmonyOS NAPI)
//-----------------------------------------------------------------------------

#pragma once

#include "napi/NAPIHelper.h"

/**
 * Lua与HarmonyOS ArkTS的桥接类
 * 替代Android版本的LuaJavaBridge，使用NAPI进行通信
 */

extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

// Temporarily commented out luabind includes due to missing boost dependency
// #include <luabind/luabind.hpp>
// #include <luabind/out_value_policy.hpp>
// #include <luabind/return_reference_to_policy.hpp>
// #include <luabind/copy_policy.hpp>
// #include <luabind/adopt_policy.hpp>
// #include <luabind/function.hpp>
// #include <luabind/raw_policy.hpp>
// #include <luabind/object.hpp>

namespace ParaEngine {

    /**
     * Lua与ArkTS桥接管理器
     */
    class LuaNapiBridge {
    public:
        static LuaNapiBridge* getInstance();
        
        // 初始化和清理
        bool init(napi_env env);
        void cleanup();
        
        // 调用ArkTS方法
        bool callArkTSFunction(const std::string& functionName, const std::string& params);
        bool callArkTSFunctionWithCallback(const std::string& functionName, const std::string& params, const std::string& callbackId);
        
        // 从ArkTS调用Lua函数
        static void callLuaFunction(const std::string& functionName, const std::string& params);
        static void callLuaFunctionFromNative(lua_State* L, const std::string& functionName, const std::string& params);
        
        // 注册Lua绑定
        static void registerLuaBindings(lua_State* L);
        
        // 平台功能调用
        static std::string getPlatformBridge();
        static void saveImageToGallery(const std::string& imageData);
        static void openEditBox(const std::string& placeholder, const std::string& defaultText, int maxLength, bool multiline);
        static void showWebView(const std::string& url);
        static void hideWebView();
        static void setScreenOrientation(int orientation);
        static void exitApplication();
        static bool hasPermission(const std::string& permission);
        static std::string getLauncherData();
        static void onAgreeUserPrivacy();
        static bool getUsbMode();
        static void logMessage(const std::string& level, const std::string& tag, const std::string& message);
        
        // 设备信息获取
        static std::string getDeviceInfo();
        static std::string getStoragePath();
        static std::string getResourcePath();
        
        // 蓝牙相关
        static bool isBluetoothAvailable();
        static void enableBluetooth();
        static void disableBluetooth();
        static void startBluetoothScan();
        static void stopBluetoothScan();
        static std::string getPairedDevices();
        
        // 屏幕录制相关
        static void startScreenRecording();
        static void stopScreenRecording();
        static void pauseScreenRecording();
        static void resumeScreenRecording();
        static bool getScreenRecordingStatus();
        
        // 文件操作
        static std::string readAssetFile(const std::string& fileName);
        static bool writeFile(const std::string& fileName, const std::string& content);
        static bool fileExists(const std::string& fileName);
        
        // 网络状态
        static bool isNetworkAvailable();
        static std::string getNetworkType();
        
    private:
        LuaNapiBridge();
        ~LuaNapiBridge();
        
        static LuaNapiBridge* s_instance;
        napi_env m_env;
        napi_ref m_arkTSObject;
        bool m_initialized;
        
        // 辅助方法
        napi_value createStringValue(const std::string& str);
        std::string getStringFromValue(napi_value value);
        bool callArkTSMethod(const std::string& methodName, napi_value* args, size_t argc, napi_value* result = nullptr);
    };

    // Lua绑定的C函数
    extern "C" {
        // 平台功能
        int lua_getPlatformBridge(lua_State* L);
        int lua_saveImageToGallery(lua_State* L);
        int lua_openEditBox(lua_State* L);
        int lua_showWebView(lua_State* L);
        int lua_hideWebView(lua_State* L);
        int lua_setScreenOrientation(lua_State* L);
        int lua_exitApplication(lua_State* L);
        int lua_hasPermission(lua_State* L);
        int lua_getLauncherData(lua_State* L);
        int lua_onAgreeUserPrivacy(lua_State* L);
        int lua_getUsbMode(lua_State* L);
        int lua_logMessage(lua_State* L);
        
        // 设备信息
        int lua_getDeviceInfo(lua_State* L);
        int lua_getStoragePath(lua_State* L);
        int lua_getResourcePath(lua_State* L);
        
        // 蓝牙功能
        int lua_isBluetoothAvailable(lua_State* L);
        int lua_enableBluetooth(lua_State* L);
        int lua_disableBluetooth(lua_State* L);
        int lua_startBluetoothScan(lua_State* L);
        int lua_stopBluetoothScan(lua_State* L);
        int lua_getPairedDevices(lua_State* L);
        
        // 屏幕录制
        int lua_startScreenRecording(lua_State* L);
        int lua_stopScreenRecording(lua_State* L);
        int lua_pauseScreenRecording(lua_State* L);
        int lua_resumeScreenRecording(lua_State* L);
        int lua_getScreenRecordingStatus(lua_State* L);
        
        // 文件操作
        int lua_readAssetFile(lua_State* L);
        int lua_writeFile(lua_State* L);
        int lua_fileExists(lua_State* L);
        
        // 网络状态
        int lua_isNetworkAvailable(lua_State* L);
        int lua_getNetworkType(lua_State* L);
    }

} // namespace ParaEngine
