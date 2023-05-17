//-----------------------------------------------------------------------------
// LuaJavaBridge.h
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2023.5.17
//-----------------------------------------------------------------------------

#pragma once

#include "jni/JniHelper.h"

/**
for luabind, The main drawback of this approach is that the compilation time will increase for the file
that does the registration, it is therefore recommended that you register everything in the same cpp-file.
*/
extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

#include <luabind/luabind.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include <luabind/copy_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/function.hpp>
#include <luabind/raw_policy.hpp>
#include <luabind/object.hpp>

#include <boost/thread/tss.hpp>
#include <unordered_map>
#include <string>
#include <vector>

namespace ParaEngine {

    const int LUAJ_ERR_OK = 0;
    const int LUAJ_ERR_TYPE_NOT_SUPPORT = (-1);
    const int LUAJ_ERR_INVALID_SIGNATURES = (-2);
    const int LUAJ_ERR_METHOD_NOT_FOUND = (-3);
    const int LUAJ_ERR_EXCEPTION_OCCURRED = (-4);
    const int LUAJ_ERR_VM_THREAD_DETACHED = (-5);
    const int LUAJ_ERR_VM_FAILURE = (-6);

    /**
    * Build a bridge between Java and Lua script.
    * This mechanism make Lua and Java call each other easily.
    */
    class LuaJavaBridge
    {
    public:
        ///@cond
        /**
        * Bind callStaticMethod of LuaJavaBridge to Lua.
        * In current mechanism, we could call LuaJavaBridge.callStaticMethod(className, methodName, sig, args) in Lua directly.
        * Meanwhile the callStaticMethod of LuaObjcBridge binding function is wrapped in the luaj.lua
        */
        static void luaopen_luaj(lua_State *L);


        /**
        * Add a reference count for the Lua functionId,and save this reference in the Lua table named luaj_function_id_retain.
        *
        * @param functionId the id of Lua function.
        * @return the reference count of the functionId if luaj_function_id_retain table exists and the corresponding value for functionId exists, otherwise return 0.
        *
        * @lua NA
        * @js NA
        */
        static int retainLuaFunctionById(int functionId);

        /**
        * Release a reference count for the Lua functionId, If the reference count is still greater than 0,save this reference in the Lua table named luaj_function_id_retain.
        * Otherwise, remove the reference about this functionId in the luaj_function_id table and the luaj_function_id_retain table by set the corresponding value nil.
        *
        * @param functionId the id of Lua function.
        * @return the reference count of the functionId if the luaj_function_id table, the luaj_function_id_retain table and the corresponding value for functionId exists a reference count for the Lua functionId is still greater than 0, and otherwise return 0.
        *
        * @lua NA
        * @js NA
        */
        static int releaseLuaFunctionById(int functionId);

        /**
        * Call the Lua function corresponding to the functionId with the string pointer arg.
        *
        * @param functionId the values corresponding to the Lua function.
        * @param arg the string pointer point to the argument.
        * @return a number value returned from the Lua function when call successfully, otherwise return -1 or the opposite number for one of the three numbers LUA_ERRRUN, LUA_ERRMEM and LUA_ERRERR.
        *
        * @lua NA
        * @js NA
        */
        static int callLuaFunctionById(int functionId, const char *arg);

        static void nplActivate(const std::string &msg, const std::string &strNPLFileName);
    private:
        typedef enum
        {
            TypeInvalid = -1,
            TypeVoid = 0,
            TypeInteger = 1,
            TypeFloat = 2,
            TypeBoolean = 3,
            TypeString = 4,
            TypeVector = 5,
            TypeFunction = 6,
            TypeShort = 7,
            TypeLong = 8,
            TypeDouble = 9,
            TypeChar = 10,
            TypeObject = 11,
        } ValueType;

        typedef std::vector<ValueType> ValueTypes;

        typedef union
        {
            int     intValue;
            float   floatValue;
            short	shortValue;
            char	charValue;
            bool    boolValue;
            long long longValue;
            double	doubleValue;
            //std::string stringValue;
            const char* stringValue;
            jobject	objValue;
        } ReturnValue;

        typedef struct
        {
            ValueTypes argvs;
            ValueType returnType;

        } FunctionInfo;


        struct JValues
        {
            JValues() : jargs(nullptr) {};
            JValues(size_t i)
                : jargs(nullptr)
            {
                create(i);
            }

