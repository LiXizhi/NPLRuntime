//-----------------------------------------------------------------------------
// Class:	Interprocess Application client. 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.4.22
// Desc:
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#include "config.h"
#include "AutoUpdaterApp.h"
#include "ParaEngineClientApp.h"

#include <boost/bind.hpp>
#include "ParaUtils.hpp"
#include "InterprocessAppClient.h"
#include "CommandLineParser.hpp"
#include "util/EnumProcess.hpp"

using namespace ParaEngine;

#ifndef IDEAL_FRAME_RATE
#define IDEAL_FRAME_RATE (1/30.f)
#endif

#ifndef OUTPUT_LOG
#define OUTPUT_LOG OutputDebugString
#endif

// uncomment following lines to debug 
//#undef OUTPUT_LOG
//#define OUTPUT_LOG(x) ::MessageBoxA(NULL, x, "LOG", MB_OK)

WCHAR* g_sWindowClassName = L"ParaWorld";
WCHAR* g_sWindowTitle = L"ParaEngine Window";
IParaEngineApp* g_pApp = NULL;

CInterprocessAppClient * g_pIPCAppClient = NULL;
ParaEngine::CInterprocessAppClient::CInterprocessAppClient( const char* appName /*= NULL*/ )
:CAutoUpdaterApp(), m_hWndParent(NULL),m_bStarted(false), m_bAutoUpdateWhenStart(false), m_nMinWidth(0),m_nMinHeight(0), m_main_timer(m_main_io_service), m_pParaEngine(NULL), m_pParaEngineApp(NULL), m_bQuit(false), m_hWnd(NULL), m_ipAppQueueIn(NULL), m_ipHostQueueOut(NULL), m_hInst(0), m_dwWinThreadID(0), m_bMainLoopExited(true)
{
	g_pIPCAppClient = this;
	SetAppName(appName);
}

ParaEngine::CInterprocessAppClient::~CInterprocessAppClient()
{
	SAFE_DELETE(m_ipAppQueueIn);
	SAFE_DELETE(m_ipHostQueueOut);
	g_pIPCAppClient = NULL;
}

int ParaEngine::CInterprocessAppClient::HandleAppMsg( InterProcessMessage& msg )
{
	switch(msg.m_nMsgType)
	{
	case PEAPP_SetParentWindow:
		{
			SetParentWindow((HWND)(msg.m_nParam1));
			break;
		}
	case PEAPP_Start:
		{
			m_nMinWidth = msg.m_nParam1;
			m_nMinHeight = msg.m_nParam2;
			if(m_bAutoUpdateWhenStart)
			{
				BeginCoreUpdate("", "web");
			}
			else
			{
				Start(m_nMinWidth, m_nMinHeight);
			}
			break;
		}
	case PEAPP_Stop:
		{
			Stop();
			break;
		}
	case PEAPP_OnSizeChange:
		{
			if(m_hWnd)
			{
				if(msg.m_nParam1 > 0)
				{
					MoveWindow(m_hWnd, 0, 0, msg.m_nParam1, msg.m_nParam2, TRUE); 
				}
			}
			break;
		}
	case PEAPP_FocusIn:
		{
			if(m_pParaEngineApp)
			{
				// this usually occurs every time the user clicks the mouse on the plugin window. 
				// SendLog("PEAPP_FocusIn");

				// Note: Since WM_MOUSEACTIVATE will do the trick, so there is no need to call anything here PE_APP_SWITCH. 
				// m_pParaEngineApp->PostWinThreadMessage(PE_APP_SWITCH, 0, 1);
			}
			break;
		}
	case PEAPP_FocusOut:
		{
			break;
		}
	case PEAPP_SetWorkingDir:
		{
			/// Note: for security reasons, it is not advised to use this function. This may be removed in future release.
			SetCurrentDirectory(msg.m_code.c_str());
			break;
		}
	case PEAPP_BeginCoreUpdate:
		{
			BeginCoreUpdate("", msg.m_code.c_str());
			break;
		}
	case PEAPP_NPL_Activate:
		{
			if(m_pParaEngineApp)
			{
				NPL::INPLRuntime* pNPLRuntime = m_pParaEngineApp->GetNPLRuntime();
				if(pNPLRuntime!=0)
				{
					NPLInterface::CNPLWriter writer;
					writer.WriteName("msg");
					writer.BeginTable();
					
					writer.WriteName("filename");
					writer.WriteValue(msg.m_filename);

					writer.WriteName("type");
					writer.WriteValue(msg.m_nMsgType);

					writer.WriteName("code");
					writer.WriteValue(msg.m_code);

					writer.WriteName("param1");
					writer.WriteValue(msg.m_nParam1);

					writer.WriteName("param2");
					writer.WriteValue(msg.m_nParam2);

					writer.WriteName("from");
					writer.WriteValue(msg.m_from);
					
					writer.EndTable();
					pNPLRuntime->Activate(NULL, "script/ide/app_ipc.lua", writer.ToString().c_str(), (int)(writer.ToString().size()));
				}
			}
			break;
		}
	default:
		break;
	}
	return 0;
}

