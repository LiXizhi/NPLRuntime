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
	* ͨ��NPL.active�ص���lua
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
		* luaͨ��c++����native
		*/
		static std::string LuaCallNative(const object& params);
	public:
		/**
		* native�첽�ص�lua (JNI)
		*/
		static void onNativeCallback(long luaCbPtr,const char * result);
	private:
		/**
		* c++����platform ��ƽ̨����ʵ��
		*/
		static std::string call_native(std::string key, const std::string jsonParam = "");
		static void call_native_withCB(std::string key, LuaCB * cb,const std::string jsonParam ="");
	};
}