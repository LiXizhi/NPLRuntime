#pragma once

#include "IParaEngineCore.h"

namespace ParaEngine
{
	class ClassDescriptor;
	class CParaEngineApp;

	ClassDescriptor* ParaEngine_GetClassDesc();

	/**
	* It implements the IParaEngineCore interface, which exposes everything in ParaEngine to plug-in applications. 
	*/
	class CParaEngineCore : public IParaEngineCore
	{
	public:
		CParaEngineCore(void);
		~CParaEngineCore(void);
		static IParaEngineCore* GetStaticInterface();
	public:
		virtual DWORD GetVersion();
		
		/** cause the main thread to sleep for the specified seconds.*/
		virtual bool Sleep(float fSeconds);

		/** return invalid */
		virtual HWND GetParaEngineHWnd();

		/** render the current frame and does not return until everything is presented to screen. 
		* this function is usually used to draw the animated loading screen. */
		virtual bool ForceRender();

		/** get the ParaEngine app interface. Please note that this function may return NULL, if no app is created. */
		virtual IParaEngineApp* GetAppInterface();

		/** create an ParaEngine application. Each application is associated with a win32 window. 
		* this function is mostly called from host application to create the application object. 
		*/
		virtual IParaEngineApp* CreateApp();

	public:
		static CParaEngineCore* GetInstance();
		/** it is recommended to call this function. */
		void DestroySingleton();
	protected:
		static WeakPtr m_pAppSingleton;
	};

}
