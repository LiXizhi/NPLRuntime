//-----------------------------------------------------------------------------
// Class:	ParaEngineServerApp
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2016.2.23
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#if defined(PARAENGINE_SERVER)
#include "BootStrapper.h"
#include "ParaWorldAsset.h"
#include "SceneObject.h"
#include "2dengine/GUIRoot.h"
#include "ParaEngineAppImp.h"
#include "ViewportManager.h"
#include "FrameRateController.h"
#include "NPLRuntime.h"
#include "AISimulator.h"
#include "AsyncLoader.h"

#include "ParaEngineServerApp.h"

using namespace ParaEngine;

ParaEngine::CParaEngineApp::CParaEngineApp(const char* lpCmdLine) 
	: CParaEngineAppBase(lpCmdLine)
{
	StartApp(lpCmdLine);
}

DWORD ParaEngine::CParaEngineApp::GetCoreUsage()
{
	return PE_USAGE_SERVICE;
}

CParaEngineApp::~CParaEngineApp()
{
	StopApp();
}

HRESULT CParaEngineApp::StartApp(const char* sCommandLine)
{
	g_pCurrentApp = this;
	std::string strCmd;
	VerifyCommandLine(sCommandLine, strCmd);
	SetAppCommandLine(strCmd.c_str());

	// loading packages 
	LoadPackages();
	BootStrapAndLoadConfig();
	InitSystemModules();

	OUTPUT_LOG1("ParaEngineServer started\n");
	
	return S_OK;
}


void CParaEngineApp::BootStrapAndLoadConfig()
{
	if (!CBootStrapper::GetSingleton()->LoadFromFile(GetAppCommandLineByParam("bootstrapper", "")))
	{
		const char* pBootFileName = GetAppCommandLineByParam("bootstrapper", "");
		if (pBootFileName && pBootFileName[0] != '\0'){
			OUTPUT_LOG("error: can not find bootstrapper file at %s\n", pBootFileName);
		}
	}
	// OUTPUT_LOG("cmd line: %s \n", GetAppCommandLine());
	OUTPUT_LOG("main loop: %s \n", CBootStrapper::GetSingleton()->GetMainLoopFile().c_str());
}

void CParaEngineApp::InitSystemModules()
{
	m_pParaWorldAsset.reset(new CParaWorldAsset());
	CAISimulator::GetSingleton()->SetGameLoop(CBootStrapper::GetSingleton()->GetMainLoopFile());
	m_pRootScene.reset(new CSceneObject());
	m_pGUIRoot.reset(CGUIRoot::CreateInstance());
	m_pViewportManager.reset(new CViewportManager());
	m_pViewportManager->SetLayout(VIEW_LAYOUT_DEFAULT, m_pRootScene.get(), m_pGUIRoot.get());
}

HRESULT CParaEngineApp::StopApp()
{
	// if it is already stopped, we shall return
	if (!m_pParaWorldAsset)
		return S_OK;

	FinalCleanup();

	m_pParaWorldAsset.reset();
	m_pRootScene.reset();
	m_pGUIRoot.reset();
	m_pViewportManager.reset();

	// Clean up all threads
	CAsyncLoader::GetSingleton().CleanUp();
	return S_OK;
}

HRESULT CParaEngineApp::FrameMove(double fTime)
{
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
	}
	OnFrameEnded();

	return S_OK;
}
#endif


