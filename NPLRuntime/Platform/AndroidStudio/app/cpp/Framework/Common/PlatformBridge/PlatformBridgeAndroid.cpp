//-----------------------------------------------------------------------------
// Authors: hyz
// CreateDate: 2022.6.27
//-----------------------------------------------------------------------------


#include "ParaEngine.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "Framework/Common/PlatformBridge/PlatformBridge.h"
#include "jni/JniHelper.h"

namespace ParaEngine {
	std::string PlatformBridge::call_native(std::string key, const std::string param) {
		std::string ret = "";
		ret = JniHelper::callStaticStringMethod("com/tatfook/paracraft/luabridge/PlatformBridge", "call_native",key, param);
		return ret;
	}

	void PlatformBridge::call_native_withCB(std::string key, LuaCB * cb, const std::string param)
	{
		long luaCbPtr = (long)(cb);
		JniHelper::callStaticVoidMethod("com/tatfook/paracraft/luabridge/PlatformBridge", "call_native_withCB",key,luaCbPtr, param);
	}
}

extern "C"
{
	JNIEXPORT void JNICALL
	Java_com_tatfook_paracraft_luabridge_PlatformBridge_onNativeCallback(JNIEnv *env, jclass clazz,
																		 jlong lua_cb_ptr,
																		 jstring result) {
		// TODO: implement onNativeCallback()
		ParaEngine::PlatformBridge::onNativeCallback(lua_cb_ptr,ParaEngine::JniHelper::jstring2string(result).c_str());
	}
}
