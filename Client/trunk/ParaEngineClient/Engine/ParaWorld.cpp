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

#include "ParaEngineApp.h"
#include "ParaEngineService.h"
#include "ParaEngineCore.h"

#include "PluginAPI.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"

#include "memdebug.h"

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
		ParaEngine::CParaEngineService service;
		exit_code = service.Run(lpCmdLine);
	}
	else
	{
		InitCommonControls();

#ifdef MULTITHREADED_APP_WINDOW
		/**
		* This is a sample code for writing your own ParaEngine Application using the simple way. 
		* the simple way is to let the CParaEngineApp to create the main window(HWND) and manage message processing automatically. 
		*/
		class CMyApp : public CParaEngineApp
		{
		public:
			CMyApp(LPSTR lpCmdLine):CParaEngineApp(lpCmdLine){};
		};

		// run as application
		CMyApp d3dApp(lpCmdLine);

		// we use high resolution timer (boost ASIO internally), hence FPS can be specified very accurately without eating all CPUs. 
		d3dApp.SetRefreshTimer(1/60.f, 0);

		exit_code = d3dApp.Run(hInst);

#else // CUSTOM_APP_WINDOW
		/**
		* We will use interface only as if in a dll to create a window. 
		* the advanced way is to create a custom window and pass its HWND to CParaEngineApp, and in the MsgProc , call CParaEngineApp's message handler. 
		* I have marked all API calls in comment prefix like "API:". 
		*/
		ClassDescriptor* pClassDesc = ParaEngine_GetClassDesc(); // this can also be retrieved from plugin interface
		IParaEngineCore* pParaEngine = (IParaEngineCore*)(pClassDesc->Create());
		if(pParaEngine)
		{
			// API: Create App to get the IParaEngineApp interface. 
			IParaEngineApp* pParaEngineApp = pParaEngine->CreateApp();
			if(pParaEngineApp)
			{
				static IParaEngineApp* pParaEngineApp_ = pParaEngineApp;
				// This illustrate how to work with an external custom window created outside ParaEngineApp. 
				class ExternalCustomWindow_
				{
				public:
					static HWND Create(HINSTANCE hInstance, int x, int y, int nWidth, int nHeight)
					{
						static const WCHAR sWindowClassName[] = L"TestWindowName";
						HWND hWnd = NULL;
						// Register the windows class
						WNDCLASSW wndClass = { 0, WndProc, 0, 0, hInstance,
							NULL, // LoadIcon( hInstance, MAKEINTRESOURCE(IDI_PARAWORLD_ICON) ),
							LoadCursor( NULL, IDC_ARROW ),
							(HBRUSH)GetStockObject(WHITE_BRUSH),
							NULL, sWindowClassName };
						RegisterClassW( &wndClass );

						// Set the window's initial style
						DWORD dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE ;
						HMENU hMenu = NULL;
						// Set the window's initial width
						RECT rc;
						SetRect( &rc, x, y, nWidth, nHeight);        
						AdjustWindowRect( &rc, dwWindowStyle, ( hMenu != NULL ) ? true : false );

						// Create the render window
						hWnd = CreateWindowW( sWindowClassName, L"strWindowTitle", dwWindowStyle,
							CW_USEDEFAULT, CW_USEDEFAULT,
							(rc.right-rc.left), (rc.bottom-rc.top), 0,
							hMenu, hInstance, 0 );
						return hWnd;
					}

					static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
					{
						// Note: do any custom processing here
						if(pParaEngineApp_)
						{
							// API: send window message to app WndProc for processing, Please note default window procedure is called internally.
							return pParaEngineApp_->MsgProcWinThread(hWnd, uMsg, wParam, lParam);
						}
						else
							return DefWindowProcW( hWnd, uMsg, wParam, lParam );
					}
				};
				
				// Specify command line such as "bootstrapper=\"script/apps/Aries/bootstrapper.xml\"";
				const char* sCommandLine = lpCmdLine; 

				// API: start the application (windows and rendering devices are not yet created), need to call Create later
				if(pParaEngineApp->StartApp(sCommandLine) != S_OK)
				{
					OUTPUT_LOG("error: failed to start app\n");
					return 0;
				}

				// set refresh timer, uncomment to render as fast as it could (Eat all CPU)
				pParaEngineApp->SetRefreshTimer(0.0333f, 1);
				
				// Create a window to render to
				int nWidth, nHeight;
				pParaEngineApp->GetWindowCreationSize(&nWidth, &nHeight);
				HWND hWnd = ExternalCustomWindow_::Create(hInst, 0,0, nWidth, nHeight);

				// API: Assign the external hWnd to app
				pParaEngineApp->SetMainWindow(hWnd);
				// API: Now create window and render device
				if(pParaEngineApp->Create() == S_OK)
				{
					// Now we're ready to receive and process Windows messages.
					HRESULT hr;
					bool bGotMsg;
					MSG  msg;
					msg.message = WM_NULL;
					PeekMessageW( &msg, NULL, 0U, 0U, PM_NOREMOVE );

					while( WM_QUIT != msg.message  )
					{
						bool bAppActive=true;
						bool bDeviceLost = false;

						// Use PeekMessage() if the app is active, so we can use idle time to
						// render the scene. Else, use GetMessageW() to avoid eating CPU time.
						if( bAppActive )
							bGotMsg = ( PeekMessageW( &msg, NULL, 0U, 0U, PM_REMOVE ) != 0 );
						else
							bGotMsg = ( GetMessageW( &msg, NULL, 0U, 0U ) != 0 );

						if( bGotMsg )
						{
							if(pParaEngineApp->MsgProcWinThreadCustom(msg.message, msg.wParam, msg.lParam)==0)
							{
								TranslateMessage( &msg );
								DispatchMessageW( &msg );
							}
						}
						else
						{
							if( bDeviceLost )
							{
								// Yield some CPU time to other processes
								Sleep( 100 ); // 100 milliseconds
							}
							if( bAppActive )
							{
								// API: Render a frame during idle time (no messages are waiting)
								if( FAILED( hr = pParaEngineApp->Render3DEnvironment() ) )
								{
									SendMessage(hWnd, WM_QUIT,NULL, NULL);
								}
							}
						}
					} // while( WM_QUIT != msg.message  )
				}
				// API: Finally call this before process exit. 
				pParaEngineApp->StopApp();
				exit_code = pParaEngineApp->GetReturnCode();
			}
		}
#endif
	}
	return exit_code;
}
