//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: Cross platformed NPL router plugin for ParaEngine
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "PluginAPI.h"

#include "NPL_imp.h"
#include "ParaEngineCore.h"

#ifdef WIN32
#define CORE_EXPORT_DECL    __declspec(dllexport)
#else
#define CORE_EXPORT_DECL
#endif

// forward declare of exported functions. 
#ifdef __cplusplus
extern "C" {
#endif
	CORE_EXPORT_DECL const char* LibDescription();
	CORE_EXPORT_DECL int LibNumberClasses();
	CORE_EXPORT_DECL unsigned long LibVersion();
	CORE_EXPORT_DECL ParaEngine::ClassDescriptor* LibClassDesc(int i);
	CORE_EXPORT_DECL void LibInit();
	CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid);
	CORE_EXPORT_DECL ParaEngine::IParaEngineCore* GetParaEngineCOREInterface();
#ifdef __cplusplus
}   /* extern "C" */
#endif

extern HINSTANCE g_hAppInstance;

using namespace ParaEngine;

typedef ClassDescriptor* (*GetClassDescMethod)();

GetClassDescMethod Plugins[] =
{
	ParaEngine::ParaEngine_GetClassDesc,
	NPL::NPL_GetClassDesc,
};

CORE_EXPORT_DECL const char* LibDescription()
{
	return "ParaEngine Server Ver 1.0.0";
}

CORE_EXPORT_DECL unsigned long LibVersion()
{
	return GetParaEngineVersion();
}

CORE_EXPORT_DECL int LibNumberClasses()
{
	return sizeof(Plugins)/sizeof(Plugins[0]);
}

CORE_EXPORT_DECL ClassDescriptor* LibClassDesc(int i)
{
	if (i < LibNumberClasses() && Plugins[i])
	{
		return Plugins[i]();
	}
	else
	{
		return NULL;
	}
}

CORE_EXPORT_DECL void LibInit()
{
	
}

CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
	//Test_NPL_DLL_Activate_Interface(nType, pVoid);
	if(nType == ParaEngine::PluginActType_STATE)
	{
		
	}
}

CORE_EXPORT_DECL ParaEngine::IParaEngineCore* GetParaEngineCOREInterface()
{
	return CParaEngineCore::GetStaticInterface();
}

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
#else
void __attribute__ ((constructor)) DllMain()
#endif
{
	// TODO: dll start up code here
#ifdef WIN32
	g_hAppInstance = hinstDLL;				// Hang on to this DLL's instance handle.
	return (TRUE);
#endif
}

