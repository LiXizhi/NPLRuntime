//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: Cross platformed NPL router plugin for ParaEngine
//-----------------------------------------------------------------------------
#include "NPLRouter.h"
#include "NPLRouterStrategy.h"

#ifdef WIN32
#define CORE_EXPORT_DECL    __declspec(dllexport)
#else
#define CORE_EXPORT_DECL
#endif

// forware declare of exported functions. 
#ifdef __cplusplus
extern "C" {
#endif
	CORE_EXPORT_DECL const char* LibDescription();
	CORE_EXPORT_DECL int LibNumberClasses();
	CORE_EXPORT_DECL unsigned long LibVersion();
	CORE_EXPORT_DECL ParaEngine::ClassDescriptor* LibClassDesc(int i);
	CORE_EXPORT_DECL void LibInit();
	CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid);
#ifdef __cplusplus
}   /* extern "C" */
#endif

HINSTANCE Instance = NULL;

using namespace ParaEngine;

extern ClassDescriptor* NPLRouter_GetClassDesc();
typedef ClassDescriptor* (*GetClassDescMethod)();

GetClassDescMethod Plugins[] =
{
	NPLRouter_GetClassDesc,
};


CORE_EXPORT_DECL const char* LibDescription()
{
	return "ParaEngine NPL Router Ver 1.0.0";
}

CORE_EXPORT_DECL unsigned long LibVersion()
{
	return 1;
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

ParaEngine::IParaEngineCore* g_pCoreInterface = NULL;
ParaEngine::IParaEngineCore* GetCoreInterface()
{
	return g_pCoreInterface;
}

CORE_EXPORT_DECL void LibInitParaEngine(IParaEngineCore* pCoreInterface)
{
	g_pCoreInterface = pCoreInterface;
}

CORE_EXPORT_DECL void LibInit()
{
}

// Written by LiXizhi for gosling, 2009.7.13: 
// The following are examples of API that you may use.
void Test_NPL_DLL_Activate_Interface(int nType, void* pVoid)
{
	if(nType == ParaEngine::PluginActType_STATE)
	{
		// Getting the runtime state (Thread) within which this file is activated. 
		NPL::INPLRuntimeState* pState = (NPL::INPLRuntimeState*)pVoid;
		const char* sMsg = pState->GetCurrentMsg();
		int nMsgLength = pState->GetCurrentMsgLength();
		
		// write something to log via NPL API interface
		pState->WriteLog("NPL Router is activated \n");
		OUTPUT_LOG(" activate input: %s\n", sMsg);

		// using logger directly via C++ API interface
#ifdef WIN32
		void * logger = NULL;
#else
		CServiceLogger_ptr logger = ParaEngine::CServiceLogger::GetLogger("NPLRouter");
#endif
		
		SERVICE_LOG(logger, 0, "Service Log from dll interface 1");
		SERVICE_LOG(logger, 0, "Service Log from dll interface 2");
		SERVICE_LOG1(logger, "hello");
		SERVICE_LOG1(logger, "hello %s", "ParaEngine");

		APP_LOG("This is an app log from dll 1");
		APP_LOG("This is an app log from dll 2");

		// Example 1: Create NPLTable and serialize to string
		std::string output;
		NPLInterface::NPLObjectProxy msg;
		msg["nid"] = (double)10;
		msg["name"] = "value";
		msg["tab"]["name1"] = "value1";
		NPLInterface::NPLHelper::NPLTableToString(NULL, msg, output);
		APP_LOG(output.c_str());

		// Example 2: deserialize NPLTable from string. 
		NPLInterface::NPLObjectProxy tabMsg = NPLInterface::NPLHelper::StringToNPLTable("{nid=10, name=\"value\", tab={name1=\"value1\"}}");
		const std::string& s_name = tabMsg["name"];
		assert((double)tabMsg["nid"] == 10);
		assert((tabMsg["name"]) == "value");
		assert((tabMsg["tab"]["name1"]) == "value1");
		output.clear();
		NPLInterface::NPLHelper::NPLTableToString("msg", tabMsg, output);
		APP_LOG(output.c_str());

		// Example 3 send any message either locally or remotely.
		// activate a local file
		pState->activate("script/test/TestNPL.lua", output.c_str(), output.size());
		// activate a remote file
		pState->activate("(some_rts)some_nid:NPLRouter.dll", output.c_str(), output.size());

		// Example 4 Accept/Reject a connection. 
		pState->GetNPLRuntime()->accept("tid", "nid");
		pState->GetNPLRuntime()->reject("tid");

		// Example 5: using tinyXML 
		//TiXmlDocument doc("config/bootstrapper.xml");
		//if(!doc.LoadFile())
		//{
		//	APP_LOG("log file failed");
		//	return ;
		//}
	}
}

CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
	//Test_NPL_DLL_Activate_Interface(nType, pVoid);
	if(nType == ParaEngine::PluginActType_STATE)
	{
		NPL::NPLRouterStrategy::GetInstance()->DoRouterStrategy(nType,pVoid);
	}
}

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
#else
void __attribute__ ((constructor)) DllMain()
#endif
{
	// TODO: dll start up code here
#ifdef WIN32
	Instance = hinstDLL;				// Hang on to this DLL's instance handle.
	return (TRUE);
#endif
}
