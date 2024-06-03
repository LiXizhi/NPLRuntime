//-----------------------------------------------------------------------------
// Class:	ParaEngineApp base
// Authors:	LiXizhi, big
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// CreateDate: 2015.1.1
// ModifyDate: 2022.1.5
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaEngineSettings.h"
#include "util/os_calls.h"
#include "util/StringHelper.h"
#include "ObjectAutoReleasePool.h"
#include "AttributesManager.h"
#include "PluginManager.h"
#include "MeshPhysicsObject.h"
#include "MeshObject.h"
#include "MissileObject.h"
#include "BipedObject.h"
#include "BMaxModel/BMaxObject.h"
#include "SkyMesh.h"
#include "BlockPieceParticle.h"
#include "ContainerObject.h"
#include "RenderTarget.h"
#include "WeatherEffect.h"
#include "OverlayObject.h"
#include "LightObject.h"
#include "NPLRuntime.h"
#include "EventsCenter.h"
#include "BootStrapper.h"
#include "NPL/NPLHelper.h"
#include "AISimulator.h"
#include "AsyncLoader.h"
#include "FileManager.h"
#include "Archive.h"
#include "ParaEngineAppBase.h"
#include "NPLPackageConfig.h"
#include "IO/ResourceEmbedded.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIMouseVirtual.h"
#include "FrameRateController.h"
#include "FileLogger.h"
#include "Render/IRenderContext.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICConfigManager.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "terrain/Terrain.h"
#include "ViewportManager.h"
#include "MoviePlatform.h"
#include "Framework/Common/Time/ParaTimer.h"
#include "EnvironmentSim.h"
#include "AutoCamera.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockWorldManager.h"
#include "ObjectManager.h"
#include "2dengine/GUIHighlight.h"
#include "3dengine/AudioEngine2.h"
#include "PluginAPI.h"
#include "Core/EventsCenter.h"
#include "Core/EventClasses.h"
#include "AutoRigger.h"
#include "ScriptParticle.h"

#if USE_DIRECTX_RENDERER
//#include "Render/context/d3d9/RenderContextD3D9.h"
#include "RenderDeviceD3D9.h"
#include "DirectXEngine.h"
#endif

#include "CoroutineThread.h"

using namespace ParaEngine;
using namespace ParaInfoCenter;
IParaEngineApp* CParaEngineAppBase::g_pCurrentApp = NULL;

extern ClassDescriptor* AudioEngine_GetClassDesc();

namespace ParaEngine
{
	/// this value changes from 0 to 1, and back to 0 in one second.
	float g_flash = 0.f;
}

/// default bootstrapper file
#define NPL_CODE_WIKI_BOOTFILE "script/apps/WebServer/WebServer.lua"

ParaEngine::CParaEngineAppBase::CParaEngineAppBase()
	: m_bEnable3DRendering(true)
	, m_isTouching(false)
	, m_nReturnCode(0)
	, m_pSingletonReleasePool(NULL)
	, m_nAppState(PEAppState_None)
	, m_hasClosingRequest(false)
	, m_Timer(nullptr)
	, m_pRenderWindow(nullptr)
	, m_pRenderContext(nullptr)
	, m_pRenderDevice(nullptr)
	, m_doWorkFRC(CFrameRateController::FRC_CONSTANT_OR_BELOW)
	, m_bActive(true), m_fFPS(0.f), m_fTime(0.f), m_fElapsedTime(0.f), m_fRefreshTimerInterval(-1.f), m_nFrameRateControl(0)
    , m_bRender(true)
{
	g_pCurrentApp = this;
	// we use high resolution timer (boost ASIO internally), hence FPS can be specified very accurately without eating all CPUs. 
	SetRefreshTimer(1 / 60.f, 0);
}

ParaEngine::CParaEngineAppBase::CParaEngineAppBase(const char* sCmd)
	: CCommandLineParams(sCmd)
	, m_bEnable3DRendering(true)
	, m_isTouching(false)
	, m_nReturnCode(0)
	, m_pSingletonReleasePool(NULL)
	, m_nAppState(PEAppState_None)
	, m_hasClosingRequest(false)
	, m_Timer(nullptr)
	, m_pRenderWindow(nullptr)
	, m_pRenderContext(nullptr)
	, m_pRenderDevice(nullptr)
	, m_doWorkFRC(CFrameRateController::FRC_CONSTANT_OR_BELOW)
	, m_bActive(true), m_fFPS(0.f), m_fTime(0.f), m_fElapsedTime(0.f), m_fRefreshTimerInterval(-1.f), m_nFrameRateControl(0)
    , m_bRender(true)
{
	g_pCurrentApp = this;
	// we use high resolution timer (boost ASIO internally), hence FPS can be specified very accurately without eating all CPUs. 
	SetRefreshTimer(1 / 60.f, 0);
}

ParaEngine::CParaEngineAppBase::~CParaEngineAppBase()
{
	StopApp();
	DestroySingletons();
	g_pCurrentApp = nullptr;
}

IParaEngineApp* ParaEngine::CParaEngineAppBase::GetInstance()
{
	return g_pCurrentApp;
}

IRenderWindow* ParaEngine::CParaEngineAppBase::GetRenderWindow()
{
	return m_pRenderWindow;
}

bool ParaEngine::CParaEngineAppBase::InitApp(IRenderWindow* pWindow, const char* sCommandLine)
{
	SetAppState(PEAppState_Device_Created);

	std::string cmd;
	VerifyCommandLine(sCommandLine, cmd);
	SetAppCommandLine(cmd.c_str());

	m_pRenderWindow = pWindow;

	InitCommandLineParams();

	CStaticInitRes::StaticInit();

	srand((unsigned int)time(NULL));

	FindParaEngineDirectory();

	RegisterObjectClasses();

	CFrameRateController::LoadFRCNormal();

	// loading packages
	LoadPackages();

	BootStrapAndLoadConfig();

	InitSystemModules();

	// load config file
	CICConfigManager *cm = CGlobals::GetICConfigManager();
	cm->LoadFromFile();

	// init audio engine
#ifdef STATIC_PLUGIN_CAUDIOENGINE
	IParaAudioEngine* pAudioEnginePlugin = (IParaAudioEngine*)AudioEngine_GetClassDesc()->Create();

	if (CAudioEngine2::GetInstance()->InitAudioEngine(pAudioEnginePlugin) != S_OK)
	{
		OUTPUT_LOG("Audio engine init fail!(audio engine plugin)\n");
		CAudioEngine2::GetInstance()->CleanupAudioEngine();
        m_bAudioEngineInitialized = false;
    }else{
        m_bAudioEngineInitialized = true;
    }
#else
	if (CAudioEngine2::GetInstance()->InitAudioEngine() != S_OK)
	{
		OUTPUT_LOG("Audio engine init fail!\n");
		CAudioEngine2::GetInstance()->CleanupAudioEngine();
        m_bAudioEngineInitialized = false;
    }else{
        m_bAudioEngineInitialized = true;
    }
#endif

	//----------------------------------------------------------
	/// Create a blank root scene with certain dimensions
	/// units is Meter.
	//----------------------------------------------------------
	m_pRootScene->SetBoundRect(1000.f, 1000.f, 0); // a very large scene
	m_pRootScene->SetMyType(_Scene);
	m_pRootScene->GetSceneState()->pAssetManager = m_pParaWorldAsset.get();
	m_pRootScene->GetSceneState()->CleanupSceneState();
	m_pRootScene->GetSceneState()->pGUIState = &(m_pGUIRoot->GetGUIState());

	/// create the default system font, the game should also use this sys font to save resources
	{
		// Load font mapping
		string value0, value1;
		DWORD nSize = 0;
		HRESULT hr;
		hr = cm->GetSize("GUI_font_mapping", &nSize);
		if (hr == E_INVALIDARG || hr == E_ACCESSDENIED) {
			//error
		}
		else {
			for (int i = 0; i < (int)nSize; i += 2) {
				if (FAILED(cm->GetTextValue("GUI_font_mapping", value0, i))) {
					break;
				}
				if (FAILED(cm->GetTextValue("GUI_font_mapping", value1, i + 1))) {
					break;
				}
				SpriteFontEntity::AddFontName(value0, value1);
			}
		}
	}

	SpriteFontEntity* pFont = NULL;
	pFont = m_pParaWorldAsset->LoadGDIFont("sys", "System", 12);

	/// set up terrain engine parameters
	ParaTerrain::Settings::GetInstance()->SetVerbose(false);
	/// enable editor mode to make device lost recoverable
	ParaTerrain::Settings::GetInstance()->SetEditor(true);
#ifdef _USE_NORMAL_
	ParaTerrain::Settings::GetInstance()->SetUseNormals(true);
#endif
	/// perform GUI static initialization
	/// perform CGUIRoot initialization
	m_pGUIRoot->OneTimeGUIInit();

    //----------------------------------------------------------
	/// Create ocean manager
    //----------------------------------------------------------
	CGlobals::GetOceanManager()->create();

    /// Load default mapping at the program start
	CGlobals::GetSettings().LoadGameEffectSet(0);

	/// Init Timer
	m_Timer = new ParaTimer();
	m_Timer->Start();

    InitRenderEnvironment();

	SetAppState(PEAppState_Ready);
	return true;
}

