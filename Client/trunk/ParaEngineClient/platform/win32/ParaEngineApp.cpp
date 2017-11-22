//-----------------------------------------------------------------------------
// Class:	CParaEngine
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8
// Desc: Just for mobile version (android, iOS, cocos2dx) used by AppDelegate
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <mmsystem.h>
#include <shellapi.h>
#include <WinVer.h>

#pragma region PE Includes
#include "resource.h"
#include <objbase.h>
#include <gdiplus.h>


#include "platform/OpenGLWrapper.h"



#include "AudioEngine2.h"
#include "ic/ICDBManager.h"
#include "ic/ICConfigManager.h"
#include "AttributesManager.h"
#include "FrameRateController.h"
#include "ParaWorldAsset.h"
#include "PluginManager.h"
#include "PluginAPI.h"
#include "EventsCenter.h"
#include "FileManager.h"
#include "NPLRuntime.h"
#include "AISimulator.h"
#include "AsyncLoader.h"
#include "AutoCamera.h"
#include "ParaEngineSettings.h"
#include "2dengine/GUIHighlight.h"
#include "2dengine/GUIDirectInput.h"
#include "2dengine/GUIRoot.h"
#include "ViewportManager.h"
#include "ObjectManager.h"
#include "IEnvironmentSim.h"
#include "SceneObject.h"
#include "BootStrapper.h"
#include "ParaEngineService.h"
#include "guicon.h"
#include "FileLogger.h"
#include "OSWindows.h"
#include "OceanManager.h"
#include "DynamicAttributeField.h"

#include "MoviePlatform.h"

// header include
#include "ParaEngineApp.h"
#include "platform/win32/ParaEngineGLView.h"
#include <time.h>
#include <boost/bind.hpp>



using namespace std;
namespace ParaEngine {


	//-----------------------------------------------------------------------------
	// Name: DXUtil_Timer()
	// Desc: Performs timer opertations. Use the following commands:
	//          TIMER_RESET           - to reset the timer
	//          TIMER_START           - to start the timer
	//          TIMER_STOP            - to stop (or pause) the timer
	//          TIMER_ADVANCE         - to advance the timer by 0.1 seconds
	//          TIMER_GETABSOLUTETIME - to get the absolute system time
	//          TIMER_GETAPPTIME      - to get the current time
	//          TIMER_GETELAPSEDTIME  - to get the time that elapsed between 
	//                                  TIMER_GETELAPSEDTIME calls
	//-----------------------------------------------------------------------------
	enum TIMER_COMMAND {
		TIMER_RESET, TIMER_START, TIMER_STOP, TIMER_ADVANCE,
		TIMER_GETABSOLUTETIME, TIMER_GETAPPTIME, TIMER_GETELAPSEDTIME
	};

	/**
	* LiXizhi 2009.1.15: this fixed a problem of timeGetTime or QueryPerformanceCount() hardware defects. see: http://support.microsoft.com/kb/274323
	*/
	// define this if u want it. 
#define USE_QueryPerformanceFrequency

