//-----------------------------------------------------------------------------
// Class:	Default d3d window that is used with ParaEngineApp(d3dapp) if no external window is specified. 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.2.21
// Desc: The window is created in a different thread. All window messages for entire application are also dispatched from that thread. 
// It uses boost::asio deadline timer for refresh timer in the main thread. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "resource.h"
#include "ParaEngineApp.h"

#include "D3DWindowDefault.h"
#include "util/EnumProcess.hpp"
#include "Winuser.h"
#include <boost/bind.hpp>


using namespace ParaEngine;

/// if defined, menu will be shown
// #define SHOW_DEFAULT_MENU

WCHAR* g_sWindowClassName = L"ParaWorld";
WCHAR* g_sWindowTitle = L"ParaEngine Window";

IParaEngineApp* g_pApp = NULL;
CD3DWindowDefault::CD3DWindowDefault()
: m_main_timer(m_main_io_service), m_bQuit(false), m_pApp(NULL), m_bMainLoopExited(true)
{
	m_hWnd = NULL;
}

// render window win thread windows proc
LRESULT CALLBACK DefaultWinThreadWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if(g_pApp)
	{
		if(uMsg == WM_DESTROY)
		{
			g_pApp->PostWinThreadMessage(PE_WM_QUIT, 0, 0);
		}
		return g_pApp->MsgProcWinThread(hWnd, uMsg, wParam, lParam);
	}
	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

void CD3DWindowDefault::DefaultWinThreadProc(HINSTANCE hInstance)
{
	//
	// create a window to render into. Register the windows class
	// 
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);

		OUTPUT_LOG("Default window thread started. and a window is created in it\n");

		WNDCLASSW wndClass = { 0, DefaultWinThreadWndProc, 0, 0, hInstance,
			NULL, // LoadIcon( hInstance, MAKEINTRESOURCE(IDI_PARAWORLD_ICON) ),
			//LoadIcon( hInstance, MAKEINTRESOURCE(IDI_KIDSMOVIE_ICON) ),
			//LoadIcon( hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON) ),
			LoadCursor( NULL, IDC_ARROW ),//LoadCursorFromFileA( "cursor.cur" ),//remember to ask artist to make the this cursor file the same as "cursor.png"
			(HBRUSH)GetStockObject(WHITE_BRUSH),
			NULL, g_sWindowClassName };
		RegisterClassW( &wndClass );

		// Set the window's initial style
		DWORD dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | 
			WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE ;
	#ifdef SHOW_DEFAULT_MENU
		HMENU hMenu = LoadMenuW( hInstance, MAKEINTRESOURCEW(IDR_MENU) );
	#else
		HMENU hMenu = NULL;
	#endif
		// Set the window's initial width
		RECT rc;
		int nWidth, nHeight;
		m_pApp->GetWindowCreationSize(&nWidth, &nHeight);
		SetRect( &rc, 0, 0, nWidth, nHeight );        
		AdjustWindowRect( &rc, dwWindowStyle, ( hMenu != NULL ) ? true : false );

		// Create the render window
		m_hWnd = CreateWindowW( g_sWindowClassName, g_sWindowTitle, dwWindowStyle,
			CW_USEDEFAULT, CW_USEDEFAULT,
			(rc.right-rc.left), (rc.bottom-rc.top), 0,
			hMenu, hInstance, 0 );

		g_pApp = m_pApp;
	}
	
	//
	// Dispatching window messages in this window thread. 
	//

	// Load keyboard accelerators
	HACCEL hAccel = LoadAcceleratorsW( hInstance, MAKEINTRESOURCEW(IDR_MAIN_ACCEL) );

	// Now we're ready to receive and process Windows messages.
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessageW( &msg, NULL, 0U, 0U, PM_NOREMOVE );

	while( WM_QUIT != msg.message  )
	{
		if( GetMessageW( &msg, NULL, 0U, 0U ) != 0 )
		{
			// Translate and dispatch the message
			if( (hAccel == NULL) || (m_hWnd == NULL) ||
				// process custom messages. standalone app must always call it in order for the game engine to act properly
				((m_pApp->MsgProcWinThreadCustom(msg.message, msg.wParam, msg.lParam)==0) &&
				(TranslateAcceleratorW( m_hWnd, hAccel, &msg ) == 0)))
			{
				TranslateMessage( &msg );
				DispatchMessageW( &msg );
			}
		}
	}
	if( hAccel != NULL )
		DestroyAcceleratorTable( hAccel );

	OUTPUT_LOG("Default window thread exit\n");
	// inform to quit
	m_bQuit = true;

	// wait for 10 seconds for main loop to exit, if not display a pop up to ask the user to terminate
	const int nWaitSeconds = 10;
	for( int i=0;i<nWaitSeconds*10; i++)
	{
		if(m_bMainLoopExited)
		{
			// this is the normal exit
			return;
		}
		::Sleep(100);
	}
	// it seems that the main loop is blocked somewhere, this can be blocking request such as curl or IPC.
	// if(::MessageBoxW(NULL, L"Do you want to close the app", L"Non-responding app", MB_OK) == IDOK)
	{
		OUTPUT_LOG("warning: we will kill this process, since 10 seconds have passed since the last WM_DESTROY message and the main thread still does not quit.\n");
		CFindKillProcess   findKillProcess;
		DWORD dwCurrentPID = GetCurrentProcessId();
		findKillProcess.KillProcess(dwCurrentPID,true);
	}
}

