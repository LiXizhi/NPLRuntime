#include "Framework/Common/PlatformBridge/PlatformBridge.h"
#import "DeviceInfo/DeviceInfo.h"

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
