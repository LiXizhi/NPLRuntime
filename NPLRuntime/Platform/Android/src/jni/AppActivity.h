#pragma once

#include "JniHelper.h"

#include <android_native_app_glue.h>
#include <string>

namespace ParaEngine {

	struct AppActivity
	{
		void processGLEventJNI(struct android_app* app);
		AppActivity(struct android_app* app);

	private:
		static const std::string classname;
		
	};

	
} // end namespace