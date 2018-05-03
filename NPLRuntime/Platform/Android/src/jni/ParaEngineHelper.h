#pragma once

#include <jni.h>
#include <string>


namespace ParaEngine {

	class ParaEngineHelper
	{
	public:
		static void init();
	private:
		static const std::string classname;
	};
}