//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2010 ParaEngine Corporation, All Rights Reserved.
// Date:	2005.4
// Description:	API for ParaEngine core interface 
//-----------------------------------------------------------------------------

#pragma once

#include "baseinterface.h"
namespace ParaEngine
{
	class IParaEngineApp;

	/**
	*  a table of virtual functions which are used by plug-ins to access the game engine 
	*/
	class IParaEngineCore : BaseInterface
	{
	public:
		virtual DWORD GetVersion()=0;
		virtual HWND GetParaEngineHWnd()=0;
		/** render the current frame and does not return until everything is presented to screen. 
		* this function is usually used to draw the animated loading screen. */
		virtual bool ForceRender()=0;
		/** cause the main thread to sleep for the specified seconds.*/
		virtual bool Sleep(float fSeconds)=0;

		/** get the ParaEngine app interface. Please note that this function may return NULL, if no app is created. */
		virtual IParaEngineApp* GetAppInterface()=0;

		/** create an ParaEngine application. Each application is associated with a win32 window. 
		* this function is mostly called from host application to create the application object. 
		*/
		virtual IParaEngineApp* CreateApp()=0;
	};
}
