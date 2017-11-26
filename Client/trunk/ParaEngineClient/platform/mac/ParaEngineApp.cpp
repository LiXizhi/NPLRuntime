//-----------------------------------------------------------------------------
// Class:	CParaEngine
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8
// Desc: Just for mobile version (android, iOS, cocos2dx) used by AppDelegate
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#ifdef PLATFORM_MAC

#include "OpenGLWrapper.h"


//#include "ParaAudioMac.h"
//#include "SimpleAudioEngine.h"
#include "ParaSimpleAudioEngine.h"


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

// header include
#include "ParaEngineApp.h"
#include <time.h>

#include "fssimplewindow.h"

#include <unordered_map>
#include <cassert>

using namespace std;
using namespace ParaEngine;

#include "GLFontFreeType.h"


CParaEngineApp::CParaEngineApp(const char*  lpCmdLine)
	:CParaEngineAppBase(lpCmdLine), m_bServerMode(false), m_bIsAppActive(true), m_bHasNewConfig(false), m_nAppState(PEAppState_None), m_nScreenWidth(960), m_nScreenHeight(640), m_fTime(0), m_fFPS(0.f)
{
	SetTouchInputting(true);
	g_pCurrentApp = this;
	CFrameRateController::LoadFRCNormal();
	StartApp(lpCmdLine);

}

CParaEngineApp::~CParaEngineApp()
{
#ifdef USE_OPENGL_RENDERER
	//TODO:wangpeng cocos2d::Director::getInstance()->getEventDispatcher()->removeEventListener(m_rendererRecreatedListener);
#endif
}

bool CParaEngineApp::IsAppActive()
{
	return m_bIsAppActive;
}

#ifdef USE_OPENGL_RENDERER
void CParaEngineApp::listenRendererRecreated(cocos2d::EventCustom* event)
{
//#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_ANDROID || PARA_TARGET_PLATFORM == PARA_PLATFORM_WP8)
	OUTPUT_LOG("\nInfo: opengl listenRendererRecreated \n\n");
	// listen the event that renderer was recreated on Android/WP8
	// all opengl related id has already become invalid at this time, no need to release them, just recreate them all.
	CGlobals::GetRenderDevice()->RendererRecreated();
	CGlobals::GetAssetManager()->RendererRecreated();
	m_pRootScene->RendererRecreated();
//#endif
}

void CParaEngineApp::applicationDidEnterBackground()
{
#if defined(STATIC_PLUGIN_CAUDIOENGINE) || defined(WIN32)
	CAudioEngine2::GetInstance()->PauseAll();
#else
	//TODO: wangpeng
	//CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
#endif
	m_bIsAppActive = false;
}

void CParaEngineApp::applicationWillEnterForeground()
{
#if defined(STATIC_PLUGIN_CAUDIOENGINE) || defined(WIN32)
	CAudioEngine2::GetInstance()->ResumeAll();
#else
	//TODO: wangpeng
	// CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
#endif
	m_bIsAppActive = true;
}

#endif

void CParaEngineApp::SetResolution(float x, float y)
{
	m_nScreenWidth = (int)x;
	m_nScreenHeight = (int)y;
}

void CParaEngineApp::GetResolution(float* pX, float* pY)
{
	if (pX)
		*pX = m_nScreenWidth;
	if (pY)
		*pY = m_nScreenHeight;
}

