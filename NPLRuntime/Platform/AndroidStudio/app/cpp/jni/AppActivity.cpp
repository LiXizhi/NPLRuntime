//----------------------------------------------------------------------
// Class:	AppActivity
// Authors:	LanZhihong, LiXizhi
// Date: 2018.3.30
// Desc: main activity, customize this class for your app
//----------------------------------------------------------------------
#include "AppActivity.h"
#include "JniHelper.h"

#include "2dengine/GUIRoot.h"

#include <jni.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>
#include <android/looper.h>
#include <memory>
#include <fcntl.h> 
#include <poll.h>

namespace ParaEngine {

	const std::string AppActivity::classname = "com/tatfook/paracraft/AppActivity";

	AppActivity::AppActivity()
		: inited(false)
	{
	}
	
	AppActivity::~AppActivity()
	{
	}

	void AppActivity::init(struct android_app* app)
	{
		memset(&info, 0, sizeof(info));
		if (JniHelper::getMethodInfo(info, app->activity->clazz, "processGLEvent", "()V"))
		{
			info.env->DeleteLocalRef(info.classID);
			inited = true;
		}
	}

	void AppActivity::processGLEventJNI(struct android_app* app)
	{
		if (inited)
		{
			info.env->CallVoidMethod(app->activity->clazz, info.methodID);
		}
	}

	std::string AppActivity::getLauncherIntentData(struct android_app* state)
	{
		ParaEngine::JniMethodInfo info;
		if (ParaEngine::JniHelper::getMethodInfo(info, state->activity->clazz, "getLauncherIntentData", "()Ljava/lang/String;"))
		{

			jstring intent_data = (jstring)info.env->CallObjectMethod(state->activity->clazz, info.methodID);
			auto ret = ParaEngine::JniHelper::jstring2string(intent_data);
			info.env->DeleteLocalRef(info.classID);
			info.env->DeleteLocalRef(intent_data);
			return ret;
		}

		return "";
	}


	struct NativeCode : public ANativeActivity {
		NativeCode()
			: nativeWindow(nullptr)
		{
			memset((ANativeActivity*)this, 0, sizeof(ANativeActivity));
			memset(&callbacks, 0, sizeof(callbacks));
		}

		~NativeCode() {
			if (callbacks.onDestroy != nullptr) {
				callbacks.onDestroy(this);
			}

			if (env != nullptr) {
				if (clazz != nullptr) {
					env->DeleteGlobalRef(clazz);
				}

				if (javaAssetManager != nullptr) {
					env->DeleteGlobalRef(javaAssetManager);
				}
			}

			if (mainWorkRead >= 0) {
				auto looper = ALooper_forThread();
				ALooper_removeFd(looper, mainWorkRead);
			}

			setSurface(nullptr);
			if (mainWorkRead >= 0) close(mainWorkRead);
			if (mainWorkWrite >= 0) close(mainWorkWrite);

		}

		void setSurface(jobject _surface) {
			if (_surface != nullptr) {
				nativeWindow = ANativeWindow_fromSurface(env, _surface);
			}
			else {
				nativeWindow = nullptr;
			}
		}

		ANativeActivityCallbacks callbacks;
		ANativeWindow* nativeWindow;

		int32_t lastWindowWidth;
		int32_t lastWindowHeight;

		std::string externalDataPathObj;
		std::string internalDataPathObj;
		std::string obbPathObj;

		// These are used to wake up the main thread to process work.
		int mainWorkRead;
		int mainWorkWrite;

		// Need to hold on to a reference here in case the upper layers destroy our
		// AssetManager.
		jobject javaAssetManager;
	};

	struct ActivityWork {
		int32_t cmd;
		int32_t arg1;
		int32_t arg2;
	};

	static bool read_work(int fd, ActivityWork* outWork) {
		int res = read(fd, outWork, sizeof(ActivityWork));
		// no need to worry about EINTR, poll loop will just come back again.
		if (res == sizeof(ActivityWork)) return true;

		if (res < 0) LOGE("Failed reading work fd: %s", strerror(errno));
		else LOGE("Truncated reading work fd: %d", res);
		return false;
	}

	enum {
		CMD_FINISH = 1,
		CMD_SET_WINDOW_FORMAT,
		CMD_SET_WINDOW_FLAGS,
		CMD_SHOW_SOFT_INPUT,
		CMD_HIDE_SOFT_INPUT,
	};

