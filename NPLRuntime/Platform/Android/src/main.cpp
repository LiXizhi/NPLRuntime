//----------------------------------------------------------------------
// Class:	main native 
// Authors:	LiXizhi@yeah.net
// Company: tatfook
// Date: 2018.3
//-----------------------------------------------------------------------
#include "android_native_app_glue.h"
#include "AppDelegate.h"
#include "jni/JniHelper.h"
#include <android/log.h>


std::string get_launcher_intent_data(struct android_app* state)
{
	ParaEngine::JniMethodInfo info;
	if (ParaEngine::JniHelper::getMethodInfo(info, state->activity->clazz, "getLauncherIntentData", "()Ljava/lang/String;"))
	{

		jstring intent_data = (jstring) info.env->CallObjectMethod(state->activity->clazz, info.methodID);
		auto ret = ParaEngine::JniHelper::jstring2string(intent_data);
		info.env->DeleteLocalRef(info.classID);
		info.env->DeleteLocalRef(intent_data);
		return ret;
	}

	return "";
}


/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
	LOGI("android_main");
    // Make sure glue isn't stripped.
	app_dummy();

	std::string intent_data = get_launcher_intent_data(state);
	LOGI("intent_data:%s", intent_data.c_str());

	using namespace ParaEngine;
	AppDelegate* app = nullptr;
	if (state->savedState)
	{
		saved_state* savedState = (saved_state*)state->savedState;
		app = savedState->app;
		LOGI("android main state saved");
	}
	else {
		app = new AppDelegate(intent_data);
		LOGI("android main new state");
	}	
	app->Run(state);
}

