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
#include "Globals.h"
#include "ParaEngineAppBase.h"
#include "ParaEngineRenderBase.h"

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/tss.hpp>
#endif

using namespace ParaEngine;

IParaEngineApp* CreateParaEngineApp();
IRenderWindow* CreateParaRenderWindow(const int width, const int height);


ParaEngine::WeakPtr ParaEngine::CParaEngineCore::m_pAppSingleton;
ParaEngine::WeakPtr ParaEngine::CParaEngineCore::m_pRenderWindowSingleton;

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


void ParaEngine::CParaEngineCore::Destroy()
{
	DestroySingleton();
}

CParaEngineCore* ParaEngine::CParaEngineCore::GetInstance()
{
	thread_local static CParaEngineCore g_instance;
	return &g_instance;
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
	return CGlobals::GetApp()->ForceRender();
}

IParaEngineApp* CParaEngineCore::GetAppInterface()
{
	return CGlobals::GetApp();
}


IParaEngineApp* CParaEngineCore::CreateApp()
{
	//IParaEngineApp* pApp = GetAppInterface();
	//assert(pApp);
	//return pApp;


	IParaEngineApp* pApp = GetAppInterface();
	if (pApp == 0)
	{
		// we will only create app if it has not been created before. 
		if (!m_pAppSingleton)
		{
			CParaEngineAppBase* pApp = (CParaEngineAppBase*)CreateParaEngineApp();
			m_pAppSingleton = pApp;
			return (IParaEngineApp*)pApp;
		}

	}
	return pApp;
}

IRenderWindow* CParaEngineCore::CreateRenderWindow(const int width, const int height)
{
	if (!m_pRenderWindowSingleton)
	{
		auto pWin = (CParaEngineAppBase*)CreateParaRenderWindow(width, height);
		m_pRenderWindowSingleton = pWin;
		return (IRenderWindow*)pWin;
	}

	return (IRenderWindow*)m_pRenderWindowSingleton.get();
}

void ParaEngine::CParaEngineCore::DestroySingleton()
{
	if (m_pAppSingleton)
	{
		m_pAppSingleton->Release();
		m_pAppSingleton.reset();
	}

	if (m_pRenderWindowSingleton)
	{
		m_pRenderWindowSingleton->Release();
		m_pRenderWindowSingleton.reset();
	}
}