//-----------------------------------------------------------------------------
// ParaEngineHelper.cpp - HarmonyOS NAPI Version
// Authors: big
// CreateDate: 2025.8.4
//-----------------------------------------------------------------------------

#include "ParaEngineHelper.h"
#include <hilog/log.h>
#include <string>
#include <vector>

// 重新定义LOG_TAG以避免冲突
#undef LOG_TAG
#define LOG_TAG "ParaEngineHelper"

namespace ParaEngine {

    ParaEngineHelper* ParaEngineHelper::s_instance = nullptr;

    ParaEngineHelper::ParaEngineHelper() 
        : m_env(nullptr), m_arktsObjectRef(nullptr), m_initialized(false) {
    }

    ParaEngineHelper::~ParaEngineHelper() {
        cleanup();
    }

    ParaEngineHelper* ParaEngineHelper::getInstance() {
        if (!s_instance) {
            s_instance = new ParaEngineHelper();
        }
        return s_instance;
    }

    bool ParaEngineHelper::init(napi_env env, napi_value arktsObject) {
        if (m_initialized) {
            OH_LOG_WARN(LOG_APP, "ParaEngineHelper already initialized");
            return true;
        }

        m_env = env;
        
        if (arktsObject != nullptr) {
            napi_status status = napi_create_reference(env, arktsObject, 1, &m_arktsObjectRef);
            if (status != napi_ok) {
                OH_LOG_ERROR(LOG_APP, "Failed to create reference to ArkTS object");
                return false;
            }
        }

        m_initialized = true;
        OH_LOG_INFO(LOG_APP, "ParaEngineHelper initialized successfully");
        return true;
    }

    void ParaEngineHelper::cleanup() {
        if (m_arktsObjectRef && m_env) {
            napi_delete_reference(m_env, m_arktsObjectRef);
            m_arktsObjectRef = nullptr;
        }
        
        m_env = nullptr;
        m_initialized = false;
        OH_LOG_INFO(LOG_APP, "ParaEngineHelper cleaned up");
    }

