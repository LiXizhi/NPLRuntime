#pragma once

#include <string>
#include <functional>

namespace ParaEngine {

	class IParaWebView : public IAttributeFields
	{
	public:
		virtual IAttributeFields* GetAttributeObject() = 0;
		virtual void loadUrl(const std::string &url, bool cleanCachedData = false) = 0;
		virtual void setAlpha(float a) = 0;
		virtual void setVisible(bool bVisible) = 0;
		virtual void SetHideViewWhenClickBack(bool b) = 0;

		// return false IParaWebView must auto release, return ture IParaWebView release by user
		typedef std::function<bool()> onCloseFunc;
		virtual void addCloseListener(onCloseFunc fun) = 0;

		static IParaWebView* createWebView(int x, int y, int w, int h);
	};

} // end namespace