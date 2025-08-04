//-----------------------------------------------------------------------------
// ParaEngineHelper.cpp - HarmonyOS NAPI Version Implementation
// Authors: LanZhiHong, big
// CreateDate: 2025.8.4
// ModifyDate: 2025.8.4
//-----------------------------------------------------------------------------

#include "../../napi/ParaEngineHelper.h"
#include "NAPIHelper.h"
#include "LuaNapiBridge.h"
#include <hilog/log.h>

#define LOG_TAG "ParaEngine"

namespace ParaEngine {

    // Static instance
    ParaEngineHelper* ParaEngineHelper::s_instance = nullptr;

    ParaEngineHelper* ParaEngineHelper::getInstance() {
        if (s_instance == nullptr) {
            s_instance = new ParaEngineHelper();
        }
        return s_instance;
    }

    ParaEngineHelper::ParaEngineHelper() : m_env(nullptr), m_arktsObjectRef(nullptr), m_initialized(false) {
        OH_LOG_INFO(LOG_APP, "ParaEngineHelper constructor");
    }

    ParaEngineHelper::~ParaEngineHelper() {
        cleanup();
        OH_LOG_INFO(LOG_APP, "ParaEngineHelper destructor");
    }

    bool ParaEngineHelper::init(napi_env env, napi_value arktsObject) {
        if (m_initialized) {
            OH_LOG_WARN(LOG_APP, "ParaEngineHelper already initialized");
            return true;
        }

        m_env = env;
        
        // Create a reference to the ArkTS object
        napi_status status = napi_create_reference(env, arktsObject, 1, &m_arktsObjectRef);
        if (status != napi_ok) {
            OH_LOG_ERROR(LOG_APP, "Failed to create reference to ArkTS object");
            return false;
        }

        m_initialized = true;
        OH_LOG_INFO(LOG_APP, "ParaEngineHelper initialized successfully");
        return true;
    }

    void ParaEngineHelper::cleanup() {
        if (!m_initialized) {
            return;
        }

        if (m_arktsObjectRef != nullptr && m_env != nullptr) {
            napi_delete_reference(m_env, m_arktsObjectRef);
            m_arktsObjectRef = nullptr;
        }

        m_env = nullptr;
        m_initialized = false;
        OH_LOG_INFO(LOG_APP, "ParaEngineHelper cleanup completed");
    }

    bool ParaEngineHelper::callArkTSMethod(const std::string& methodName, const std::vector<napi_value>& args, napi_value* result) {
        if (!m_initialized || m_env == nullptr || m_arktsObjectRef == nullptr) {
            OH_LOG_ERROR(LOG_APP, "ParaEngineHelper not initialized");
            return false;
        }

        napi_value arktsObject;
        napi_status status = napi_get_reference_value(m_env, m_arktsObjectRef, &arktsObject);
        if (status != napi_ok) {
            OH_LOG_ERROR(LOG_APP, "Failed to get ArkTS object reference");
            return false;
        }

        napi_value method;
        status = napi_get_named_property(m_env, arktsObject, methodName.c_str(), &method);
        if (status != napi_ok) {
            OH_LOG_ERROR(LOG_APP, "Failed to get method: %s", methodName.c_str());
            return false;
        }

        napi_value callResult;
        status = napi_call_function(m_env, arktsObject, method, args.size(), args.data(), &callResult);
        if (status != napi_ok) {
            OH_LOG_ERROR(LOG_APP, "Failed to call method: %s", methodName.c_str());
            return false;
        }

        if (result != nullptr) {
            *result = callResult;
        }

        return true;
    }

    bool ParaEngineHelper::callArkTSMethod(const std::string& methodName, const std::string& params) {
        std::vector<napi_value> args;
        if (!params.empty()) {
            napi_value paramValue = NAPIHelper::createString(m_env, params);
            args.push_back(paramValue);
        }
        return callArkTSMethod(methodName, args);
    }

    // Platform functionality implementations
    std::string ParaEngineHelper::getPlatformBridge() {
        return callArkTSMethodWithStringResult("getPlatformBridge", {});
    }

    void ParaEngineHelper::saveImageToGallery(const std::string& imageData) {
        callArkTSMethod("saveImageToGallery", {imageData});
    }

    void ParaEngineHelper::openEditBox(const std::string& placeholder, const std::string& defaultText, int maxLength, bool multiline) {
        std::vector<std::string> args = {placeholder, defaultText, std::to_string(maxLength), multiline ? "true" : "false"};
        callArkTSMethodWithStringResult("openEditBox", args);
    }

    void ParaEngineHelper::showWebView(const std::string& url) {
        callArkTSMethod("showWebView", {url});
    }

    void ParaEngineHelper::hideWebView() {
        callArkTSMethod("hideWebView", "");
    }

    void ParaEngineHelper::setScreenOrientation(int orientation) {
        callArkTSMethod("setScreenOrientation", {std::to_string(orientation)});
    }

    void ParaEngineHelper::exitApplication() {
        callArkTSMethod("exitApplication", "");
    }

    bool ParaEngineHelper::hasPermission(const std::string& permission) {
        return callArkTSMethodWithBoolResult("hasPermission", {permission});
    }

    std::string ParaEngineHelper::getLauncherData() {
        return callArkTSMethodWithStringResult("getLauncherData", {});
    }

    void ParaEngineHelper::onAgreeUserPrivacy() {
        callArkTSMethod("onAgreeUserPrivacy", "");
    }

    bool ParaEngineHelper::getUsbMode() {
        return callArkTSMethodWithBoolResult("getUsbMode", {});
    }

