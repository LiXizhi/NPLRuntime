//-----------------------------------------------------------------------------
// Class: CCLuaObjcBridge.h
// Authors: big
// Emails: onedou@126.com
// CreateDate: 2023.3.23
// ModifyDate: 2024.4.24
//-----------------------------------------------------------------------------

#ifndef __LUA_OBJC_BRIDGE_H_
#define __LUA_OBJC_BRIDGE_H_

#include <string>
#include <map>
#include <iostream>

namespace luabind
{
    namespace adl
    {
        class object;
    }
};

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace ParaEngine
{
    /** LuaBridge Error enum, with inline docs. */
    typedef enum
    {
        kLuaBridgeErrorOk = 0,                 /** &lt; value 0 */
        kLuaBridgeErrorInvalidParameters = -1, /** &lt; value -1 */
        kLuaBridgeErrorClassNotFound = -2,     /** &lt; value -2 */
        kLuaBridgeErrorMethodNotFound = -3,    /** &lt; value -3 */
        kLuaBridgeErrorExceptionOccurred = -4, /** &lt; value -4 */
        kLuaBridgeErrorMethodSignature = -5,   /** &lt; value -5 */
        kLuaBridgeErrorJavaVMError = -6,       /** &lt; value -6 */
    } LuaBridgeError;

    /**
     * Build a bridge between ObjectC and Lua script.
     * This mechanism make Lua and ObjectC call each other easily.
     */
    class LuaObjcBridge
    {
    public:
        /**
         * Bind callObjcStaticMethod of LuaObjcBridge to Lua.
         * In current mechanism,we could call LuaObjcBridge.callStaticMethod(className, methodName, args) in Lua directly.
         * Meanwhile the callObjcStaticMethod of LuaObjcBridge binding function is wrapped in the luaoc.lua
         *
         * @param L the current lua_State
         * @js NA
         * @lua NA
         */
        static void luaopen_luaoc(lua_State *L);
        static void nplActivate(const std::string &msg, const std::string &strNPLFileName);

    protected:
        static luabind::adl::object callObjcStaticMethod(const std::string &className, const std::string &functionName, const luabind::adl::object &argvs, lua_State *L);
        
    public:
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
            TypeNSDictionary = 12,
        } ValueType;
        
        typedef union
        {
            int intValue;
            float floatValue;
            short shortValue;
            char charValue;
            bool boolValue;
            long long longValue;
            double doubleValue;
        } ReturnValue;

        class ReturnObject
        {
        public:
            struct Handle
            {
                Handle();
                ~Handle();

                void setData(const ValueType &valueType, const ReturnValue &returnValue);
                void setData(const ValueType &valueType, void *oc_id);

                void *m_obj;
                ValueType _valueType;
                ReturnValue _returnValue;
            };

            ReturnObject(const ValueType &valueType, const ReturnValue &returnValue);
            ReturnObject(const ValueType &valueType, void *oc_id);
            ReturnObject(ReturnObject const &other);
            ReturnObject &operator=(ReturnObject const &other);

            const ValueType &getReturnType() const;
            const ReturnValue &getReturnVaule() const;

            const char *getStrVaule() const;

            void *getOcObj();

            void getDictionaryMap(std::map<std::string, LuaObjcBridge::ReturnObject> &mapTarget) const;

        protected:
            std::shared_ptr<Handle> _handle;
        };

        static luabind::adl::object ReturnObject2LuabindObject(lua_State *L, const ReturnObject &returnObject);

        struct OcFunction
        {
            OcFunction();
            ~OcFunction();
            void pushOcValue(const std::string &key, const std::string &value);
            void pushOcValue(const std::string &key, const float &value);
            void pushOcValue(const std::string &key, const bool value);
            void *argvDatas;

            ReturnObject callFunction(const std::string &className_, const std::string &methodName_);
        };
    };
}

#endif
