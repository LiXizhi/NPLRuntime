#include "JniHelper.h"

#include <android/log.h>
#include <android/api-level.h>
#include <jni.h>

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	LOGI("JNI_OnLoad begin");

	ParaEngine::JniHelper::setJavaVM(vm);

	LOGI("JNI_OnLoad end");

	return JNI_VERSION_1_4;
}
