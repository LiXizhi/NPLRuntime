#include "ParaEngine.h"
#include "WebView.h"
#include "NPLRuntime.h"
#include "NPLHelper.h"
#include "PluginManager.h"
#include <atomic>
#include <sstream>

namespace ParaEngine
{
	namespace
	{
		std::atomic<uint64_t> s_nextWebViewId{ 1 };

		const char* GetWebViewPluginDllName()
		{
#ifdef _DEBUG
			return "ParaWebView_d.dll";
#else
			return "ParaWebView.dll";
#endif
		}
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
			SendPluginCommand("Quit");
			m_bStarted = false;
		}
	}

	void ParaEngineWebView::SendPluginCommand(const std::string& cmd, const std::string& extraFields)
	{
		auto* pRuntime = CGlobals::GetNPLRuntime();
		NPL::NPLRuntimeState_ptr pMainState = pRuntime ? pRuntime->GetMainRuntimeState() : NPL::NPLRuntimeState_ptr();
		if (!pMainState)
			return;

		HWND hParent = CGlobals::GetAppHWND();
		std::ostringstream oss;
		oss << "msg={cmd=";
		std::string quoted;
		NPL::NPLHelper::EncodeStringInQuotation(quoted, 0, cmd.c_str());
		oss << quoted;
		quoted.clear();
		NPL::NPLHelper::EncodeStringInQuotation(quoted, 0, m_id.c_str());
		oss << ",id=" << quoted;
		oss << ",x=" << m_x << ",y=" << m_y << ",width=" << m_width << ",height=" << m_height;
		oss << ",visible=" << (m_bVisible ? "true" : "false");
		oss << ",transparent=" << ((m_bSubView || m_alpha < 0.999f) ? "true" : "false");
		if (hParent)
		{
			oss << ",parent_handle=\"" << reinterpret_cast<uintptr_t>(hParent) << "\"";
		}
		if (!m_url.empty())
		{
			quoted.clear();
			NPL::NPLHelper::EncodeStringInQuotation(quoted, 0, m_url.c_str());
			oss << ",url=" << quoted;
		}
		if (!extraFields.empty())
		{
			oss << "," << extraFields;
		}
		oss << "};";

		const std::string code = oss.str();
		pMainState->activate(GetWebViewPluginDllName(), code.c_str(), static_cast<int>(code.size()));
	}

	void ParaEngineWebView::loadUrl(const std::string& url, bool cleanCachedData /*= false*/)
	{
		(void)cleanCachedData;
		m_url = url;
		if (!m_bStarted)
		{
			m_bStarted = true;
			SendPluginCommand("Start");
		}
		else
		{
			SendPluginCommand("Open");
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
			SendPluginCommand("Show");
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
			SendPluginCommand("Open");
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
			SendPluginCommand("Show");
		}
	}

	void ParaEngineWebView::move(int x, int y)
	{
		m_x = x;
		m_y = y;
		if (m_bStarted)
		{
			SendPluginCommand("ChangePosSize");
		}
	}

	void ParaEngineWebView::resize(int width, int height)
	{
		m_width = width > 0 ? width : 1;
		m_height = height > 0 ? height : 1;
		if (m_bStarted)
		{
			SendPluginCommand("ChangePosSize");
		}
	}

	void ParaEngineWebView::activate(const std::string& filepath, const std::string& msg)
	{
		if (!m_bStarted)
			return;
		std::ostringstream payload;
		std::string qFile, qMsg;
		NPL::NPLHelper::EncodeStringInQuotation(qFile, 0, filepath.c_str());
		NPL::NPLHelper::EncodeStringInQuotation(qMsg, 0, msg.c_str());
		std::string jsonPayload = "{\"file\":" + qFile + ",\"params\":" + qMsg + "}";
		std::string qContent;
		NPL::NPLHelper::EncodeStringInQuotation(qContent, 0, jsonPayload.c_str());
		payload << "message_content=" << qContent;
		SendPluginCommand("CallJsFunc", payload.str());
	}
} // namespace ParaEngine