	//-----------------------------------------------------------------------------
	// Name: DXUtil_Timer()
	// Desc: Performs timer opertations. Use the following commands:
	//          TIMER_RESET           - to reset the timer
	//          TIMER_START           - to start the timer
	//          TIMER_STOP            - to stop (or pause) the timer
	//          TIMER_ADVANCE         - to advance the timer by 0.1 seconds
	//          TIMER_GETABSOLUTETIME - to get the absolute system time
	//          TIMER_GETAPPTIME      - to get the current time
	//          TIMER_GETELAPSEDTIME  - to get the time that elapsed between 
	//                                  TIMER_GETELAPSEDTIME calls
	//-----------------------------------------------------------------------------
	static double __stdcall DXUtil_Timer(TIMER_COMMAND command)
	{
		static BOOL     m_bTimerInitialized = FALSE;
		static BOOL     m_bUsingQPF = FALSE;
		static BOOL     m_bTimerStopped = TRUE;
		static LONGLONG m_llQPFTicksPerSec = 0;
		static LARGE_INTEGER liFrequency;

		static LONGLONG mLastTime;
		static LARGE_INTEGER mStartTime;
		static DWORD mStartTick;

		int i = 0;

#ifdef USE_QueryPerformanceFrequency
		// Initialize the timer
		if (FALSE == m_bTimerInitialized)
		{
			m_bTimerInitialized = TRUE;

			// Use QueryPerformanceFrequency() to get frequency of timer.  If QPF is
			// not supported, we will timeGetTime() which returns milliseconds.
			m_bUsingQPF = QueryPerformanceFrequency(&liFrequency);
			if (m_bUsingQPF)
				m_llQPFTicksPerSec = liFrequency.QuadPart;

			// remember the first tick. 
			QueryPerformanceCounter(&mStartTime);
			mStartTick = GetTickCount();
			mLastTime = mStartTime.QuadPart;
		}
#endif

		if (m_bUsingQPF)
		{
			static LONGLONG m_llStopTime = 0;
			static LONGLONG m_llLastElapsedTime = 0;
			static LONGLONG m_llBaseTime = 0;
			double fTime;
			double fElapsedTime;
			LARGE_INTEGER liCurrent;

			// Get either the current time or the stop time, depending
			// on whether we're stopped and what command was sent
			if (m_llStopTime != 0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
				liCurrent.QuadPart = m_llStopTime;
			else
			{
				// this is a fix of http://support.microsoft.com/kb/274323
				QueryPerformanceCounter(&liCurrent);

				LONGLONG newTime = liCurrent.QuadPart - mStartTime.QuadPart;
				// scale by 1000 for milliseconds
				unsigned long newTicks = (unsigned long)(1000 * newTime / liFrequency.QuadPart);

				// detect and compensate for performance counter leaps
				// (surprisingly common, see Microsoft KB: Q274323)
				unsigned long check = GetTickCount() - mStartTick;
				signed long msecOff = (signed long)(newTicks - check);
				if (msecOff < -200 || msecOff > 200)
				{
					// We must keep the timer running forward :)
					LONGLONG adjust = std::min(msecOff * liFrequency.QuadPart / 1000, newTime - mLastTime);
					mStartTime.QuadPart += adjust;
					newTime -= adjust;

					// Re-calculate milliseconds
					newTicks = (unsigned long)(1000 * newTime / liFrequency.QuadPart);
				}
				// Record last time for adjust
				mLastTime = newTime;
				liCurrent.QuadPart = newTime;
			}

			// Return the elapsed time
			if (command == TIMER_GETELAPSEDTIME)
			{
				fElapsedTime = (double)(liCurrent.QuadPart - m_llLastElapsedTime) / (double)m_llQPFTicksPerSec;
				m_llLastElapsedTime = liCurrent.QuadPart;
				return fElapsedTime;
			}

			// Return the current time
			if (command == TIMER_GETAPPTIME)
			{
				double fAppTime = (double)(liCurrent.QuadPart - m_llBaseTime) / (double)m_llQPFTicksPerSec;
				return fAppTime;
			}

			// Reset the timer
			if (command == TIMER_RESET)
			{
				m_llBaseTime = liCurrent.QuadPart;
				m_llLastElapsedTime = liCurrent.QuadPart;
				m_llStopTime = 0;
				m_bTimerStopped = FALSE;
				return 0.0;
			}

			// Start the timer
			if (command == TIMER_START)
			{
				if (m_bTimerStopped)
					m_llBaseTime += liCurrent.QuadPart - m_llStopTime;
				m_llStopTime = 0;
				m_llLastElapsedTime = liCurrent.QuadPart;
				m_bTimerStopped = FALSE;
				return 0.0;
			}

			// Stop the timer
			if (command == TIMER_STOP)
			{
				if (!m_bTimerStopped)
				{
					m_llStopTime = liCurrent.QuadPart;
					m_llLastElapsedTime = liCurrent.QuadPart;
					m_bTimerStopped = TRUE;
				}
				return 0.0;
			}

			// Advance the timer by 1/10th second
			if (command == TIMER_ADVANCE)
			{
				m_llStopTime += m_llQPFTicksPerSec / 10;
				return 0.0f;
			}

			if (command == TIMER_GETABSOLUTETIME)
			{
				fTime = liCurrent.QuadPart / (double)m_llQPFTicksPerSec;
				return fTime;
			}

			return -1.0; // Invalid command specified
		}
		else
		{
			// Get the time using timeGetTime()
			static double m_fLastElapsedTime = 0.0;
			static double m_fBaseTime = 0.0;
			static double m_fStopTime = 0.0;
			double fTime;
			double fElapsedTime;

			// Get either the current time or the stop time, depending
			// on whether we're stopped and what command was sent
			if (m_fStopTime != 0.0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
				fTime = m_fStopTime;
			else
				fTime = GetTickCount() * 0.001;

			// Return the elapsed time
			if (command == TIMER_GETELAPSEDTIME)
			{
				fElapsedTime = (double)(fTime - m_fLastElapsedTime);
				m_fLastElapsedTime = fTime;
				return fElapsedTime;
			}

			// Return the current time
			if (command == TIMER_GETAPPTIME)
			{
				return (fTime - m_fBaseTime);
			}

			// Reset the timer
			if (command == TIMER_RESET)
			{
				m_fBaseTime = fTime;
				m_fLastElapsedTime = fTime;
				m_fStopTime = 0;
				m_bTimerStopped = FALSE;
				return 0.0;
			}

			// Start the timer
			if (command == TIMER_START)
			{
				if (m_bTimerStopped)
					m_fBaseTime += fTime - m_fStopTime;
				m_fStopTime = 0.0f;
				m_fLastElapsedTime = fTime;
				m_bTimerStopped = FALSE;
				return 0.0;
			}

			// Stop the timer
			if (command == TIMER_STOP)
			{
				if (!m_bTimerStopped)
				{
					m_fStopTime = fTime;
					m_fLastElapsedTime = fTime;
					m_bTimerStopped = TRUE;
				}
				return 0.0;
			}

			// Advance the timer by 1/10th second
			if (command == TIMER_ADVANCE)
			{
				m_fStopTime += 0.1;
				return 0.0;
			}

			if (command == TIMER_GETABSOLUTETIME)
			{
				return fTime;
			}

			return -1.0; // Invalid command specified
		}
	}

	HINSTANCE g_hAppInstance = nullptr;
	WCHAR* g_sWindowClassName = L"ParaWorld";
	WCHAR* g_sWindowTitle = L"ParaEngine Window";
	ULONG_PTR						g_gdiplusToken;
	Gdiplus::GdiplusStartupInput	g_gdiplusStartupInput;

	CParaEngineApp::CParaEngineApp(const char*  lpCmdLine)
		: CParaEngineAppBase(lpCmdLine)
		, m_bServerMode(false)
		, m_bIsAppActive(true)
		, m_bHasNewConfig(false)
		, m_nAppState(PEAppState_None)
		, m_nScreenWidth(960)
		, m_nScreenHeight(640)
		, m_fTime(0)
		, m_fFPS(0.f)
		, m_main_timer(m_main_io_service)
		, m_bQuit(false)
		, m_bMainLoopExited(true)
		, m_pWinRawMsgQueue(nullptr)
		, m_dwWinThreadID(0)
		, m_hWnd(NULL)
		, m_fRefreshTimerInterval(IDEAL_FRAME_RATE)
		, m_bStartFullscreen(false)
		, m_nInitialGameEffectSet(0)
	{
		//SetTouchInputting(true);
		g_pCurrentApp = this;
		CFrameRateController::LoadFRCNormal();
		StartApp(lpCmdLine);

	}

	CParaEngineApp::~CParaEngineApp()
	{
	}

	void CParaEngineApp::SetRefreshTimer(float fTimeInterval, int nFrameRateControl)
	{
		m_fRefreshTimerInterval = fTimeInterval;
	}

	float CParaEngineApp::GetRefreshTimer() 
	{
		return m_fRefreshTimerInterval;
	}

	bool CParaEngineApp::IsAppActive()
	{
		return m_bIsAppActive;
	}

	HWND CParaEngineApp::GetMainWindow()
	{
		return m_hWnd;
	}

	void CParaEngineApp::SetMainWindow(HWND hWnd, bool bIsExternalWindow)
	{
		m_hWnd = hWnd;
		m_dwWinThreadID = ::GetWindowThreadProcessId(hWnd, NULL);
	}


	void CParaEngineApp::SetResolution(float x, float y)
	{
		m_nScreenWidth = (int)x;
		m_nScreenHeight = (int)y;

		if (m_pGLView)
		{
			m_pGLView->setFrameSize(x, y);
		}
	}

	void CParaEngineApp::GetResolution(float* pX, float* pY)
	{
		if (pX)
			*pX = (float)m_nScreenWidth;
		if (pY)
			*pY = (float)m_nScreenHeight;
	}

	void CParaEngineApp::SetScreenResolution(const Vector2& vSize)
	{
		//static bool s_bFirstTimeSet = true;
		// for mobile version, we will only allow setting it once at start up, scripting interface can not change the resolution.
		//if (s_bFirstTimeSet)
		{
			//s_bFirstTimeSet = false;
			SetResolution(vSize.x, vSize.y);
		}

		ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();
		CVariable value;
		value = (int)(vSize.x);
		settings.SetDynamicField("ScreenWidth", value);
		value = (int)(vSize.y);
		settings.SetDynamicField("ScreenHeight", value);
	}

	bool CParaEngineApp::IsSlateMode()
	{
		return true;
	}


	ParaEngine::Vector2 CParaEngineApp::GetScreenResolution()
	{
		Vector2 size;
		GetResolution(&size.x, &size.y);
		return size;
	}

	void CParaEngineApp::GetWindowCreationSize(int * pWidth, int * pHeight)
	{
		if (pWidth)
			*pWidth = m_nScreenWidth;
		if (pHeight)
			*pHeight = m_nScreenHeight;
	}

#if defined(STATIC_PLUGIN_CAUDIOENGINE)
	extern ClassDescriptor* AudioEngine_GetClassDesc();
#endif
	void CParaEngineApp::InitAudioEngine()
	{
#if defined(STATIC_PLUGIN_CAUDIOENGINE)
		OUTPUT_LOG("static cAudioEngine plugin loaded\n");
		CAudioEngine2::GetInstance()->InitAudioEngine((IParaAudioEngine*)AudioEngine_GetClassDesc()->Create());
#elif defined(WIN32)
		// load from dll
		CAudioEngine2::GetInstance()->InitAudioEngine();
#else
		OUTPUT_LOG("native Simple AudioEngine loaded\n");
		CAudioEngine2::GetInstance()->InitAudioEngine((IParaAudioEngine*)CParaSimpleAudioEngine::GetInstance());
#endif
	}

	HRESULT CParaEngineApp::Init(HWND* pHWND)
	{
		AutoSetLocale();

		CGUIEvent::StaticInit();

		CGlobals::GetSettings().LoadGameEffectSet(m_nInitialGameEffectSet);

		InitAudioEngine();

		// Create a blank root scene with certain dimensions
		m_pRootScene->SetBoundRect(1000.f, 1000.f, 0); // a very large scene
		m_pRootScene->SetMyType(_Scene);
		m_pRootScene->GetSceneState()->pAssetManager = CGlobals::GetAssetManager();
		m_pRootScene->GetSceneState()->CleanupSceneState();
		m_pRootScene->GetSceneState()->pGUIState = &(m_pGUIRoot->GetGUIState());

		/************************************************************************/
		/* Create ocean manager                                                 */
		/************************************************************************/
		CGlobals::GetOceanManager()->create();

		// GUI
		if (Is3DRenderingEnabled()) {
			CGlobals::GetAssetManager()->LoadGDIFont("sys", "System", 14);
		}
		CGlobals::GetGUI()->OneTimeGUIInit();

		return OneTimeSceneInit(pHWND);
	}

	HRESULT CParaEngineApp::OneTimeSceneInit(HWND* pHWND)
	{
		//load config file
		CGlobals::GetICConfigManager()->LoadFromFile();
		ParaInfoCenter::CICDBManager::StaticInit();

		srand(GetTickCount());

		//Performance monitor
		PERF_BEGIN("Program");

		/// load frame rate controller
		CFrameRateController::LoadFRCNormal();

		return S_OK;
	}


	void CParaEngineApp::InitSystemModules()
	{
		BootStrapAndLoadConfig();
		m_pParaWorldAsset.reset(new CParaWorldAsset());
		m_pRootScene.reset(new CSceneObject());
		m_pGUIRoot.reset(CGUIRoot::CreateInstance());
		m_pViewportManager.reset(new CViewportManager());
		m_pViewportManager->SetLayout(VIEW_LAYOUT_DEFAULT, m_pRootScene.get(), m_pGUIRoot.get());
	}

	void CParaEngineApp::InitApp(const char* sCommandLine)
	{
		if (m_pWinRawMsgQueue == 0)
			m_pWinRawMsgQueue = new CWinRawMsgQueue();

		SetAppCommandLine(sCommandLine);

		InitCommandLineParams();

		//COSInfo::DumpSystemInfo();

		if (Is3DRenderingEnabled())
		{
			// Initialize GDI+.
			Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
		}

		// Initialize COM
		CoInitialize(NULL);
	}


	HRESULT CParaEngineApp::StopApp()
	{
		SAFE_DELETE(m_pWinRawMsgQueue);

		InvalidateDeviceObjects();
		DeleteDeviceObjects();

		FinalCleanup();
		m_pParaWorldAsset.reset();
		m_pRootScene.reset();
		m_pGUIRoot.reset();
		m_pViewportManager.reset();
		m_pGUIRoot.reset();

		// delete m_pAudioEngine;
		CoUninitialize();

		//#ifdef LOG_FILES_ACTIVITY
		if (CFileLogger::GetInstance()->IsBegin())
		{
			CFileLogger::GetInstance()->EndFileLog();
			CFileLogger::GetInstance()->SaveLogToFile("temp/filelog.txt");
		}

		return S_OK;
	}

	HRESULT CParaEngineApp::FinalCleanup()
	{
		//CAsyncLoader::GetSingleton().Stop();

		if (!CGlobals::GetAISim()->IsCleanedUp())
		{
			if (CGlobals::GetEventsCenter())
			{
				SystemEvent event(SystemEvent::SYS_WM_DESTROY, "");
				// set sync mode, so that event is processed immediately, since there is no next frame move.
				event.SetAsyncMode(false);
				CGlobals::GetEventsCenter()->FireEvent(event);
			}
		}
		CGlobals::GetNPLRuntime()->Cleanup();
		CGlobals::GetAISim()->CleanUp();

		m_pGUIRoot->Release();		// GUI: 2D engine
		m_pRootScene->Cleanup();
		CSingleton<CObjectManager>::Instance().Finalize();
		CSingleton<CGUIHighlightManager>::Instance().Finalize();
		CGlobals::GetAssetManager()->Cleanup();
		CAudioEngine2::GetInstance()->CleanupAudioEngine();
		return S_OK;
	}


	HRESULT CParaEngineApp::InitDeviceObjects()
	{
		CGlobals::GetAssetManager()->InitDeviceObjects();
		CGlobals::GetScene()->InitDeviceObjects();
		CGlobals::GetGUI()->InitDeviceObjects();
		return S_OK;
	}

	HRESULT CParaEngineApp::RestoreDeviceObjects()
	{
		CGlobals::GetAssetManager()->RestoreDeviceObjects();
		CGlobals::GetScene()->RestoreDeviceObjects();
		CGlobals::GetGUI()->RestoreDeviceObjects(m_nScreenWidth, m_nScreenHeight);
		return 0;
	}

	HRESULT CParaEngineApp::InvalidateDeviceObjects()
	{
		CGlobals::GetGUI()->InvalidateDeviceObjects();
		CGlobals::GetScene()->InvalidateDeviceObjects();
		CGlobals::GetAssetManager()->InvalidateDeviceObjects();
		return 0;
	}

	HRESULT CParaEngineApp::DeleteDeviceObjects()
	{
		CGlobals::GetGUI()->DeleteDeviceObjects();
		CGlobals::GetScene()->DeleteDeviceObjects();
		CGlobals::GetAssetManager()->DeleteDeviceObjects();
		return 0;
	}

	LRESULT CParaEngineApp::SendMessageToApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (m_pWinRawMsgQueue)
		{
			CWinRawMsg_ptr msg(new CWinRawMsg(hWnd, uMsg, wParam, lParam));
			m_pWinRawMsgQueue->push(msg);
		}
		return 0;
	}

	bool CParaEngineApp::GetMessageFromApp(CWinRawMsg* pMsg)
	{
		if (m_pWinRawMsgQueue)
		{
			CWinRawMsg_ptr msg;
			if (m_pWinRawMsgQueue->try_pop(msg))
			{
				if (pMsg)
				{
					(*pMsg) = *msg;
				}
				return true;
			}
		}
		return false;
	}


	bool CParaEngineApp::PostWinThreadMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (m_dwWinThreadID != 0)
		{
			return !!::PostThreadMessage(m_dwWinThreadID, uMsg, wParam, lParam);
		}
		return false;
	}

