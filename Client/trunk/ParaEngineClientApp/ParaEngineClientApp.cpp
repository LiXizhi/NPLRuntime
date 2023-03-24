//-----------------------------------------------------------------------------
// Class:	ParaEngineClient Application 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.3.2
// Desc: This exe must be run in the working directory of ParaEngineClient.dll
//-----------------------------------------------------------------------------
#include "config.h"
#include "AutoUpdaterClient.h"
#include "ParaEngineClientApp.h"
#include "InterprocessQueue.hpp"
#include "InterprocessAppClient.h"
#include "util/EnumProcess.hpp"
#include "CommandLineParser.hpp"

using namespace ParaEngine;

#pragma region CMyApp Header
class CMyApp;
static CMyApp* g_pMyApp = NULL; 
class CMyApp : public ParaEngine::CAutoUpdaterApp
{
public:
	CMyApp(HINSTANCE hInst=NULL)
		: CAutoUpdaterApp(), m_pParaEngine(NULL), m_pParaEngineApp(NULL), m_hInst(hInst), m_hWndUpdater(NULL){
		g_pMyApp = this;
	}
	~CMyApp(){
		if(m_pParaEngineApp){
			m_pParaEngineApp->DeleteInterface();
			m_pParaEngineApp = NULL;
		}
		g_pMyApp = NULL;
	}

	/** do auto update and display the progress */
	bool DoAutoUpdate(const char* lpCmdLine);

	/** load the ParaEngine plug in dll if not. 
	* @return true if loaded. 
	*/
	bool CheckLoad();

	/** run the application */
	int Run(HINSTANCE hInst,const char* lpCmdLine);

	/** window procedure for a very simple auto updater */
	static LRESULT CALLBACK AutoUpdaterWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	/** timer callback for auto updater */
	void AutoUpdaterTimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
	
public:
	virtual void OnProgress( const NPL::IAutoUpdaterClient::ProgressMessage& msg );
private:

	ParaEngine::CPluginLoader m_ParaEngine_plugin;
	ParaEngine::IParaEngineCore * m_pParaEngine;
	ParaEngine::IParaEngineApp * m_pParaEngineApp;
	HINSTANCE m_hInst;
	HWND m_hWndUpdater;
	std::wstring m_autoupdater_text;
};

#pragma endregion CMyApp Header

bool CMyApp::CheckLoad()
{
	if(m_ParaEngine_plugin.IsValid())
	{
		return true;
	}

#ifdef _DEBUG
	// post_fix with _d
	m_ParaEngine_plugin.Init("ParaEngineClient_d.dll"); 
#else
	m_ParaEngine_plugin.Init("ParaEngineClient.dll");
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

int CMyApp::Run( HINSTANCE hInst,const char* lpCmdLine )
{
	if(!CheckLoad())
		return E_FAIL;

	m_pParaEngineApp = m_pParaEngine->CreateApp();
	if(m_pParaEngineApp == 0)
		return E_FAIL;

	if(m_pParaEngineApp->StartApp(lpCmdLine) != S_OK)
		return E_FAIL;

	// Set Frame Rate
	//m_pParaEngineApp->SetRefreshTimer(1/45.f, 0);
	m_pParaEngineApp->SetRefreshTimer(1/30.f, 0);

	// Run to end
	return m_pParaEngineApp->Run(hInst);
}

#pragma region autoupdater app
LRESULT CALLBACK CMyApp::AutoUpdaterWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
	case WM_DESTROY:
		g_pMyApp->ReleaseAutoUpdater();
		PostMessage(NULL, PE_WM_QUIT,NULL, NULL);
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;RECT  rect;
			HDC hdc = BeginPaint (hWnd, &ps);
			GetClientRect (hWnd, &rect);
			//show the device found 
			
			DrawTextW(hdc, g_pMyApp->m_autoupdater_text.c_str(), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			EndPaint (hWnd, &ps) ;
		}
		return 0;
	default:
		break;
	}
	return DefWindowProcW( hWnd, uMsg, wParam, lParam );
}

