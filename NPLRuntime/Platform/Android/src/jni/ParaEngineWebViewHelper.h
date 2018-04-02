#pragma once
#include <jni.h>
#include <string>
#include <unordered_map>

namespace ParaEngine {

	class ParaEngineWebView : public IAttributeFields
	{
	public:
		virtual ~ParaEngineWebView();

		typedef ParaEngine::weak_ptr<IObject, ParaEngineWebView> WeakPtr_type;
		static ParaEngineWebView* createWebView(int x, int y, int w, int h);

		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "ParaEngineWebView"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		virtual int Release() override;

		void loadUrl(const std::string &url, bool cleanCachedData = false);
		void setAlpha(float a);

		// test interface
		static bool openWebView(int x, int y, int w, int h, const std::string& url);
		static bool closeWebView();


		static void onCloseView(int handle);
	private:
		static const std::string classname;

		static std::unordered_map<int, ParaEngineWebView*> m_views;
	protected:
		ParaEngineWebView();
		

		void setHandle(int handle) { m_handle = handle;  }

	private:
		int m_handle;
	};


} // end namespace