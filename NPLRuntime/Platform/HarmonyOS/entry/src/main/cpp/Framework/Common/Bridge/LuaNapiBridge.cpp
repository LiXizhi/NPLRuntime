//-----------------------------------------------------------------------------
// LuaNapiBridge.cpp - HarmonyOS NAPI Version Implementation
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2025.8.4 (Migrated to HarmonyOS NAPI)
//-----------------------------------------------------------------------------

#include "LuaNapiBridge.h"
#include <hilog/log.h>
#include <fstream>
#include <filesystem>

#define LOG_TAG "ParaEngine"

namespace ParaEngine {

    // 静态成员变量定义
    LuaNapiBridge* LuaNapiBridge::s_instance = nullptr;

    LuaNapiBridge::LuaNapiBridge() 
        : m_env(nullptr)
        , m_arkTSObject(nullptr)
        , m_initialized(false) {
    }

    LuaNapiBridge::~LuaNapiBridge() {
        cleanup();
    }

    LuaNapiBridge* LuaNapiBridge::getInstance() {
        if (s_instance == nullptr) {
            s_instance = new LuaNapiBridge();
        }
        return s_instance;
    }

    bool LuaNapiBridge::init(napi_env env) {
        if (m_initialized) {
            return true;
        }

        m_env = env;
        m_initialized = true;

        OH_LOG_INFO(LOG_APP, "LuaNapiBridge initialized");
        return true;
    }

    void LuaNapiBridge::cleanup() {
        if (!m_initialized) {
            return;
        }

        if (m_arkTSObject != nullptr && m_env != nullptr) {
            napi_delete_reference(m_env, m_arkTSObject);
            m_arkTSObject = nullptr;
        }

        m_env = nullptr;
        m_initialized = false;

        OH_LOG_INFO(LOG_APP, "LuaNapiBridge cleanup completed");
    }

    napi_value LuaNapiBridge::createStringValue(const std::string& str) {
        if (m_env == nullptr) {
            return nullptr;
        }

        napi_value result;
        napi_status status = napi_create_string_utf8(m_env, str.c_str(), str.length(), &result);
        return (status == napi_ok) ? result : nullptr;
    }

    std::string LuaNapiBridge::getStringFromValue(napi_value value) {
        if (m_env == nullptr || value == nullptr) {
            return "";
        }

        size_t str_size = 0;
        napi_status status = napi_get_value_string_utf8(m_env, value, nullptr, 0, &str_size);
        if (status != napi_ok || str_size == 0) {
            return "";
        }

        std::string result(str_size, '\0');
        status = napi_get_value_string_utf8(m_env, value, &result[0], str_size + 1, nullptr);
        return (status == napi_ok) ? result : "";
    }

    bool LuaNapiBridge::callArkTSMethod(const std::string& methodName, napi_value* args, size_t argc, napi_value* result) {
        if (m_env == nullptr || m_arkTSObject == nullptr) {
            OH_LOG_ERROR(LOG_APP, "ArkTS object not set");
            return false;
        }

        napi_value arkTSObj;
        napi_status status = napi_get_reference_value(m_env, m_arkTSObject, &arkTSObj);
        if (status != napi_ok) {
            OH_LOG_ERROR(LOG_APP, "Failed to get ArkTS object reference");
            return false;
        }

        napi_value method;
        status = napi_get_named_property(m_env, arkTSObj, methodName.c_str(), &method);
        if (status != napi_ok) {
            OH_LOG_ERROR(LOG_APP, "Failed to get method: %s", methodName.c_str());
            return false;
        }

        napi_value callResult;
        status = napi_call_function(m_env, arkTSObj, method, argc, args, &callResult);
        if (status != napi_ok) {
            OH_LOG_ERROR(LOG_APP, "Failed to call method: %s", methodName.c_str());
            return false;
        }

        if (result != nullptr) {
            *result = callResult;
        }

        return true;
    }

    bool LuaNapiBridge::callArkTSFunction(const std::string& functionName, const std::string& params) {
        napi_value args[2];
        args[0] = createStringValue(functionName);
        args[1] = createStringValue(params);

        return callArkTSMethod("callFunction", args, 2);
    }

    bool LuaNapiBridge::callArkTSFunctionWithCallback(const std::string& functionName, const std::string& params, const std::string& callbackId) {
        napi_value args[3];
        args[0] = createStringValue(functionName);
        args[1] = createStringValue(params);
        args[2] = createStringValue(callbackId);

        return callArkTSMethod("callFunctionWithCallback", args, 3);
    }

