#pragma once
#include <jni.h>
#include <string>

namespace ParaEngine {

	class ParaEngineWebView : public IAttributeFields
	{
	public:
		typedef ParaEngine::weak_ptr<IObject, ParaEngineWebView> WeakPtr_type;
		static ParaEngineWebView* createWebView();

		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "ParaEngineWebView"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		void loadUrl(const std::string &url, bool cleanCachedData = false);

	private:
		static const std::string classname;
	protected:
		ParaEngineWebView();
		virtual ~ParaEngineWebView();

		void setHandle(int handle) { m_handle = handle;  }

	private:
		int m_handle;
	};


} // end namespace