#pragma region Core Game Loop
void ParaEngine::CInterprocessAppClient::SetAppName( const char* appName )
{
	if(appName!=0 && m_app_name != appName)
	{
		m_app_name = appName;

		// create app queue to read any interprocess messages sent to this client process. 
		SAFE_DELETE(m_ipAppQueueIn);
		m_ipAppQueueIn = new CInterprocessQueue(m_app_name.c_str(), IPQU_open_or_create);
	}
}

int ParaEngine::CInterprocessAppClient::SetCommandLine(const char* sCmdLine)
{
	if(CAutoUpdaterApp::SetCommandLine(sCmdLine) >0)
	{
		// init IPC as well 
		SetAppName(m_pCmdLineParams->GetValue("appid", NULL));
		SAFE_DELETE(m_ipHostQueueOut);
		m_ipHostQueueOut = new CInterprocessQueue(m_pCmdLineParams->GetValue("apphost", NULL), IPQU_open_or_create);
		return 1;
	}
	return 0;
}
int ParaEngine::CInterprocessAppClient::Run( HINSTANCE hInst, const char* sCmdLine )
{
	SetCommandLine(sCmdLine);
	if(m_pCmdLineParams == 0)
		return -1;
	
	m_sUsage = m_pCmdLineParams->GetValue("usage", "");

	if(m_sUsage == "webplayer")
	{
		EnableAutoUpdateWhenComplete(true);
		SetAppDir();
	}

	m_hInst = hInst;

	// start the main loop timer. 
	m_main_timer.expires_from_now(std::chrono::seconds(0));
	m_main_timer.async_wait(boost::bind(&CInterprocessAppClient::handle_mainloop_timer, this, boost::asio::placeholders::error));

	// start the main loop now
	m_bMainLoopExited = false;
	m_main_io_service.run();
	m_bMainLoopExited = true;

	if(m_win_thread)
	{
		m_win_thread->join();
	}

	int nReturnCode = 0;
	if(m_pParaEngineApp)
	{
		m_pParaEngineApp->StopApp();
		nReturnCode = m_pParaEngineApp->GetReturnCode();
	}
	return nReturnCode;
}

void ParaEngine::CInterprocessAppClient::SetParentWindow( HWND hWndParent )
{
	m_hWndParent = hWndParent;
}
int ParaEngine::CInterprocessAppClient::Start(int nMinResolutionWidth, int nMinResolutionHeight)
{
	if(m_bStarted || m_bQuit)
	{
		// can not call start twice.
		return E_FAIL;
	}
	m_bStarted = true;

	SendHostMsg(PEAPP_LoadingProgress_LoadingLibs);
	if(!CheckLoad())
		return E_FAIL;

	SendHostMsg(PEAPP_LoadingProgress_CreatingApp);
	m_pParaEngineApp = m_pParaEngine->CreateApp();
	if(m_pParaEngineApp == 0)
		return E_FAIL;

	SendHostMsg(PEAPP_LoadingProgress_StartingApp);
	if(m_pParaEngineApp->StartApp(m_sCmdLine.c_str()) != S_OK)
		return E_FAIL;

	// Set Frame Rate
	//m_pParaEngineApp->SetRefreshTimer(1/45.f, 0);
	m_pParaEngineApp->SetRefreshTimer(1/30.f, 0);
	
	// load some default settings
	m_pParaEngineApp->SetIgnoreWindowSizeChange(false);
	m_pParaEngineApp->SetCoreUsage(PE_USAGE_WEB_BROWSER);
	if(nMinResolutionWidth != 0 || nMinResolutionHeight != 0)
	{
		m_pParaEngineApp->SetMinUIResolution(nMinResolutionWidth, nMinResolutionHeight, false);
	}

	// create the child window
	StartWindowThread();

	return 0;
}

int ParaEngine::CInterprocessAppClient::Stop()
{
	m_bQuit = true;
	if(m_dwWinThreadID != 0)
	{
		// this simulate the WM_QUIT to let the window thread to quit. 
		PostThreadMessage(m_dwWinThreadID,WM_QUIT, 0,0);
	}
	return true;
}

