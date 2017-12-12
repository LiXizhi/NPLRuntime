#pragma once
#include "PEtypes.h"

namespace NPL{
	class INPLRuntime;
}

namespace ParaEngine
{
	/** player message */
	struct CPlayerMessage
	{

	};

	/** ParaPhysics core interface. 
	*/
	class IParaWebPlayer
	{
	public:
		enum PWP_MSG_CODE{
			PWP_FAILED_LOAD_PELIB,
			PWP_FAILED_CREATE_PEAPP,
			PWP_FAILED_START_PEAPP,
			PWP_FAILED_CREATE_DEVICE,
			PWP_FAILED_LOCATE_PARAENGINE_DIR,
			PWP_FAILED_CREATE_PE_AUTOUPDATER,
		};
		/** get the interface version */
		virtual int GetVersion() = 0;

		/** whether this is a debug build. */
		virtual bool IsDebugging() = 0;

		/** find the ParaEngine dir and set as working directory. 
		* @param sHintDir: give us a dir to search for. 
		*/
		virtual void FindParaEngineDirectory(const char* sHintDir) = 0;

		/** start command line.*/
		virtual int Start(const char* sCmdLine) = 0;

		/** force updating the core library in the current working directory. 
		* It will send following messages. this function returns when update is complete
		*	'updt.s': update progress started. value is msg
		*	'updt.p': update progress report. var1 is finished count, var2 is allcount
		*	'updt.e': error occurs during update. value is the error message. 
		*	'updt.u': unknown version detected. value is the error message. 
		*	'updt.c': completed. value is the current version. 
		*	'updt.n': no change detected. value is the current version. 
		*	'updt.b': broken file during file transfer. var1 is finished count, var2 is allcount
		*/
		virtual bool DoCoreUpdate() = 0;

		/** begin core update. one can use TryGetNextCoreUpdateMessage() to process the update message. 
		* @param sCurVersion: "0" is forcing full update, "" is using the current version in the current working directory. or a version number like "0.2.70"
		* @param sSessionDir: we will keep temporary download file under update/[sUpdaterName]/[version] directory. if this is NULL, we will use the default name "web"
		*/
		virtual bool BeginCoreUpdate(const char* sCurVersion, const char* sSessionDir) = 0;

		/**
		* same as DoCoreUpdate(), except that it allows us to poll the next message. 
		* typically, we can call it from a timer that periodically check if there is a new update message. 
		*/
		virtual bool TryGetNextCoreUpdateMessage() = 0;

		/** set the hWnd on to which we will render and process window messages. 
		* this function should be called prior to StartApp(). If this function is not called, ParaEngine will create its own window for rendering. 
		* @note: the rendering device size will use the client area of the input window
		* @param hWnd: the Window on to which we will render. 
		*/
		virtual void SetMainWindow(HWND hWnd) = 0;
		virtual HWND GetMainWindow() = 0;

		/** call this function for processing main window messages in the browser main window thread. 
		* return false to let the default window message procedure process or true if intercepted. 
		*/
		virtual bool CustomWinProc(HWND hWnd, UINT uMsg, void* wParam, void* lParamm,void* lRes) = 0;

		/** the window message processor. One needs send all messages belonging to the main window to this function, after calling Create().  */
		// virtual IMessageSystem* GetMessageSystem() = 0;

		/** this function is called whenever the application is disabled or enabled. usually called when receiving the WM_ACTIVATEAPP message. 
		* [main thread only]
		*/
		virtual void ActivateApp( bool bActivate) = 0;

		/** all this to unload the IPhysics. Pointer to this class will be invalid after the call
		*/
		virtual void Release() = 0;

		/**
		* get the NPL runtime for browser to send messages to the player. 
		*/
		virtual NPL::INPLRuntime* GetPlayerNPLRuntime() = 0;

		/** set NPL runtime in the browser plugin. so that we can send message to it. 
		*/
		virtual void SetBrowserNPLSystem(NPL::INPLRuntime* pTarget) = 0;

		/** set the redist directory which contains this dll. This function is called by the browser plugin when this dll is loaded. */
		virtual void SetRedistDir(const char* sDir) = 0;
	};
}