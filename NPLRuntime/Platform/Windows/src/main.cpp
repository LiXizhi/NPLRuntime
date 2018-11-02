//-----------------------------------------------------------------------------
// Class:	ParaWorld Game Entry File
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Dev Studio
// Date:	2004.3.8
// Revised: 2010.2.18
// Desc: The main game class. WinMain(). Game specific logic is here.
// Please see the macro comments
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "resource.h"

#include "ParaEngineService.h"
#include "ParaEngineCore.h"

#include "PluginAPI.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"
#include "guicon.h"

#include "memdebug.h"
#include "CommandLineParams.h"

#include "RenderWindowDelegate.h"
#include "WindowsApplication.h"

using namespace ParaEngine;
extern HINSTANCE g_hAppInstance;

extern "C" __declspec(dllexport) const char* LibDescription();

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
	g_hAppInstance = hInst;
	int exit_code = 0;

#if defined(_DEBUG)
	RedirectIOToConsole();
#endif

	RenderWindowDelegate renderWindow;

	CWindowsApplication app(hInst);
	app.InitApp(&renderWindow, lpCmdLine);
	app.Run(hInst);

	return 0;
}
