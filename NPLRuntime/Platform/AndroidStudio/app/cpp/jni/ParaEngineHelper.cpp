//-----------------------------------------------------------------------------
// ParaEngineHelper.h
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "NPLScriptingState.h"
#include "ParaEngineHelper.h"
#include "JniHelper.h"
#include "ParaEngineSettings.h"
#include "ParaScriptingWorld.h"
#include "AppDelegate.h"

#include "Framework/Common/Bridge/LuaJavaBridge.h"
#include <boost/format.hpp>
#include <android/asset_manager_jni.h>

namespace ParaEngine {
	const std::string ParaEngineHelper::classname = "com/tatfook/paracraft/ParaEngineHelper";

	void ParaEngineHelper::init()
	{
		std::string languageName = JniHelper::callStaticStringMethod(classname, "getCurrentLanguage");

		LanguageType ret = LanguageType::ENGLISH;
		if (languageName == "zh")
		{
			ret = LanguageType::CHINESE;
		}
		else if (languageName == "en")
		{
			ret = LanguageType::ENGLISH;
		}
		else if (languageName == "fr")
		{
			ret = LanguageType::FRENCH;
		}
		else if (languageName == "it")
		{
			ret = LanguageType::ITALIAN;
		}
		else if (languageName == "de")
		{
			ret = LanguageType::GERMAN;
		}
		else if (languageName == "es")
		{
			ret = LanguageType::SPANISH;
		}
		else if (languageName == "ru")
		{
			ret = LanguageType::RUSSIAN;
		}
		else if (languageName == "nl")
		{
			ret = LanguageType::DUTCH;
		}
		else if (languageName == "ko")
		{
			ret = LanguageType::KOREAN;
		}
		else if (languageName == "ja")
		{
			ret = LanguageType::JAPANESE;
		}
		else if (languageName == "hu")
		{
			ret = LanguageType::HUNGARIAN;
		}
		else if (languageName == "pt")
		{
			ret = LanguageType::PORTUGUESE;
		}
		else if (languageName == "ar")
		{
			ret = LanguageType::ARABIC;
		}
		else if (languageName == "nb")
		{
			ret = LanguageType::NORWEGIAN;
		}
		else if (languageName == "pl")
		{
			ret = LanguageType::POLISH;
		}
		else if (languageName == "tr")
		{
			ret = LanguageType::TURKISH;
		}
		else if (languageName == "uk")
		{
			ret = LanguageType::UKRAINIAN;
		}
		else if (languageName == "ro")
		{
			ret = LanguageType::ROMANIAN;
		}
		else if (languageName == "bg")
		{
			ret = LanguageType::BULGARIAN;
		}

		ParaEngineSettings::GetSingleton().SetCurrentLanguage(ret);
	}

	std::string ParaEngineHelper::getWritablePath()
	{
		return JniHelper::callStaticStringMethod(classname, "getWritablePath");
	}

	std::string ParaEngineHelper::getExternalStoragePath()
	{
		return JniHelper::callStaticStringMethod(classname, "getExternalStoragePath");
	}

	std::string ParaEngineHelper::getObbPath()
	{
		return JniHelper::callStaticStringMethod(classname, "getObbPath");
	}

	int ParaEngineHelper::getWifiIP()
	{
        return JniHelper::callStaticIntMethod(classname, "getWifiIP");
	}

    std::string ParaEngineHelper::getWifiMAC()
    {
        return JniHelper::callStaticStringMethod(classname, "getWifiMAC");
    }

    std::string ParaEngineHelper::getMachineID()
    {
        return JniHelper::callStaticStringMethod(classname, "getMachineID");
    }

    static std::string s_activate_file;
	static int s_callback_id;
    void ParaEngineHelper::OpenFileDialog(const char* filter, const char* activate_file, int callback_id) {
        if (activate_file) s_activate_file = activate_file;
        if (callback_id) s_callback_id = callback_id;
		JniHelper::callStaticVoidMethod(classname, "OpenFileDialog", filter);
	}

	void ParaEngineHelper::OpenFileDialogCallback(std::string filepath) {
		std::stringstream ss;
		ss << "msg={_callbackIdx=" << s_callback_id << ",filepath=[["<< filepath<<"]]}"<< std::endl;
		NPL::NPLRuntimeState_ptr rsptr = NPL::CNPLRuntime::GetInstance()->GetRuntimeState(s_activate_file);
		NPL::CNPLRuntime::GetInstance()->NPL_Activate(rsptr, s_activate_file.c_str(), ss.str().c_str());
	}
}

extern "C" {
	using namespace ParaEngine;

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineHelper_nativeSetContext(JNIEnv* env, jclass clazz, jobject context, jobject assetManager)
	{
		JniHelper::setClassLoaderFrom(context);
		JniHelper::setAssetmanager(AAssetManager_fromJava(env, assetManager));
	}

	JNIEXPORT jstring JNICALL Java_com_tatfook_paracraft_ParaEngineHelper_GetWorldDirectory(JNIEnv *env, jclass clazz) {
		// TODO: implement GetWorldDirectory()
		std::string world_directory = ParaScripting::ParaWorld::GetWorldDirectory();
		return env->NewStringUTF(world_directory.c_str());
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineHelper_OpenFileDialogNativeCallback(JNIEnv *env, jclass clazz, jstring filepath) {
		// TODO: implement OpenFileDialogCallback()
		ParaEngineHelper::OpenFileDialogCallback(JniHelper::jstring2string(filepath));
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineHelper_setKeyState(JNIEnv *env, jclass clazz, jint key_code,jint key_state) {
		// TODO: implement setKeyState()
		boost::format fmt("NPL.activate('Keyboard.lua', {msg={type='keyEvent',keyCode=%d,keyState=%d}})");
		fmt % key_code % key_state;
		std::string code = fmt.str();
		ParaEngine::LuaJavaBridge::nplActivate(code, "");
	}

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineHelper_onCmdProtocol(JNIEnv *env, jclass clazz, jstring str_protocol) {
        // TODO: implement onCmdProtocol()
        std::string cmd = JniHelper::jstring2string(str_protocol);
        env->DeleteLocalRef(str_protocol);
        AppDelegate::getInstance().onCmdLine(cmd);
    }
}
