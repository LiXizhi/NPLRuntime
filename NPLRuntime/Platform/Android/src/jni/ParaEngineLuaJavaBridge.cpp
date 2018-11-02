#include "ParaEngine.h"
#include "JniHelper.h"
#include "NPLRuntime.h"
#include "NPLScriptingState.h"
#include "Framework/Common/Bridge/LuaJavaBridge.h"

using namespace ParaEngine;

extern "C" {
	JNIEXPORT jint JNICALL Java_com_tatfook_paracraft_ParaEngineLuaJavaBridge_callLuaFunctionWithString(JNIEnv *env, jclass, jint luaFunctionId, jstring value)
	{
		std::string strValue = JniHelper::getStringUTFCharsJNI(env, value);
		env->DeleteLocalRef(value);
		int ret = LuaJavaBridge::callLuaFunctionById(luaFunctionId, strValue.c_str());
		return ret;
	}


	JNIEXPORT jint JNICALL Java_com_tatfook_paracraft_ParaEngineLuaJavaBridge_retainLuaFunction(JNIEnv *env, jclass, jint luaFunctionId)
	{
		return LuaJavaBridge::retainLuaFunctionById(luaFunctionId);
	}

	JNIEXPORT jint JNICALL Java_com_tatfook_paracraft_ParaEngineLuaJavaBridge_releaseLuaFunction(JNIEnv *env, jclass, jint luaFunctionId)
	{
		return LuaJavaBridge::releaseLuaFunctionById(luaFunctionId);
	}

	JNIEXPORT jint JNICALL Java_com_tatfook_paracraft_ParaEngineLuaJavaBridge_nplActivate(JNIEnv *env, jclass, jstring pFilePath, jstring pMsg)
	{
		std::string strNPLFileName = JniHelper::getStringUTFCharsJNI(env, pFilePath);
		env->DeleteLocalRef(pFilePath);

		std::string msg = JniHelper::getStringUTFCharsJNI(env, pMsg);
		env->DeleteLocalRef(pMsg);

		NPL::NPLRuntimeState_ptr rsptr = NPL::CNPLRuntime::GetInstance()->GetRuntimeState(strNPLFileName);
		NPL::CNPLRuntime::GetInstance()->NPL_Activate(rsptr, strNPLFileName.c_str(), msg.c_str());

		return 0;
	}
} // end extern