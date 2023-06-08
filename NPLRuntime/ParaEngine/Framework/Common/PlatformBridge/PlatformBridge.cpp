#include "PlatformBridge.h"
#ifdef EMSCRIPTEN_SINGLE_THREAD
#define auto_ptr unique_ptr
#endif
#include "ParaEngine.h"
extern "C"
{
#include "lua.h"
}
#include "ParaScriptBindings/ParaScriptingNPL.h"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include "Core/NPLInterface.hpp"
#include "NPL/NPLHelper.h"

namespace ParaEngine {
	std::string PlatformBridge::LuaCallNative(const object& params)
	{
		std::string key="";
		std::string jsonParam="";
		LuaCB * cb = 0;
		if (type(params) == LUA_TTABLE) {
			int callbackIdx;
			const char* luafile;
			if (params["luafile"] && params["callbackIdx"]) {
				luafile = object_cast<const char*>(params["luafile"]);
				callbackIdx = object_cast<int>(params["callbackIdx"]);
				
				std::string xx = luafile;
				cb = new LuaCB(luafile, callbackIdx);
			}
			if (params["key"]) {
				key = object_cast<const char*>(params["key"]);
			}
			if (params["jsonParam"]) {
				jsonParam = object_cast<const char*>(params["jsonParam"]);
			}
		}
		else if (type(params) == LUA_TSTRING) {
			key = object_cast<const char*>(params);
		}
		
		if (cb) {
			PlatformBridge::call_native_withCB(key, cb, jsonParam);
		}
		else {
			return PlatformBridge::call_native(key, jsonParam);
		}
		return "";
	}

	void PlatformBridge::onNativeCallback(long luaCbPtr, const char * result)
	{
		if (luaCbPtr == 0) {
			return;
		}
		LuaCB *cb = ((LuaCB *)luaCbPtr);

		int callbackIdx = cb->idx;
		std::string luafile = cb->luafile;
		{
			std::stringstream ss;
			ss << "msg={_callbackIdx=" << callbackIdx << ",ret=[[" << result << "]]}" << std::endl;
			auto str = ss.str();
			ParaScripting::CNPL::activate2_(luafile.c_str(), ss.str().c_str());
		};

		delete cb;
	}

	/*ƽ̨����ʵ��
	std::string PlatformBridge::call_native(std::string key, const std::string jsonParam) {return "";}

	void PlatformBridge::call_native_withCB(std::string key, LuaCB * cb, const std::string jsonParam){}
	*/
}
