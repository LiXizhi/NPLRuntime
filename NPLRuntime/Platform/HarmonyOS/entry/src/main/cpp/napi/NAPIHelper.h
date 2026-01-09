//-----------------------------------------------------------------------------
// NAPIHelper.h - HarmonyOS NAPI Helper
// Authors: big
// CreateDate: 2025.8.4
//-----------------------------------------------------------------------------

#pragma once

#include <napi/native_api.h>
#include <string>
#include <vector>
#include <hilog/log.h>

namespace ParaEngine {

    /**
     * NAPI辅助工具类
     * 提供NAPI相关的便捷方法
     */
    class NAPIHelper {
    public:
        // 错误处理
        static bool checkError(napi_env env, napi_status status, const char* message);
        static void throwError(napi_env env, const char* message);
        
        // 类型转换
        static napi_value createString(napi_env env, const std::string& str);
        static napi_value createNumber(napi_env env, double value);
        static napi_value createBoolean(napi_env env, bool value);
        static napi_value createObject(napi_env env);
        static napi_value createArray(napi_env env, size_t length = 0);
        static napi_value createNull(napi_env env);
        static napi_value createUndefined(napi_env env);
        
        // 值获取
        static std::string getString(napi_env env, napi_value value);
        static double getNumber(napi_env env, napi_value value);
        static bool getBoolean(napi_env env, napi_value value);
        static int32_t getInt32(napi_env env, napi_value value);
        static uint32_t getUint32(napi_env env, napi_value value);
        
        // 对象操作
        static napi_value getProperty(napi_env env, napi_value object, const std::string& key);
        static bool setProperty(napi_env env, napi_value object, const std::string& key, napi_value value);
        static bool hasProperty(napi_env env, napi_value object, const std::string& key);
        static std::vector<std::string> getPropertyNames(napi_env env, napi_value object);
        
        // 数组操作
        static uint32_t getArrayLength(napi_env env, napi_value array);
        static napi_value getArrayElement(napi_env env, napi_value array, uint32_t index);
        static bool setArrayElement(napi_env env, napi_value array, uint32_t index, napi_value value);
        
        // 函数调用
        static napi_value callFunction(napi_env env, napi_value object, const std::string& functionName, 
                                     const std::vector<napi_value>& args);
        static napi_value callFunction(napi_env env, napi_value function, napi_value thisArg,
                                     const std::vector<napi_value>& args);
        
        // 类型检查
        static bool isString(napi_env env, napi_value value);
        static bool isNumber(napi_env env, napi_value value);
        static bool isBoolean(napi_env env, napi_value value);
        static bool isObject(napi_env env, napi_value value);
        static bool isArray(napi_env env, napi_value value);
        static bool isFunction(napi_env env, napi_value value);
        static bool isNull(napi_env env, napi_value value);
        static bool isUndefined(napi_env env, napi_value value);
        
        // 引用管理
        static napi_ref createReference(napi_env env, napi_value value, uint32_t refCount = 1);
        static napi_value getReferenceValue(napi_env env, napi_ref ref);
        static void deleteReference(napi_env env, napi_ref ref);
        
        // 异步操作
        static napi_value createPromise(napi_env env, napi_deferred* deferred);
        static void resolvePromise(napi_env env, napi_deferred deferred, napi_value result);
        static void rejectPromise(napi_env env, napi_deferred deferred, napi_value error);
        
        // 回调管理
        static napi_value createCallback(napi_env env, napi_callback callback, void* data = nullptr);
        
        // 线程安全调用
        static bool createThreadSafeFunction(napi_env env, napi_value func, const std::string& name,
                                           napi_threadsafe_function* tsFunc);
        static void callThreadSafeFunction(napi_threadsafe_function tsFunc, void* data);
        static void releaseThreadSafeFunction(napi_threadsafe_function tsFunc);
        
        // JSON操作
        static napi_value parseJSON(napi_env env, const std::string& jsonStr);
        static std::string stringifyJSON(napi_env env, napi_value object);
        
        // 日志输出
        static void logDebug(const char* tag, const char* format, ...);
        static void logInfo(const char* tag, const char* format, ...);
        static void logWarn(const char* tag, const char* format, ...);
        static void logError(const char* tag, const char* format, ...);
        
    private:
        // 禁止实例化
        NAPIHelper() = delete;
        ~NAPIHelper() = delete;
        
        static const char* getErrorMessage(napi_status status);
    };

    // 便捷宏定义
    #define NAPI_CALL(env, call) \
        do { \
            napi_status status = (call); \
            if (status != napi_ok) { \
                NAPIHelper::checkError(env, status, #call); \
                return nullptr; \
            } \
        } while(0)

    #define NAPI_CALL_RETURN_VOID(env, call) \
        do { \
            napi_status status = (call); \
            if (status != napi_ok) { \
                NAPIHelper::checkError(env, status, #call); \
                return; \
            } \
        } while(0)

    #define NAPI_CALL_RETURN_BOOL(env, call) \
        do { \
            napi_status status = (call); \
            if (status != napi_ok) { \
                NAPIHelper::checkError(env, status, #call); \
                return false; \
            } \
        } while(0)

} // namespace ParaEngine
