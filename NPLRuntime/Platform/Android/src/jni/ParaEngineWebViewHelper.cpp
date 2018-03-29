#include "ParaEngine.h"
#include "ParaEngineWebViewHelper.h"
#include "AppDelegate.h"
#include "AttributeClass.h"
#include "JniHelper.h"
#include <jni.h>
#include <android/log.h>



namespace ParaEngine {
	const std::string ParaEngineWebView::classname = "com/tatfook/paracraft/ParaEngineWebViewHelper";

	ParaEngineWebView::ParaEngineWebView()
		: m_handle(-1)
	{

	}

	ParaEngineWebView::~ParaEngineWebView()
	{
		if (m_handle != -1)
		{
			JniHelper::callStaticVoidMethod(classname, "removeWebView", m_handle);
		}
	}

	ParaEngineWebView* ParaEngineWebView::createWebView()
	{
		JniMethodInfo t;
		if (JniHelper::getStaticMethodInfo(t, classname.c_str(), "createWebView", "()I")) 
		{
			jint handle = t.env->CallStaticIntMethod(t.classID, t.methodID);
			t.env->DeleteLocalRef(t.classID);
			ParaEngineWebView* pView = new ParaEngineWebView();
			pView->setHandle(handle);
			return pView;
		}
		else
		{
			return nullptr;
		}
	}

	void ParaEngineWebView::loadUrl(const std::string &url, bool cleanCachedData)
	{
		JniHelper::callStaticVoidMethod(classname, "loadUrl", m_handle, url, cleanCachedData);
	}

	int ParaEngineWebView::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		// install parent fields if there are any. Please replace __super with your parent class name.
		IAttributeFields::InstallFields(pClass, bOverride);
		PE_ASSERT(pClass != nullptr);

		return S_OK;
	}

} // end namespace


extern "C" {

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineWebViewHelper_onJsCallback(JNIEnv *env, jclass, jint index, jstring jmessage)
	{

	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineWebViewHelper_didFailLoading(JNIEnv *env, jclass, jint index, jstring jmessage)
	{

	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineWebViewHelper_didFinishLoading(JNIEnv *env, jclass, jint index, jstring jmessage)
	{

	}

	JNIEXPORT jboolean JNICALL Java_com_tatfook_paracraft_ParaEngineWebViewHelper_shouldStartLoading(JNIEnv *env, jclass, jint index, jstring jmessage)
	{
		return 1;
	}
} // end extern