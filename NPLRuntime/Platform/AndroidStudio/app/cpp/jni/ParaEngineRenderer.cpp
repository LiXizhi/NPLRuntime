//
// Created by lan on 2019/12/30.
//

#include "ParaEngineRenderer.h"
#include "ParaEngineActivity.h"
#include "JniHelper.h"
#include "AppDelegate.h"

#include <jni.h>

namespace ParaEngine {
    const std::string ParaEngineRenderer::classname = "com/tatfook/paracraft/ParaEngineRenderer";
} // namespace ParaEngine

extern "C" {
    using namespace ParaEngine;

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeInit(JNIEnv* env, jclass clazz, jint w, jint h)
    {
        AppDelegate::getInstance().init(w, h, ParaEngineActivity::getLauncherIntentData());
    }

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeTouchesBegin(JNIEnv* env, jclass clazz, jint id, jfloat x, jfloat y)
    {
        AppDelegate::getInstance().handle_touches_begin(id, x, y);
    }

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeTouchesEnd(JNIEnv* env, jclass clazz, jint id, jfloat x, jfloat y)
    {
        AppDelegate::getInstance().handle_touches_end(id, x, y);
    }

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeTouchesMove(JNIEnv* env, jclass clazz, jintArray ids, jfloatArray xs, jfloatArray ys)
    {
        int size = env->GetArrayLength(ids);
        jint id[size];
        jfloat x[size];
        jfloat y[size];

        env->GetIntArrayRegion(ids, 0, size, id);
        env->GetFloatArrayRegion(xs, 0, size, x);
        env->GetFloatArrayRegion(ys, 0, size, y);

        AppDelegate::getInstance().handle_touches_move(id, x, y, size);
    }

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeTouchesCancel(JNIEnv* env, jclass clazz, jintArray ids, jfloatArray xs, jfloatArray ys)
    {
        int size = env->GetArrayLength(ids);
        jint id[size];
        jfloat x[size];
        jfloat y[size];

        env->GetIntArrayRegion(ids, 0, size, id);
        env->GetFloatArrayRegion(xs, 0, size, x);
        env->GetFloatArrayRegion(ys, 0, size, y);

        AppDelegate::getInstance().handle_touches_cancel(id, x, y, size);
    }

    JNIEXPORT jboolean JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeKeyEvent(JNIEnv* env, jclass clazz, jint keyCode, jboolean isPressed)
    {
        return AppDelegate::getInstance().handle_key_input(keyCode, isPressed);
    }

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeOnSurfaceChanged(JNIEnv* env, jclass clazz, jint w, jint h)
    {
        AppDelegate::getInstance().init(w, h, ParaEngineActivity::getLauncherIntentData());
    }


    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeRender(JNIEnv* env, jclass clazz)
    {
        AppDelegate::getInstance().handle_mainloop();
    }

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeOnPause(JNIEnv* env, jclass clazz)
    {
        AppDelegate::getInstance().OnPause();
    }

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineRenderer_nativeOnResume(JNIEnv* env, jclass clazz)
    {
        AppDelegate::getInstance().OnResume();
    }
} // extern "C"