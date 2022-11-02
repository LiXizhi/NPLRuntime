//-----------------------------------------------------------------------------
// ParaEngineGLSurfaceView.cpp
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2022.11.2
//-----------------------------------------------------------------------------

#include "ParaEngineGLSurfaceView.h"
#include "JniHelper.h"

#include "2dengine/GUIIMEDelegate.h"
#include "2dengine/GUIRoot.h"

#include <jni.h>

namespace ParaEngine {
    const std::string ParaEngineGLSurfaceView::classname = "com/tatfook/paracraft/ParaEngineGLSurfaceView";

    void ParaEngineGLSurfaceView::setIMEKeyboardState(bool bOpen, bool bMoveView, int ctrlBottom)
    {
        JniHelper::callStaticVoidMethod(classname, "setIMEKeyboardState", bOpen, bMoveView, ctrlBottom);
    }
}

extern "C" {
    using namespace ParaEngine;

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineGLSurfaceView_nativeSetSurface(JNIEnv* env, jclass clazz, jobject surface)
    {
        JniHelper::setSurface(env, surface);
    }

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineGLSurfaceView_nativeDeleteBackward(JNIEnv* env, jclass clazz)
    {
        if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        {
            return;
        }
        auto pGUI = CGUIRoot::GetInstance();
        if (pGUI)
        {
            pGUI->SendKeyDownEvent(EVirtualKey::KEY_BACK);
            pGUI->SendKeyUpEvent(EVirtualKey::KEY_BACK);
        }

    }

    JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineGLSurfaceView_nativeOnUnicodeChar(JNIEnv *env, jclass clazz, jstring text)
    {
        if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        {
            return;
        }

        auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();

        if (pGUI)
        {
            auto strText = JniHelper::getStringUTF16CharsJNI(env, text);

            if (!strText.empty())
            {
                std::wstring s;

                for (size_t i = 0; i < strText.size(); i++)
                {
                    s += (WCHAR)strText[i];
                }

                pGUI->OnHandleWinMsgChars(s);
            }
        }
    }

    JNIEXPORT void JNICALL
        Java_com_tatfook_paracraft_ParaTextInputWrapper_nativeSetText
        (JNIEnv *env, jclass clazz, jstring text)
    {
        if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        {
            return;
        }

        auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();

        if (pGUI)
        {
            std::string strText = JniHelper::getStringUTFCharsJNI(env, text);
            pGUI->SetTextA(strText.c_str());
        }
    }

    JNIEXPORT jstring JNICALL
        Java_com_tatfook_paracraft_ParaTextInputWrapper_nativeGetText
        (JNIEnv *env, jclass clazz)
    {
        if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        {
            return env->NewStringUTF("");
        }

        ParaEngine::CGUIBase *pGUI = ParaEngine::CGUIRoot::GetInstance()->GetUIKeyFocus();

        if (pGUI)
        {
            std::string curText;
            pGUI->GetTextA(curText);

            return env->NewStringUTF(curText.c_str());
        }

        return env->NewStringUTF("");
    }

    JNIEXPORT void JNICALL
        Java_com_tatfook_paracraft_ParaTextInputWrapper_nativeSetCaretPosition
        (JNIEnv *env, jclass clazz, jint caretPosition)
    {
        if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        {
            return;
        }

        auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();

        if (pGUI)
        {
            pGUI->SetCaretPosition(caretPosition);
        }
    }

    JNIEXPORT int JNICALL
        Java_com_tatfook_paracraft_ParaTextInputWrapper_nativeGetCaretPosition
        (JNIEnv *env, jclass clazz, jint caretPosition)
    {
        if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
        {
            return 0;
        }

        auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();

        if (pGUI)
        {
            return pGUI->GetCaretPosition();
        }

        return 0;
    }
}