    void ParaEngineHelper::logMessage(const std::string& level, const std::string& tag, const std::string& message) {
        callArkTSMethodWithStringResult("logMessage", {level, tag, message});
    }

    // Device info
    std::string ParaEngineHelper::getDeviceInfo() {
        return callArkTSMethodWithStringResult("getDeviceInfo", {});
    }

    std::string ParaEngineHelper::getStoragePath() {
        return callArkTSMethodWithStringResult("getStoragePath", {});
    }

    std::string ParaEngineHelper::getResourcePath() {
        return callArkTSMethodWithStringResult("getResourcePath", {});
    }

    // Bluetooth functionality
    bool ParaEngineHelper::isBluetoothAvailable() {
        return callArkTSMethodWithBoolResult("isBluetoothAvailable", {});
    }

    void ParaEngineHelper::enableBluetooth() {
        callArkTSMethod("enableBluetooth", "");
    }

    void ParaEngineHelper::disableBluetooth() {
        callArkTSMethod("disableBluetooth", "");
    }

    void ParaEngineHelper::startBluetoothScan() {
        callArkTSMethod("startBluetoothScan", "");
    }

    void ParaEngineHelper::stopBluetoothScan() {
        callArkTSMethod("stopBluetoothScan", "");
    }

    std::string ParaEngineHelper::getPairedDevices() {
        return callArkTSMethodWithStringResult("getPairedDevices", {});
    }

    // Screen recording
    void ParaEngineHelper::startScreenRecording() {
        callArkTSMethod("startScreenRecording", "");
    }

    void ParaEngineHelper::stopScreenRecording() {
        callArkTSMethod("stopScreenRecording", "");
    }

    void ParaEngineHelper::pauseScreenRecording() {
        callArkTSMethod("pauseScreenRecording", "");
    }

    void ParaEngineHelper::resumeScreenRecording() {
        callArkTSMethod("resumeScreenRecording", "");
    }

    bool ParaEngineHelper::getScreenRecordingStatus() {
        return callArkTSMethodWithBoolResult("getScreenRecordingStatus", {});
    }

    // File operations
    std::string ParaEngineHelper::readAssetFile(const std::string& fileName) {
        return callArkTSMethodWithStringResult("readAssetFile", {fileName});
    }

    bool ParaEngineHelper::writeFile(const std::string& fileName, const std::string& content) {
        return callArkTSMethodWithBoolResult("writeFile", {fileName, content});
    }

    bool ParaEngineHelper::fileExists(const std::string& fileName) {
        return callArkTSMethodWithBoolResult("fileExists", {fileName});
    }

    // Network
    bool ParaEngineHelper::isNetworkAvailable() {
        return callArkTSMethodWithBoolResult("isNetworkAvailable", {});
    }

    std::string ParaEngineHelper::getNetworkType() {
        return callArkTSMethodWithStringResult("getNetworkType", {});
    }

    // Static callback methods
    void ParaEngineHelper::onEditBoxResult(const std::string& callbackId, const std::string& result) {
        OH_LOG_INFO(LOG_APP, "onEditBoxResult: %s = %s", callbackId.c_str(), result.c_str());
        // TODO: Handle edit box result
    }

    void ParaEngineHelper::onWebViewEvent(const std::string& event, const std::string& data) {
        OH_LOG_INFO(LOG_APP, "onWebViewEvent: %s = %s", event.c_str(), data.c_str());
        // TODO: Handle web view event
    }

    void ParaEngineHelper::onPermissionResult(const std::string& permission, bool granted) {
        OH_LOG_INFO(LOG_APP, "onPermissionResult: %s = %s", permission.c_str(), granted ? "granted" : "denied");
        // TODO: Handle permission result
    }

    void ParaEngineHelper::onBluetoothEvent(const std::string& event, const std::string& data) {
        OH_LOG_INFO(LOG_APP, "onBluetoothEvent: %s = %s", event.c_str(), data.c_str());
        // TODO: Handle bluetooth event
    }

    void ParaEngineHelper::onScreenRecordEvent(const std::string& event, const std::string& data) {
        OH_LOG_INFO(LOG_APP, "onScreenRecordEvent: %s = %s", event.c_str(), data.c_str());
        // TODO: Handle screen record event
    }

    // Private helper methods
    napi_value ParaEngineHelper::createArgsArray(const std::vector<std::string>& stringArgs) {
        napi_value argsArray;
        napi_create_array_with_length(m_env, stringArgs.size(), &argsArray);
        
        for (size_t i = 0; i < stringArgs.size(); i++) {
            napi_value argValue = NAPIHelper::createString(m_env, stringArgs[i]);
            napi_set_element(m_env, argsArray, i, argValue);
        }
        
        return argsArray;
    }

    std::string ParaEngineHelper::callArkTSMethodWithStringResult(const std::string& methodName, const std::vector<std::string>& args) {
        std::vector<napi_value> napiArgs;
        for (const auto& arg : args) {
            napiArgs.push_back(NAPIHelper::createString(m_env, arg));
        }
        
        napi_value result;
        if (callArkTSMethod(methodName, napiArgs, &result)) {
            return NAPIHelper::getString(m_env, result);
        }
        
        return "";
    }

    bool ParaEngineHelper::callArkTSMethodWithBoolResult(const std::string& methodName, const std::vector<std::string>& args) {
        std::vector<napi_value> napiArgs;
        for (const auto& arg : args) {
            napiArgs.push_back(NAPIHelper::createString(m_env, arg));
        }
        
        napi_value result;
        if (callArkTSMethod(methodName, napiArgs, &result)) {
            return NAPIHelper::getBoolean(m_env, result);
        }
        
        return false;
    }

} // namespace ParaEngine
