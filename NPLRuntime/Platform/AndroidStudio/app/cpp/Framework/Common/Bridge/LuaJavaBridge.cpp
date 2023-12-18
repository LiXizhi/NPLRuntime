//-----------------------------------------------------------------------------
// LuaJavaBridge.cpp
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2023.5.17
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "LuaJavaBridge.h"
#include "NPLRuntime.h"
#include "NPLScriptingState.h"

namespace ParaEngine {
    void LuaJavaBridge::nplActivate(const std::string &msg, const std::string &strNPLFileName)
    {
        NPL::NPLRuntimeState_ptr rsptr = NPL::CNPLRuntime::GetInstance()->GetRuntimeState(strNPLFileName);
        NPL::CNPLRuntime::GetInstance()->NPL_Activate(rsptr, strNPLFileName.c_str(), msg.c_str());
    }

    bool LuaJavaBridge::convertType(JNIEnv *env, JValues &values, size_t index, ValueType type, const luabind::object &o)
    {
        auto objectType = luabind::type(o);

        jvalue& value = values.jargs[index];

        switch (type)
        {
            case TypeInteger:
            {
                if (objectType == LUA_TNUMBER)
                {
                    value.i = luabind::object_cast<int>(o);
                    return true;
                }
                else if (objectType == LUA_TFUNCTION)
                {
                    value.i = retainLuaFunction(o);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            case TypeFloat:
            {
                if (objectType == LUA_TNUMBER)
                {
                    value.f = luabind::object_cast<float>(o);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            case TypeBoolean:
            {
                if (objectType == LUA_TBOOLEAN)
                {
                    value.z = luabind::object_cast<bool>(o);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            case TypeString:
            {
                if (objectType == LUA_TSTRING)
                {
                    const char* p = luabind::object_cast<const char*>(o);
                    value.l = env->NewStringUTF(p);
                    values.needRelease(index);

                    return true;
                }
                else
                {
                    return false;
                }
            }
            
            case TypeShort:
            {
                if (objectType == LUA_TNUMBER)
                {
                    value.s = luabind::object_cast<short>(o);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            case TypeLong:
            {
                if (objectType == LUA_TNUMBER)
                {
                    value.j = luabind::object_cast<long long>(o);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            case TypeDouble:
            {
                if (objectType == LUA_TNUMBER)
                {
                    value.d = luabind::object_cast<double>(o);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            case TypeChar:
            {
                if (objectType == LUA_TNUMBER)
                {
                    value.c = luabind::object_cast<char>(o);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            case TypeObject:
            {
                if (objectType == LUA_TUSERDATA)
                {
                    try
                    {
                        JObject obj = luabind::object_cast<JObject>(o);
                        value.l = obj.get();
                    }
                    catch (...)
                    {
                        return false;
                    }
                }
                else
                    return false;
            }

            default:
            {
                return false;
            }
        }
    }

    bool LuaJavaBridge::buildJavaValues(JniMethodInfo &t,
                                        FunctionInfo &info,
                                        const luabind::object &argvs,
                                        JValues &ret)
    {
        auto count = info.argvs.size();

        luabind::iterator itr(argvs), end;

        ret.create(count);

        for (size_t i = 0; i < count; i++)
        {
            if (itr == end)
            {
                OUTPUT_LOG("param not enough, need = %d, has = %d", (int)info.argvs.size(), (int)i);
                return false;
            }


            auto& arg = info.argvs[i];
            if (!convertType(t.env, ret, i, arg, *itr))
            {
                OUTPUT_LOG("param type '%d' is not supported, index = %d", static_cast<int>(arg), (int)i);
                return false;
            }

            itr++;
        }

        return true;
    }

    bool LuaJavaBridge::callVoidMethod(JniMethodInfo &t,
                                       FunctionInfo &info,
                                       JObject &jobj,
                                       const luabind::object &argvs)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            t.env->CallVoidMethodA(jobj.get(), t.methodID, v.jargs);
        }
        else
        {
            t.env->CallVoidMethod(jobj.get(), t.methodID);
        }

        return true;
    }

    bool LuaJavaBridge::callIntMethod(JniMethodInfo &t,
                                      FunctionInfo &info,
                                      JObject &jobj,
                                      const luabind::object &argvs,
                                      int &ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            ret = t.env->CallIntMethodA(jobj.get(), t.methodID, v.jargs);
        }
        else
        {
            ret = t.env->CallIntMethod(jobj.get(), t.methodID);
        }

        return true;
    }

    bool LuaJavaBridge::callFloatMethod(JniMethodInfo &t,
                                        FunctionInfo &info,
                                        JObject &jobj,
                                        const luabind::object &argvs,
                                        float &ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            ret = t.env->CallFloatMethodA(jobj.get(), t.methodID, v.jargs);
        }
        else
        {
            ret = t.env->CallFloatMethod(jobj.get(), t.methodID);
        }

        return true;
    }

    bool LuaJavaBridge::callBooleanMethod(JniMethodInfo &t,
                                          FunctionInfo &info,
                                          JObject &jobj,
                                          const luabind::object &argvs, bool& ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            ret = t.env->CallBooleanMethodA(jobj.get(), t.methodID, v.jargs);
        }
        else
        {
            ret = t.env->CallBooleanMethod(jobj.get(), t.methodID);
        }

        return true;
    }

    bool LuaJavaBridge::callStringMethod(JniMethodInfo &t,
                                         FunctionInfo &info,
                                         JObject &jobj,
                                         const luabind::object &argvs,
                                         std::string &ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            jstring _ret = (jstring)t.env->CallObjectMethodA(jobj.get(), t.methodID, v.jargs);

            ret = JniHelper::jstring2string(_ret);
            t.env->DeleteLocalRef(_ret);
        }
        else
        {
            jstring _ret = (jstring)t.env->CallObjectMethod(jobj.get(), t.methodID);
            ret = JniHelper::jstring2string(_ret);
            t.env->DeleteLocalRef(_ret);
        }

        return true;
    }

    bool LuaJavaBridge::callObjectMethod(JniMethodInfo &t,
                                         FunctionInfo &info,
                                         JObject &jobj,
                                         const luabind::object &argvs,
                                         JObject &ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            auto jo = t.env->CallObjectMethodA(jobj.get(), t.methodID, v.jargs);
            ret.set(jo);
            t.env->DeleteLocalRef(jo);
        }
        else
        {
            auto jo = t.env->CallObjectMethod(jobj.get(), t.methodID);
            ret.set(jo);
            t.env->DeleteLocalRef(jo);
        }

        return true;
    }

    bool LuaJavaBridge::callStaticFloatMethod(JniMethodInfo &t,
                                              FunctionInfo &info,
                                              const luabind::object &argvs,
                                              float &ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            ret = t.env->CallStaticFloatMethodA(t.classID, t.methodID, v.jargs);
        }
        else
        {
            ret = t.env->CallStaticFloatMethod(t.classID, t.methodID);
        }

        return true;
    }

    bool LuaJavaBridge::callStaticObjectMethod(JniMethodInfo &t,
                                               FunctionInfo &info,
                                               const luabind::object &argvs,
                                               JObject &ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            auto jobj = t.env->CallStaticObjectMethodA(t.classID, t.methodID, v.jargs);
            ret.set(jobj);
            t.env->DeleteLocalRef(jobj);
        }
        else
        {
            auto jobj = t.env->CallStaticObjectMethod(t.classID, t.methodID);
            ret.set(jobj);
            t.env->DeleteLocalRef(jobj);
        }

        return true;
    }

    bool LuaJavaBridge::callStaticStringMethod(JniMethodInfo &t,
                                               FunctionInfo &info,
                                               const luabind::object &argvs,
                                               std::string &ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            jstring _ret = (jstring)t.env->CallStaticObjectMethodA(t.classID, t.methodID, v.jargs);

            ret = JniHelper::jstring2string(_ret);
            t.env->DeleteLocalRef(_ret);
        }
        else
        {
            jstring _ret = (jstring)t.env->CallStaticObjectMethod(t.classID, t.methodID);
            ret = JniHelper::jstring2string(_ret);
            t.env->DeleteLocalRef(_ret);
        }

        return true;
    }


    bool LuaJavaBridge::callStaticBooleanMethod(JniMethodInfo &t,
                                                FunctionInfo &info,
                                                const luabind::object &argvs,
                                                bool &ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            ret = t.env->CallStaticBooleanMethodA(t.classID, t.methodID, v.jargs);
        }
        else
        {
            ret = t.env->CallStaticBooleanMethod(t.classID, t.methodID);
        }

        return true;
    }

    bool LuaJavaBridge::callStaticIntMethod(JniMethodInfo &t,
                                            FunctionInfo &info,
                                            const luabind::object &argvs,
                                            int &ret)
    {
        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            ret = t.env->CallStaticIntMethodA(t.classID, t.methodID, v.jargs);
        }
        else
        {
            ret = t.env->CallStaticIntMethod(t.classID, t.methodID);
        }

        return true;
    }

    bool LuaJavaBridge::callStaticVoidMethod(JniMethodInfo &t,
                                             FunctionInfo &info,
                                             const luabind::object &argvs)
    {

        auto count = info.argvs.size();

        if (count > 0)
        {
            JValues v;
            if (!buildJavaValues(t, info, argvs, v))
                return false;

            t.env->CallStaticVoidMethodA(t.classID, t.methodID, v.jargs);
        }
        else
        {
            t.env->CallStaticVoidMethod(t.classID, t.methodID);
        }

        return true;
    }

    luabind::object LuaJavaBridge::callJavaMethod(JObject& jobj,
                                                  const std::string& functionName,
                                                  const std::string& sig,
                                                  const luabind::object& argvs,
                                                  lua_State *L)
    {
        JniMethodInfo mInfo;
        bool callSuccess = false;

        if (!JniHelper::getMethodInfo(mInfo, jobj.get(), functionName.c_str(), sig.c_str()))
        {
            OUTPUT_LOG("callJavaMethod: Failed to find method id of %s %s", functionName.c_str(), sig.c_str());
            return object();
        }

        do
        {
            FunctionInfo info;
            auto err = validateMethodSig(sig, info);
            if (err != LUAJ_ERR_OK)
            {
                OUTPUT_LOG("callJavaMethod: Failed to validate sig of %s %s err = %d", functionName.c_str(), sig.c_str(), err);
                break;
            }

            auto argCount = info.argvs.size();
            auto argvsType = luabind::type(argvs);

            if (argCount > 0 && argvsType != LUA_TTABLE)
            {
                OUTPUT_LOG("callJavaMethod: argvs is Invalid ");
                break;
            }


            luabind::object ret;
            switch (info.returnType)
            {
                case TypeVoid:
                {
                    if (!callVoidMethod(mInfo, info, jobj, argvs))
                        goto error_end;
                    break;
                }

                case TypeInteger:
                {
                    int i;
                    if (!callIntMethod(mInfo, info, jobj, argvs, i))
                        goto error_end;

                    ret = luabind::object(L, i);

                    break;
                }

                case TypeFloat:
                {
                    float f;
                    if (!callFloatMethod(mInfo, info, jobj, argvs, f))
                        goto error_end;

                    ret = luabind::object(L, f);
                    break;
                }

                case TypeBoolean:
                {
                    bool b;
                    if (!callBooleanMethod(mInfo, info, jobj, argvs, b))
                        goto error_end;

                    ret = luabind::object(L, b);
                    break;
                }

                case TypeString:
                {
                    std::string s;
                    if (!callStringMethod(mInfo, info, jobj, argvs, s))
                        goto error_end;

                    ret = luabind::object(L, s);

                    break;
                }

                case TypeObject:
                {
                    JObject o;
                    if (!callObjectMethod(mInfo, info, jobj, argvs, o))
                        goto error_end;

                    ret = luabind::object(L, o);

                    break;
                }

                default:
                {
                    goto error_end;
                    break;
                }
            }

            if (mInfo.env->ExceptionCheck() == JNI_TRUE)
            {
                mInfo.env->ExceptionDescribe();
                mInfo.env->ExceptionClear();
                break;
            }
            else
            {
                callSuccess = true;
            }

            mInfo.env->DeleteLocalRef(mInfo.classID);

            auto t = luabind::newtable(L);
            t["success"] = callSuccess;
            t["result"] = ret;

            return t;

        } while (false);

    error_end:
        mInfo.env->DeleteLocalRef(mInfo.classID);


        auto t = luabind::newtable(L);
        t["success"] = false;
        return t;
    }

    luabind::object LuaJavaBridge::callJavaStaticMethod(const std::string& className,
                                                        const std::string& functionName,
                                                        const std::string& sig,
                                                        const luabind::object& argvs,
                                                        lua_State *L)
    {
        JniMethodInfo mInfo;
        bool callSuccess = false;

        if (!JniHelper::getStaticMethodInfo(mInfo, className.c_str(), functionName.c_str(), sig.c_str()))
        {
            OUTPUT_LOG("callJavaStaticMethod: Failed to find method id of %s.%s %s", className.c_str(), functionName.c_str(), sig.c_str());
            return object();
        }

        do
        {
            FunctionInfo info;
            auto err = validateMethodSig(sig, info);
            if (err != LUAJ_ERR_OK)
            {
                OUTPUT_LOG("callJavaStaticMethod: Failed to validate sig of %s.%s %s err = %d", className.c_str(), functionName.c_str(), sig.c_str(), err);
                break;
            }

            auto argCount = info.argvs.size();
            auto argvsType = luabind::type(argvs);

            if (argCount > 0 && argvsType != LUA_TTABLE)
            {
                OUTPUT_LOG("callJavaStaticMethod: argvs is Invalid ");
                break;
            }


            luabind::object ret;
            switch (info.returnType)
            {
                case TypeVoid:
                {
                    if (!callStaticVoidMethod(mInfo, info, argvs))
                        goto error_end;
                    break;
                }

                case TypeInteger:
                {
                    int i;
                    if (!callStaticIntMethod(mInfo, info, argvs, i))
                        goto error_end;

                    ret = luabind::object(L, i);

                    break;
                }

                case TypeFloat:
                {
                    float f;
                    if (!callStaticFloatMethod(mInfo, info, argvs, f))
                        goto error_end;

                    ret = luabind::object(L, f);
                    break;
                }

                case TypeBoolean:
                {
                    bool b;
                    if (!callStaticBooleanMethod(mInfo, info, argvs, b))
                        goto error_end;

                    ret = luabind::object(L, b);
                    break;
                }

                case TypeString:
                {
                    std::string s;
                    if (!callStaticStringMethod(mInfo, info, argvs, s))
                        goto error_end;

                    ret = luabind::object(L, s);

                    break;
                }

                case TypeObject:
                {
                    JObject o;
                    if (!callStaticObjectMethod(mInfo, info, argvs, o))
                        goto error_end;

                    ret = luabind::object(L, o);

                    break;
                }

                default:
                {
                    goto error_end;
                    break;
                }
            }

            if (mInfo.env->ExceptionCheck() == JNI_TRUE)
            {
                mInfo.env->ExceptionDescribe();
                mInfo.env->ExceptionClear();
                break;
            }
            else
            {
                callSuccess = true;
            }

            mInfo.env->DeleteLocalRef(mInfo.classID);

            auto t = luabind::newtable(L);
            t["success"] = callSuccess;
            t["result"] = ret;

            return t;

        } while (false);

    error_end:
        mInfo.env->DeleteLocalRef(mInfo.classID);

        auto t = luabind::newtable(L);
        t["success"] = false;
        return t;
    }

    LuaJavaBridge::ValueType LuaJavaBridge::checkType(const std::string &sig, size_t *pos)
    {
        switch (sig[*pos])
        {
        case 'I':
            return TypeInteger;
        case 'F':
            return TypeFloat;
        case 'Z':
            return TypeBoolean;
        case 'V':
            return TypeVoid;
        case 'C':
            return TypeChar;
        case 'S':
            return TypeShort;
        case 'J':
            return TypeLong;
        case 'D':
            return TypeDouble;
        case 'L':
            size_t pos2 = sig.find_first_of(';', *pos + 1);
            if (pos2 == string::npos)
            {
                return TypeInvalid;
            }

            const string t = sig.substr(*pos, pos2 - *pos + 1);
            if (t.compare("Ljava/lang/String;") == 0)
            {
                *pos = pos2;
                return TypeString;
            }
            else if (t.compare("Ljava/util/Vector;") == 0)
            {
                *pos = pos2;
                return TypeVector;
            }
            else
            {
                *pos = pos2;
                return TypeObject;
            }
        }

        return TypeInvalid;
    }

    int LuaJavaBridge::validateMethodSig(const std::string& sig, FunctionInfo& info)
    {
        auto len = sig.length();
        if (len < 3 || sig[0] != '(') // min sig is "()V"
        {
            return LUAJ_ERR_INVALID_SIGNATURES;
        }

        size_t pos = 1;

        while (pos < len && sig[pos] != ')')
        {
            auto type = checkType(sig, &pos);
            if (type == TypeInvalid) return LUAJ_ERR_TYPE_NOT_SUPPORT;

            info.argvs.push_back(type);
            pos++;
        }

        if (pos >= len || sig[pos] != ')')
        {
            return LUAJ_ERR_INVALID_SIGNATURES;
        }

        pos++;
        info.returnType = checkType(sig, &pos);

        if (info.returnType != TypeInvalid)
        {
            return LUAJ_ERR_OK;
        }
        else
        {
            return LUAJ_ERR_INVALID_SIGNATURES;
        }
    }

    boost::thread_specific_ptr<LuaJavaBridge::FuncMap> LuaJavaBridge::_funcMap;

    int LuaJavaBridge::retainLuaFunction(const luabind::object& funcObj)
    {
        if (!_funcMap.get())
        {
            _funcMap.reset(new FuncMap());
        }

        FuncMap& funcMap = *_funcMap;

        static  boost::thread_specific_ptr<int> _newFuncId;
        if (!_newFuncId.get())
        {
            _newFuncId.reset(new int);
            *_newFuncId = 0;
        }

        int& newFuncId = *_newFuncId;

        auto& funcInfo = funcMap[newFuncId];
        funcInfo.retainCount = 1;
        funcInfo.funcObj = funcObj;

        return newFuncId++;
    }

    int LuaJavaBridge::retainLuaFunctionById(int functionId)
    {
        if (!_funcMap.get())
        {
            _funcMap.reset(new FuncMap());
        }

        FuncMap& funcMap = *_funcMap;

        auto it = funcMap.find(functionId);
        if (it != funcMap.end())
        {
            return ++it->second.retainCount;
        }

        return 0;
    }

    int LuaJavaBridge::releaseLuaFunctionById(int functionId)
    {
        if (!_funcMap.get())
        {
            _funcMap.reset(new FuncMap());
        }

        FuncMap& funcMap = *_funcMap;

        auto it = funcMap.find(functionId);
        if (it != funcMap.end())
        {
            auto& retainCount = it->second.retainCount;
            --retainCount;
            if (retainCount <= 0)
            {
                funcMap.erase(it);
                return 0;
            }
            else
            {
                return retainCount;
            }
        }
        
        return 0;
    }

    int LuaJavaBridge::callLuaFunctionById(int functionId, const char *arg)
    {
        if (!_funcMap.get())
        {
            _funcMap.reset(new FuncMap());
        }

        FuncMap& funcMap = *_funcMap;

        auto it = funcMap.find(functionId);
        if (it != funcMap.end())
        {
            auto& callback = it->second.funcObj;
            auto L = callback.interpreter();
            callback.push(L);
            int top = lua_gettop(L);
            lua_pushstring(L, arg);
            int nResult = lua_pcall(L, 1, LUA_MULTRET, 0);

            if (nResult != 0)
            {
                OUTPUT_LOG("LUA throw error: err_msg[%s]\n", lua_tostring(L, -1));
                lua_pop(L, 1);
            }
            else
            {
                int num_results = lua_gettop(L) - top + 1;
                if (num_results > 0)
                    lua_pop(L, num_results);

                return 0;
            }
        }

        return -1;
    }

    void LuaJavaBridge::luaopen_luaj(lua_State *L)
    {
        using namespace luabind;

        module(L)
        [
            namespace_("LuaJavaBridge")
            [
                def("callJavaStaticMethod",&LuaJavaBridge::callJavaStaticMethod, raw(_5)),
                def("callJavaMethod", &LuaJavaBridge::callJavaMethod, raw(_5))
            ]
        ];
    }
}

extern "C"
{
    /** to load the lib, please call:
    NPL.call("LuaJavaBridge.cpp", {});
    NPL.activate("LuaJavaBridge.cpp");
    */
    PE_CORE_DECL NPL::NPLReturnCode NPL_activate_LuaJavaBridge_cpp(NPL::INPLRuntimeState* pState)
    {
        auto pRuntimeState = ParaEngine::CGlobals::GetNPLRuntime()->GetRuntimeState(pState->GetName());
        ParaEngine::LuaJavaBridge::luaopen_luaj(pRuntimeState->GetLuaState());
        OUTPUT_LOG("LuaJavaBridge loaded\n");
        return NPL::NPL_OK;
    };
}