HRESULT CD3DWindowDefault::StartWindowThread(HINSTANCE hInstance)
{
	if( m_hWnd != NULL)
	{
		OUTPUT_LOG("warning: StartWindowThread will manage its own window. but a hWnd is already assigned. \n");
		return E_FAIL;
	}
	// start the io thread
	m_win_thread.reset(new boost::thread(boost::bind(&CD3DWindowDefault::DefaultWinThreadProc, this, hInstance)));
	return S_OK;
}



int ParaEngine::CD3DWindowDefault::CalculateRenderTime(double fIdealInterval, double* pNextInterval)
{
	double fCurTime = DXUtil_Timer(TIMER_GETAPPTIME);
	double fNextInterval = 0.f;
	int nUpdateFrameDelta = 0;
	int nFrameDelta = 1;

	const bool USE_ADAPTIVE_INTERVAL = true;
	if (USE_ADAPTIVE_INTERVAL)
	{
		// --------adaptive interval algorithm
		// check FPS by modifying the interval adaptively until FPS is within a good range.
		// we will adjust every 1 second
		static double fLastTime = fCurTime;
		static double fLastFrameTime = fCurTime;
		static double fSeconds = fCurTime;

		static double fAdaptiveInterval = 1 / 60.0; // initial value for tying
		static double fLastIdealInterval = 1 / 30.0;
		// static double fLastDrawTime = fCurTime;

		if (fLastIdealInterval != fIdealInterval)
		{
			if (fLastIdealInterval > fIdealInterval)
				fAdaptiveInterval = fIdealInterval;
			fLastIdealInterval = fIdealInterval;
		}

		static int nFPS = 0;
		nFPS++;

		if ((fSeconds + 1)<fCurTime)
		{
			// adaptive delta
			double fDelta = nFPS*fIdealInterval;

			if (fDelta > 1.5)
			{
				fAdaptiveInterval = fAdaptiveInterval + 0.002f;
			}
			else if (fDelta > 1.3)
			{
				fAdaptiveInterval = fAdaptiveInterval + 0.001f;
			}
			else if (nFPS*fIdealInterval < 1)
			{
				fAdaptiveInterval = fAdaptiveInterval - 0.001f;
				if (fAdaptiveInterval < 0)
				{
					fAdaptiveInterval = 0.f;
				}
			}
			fSeconds = fCurTime;
			nFPS = 0;
		}
		/** tricky: run the main_loop as fast as possible at least 100FPS, so that FPS is more accurate. */
		fAdaptiveInterval = Math::Min(fAdaptiveInterval, 1 / 100.0);

		fNextInterval = fAdaptiveInterval;

		if ((fCurTime - fLastTime) > 1.f)
		{
			// too laggy
			nFrameDelta = 2;
			fLastFrameTime = fCurTime;
			fLastTime = fCurTime;
		}
		else
		{
			nFrameDelta = (int)((fCurTime - fLastFrameTime) / fIdealInterval + 0.5);

			fLastFrameTime = fLastFrameTime + (nFrameDelta * fIdealInterval);
			fLastTime = fCurTime;
		}
	}
	else
	{
		// --------fixed interval algorithm
		// continue with next activation. 
		static double s_next_time = 0;
		fNextInterval = s_next_time - fCurTime;
		if (fNextInterval <= 0)
		{
			s_next_time = fCurTime;
			fNextInterval = 0;
		}
		else if (fNextInterval >= fIdealInterval)
		{
			fNextInterval = fIdealInterval;
			s_next_time = fCurTime;
		}
		s_next_time = s_next_time + fIdealInterval;
	}

	/** define to output interval to log file to change timer implementation. */
	// #define DEBUG_TIMER_INTERVAL
#ifdef DEBUG_TIMER_INTERVAL
	{
		// debug timer
		static double fLastTime = fCurTime;
		static double fSeconds = fCurTime;
		static int nFPS = 0;
		nFPS++;
		if ((fSeconds + 1) < fCurTime)
		{
			fSeconds = fCurTime;
			OUTPUT_LOG("%d Second(FPS: %d)--------------\n", (int)(fSeconds), nFPS);
			nFPS = 0;
		}

		OUTPUT_LOG("Tick: delta:%d, global:%d, next:%d\n", (int)((fCurTime - fLastTime) * 1000),
			(int)(fCurTime * 1000), (int)(fNextInterval * 1000));
		fLastTime = fCurTime;
	}
#endif	
	if (pNextInterval)
		*pNextInterval = fNextInterval;
	return nFrameDelta;
}

