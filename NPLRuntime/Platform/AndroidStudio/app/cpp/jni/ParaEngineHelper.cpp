#include "ParaEngineHelper.h"
#include "JniHelper.h"
#include "ParaEngineSettings.h"

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
}

extern "C" {
	using namespace ParaEngine;

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineHelper_nativeSetContext(JNIEnv* env, jclass clazz, jobject context, jobject assetManager)
	{
		JniHelper::setClassLoaderFrom(context);
		JniHelper::setAssetmanager(AAssetManager_fromJava(env, assetManager));
	}
}