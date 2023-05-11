//-----------------------------------------------------------------------------
// ParaEngineWebViewHelper.h
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

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
		static void openExternalBrowser(const std::string& url);
		static ParaEngineWebView *createWebView(int x, int y, int w, int h);
		static ParaEngineWebView *setOrientation(int type);

		virtual int Release() override;
		virtual void loadUrl(const std::string &url, bool cleanCachedData = false) override;
		virtual void setAlpha(float a) override;
		virtual void setVisible(bool bVisible) override;
		virtual void SetHideViewWhenClickBack(bool b) override;
		virtual void SetIgnoreCloseWhenClickBack(bool b) override;
        virtual void SetCloseWhenClickBackground(bool b) override;
		virtual void addCloseListener(onCloseFunc fun) override;
		virtual void Refresh() override;
		virtual void hideCloseButton(bool bHide) override;
		virtual void bringToTop() override {};
		virtual void move(int x, int y) override;
		virtual void resize(int width, int height) override;
		virtual void activate(const std::string &filepath, const std::string &msg) override;

		static bool openWebView(int x, int y, int w, int h, const std::string& url);
		// static bool closeWebView();

		static void onCloseView(int handle);

	private:
		static const std::string classname;
		static std::unordered_map<int, ParaEngineWebView*> m_views;

		onCloseFunc m_onClose;
	protected:
		ParaEngineWebView();
		void setHandle(int handle) { m_handle = handle; }

	private:
		int m_handle;
	};
}
