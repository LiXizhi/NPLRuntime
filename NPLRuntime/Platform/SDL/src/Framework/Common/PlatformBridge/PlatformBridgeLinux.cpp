#include "Framework/Common/PlatformBridge/PlatformBridge.h"

namespace ParaEngine {
	std::string PlatformBridge::call_native(std::string key, const std::string param) {
		std::string ret = "";
		if (key == "GetSysInfo") {
			
		}else if (key == "GetAppInfo") {

		}
		
		return ret;
	}

	void PlatformBridge::call_native_withCB(std::string key, LuaCB * cb, const std::string param)
	{
		
	}
}