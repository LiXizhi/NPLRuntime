#pragma once

#include <jni.h>
#include <string>


namespace ParaEngine {

	class ParaEngineHelper
	{
	public:
		static void init();

		static std::string getWritablePath();
		static std::string getExternalStoragePath();
		static std::string getObbPath();
		static int getWifiIP();
		static std::string getWifiMAC();
	private:
		static const std::string classname;
	};
}