	static int mainWorkCallback(int fd, int events, void* data)
	{
		NativeCode* code = (NativeCode*)data;

		if ((events & POLLIN) == 0) {
			return 1;
		}

		ActivityWork work;

		if (!read_work(code->mainWorkRead, &work)) {
			return 1;
		}

		/*
		switch (work.cmd) {
		case CMD_FINISH: {
			code->env->CallVoidMethod(code->clazz, gNativeActivityClassInfo.finish);
			code->messageQueue->raiseAndClearException(code->env, "finish");
		} break;
		case CMD_SET_WINDOW_FORMAT: {
			code->env->CallVoidMethod(code->clazz,
				gNativeActivityClassInfo.setWindowFormat, work.arg1);
			code->messageQueue->raiseAndClearException(code->env, "setWindowFormat");
		} break;
		case CMD_SET_WINDOW_FLAGS: {
			code->env->CallVoidMethod(code->clazz,
				gNativeActivityClassInfo.setWindowFlags, work.arg1, work.arg2);
			code->messageQueue->raiseAndClearException(code->env, "setWindowFlags");
		} break;
		case CMD_SHOW_SOFT_INPUT: {
			code->env->CallVoidMethod(code->clazz,
				gNativeActivityClassInfo.showIme, work.arg1);
			code->messageQueue->raiseAndClearException(code->env, "showIme");
		} break;
		case CMD_HIDE_SOFT_INPUT: {
			code->env->CallVoidMethod(code->clazz,
				gNativeActivityClassInfo.hideIme, work.arg1);
			code->messageQueue->raiseAndClearException(code->env, "hideIme");
		} break;
		default:
			LOGE("Unknown work command: %d", work.cmd);
			break;
		}
		*/

		return 1;
	}

} // end namespace

using namespace ParaEngine;

