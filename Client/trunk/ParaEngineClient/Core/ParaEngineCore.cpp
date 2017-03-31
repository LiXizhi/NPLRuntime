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
	return CParaEngineApp::GetInstance()->ForceRender();
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