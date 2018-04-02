#include "ParaEngine.h"
#include "ParaEngineWebViewHelper.h"
#include "AppDelegate.h"
#include "AttributeClass.h"
#include "JniHelper.h"
#include <jni.h>
#include <android/log.h>



namespace ParaEngine {
	const std::string ParaEngineWebView::classname = "com/tatfook/paracraft/ParaEngineWebViewHelper";
	std::unordered_map<int, ParaEngineWebView*> ParaEngineWebView::m_views;

	ParaEngineWebView::ParaEngineWebView()
		: m_handle(-1)
	{
		CGlobals::GetApp()->ActivateApp(false);
	}

	ParaEngineWebView::~ParaEngineWebView()
	{
		if (m_handle != -1)
		{
			JniHelper::callStaticVoidMethod(classname, "removeWebView", m_handle);
		}
		CGlobals::GetApp()->ActivateApp(true);
	}

	ParaEngineWebView* ParaEngineWebView::createWebView(int x, int y, int w, int h)
	{
		JniMethodInfo t;
		if (JniHelper::getStaticMethodInfo(t, classname.c_str(), "createWebView", "(IIII)I")) 
		{
			jint handle = t.env->CallStaticIntMethod(t.classID, t.methodID, x, y, w, h);
			t.env->DeleteLocalRef(t.classID);
			ParaEngineWebView* pView = new ParaEngineWebView();
			pView->setHandle(handle);
			m_views[handle] = pView;
			return pView;
		}
		else
		{
			return nullptr;
		}
	}

	void ParaEngineWebView::setAlpha(float a)
	{
		JniHelper::callStaticVoidMethod(classname, "setViewAlpha", m_handle, a);
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

	void ParaEngineWebView::onCloseView(int handle)
	{
		auto it = m_views.find(handle);
		if (it != m_views.end())
		{
			auto pView = it->second;
			pView->m_handle = -1;
			pView->Release();
			m_views.erase(it);
		}
		CGlobals::GetApp()->ActivateApp(true);
	}

	bool ParaEngineWebView::openWebView(int x, int y, int w, int h, const std::string& url)
	{
		JniHelper::callStaticVoidMethod(classname, "openWebView", x, y, w, h, url);
		return true;
	}

	bool ParaEngineWebView::closeWebView()
	{
		JniHelper::callStaticVoidMethod(classname, "closeWebView");
		return true;
	}

} // end namespace


using namespace ParaEngine;

extern "C" {
	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineWebViewHelper_onCloseView(JNIEnv *env, jclass, jint index)
	{
		ParaEngineWebView::onCloseView(index);
	}


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
		return 0;
	}
} // end extern