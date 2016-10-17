//-----------------------------------------------------------------------------
// Class:	CParaEngineCore
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2006.8.4
// Desc: It implements the IParaEngineCore interface, which exposes everything in ParaEngine to plug-in applications. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "PluginAPI.h"
#include "ParaEngineCore.h"
#include "FrameRateController.h"
#include "ParaEngineAppImp.h"
#include <boost/thread/tss.hpp>

using namespace ParaEngine;


ParaEngine::WeakPtr ParaEngine::CParaEngineCore::m_pAppSingleton;

/** @def class id*/
#define PARAENGINE_CLASS_ID Class_ID(0x2b903b29, 0x47e409af)

/** description class */
class CParaEngineClassDesc : public ClassDescriptor
{
public:

	void* Create(bool loading = FALSE)
	{
		return CParaEngineCore::GetInstance();
	}

	const char* ClassName()
	{
		return "ParaEngine";
	}

	SClass_ID SuperClassID()
	{
		return OBJECT_MODIFIER_CLASS_ID;
	}

	Class_ID ClassID()
	{
		return PARAENGINE_CLASS_ID;
	}

	const char* Category()
	{
		return "ParaEngine";
	}

	const char* InternalName()
	{
		return "ParaEngineCore";
	}

	HINSTANCE HInstance()
	{
		return NULL;
	}
};

ClassDescriptor* ParaEngine::ParaEngine_GetClassDesc()
{
	static CParaEngineClassDesc Desc;
	return &Desc;
}


CParaEngineCore::CParaEngineCore(void)
{
}

CParaEngineCore::~CParaEngineCore(void)
{
	DestroySingleton();
}


CParaEngineCore* ParaEngine::CParaEngineCore::GetInstance()
{
	static boost::thread_specific_ptr< CParaEngineCore > g_instance;
	if (!g_instance.get())
		g_instance.reset(new CParaEngineCore());
	return (g_instance.get());
}

IParaEngineCore* CParaEngineCore::GetStaticInterface()
{
	return (IParaEngineCore*)CParaEngineCore::GetInstance();
}

DWORD CParaEngineCore::GetVersion()
{
	return GetParaEngineVersion();
}

bool CParaEngineCore::Sleep(float fSeconds)
{
	SLEEP(static_cast<DWORD>(fSeconds * 1000));
	return true;
}

HWND CParaEngineCore::GetParaEngineHWnd()
{
	return 0;
}

bool CParaEngineCore::ForceRender()
{
#ifdef USE_DIRECTX_RENDERER
	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();

	if (pd3dDevice == 0)
		return false;
	bool bSucceed = false;
#ifndef ONLY_FORCERENDER_GUI 
	if (SUCCEEDED(CParaEngineApp::GetInstance()->Render()))
	{
		bSucceed = SUCCEEDED(CParaEngineApp::GetInstance()->PresentScene());
	}
#else	
	if (SUCCEEDED(pd3dDevice->BeginScene()))
	{
		// since we use EnableAutoDepthStencil, The device will create a depth-stencil buffer when it is created. The depth-stencil buffer will be automatically set as the render target of the device. 
		// When the device is reset, the depth-stencil buffer will be automatically destroyed and recreated in the new size. 
		// However, we must SetRenderTarget to the back buffer in each frame in order for  EnableAutoDepthStencil work properly for the backbuffer as well. 
		pd3dDevice->SetRenderTarget(0, CGlobals::GetDirectXEngine().GetRenderTarget(0)); // force setting render target to back buffer. and 

		/// clear to scene
		pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
			0x00000000, 1.0f, 0L);
		CGlobals::GetGUI()->AdvanceGUI(0);
		pd3dDevice->EndScene();
		bSucceed = SUCCEEDED(CParaEngineApp::GetInstance()->Present(NULL, NULL, NULL, NULL));
	}
#endif

	return bSucceed;
#else
	return true;
#endif
}

IParaEngineApp* CParaEngineCore::GetAppInterface()
{
	return (IParaEngineApp*)(CParaEngineApp::GetInstance());
}


IParaEngineApp* CParaEngineCore::CreateApp()
{
	IParaEngineApp* pApp = GetAppInterface();
	if (pApp == 0)
	{
		// we will only create app if it has not been created before. 
		if (!m_pAppSingleton)
		{
			CParaEngineApp* pApp = new CParaEngineApp();
			m_pAppSingleton = pApp;
			return (IParaEngineApp*)pApp;
		}

	}
	return pApp;
}

void ParaEngine::CParaEngineCore::DestroySingleton()
{
	if (m_pAppSingleton)
	{
		m_pAppSingleton->Release();
		m_pAppSingleton.reset();
	}
}