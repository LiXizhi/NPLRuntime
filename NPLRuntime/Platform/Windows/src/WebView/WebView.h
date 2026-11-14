#pragma once

#include "IParaWebView.h"
#include <string>

namespace ParaEngine
{
	class ParaEngineWebView : public IParaWebView
	{
	public:
		ParaEngineWebView(int x, int y, int w, int h, bool bSubView);
		virtual ~ParaEngineWebView();

		static ParaEngineWebView* createWebView(int x, int y, int w, int h);
		static ParaEngineWebView* createSubWebView(int x, int y, int w, int h);

		virtual void loadUrl(const std::string& url, bool cleanCachedData = false) override;
		virtual void setAlpha(float a) override;
		virtual void setVisible(bool bVisible) override;
		virtual void SetHideViewWhenClickBack(bool b) override;
		virtual void SetIgnoreCloseWhenClickBack(bool b) override;
		virtual void SetCloseWhenClickBackground(bool b) override;
		virtual void Refresh() override;
		virtual void hideCloseButton(bool bHide) override;
		virtual void addCloseListener(onCloseFunc fun) override;
		virtual void bringToTop() override;
		virtual void move(int x, int y) override;
		virtual void resize(int width, int height) override;
		virtual void activate(const std::string& filepath, const std::string& msg) override;

	private:
		void SendPluginCommand(const std::string& cmd, const std::string& extraFields = "");

		std::string m_id;
		std::string m_url;
		int m_x;
		int m_y;
		int m_width;
		int m_height;
		float m_alpha;
		bool m_bVisible;
		bool m_bSubView;
		bool m_bStarted;
		onCloseFunc m_onClose;
	};
} // namespace ParaEngine
