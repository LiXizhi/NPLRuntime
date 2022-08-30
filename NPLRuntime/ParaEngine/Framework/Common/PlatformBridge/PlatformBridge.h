#pragma once

#include <string>
#include <functional>

namespace luabind
{
	namespace adl {
		class object;
	}
	using adl::object;
}

namespace ParaEngine
{
	using namespace luabind;
	using namespace std;

	/**
	* 通过NPL.active回调到lua
	*/
	struct LuaCB
	{
		int idx;
		const char* luafile;
		LuaCB(const char* luafile, int idx) {
			this->luafile = luafile;
			this->idx = idx;
		}
	};

	class PlatformBridge
	{
	public:
		/**
		* lua通过c++调用native
		*/
		static std::string LuaCallNative(const object& params);
	public:
		/**
		* native异步回调lua (JNI)
		*/
		static void onNativeCallback(long luaCbPtr,const char * result);
	private:
		/**
		* c++调用platform ，平台各自实现
		*/
		static std::string call_native(std::string key, const std::string jsonParam = "");
		static void call_native_withCB(std::string key, LuaCB * cb,const std::string jsonParam ="");
	};
}