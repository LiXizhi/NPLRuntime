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

#include "WindowsApplication.h"


using namespace ParaEngine;

/** main window class name: used to prevent multiple instances of this application from running. */
extern WCHAR* g_sWindowClassName;

/** 
There are two different ways to create a ParaEngine Application.
	- MULTITHREADED_APP_WINDOW: the simple way is to let the CParaEngineApp to create the main window(HWND) and manage message processing automatically. Internally windows are created on a different thread. 
	- CUSTOM_APP_WINDOW: the advanced way is to create a custom window and pass its HWND to CParaEngineApp, and in the MsgProc , call CParaEngineApp's message handler. 
		the advanced way is usually used when ParaEngine is compiled as a dll. The window can be created in a different thread. See D3DWindowDefault.cpp for multithreaded custom window creation. 

Use the macro to turn on a given way of creating application. 
*/
#define MULTITHREADED_APP_WINDOW
#define CUSTOM_APP_WINDOW

#ifdef _DEBUG
/** @def turn on memory dump when the application exits. only for debug build.*/
// #define DETECT_MEMORY_LEAK
#endif

extern HINSTANCE g_hAppInstance;
//-----------------------------------------------------------------------------
// Name: WinMain()
/// Entry point to the program. Initializes everything, and goes into a
///       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
	g_hAppInstance = hInst;

#ifdef DETECT_MEMORY_LEAK
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// Added LXZ 2007.12.24
	// Uncomment to Set a Breakpoint on a Memory Allocation Number
	// More info at http://msdn2.microsoft.com/en-us/library/w2fhc9a3(vs.71).aspx
	// _CrtSetBreakAlloc(50147);
	
#endif

	CCommandLineParams cmdParams(lpCmdLine);
	if(strcmp("true", cmdParams.GetAppCommandLineByParam("single", "false")) == 0)
	{
		// only prevent multiple instance if single is true.
		HWND hWnd = ::FindWindowW(g_sWindowClassName,NULL);
		if(hWnd !=NULL)
		{
			// OUTPUT_LOG("warning: multiple instances of the same ParaWorld application is opened. \n");

			// There is already an instance running, bring it to top and quit.
			if(::SetForegroundWindow(hWnd) == 0)
			{
				OUTPUT_LOG("warning: bring last instance to top failed\n");
			}

			// send ID_GAME_COMMANDLINE message to the existing instance. We will write the current command line to a temp/cmdline.txt for reference.
			COPYDATASTRUCT MyCDS;
			MyCDS.dwData = ID_GAME_COMMANDLINE; // function identifier
			MyCDS.cbData = strnlen( lpCmdLine, 4096)+1; // size of data
			MyCDS.lpData = lpCmdLine;           // data structure
			
			::SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM) (LPVOID) &MyCDS);
			return 0;
		}
	}

	int exit_code = 0;
	if( strcmp("true", cmdParams.GetAppCommandLineByParam("servermode", "false")) == 0 || 
		strcmp("true", cmdParams.GetAppCommandLineByParam("i", "false")) == 0)
	{
		// run as a service
		RedirectIOToConsole();
		ParaEngine::CParaEngineService service;
		exit_code = service.Run(lpCmdLine);

	}
	else
	{
		InitCommonControls();
		// run as application
		CWindowsApplication app(lpCmdLine);
		// we use high resolution timer (boost ASIO internally), hence FPS can be specified very accurately without eating all CPUs. 
		app.SetRefreshTimer(1/60.f, 0);
		exit_code = app.Run(hInst);
	}
	return exit_code;
}
