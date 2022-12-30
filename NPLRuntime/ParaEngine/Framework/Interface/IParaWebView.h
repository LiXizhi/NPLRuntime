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
		ATTRIBUTE_METHOD1(IParaWebView, SetIgnoreCloseWhenClickBack_s, bool) { cls->SetIgnoreCloseWhenClickBack(p1); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebView, SetCloseWhenClickBackground_s, bool) { cls->SetCloseWhenClickBackground(p1); return S_OK; }
		ATTRIBUTE_METHOD(IParaWebView, Refresh_s) { cls->Refresh(); return S_OK; }
        ATTRIBUTE_METHOD1(IParaWebView, hideCloseButton_s, bool) { cls->hideCloseButton(p1); return S_OK; }
		ATTRIBUTE_METHOD(IParaWebView, bringToTop_s) { cls->bringToTop(); return S_OK; }
		ATTRIBUTE_METHOD2(IParaWebView, move_s, float) { cls->move((int)p1, (int)p2); return  S_OK;  }
		ATTRIBUTE_METHOD2(IParaWebView, resize_s, float) { cls->resize((int)p1, (int)p2); return  S_OK; }

		IAttributeFields* GetAttributeObject();

		virtual void loadUrl(const std::string &url, bool cleanCachedData = false) = 0;
		virtual void setAlpha(float a) = 0;
		virtual void setVisible(bool bVisible) = 0;
		virtual void SetHideViewWhenClickBack(bool b) = 0;
		virtual void SetIgnoreCloseWhenClickBack(bool b) = 0;
        virtual void SetCloseWhenClickBackground(bool b) = 0;
		virtual void Refresh() = 0;
        virtual void hideCloseButton(bool bHide) = 0;
		virtual void bringToTop() = 0;
        virtual void move(int x, int y) = 0;
        virtual void resize(int width, int height) = 0;

		// return false IParaWebView must auto release, return ture IParaWebView release by user
		typedef std::function<bool()> onCloseFunc;
		virtual void addCloseListener(onCloseFunc fun) = 0;

#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_ANDROID)
		static IParaWebView* setOrientation(int type);
#endif
		static IParaWebView* createWebView(int x, int y, int w, int h);
		static IParaWebView* createSubViewView(int x, int y, int w, int h);
	};

} // end namespace
