//-----------------------------------------------------------------------------
// Class:	CParaEngine
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8
// Desc: Just for mobile version (android, iOS, cocos2dx) used by AppDelegate
//-----------------------------------------------------------------------------

#include "ParacraftApp.h"

#include <time.h>

using namespace std;
using namespace ParaEngine;


CParaEngineApp::CParaEngineApp(const char*  lpCmdLine)
	:CParaEngineAppBase(lpCmdLine), m_bServerMode(false), m_bIsAppActive(true), m_bHasNewConfig(false), m_nAppState(PEAppState_None), m_nScreenWidth(960), m_nScreenHeight(640), m_fTime(0), m_fFPS(0.f)
{
	SetTouchInputting(true);
	CFrameRateController::LoadFRCNormal();
	StartApp(lpCmdLine);
}

CParaEngineApp::~CParaEngineApp()
{

}

bool CParaEngineApp::IsAppActive()
{
	return m_bIsAppActive;
}

#ifdef USE_OPENGL_RENDERER
void CParaEngineApp::OnRenderRecreated()
{

	//CGlobals::GetRenderDevice()->RendererRecreated();
	CGlobals::GetAssetManager()->RendererRecreated();
	m_pRootScene->RendererRecreated();
}

void CParaEngineApp::applicationDidEnterBackground()
{
#if defined(STATIC_PLUGIN_CAUDIOENGINE) || defined(WIN32)
	CAudioEngine2::GetInstance()->PauseAll();
#else
	CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
#endif
	m_bIsAppActive = false;
}

void CParaEngineApp::applicationWillEnterForeground()
{
#if defined(STATIC_PLUGIN_CAUDIOENGINE) || defined(WIN32)
	CAudioEngine2::GetInstance()->ResumeAll();
#else
	CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
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
	//OUTPUT_LOG("static cAudioEngine plugin loaded\n");
	//CAudioEngine2::GetInstance()->InitAudioEngine((IParaAudioEngine*)AudioEngine_GetClassDesc()->Create());
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

	InitDeviceObjects();
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


void CParaEngineApp::StopApp()
{

	InvalidateDeviceObjects();
	DeleteDeviceObjects();

	FinalCleanup();
	m_pRootScene.reset();
	m_pGUIRoot.reset();
	m_pViewportManager.reset();
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

bool CParaEngineApp::StartApp(const char* sCommandLine)
{
	LoadPackages();
	InitSystemModules();
	return true;
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


	float fElapsedTime = (float)(CGlobals::GetFrameRateController(FRC_RENDER)->FrameMove(fTime));

	LinearColor color = m_pRootScene->GetClearColor();
	CGlobals::GetRenderDevice()->SetClearColor(Color4f(color.r, color.g, color.b, color.a));
	CGlobals::GetRenderDevice()->SetClearDepth(1);
	CGlobals::GetRenderDevice()->SetClearStencil(0);
	CGlobals::GetRenderDevice()->Clear(true, true, true);

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
	CGlobals::GetRenderDevice()->Present();
	FrameMove(fElapsedTime);
	return S_OK;
}

int CParaEngineApp::Run(HINSTANCE hInstance)
{
	return 0;
}

HRESULT ParaEngine::CParaEngineApp::DoWork()
{
	auto nStartTime = GetTickCount();

	if (GetAppState() != PEAppState_Exiting)
	{
		Render3DEnvironment();
		auto nCurTickCount = GetTickCount() - nStartTime;
		FrameMove(nCurTickCount / 1000.f);
		// 30FPS
		SLEEP(33);
	}
}

void ParaEngine::CParaEngineApp::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineApp::AppHasFocus()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void ParaEngine::CParaEngineApp::GetStats(string& output, DWORD dwFields)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineApp::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void ParaEngine::CParaEngineApp::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
{
	throw std::logic_error("The method or operation is not implemented.");
}

const char* ParaEngine::CParaEngineApp::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineApp::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
{
	throw std::logic_error("The method or operation is not implemented.");
}

DWORD ParaEngine::CParaEngineApp::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineApp::GetAutoLowerFrameRateWhenNotFocused()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void ParaEngine::CParaEngineApp::SetToggleSoundWhenNotFocused(bool bEnabled)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::CParaEngineApp::GetToggleSoundWhenNotFocused()
{
	throw std::logic_error("The method or operation is not implemented.");
}