bool ParaEngine::CInterprocessAppClient::CheckLoad()
{
	if(m_ParaEngine_plugin.IsValid())
	{
		return true;
	}
#ifdef _DEBUG
	// post_fix with _d
	m_ParaEngine_plugin.Init("AwesomeTruck_d.dll"); 
#else
	m_ParaEngine_plugin.Init("AwesomeTruck.dll");
#endif

	int nClassCount = m_ParaEngine_plugin.GetNumberOfClasses();
	for (int i=0; i<nClassCount; ++i)
	{
		ClassDescriptor* pDesc = m_ParaEngine_plugin.GetClassDescriptor(i);
		if(pDesc)
		{
			if(strcmp(pDesc->ClassName(), "ParaEngine") == 0)
			{
				m_pParaEngine = (ParaEngine::IParaEngineCore*)(pDesc->Create());
			}
		}
	}
	return m_ParaEngine_plugin.IsValid();
}

double ParaEngine::CInterprocessAppClient::GetElapsedAppTime()
{
	static int64 start_time = CParaUtils::GetTimeMS();
	double elapsedTime = (double)(CParaUtils::GetTimeMS() - start_time);
	return elapsedTime / 1000;
}

void ParaEngine::CInterprocessAppClient::handle_mainloop_timer( const boost::system::error_code& err )
{
	if (!err && !m_bQuit)
	{
		ParaEngine::Mutex::ScopedLock lock_(m_mutex);
		double fIdealInterval = 0;

		if(m_pParaEngineApp)
		{
			PEAppState dwState = m_pParaEngineApp->GetAppState();
			if(dwState == PEAppState_Ready)
			{
				// the frame move and render the scene. 
				m_pParaEngineApp->Render3DEnvironment(true);
			}
			else if(dwState == PEAppState_Device_Error)
			{
				return;
			}
			else if(m_hWnd)
			{
				if(dwState == PEAppState_None)
				{
					SendHostMsg(PEAPP_LoadingProgress_CreatingDevice);
					// create the D3D if not created yet. 
					m_pParaEngineApp->SetMainWindow(m_hWnd, true);
					m_pParaEngineApp->Create();
					SendHostMsg(PEAPP_LoadingProgress_GameLoopStarted);
				}
			}

			fIdealInterval = (m_pParaEngineApp->GetRefreshTimer() <= 0) ? IDEAL_FRAME_RATE : m_pParaEngineApp->GetRefreshTimer();
		}
		else
		{
			fIdealInterval = IDEAL_FRAME_RATE;

			if(!m_bAutoUpdateComplete && m_pAutoUpdater!=0)
			{
				// fetch only one message per frame to prevent too many messages to be processed per tick. 
				m_bAutoUpdateComplete = TryGetNextCoreUpdateMessage();
				if(m_bAutoUpdateComplete)
					ReleaseAutoUpdater();
			}
			else if(m_bAutoUpdateWhenStart && m_bAutoUpdateComplete)
			{
				ReleaseAutoUpdater();
				Start(m_nMinWidth, m_nMinHeight);
			}
		}
		
		{
			// read from interprocess queue for any app related messages. 
			const double player_runtime_interval = 0.03f;
			static double s_player_runtime_elapsed = 0.f;
			s_player_runtime_elapsed += fIdealInterval;
			if(s_player_runtime_elapsed > player_runtime_interval && m_ipAppQueueIn)
			{
				InterProcessMessage msg_in;
				unsigned int nPriority = 0;
				int nResult = 0;
				int hr = 0;
				while((hr = m_ipAppQueueIn->try_receive(msg_in, nPriority)) == IPRC_OK)
				{
					nResult = HandleAppMsg(msg_in);
					if(nResult != 0)
					{
						m_bQuit = true;
					}
				}
			}
		}

		// continue with next activation. 
		double fCurTime = GetElapsedAppTime();
		double fNextInterval = 0.f;

		const bool USE_ADAPTIVE_INTERVAL = true;
		if(USE_ADAPTIVE_INTERVAL)
		{
			// --------adaptive interval algorithm
			// check FPS by modifying the interval adaptively until FPS is within a good range.
			// we will adjust every 1 second
			static int nFPS = 0;
			static double fLastTime = fCurTime;
			static double fSeconds = fCurTime;
			static double fAdaptiveInterval = 0.005f; // initial value for tying
			static double fLastIdealInterval = 1/30.0;

			if(fLastIdealInterval != fIdealInterval)
			{
				if(fLastIdealInterval > fIdealInterval)
					fAdaptiveInterval = fIdealInterval;
				fLastIdealInterval = fIdealInterval;
			}

			nFPS ++;
			if((fSeconds + 1)<fCurTime)
			{
				double fDelta = nFPS*fIdealInterval;
				if(fDelta > 1.5)
				{
					fAdaptiveInterval = fAdaptiveInterval + 0.002f;
				}
				else if(fDelta > 1.3)
				{
					fAdaptiveInterval = fAdaptiveInterval + 0.001f;
				}
				else if(nFPS*fIdealInterval < 1)
				{
					fAdaptiveInterval = fAdaptiveInterval - 0.001f;
					if(fAdaptiveInterval < 0)
					{
						fAdaptiveInterval = 0.f;
					}
				}
				fSeconds += 1;
				nFPS = 0;
			}
			fNextInterval = fAdaptiveInterval;
			fLastTime = fCurTime;
		}
		else
		{
			// --------fixed interval algorithm
			// continue with next activation. 
			static double s_next_time = 0;
			fNextInterval = s_next_time - fCurTime;
			if(fNextInterval <= 0)
			{
				s_next_time = fCurTime;
				fNextInterval = 0;
			}
			else if(fNextInterval >= fIdealInterval)
			{
				fNextInterval = fIdealInterval;
				s_next_time = fCurTime;
			}
			s_next_time = s_next_time + fIdealInterval;
		}

		m_main_timer.expires_from_now(std::chrono::milliseconds((int)(fNextInterval*1000)));
		m_main_timer.async_wait(boost::bind(&CInterprocessAppClient::handle_mainloop_timer, this, boost::asio::placeholders::error));
	}
}

