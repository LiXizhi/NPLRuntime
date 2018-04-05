#pragma once

#include "JniHelper.h"

#include <android_native_app_glue.h>
#include <string>

namespace ParaEngine {

	struct AppActivity
	{
		void processGLEventJNI(struct android_app* app);
		void init(struct android_app* app);
		
		static std::string getLauncherIntentData(struct android_app* state);

		AppActivity();
		~AppActivity();
	private:
		static const std::string classname;
		JniMethodInfo info;
		bool inited;
	};

	
} // end namespace