void CMyApp::AutoUpdaterTimerProc( HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime )
{
	if(m_pAutoUpdater!=0 && !m_bAutoUpdateComplete)
	{
		// fetch only one message per frame to prevent too many messages to be processed per tick. 
		m_bAutoUpdateComplete = TryGetNextCoreUpdateMessage();
		if(m_bAutoUpdateComplete)
		{
			ReleaseAutoUpdater();
			PostMessage(m_hWndUpdater, WM_CLOSE,NULL, NULL);
		}
	}
	else
	{
		PostMessage(m_hWndUpdater, WM_CLOSE,NULL, NULL);
	}
}
void CALLBACK AutoUpdaterTimerProc_( HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime )
{
	if(g_pMyApp)
		g_pMyApp->AutoUpdaterTimerProc(hwnd, uMsg, idEvent, dwTime);
}

WCHAR* g_sAutoUpdaterWindowClassName = L"ParaEngineAutoUpdaterWnd";
WCHAR* g_sAutoUpdaterWindowTitle = L"正在更新,请稍候...";

/** now do auto update. */
bool CMyApp::DoAutoUpdate( const char* lpCmdLine )
{
	SetCommandLine(lpCmdLine);

	std::string app_dir = m_pCmdLineParams->GetValue("app_dir", "");
	if(!app_dir.empty())
	{
		if(SetAppDir()!=0)
		{
			::MessageBoxW(NULL, L"Sorry, we can not set the app working directory.", L"ParaEngine", MB_OK);
			return false;
		}
	}

	std::string no_update = m_pCmdLineParams->GetValue("noupdate", "");
	std::string updateurl = m_pCmdLineParams->GetValue("updateurl", "");
	if(updateurl.empty() || no_update == "true")
		return true;
	else
	{
		// update to "web" folder, even it is standalone app
		BeginCoreUpdate("", "web");
		
		// now let us create the window 
		WNDCLASSW wndClass = { 0, AutoUpdaterWndProc, 0, 0, m_hInst,
			NULL,
			NULL,
			(HBRUSH)GetStockObject(WHITE_BRUSH),
			NULL, g_sAutoUpdaterWindowClassName };
		RegisterClassW( &wndClass );

		HWND hWnd = CreateWindowExW( 0, g_sAutoUpdaterWindowClassName, g_sAutoUpdaterWindowTitle, 
			WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, 200, 60, NULL, 0 , m_hInst, 0 );
		m_hWndUpdater = hWnd;
		if(hWnd==0)
			return false;

		// Now we're ready to receive and process Windows messages.
		MSG  msg;
		::SetTimer(hWnd, 1001, 200, AutoUpdaterTimerProc_);
		while( GetMessageW( &msg, NULL, 0U, 0U ) != 0  && (PE_WM_QUIT != msg.message))
		{
			// Translate and dispatch the message
			TranslateMessage( &msg );
			DispatchMessageW( &msg );
		}
		return m_bAutoUpdateComplete;
	}
	return true;
}

void CMyApp::OnProgress( const NPL::IAutoUpdaterClient::ProgressMessage& msg )
{
	switch(msg.m_eventType)
	{
	case NPL::IAutoUpdaterClient::AUP_PROGRESS:
		{
			if(msg.m_allcount > 0)
			{
				WCHAR sText[256];
				swprintf(sText, L"已完成: %.2f%%",((float)msg.m_finishcount/(float)msg.m_allcount)*100.f);
				m_autoupdater_text = sText;
				InvalidateRect(m_hWndUpdater, NULL, TRUE);
				UpdateWindow(m_hWndUpdater);
			}
		}
		break;
	case NPL::IAutoUpdaterClient::AUP_UNKNOWN:
	case NPL::IAutoUpdaterClient::AUP_BROKENFILE:
	case NPL::IAutoUpdaterClient::AUP_ERROR:
		{
			m_autoupdater_text = L"更新遇到了问题, 请重新安装";
			InvalidateRect(m_hWndUpdater, NULL, TRUE);
			UpdateWindow(m_hWndUpdater);
		}
		break;
	/*case NPL::IAutoUpdaterClient::AUP_DO_APPLY_PATCH:
		{
			// TODO: Find a way to close the executable and then restart. 
			NPL::CAutoUpdaterClient* pUpdater = CreateGetAutoUpdater();
			if(pUpdater)
			{
				pUpdater->OnProgress(NPL::IAutoUpdaterClient::AUP_COMPLETED, "AutoUpdater: Updating finished\n");
			}
		}
		return;*/
	default:
		break;
	}
	CAutoUpdaterApp::OnProgress(msg);
}
#pragma endregion autoupdater app

