#include "ParaEngine.h"
#include "ParaEngineNativeView.h"
#include "JniHelper.h"

#include "2dengine/GUIIMEDelegate.h"
#include "2dengine/GUIRoot.h"

namespace ParaEngine {
    static const std::string classname =  "com/tatfook/paracraft/ParaEngineNativeView";

    void ParaEngineNativeView::setIMEKeyboardState(bool bOpen)
    {
        if (bOpen)
            JniHelper::callStaticVoidMethod(classname, "openIMEKeyboard");
        else
            JniHelper::callStaticVoidMethod(classname, "closeIMEKeyboard");
    }
} // namespace ParaEngine

using namespace ParaEngine;
extern "C" {


JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineNativeView_nativeDeleteBackward(JNIEnv* env, jclass clazz)
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

JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineNativeView_nativeOnUnicodeChar(JNIEnv* env, jclass clazz, jstring text)
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

JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineNativeView_onKeyBack(JNIEnv* env, jobject clazz, jboolean bDown)
{
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }
    auto pGUI = CGUIRoot::GetInstance();
    if (pGUI)
    {
        if (bDown)
            pGUI->SendKeyDownEvent(EVirtualKey::KEY_ESCAPE);
        else
            pGUI->SendKeyUpEvent(EVirtualKey::KEY_ESCAPE);
    }

    return;
}

JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineNativeView_onKeyMenu(JNIEnv* env, jobject clazz, jboolean bDown)
{
    return;
}

} // extern "C"