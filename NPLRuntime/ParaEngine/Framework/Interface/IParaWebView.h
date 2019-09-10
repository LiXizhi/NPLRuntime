#pragma once

#include <string>
#include <functional>

namespace ParaEngine {

	class IParaWebView : public IAttributeFields
	{
	public:

		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "IParaWebView"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);


		ATTRIBUTE_METHOD1(IParaWebView, loadUrl_s, const char*) { cls->loadUrl(p1); return S_OK; }
		ATTRIBUTE_METHOD1(IParaWebView, setAlpha_s, float) { cls->setAlpha(p1); return S_OK; }
		ATTRIBUTE_METHOD1(IParaWebView, setVisible_s, bool) { cls->setVisible(p1); return S_OK; }
		ATTRIBUTE_METHOD1(IParaWebView, SetHideViewWhenClickBack_s, bool) { cls->SetHideViewWhenClickBack(p1); return S_OK; }
		ATTRIBUTE_METHOD(IParaWebView, Refresh_s) { cls->Refresh(); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebView, hideCloseButton_s, bool) { cls->hideCloseButton(p1); return S_OK; }
		ATTRIBUTE_METHOD(IParaWebView, bringToTop_s) { cls->bringToTop(); return S_OK; }
        ATTRIBUTE_METHOD(IParaWebView, move_s) { cls->move(); return S_OK; }
        ATTRIBUTE_METHOD(IParaWebView, resize_s) { cls->resize(); return S_OK; }
        ATTRIBUTE_METHOD(IParaWebView, close_s) {cls->close(); return S_OK; }

		IAttributeFields* GetAttributeObject();

		virtual void loadUrl(const std::string &url, bool cleanCachedData = false) = 0;
		virtual void setAlpha(float a) = 0;
		virtual void setVisible(bool bVisible) = 0;
		virtual void SetHideViewWhenClickBack(bool b) = 0;
		virtual void Refresh() = 0;
        virtual void hideCloseButton(bool bHide) = 0;
		virtual void bringToTop() = 0;
        virtual void move() = 0;
        virtual void resize() = 0;
        virtual void close() = 0;

		// return false IParaWebView must auto release, return ture IParaWebView release by user
		typedef std::function<bool()> onCloseFunc;
		virtual void addCloseListener(onCloseFunc fun) = 0;

		static IParaWebView* createWebView(int x, int y, int w, int h);
		static IParaWebView* createSubViewView(int x, int y, int w, int h);
	};

} // end namespace
