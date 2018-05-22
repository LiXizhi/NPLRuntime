#pragma once
#include "ParaScriptingGlobal.h"
#include "IParaWebView.h"
#include <jni.h>
#include <string>
#include <unordered_map>


namespace ParaEngine {


	class ParaEngineWebView : public IParaWebView
	{
	public:
		virtual ~ParaEngineWebView();

		typedef ParaEngine::weak_ptr<IObject, ParaEngineWebView> WeakPtr_type;
		static ParaEngineWebView* createWebView(int x, int y, int w, int h);

		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "ParaEngineWebView"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);


		ATTRIBUTE_METHOD1(ParaEngineWebView, loadUrl_s, const char*) { cls->loadUrl(p1); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineWebView, setAlpha_s, float) { cls->setAlpha(p1); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineWebView, setVisible_s, bool) { cls->setVisible(p1); return S_OK; }
		ATTRIBUTE_METHOD1(ParaEngineWebView, SetHideViewWhenClickBack_s, bool) { cls->SetHideViewWhenClickBack(p1); return S_OK; }
		ATTRIBUTE_METHOD(ParaEngineWebView, Refresh_s) { cls->Refresh(); return S_OK; }

		virtual int Release() override;

		virtual void loadUrl(const std::string &url, bool cleanCachedData = false) override;
		virtual void setAlpha(float a) override;
		virtual void setVisible(bool bVisible) override;
		virtual void SetHideViewWhenClickBack(bool b) override;
		virtual IAttributeFields* GetAttributeObject() override;
		virtual void addCloseListener(onCloseFunc fun) override;
		virtual void Refresh() override;

		// test interface
		static bool openWebView(int x, int y, int w, int h, const std::string& url);
		static bool closeWebView();


		static void onCloseView(int handle);


	private:
		static const std::string classname;
		static std::unordered_map<int, ParaEngineWebView*> m_views;

		onCloseFunc m_onClose;
	protected:
		ParaEngineWebView();
		

		void setHandle(int handle) { m_handle = handle;  }

	private:
		int m_handle;
	};


} // end namespace