/** create app process embedded in another process(window) */
int CreateHostApp(HINSTANCE hInst, const std::string& sAppCmdLine)
{
	if(sAppCmdLine.find("appid=") != std::string::npos)
	{
		// this process is started by a host process to create app and render window inside host's parent window. 
		// we should create interprocess loop here. 
		CInterprocessAppClient app("MyApp");
		app.SetCommandLine(sAppCmdLine.c_str());

		// if command line contains single="true", we will display a popup dialog to confirm with the user that multiple instances are not allowed. It will either kill previous one or exit.  
		bool bForceSingle = (sAppCmdLine.find("single=\"true\"") != std::string::npos || sAppCmdLine.find("single=true") != std::string::npos);
		// if command line contains asksingle="true", we will display a popup dialog to confirm with the user that multiple instances are not recommended. It will either kill previous one or continue
		bool bAskSingle = (sAppCmdLine.find("asksingle=\"true\"") != std::string::npos || sAppCmdLine.find("asksingle=true") != std::string::npos);

		std::string sProcessName = "ParaEngineClient.exe";
		CFindKillProcess   findKillProcess;
		DWORD   pid   =   findKillProcess.FindProcess(sProcessName.c_str());
		if(pid != 0)
		{
			// error code 1000 means multiple instance related. 
			app.SendError(1000, "ask_multiple_instance");
			if(bForceSingle || bAskSingle)
			{
				if(::MessageBoxW(NULL, L"发现你在运行多个游戏, 是否强制关闭之前的游戏", L"哈奇小镇-Web版", MB_YESNO) == IDYES)
				{
					app.SendError(1000, "force_close_start");
					if(!findKillProcess.FindAndKillProcess(sProcessName.c_str()))
					{
						app.SendError(1000, "force_close_failed");
						::MessageBoxW(NULL, L"抱歉, 无法关闭之前的游戏, 您可以尝试重启电脑或关掉所有浏览器并手工删除ParaEngineClient.exe进程", L"小错误", MB_OK);
						return -1;
					}
					else
					{
						app.SendError(1000, "force_close_succeed");
					}
				}
				else
				{
					if(bForceSingle)
					{
						app.SendError(1000, "please_manually_close");
						return -1;
					}
				}
			}
		}
		
		return app.Run(hInst, sAppCmdLine.c_str());
	}
	return -1;
}

/** create a standalone app */
int CreateStandAloneApp(HINSTANCE hInst, const std::string& sAppCmdLine)
{
	if(sAppCmdLine.find("single=\"true\"") != std::string::npos || sAppCmdLine.find("single=true") != std::string::npos 
		|| sAppCmdLine.find("single=%22true%22") != std::string::npos)
	{
		// only prevent multiple instance if single is true.
		HWND hWnd = ::FindWindow(_T("ParaWorld"),NULL);
		if(hWnd !=NULL)
		{
			// There is already an instance running, bring it to top and quit.
			if(::SetForegroundWindow(hWnd) == 0)
			{
				OutputDebugString(_T("warning: bring last instance to top failed\n"));
			}

			#define ID_GAME_COMMANDLINE             40061
			// send ID_GAME_COMMANDLINE message to the existing instance. We will write the current command line to a temp/cmdline.txt for reference.
			COPYDATASTRUCT MyCDS;
			MyCDS.dwData = ID_GAME_COMMANDLINE; // function identifier
			MyCDS.cbData = sAppCmdLine.size() + 1; // size of data
			MyCDS.lpData = (void*)(sAppCmdLine.c_str());           // data structure

			::SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM) (LPVOID) &MyCDS);
			return 0;
		}
	}

	CMyApp myApp(hInst);
	if(sAppCmdLine.find("updateurl=") != std::string::npos)
	{
		// we will do auto update if update url is provided. 
		if(!myApp.DoAutoUpdate(sAppCmdLine.c_str()))
		{
			return 0;
		}
	}
	return myApp.Run(0, sAppCmdLine.c_str());
}
//-----------------------------------------------------------------------------
// Name: WinMain()
/// Entry point to the program. Initializes everything, and goes into a
///       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
	std::string sAppCmdLine;
	if(lpCmdLine)
		sAppCmdLine = lpCmdLine;
	
	if(sAppCmdLine.find("appid=") != std::string::npos)
	{
		return CreateHostApp(hInst, sAppCmdLine);
	}
	else
	{
		return CreateStandAloneApp(hInst, sAppCmdLine);
	}
}
