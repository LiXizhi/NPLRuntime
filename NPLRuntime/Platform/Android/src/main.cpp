
#include "android_native_app_glue.h"
#include "AppDelegate.h"
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ParaEngine", __VA_ARGS__))

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{

	return JNI_VERSION_1_4;
}

inline std::string jni_sting_to_std_string(JNIEnv* env, jstring jstr)
{
	if (!jstr)return "";
	const char *tmp_str = env->GetStringUTFChars(jstr, 0);
	std::string ret = tmp_str;
	env->ReleaseStringUTFChars(jstr, tmp_str);
	return ret;
}


std::string get_launcher_intent_data(struct android_app* state)
{
	JNIEnv *env;
	state->activity->vm->AttachCurrentThread(&env, 0);
	jobject me = state->activity->clazz;

	jclass acl = env->GetObjectClass(me); //class pointer of NativeActivity
	jmethodID giid = env->GetMethodID(acl, "getIntent", "()Landroid/content/Intent;");
	jobject intent = env->CallObjectMethod(me, giid); //Got our intent

	if (intent != nullptr)
	{

		jclass icl = env->GetObjectClass(intent); //class pointer of Intent

		jmethodID method_get_action = env->GetMethodID(icl, "getAction", "()Ljava/lang/String;");
		jstring action = (jstring)env->CallObjectMethod(intent, method_get_action);

		std::string action_str = jni_sting_to_std_string(env, action);
		if (action_str == "android.intent.action.VIEW")
		{
			jmethodID method_get_data_string = env->GetMethodID(icl, "getDataString", "()Ljava/lang/String;");
			jstring intent_data = (jstring)env->CallObjectMethod(intent, method_get_data_string);
			std::string data_str = jni_sting_to_std_string(env, intent_data);
			return data_str;
		}
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
		app = new AppDelegate;
		LOGI("android main new state");
	}	
	app->Run(state);
}