// render window win thread windows proc
LRESULT CALLBACK ParaEngine::CInterprocessAppClient::DefaultWinThreadWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if(g_pApp)
	{
		switch(uMsg)
		{
		case  WM_SETFOCUS:
			// inject WM_ACTIVATEAPP to simulate activate APP
			g_pApp->MsgProcWinThread(hWnd, WM_ACTIVATEAPP, (WPARAM)TRUE, 0, false);
			return 0;
			break;
		case  WM_KILLFOCUS:
			{
				HWND hNewFocusWnd = (HWND)wParam;
				bool bDoesChildHasFocus = false;
				if(hNewFocusWnd == NULL)
				{
					// try parent and parent's parent
					for(int i=0;i<3 && hNewFocusWnd != NULL; i++)
					{
						if(hNewFocusWnd != hWnd)
						{
							hNewFocusWnd = GetParent(hNewFocusWnd);
						}
						else
						{
							bDoesChildHasFocus = true;
							break;
						}
					}
				}
				
				if(!bDoesChildHasFocus)
				{
					// inject WM_ACTIVATEAPP to simulate activate APP
					g_pApp->MsgProcWinThread(hWnd, WM_ACTIVATEAPP, (WPARAM)FALSE, 0, false);
				}
				return 0;
				break;
			}
		case WM_MOUSEWHEEL:
			{
				// intercept the mouse wheel, and prevent it to be propagated to parent window by returning 0 and do not call default proc. 
				g_pApp->MsgProcWinThread(hWnd, uMsg, wParam, lParam, false);
				return 0;
			}
		case WM_CLOSE:
			g_pApp->MsgProcWinThread(hWnd, uMsg, wParam, lParam, false);
			return 0;
		case WM_DESTROY:
		case WM_NCDESTROY:
			// in case the window is destroyed, we shall exit the application as well.
			if(g_pIPCAppClient){
				g_pIPCAppClient->Stop();
			}
			break;
		}
		return g_pApp->MsgProcWinThread(hWnd, uMsg, wParam, lParam);
	}
	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

HRESULT ParaEngine::CInterprocessAppClient::StartWindowThread()
{
	if( m_hWndParent == NULL)
	{
		OUTPUT_LOG("warning: parent window handle is not set\n");
		return E_FAIL;
	}
	// start the io thread
	if(!m_win_thread)
	{
		m_win_thread.reset(new boost::thread(boost::bind(&CInterprocessAppClient::DefaultWinThreadProc, this)));
	}
	return 0;
}