            void create(size_t i)
            {
                jargs = new jvalue[i];
            }

            void clear()
            {
                if (jargs)
                {
                    for (size_t i = 0; i < releaseIndex.size(); i++)
                    {
                        auto index = releaseIndex[i];
                        auto jstr = (jstring)jargs[index].l;

                        JniHelper::getEnv()->DeleteLocalRef(jstr);
                    }
                    releaseIndex.clear();

                    delete[] jargs;
                    jargs = nullptr;
                }
            }

            void needRelease(size_t i)
            {
                releaseIndex.push_back(i);
            }

            ~JValues()
            {
                clear();
            }


            std::vector<size_t> releaseIndex;
            jvalue *jargs;
        };

        struct JObject
        {
            struct Handle
            {
                Handle(jobject o) : m_obj(o) {};
                Handle() : m_obj(nullptr) {};
                ~Handle()
                {
                    if (m_obj)
                        JniHelper::getEnv()->DeleteGlobalRef(m_obj);
                }

                jobject m_obj;
            };

            JObject()
            {

            }

            JObject(jobject o)
            {
                set(o);
            };

            JObject(JObject const&  other)
            {
                m_handle = other.m_handle;
            }

            JObject& operator=(JObject const& other)
            {
                m_handle = other.m_handle;
                return *this;
            }

            jobject get() const
            {
                if (m_handle.get())
                    return m_handle->m_obj;
                else
                    return nullptr;
            };

            void set(jobject o)
            {
                if (o)
                    o = JniHelper::getEnv()->NewGlobalRef(o);
                m_handle.reset(new Handle(o));
            }


            std::shared_ptr<Handle> m_handle;
        };

        static luabind::object callJavaStaticMethod(const std::string& className, const std::string& functionName, const std::string& sig, const luabind::object& argvs, lua_State *L);
        static luabind::object callJavaMethod(JObject& o, const std::string& functionName, const std::string& sig, const luabind::object& argvs, lua_State *L);
        static void releaseJavaObjcet(jobject o);

        static int validateMethodSig(const std::string& sig, FunctionInfo& info);
        static ValueType checkType(const std::string& sig, size_t *pos);
        static bool convertType(JNIEnv* env, JValues& values, size_t index, ValueType type, const luabind::object& o);
        static bool buildJavaValues(JniMethodInfo& t, FunctionInfo& info, const luabind::object& argvs, JValues& ret);

        static bool callStaticVoidMethod(JniMethodInfo& t, FunctionInfo& info, const luabind::object& argvs);
        static bool callStaticIntMethod(JniMethodInfo& t, FunctionInfo& info, const luabind::object& argvs, int& ret);
        static bool callStaticFloatMethod(JniMethodInfo& t, FunctionInfo& info, const luabind::object& argvs, float& ret);
        static bool callStaticBooleanMethod(JniMethodInfo& t, FunctionInfo& info, const luabind::object& argvs, bool& ret);
        static bool callStaticStringMethod(JniMethodInfo& t, FunctionInfo& info, const luabind::object& argvs, std::string& ret);
        static bool callStaticObjectMethod(JniMethodInfo& t, FunctionInfo& info, const luabind::object& argvs, JObject& ret);


        static bool callVoidMethod(JniMethodInfo& t, FunctionInfo& info, JObject& jobj, const luabind::object& argvs);
        static bool callIntMethod(JniMethodInfo& t, FunctionInfo& info, JObject& jobj, const luabind::object& argvs, int& ret);
        static bool callFloatMethod(JniMethodInfo& t, FunctionInfo& info, JObject& jobj, const luabind::object& argvs, float& ret);
        static bool callBooleanMethod(JniMethodInfo& t, FunctionInfo& info, JObject& jobj, const luabind::object& argvs, bool& ret);
        static bool callStringMethod(JniMethodInfo& t, FunctionInfo& info, JObject& jobj, const luabind::object& argvs, std::string& ret);
        static bool callObjectMethod(JniMethodInfo& t, FunctionInfo& info, JObject& jobj, const luabind::object& argvs, JObject& ret);

        struct FuncInfo
        {
            luabind::object funcObj;
            int retainCount;
        };
        typedef std::unordered_map<int, FuncInfo> FuncMap;

        static boost::thread_specific_ptr<FuncMap> _funcMap;

        static int retainLuaFunction(const luabind::object& funcObj);
    };
}
