#include "ParaEngineHelper.h"
#include "JniHelper.h"
#include "ParaEngineSettings.h"


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
}