void ParaEngine::CInterprocessAppClient::DefaultWinThreadProc()
{
	if(m_hWndParent == 0 && m_pParaEngineApp){
		SendLog("No Parent window specified.");
		return;
	}

	//
	// create a child window to render into. Register the windows class
	// 
	{
		ParaEngine::Mutex::ScopedLock lock_(m_win_thread_mutex);

		SendLog("Default window thread started. and a window is created in it");

		WNDCLASSW wndClass = { 0, DefaultWinThreadWndProc, 0, 0, m_hInst,
			NULL,
			LoadCursor( NULL, IDC_ARROW ),
			(HBRUSH)GetStockObject(WHITE_BRUSH),
			NULL, g_sWindowClassName };
		RegisterClassW( &wndClass );

		// Set the window's initial width
		RECT rcClient; 
		GetClientRect(m_hWndParent, &rcClient);
		int nWidth = (rcClient.right - rcClient.left);
		int nHeight = (rcClient.bottom - rcClient.top);

		// In safari browser, the parent window size is (0,0) at this time, so we simply use the min size. 
		if(nWidth < 1 || nHeight <1)
		{
			// just ensure window size is positive, otherwise d3d will throw internal error. 
			nWidth = max(nWidth,1); nWidth = max(m_nMinWidth, nWidth);
			nHeight = max(nHeight,1); nHeight = max(m_nMinHeight, nHeight);
		}

		// Create the render window
		// Note By LiXizhi 2010.11.16: WS_EX_NOPARENTNOTIFY will prevent WM_PARENTNOTIFY to be sent to parent window whenever use clicks mouses, etc.
		HWND hWnd = CreateWindowExW( WS_EX_NOPARENTNOTIFY, g_sWindowClassName, g_sWindowTitle, WS_OVERLAPPED,
			0, 0, nWidth, nHeight, m_hWndParent, 0 , m_hInst, 0 );
		if(hWnd!=0)
		{
			/** LiXizhi 2010.4.25: this is extremely tricky. We first create an invisible window using WS_OVERLAPPED style. 
			This will trick the win32 to create IME context for the current thread. Then we will re-parent the window and remove the WS_OVERLAPPED style. 
			And by resizing the window twice afterwards, we will force the win32 window to remove the window title bar. 
			If we do not do this, and create the window using WS_CHILD | WS_VISIBLE style directly, IME messages will not be received by the child window. 
			*/
			SetParent(hWnd,m_hWndParent);
			SetWindowLong(hWnd, GWL_STYLE, WS_CHILD | WS_VISIBLE);
			MoveWindow(hWnd, 0, 0, nWidth+1, nHeight, TRUE);
			MoveWindow(hWnd, 0, 0, nWidth, nHeight, TRUE);

			g_pApp = m_pParaEngineApp;
			m_hWnd = hWnd;
		}
		else
		{
			SendLog("Failed to create main 3d window");
		}
	}

	m_dwWinThreadID = GetCurrentThreadId();

	//
	// Dispatching window messages in this window thread. 
	//

	// Now we're ready to receive and process Windows messages.
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessageW( &msg, NULL, 0U, 0U, PM_NOREMOVE );

	while( WM_QUIT != msg.message  )
	{
		if( GetMessageW( &msg, NULL, 0U, 0U ) != 0 )
		{
			// Translate and dispatch the message
			if( (m_hWnd == NULL) ||
				// process custom messages. standalone app must always call it in order for the game engine to act properly
				(m_pParaEngineApp->MsgProcWinThreadCustom(msg.message, msg.wParam, msg.lParam)==0) )
			{
				TranslateMessage( &msg );
				DispatchMessageW( &msg );
			}
		}
	}
	m_dwWinThreadID = 0;

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
	// if(::MessageBoxW(NULL, L"点击确定强制关闭程序", L"请确认", MB_OK) == IDOK)
	{
		CFindKillProcess   findKillProcess;
		DWORD dwCurrentPID = GetCurrentProcessId();
		findKillProcess.KillProcess(dwCurrentPID,true);
	}
}

bool ParaEngine::CInterprocessAppClient::SendHostMsg( int nMsg, DWORD param1 /*= 0*/, DWORD param2 /*= 0*/, const char* filename /*= NULL*/, const char* sCode /*= NULL*/, int nPriority /*= 0*/ )
{
	if(m_ipHostQueueOut)
	{
		InterProcessMessage msg;
		msg.m_method = "app";
		msg.m_nMsgType = nMsg;
		msg.m_nParam1 = param1;
		msg.m_nParam2 = param2;
		msg.m_from = m_ipHostQueueOut->GetName();
		if(filename)
		{
			msg.m_filename = filename;
		}
		if(sCode)
		{
			msg.m_code = sCode;
		}
		return m_ipHostQueueOut->try_send(msg, nPriority) == IPRC_OK;
	}
	return false;
}

void ParaEngine::CInterprocessAppClient::EnableAutoUpdateWhenComplete( bool bEnable )
{
	m_bAutoUpdateWhenStart = bEnable;
	m_bAutoUpdateComplete = !m_bAutoUpdateWhenStart;
}

#pragma endregion Core Game Loop