void CParaEngineApp::SetScreenResolution(const Vector2& vSize)
{
	static bool s_bFirstTimeSet = true;
	// for mobile version, we will only allow setting it once at start up, scripting interface can not change the resolution.
	if (s_bFirstTimeSet)
	{
		s_bFirstTimeSet = false;
		SetResolution(vSize.x, vSize.y);
	}
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

HRESULT CParaEngineApp::Init( HWND* pHWND )
{
	AutoSetLocale();

	CGUIEvent::StaticInit();

	CGlobals::GetSettings().LoadGameEffectSet(0);

	InitAudioEngine();

	// Create a blank root scene with certain dimensions
	m_pRootScene->SetBoundRect(1000.f, 1000.f, 0); // a very large scene
	m_pRootScene->SetMyType(_Scene);
	m_pRootScene->GetSceneState()->pAssetManager = CGlobals::GetAssetManager();
	m_pRootScene->GetSceneState()->CleanupSceneState();
	m_pRootScene->GetSceneState()->pGUIState = &(m_pGUIRoot->GetGUIState());

	// GUI
	if (Is3DRenderingEnabled()){
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

	srand( GetTickCount() );

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


HRESULT CParaEngineApp::StopApp()
{
#ifdef USE_OPENGL_RENDERER
	//TODO: wangpeng
	//CocosDenshion::SimpleAudioEngine::end();
#endif
	InvalidateDeviceObjects();
	DeleteDeviceObjects();

	FinalCleanup();
	m_pRootScene.reset();
	m_pGUIRoot.reset();
	m_pViewportManager.reset();
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

HRESULT CParaEngineApp::FrameMove(double fTime)
{





	m_fTime = fTime;
	double fElapsedGameTime = CGlobals::GetFrameRateController(FRC_GAME)->FrameMove(fTime);
	double fElapsedEnvSimTime = CGlobals::GetFrameRateController(FRC_SIM)->FrameMove(fTime);
	double fElapsedIOTime = CGlobals::GetFrameRateController(FRC_IO)->FrameMove(fTime);
	if( fElapsedEnvSimTime > 0 )
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

void CParaEngineApp::SetWindowText( const char* pChar )
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

void CParaEngineApp::SetHasNewConfig( bool bHasNewConfig )
{
	m_bHasNewConfig = bHasNewConfig;
}

void CParaEngineApp::Exit( int nReturnCode /*= 0*/ )
{
	SetReturnCode(nReturnCode);
	OUTPUT_LOG("program exited with code %d\n", nReturnCode);
	SetAppState(PEAppState_Exiting);

	FontFreeType::shutdownFreeType_exit();
}

ParaEngine::PEAppState CParaEngineApp::GetAppState()
{
	return m_nAppState;
}

void CParaEngineApp::SetAppState( ParaEngine::PEAppState state )
{
	m_nAppState = state;
}

NPL::INPLRuntime* CParaEngineApp::GetNPLRuntime()
{
	return (NPL::INPLRuntime*)(NPL::CNPLRuntime::GetInstance());
}

HRESULT CParaEngineApp::StartApp(const char* sCommandLine)
{
	LoadPackages();
	InitSystemModules();
	return S_OK;
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

	RenderDevice::ClearAllPerfCount();

	float fElapsedTime = (float)(CGlobals::GetFrameRateController(FRC_RENDER)->FrameMove(fTime));

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
	return S_OK;
}



static std::unordered_map<int,uint32> g_KeyCodeMap;
static void InitKeyCodeMap()
{
    
    
    static uint32 kv[FSKEY_NUM_KEYCODE*2]= {
        FSKEY_NULL,0,
        FSKEY_SPACE,VK_SPACE,
        FSKEY_0,'0',
        FSKEY_1,'1',
        FSKEY_2,'2',
        FSKEY_3,'3',
        FSKEY_4,'4',
        FSKEY_5,'5',
        FSKEY_6,'6',
        FSKEY_7,'7',
        FSKEY_8,'8',
        FSKEY_9,'9',
        FSKEY_A,'A',
        FSKEY_B,'B',
        FSKEY_C,'C',
        FSKEY_D,'D',
        FSKEY_E,'E',
        FSKEY_F,'F',
        FSKEY_G,'G',
        FSKEY_H,'H',
        FSKEY_I,'I',
        FSKEY_J,'J',
        FSKEY_K,'K',
        FSKEY_L,'L',
        FSKEY_M,'M',
        FSKEY_N,'N',
        FSKEY_O,'O',
        FSKEY_P,'P',
        FSKEY_Q,'Q',
        FSKEY_R,'R',
        FSKEY_S,'S',
        FSKEY_T,'T',
        FSKEY_U,'U',
        FSKEY_V,'V',
        FSKEY_W,'W',
        FSKEY_X,'X',
        FSKEY_Y,'Y',
        FSKEY_Z,'Z',
        FSKEY_ESC,VK_ESCAPE,
        FSKEY_F1,VK_F1,
        FSKEY_F2,VK_F2,
        FSKEY_F3,VK_F3,
        FSKEY_F4,VK_F4,
        FSKEY_F5,VK_F5,
        FSKEY_F6,VK_F6,
        FSKEY_F7,VK_F7,
        FSKEY_F8,VK_F8,
        FSKEY_F9,VK_F9,
        FSKEY_F10,VK_F10,
        FSKEY_F11,VK_F11,
        FSKEY_F12,VK_F12,
        FSKEY_PRINTSCRN,VK_PRINT,
        FSKEY_CAPSLOCK,VK_CAPITAL,
        FSKEY_SCROLLLOCK,VK_SCROLL,
        FSKEY_PAUSEBREAK,VK_PAUSE,
        FSKEY_BS,VK_BACK,
        FSKEY_TAB,VK_TAB,
        FSKEY_ENTER,VK_RETURN,
        FSKEY_SHIFT,VK_SHIFT,
        FSKEY_CTRL,VK_CONTROL,
        FSKEY_ALT,VK_LMENU,
        FSKEY_INS,VK_INSERT,
        FSKEY_DEL,VK_DELETE,
        FSKEY_HOME,VK_HOME,
        FSKEY_END,VK_END,
        FSKEY_PAGEUP,VK_PRIOR,
        FSKEY_PAGEDOWN,VK_NEXT,
        FSKEY_UP,VK_UP,
        FSKEY_DOWN,VK_DOWN,
        FSKEY_LEFT,VK_LEFT,
        FSKEY_RIGHT,VK_RIGHT,
        FSKEY_NUMLOCK,VK_NUMLOCK,
        FSKEY_TILDA,0,
        FSKEY_MINUS,VK_OEM_MINUS,
        FSKEY_PLUS,VK_OEM_PLUS,
        FSKEY_LBRACKET,VK_OEM_4,
        FSKEY_RBRACKET,VK_OEM_6,
        FSKEY_BACKSLASH,VK_OEM_5,
        FSKEY_SEMICOLON,VK_OEM_1,
        FSKEY_SINGLEQUOTE,VK_OEM_7,
        FSKEY_COMMA,VK_OEM_COMMA,
        FSKEY_DOT,VK_OEM_PERIOD,
        FSKEY_SLASH,VK_OEM_2,
        FSKEY_TEN0,VK_NUMPAD0,
        FSKEY_TEN1,VK_NUMPAD1,
        FSKEY_TEN2,VK_NUMPAD2,
        FSKEY_TEN3,VK_NUMPAD3,
        FSKEY_TEN4,VK_NUMPAD4,
        FSKEY_TEN5,VK_NUMPAD5,
        FSKEY_TEN6,VK_NUMPAD6,
        FSKEY_TEN7,VK_NUMPAD7,
        FSKEY_TEN8,VK_NUMPAD8,
        FSKEY_TEN9,VK_NUMPAD9,
        FSKEY_TENDOT,VK_OEM_PERIOD,
        FSKEY_TENSLASH,VK_OEM_2,
        FSKEY_TENSTAR,VK_MULTIPLY,
        FSKEY_TENMINUS,VK_OEM_MINUS,
        FSKEY_TENPLUS,VK_OEM_PLUS,
        FSKEY_TENENTER,VK_RETURN,
        FSKEY_WHEELUP,0,
        FSKEY_WHEELDOWN,0
    };
    g_KeyCodeMap.clear();
    
    for(int i = 0;i<FSKEY_NUM_KEYCODE*2;i+=2)
    {
        g_KeyCodeMap[kv[i]] = kv[i]+1;
    }
    
    //g_KeyCodeMap[]
}

void CParaEngineApp::UpdateKey()
{
    //int FsInkey(void);
    //int FsInkeyChar();
    
    int fsKeyCode = FsInkey();
    if(fsKeyCode == 0) return;
    
    int fsKeyState = FsGetKeyState(fsKeyCode); //0 keyup 1 keydown
    
    UINT msg = WM_KEYUP;
    if(fsKeyState == 1)
    {
        msg = WM_KEYDOWN;
    }
    
    uint32 keyCode = g_KeyCodeMap[fsKeyCode];
    
    CGUIRoot::GetInstance()->GetKeyboard()->PushKeyEvent(msg,keyCode,0);
    
    int character = FsInkeyChar();
    if(character > 0)
    {
        auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();
        if(pGUI)
        {
            std::wstring s;
            s+= (WCHAR)character;
            pGUI->OnHandleWinMsgChars(s);
        }
    }
    
    
    //int FmsgeyChar(void);
    //int FsGetKeyState(int fsKeyCode);
}


void CParaEngineApp::UpdateMouse()
{
	static int oldX = 0;
	static int oldY = 0;
	static int oldBtnLeft = 0;
	static int oldBtnRight = 0;
	static int oldBtnMiddle = 0;

	int x,y,btnLeft,btnRight,btnMiddle;
	FsGetMouseState(btnLeft,btnMiddle,btnRight,x,y);

	if ( oldBtnLeft != btnLeft )
	{
		this->SetTouchInputting(false);
		if ( btnLeft == 0 )
		{
            CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_LBUTTONUP, 0, MAKELPARAM(x, y));
		}
		else
		{
            CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_LBUTTONDOWN, 0, MAKELPARAM(x, y));
		}
		oldBtnLeft = btnLeft;
	}
	else if ( oldBtnRight != btnRight )
	{
		this->SetTouchInputting(false);
		if ( btnRight == 0 )
		{
            CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_RBUTTONUP, 0, MAKELPARAM(x, y));
		}
		else
		{
            CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_RBUTTONDOWN, 0, MAKELPARAM(x, y));
		}
		oldBtnRight = btnRight;
	}
	else if ( oldBtnMiddle != btnMiddle )
	{
		this->SetTouchInputting(false);
		if ( btnMiddle == 0 )
		{
            CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MBUTTONUP, 0, MAKELPARAM(x, y));
		}
		else
		{
            CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MBUTTONDOWN, 0, MAKELPARAM(x, y));
		}
		oldBtnMiddle = btnMiddle;
	}
	else if ( (oldX != x) ||  (oldY != y) )
	{
		CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
	}

	oldX = x;
	oldY = y;
}

int CParaEngineApp::Run(HINSTANCE hInstance)
{
    
    InitKeyCodeMap();
    
	FsPassedTime();  // Reset the timer

	// this is server mode
	auto nStartTime = GetTickCount();

	while (GetAppState() != PEAppState_Exiting)
	{

        FsPollDevice();
        if(GetAppState() == PEAppState_Exiting || GetAppState() == PEAppState_Exited)
        {
            break;
        }

        FsGetWindowSize(m_nScreenWidth,m_nScreenHeight);

        UpdateMouse();
        UpdateKey();

		auto nCurTickCount = GetTickCount() - nStartTime;
		FrameMove(nCurTickCount / 1000.f);

		int passed = FsPassedTime();


        glClear(GL_COLOR_BUFFER_BIT);

		Render3DEnvironment(true);

        FsSwapBuffers();
        FsSleep(33-passed);

	}



	//StopApp();

	return GetReturnCode();

}

#endif