	// return 0 if not processed, 1 if processed, 2 if no further messages in the queue should ever be processed.
	LRESULT CParaEngineApp::MsgProcApp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		bool bIsSceneEnabled = !(m_pRootScene == NULL || !m_pRootScene->IsInitialized());
		if (bIsSceneEnabled)
		{
			// let the GUI system to process the message first.
			bool bNoFurtherProcess = false;
			LRESULT result = m_pGUIRoot->MsgProc(hWnd, uMsg, wParam, lParam, bNoFurtherProcess);
			if (bNoFurtherProcess) {
				return 1;
			}
		}

		return 0;
	}


	HRESULT CParaEngineApp::FrameMove(double fTime)
	{

		//{
		//	HRESULT result = 0;
		//	CWinRawMsg msg;
		//	while (GetMessageFromApp(&msg))
		//	{
		//		if (MsgProcApp(msg.m_hWnd, msg.m_uMsg, msg.m_wParam, msg.m_lParam) == 2)
		//		{
		//			return S_OK;
		//		}
		//	}
		//}

		double fElapsedGameTime = CGlobals::GetFrameRateController(FRC_GAME)->FrameMove(fTime);
		double fElapsedEnvSimTime = CGlobals::GetFrameRateController(FRC_SIM)->FrameMove(fTime);
		double fElapsedIOTime = CGlobals::GetFrameRateController(FRC_IO)->FrameMove(fTime);
		if (fElapsedEnvSimTime > 0)
		{
			{
				static double g_last_time = 0.f;
				// since it is server build, we will activate as many times as possible.
				PERF1("Script&Net FrameMove");
				CGlobals::GetAISim()->FrameMove((float)(fTime - g_last_time));
				g_last_time = fTime;
			}
			{
				PERF1("EnvironmentSim");
				CGlobals::GetEnvSim()->Animate((float)fElapsedEnvSimTime);  // generate valid LLE from HLE
			}
			CAudioEngine2::GetInstance()->Update();


			HandleUserInput(); // user input.

			// please note: IOTime controller is never used. here the scene delta must be the same as EnvSim delta
			// this fix a camera flicker bug, where the camera is not updated, but the environment and render tick is called.
			// on ParaEngineClient, this is not necessary.
			m_pRootScene->Animate((float)fElapsedEnvSimTime);
		}

		OnFrameEnded();
		return S_OK;
	}

	void CParaEngineApp::HandleUserInput()
	{
		/** handle 2D GUI input: dispatch mouse and key event for gui objects. */
		m_pGUIRoot->HandleUserInput();

		if (IsAppActive())
		{
			/** handle the automatic camera user input.*/
			CAutoCamera* pCamera = ((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()));
			if (pCamera)
				pCamera->HandleUserInput();

			/** 3D scene handler */
			CGlobals::GetScene()->HandleUserInput();
		}
	}

	void CParaEngineApp::SetWindowText(const char* pChar)
	{

	}

	const char* CParaEngineApp::GetWindowText()
	{
		return CGlobals::GetString().c_str();
	}

	void CParaEngineApp::WriteConfigFile(const char* sFileName)
	{

	}

	bool CParaEngineApp::HasNewConfig()
	{
		return m_bHasNewConfig;
	}

	void CParaEngineApp::SetHasNewConfig(bool bHasNewConfig)
	{
		m_bHasNewConfig = bHasNewConfig;
	}

	void CParaEngineApp::Exit(int nReturnCode /*= 0*/)
	{
		SetReturnCode(nReturnCode);
		OUTPUT_LOG("program exited with code %d\n", nReturnCode);
		SetAppState(PEAppState_Exiting);

#ifdef TODO_FONT
		FontFreeType::shutdownFreeType_exit();
#endif
	}

	ParaEngine::PEAppState CParaEngineApp::GetAppState()
	{
		return m_nAppState;
	}

	void CParaEngineApp::SetAppState(ParaEngine::PEAppState state)
	{
		m_nAppState = state;
	}

	NPL::INPLRuntime* CParaEngineApp::GetNPLRuntime()
	{
		return (NPL::INPLRuntime*)(NPL::CNPLRuntime::GetInstance());
	}

	HRESULT CParaEngineApp::StartApp(const char* sCommandLine)
	{
		SetCurrentInstance(this);
		std::string strCmd;
		VerifyCommandLine(sCommandLine, strCmd);
		InitApp(strCmd.c_str());

		LoadPackages();
		InitSystemModules();
		LoadAndApplySettings();
		return S_OK;
	}


	void CParaEngineApp::LoadAndApplySettings()
	{
		// load from settings.
		auto& settings = ParaEngineSettings::GetSingleton();

		CDynamicAttributeField* pField = NULL;
		if ((pField = settings.GetDynamicField("StartFullscreen")))
			m_bStartFullscreen = (bool)(*pField);
		else
			m_bStartFullscreen = false;

		const char* sIsFullScreen = GetAppCommandLineByParam("fullscreen", NULL);
		if (sIsFullScreen)
			m_bStartFullscreen = (strcmp("true", sIsFullScreen) == 0);

		if ((pField = settings.GetDynamicField("ScreenWidth")))
			m_nScreenWidth = (int)(*pField);

		if ((pField = settings.GetDynamicField("ScreenHeight")))
			m_nScreenHeight = (int)(*pField);

		if ((pField = settings.GetDynamicField("ScriptEditor")))
			settings.SetScriptEditor((const string&)(*pField));

		if ((pField = settings.GetDynamicField("GameEffectSet")))
			m_nInitialGameEffectSet = (int)(*pField);

		if ((pField = settings.GetDynamicField("InverseMouse")))
			settings.SetMouseInverse((bool)(*pField));

		if ((pField = settings.GetDynamicField("language")))
			settings.SetLocale((const char*)(*pField));

		if ((pField = settings.GetDynamicField("TextureLOD")))
			settings.SetTextureLOD((int)(*pField));

		
	}

	void CParaEngineApp::BootStrapAndLoadConfig()
	{
		if (CBootStrapper::GetSingleton()->LoadFromFile(GetAppCommandLineByParam("bootstrapper", "script/main.lua")))
		{
			CGlobals::GetAISim()->SetGameLoop(CBootStrapper::GetSingleton()->GetMainLoopFile());
		}
		else
		{
			OUTPUT_LOG("error: can not find bootstrapper file at %s\n", GetAppCommandLineByParam("bootstrapper", ""));
		}
	}

	CViewportManager* CParaEngineApp::GetViewportManager()
	{
		return m_pViewportManager.get();
	}

	void CParaEngineApp::UpdateStats(double fTime)
	{
		// Keep track of the frame count
		static double fLastTime = 0.0f;
		static DWORD dwFrames = 0;
		++dwFrames;
		// Update the scene stats once per second
		if (fTime - fLastTime > 1.0f)
		{
			m_fFPS = (float)(dwFrames / (fTime - fLastTime));
			fLastTime = fTime;
			dwFrames = 0;
		}
	}

	float CParaEngineApp::GetFPS() const
	{
		return m_fFPS;
	}

	HRESULT CParaEngineApp::Render3DEnvironment(bool bForceRender /*= false*/)
	{
		double fTime = m_fTime;
		UpdateStats(fTime);

		if (m_bServerMode)
			return E_FAIL;

		float fElapsedTime = (float)(CGlobals::GetFrameRateController(FRC_RENDER)->FrameMove(fTime));

		// Get the app's time, in seconds. Skip rendering if no time elapsed
		double fAppTime = DXUtil_Timer(TIMER_GETAPPTIME);
		double fElapsedAppTime = DXUtil_Timer(TIMER_GETELAPSEDTIME);

		//////////////////////////////////////////////////////////////////////////
		//
		// frame rate control at 30 FPS
		//
		//////////////////////////////////////////////////////////////////////////

		static double fRenderTime = 0;
		static double fConstTime = 0;
		bool bUseIdealFrameRate = /*(m_nFrameRateControl == 1);*/ false;

		fRenderTime += fElapsedAppTime;
		fConstTime += fElapsedAppTime;

		if (!bUseIdealFrameRate || (fConstTime >= IDEAL_FRAME_RATE) || bForceRender)
		{
			//m_fElapsedTime = bUseIdealFrameRate ? fRenderTime : fElapsedAppTime;


			if (bForceRender)
			{
				fConstTime = 0;
			}
			else
			{
				// TRICKY: this helps to smooth the frame rate to IDEAL_FRAME_RATE, when force render is false
				fConstTime -= IDEAL_FRAME_RATE;
				if (fConstTime >= IDEAL_FRAME_RATE) // if 0.5 seconds is passed, we will catch up 
					fConstTime = 0;
			}
			fRenderTime = 0;

			if ((0.0f == fElapsedAppTime) /*&& m_bFrameMoving*/)
				return S_OK;

			if (bUseIdealFrameRate)
			{
				// only use ideal frame rate if interval is smaller than it.
				if (m_fRefreshTimerInterval < IDEAL_FRAME_RATE)
				{
					// TRICKY: this fixed a bug of inaccurate timing, we will assume a perfect timing here, but correct it if it differentiate too much from   
					// FrameMove (animate) the scene, frame move is called before Render(), since the frame move may contain ForceRender() calls
					// Store the time for the app
					if (m_fTime < (fAppTime - IDEAL_FRAME_RATE)) // if 0.5 seconds is passed, we will catch up 
						m_fTime = fAppTime;
					else
						m_fTime += IDEAL_FRAME_RATE;
				}
				else
				{
					// if we special refresh timer to be 0.03334 or bigger, we shall use absolute time. 
					m_fTime = fAppTime;
				}
			}
			else
			{
				m_fTime = fAppTime;
			}

			// OUTPUT_LOG("%f:%f, %f, %f\n", m_fTime, fAppTime, m_fElapsedTime, fConstTime);

			//__try
			{
				FrameMove(m_fTime);
				
				{
					RenderDevice::ClearAllPerfCount();

					CMoviePlatform* pMoviePlatform = CGlobals::GetMoviePlatform();
					pMoviePlatform->BeginCaptureFrame();

					LinearColor color(0.f, 0.f, 1.f, 0.f);
					CGlobals::GetRenderDevice()->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, m_pRootScene->GetClearColor(), 1.0f, 0L);

					PERF1("Main Render");
					{
						CGlobals::GetAssetManager()->RenderFrameMove(fElapsedTime); // for asset manager

						m_pViewportManager->UpdateViewport(m_nScreenWidth, m_nScreenHeight);
						{
							PERF1("3D Scene Render");
							m_pViewportManager->Render(fElapsedTime, PIPELINE_3D_SCENE);
						}
						{
							PERF1("GUI Render");
							m_pViewportManager->Render(fElapsedTime, PIPELINE_UI);
						}
						{
							m_pViewportManager->Render(fElapsedTime, PIPELINE_POST_UI_3D_SCENE);
						}
					}

					pMoviePlatform->EndCaptureFrame();
				}
			}
			//__except (GenerateDump(GetExceptionInformation()))
			//{
			//	exit(0);
			//}
			// Show the frame on the primary surface.
		}


		return S_OK;
	}


	void CParaEngineApp::UpdateMouse()
	{
		static int oldX = 0;
		static int oldY = 0;
		static int oldBtnLeft = 0;
		static int oldBtnRight = 0;
		static int oldBtnMiddle = 0;

		int x, y, btnLeft, btnRight, btnMiddle;

		// FsGetMouseState(btnLeft,btnMiddle,btnRight,x,y);

		if (oldBtnLeft != btnLeft)
		{
			this->SetTouchInputting(false);
			if (btnLeft == 0)
			{
				CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_LBUTTONUP, 0, MAKELPARAM(x, y));
			}
			else
			{
				CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_LBUTTONDOWN, 0, MAKELPARAM(x, y));
			}
			oldBtnLeft = btnLeft;
		}
		else if (oldBtnRight != btnRight)
		{
			this->SetTouchInputting(false);
			if (btnRight == 0)
			{
				CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_RBUTTONUP, 0, MAKELPARAM(x, y));
			}
			else
			{
				CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_RBUTTONDOWN, 0, MAKELPARAM(x, y));
			}
			oldBtnRight = btnRight;
		}
		else if (oldBtnMiddle != btnMiddle)
		{
			this->SetTouchInputting(false);
			if (btnMiddle == 0)
			{
				CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MBUTTONUP, 0, MAKELPARAM(x, y));
			}
			else
			{
				CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MBUTTONDOWN, 0, MAKELPARAM(x, y));
			}
			oldBtnMiddle = btnMiddle;
		}
		else if ((oldX != x) || (oldY != y))
		{
			CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
		}

		oldX = x;
		oldY = y;
	}

	bool CParaEngineApp::IsDebugBuild()
	{
#ifdef _DEBUG
		return true;
#else
		return false;
#endif
	}


	HRESULT CParaEngineApp::Create(HINSTANCE hInstance)
	{
		SetAppState(PEAppState_Device_Created);

		Init(nullptr);
		InitDeviceObjects();
		RestoreDeviceObjects();

		SetAppState(PEAppState_Ready);

		return S_OK;
	}


	bool CParaEngineApp::IsKeyPressed(DWORD nKey)
	{
		return (m_pGLView) ? m_pGLView->IsKeyPressed(nKey) : false;
	}


	int CParaEngineApp::CalculateRenderTime(double fIdealInterval, double* pNextInterval)
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

	void CParaEngineApp::handle_mainloop_timer(const boost::system::error_code& err)
	{
		m_bQuit = m_pGLView ? m_pGLView->windowShouldClose() : false;

		if (!err && !m_bQuit)
		{
				PEAppState dwState = GetAppState();
				if (dwState == PEAppState_Ready)
				{
				}
				else if (dwState == PEAppState_Device_Error /*|| dwState == PEAppState_Exiting*/)
				{
					return;
				}
				else if (dwState == PEAppState_None)
				{
					Create(nullptr);
				}

				double fIdealInterval = (GetRefreshTimer() <= 0) ? IDEAL_FRAME_RATE : GetRefreshTimer();
				double fNextInterval = 0.f;
				int nFrameDelta = CalculateRenderTime(fIdealInterval, &fNextInterval);

				if (nFrameDelta > 0)
				{
					if (dwState == PEAppState_Ready)
					{
						// the frame move and render the scene. 
						m_pGLView->pollEvents();
						Render3DEnvironment(false);
						m_pGLView->swapBuffers();
					}
				}

				m_main_timer.expires_from_now(std::chrono::milliseconds((int)(fNextInterval * 1000)));
				m_main_timer.async_wait(boost::bind(&CParaEngineApp::handle_mainloop_timer, this, boost::asio::placeholders::error));
		}
	}

	int CParaEngineApp::Run(HINSTANCE hInstance)
	{

		//add a console window for debug or when in server mode.
		if (!Is3DRenderingEnabled() || IsDebugBuild())
		{
			/*const char* sInteractiveMode = GetAppCommandLineByParam("i", NULL);
			bool bIsInterpreterMode = (sInteractiveMode && strcmp(sInteractiveMode, "true") == 0);
			if (!bIsInterpreterMode)*/
			{
				RedirectIOToConsole();
			}
		}
		auto result = 0;
		if (Is3DRenderingEnabled())
		{

			m_pGLView = CParaEngineGLView::createWithRect("ParaEngine", Rect(0, 0, (float)m_nScreenWidth, (float)m_nScreenHeight));

			SetMainWindow(m_pGLView->getWin32Window());

			using namespace boost::asio;

			// start the main loop timer. 
			m_main_timer.expires_from_now(std::chrono::milliseconds(50));
			m_main_timer.async_wait(boost::bind(&CParaEngineApp::handle_mainloop_timer, this, boost::asio::placeholders::error));

			// start the main loop now
			m_bMainLoopExited = false;
			m_main_io_service.run();
			m_bMainLoopExited = true;

			if (m_pGLView->isOpenGLReady())
			{
				m_pGLView->end();
			}


			return GetReturnCode();

		}
		else
		{
			// the console window is used.
			CParaEngineService service;
			result = service.Run(0, this);
		}
		return result;

	}

} // end namespace