    void LuaNapiBridge::callLuaFunction(const std::string& functionName, const std::string& params) {
        // TODO: 实现调用Lua函数的逻辑
        OH_LOG_INFO(LOG_APP, "callLuaFunction: %s with params: %s", functionName.c_str(), params.c_str());
    }

    void LuaNapiBridge::callLuaFunctionFromNative(lua_State* L, const std::string& functionName, const std::string& params) {
        if (L == nullptr) {
            return;
        }

        // TODO: 实现从Native调用Lua函数的逻辑
        OH_LOG_INFO(LOG_APP, "callLuaFunctionFromNative: %s with params: %s", functionName.c_str(), params.c_str());
    }

    void LuaNapiBridge::registerLuaBindings(lua_State* L) {
        if (L == nullptr) {
            return;
        }

        // TODO: 注册Lua绑定
        OH_LOG_INFO(LOG_APP, "registerLuaBindings called");
    }

    // ==============================================
    // 平台功能实现
    // ==============================================

    std::string LuaNapiBridge::getPlatformBridge() {
        return "HarmonyOS";
    }

    void LuaNapiBridge::saveImageToGallery(const std::string& imageData) {
        LuaNapiBridge* instance = getInstance();
        napi_value args[1];
        args[0] = instance->createStringValue(imageData);
        instance->callArkTSMethod("saveImageToGallery", args, 1);
    }

    void LuaNapiBridge::openEditBox(const std::string& placeholder, const std::string& defaultText, int maxLength, bool multiline) {
        LuaNapiBridge* instance = getInstance();
        napi_value args[4];
        args[0] = instance->createStringValue(placeholder);
        args[1] = instance->createStringValue(defaultText);
        napi_create_int32(instance->m_env, maxLength, &args[2]);
        napi_create_int32(instance->m_env, multiline ? 1 : 0, &args[3]);
        instance->callArkTSMethod("openEditBox", args, 4);
    }

    void LuaNapiBridge::showWebView(const std::string& url) {
        LuaNapiBridge* instance = getInstance();
        napi_value args[1];
        args[0] = instance->createStringValue(url);
        instance->callArkTSMethod("showWebView", args, 1);
    }