extern "C" {
	void ANativeActivity_onCreate(ANativeActivity* activity,
		void* savedState, size_t savedStateSize);

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onPauseNative(JNIEnv* env, jobject clazz, jlong handle)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onPause != nullptr) {
				code->callbacks.onPause(code);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onResumeNative(JNIEnv* env, jobject clazz, jlong handle)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onResume != nullptr) {
				code->callbacks.onResume(code);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onStartNative(JNIEnv* env, jobject clazz, jlong handle)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onStart != nullptr) {
				code->callbacks.onStart(code);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onStopNative(JNIEnv* env, jobject clazz, jlong handle)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onStop != nullptr) {
				code->callbacks.onStop(code);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onConfigurationChangedNative(JNIEnv* env, jobject clazz, jlong handle)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onConfigurationChanged != nullptr) {
				code->callbacks.onConfigurationChanged(code);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onLowMemoryNative(JNIEnv* env, jobject clazz, jlong handle)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onLowMemory != nullptr) {
				code->callbacks.onLowMemory(code);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onWindowFocusChangedNative(JNIEnv* env, jobject clazz, jlong handle, jboolean hasFocus)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onWindowFocusChanged != NULL) {
				code->callbacks.onWindowFocusChanged(code, hasFocus ? 1 : 0);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onSurfaceCreatedNative(JNIEnv* env, jobject clazz, jlong handle, jobject surface)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			code->setSurface(surface);
			if (code->nativeWindow != nullptr && code->callbacks.onNativeWindowCreated != nullptr) {
				code->callbacks.onNativeWindowCreated(code, code->nativeWindow);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onSurfaceChangedNative(JNIEnv* env, jobject clazz, jlong handle, jobject surface, jint format, jint width, jint height)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			auto oldNativeWindow = code->nativeWindow;

			code->setSurface(surface);
			if (oldNativeWindow != code->nativeWindow) {
				if (oldNativeWindow != nullptr && code->callbacks.onNativeWindowDestroyed != nullptr) {
					code->callbacks.onNativeWindowDestroyed(code,
						oldNativeWindow);
				}
				if (code->nativeWindow != nullptr) {
					if (code->callbacks.onNativeWindowCreated != nullptr) {
						code->callbacks.onNativeWindowCreated(code,
							code->nativeWindow);
					}
					code->lastWindowWidth = ANativeWindow_getWidth(code->nativeWindow);
					code->lastWindowHeight = ANativeWindow_getWidth(code->nativeWindow);
				}
			}
			else {
				// Maybe it resized?
				int32_t newWidth = ANativeWindow_getWidth(code->nativeWindow);
				int32_t newHeight = ANativeWindow_getWidth(code->nativeWindow);
				if (newWidth != code->lastWindowWidth
					|| newHeight != code->lastWindowHeight) {
					if (code->callbacks.onNativeWindowResized != nullptr) {
						code->callbacks.onNativeWindowResized(code,
							code->nativeWindow);
					}
				}
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onSurfaceRedrawNeededNative(JNIEnv* env, jobject clazz, jlong handle, jobject surface)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->nativeWindow != nullptr && code->callbacks.onNativeWindowRedrawNeeded != nullptr) {
				code->callbacks.onNativeWindowRedrawNeeded(code, code->nativeWindow);
			}
		}
	}


	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onSurfaceDestroyedNative(JNIEnv* env, jobject clazz, jlong handle)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->nativeWindow != nullptr && code->callbacks.onNativeWindowDestroyed != nullptr) {
				code->callbacks.onNativeWindowDestroyed(code,
					code->nativeWindow);
			}
			code->setSurface(nullptr);
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onInputQueueCreatedNative(JNIEnv* env, jobject clazz, jlong handle, jlong queuePtr)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onInputQueueCreated != NULL) {
				AInputQueue* queue = reinterpret_cast<AInputQueue*>(queuePtr);
				code->callbacks.onInputQueueCreated(code, queue);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onInputQueueDestroyedNative(JNIEnv* env, jobject clazz, jlong handle, jlong queuePtr)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onInputQueueDestroyed != nullptr) {
				AInputQueue* queue = reinterpret_cast<AInputQueue*>(queuePtr);
				code->callbacks.onInputQueueDestroyed(code, queue);
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onContentRectChangedNative(JNIEnv* env, jobject clazz, jlong handle, jint x, jint y, jint w, jint h)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onContentRectChanged != nullptr) {
				ARect rect;
				rect.left = x;
				rect.top = y;
				rect.right = x + w;
				rect.bottom = y + h;
				code->callbacks.onContentRectChanged(code, &rect);
			}
		}
	}


	JNIEXPORT jbyteArray JNICALL Java_com_tatfook_paracraft_AppActivity_onSaveInstanceStateNative(JNIEnv* env, jobject clazz, jlong handle)
	{
		jbyteArray array = nullptr;

		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			if (code->callbacks.onSaveInstanceState != nullptr) {
				size_t len = 0;
				jbyte* state = (jbyte*)code->callbacks.onSaveInstanceState(code, &len);
				if (len > 0) {
					array = env->NewByteArray(len);
					if (array != nullptr) {
						env->SetByteArrayRegion(array, 0, len, state);
					}
				}
				if (state != nullptr) {
					free(state);
				}
			}
		}
		return array;
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_unloadNativeCode(JNIEnv* env, jobject clazz, jlong handle)
	{
		if (handle != 0) {
			NativeCode* code = (NativeCode*)handle;
			delete code;
		}
	}

	std::unique_ptr<NativeCode> code;


	JNIEXPORT jlong JNICALL Java_com_tatfook_paracraft_AppActivity_nativeInit(JNIEnv* env, jobject clazz, jobject jlooper, jstring internalDataDir, jstring obbDir, jstring externalDataPath, jint sdkVersion, jobject jAssetMgr, jbyteArray savedState)
	{
		code.reset(new NativeCode());

		int msgpipe[2];
		if (pipe(msgpipe))
		{
			LOGE("could not create pipe: %s", strerror(errno));
			return 0;
		}

		code->mainWorkRead = msgpipe[0];
		code->mainWorkWrite = msgpipe[1];

		int result = fcntl(code->mainWorkRead, F_SETFL, O_NONBLOCK);

		if (result != 0)
		{
			LOGE("Could not make main work read pipe non-blocking: %s", strerror(errno));
			return 0;
		}

		result = fcntl(code->mainWorkWrite, F_SETFL, O_NONBLOCK);

		if (result != 0)
		{
			LOGE("ould not make main work write pipe non-blocking: %s", strerror(errno));
			return 0;
		}

		auto looper = ALooper_forThread();
		ALooper_addFd(looper, code->mainWorkRead, 0, ALOOPER_EVENT_INPUT, mainWorkCallback, code.get());

		code->ANativeActivity::callbacks = &code->callbacks;

		if (env->GetJavaVM(&code->vm) < 0) {
			LOGE("NativeActivity GetJavaVM failed");
			return 0;
		}

		code->env = env;
		code->clazz = env->NewGlobalRef(clazz);

		code->internalDataPathObj = JniHelper::jstring2string(internalDataDir);
		code->internalDataPath = code->internalDataPathObj.c_str();

		if (externalDataPath != nullptr) {
			code->externalDataPathObj = JniHelper::jstring2string(externalDataPath);
		}
		code->externalDataPath = code->externalDataPathObj.c_str();

		code->sdkVersion = sdkVersion;

		code->javaAssetManager = env->NewGlobalRef(jAssetMgr);
		code->assetManager = AAssetManager_fromJava(env, code->javaAssetManager);

		if (obbDir != nullptr) {
			code->obbPathObj = JniHelper::jstring2string(obbDir);
		}
		code->obbPath = code->obbPathObj.c_str();

		jbyte* rawSavedState = nullptr;
		jsize rawSavedSize = 0;
		if (savedState != nullptr) {
			rawSavedState = env->GetByteArrayElements(savedState, nullptr);
			rawSavedSize = env->GetArrayLength(savedState);
		}

		JniHelper::setClassLoaderFrom(code->clazz);

		ANativeActivity_onCreate(code.get(), rawSavedState, rawSavedSize);

		if (rawSavedState != nullptr) {
			env->ReleaseByteArrayElements(savedState, rawSavedState, 0);
		}
		
		return (jlong)code.release();
	}

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_AppActivity_onKeyBack(JNIEnv* env, jobject clazz, jboolean bDown)
    {
        return;
    }


}