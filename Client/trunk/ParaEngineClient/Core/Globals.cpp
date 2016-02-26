//-----------------------------------------------------------------------------
// Class:	CGlobals
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "AISimulator.h"
#include "FrameRateController.h"
#include "FileManager.h"
#include "ParaEngineSettings.h"
#include "IParaEngineApp.h"
#include "ParaEngineAppImp.h"
#include "PluginManager.h"
#include "EventsCenter.h"
#include "ic/ICConfigManager.h"
#include "util/Report.h"
#include "2dengine/GUIRoot.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "WorldInfo.h"
#include "ViewportManager.h"
#include "SelectionManager.h"
#include "OceanManager.h"
#include "LightManager.h"
#include "AttributesManager.h"
#include "DataProviderManager.h"
#include "MoviePlatform.h"
#include "Globals.h"

#if PLATFORM_WINDOWS
#include "DirectXEngine.h"

#ifdef USE_XACT_AUDIO_ENGINE
#include "AudioEngine.h"
#endif
	
namespace ParaEngine 
{
	using namespace ParaTerrain;
#if defined (PLATFORM_WINDOWS) && !defined(PARAENGINE_SERVER)
	extern HWND* g_pHwndHWND;
#endif
}
#endif

namespace ParaEngine 
{
	bool g_bIsLoading = false;
	CReport g_report;
	TransformStack        g_worldMatrixStack;
	TransformStack        g_viewMatrixStack;
	TransformStack        g_projectionMatrixStack;

	extern CFrameRateController g_renderFRC;
	extern CFrameRateController g_ioFRC;
	extern CFrameRateController g_simFRC;
	extern CFrameRateController g_gameTime;
}
using namespace ParaEngine;

CGlobals::CGlobals(void)
{
	
}

CGlobals::~CGlobals(void)
{
	
}

IParaEngineApp* CGlobals::GetApp()
{
	return CParaEngineApp::GetInstance();
}

TransformStack& CGlobals::GetWorldMatrixStack()
{
	return g_worldMatrixStack;
}

TransformStack& CGlobals::GetViewMatrixStack()
{
	return g_viewMatrixStack;
}

TransformStack& CGlobals::GetProjectionMatrixStack()
{
	return g_projectionMatrixStack;
}

CAISimulator* CGlobals::GetAISim()
{
	return CAISimulator::GetSingleton();
}

double CGlobals::GetGameTime()
{
	return (g_gameTime.GetTime());
}

CFileManager* CGlobals::GetFileManager()
{
	return CFileManager::GetInstance();
}

CReport* CGlobals::GetReport()
{
	return & g_report;
}


NPL::INPLStimulationPipe* CGlobals::GetNetworkLayer()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetworkLayerPipe();
}

NPL::CNPLRuntime* CGlobals::GetNPLRuntime()
{
	return NPL::CNPLRuntime::GetInstance();
}

CFrameRateController* CGlobals::GetFrameRateController(FRCType nType)
{
	switch (nType)
	{
	case FRC_RENDER:
		return &g_renderFRC;
	case FRC_IO:
		return &g_ioFRC;
	case FRC_SIM:
		return &g_simFRC;
	case FRC_GAME:
		return &g_gameTime;
	default:
		break;
	}
	return NULL;
}

bool CGlobals::WillGenReport(const char* itemName)
{
#if defined _DEBUG
	return true;
#elif defined (PARAENGINE_CLIENT)
	return GetScene()->IsGenerateReport();
#else
	return false;
#endif
}

const Matrix4* CGlobals::GetIdentityMatrix()
{
	return &Matrix4::IDENTITY;
}

bool CGlobals::IsLoading()
{
	return g_bIsLoading;
}

void CGlobals::SetLoading(bool bIsLoading)
{
	g_bIsLoading = bIsLoading;
}
const string& CGlobals::GetString(int nStringID)
{
	static string empty;
	if (nStringID == 0)
		return empty;
	//TODO: add other string ID
	return empty;
}

ParaEngineSettings& CGlobals::GetSettings()
{
	return ParaEngineSettings::GetSingleton();
}

CPluginManager* CGlobals::GetPluginManager()
{
	return CPluginManager::GetInstance();
}

ParaInfoCenter::CICConfigManager* CGlobals::GetICConfigManager()
{
	return ParaInfoCenter::CICConfigManager::GetInstance();
}
CEventsCenter* CGlobals::GetEventsCenter()
{
	return CEventsCenter::GetInstance();
}

CGUIRoot* CGlobals::GetGUI()
{
	return CGUIRoot::GetInstance();
}

CParaWorldAsset* CGlobals::GetAssetManager()
{
	return CParaWorldAsset::GetSingleton();
}

CSceneObject* CGlobals::GetScene()
{
	return CSceneObject::GetInstance();
}

ParaTerrain::CGlobalTerrain* CGlobals::GetGlobalTerrain()
{
	return (GetScene()->GetGlobalTerrain());
}

CWorldInfo* CGlobals::GetWorldInfo()
{
	return &CWorldInfo::GetSingleton();
}

SceneState* CGlobals::GetSceneState()
{
	return GetScene()->GetSceneState();
}

CPhysicsWorld* CGlobals::GetPhysicsWorld()
{
	return GetScene()->GetPhysicsWorld();
}

CViewportManager* CGlobals::GetViewportManager()
{
	return CParaEngineApp::GetInstance()->GetViewportManager();
}

CSelectionManager* CGlobals::GetSelectionManager()
{
	return CSelectionManager::GetSingleton();
}

IEnvironmentSim* CGlobals::GetEnvSim()
{
	return GetScene()->GetEnvSim();
}

COceanManager* CGlobals::GetOceanManager()
{
	return COceanManager::GetInstance();
}

CLightManager* CGlobals::GetLightManager()
{
	return &CLightManager::GetSingleton();
}

RenderDevicePtr CGlobals::GetRenderDevice()
{
#ifdef USE_DIRECTX_RENDERER
	return GetDirectXEngine().GetRenderDevice();
#else
	return RenderDevice::GetInstance();
#endif
}

EffectManager* CGlobals::GetEffectManager()
{
	return &(CParaWorldAsset::GetSingleton()->GetEffectManager());
}

CAttributesManager* CGlobals::GetAttributesManager()
{
	return &CAttributesManager::GetSingleton();
}
CDataProviderManager* CGlobals::GetDataProviderManager()
{
	return &CDataProviderManager::GetSingleton();
}

CMoviePlatform* CGlobals::GetMoviePlatform()
{
	return &CMoviePlatform::GetSingleton();
}

HWND CGlobals::GetAppHWND()
{
#if defined (PLATFORM_WINDOWS) && !defined(PARAENGINE_SERVER)
	if (g_pHwndHWND != 0)
		return *g_pHwndHWND;
	else
#endif
	return 0;
}

#if defined(PLATFORM_WINDOWS) && defined(USE_DIRECTX_RENDERER)

DirectXEngine& CGlobals::GetDirectXEngine()
{
	static DirectXEngine dxengine_;
	return dxengine_;
}

CAudioEngine* CGlobals::GetAudioEngine()
{
#ifdef USE_XACT_AUDIO_ENGINE
	return CAudioEngine::GetInstance();
#else
	return NULL;
#endif
}

#endif