    void LuaNapiBridge::hideWebView() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("hideWebView", nullptr, 0);
    }

    void LuaNapiBridge::setScreenOrientation(int orientation) {
        LuaNapiBridge* instance = getInstance();
        napi_value args[1];
        napi_create_int32(instance->m_env, orientation, &args[0]);
        instance->callArkTSMethod("setScreenOrientation", args, 1);
    }

    void LuaNapiBridge::exitApplication() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("exitApplication", nullptr, 0);
    }

    bool LuaNapiBridge::hasPermission(const std::string& permission) {
        LuaNapiBridge* instance = getInstance();
        napi_value args[1];
        args[0] = instance->createStringValue(permission);
        
        napi_value result;
        if (instance->callArkTSMethod("hasPermission", args, 1, &result)) {
            bool hasPermission = false;
            napi_get_value_bool(instance->m_env, result, &hasPermission);
            return hasPermission;
        }
        return false;
    }

    std::string LuaNapiBridge::getLauncherData() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("getLauncherData", nullptr, 0, &result)) {
            return instance->getStringFromValue(result);
        }
        return "";
    }

    void LuaNapiBridge::onAgreeUserPrivacy() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("onAgreeUserPrivacy", nullptr, 0);
    }

    bool LuaNapiBridge::getUsbMode() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("getUsbMode", nullptr, 0, &result)) {
            bool usbMode = false;
            napi_get_value_bool(instance->m_env, result, &usbMode);
            return usbMode;
        }
        return false;
    }

    void LuaNapiBridge::logMessage(const std::string& level, const std::string& tag, const std::string& message) {
        LuaNapiBridge* instance = getInstance();
        napi_value args[3];
        args[0] = instance->createStringValue(level);
        args[1] = instance->createStringValue(tag);
        args[2] = instance->createStringValue(message);
        instance->callArkTSMethod("logMessage", args, 3);
    }

    // ==============================================
    // 设备信息获取
    // ==============================================

    std::string LuaNapiBridge::getDeviceInfo() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("getDeviceInfo", nullptr, 0, &result)) {
            return instance->getStringFromValue(result);
        }
        return "{}";
    }

    std::string LuaNapiBridge::getStoragePath() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("getStoragePath", nullptr, 0, &result)) {
            return instance->getStringFromValue(result);
        }
        return "";
    }

    std::string LuaNapiBridge::getResourcePath() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("getResourcePath", nullptr, 0, &result)) {
            return instance->getStringFromValue(result);
        }
        return "";
    }

    // ==============================================
    // 蓝牙相关
    // ==============================================

    bool LuaNapiBridge::isBluetoothAvailable() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("isBluetoothAvailable", nullptr, 0, &result)) {
            bool available = false;
            napi_get_value_bool(instance->m_env, result, &available);
            return available;
        }
        return false;
    }

    void LuaNapiBridge::enableBluetooth() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("enableBluetooth", nullptr, 0);
    }

    void LuaNapiBridge::disableBluetooth() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("disableBluetooth", nullptr, 0);
    }

    void LuaNapiBridge::startBluetoothScan() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("startBluetoothScan", nullptr, 0);
    }

    void LuaNapiBridge::stopBluetoothScan() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("stopBluetoothScan", nullptr, 0);
    }

    std::string LuaNapiBridge::getPairedDevices() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("getPairedDevices", nullptr, 0, &result)) {
            return instance->getStringFromValue(result);
        }
        return "[]";
    }

    // ==============================================
    // 屏幕录制相关
    // ==============================================

    void LuaNapiBridge::startScreenRecording() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("startScreenRecording", nullptr, 0);
    }

    void LuaNapiBridge::stopScreenRecording() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("stopScreenRecording", nullptr, 0);
    }

    void LuaNapiBridge::pauseScreenRecording() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("pauseScreenRecording", nullptr, 0);
    }

    void LuaNapiBridge::resumeScreenRecording() {
        LuaNapiBridge* instance = getInstance();
        instance->callArkTSMethod("resumeScreenRecording", nullptr, 0);
    }

    bool LuaNapiBridge::getScreenRecordingStatus() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("getScreenRecordingStatus", nullptr, 0, &result)) {
            bool status = false;
            napi_get_value_bool(instance->m_env, result, &status);
            return status;
        }
        return false;
    }

    // ==============================================
    // 文件操作
    // ==============================================

    std::string LuaNapiBridge::readAssetFile(const std::string& fileName) {
        // 使用ResourceManager读取资源文件
        std::string content = NAPIHelper::readAssetFile(fileName);
        return content;
    }

    bool LuaNapiBridge::writeFile(const std::string& fileName, const std::string& content) {
        try {
            std::ofstream file(fileName);
            if (file.is_open()) {
                file << content;
                file.close();
                return true;
            }
        } catch (const std::exception& e) {
            OH_LOG_ERROR(LOG_APP, "Failed to write file %s: %s", fileName.c_str(), e.what());
        }
        return false;
    }

    bool LuaNapiBridge::fileExists(const std::string& fileName) {
        return std::filesystem::exists(fileName);
    }

    // ==============================================
    // 网络状态
    // ==============================================

    bool LuaNapiBridge::isNetworkAvailable() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("isNetworkAvailable", nullptr, 0, &result)) {
            bool available = false;
            napi_get_value_bool(instance->m_env, result, &available);
            return available;
        }
        return false;
    }

    std::string LuaNapiBridge::getNetworkType() {
        LuaNapiBridge* instance = getInstance();
        napi_value result;
        if (instance->callArkTSMethod("getNetworkType", nullptr, 0, &result)) {
            return instance->getStringFromValue(result);
        }
        return "unknown";
    }

} // namespace ParaEngine

// ==============================================
// Lua绑定的C函数实现
// ==============================================

