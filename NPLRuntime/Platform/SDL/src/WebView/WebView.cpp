#include "ParaEngine.h"
#include "WebView.h"

#ifdef __EMSCRIPTEN__
#include "Globals.h"
#include "3dengine/ParaEngineSettings.h"
#include "2dengine/GUIRoot.h"
#include "json/json.h"
#include <atomic>
#endif

namespace ParaEngine
{
#ifdef __EMSCRIPTEN__
	namespace
	{
		std::atomic<uint64_t> s_nextWebViewId{ 1 };
	}

	IParaWebView* IParaWebView::createWebView(int x, int y, int w, int h)
	{
		return ParaEngineWebView::createWebView(x, y, w, h);
	}

	IParaWebView* IParaWebView::createSubViewView(int x, int y, int w, int h)
	{
		return ParaEngineWebView::createSubWebView(x, y, w, h);
	}

	ParaEngineWebView* ParaEngineWebView::createWebView(int x, int y, int w, int h)
	{
		return new ParaEngineWebView(x, y, w, h, false);
	}

	ParaEngineWebView* ParaEngineWebView::createSubWebView(int x, int y, int w, int h)
	{
		return new ParaEngineWebView(x, y, w, h, true);
	}

	ParaEngineWebView::ParaEngineWebView(int x, int y, int w, int h, bool bSubView)
		: m_id("pe_wv_" + std::to_string(s_nextWebViewId.fetch_add(1)))
		, m_x(x)
		, m_y(y)
		, m_width(w > 0 ? w : 800)
		, m_height(h > 0 ? h : 600)
		, m_alpha(1.0f)
		, m_bVisible(true)
		, m_bSubView(bSubView)
		, m_bStarted(false)
	{
	}

	ParaEngineWebView::~ParaEngineWebView()
	{
		if (m_bStarted)
		{
			SendJsCommand("Quit");
			m_bStarted = false;
		}
	}

	void ParaEngineWebView::SendJsCommand(const std::string& cmd, const std::string& toFilename, const std::string& params)
	{
		float scaleX = 1.0f;
		float scaleY = 1.0f;
		if (CGUIRoot::GetInstance() != nullptr)
		{
			CGUIRoot::GetInstance()->GetUIScale(&scaleX, &scaleY);
		}

		Json::Value payload(Json::objectValue);
		payload["target"] = "webview";
		payload["cmd"] = cmd;
		payload["id"] = m_id;
		payload["x"] = m_x;
		payload["y"] = m_y;
		payload["width"] = m_width;
		payload["height"] = m_height;
		payload["scale_x"] = scaleX;
		payload["scale_y"] = scaleY;
		payload["visible"] = m_bVisible;
		payload["transparent"] = (m_bSubView || m_alpha < 0.999f);
		if (!m_url.empty())
		{
			payload["url"] = m_url;
		}
		if (!toFilename.empty() || !params.empty())
		{
			payload["to_filename"] = toFilename;
			payload["params"] = params;
		}

		Json::FastWriter writer;
		std::string jsonStr = writer.write(payload);
		CGlobals::GetSettings().SendMsgToJS(jsonStr.c_str());
	}

	void ParaEngineWebView::loadUrl(const std::string& url, bool cleanCachedData)
	{
		(void)cleanCachedData;
		m_url = url;
		if (!m_bStarted)
		{
			m_bStarted = true;
			SendJsCommand("Start");
		}
		else
		{
			SendJsCommand("Open");
		}
	}

	void ParaEngineWebView::setAlpha(float a)
	{
		m_alpha = a;
	}

	void ParaEngineWebView::setVisible(bool bVisible)
	{
		m_bVisible = bVisible;
		if (m_bStarted)
		{
			SendJsCommand("Show");
		}
	}

	void ParaEngineWebView::SetHideViewWhenClickBack(bool b)
	{
		(void)b;
	}

	void ParaEngineWebView::SetIgnoreCloseWhenClickBack(bool b)
	{
		(void)b;
	}

	void ParaEngineWebView::SetCloseWhenClickBackground(bool b)
	{
		(void)b;
	}

	void ParaEngineWebView::Refresh()
	{
		if (m_bStarted && !m_url.empty())
		{
			SendJsCommand("Open");
		}
	}

	void ParaEngineWebView::hideCloseButton(bool bHide)
	{
		(void)bHide;
	}

	void ParaEngineWebView::addCloseListener(onCloseFunc fun)
	{
		m_onClose = std::move(fun);
	}

	void ParaEngineWebView::bringToTop()
	{
		if (m_bStarted && m_bVisible)
		{
			SendJsCommand("Show");
		}
	}

	void ParaEngineWebView::move(int x, int y)
	{
		m_x = x;
		m_y = y;
		if (m_bStarted)
		{
			SendJsCommand("ChangePosSize");
		}
	}

	void ParaEngineWebView::resize(int width, int height)
	{
		m_width = width > 0 ? width : 1;
		m_height = height > 0 ? height : 1;
		if (m_bStarted)
		{
			SendJsCommand("ChangePosSize");
		}
	}

	void ParaEngineWebView::activate(const std::string& filepath, const std::string& msg)
	{
		if (!m_bStarted)
			return;
		SendJsCommand("PostMessage", filepath, msg);
	}
#else
	IParaWebView* IParaWebView::createWebView(int x, int y, int w, int h)
	{
		(void)x; (void)y; (void)w; (void)h;
		return nullptr;
	}

	IParaWebView* IParaWebView::createSubViewView(int x, int y, int w, int h)
	{
		(void)x; (void)y; (void)w; (void)h;
		return nullptr;
	}
#endif
} // namespace ParaEngine
