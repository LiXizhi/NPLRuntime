#include "Framework/Common/PlatformBridge/PlatformBridge.h"
#include "DeviceInfo/DeviceInfo.h"

NSString* getCountryCode()
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *currentLanguage = [languages objectAtIndex:0];

    // get the current language code.(such as English is "en", Chinese is "zh" and so on)
    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:currentLanguage];
    NSString * languageCode = [temp objectForKey:NSLocaleLanguageCode];
    NSString * countryCode = [temp objectForKey:NSLocaleCountryCode];
    return countryCode;
}

namespace ParaEngine {
	std::string PlatformBridge::call_native(std::string key, const std::string param) {
		std::string ret = "";
		if (key == "getDeviceInfo") {
            ret = [[DeviceInfo getDeviceInfoJsonStr] UTF8String];
		}else if (key == "getAppInfo") {
            ret = [[DeviceInfo getAppInfoJsonStr] UTF8String];
		}
		
		return ret;
	}

	void PlatformBridge::call_native_withCB(std::string key, LuaCB * cb, const std::string param)
	{
		
	}
}
