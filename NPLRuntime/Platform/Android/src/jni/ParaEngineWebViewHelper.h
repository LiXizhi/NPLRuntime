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

		virtual int Release() override;

		virtual void loadUrl(const std::string &url, bool cleanCachedData = false) override;
		virtual void setAlpha(float a) override;
		virtual void setVisible(bool bVisible) override;
		virtual void SetHideViewWhenClickBack(bool b) override;
		virtual void addCloseListener(onCloseFunc fun) override;
		virtual void Refresh() override;
		virtual void hideCloseButton(bool bHide) override;
		virtual void bringToTop() override {};

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