void ParaEngine::CParaEngineAppBase::InitRenderEnvironment()
{

	/// Create RenderDevice
	m_pRenderContext = IRenderContext::Create();
	RenderConfiguration cfg;
	cfg.renderWindow = m_pRenderWindow;
	m_pRenderDevice = m_pRenderContext->CreateDevice(cfg);
	CGlobals::SetRenderDevice(m_pRenderDevice);

	ParaViewport vp;
	vp.X = 0; vp.Y = 0;
	vp.Width = m_pRenderWindow->GetWidth();
	vp.Height = m_pRenderWindow->GetHeight();
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	m_pRenderDevice->SetViewport(vp);
	
	InitDeviceObjects();
	RestoreDeviceObjects();
}

void ParaEngine::CParaEngineAppBase::ResetRenderEnvironment()
{
	InvalidateDeviceObjects();

	RenderConfiguration cfg;
	cfg.renderWindow = m_pRenderWindow;
	if (!m_pRenderContext->ResetDevice(m_pRenderDevice, cfg))
	{
		OUTPUT_LOG("reset d3d device failed because Reset function failed\n");
		return;
	}

	/// Initialize the app's device-dependent objects
	RestoreDeviceObjects();
}

float ParaEngine::CParaEngineAppBase::GetFPS()
{
	return m_fFPS;
}

void ParaEngine::CParaEngineAppBase::UpdateFrameStats(double fTime)
{
	/// Keep track of the frame count
	static double fLastTime = 0.0f;
	static DWORD dwFrames = 0;
	++dwFrames;

	/// Update the scene stats once per second
	if (fTime - fLastTime > 1.0f)
	{
		m_fFPS = (float)(dwFrames / (fTime - fLastTime));
		fLastTime = fTime;
		dwFrames = 0;
    }
}

HRESULT ParaEngine::CParaEngineAppBase::InitDeviceObjects()
{
#if USE_DIRECTX_RENDERER
	/// stage b.1
	auto d3d9RenderDevice = static_cast<RenderDeviceD3D9*>(m_pRenderDevice);
	CGlobals::GetDirectXEngine().InitDeviceObjects(d3d9RenderDevice->GetContext(), d3d9RenderDevice->GetDirect3DDevice9(), NULL);
#endif
	/// Asset must be the first to be initialized. Otherwise, the global device object will not be valid
	m_pParaWorldAsset->InitDeviceObjects();
	m_pRootScene->InitDeviceObjects();
	m_pGUIRoot->InitDeviceObjects();

	return S_OK;
}

HRESULT ParaEngine::CParaEngineAppBase::DeleteDeviceObjects()
{
	m_pRootScene->DeleteDeviceObjects();
	m_pGUIRoot->DeleteDeviceObjects();
	m_pParaWorldAsset->DeleteDeviceObjects();
#if USE_DIRECTX_RENDERER
	CGlobals::GetDirectXEngine().DeleteDeviceObjects();
#endif

	return S_OK;
}

