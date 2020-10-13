#pragma once

#include "ParaScriptingCommon.h"
#include "ParaScriptingGlobal.h"
#include "IParaWebView.h"

#include <boost/shared_ptr.hpp>
#include <string>

namespace ParaScripting
{
	class PE_CORE_DECL ParaWebView
	{
	private:
		ParaWebView(IParaWebView* wv);
	public:
#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_ANDROID)
		static void setOrientation(int type);
#endif
		static ParaWebView createWebView(int x, int y, int w, int h);
		static ParaWebView createSubViewView(int x, int y, int w, int h);

		ParaAttributeObject GetAttributeObject();
		void loadUrl1(const std::string &url);
		void loadUrl2(const std::string &url, bool cleanCachedData);
		void closeAndRelease();


		typedef boost::shared_ptr<IParaWebView> IParaWebView_ptr;

	private:
		IParaWebView_ptr m_pParaWebView;
	};
} // end namespace
