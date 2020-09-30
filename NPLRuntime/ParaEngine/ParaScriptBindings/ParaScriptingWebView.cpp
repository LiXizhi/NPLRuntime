#include "ParaEngine.h"
#include "ParaScriptingWebView.h"
#include "ParaScripting.h"

#include <luabind/luabind.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/raw_policy.hpp>

namespace ParaScripting
{
#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_ANDROID)
	void ParaWebView::setOrientation(int type)
	{
		IParaWebView::setOrientation(type);
	}
#endif
	ParaWebView ParaWebView::createWebView(int x, int y, int w, int h)
	{
		auto pWebView = IParaWebView::createWebView(x, y, w, h);
		return ParaWebView(pWebView);
	}

	ParaWebView ParaWebView::createSubViewView(int x, int y, int w, int h)
	{
		auto pSubViewView = IParaWebView::createSubViewView(x, y, w, h);
		return ParaWebView(pSubViewView);
	}

	ParaWebView::ParaWebView(IParaWebView* wv)
	{
		m_pParaWebView.reset(wv);

		wv->addCloseListener([this]() {
			this->m_pParaWebView.reset();
			return true;
		});
	}

	ParaAttributeObject ParaWebView::GetAttributeObject()
	{
		if (m_pParaWebView)
			return ParaAttributeObject(m_pParaWebView->GetAttributeObject());
		else
			return ParaAttributeObject();
	}

	void ParaWebView::loadUrl1(const std::string &url)
	{
		loadUrl2(url, false);
	}

	void ParaWebView::loadUrl2(const std::string &url, bool cleanCachedData)
	{
		if (m_pParaWebView)
		{
			m_pParaWebView->loadUrl(url, cleanCachedData);
		}
	}

	void ParaWebView::closeAndRelease()
	{
		m_pParaWebView.reset();
	}

#ifdef CROSS_PLATFORM
	void CNPLScriptingState::LoadHAPI_WebView()
	{
		using namespace luabind;
		lua_State* L = GetLuaState();
		module(L)
		[
			namespace_("WebView")
			[
				class_<ParaWebView>("ParaWebView")
					.def("GetAttributeObject", &ParaWebView::GetAttributeObject)
					.def("loadUrl", &ParaWebView::loadUrl1)
					.def("loadUrl", &ParaWebView::loadUrl2)
					.def("closeAndRelease", &ParaWebView::closeAndRelease),
#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_ANDROID)
				def("setOrientation", ParaWebView::setOrientation),
#endif
				def("createWebView", ParaWebView::createWebView),
				def("createSubViewView", ParaWebView::createSubViewView)
			]
		];
	}
#endif

} // end namespace