    bool ParaEngineHelper::callArkTSMethod(const std::string& methodName, const std::vector<napi_value>& args, napi_value* result) {
        if (!m_initialized || !m_env || !m_arktsObjectRef) {
            OH_LOG_ERROR(LOG_APP, "ParaEngineHelper not properly initialized");
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

        if (result) {
            *result = callResult;
        }

        return true;
    }

    bool ParaEngineHelper::callArkTSMethod(const std::string& methodName, const std::string& params) {
        if (!m_initialized || !m_env) {
            return false;
        }

        napi_value stringValue;
        napi_status status = napi_create_string_utf8(m_env, params.c_str(), NAPI_AUTO_LENGTH, &stringValue);
        if (status != napi_ok) {
            return false;
        }

        std::vector<napi_value> args = { stringValue };
        return callArkTSMethod(methodName, args);
    }

    // 平台功能实现（基础版本）
    std::string ParaEngineHelper::getPlatformBridge() {
        // TODO: 实现平台桥接功能
        return "HarmonyOS";
    }

    void ParaEngineHelper::saveImageToGallery(const std::string& imageData) {
        // TODO: 实现保存图片到相册功能
        OH_LOG_INFO(LOG_APP, "saveImageToGallery called");
    }

    void ParaEngineHelper::openEditBox(const std::string& placeholder, const std::string& defaultText, int maxLength, bool multiline) {
        // TODO: 实现打开编辑框功能
        OH_LOG_INFO(LOG_APP, "openEditBox called");
    }

    void ParaEngineHelper::showWebView(const std::string& url) {
        // TODO: 实现显示WebView功能
        OH_LOG_INFO(LOG_APP, "showWebView called with URL: %s", url.c_str());
    }

    void ParaEngineHelper::hideWebView() {
        // TODO: 实现隐藏WebView功能
        OH_LOG_INFO(LOG_APP, "hideWebView called");
    }

    void ParaEngineHelper::setScreenOrientation(int orientation) {
        // TODO: 实现设置屏幕方向功能
        OH_LOG_INFO(LOG_APP, "setScreenOrientation called with orientation: %d", orientation);
    }

    void ParaEngineHelper::exitApplication() {
        // TODO: 实现退出应用功能
        OH_LOG_INFO(LOG_APP, "exitApplication called");
    }

    bool ParaEngineHelper::hasPermission(const std::string& permission) {
        // TODO: 实现权限检查功能
        OH_LOG_INFO(LOG_APP, "hasPermission called for: %s", permission.c_str());
        return true; // 默认返回true
    }

    std::string ParaEngineHelper::getLauncherData() {
        // TODO: 实现获取启动数据功能
        return "";
    }

    void ParaEngineHelper::onAgreeUserPrivacy() {
        // TODO: 实现用户隐私协议同意功能
        OH_LOG_INFO(LOG_APP, "onAgreeUserPrivacy called");
    }

    bool ParaEngineHelper::getUsbMode() {
        // TODO: 实现获取USB模式功能
        return false;
    }

    void ParaEngineHelper::logMessage(const std::string& level, const std::string& tag, const std::string& message) {
        // 实现日志记录功能
        if (level == "info") {
            OH_LOG_INFO(LOG_APP, "[%s] %s", tag.c_str(), message.c_str());
        } else if (level == "error") {
            OH_LOG_ERROR(LOG_APP, "[%s] %s", tag.c_str(), message.c_str());
        } else if (level == "warn") {
            OH_LOG_WARN(LOG_APP, "[%s] %s", tag.c_str(), message.c_str());
        } else {
            OH_LOG_DEBUG(LOG_APP, "[%s] %s", tag.c_str(), message.c_str());
        }
    }

    // 设备信息
    std::string ParaEngineHelper::getDeviceInfo() {
        // TODO: 实现获取设备信息功能
        return "HarmonyOS Device";
    }

    std::string ParaEngineHelper::getStoragePath() {
        // TODO: 实现获取存储路径功能
        return "/data/storage/el2/base/haps/entry/files";
    }

    std::string ParaEngineHelper::getResourcePath() {
        // TODO: 实现获取资源路径功能
        return "/data/storage/el1/bundle/entry/resources";
    }

    // 蓝牙功能（基础实现）
    bool ParaEngineHelper::isBluetoothAvailable() {
        // TODO: 实现蓝牙可用性检查
        return false;
    }

    void ParaEngineHelper::enableBluetooth() {
        OH_LOG_INFO(LOG_APP, "enableBluetooth called");
    }

    void ParaEngineHelper::disableBluetooth() {
        OH_LOG_INFO(LOG_APP, "disableBluetooth called");
    }

    void ParaEngineHelper::startBluetoothScan() {
        OH_LOG_INFO(LOG_APP, "startBluetoothScan called");
    }

    void ParaEngineHelper::stopBluetoothScan() {
        OH_LOG_INFO(LOG_APP, "stopBluetoothScan called");
    }

    std::string ParaEngineHelper::getPairedDevices() {
        // TODO: 实现获取已配对设备功能
        return "[]";
    }

    // 屏幕录制功能（基础实现）
    void ParaEngineHelper::startScreenRecording() {
        OH_LOG_INFO(LOG_APP, "startScreenRecording called");
    }

    void ParaEngineHelper::stopScreenRecording() {
        OH_LOG_INFO(LOG_APP, "stopScreenRecording called");
    }

    void ParaEngineHelper::pauseScreenRecording() {
        OH_LOG_INFO(LOG_APP, "pauseScreenRecording called");
    }

    void ParaEngineHelper::resumeScreenRecording() {
        OH_LOG_INFO(LOG_APP, "resumeScreenRecording called");
    }

    bool ParaEngineHelper::getScreenRecordingStatus() {
        // TODO: 实现获取屏幕录制状态功能
        return false;
    }

    // 文件操作
    std::string ParaEngineHelper::readAssetFile(const std::string& fileName) {
        // TODO: 实现读取资源文件功能
        OH_LOG_INFO(LOG_APP, "readAssetFile called for: %s", fileName.c_str());
        return "";
    }

    bool ParaEngineHelper::writeFile(const std::string& fileName, const std::string& content) {
        // TODO: 实现写文件功能
        OH_LOG_INFO(LOG_APP, "writeFile called for: %s", fileName.c_str());
        return true;
    }

    bool ParaEngineHelper::fileExists(const std::string& fileName) {
        // TODO: 实现文件存在性检查功能
        OH_LOG_INFO(LOG_APP, "fileExists called for: %s", fileName.c_str());
        return false;
    }

    // 网络状态
    bool ParaEngineHelper::isNetworkAvailable() {
        // TODO: 实现网络可用性检查功能
        return true;
    }

    std::string ParaEngineHelper::getNetworkType() {
        // TODO: 实现获取网络类型功能
        return "wifi";
    }

    // 静态回调方法
    void ParaEngineHelper::onEditBoxResult(const std::string& callbackId, const std::string& result) {
        OH_LOG_INFO(LOG_APP, "onEditBoxResult called");
    }

    void ParaEngineHelper::onWebViewEvent(const std::string& event, const std::string& data) {
        OH_LOG_INFO(LOG_APP, "onWebViewEvent called: %s", event.c_str());
    }

    void ParaEngineHelper::onPermissionResult(const std::string& permission, bool granted) {
        OH_LOG_INFO(LOG_APP, "onPermissionResult called: %s = %s", permission.c_str(), granted ? "granted" : "denied");
    }

    void ParaEngineHelper::onBluetoothEvent(const std::string& event, const std::string& data) {
        OH_LOG_INFO(LOG_APP, "onBluetoothEvent called: %s", event.c_str());
    }

    void ParaEngineHelper::onScreenRecordEvent(const std::string& event, const std::string& data) {
        OH_LOG_INFO(LOG_APP, "onScreenRecordEvent called: %s", event.c_str());
    }

    // 辅助方法
    napi_value ParaEngineHelper::createArgsArray(const std::vector<std::string>& stringArgs) {
        if (!m_env) {
            return nullptr;
        }

        napi_value array;
        napi_status status = napi_create_array_with_length(m_env, stringArgs.size(), &array);
        if (status != napi_ok) {
            return nullptr;
        }

        for (size_t i = 0; i < stringArgs.size(); i++) {
            napi_value stringValue;
            status = napi_create_string_utf8(m_env, stringArgs[i].c_str(), NAPI_AUTO_LENGTH, &stringValue);
            if (status == napi_ok) {
                napi_set_element(m_env, array, i, stringValue);
            }
        }

        return array;
    }

    std::string ParaEngineHelper::callArkTSMethodWithStringResult(const std::string& methodName, const std::vector<std::string>& args) {
        if (!m_initialized || !m_env) {
            return "";
        }

        napi_value argsArray = createArgsArray(args);
        if (!argsArray) {
            return "";
        }

        std::vector<napi_value> napiArgs = { argsArray };
        napi_value result;
        if (callArkTSMethod(methodName, napiArgs, &result)) {
            size_t length;
            napi_status status = napi_get_value_string_utf8(m_env, result, nullptr, 0, &length);
            if (status == napi_ok && length > 0) {
                std::string resultStr(length, '\0');
                status = napi_get_value_string_utf8(m_env, result, &resultStr[0], length + 1, &length);
                if (status == napi_ok) {
                    return resultStr;
                }
            }
        }

        return "";
    }

    bool ParaEngineHelper::callArkTSMethodWithBoolResult(const std::string& methodName, const std::vector<std::string>& args) {
        if (!m_initialized || !m_env) {
            return false;
        }

        napi_value argsArray = createArgsArray(args);
        if (!argsArray) {
            return false;
        }

        std::vector<napi_value> napiArgs = { argsArray };
        napi_value result;
        if (callArkTSMethod(methodName, napiArgs, &result)) {
            bool boolResult;
            napi_status status = napi_get_value_bool(m_env, result, &boolResult);
            if (status == napi_ok) {
                return boolResult;
            }
        }

        return false;
    }

} // namespace ParaEngine