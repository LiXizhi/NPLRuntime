//-----------------------------------------------------------------------------
// NAPIHelper.cpp - HarmonyOS NAPI Helper Implementation
// Authors: LanZhiHong, big
// CreateDate: 2025.8.4
// ModifyDate: 2025.8.4
//-----------------------------------------------------------------------------

#include "NAPIHelper.h"
#include <hilog/log.h>

#undef LOG_TAG
#define LOG_TAG "ParaEngine"

namespace ParaEngine {

    // 错误处理
    bool NAPIHelper::checkError(napi_env env, napi_status status, const char* message) {
        if (status != napi_ok) {
            OH_LOG_ERROR(LOG_APP, "NAPI Error: %s", message);
            return false;
        }
        return true;
    }

    void NAPIHelper::throwError(napi_env env, const char* message) {
        napi_throw_error(env, nullptr, message);
    }

    // 类型转换
    napi_value NAPIHelper::createString(napi_env env, const std::string& str) {
        napi_value result;
        napi_status status = napi_create_string_utf8(env, str.c_str(), str.length(), &result);
        return (status == napi_ok) ? result : nullptr;
    }

    napi_value NAPIHelper::createNumber(napi_env env, double value) {
        napi_value result;
        napi_status status = napi_create_double(env, value, &result);
        return (status == napi_ok) ? result : nullptr;
    }

    napi_value NAPIHelper::createBoolean(napi_env env, bool value) {
        napi_value result;
        napi_status status = napi_get_boolean(env, value, &result);
        return (status == napi_ok) ? result : nullptr;
    }

    napi_value NAPIHelper::createObject(napi_env env) {
        napi_value result;
        napi_status status = napi_create_object(env, &result);
        return (status == napi_ok) ? result : nullptr;
    }

    napi_value NAPIHelper::createArray(napi_env env, size_t length) {
        napi_value result;
        napi_status status = napi_create_array_with_length(env, length, &result);
        return (status == napi_ok) ? result : nullptr;
    }

    napi_value NAPIHelper::createNull(napi_env env) {
        napi_value result;
        napi_status status = napi_get_null(env, &result);
        return (status == napi_ok) ? result : nullptr;
    }

    napi_value NAPIHelper::createUndefined(napi_env env) {
        napi_value result;
        napi_status status = napi_get_undefined(env, &result);
        return (status == napi_ok) ? result : nullptr;
    }

    // 值获取
    std::string NAPIHelper::getString(napi_env env, napi_value value) {
        size_t str_size = 0;
        napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &str_size);
        if (status != napi_ok || str_size == 0) {
            return "";
        }
        
        std::string result(str_size, '\0');
        status = napi_get_value_string_utf8(env, value, &result[0], str_size + 1, nullptr);
        return (status == napi_ok) ? result : "";
    }

    double NAPIHelper::getNumber(napi_env env, napi_value value) {
        double result = 0.0;
        napi_status status = napi_get_value_double(env, value, &result);
        return (status == napi_ok) ? result : 0.0;
    }

    bool NAPIHelper::getBoolean(napi_env env, napi_value value) {
        bool result = false;
        napi_status status = napi_get_value_bool(env, value, &result);
        return (status == napi_ok) ? result : false;
    }

    int32_t NAPIHelper::getInt32(napi_env env, napi_value value) {
        int32_t result = 0;
        napi_status status = napi_get_value_int32(env, value, &result);
        return (status == napi_ok) ? result : 0;
    }

    uint32_t NAPIHelper::getUint32(napi_env env, napi_value value) {
        uint32_t result = 0;
        napi_status status = napi_get_value_uint32(env, value, &result);
        return (status == napi_ok) ? result : 0;
    }

    // 对象操作
    napi_value NAPIHelper::getProperty(napi_env env, napi_value object, const std::string& key) {
        napi_value result;
        napi_status status = napi_get_named_property(env, object, key.c_str(), &result);
        return (status == napi_ok) ? result : nullptr;
    }

    bool NAPIHelper::setProperty(napi_env env, napi_value object, const std::string& key, napi_value value) {
        napi_status status = napi_set_named_property(env, object, key.c_str(), value);
        return (status == napi_ok);
    }

    bool NAPIHelper::hasProperty(napi_env env, napi_value object, const std::string& key) {
        bool result = false;
        napi_status status = napi_has_named_property(env, object, key.c_str(), &result);
        return (status == napi_ok) ? result : false;
    }

    std::vector<std::string> NAPIHelper::getPropertyNames(napi_env env, napi_value object) {
        std::vector<std::string> result;
        napi_value names;
        napi_status status = napi_get_property_names(env, object, &names);
        if (status != napi_ok) {
            return result;
        }
        
        uint32_t length = getArrayLength(env, names);
        for (uint32_t i = 0; i < length; i++) {
            napi_value name = getArrayElement(env, names, i);
            if (name != nullptr) {
                result.push_back(getString(env, name));
            }
        }
        return result;
    }

    // 数组操作
    uint32_t NAPIHelper::getArrayLength(napi_env env, napi_value array) {
        uint32_t result = 0;
        napi_status status = napi_get_array_length(env, array, &result);
        return (status == napi_ok) ? result : 0;
    }

    napi_value NAPIHelper::getArrayElement(napi_env env, napi_value array, uint32_t index) {
        napi_value result;
        napi_status status = napi_get_element(env, array, index, &result);
        return (status == napi_ok) ? result : nullptr;
    }

    bool NAPIHelper::setArrayElement(napi_env env, napi_value array, uint32_t index, napi_value value) {
        napi_status status = napi_set_element(env, array, index, value);
        return (status == napi_ok);
    }

    // 类型检查
    bool NAPIHelper::isString(napi_env env, napi_value value) {
        napi_valuetype type;
        napi_status status = napi_typeof(env, value, &type);
        return (status == napi_ok) && (type == napi_string);
    }

    bool NAPIHelper::isNumber(napi_env env, napi_value value) {
        napi_valuetype type;
        napi_status status = napi_typeof(env, value, &type);
        return (status == napi_ok) && (type == napi_number);
    }

    bool NAPIHelper::isBoolean(napi_env env, napi_value value) {
        napi_valuetype type;
        napi_status status = napi_typeof(env, value, &type);
        return (status == napi_ok) && (type == napi_boolean);
    }

    bool NAPIHelper::isObject(napi_env env, napi_value value) {
        napi_valuetype type;
        napi_status status = napi_typeof(env, value, &type);
        return (status == napi_ok) && (type == napi_object);
    }

    bool NAPIHelper::isArray(napi_env env, napi_value value) {
        bool result = false;
        napi_status status = napi_is_array(env, value, &result);
        return (status == napi_ok) && result;
    }

    bool NAPIHelper::isFunction(napi_env env, napi_value value) {
        napi_valuetype type;
        napi_status status = napi_typeof(env, value, &type);
        return (status == napi_ok) && (type == napi_function);
    }

    bool NAPIHelper::isNull(napi_env env, napi_value value) {
        napi_valuetype type;
        napi_status status = napi_typeof(env, value, &type);
        return (status == napi_ok) && (type == napi_null);
    }

    bool NAPIHelper::isUndefined(napi_env env, napi_value value) {
        napi_valuetype type;
        napi_status status = napi_typeof(env, value, &type);
        return (status == napi_ok) && (type == napi_undefined);
    }

} // namespace ParaEngine