int ParaEngine::CParaEngineAppBase::CalculateRenderTime(double* pNextInterval)
{
	double fIdealInterval = GetRefreshTimer();
	double fCurTime = ParaTimer::GetAbsoluteTime();
	double fNextInterval = 0.f;
	int nUpdateFrameDelta = 0;
	int nFrameDelta = 1;

	const bool USE_ADAPTIVE_INTERVAL = true;
	if (USE_ADAPTIVE_INTERVAL)
	{
		/// adaptive interval algorithm
		/// check FPS by modifying the interval adaptively until FPS is within a good range.
		/// we will adjust every 1 second
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

		if ((fSeconds + 1) < fCurTime)
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

		/// tricky: run the main_loop as fast as possible at least 100FPS, so that FPS is more accurate.
		fAdaptiveInterval = Math::Min(fAdaptiveInterval, 1 / 100.0);

		fNextInterval = fAdaptiveInterval;

		if ((fCurTime - fLastTime) > 1.f)
		{
			/// too laggy
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
		/// fixed interval algorithm
		/// continue with next activation.
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

/// define to output interval to log file to change timer implementation.
//#define DEBUG_TIMER_INTERVAL
#ifdef DEBUG_TIMER_INTERVAL
	{
		/// debug timer
		static double fLastTime = fCurTime;
		static double fSeconds = fCurTime;
		static int nFPS = 0;
		nFPS++;

		if ((fSeconds + 1) < fCurTime)
		{
			fSeconds = fCurTime;
			OUTPUT_LOG("%d Second(FPS: %d)\n", (int)(fSeconds), nFPS);
			nFPS = 0;
		}

		OUTPUT_LOG("Tick: delta:%d, global:%d, next:%d\n",
                   (int)((fCurTime - fLastTime) * 1000),
                   (int)(fCurTime * 1000),
                   (int)(fNextInterval * 1000));

        fLastTime = fCurTime;
	}
#endif	
	if (pNextInterval)
		*pNextInterval = fNextInterval;

	return nFrameDelta;
}

double ParaEngine::CParaEngineAppBase::GetAppTime()
{
	return m_fTime;
}

double ParaEngine::CParaEngineAppBase::GetElapsedTime()
{
	return m_fElapsedTime;
}

HRESULT ParaEngine::CParaEngineAppBase::RestoreDeviceObjects()
{
	int width = m_pRenderWindow->GetWidth();
	int height = m_pRenderWindow->GetHeight();

	/// Set up the camera's projection matrix
	float aspectRatio = width / (float)height;
	m_pRootScene->RestoreDeviceObjects();
	m_pParaWorldAsset->RestoreDeviceObjects();
	m_pGUIRoot->RestoreDeviceObjects(width, height); /// GUI: 2D engine
	ParaTerrain::Settings::GetInstance()->SetScreenWidth(width);
	ParaTerrain::Settings::GetInstance()->SetScreenHeight(height);

#if USE_DIRECTX_RENDERER
	CGlobals::GetDirectXEngine().RestoreDeviceObjects();
#endif

	return S_OK;
}

HRESULT ParaEngine::CParaEngineAppBase::InvalidateDeviceObjects()
{
	m_pRootScene->InvalidateDeviceObjects();
	m_pParaWorldAsset->InvalidateDeviceObjects();
	m_pGUIRoot->InvalidateDeviceObjects(); /// GUI: 2D engine
#if USE_DIRECTX_RENDERER
	CGlobals::GetDirectXEngine().InvalidateDeviceObjects();
#endif

	return S_OK;
}

void ParaEngine::CParaEngineAppBase::Render()
{
	double fTime = m_fTime;
	UpdateFrameStats(fTime);
	CMoviePlatform* pMoviePlatform = CGlobals::GetMoviePlatform();
	pMoviePlatform->BeginCaptureFrame();

	float fElapsedTime = (float)(CGlobals::GetFrameRateController(FRC_RENDER)->FrameMove(fTime));
	m_fElapsedTime = fElapsedTime;

	RenderDevice::ClearAllPerfCount();

	m_pRenderDevice->BeginScene();
	{
		CGlobals::GetAssetManager()->RenderFrameMove(fElapsedTime);
#if USE_DIRECTX_RENDERER
        /// force setting render target to back buffer. and
		GETD3D(m_pRenderDevice)->SetRenderTarget(0, CGlobals::GetDirectXEngine().GetRenderTarget(0));
#endif
		auto color = m_pRootScene->GetClearColor();
		auto pDevice = CGlobals::GetRenderDevice();

		/// NOTE: on android devices will ignore all gl calls after the last draw call, so we need to restore everything to default settings
		pDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pDevice->SetRenderState(ERenderState::ZENABLE, TRUE);
		pDevice->SetRenderState(ERenderState::ZFUNC, D3DCMP_LESSEQUAL);
		pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
		pDevice->SetClearColor(Color4f(color.r, color.g, color.b, color.a));
		pDevice->SetClearDepth(1.0f);
		pDevice->SetClearStencil(0);
		pDevice->Clear(true, true, true);
		m_pViewportManager->UpdateViewport(m_pRenderWindow->GetWidth(), m_pRenderWindow->GetHeight());
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

	m_pRenderDevice->EndScene();
	pMoviePlatform->EndCaptureFrame();
}

void ParaEngine::CParaEngineAppBase::HandleUserInput()
{
	/// escape input if app does not have focus
	if (!IsAppActive())
		return;

	/// handle 2D GUI input: dispatch mouse and key event for gui objects.
	m_pGUIRoot->HandleUserInput();

	/// handle the camera user input. One can also block camera input and handle everything from script.
	CAutoCamera* pCamera = ((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()));
	if (pCamera)
		pCamera->HandleUserInput();

	/// handle 3D scene input
	CGlobals::GetScene()->HandleUserInput();
}

void ParaEngine::CParaEngineAppBase::ActivateApp(bool bActivate)
{
	m_bActive = bActivate;
}

bool ParaEngine::CParaEngineAppBase::FrameMove(double fTime)
{
	m_fTime = fTime;
	if (GetAppState() == PEAppState_Stopped)
		return true;

	double fElapsedGameTime = CGlobals::GetFrameRateController(FRC_GAME)->FrameMove(fTime);
	PERF_BEGIN("Main FrameMove");
	double fElapsedEnvSimTime = CGlobals::GetFrameRateController(FRC_SIM)->FrameMove(fTime);
	if (fElapsedEnvSimTime > 0)
	{
		PERF_BEGIN("Script&Net FrameMove");
		CAISimulator::GetSingleton()->FrameMove((float)fElapsedEnvSimTime);
		PERF_END("Script&Net FrameMove");

        PERF_BEGIN("EnvironmentSim");
		CGlobals::GetEnvSim()->Animate((float)fElapsedEnvSimTime);  // generate valid LLE from HLE
		PERF_END("EnvironmentSim");

		if(m_bAudioEngineInitialized)
			CAudioEngine2::GetInstance()->Update();

		{
			/// animate g_flash value for some beeper effect, such as object selection.
			static float s_flash = 0.f;
			s_flash += (float)fElapsedEnvSimTime * 2;
			if (s_flash > 2)
				s_flash = 0;
			if (s_flash > 1)
				g_flash = 2 - s_flash;
			else
				g_flash = s_flash;
		}
	}

	double fElapsedIOTime = CGlobals::GetFrameRateController(FRC_IO)->FrameMove(fTime);
	//OUTPUT_LOG("curTime %f: elapsedIO: %f, simtTime: %f\n", fTime, fElapsedIOTime, fElapsedEnvSimTime);
	if (fElapsedIOTime > 0)
	{
		HandleUserInput();
	}

	if (fElapsedEnvSimTime > 0 || fElapsedIOTime > 0)
	{
		m_pRootScene->Animate(fElapsedEnvSimTime > 0 ? (float)fElapsedEnvSimTime : 0.f);
	}

#ifdef USE_XACT_AUDIO_ENGINE
	/// for audio engine
	if (m_pAudioEngine && m_pAudioEngine->IsAudioEngineEnabled())
	{
		if (m_pAudioEngine->IsValid())
		{
			PERF1("Audio Engine Framemove");
			m_pAudioEngine->DoWork();
		}
	}
#endif

	PERF_END("Main FrameMove");
	OnFrameEnded();
	return true;
}

void ParaEngine::CParaEngineAppBase::OnPause()
{
	//InvalidateDeviceObjects();
	ActivateApp(false);
    setRenderEnabled(false);

	SystemEvent e(SystemEvent::SYS_PAUSE, "");
	CGlobals::GetEventsCenter()->FireEvent(e);
}

void ParaEngine::CParaEngineAppBase::OnResume()
{
	ActivateApp(true);
    setRenderEnabled(true);

	SystemEvent e(SystemEvent::SYS_RESUME, "");
	CGlobals::GetEventsCenter()->FireEvent(e);
}

void ParaEngine::CParaEngineAppBase::OnRendererRecreated(IRenderWindow * renderWindow)
{
	ActivateApp(false);
	m_pRenderWindow = renderWindow;
	m_pRenderContext = IRenderContext::Create();

	RenderConfiguration cfg;
	cfg.renderWindow = m_pRenderWindow;
	m_pRenderDevice = m_pRenderContext->CreateDevice(cfg);
	CGlobals::SetRenderDevice(m_pRenderDevice);

	ParaViewport vp;
	vp.X = 0; vp.Y = 0;
	vp.Width = m_pRenderWindow->GetWidth();
	vp.Height = m_pRenderWindow->GetHeight();
	vp.MinZ = 0;
	vp.MaxZ = 1.0f;
	m_pRenderDevice->SetViewport(vp);

	m_pParaWorldAsset->RendererRecreated();
	m_pRootScene->RendererRecreated();

	SystemEvent e(SystemEvent::SYS_RENDERER_RECREATED, "");
	//e.SetAsyncMode(false);
	CGlobals::GetEventsCenter()->FireEvent(e);
	ActivateApp(true);
}

void ParaEngine::CParaEngineAppBase::OnRendererDestroyed()
{
	//InvalidateDeviceObjects();
	//m_pParaWorldAsset->RendererRecreated();
	//m_pRootScene->RendererRecreated();

	if (m_pRenderDevice)
	{
		delete m_pRenderDevice;
		m_pRenderDevice = nullptr;
		CGlobals::SetRenderDevice(nullptr);
	}

	if (m_pRenderContext)
	{
		delete m_pRenderContext;
		m_pRenderContext = nullptr;
	}

	if (m_pRenderWindow)
	{
		delete m_pRenderWindow;
		m_pRenderWindow = nullptr;
	}
}

bool ParaEngine::CParaEngineAppBase::StartApp()
{
	return true;
}

void ParaEngine::CParaEngineAppBase::StopApp()
{
	/// if it is already stopped, we shall return
	if (!m_pParaWorldAsset)
		return;

	FinalCleanup();
	m_pParaWorldAsset.reset();
	m_pRootScene.reset();
	m_pGUIRoot.reset();
	m_pViewportManager.reset();
	m_pGUIRoot.reset();
#ifdef WIN32
	//#ifdef LOG_FILES_ACTIVITY
	if (CFileLogger::GetInstance()->IsBegin())
	{
		CFileLogger::GetInstance()->EndFileLog();
		CFileLogger::GetInstance()->SaveLogToFile("temp/filelog.txt");
	}
#endif
	//#endif
#ifdef EXTRACT_INSTALL_FILE
	CFileLogger::GetInstance()->MirrorFiles("_InstallFiles/");
#endif

	//Gdiplus::GdiplusShutdown(g_gdiplusToken);

	/// delete all singletons
	DestroySingletons();
}

void ParaEngine::CParaEngineAppBase::DestroySingletons()
{
	CObjectAutoReleasePool::DestoryInstance();
	SAFE_DELETE(m_pSingletonReleasePool);
}

void ParaEngine::CParaEngineAppBase::OnFrameEnded()
{
	CObjectAutoReleasePool::GetInstance()->clear();
}

ParaEngine::IAttributeFields* ParaEngine::CParaEngineAppBase::GetAttributeObject()
{
	return &ParaEngineSettings::GetSingleton();
}

bool ParaEngine::CParaEngineAppBase::InitCommandLineParams()
{
	const char* sWritablePath = GetAppCommandLineByParam("writablepath", NULL);
	if (sWritablePath && sWritablePath[0] != 0) {
		CParaFile::SetWritablePath(sWritablePath);
	}

	const char* nAssetLogLevel = GetAppCommandLineByParam("assetlog_level", NULL);
	if (nAssetLogLevel && nAssetLogLevel[0] != 0) 
	{
		int nLevel = 0;
		if (strcmp(nAssetLogLevel, "all") == 0)
			nLevel = ParaEngine::CAsyncLoader::Log_All;
		else if (strcmp(nAssetLogLevel, "remote") == 0)
			nLevel = ParaEngine::CAsyncLoader::Log_Remote;
		else if (strcmp(nAssetLogLevel, "error") == 0)
			nLevel = ParaEngine::CAsyncLoader::Log_Error;
		ParaEngine::CAsyncLoader::GetSingleton().SetLogLevel(0);
	}

	const char* sLogFile = GetAppCommandLineByParam("logfile", NULL);
	if (sLogFile && sLogFile[0] != 0){
		CLogger::GetSingleton().SetLogFile(sLogFile);
	}

	const char* sServerMode = GetAppCommandLineByParam("servermode", NULL);
	const char* sInteractiveMode = GetAppCommandLineByParam("i", NULL);
	bool bIsServerMode = (sServerMode && strcmp(sServerMode, "true") == 0);
	bool bIsInterpreterMode = (sInteractiveMode && strcmp(sInteractiveMode, "true") == 0);
	Enable3DRendering(!bIsServerMode && !bIsInterpreterMode);

	const char* sDevFolder = GetAppCommandLineByParam("dev", NULL);
	if (sDevFolder)
	{
		if (sDevFolder[0] == '\0' || (sDevFolder[1] == '\0' && (sDevFolder[0] == '/' || sDevFolder[0] == '.')))
		{
			sDevFolder = CParaFile::GetCurDirectory(0).c_str();
		}
		CParaFile::SetDevDirectory(sDevFolder);
	}

	return true;
}

void ParaEngine::CParaEngineAppBase::setIMEKeyboardState(bool bOpen, bool bMoveView, int ctrlBottom, const string& editParams)
{
}

bool ParaEngine::CParaEngineAppBase::ForceRender()
{
	if (!getRenderEnabled())
		return false;

	auto pRenderDevice = m_pRenderDevice;

#ifndef ONLY_FORCERENDER_GUI 
	Render();
	pRenderDevice->Flush();

	return true;
#else
	if (pRenderDevice->BeginScene())
	{
#if USE_DIRECTX_RENDERER
		GETD3D(pRenderDevice)->SetRenderTarget(0, CGlobals::GetDirectXEngine().GetRenderTarget(0)); // force setting render target to back buffer. and
#endif

		auto color = m_pRootScene->GetClearColor();

		// NOTE: on android devices will ignore all gl calls after the last draw call, so we need to restore everything to default settings
		pRenderDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pRenderDevice->SetRenderState(ERenderState::ZENABLE, TRUE);
		pRenderDevice->SetRenderState(ERenderState::ZFUNC, D3DCMP_LESSEQUAL);
		pRenderDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);

		pRenderDevice->SetClearColor(Color4f(color.r, color.g, color.b, color.a));
		pRenderDevice->SetClearDepth(1.0f);
		pRenderDevice->SetClearStencil(0);
		pRenderDevice->Clear(true, true, true);

		CGlobals::GetGUI()->AdvanceGUI(0);

		pRenderDevice->EndScene();

		//bSucceed = pRenderDevice->Present();
		pRenderDevice->Flush();
		return true;
	}

	return false;
#endif 
}

const char * ParaEngine::CParaEngineAppBase::GetModuleDir() 
{ 
	return m_sModuleDir.c_str(); 
}

ParaEngine::CViewportManager * ParaEngine::CParaEngineAppBase::GetViewportManager()
{
	return m_pViewportManager.get();
}

/// use RegisterObjectFactory instead
void ParaEngine::CParaEngineAppBase::RegisterObjectClass(IAttributeFields* pObject)
{
	if (pObject)
	{
		ref_ptr<IAttributeFields> obj(pObject);
		pObject->GetAttributeClass();
	}
}

void ParaEngine::CParaEngineAppBase::RegisterObjectClasses()
{
	CAttributesManager* pAttManager = CGlobals::GetAttributesManager();
	pAttManager->RegisterObjectFactory("CWeatherEffect", new CDefaultObjectFactory<CWeatherEffect>());
	pAttManager->RegisterObjectFactory("CMeshPhysicsObject", new CDefaultObjectFactory<CMeshPhysicsObject>());
	pAttManager->RegisterObjectFactory("CMeshObject", new CDefaultObjectFactory<CMeshObject>());
	pAttManager->RegisterObjectFactory("BlockPieceParticle", new CDefaultObjectFactory<CBlockPieceParticle>());
	pAttManager->RegisterObjectFactory("CContainerObject", new CDefaultObjectFactory<CContainerObject>());
	pAttManager->RegisterObjectFactory("CRenderTarget", new CDefaultObjectFactory<CRenderTarget>());
	pAttManager->RegisterObjectFactory("CMissileObject", new CDefaultObjectFactory<CMissileObject>());
	pAttManager->RegisterObjectFactory("BMaxObject", new CDefaultObjectFactory<BMaxObject>());
	pAttManager->RegisterObjectFactory("CSkyMesh", new CDefaultObjectFactory<CSkyMesh>());
	pAttManager->RegisterObjectFactory("COverlayObject", new CDefaultObjectFactory<COverlayObject>());
	pAttManager->RegisterObjectFactory("CLightObject", new CDefaultObjectFactory<CLightObject>());
	pAttManager->RegisterObjectFactory("CAutoRigger", new CDefaultObjectFactory<CAutoRigger>());
	pAttManager->RegisterObjectFactory("CScriptParticle", new CDefaultObjectFactory<CScriptParticle>());
	/// TODO add more here:
}

bool ParaEngine::CParaEngineAppBase::Is3DRenderingEnabled()
{
	return m_bEnable3DRendering;
}

void ParaEngine::CParaEngineAppBase::Enable3DRendering(bool val)
{
	m_bEnable3DRendering = val;
}

bool ParaEngine::CParaEngineAppBase::IsTouchInputting()
{
	return m_isTouching;
}

bool ParaEngine::CParaEngineAppBase::IsSlateMode()
{
	return false;
}

HRESULT ParaEngine::CParaEngineAppBase::DoWork()
{
#ifdef EMSCRIPTEN_SINGLE_THREAD
	CoroutineThread::Tick();
#endif

	m_fTime = m_Timer->GetAppTime();
	if (m_pRenderWindow == nullptr)
		return S_FALSE;
	
	auto d = m_doWorkFRC.FrameMove(m_fTime);

	if (d > 0)
	{
		FrameMove(m_fTime);
        if (getRenderEnabled())
        {
			Render();

			if (!CGlobals::GetViewportManager()->GetIsXR())
			{
				m_pRenderDevice->Present();
			}
        }
		
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
	
}

void ParaEngine::CParaEngineAppBase::SetTouchInputting(bool bTouchInputting)
{
	m_isTouching = bTouchInputting;
}

bool ParaEngine::CParaEngineAppBase::IsAppActive()
{
	return m_bActive;
}

void ParaEngine::CParaEngineAppBase::GameToClient(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
}

void ParaEngine::CParaEngineAppBase::ClientToGame(int& inout_x, int & inout_y, bool bInBackbuffer /*= true*/)
{
}

bool ParaEngine::CParaEngineAppBase::AppHasFocus()
{
	return true;
}

void ParaEngine::CParaEngineAppBase::GetStats(string& output, DWORD dwFields)
{
}

bool ParaEngine::CParaEngineAppBase::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
{
	return false;
}

void ParaEngine::CParaEngineAppBase::SetAutoLowerFrameRateWhenNotFocused(bool bEnabled)
{
}

const char* ParaEngine::CParaEngineAppBase::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
{
	return NULL;
}

bool ParaEngine::CParaEngineAppBase::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
{
	return false;
}

DWORD ParaEngine::CParaEngineAppBase::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
{
	return 0;
}

bool ParaEngine::CParaEngineAppBase::GetAutoLowerFrameRateWhenNotFocused()
{
	return false;
}

void ParaEngine::CParaEngineAppBase::SetToggleSoundWhenNotFocused(bool bEnabled)
{
}

bool ParaEngine::CParaEngineAppBase::GetToggleSoundWhenNotFocused()
{
	return false;
}

void ParaEngine::CParaEngineAppBase::onCmdLine(const std::string& cmd)
{
	OUTPUT_LOG("onCmdLine: %s", cmd.c_str());

	if (!cmd.empty())
	{
		/// msg = command line.
		string msg = "msg=";
		NPL::NPLHelper::EncodeStringInQuotation(msg, (int)msg.size(), cmd.c_str());
		msg.append(";");
		SystemEvent event(SystemEvent::SYS_COMMANDLINE, msg);
		CGlobals::GetEventsCenter()->FireEvent(event);
	}
}

void ParaEngine::CParaEngineAppBase::SetRefreshTimer(float fTimeInterval, int nFrameRateControl /*= 0*/)
{
	if (nFrameRateControl == 1)
	{
		CFrameRateController::LoadFRCNormal(fTimeInterval);
		m_doWorkFRC.SetType(CFrameRateController::FRC_CONSTANT_OR_BELOW);
	}
	else
	{
		CFrameRateController::LoadFRCRealtime(fTimeInterval);
		m_doWorkFRC.SetType(CFrameRateController::FRC_NONE);
	}

	m_fRefreshTimerInterval = fTimeInterval;
	m_nFrameRateControl = nFrameRateControl;
	m_doWorkFRC.m_fConstDeltaTime = (m_fRefreshTimerInterval <= 0.f) ? IDEAL_FRAME_RATE : m_fRefreshTimerInterval;
}

float ParaEngine::CParaEngineAppBase::GetRefreshTimer()
{
	return m_fRefreshTimerInterval;
}

DWORD ParaEngine::CParaEngineAppBase::GetCoreUsage()
{
	return PE_USAGE_STANDALONE | PE_USAGE_OWN_D3D | PE_USAGE_OWN_WINDOW;
}

void ParaEngine::CParaEngineAppBase::SystemMessageBox(const std::string& msg)
{
	OUTPUT_LOG("System Message: %s \n", msg.c_str());
#ifdef WIN32
	//::MessageBox(CGlobals::GetAppHWND(), msg.c_str(), "System Message", MB_OK);
#endif
}

void ParaEngine::CParaEngineAppBase::SetAppCommandLine(const char* pCommandLine)
{
	CCommandLineParams::SetAppCommandLine(pCommandLine);
	InitCommandLineParams();

	static bool bFirstCall = true;
	if (bFirstCall)
	{
		bFirstCall = false;
		OUTPUT_LOG1("NPL Runtime started\n");
		OUTPUT_LOG("NPL bin dir: %s\n", m_sModuleDir.c_str());
		if (m_sPackagesDir.empty()) {
			OUTPUT_LOG("no packages at: %s\n", m_sPackagesDir.c_str());
		}
		OUTPUT_LOG("WorkingDir: %s\n", m_sInitialWorkingDir.c_str());
		OUTPUT_LOG("WritablePath: %s\n", CParaFile::GetWritablePath().c_str());
	}
}

const char* ParaEngine::CParaEngineAppBase::GetAppCommandLine()
{
	return CCommandLineParams::GetAppCommandLine();
}

const char* ParaEngine::CParaEngineAppBase::GetAppCommandLineByParam(const char* pParam, const char* defaultValue)
{
	return CCommandLineParams::GetAppCommandLineByParam(pParam, defaultValue);
}

void ParaEngine::CParaEngineAppBase::Exit(int nReturnCode /*= 0*/)
{
	OUTPUT_LOG("program exited with code %d\n", nReturnCode);
	SetReturnCode(nReturnCode);
	SetAppState(PEAppState_Exiting);
}

ParaEngine::PEAppState ParaEngine::CParaEngineAppBase::GetAppState()
{
	return m_nAppState;
}

void ParaEngine::CParaEngineAppBase::SetAppState(ParaEngine::PEAppState state)
{
	m_nAppState = state;
}

bool ParaEngine::CParaEngineAppBase::FinalCleanup()
{
	CBlockWorldManager::GetSingleton()->Cleanup();

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
	// AI simulator should be cleaned up prior to NPL runtime. 
	CGlobals::GetAISim()->CleanUp();
	CGlobals::GetNPLRuntime()->Cleanup();
	

	CGlobals::GetMoviePlatform()->Cleanup();

    if (m_pGUIRoot.get())
        m_pGUIRoot->Release();		// GUI: 2D engine
    if (m_pRootScene.get())
        m_pRootScene->Cleanup();
	CSingleton<CObjectManager>::Instance().Finalize();
	CSingleton<CGUIHighlightManager>::Instance().Finalize();
    if (m_pParaWorldAsset.get())
        m_pParaWorldAsset->Cleanup();
	//Performance Monitor
	PERF_END("Program");
	PERF_REPORT();
	if(m_bAudioEngineInitialized)
		CAudioEngine2::GetInstance()->CleanupAudioEngine();
	return S_OK;
}

NPL::INPLRuntime* ParaEngine::CParaEngineAppBase::GetNPLRuntime()
{
	return CGlobals::GetNPLRuntime();
}

CRefCounted* ParaEngine::CParaEngineAppBase::AddToSingletonReleasePool(CRefCounted* pObject)
{
	if (!m_pSingletonReleasePool)
		m_pSingletonReleasePool = new CObjectAutoReleasePool();
	m_pSingletonReleasePool->AddObject(pObject);
	return pObject;
}

void ParaEngine::CParaEngineAppBase::VerifyCommandLine(const char* sCommandLine, std::string &strCmd)
{
	if (sCommandLine)
		strCmd = sCommandLine;

	if (strCmd.find("bootstrapper") == string::npos)
	{
		auto nPos = string::npos;
		if ((((nPos=strCmd.rfind(".npl")) != string::npos) || ((nPos = strCmd.rfind(".lua")) != string::npos)))
		{
			/// just in case, user has specified XXX.lua instead of bootstrapper=XXX.lua || .npl in the command line.
			/// following format are valid for specifying bootstrapper file.
			/// - [parameters] bootstrapper.lua
			/// - bootstrapper.lua [parameters]
			auto nFilenameFromPos = strCmd.rfind(" ", nPos);
			if (nFilenameFromPos != string::npos)
			{
				strCmd = strCmd.substr(0, nFilenameFromPos + 1) + std::string("bootstrapper=") +
					strCmd.substr(nFilenameFromPos + 1, nPos - nFilenameFromPos + 3) + strCmd.substr(nPos + 4);
			}
			else
			{
				strCmd = std::string("bootstrapper=") + strCmd.substr(0, nPos + 4) + strCmd.substr(nPos + 4);
			}
		}
		else
		{
			/// if no bootstrapper is specified, try to find the config.txt in current directory.
			CParaFile configFile;
			if (configFile.OpenFile("config.txt"))
			{
				std::string sCmdLine;
				configFile.GetNextAttribute("cmdline", sCmdLine);
				configFile.close();
				if (sCmdLine.find("bootstrapper") != string::npos)
				{
					strCmd = strCmd + " " + sCmdLine;
				}
			}
		}
	}
}

bool ParaEngine::CParaEngineAppBase::HasClosingRequest()
{
	return m_hasClosingRequest;
}

void ParaEngine::CParaEngineAppBase::SetHasClosingRequest(bool val)
{
	m_hasClosingRequest = val;
}

bool ParaEngine::CParaEngineAppBase::LoadNPLPackage(const char* sFilePath_, std::string * pOutMainFile)
{
	std::string sFilePath = sFilePath_;
	std::string sPKGDir;
	bool bHasOutputMainFile = false;
	CNPLPackageConfig config;
	if (sFilePath.size() > 0 && sFilePath[sFilePath.size() - 1] == '/')
	{
		std::string sDirName = sFilePath.substr(0, sFilePath.size() - 1);
		
		if (!CParaFile::GetDevDirectory().empty())
		{
			std::string sFullDir = CParaFile::GetAbsolutePath(sDirName, CParaFile::GetDevDirectory());
			if (CParaFile::DoesFileExist2(sFullDir.c_str(), FILE_ON_DISK))
			{
				sPKGDir = sFullDir;
			}
		}

		if (!sPKGDir.empty())
		{
			// found packages under dev folder
		}
		else 
		{
			std::string sFullDir;

			if ( CParaFile::IsAbsolutePath(sDirName) && CParaFile::DoesFileExist2(sDirName.c_str(), FILE_ON_DISK, &sFullDir))
			{
				sPKGDir = sFullDir;
			}
			else
			{
				sFullDir = CParaFile::GetAbsolutePath(sDirName, CParaFile::GetCurDirectory(0));
				if (CParaFile::DoesFileExist2(sFullDir.c_str(), FILE_ON_DISK))
				{
					sPKGDir = sFullDir;
				}
#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_ANDROID)
				else if( !CParaFile::IsAbsolutePath(sFullDir))
				{
					/// this fixed a special condition on android, when npl_packages are deployed to asset folder, where current directory is ""
					std::string packageFile = sFullDir + "/package.npl";
					if (CParaFile::DoesFileExist2(packageFile.c_str(), FILE_ON_DISK))
					{
						sPKGDir = sFullDir;
					}
				}
#endif

				if (strcmp("false", CCommandLineParams::GetAppCommandLineByParam("disable-parent-package-lookup", "false")) == 0)
				{
					if (sPKGDir.empty() && !m_sModuleDir.empty())
					{
						std::string workingDir = m_sModuleDir;
						// search for all parent directory for at most 5 levels
						for (int i = 0; i < 5 && !workingDir.empty(); ++i)
						{
							std::string sFullDir = CParaFile::GetAbsolutePath(sDirName, workingDir);
							if (CParaFile::DoesFileExist2(sFullDir.c_str(), FILE_ON_DISK))
							{
								sPKGDir = sFullDir;
								break;
							}
							else
							{
								workingDir = CParaFile::GetParentDirectoryFromPath(workingDir, 1);
							}
						}
					}
				}
			}
		}

		if (sPKGDir.empty())
		{
			/// if package folder is not found, we will search for zip and pkg file with the same name as the folder name.
			std::string pkgFile = sDirName + ".zip";

			CArchive* pArchive = nullptr;

			if (!CParaFile::GetDevDirectory().empty())
			{
				std::string sFullDir = CParaFile::GetAbsolutePath(pkgFile, CParaFile::GetDevDirectory());
				pArchive = CFileManager::GetInstance()->GetArchive(sFullDir);
				if (pArchive == nullptr)
				{
					if (CFileManager::GetInstance()->OpenArchive(sFullDir, false))
					{
						pArchive = CFileManager::GetInstance()->GetArchive(sFullDir);
					}
				}
			}

			if (pArchive == nullptr)
			{
				pArchive = CFileManager::GetInstance()->GetArchive(pkgFile);

				if (pArchive == nullptr)
				{
					if (CFileManager::GetInstance()->OpenArchive(pkgFile, false))
					{
						pArchive = CFileManager::GetInstance()->GetArchive(pkgFile);
					}
				}
			}

			if (pArchive != nullptr)
			{
				/// locate "package.npl" in root folder of zip file
				CParaFile file;
				if (file.OpenFile(pArchive, "package.npl"))
				{
					config.open(file.getBuffer(), file.getSize());
				}
				else
				{
					static CSearchResult result;
					result.InitSearch(sDirName + "/", 0, 1, 0);
					pArchive->FindFiles(result, "", "*/package.npl", 0);
					if (result.GetNumOfResult() > 0)
					{
						// "*/package" just in case we zipped data in a subfolder.
						CParaFile file;
						if (file.OpenFile(pArchive, result.GetItem(0).c_str()))
						{
							std::string sBaseDir = result.GetItem(0).substr(0, result.GetItem(0).size() - 12);
							pArchive->SetBaseDirectory(sBaseDir.c_str());
							config.open(file.getBuffer(), file.getSize());
						}
					}
				}
			}

			if (config.IsOpened())
			{
				sPKGDir = pkgFile;

				if (!config.IsSearchPath())
				{
					pArchive->SetRootDirectory(sFilePath);
					config.SetMainFile(CParaFile::GetAbsolutePath(config.GetMainFile(), sFilePath));
				}

				if (pOutMainFile)
					*pOutMainFile = config.GetMainFile();
			}
		}
	}

	if (!sPKGDir.empty())
	{
		if (!config.IsOpened())
		{
			/// disk file based package
			std::string packageFile = sPKGDir + "/package.npl";
			CParaFile file;
			if (file.OpenFile(packageFile.c_str(), true, 0, false, FILE_ON_DISK))
			{
				config.open(file.getBuffer(), file.getSize());
				/// output main file
				if (!config.IsSearchPath())
				{
					if (sFilePath.size() > 3 && sFilePath[0] == '.' && ((sFilePath[1] == '.' && sFilePath[2] == '/') || (sFilePath[1] == '/')))
					{
						/// use absolute path if folder begins with ../ or ./
						config.SetMainFile(CParaFile::GetAbsolutePath(config.GetMainFile(), sPKGDir));
					}
					else
					{
						/// relative to root path
						config.SetMainFile(CParaFile::GetAbsolutePath(config.GetMainFile(), sFilePath));
					}
				}
			}

			if (pOutMainFile)
				*pOutMainFile = config.GetMainFile();

			if (config.IsSearchPath())
				return CFileManager::GetInstance()->AddSearchPath(sPKGDir.c_str());
		}

		return true;
	}
	else
	{
		/// open pkg/zip
		if (CParaFile::DoesFileExist(sFilePath_))
		{
			if (CFileManager::GetInstance()->OpenArchive(sFilePath_))
			{
				return true;
			}
		}

	}

	return false;
}

void CParaEngineAppBase::AutoSetLocale()
{
	/// set locale according to current system language.
	const char* lang = "enUS";
	if (ParaEngineSettings::GetSingleton().GetCurrentLanguage() == LanguageType::CHINESE)
		lang = "zhCN";

	ParaEngineSettings::GetSingleton().SetLocale(lang);
}

void CParaEngineAppBase::WriteToLog(const char* zFormat, ...)
{
	va_list args;
	va_start(args, zFormat);
	ParaEngine::CLogger::GetSingleton().WriteFormatedVarList(zFormat, args);
	va_end(args);
}

void CParaEngineAppBase::AppLog(const char* sFormat)
{
	OUTPUT_LOG1("%s\n", sFormat);
}

void CParaEngineAppBase::SetReturnCode(int nReturnCode)
{
	m_nReturnCode = nReturnCode;
}

int CParaEngineAppBase::GetReturnCode()
{
	return m_nReturnCode;
}

void ParaEngine::CParaEngineAppBase::LoadPackagesInFolder(const std::string& sPkgFolder)
{
	/// we will load all packages that matches the following pattern in the order given by their name,
	/// such that "main_001.pkg" is always loaded before "main_002.pkg"
#define MAIN_PACKAGE_FILE_PATTERN	"main*.pkg"
	CSearchResult* result = CFileManager::GetInstance()->SearchFiles(sPkgFolder,
                                                                     MAIN_PACKAGE_FILE_PATTERN,
                                                                     "",
                                                                     0,
                                                                     1000,
                                                                     0);

	bool bIs64Bits = sizeof(void*) > 4;
	int nNum = 0;
	if (result != 0)
	{
		/// we will sort by file name
		std::vector<std::string> fileList;
		nNum = result->GetNumOfResult();
		for (int i = 0; i < nNum; ++i)
		{
			fileList.push_back(result->GetItem(i));
		}

		/// we will enqueue in reverse order, so that main_002 is pushed first, and then main_001
		std::sort(fileList.begin(), fileList.end(), std::greater<std::string>());

		if (fileList.size() == 0)
		{
			if (CParaFile::DoesFileExist("main.pkg") || CParaFile::DoesFileExist("main.zip"))
				fileList.push_back("main.zip");
			if (bIs64Bits && CParaFile::DoesFileExist("main_64bits.pkg"))
				fileList.push_back("main_64bits.pkg");
			else if (!bIs64Bits && CParaFile::DoesFileExist("main_32bits.pkg"))
				fileList.push_back("main_32bits.pkg");

			if (CParaFile::DoesFileExist("main_full.pkg"))
				fileList.push_back("main_full.pkg");
			if (bIs64Bits && CParaFile::DoesFileExist("main_full_64bits.pkg"))
				fileList.push_back("main_full_64bits.pkg");
			else if (!bIs64Bits && CParaFile::DoesFileExist("main_full_32bits.pkg"))
				fileList.push_back("main_full_32bits.pkg");
		}

		for (auto& filename : fileList)
		{
			int nSize = (int)filename.size();

			if (nSize > 11 && filename[nSize - 11] == '_' && filename[nSize - 8] == 'b')
			{
				if (filename[nSize - 10] == '3' && filename[nSize - 9] == '2' && bIs64Bits)
				{
					continue;
				}
				else if (filename[nSize - 10] == '6' && filename[nSize - 9] == '4' && !bIs64Bits)
				{
					continue;
				}
			}

			/// always load by relative path first, and then by absolute path.
			/// For example, when there is a package in current working directory, it will be used instead the one in packages/ folder.
			if (!CFileManager::GetInstance()->OpenArchive(filename))
			{
				if (!CFileManager::GetInstance()->OpenArchive(sPkgFolder + filename))
				{
					OUTPUT_LOG("failed loading package: %s%s\n", sPkgFolder.c_str(), filename.c_str());
				}
			}
		}
	}
}

bool ParaEngine::CParaEngineAppBase::FindBootStrapper()
{
	const char* pBootFileName = GetAppCommandLineByParam("bootstrapper", "");
	if (pBootFileName[0] == '\0' && ! CBootStrapper::GetSingleton()->IsEmpty())
	{
		/// may be the `loadpackage` params have set the bootstrapper if not explicitly specified by the application.
		pBootFileName = CBootStrapper::GetSingleton()->GetMainLoopFile().c_str();
		OUTPUT_LOG("try bootstrapper in loadpackage command: %s\n", pBootFileName);
	}

	bool bHasBootstrapper = CBootStrapper::GetSingleton()->LoadFromFile(pBootFileName);
	if (!bHasBootstrapper)
	{
		if (pBootFileName && pBootFileName[0] != '\0')
        {
			OUTPUT_LOG("error: can not find bootstrapper file at %s\n", pBootFileName);
		}

		pBootFileName = NPL_CODE_WIKI_BOOTFILE;
		bHasBootstrapper = CBootStrapper::GetSingleton()->LoadFromFile(pBootFileName);
		OUTPUT_LOG("We are using default bootstrapper at %s\n", pBootFileName);

		if (!bHasBootstrapper)
		{
			OUTPUT_LOG("However, we can not locate that file either. Have you installed npl_package/main? \n\n\nPlease install it at https://github.com/NPLPackages/main\n\n\n");
		}
	}

	// OUTPUT_LOG("cmd line: %s \n", GetAppCommandLine());
	OUTPUT_LOG("main loop: %s \n", CBootStrapper::GetSingleton()->GetMainLoopFile().c_str());
	return true;
}

void CParaEngineAppBase::LoadPackages()
{
	std::string sRootDir = CParaFile::GetCurDirectory(0);
	OUTPUT_LOG("ParaEngine Root Dir is %s\n", sRootDir.c_str());

    /// always load main package folder if exist
	std::string sOutputFile;
	if (LoadNPLPackage("npl_packages/main/", &sOutputFile))
	{
		if (!sOutputFile.empty())
			CGlobals::GetNPLRuntime()->GetMainRuntimeState()->Loadfile_async(sOutputFile.c_str());
	}
	
	/// load packages via command line
	const char* sPackages = GetAppCommandLineByParam("loadpackage", NULL);
	if (sPackages)
	{
		std::vector<std::string> listPackages;
		StringHelper::split(sPackages, ";", listPackages);
		for (const std::string& package : listPackages)
		{
			std::string sOutputFile;
			if (LoadNPLPackage(package.c_str(), &sOutputFile))
			{
				if (!sOutputFile.empty())
                    CGlobals::GetNPLRuntime()->GetMainRuntimeState()->Loadfile_async(sOutputFile.c_str());
			}
		}
	}

	LoadPackagesInFolder(sRootDir);
	if (m_sPackagesDir.empty())
		m_sPackagesDir = sRootDir;

	OUTPUT_LOG("./packages dir: %s\n", m_sPackagesDir.c_str());
	if (m_sPackagesDir != sRootDir)
		LoadPackagesInFolder(m_sPackagesDir);
}

bool CParaEngineAppBase::FindParaEngineDirectory(const char* sHint)
{
	std::string sModuleDir = ParaEngine::GetExecutablePath();
	if (!sModuleDir.empty())
	{
		m_sModuleDir = CParaFile::GetParentDirectoryFromPath(sModuleDir);
		std::string packagesDir = m_sModuleDir + "packages";
		if (!CParaFile::DoesFileExist(packagesDir.c_str(), false))
		{
			packagesDir = CParaFile::GetParentDirectoryFromPath(m_sModuleDir, 1) + "packages";
			if (!CParaFile::DoesFileExist(packagesDir.c_str(), false))
			{
				packagesDir = "";
			}
		}

		if (!packagesDir.empty())
		{
			m_sPackagesDir = packagesDir + "\\";
		}
	}
#ifdef WIN32
	/// ParaEngine.sig must be called first, to locate the root dir.
	if (!CParaFile::DoesFileExist(PARAENGINE_SIG_FILE, false))
	{
		if (!sModuleDir.empty())
		{
			std::string workingDir = m_sModuleDir;
			std::string sigPath = workingDir + PARAENGINE_SIG_FILE;
			bool bFoundSigFile = false;
			if (!CParaFile::DoesFileExist(sigPath.c_str(), false))
			{
				/// search the parent directory of the module for signature file, if file exist, use it as current working directory.
				sigPath = CParaFile::GetParentDirectoryFromPath(m_sModuleDir, 1) + PARAENGINE_SIG_FILE;
				if (CParaFile::DoesFileExist(sigPath.c_str(), false))
				{
					workingDir = CParaFile::GetParentDirectoryFromPath(m_sModuleDir, 1);
					bFoundSigFile = true;
				}
			}
			else
				bFoundSigFile = true;

			if (bFoundSigFile)
                ::SetCurrentDirectory(workingDir.c_str());
		}

		/// set the current directory by reading from the registry.
		// const char* sInstallDir = ReadRegStr("HKEY_CURRENT_USER", "SOFTWARE\\ParaEngine\\ParaWorld", "");
		// if(sInstallDir)
        //     ::SetCurrentDirectory(sInstallDir);
	}
	else
	{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
		std::wstring sModuleDir16 = StringHelper::MultiByteToWideChar(sModuleDir.c_str(), DEFAULT_FILE_ENCODING);
		int len = (int)(sModuleDir16.size());
		std::string::size_type last_dir_index = sModuleDir16.find_last_of(L"/\\");
		if (last_dir_index != std::string::npos)
		{
			// this ensures that the application is in the directory where module is installed. 
			std::wstring sParentDir = sModuleDir16.substr(0, last_dir_index + 1);
			::SetCurrentDirectoryW(sParentDir.c_str());
		}
#endif
	}

	{
		char sWorkingDir[512 + 1] = { 0 };
		memset(sWorkingDir, 0, sizeof(sWorkingDir));
		::GetCurrentDirectory(MAX_PATH, sWorkingDir);
		m_sInitialWorkingDir = sWorkingDir;
#ifdef PARAENGINE_MOBILE
		CGlobals::GetFileManager()->AddDiskSearchPath(sWorkingDir);
#endif
	}
#endif
	return true;
}

void ParaEngine::CParaEngineAppBase::BootStrapAndLoadConfig()
{
	FindBootStrapper();
	{
		/// load settings from config/config.txt or config/config.new.txt
		string sConfigFile = CParaFile::GetCurDirectory(CParaFile::APP_CONFIG_DIR) + "config.new.txt";
		bool bHasNewConfig = CParaFile::DoesFileExist(sConfigFile.c_str());
		SetHasNewConfig(bHasNewConfig);
		if (!bHasNewConfig)
			sConfigFile = (CParaFile::GetCurDirectory(CParaFile::APP_CONFIG_DIR) + "config.txt");

		string sFileName = CBootStrapper::GetSingleton()->GetConfigFile().empty() ? sConfigFile : CBootStrapper::GetSingleton()->GetConfigFile();
		CParaFile file(sFileName.c_str());
		if (!file.isEof())
		{
			string content = file.getBuffer();
			ParaEngineSettings::GetSingleton().LoadDynamicFieldsFromString(content);
		}

		if (bHasNewConfig)
		{
			CParaFile::DeleteFile(sConfigFile, false);
		}
	}
}

void ParaEngine::CParaEngineAppBase::InitSystemModules()
{
	m_pParaWorldAsset.reset(new CParaWorldAsset());
	CAISimulator::GetSingleton()->SetGameLoop(CBootStrapper::GetSingleton()->GetMainLoopFile());
	m_pRootScene.reset(new CSceneObject());
	m_pGUIRoot.reset(CGUIRoot::CreateInstance());
	m_pViewportManager.reset(new CViewportManager());
	m_pViewportManager->SetLayout(VIEW_LAYOUT_DEFAULT, m_pRootScene.get(), m_pGUIRoot.get());
}

void ParaEngine::CParaEngineAppBase::SetLandscapeMode(std::string landscapeMode)
{
	m_sLandscapeMode = landscapeMode;
}

std::string ParaEngine::CParaEngineAppBase::GetLandscapeMode()
{
	return m_sLandscapeMode;
}