void CD3DWindowDefault::handle_mainloop_timer(const boost::system::error_code& err)
{
	if (!err && !m_bQuit)
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);

		PEAppState dwState = m_pApp->GetAppState();
		if (dwState == PEAppState_Ready)
		{
		}
		else if(dwState == PEAppState_Device_Error /*|| dwState == PEAppState_Exiting*/)
		{
			return;
		}
		else if(m_hWnd)
		{
			if(dwState == PEAppState_None)
			{
				// create the D3D if not created yet. 
				m_pApp->SetMainWindow(m_hWnd, false); // since we are not creating external window, we will pass false. 
				m_pApp->Create();
			}
		}

		double fIdealInterval = (m_pApp->GetRefreshTimer() <= 0) ? IDEAL_FRAME_RATE : m_pApp->GetRefreshTimer();
		double fNextInterval = 0.f;
		int nFrameDelta = CalculateRenderTime(fIdealInterval, &fNextInterval);

		if (nFrameDelta > 0)
		{
			if (dwState == PEAppState_Ready)
			{
				// the frame move and render the scene. 
				m_pApp->Render3DEnvironment(false);
			}
		}
			
		m_main_timer.expires_from_now(std::chrono::milliseconds((int)(fNextInterval*1000)));
		m_main_timer.async_wait(boost::bind(&CD3DWindowDefault::handle_mainloop_timer, this, boost::asio::placeholders::error));
	}
}

int CD3DWindowDefault::Run(HINSTANCE hInstance)
{
	if(m_pApp == 0)
	{
		OUTPUT_LOG("error: application interface must be assigned before creating windows. ");
		return E_FAIL;
	}

	if(hInstance == 0)
	{
		hInstance = m_pApp->GetModuleHandle();
	}

	using namespace boost::asio;

	// create the default window in another thread. 
	StartWindowThread(hInstance);
	
	// start the main loop timer. 
	m_main_timer.expires_from_now(std::chrono::seconds(0));
	m_main_timer.async_wait(boost::bind(&CD3DWindowDefault::handle_mainloop_timer, this, boost::asio::placeholders::error));
	
	// start the main loop now
	m_bMainLoopExited = false;
	m_main_io_service.run();
	m_bMainLoopExited = true;

	m_win_thread->join();

	return m_pApp->GetReturnCode();
}