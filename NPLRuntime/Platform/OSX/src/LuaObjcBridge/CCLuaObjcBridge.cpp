//-----------------------------------------------------------------------------
// Class: CCLuaObjcBridge.h
// Authors: big
// Emails: onedou@126.com
// CreateDate: 2023.3.23
// ModifyDate: 2024.4.24
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "NPLScriptingState.h"

#include "CCLuaObjcBridge.h"

#include <luabind/luabind.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include <luabind/copy_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/function.hpp>
#include <luabind/raw_policy.hpp>
#include <luabind/object.hpp>

namespace ParaEngine
{
    void LuaObjcBridge::luaopen_luaoc(lua_State *L)
    {
        using namespace luabind;

        module(L)
            [namespace_("LuaObjcBridge")
                 [def("callStaticMethod", &LuaObjcBridge::callObjcStaticMethod, raw(_4))]];
    }

    /**
     className
     methodName
     args
     */
    luabind::object LuaObjcBridge::callObjcStaticMethod(const std::string &className_, const std::string &methodName_, const luabind::object &argvs, lua_State *L)
    {
        luabind::object ret;

        const char *className = className_.c_str();
        const char *methodName = methodName_.c_str();

        if (!className || !methodName)
        {
            return ret;
        }

        OcFunction ocfun;

        luabind::iterator itr(argvs), end;

        while (itr != end)
        {
            const luabind::object &o = *itr;
            auto objectType = luabind::type(o);
            string key_str = object_cast<string>(itr.key());

            switch (objectType)
            {
            case LUA_TNUMBER:
            {
                float iNumer = luabind::object_cast<float>(o);
                ocfun.pushOcValue(key_str, iNumer);
                break;
            }
            case LUA_TBOOLEAN:
            {
                bool iBool = luabind::object_cast<bool>(o);
                ocfun.pushOcValue(key_str, iBool);
                break;
            }
            case LUA_TSTRING:
            {
                string iString = object_cast<string>(o);
                ocfun.pushOcValue(key_str, iString);
                break;
            }
            case LUA_TFUNCTION:
            {
                break;
            }
            }

            ++itr;
        }

        return ReturnObject2LuabindObject(L, ocfun.callFunction(className_, methodName_));
    }

    luabind::object LuaObjcBridge::ReturnObject2LuabindObject(lua_State *L, const ReturnObject &ocObject)
    {
        luabind::object ret;

        if (ocObject.getReturnType() == TypeString)
        {
            std::string s(ocObject.getStrVaule());
            ret = luabind::object(L, s);
        }
        else if (ocObject.getReturnType() == TypeFloat)
            ret = luabind::object(L, ocObject.getReturnVaule().floatValue);
        else if (ocObject.getReturnType() == TypeInteger)
            ret = luabind::object(L, ocObject.getReturnVaule().intValue);
        else if (ocObject.getReturnType() == TypeBoolean)
            ret = luabind::object(L, ocObject.getReturnVaule().boolValue);
        else if (ocObject.getReturnType() == TypeObject)
            ret = luabind::object(L, ocObject);
        else if (ocObject.getReturnType() == TypeNSDictionary)
        {
            typedef std::map<std::string, LuaObjcBridge::ReturnObject> ReturnObjMap;
            ReturnObjMap map_;
            ocObject.getDictionaryMap(map_);
            ret = luabind::newtable(L);
            for (auto &itr : map_)
            {
                const char *key = itr.first.c_str();
                ret[key] = ReturnObject2LuabindObject(L, itr.second);
            }
        }

        return ret;
    }

    void LuaObjcBridge::nplActivate(const std::string &msg, const std::string &strNPLFileName)
    {
        NPL::NPLRuntimeState_ptr rsptr = NPL::CNPLRuntime::GetInstance()->GetRuntimeState(strNPLFileName);
        NPL::CNPLRuntime::GetInstance()->NPL_Activate(rsptr, strNPLFileName.c_str(), msg.c_str());
    }
}

extern "C"
{
    /** to load the lib, please call:
     NPL.call("LuaJavaBridge.cpp", {});
     NPL.activate("LuaJavaBridge.cpp");
     */
    PE_CORE_DECL NPL::NPLReturnCode NPL_activate_LuaObjcBridge_cpp(NPL::INPLRuntimeState *pState)
    {
        auto pRuntimeState = ParaEngine::CGlobals::GetNPLRuntime()->GetRuntimeState(pState->GetName());
        ParaEngine::LuaObjcBridge::luaopen_luaoc(pRuntimeState->GetLuaState());
        OUTPUT_LOG("LuaObjcBridge loaded\n");
        return NPL::NPL_OK;
    };
}
