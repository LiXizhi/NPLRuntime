#include "JniHelper.h"

#include <android/log.h>
#include <android/api-level.h>
#include <jni.h>

#ifdef OPENAL_STATIC
extern "C" JNIEXPORT jint JNICALL OpenAL_Init(JavaVM *jvm, void* reserved);
#endif

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	LOGI("JNI_OnLoad begin");

	ParaEngine::JniHelper::setJavaVM(vm);

	LOGI("JNI_OnLoad end"); 

#ifdef OPENAL_STATIC
	OpenAL_Init(vm, reserved);
#endif

	return JNI_VERSION_1_4;
}

