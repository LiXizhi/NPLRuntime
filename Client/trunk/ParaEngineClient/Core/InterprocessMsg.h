#pragma once
//-----------------------------------------------------------------------------
// Class:	Interprocess Application messages
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.4.22
//-----------------------------------------------------------------------------
namespace ParaEngine
{
	/**
	* interprocess application messages between app client process to host app process
	* The host app process create the app client process from command line. 
	* command line contains "appid" and "apphost" parameters which specifies the interprocess message queue used between these two processes. 
	* e.g. "D:\lxzsrc\ParaEngine\ParaWorld\ParaEngineClient_d.exe" appid="MyApp" apphost="MyWinForm" bootstrapper="script/apps/Taurus/bootstrapper.xml"
	*/
	enum PeAppMsgEnum
	{
		/// set the parent window, param1 = HWND(parent window hwnd)
		PEAPP_SetParentWindow = 0,
		/// start the application, param1=min_resolution_width, param2 = min_resolution_height,
		PEAPP_Start,
		/// stop the application, it will terminate the app. and can not be started again. 
		PEAPP_Stop,
		/// Parent Window Size Changed, param1=width, param2=height
		PEAPP_OnSizeChange,
		/// whenever receives focus, we usually activate the application and render with higher frame rate when has focus. 
		PEAPP_FocusIn,
		/// whenever loses focus
		PEAPP_FocusOut,
		/// send a general log message. m_code contains the log string. 
		PEAPP_Log,
		/// send a general error message. parm1 contains the error code and m_code contains the error string. 
		PEAPP_Error,
		/// set working directory. m_code contains the working dir. For security reasons, it is not advised to use this function and may be removed without notice in future
		PEAPP_SetWorkingDir,
		/// activate an NPL file in the current process. Please note that, the NPL file must be trusted by the client in order to be activated. using NPL.AddPublicFile(). 
		PEAPP_NPL_Activate,
		
		/// ParaEngine loading progress, those messages are sent back from app client process to host app process. 
		PEAPP_LoadingProgress_LoadingLibs = 100,
		PEAPP_LoadingProgress_CreatingApp,
		PEAPP_LoadingProgress_StartingApp,
		PEAPP_LoadingProgress_CreatingDevice,
		PEAPP_LoadingProgress_GameLoopStarted,

		/// sent by the host to client to begin core updating, by loading the AutoUpdater.dll under the current working directory. One needs to set current working dir properly before calling this function.  
		PEAPP_BeginCoreUpdate = 200,
		/// sent by the app client to host to inform core update progress. 
		PEAPP_UpdateProgress_Started,
		/** param1 is bytes finished, param2 is total bytes. */
		PEAPP_UpdateProgress_Progress,
		PEAPP_UpdateProgress_Error,
		PEAPP_UpdateProgress_UnknownVersion,
		PEAPP_UpdateProgress_BrokenFile,
		/// when update complete or no change detected, the host can send PEAPP_SetParentWindow and PEAPP_Start to launch the core game engine. 
		PEAPP_UpdateProgress_Completed,
		PEAPP_UpdateProgress_NoChangeDetected,
	};
}