extern "C" {
    using namespace ParaEngine;

    int lua_getPlatformBridge(lua_State* L) {
        std::string platform = LuaNapiBridge::getPlatformBridge();
        lua_pushstring(L, platform.c_str());
        return 1;
    }

    int lua_saveImageToGallery(lua_State* L) {
        const char* imageData = luaL_checkstring(L, 1);
        LuaNapiBridge::saveImageToGallery(imageData);
        return 0;
    }

    int lua_openEditBox(lua_State* L) {
        const char* placeholder = luaL_checkstring(L, 1);
        const char* defaultText = luaL_checkstring(L, 2);
        int maxLength = luaL_checkinteger(L, 3);
        bool multiline = lua_toboolean(L, 4);
        LuaNapiBridge::openEditBox(placeholder, defaultText, maxLength, multiline);
        return 0;
    }

    int lua_showWebView(lua_State* L) {
        const char* url = luaL_checkstring(L, 1);
        LuaNapiBridge::showWebView(url);
        return 0;
    }

    int lua_hideWebView(lua_State* L) {
        LuaNapiBridge::hideWebView();
        return 0;
    }

    int lua_setScreenOrientation(lua_State* L) {
        int orientation = luaL_checkinteger(L, 1);
        LuaNapiBridge::setScreenOrientation(orientation);
        return 0;
    }

    int lua_exitApplication(lua_State* L) {
        LuaNapiBridge::exitApplication();
        return 0;
    }

    int lua_hasPermission(lua_State* L) {
        const char* permission = luaL_checkstring(L, 1);
        bool hasPermission = LuaNapiBridge::hasPermission(permission);
        lua_pushboolean(L, hasPermission);
        return 1;
    }

    int lua_getLauncherData(lua_State* L) {
        std::string data = LuaNapiBridge::getLauncherData();
        lua_pushstring(L, data.c_str());
        return 1;
    }

    int lua_onAgreeUserPrivacy(lua_State* L) {
        LuaNapiBridge::onAgreeUserPrivacy();
        return 0;
    }

    int lua_getUsbMode(lua_State* L) {
        bool usbMode = LuaNapiBridge::getUsbMode();
        lua_pushboolean(L, usbMode);
        return 1;
    }

    int lua_logMessage(lua_State* L) {
        const char* level = luaL_checkstring(L, 1);
        const char* tag = luaL_checkstring(L, 2);
        const char* message = luaL_checkstring(L, 3);
        LuaNapiBridge::logMessage(level, tag, message);
        return 0;
    }

    int lua_getDeviceInfo(lua_State* L) {
        std::string deviceInfo = LuaNapiBridge::getDeviceInfo();
        lua_pushstring(L, deviceInfo.c_str());
        return 1;
    }

    int lua_getStoragePath(lua_State* L) {
        std::string storagePath = LuaNapiBridge::getStoragePath();
        lua_pushstring(L, storagePath.c_str());
        return 1;
    }

    int lua_getResourcePath(lua_State* L) {
        std::string resourcePath = LuaNapiBridge::getResourcePath();
        lua_pushstring(L, resourcePath.c_str());
        return 1;
    }

    int lua_isBluetoothAvailable(lua_State* L) {
        bool available = LuaNapiBridge::isBluetoothAvailable();
        lua_pushboolean(L, available);
        return 1;
    }

    int lua_enableBluetooth(lua_State* L) {
        LuaNapiBridge::enableBluetooth();
        return 0;
    }

    int lua_disableBluetooth(lua_State* L) {
        LuaNapiBridge::disableBluetooth();
        return 0;
    }

    int lua_startBluetoothScan(lua_State* L) {
        LuaNapiBridge::startBluetoothScan();
        return 0;
    }

    int lua_stopBluetoothScan(lua_State* L) {
        LuaNapiBridge::stopBluetoothScan();
        return 0;
    }

    int lua_getPairedDevices(lua_State* L) {
        std::string devices = LuaNapiBridge::getPairedDevices();
        lua_pushstring(L, devices.c_str());
        return 1;
    }

    int lua_startScreenRecording(lua_State* L) {
        LuaNapiBridge::startScreenRecording();
        return 0;
    }

    int lua_stopScreenRecording(lua_State* L) {
        LuaNapiBridge::stopScreenRecording();
        return 0;
    }

    int lua_pauseScreenRecording(lua_State* L) {
        LuaNapiBridge::pauseScreenRecording();
        return 0;
    }

    int lua_resumeScreenRecording(lua_State* L) {
        LuaNapiBridge::resumeScreenRecording();
        return 0;
    }

    int lua_getScreenRecordingStatus(lua_State* L) {
        bool status = LuaNapiBridge::getScreenRecordingStatus();
        lua_pushboolean(L, status);
        return 1;
    }

    int lua_readAssetFile(lua_State* L) {
        const char* fileName = luaL_checkstring(L, 1);
        std::string content = LuaNapiBridge::readAssetFile(fileName);
        lua_pushstring(L, content.c_str());
        return 1;
    }

    int lua_writeFile(lua_State* L) {
        const char* fileName = luaL_checkstring(L, 1);
        const char* content = luaL_checkstring(L, 2);
        bool success = LuaNapiBridge::writeFile(fileName, content);
        lua_pushboolean(L, success);
        return 1;
    }

    int lua_fileExists(lua_State* L) {
        const char* fileName = luaL_checkstring(L, 1);
        bool exists = LuaNapiBridge::fileExists(fileName);
        lua_pushboolean(L, exists);
        return 1;
    }

    int lua_isNetworkAvailable(lua_State* L) {
        bool available = LuaNapiBridge::isNetworkAvailable();
        lua_pushboolean(L, available);
        return 1;
    }

    int lua_getNetworkType(lua_State* L) {
        std::string networkType = LuaNapiBridge::getNetworkType();
        lua_pushstring(L, networkType.c_str());
        return 1;
    }
}
