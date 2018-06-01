#include "ParaEngine.h"
#include "ParaEngineWebViewHelper.h"
#include "AppDelegate.h"
#include "AttributeClass.h"
#include "JniHelper.h"
#include "NPLRuntime.h"
#include "NPLScriptingState.h"


#include <luabind/luabind.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include <luabind/copy_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/function.hpp>

#include <jni.h>
#include <android/log.h>



namespace ParaEngine {
	const std::string ParaEngineWebView::classname = "com/tatfook/paracraft/ParaEngineWebViewHelper";
	std::unordered_map<int, ParaEngineWebView*> ParaEngineWebView::m_views;

	IParaWebView* IParaWebView::createWebView(int x, int y, int w, int h)
	{
		return ParaEngineWebView::createWebView(x, y, w, h);
	}

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

			if (m_views.size() >= 1)
			{
				OUTPUT_LOG("ParaEngineWebView: ActivateApp(false)");
				CGlobals::GetApp()->ActivateApp(false);
			}

			return pView;
		}
		else
		{
			return nullptr;
		}
	}



	int ParaEngineWebView::Release()
	{
		if (GetRefCount() <= 1)
		{
			auto it = m_views.find(m_handle);
			if (it != m_views.end())
			{
				m_views.erase(it);
			}

			if (m_views.size() == 0 )
			{
				OUTPUT_LOG("ParaEngineWebView: ActivateApp(true)");
				CGlobals::GetApp()->ActivateApp(true);
			}
		}

		return IAttributeFields::Release();
	}

	void ParaEngineWebView::onCloseView(int handle)
	{
		auto it = m_views.find(handle);
		if (it != m_views.end())
		{
			auto pView = it->second;
			m_views.erase(it);

			//OUTPUT_LOG("ParaEngineWebView: m_views size %d", m_views.size());

			pView->m_handle = -1;

			if (pView->m_onClose == nullptr)
				pView->Release();
			else
			{
				if (!pView->m_onClose())
				{
					pView->Release();
				}
			}
		}
	}

	void ParaEngineWebView::setAlpha(float a)
	{
		JniHelper::callStaticVoidMethod(classname, "setViewAlpha", m_handle, a);
	}

	void ParaEngineWebView::setVisible(bool bVisible)
	{
		JniHelper::callStaticVoidMethod(classname, "setVisible", m_handle, bVisible);
	}


	void ParaEngineWebView::SetHideViewWhenClickBack(bool b)
	{
		JniHelper::callStaticVoidMethod(classname, "SetHideViewWhenClickBack", m_handle, b);
	}


	void ParaEngineWebView::loadUrl(const std::string &url, bool cleanCachedData)
	{
		JniHelper::callStaticVoidMethod(classname, "loadUrl", m_handle, url, cleanCachedData); 
	}  

	void ParaEngineWebView::hideCloseButton(bool bHide)
	{

	}

	void ParaEngineWebView::addCloseListener(onCloseFunc fun)
	{
		m_onClose = fun;
	}
	 
	void ParaEngineWebView::Refresh()
	{
		JniHelper::callStaticVoidMethod(classname, "reload", m_handle);
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
		return JNI_TRUE;
	}

	JNIEXPORT void JNICALL Java_com_tatfook_paracraft_ParaEngineWebViewHelper_transportCmdLine(JNIEnv *env, jclass, jstring value)
	{
		std::string cmd = JniHelper::jstring2string(value);
		env->DeleteLocalRef(value);
		AppDelegate::getInstance().onCmdLine(cmd);
	}


} // end extern