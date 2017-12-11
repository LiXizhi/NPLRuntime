#ifdef WIN32
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#endif

#include "PluginAPI.h"

/**
* Optional NPL includes, just in case you want to use some core functions see GetCoreInterface()
*/
#include "INPLRuntime.h"
#include "INPLRuntimeState.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"

using namespace ParaEngine;


#ifdef WIN32
#define CORE_EXPORT_DECL    __declspec(dllexport)
#else
#define CORE_EXPORT_DECL
#endif

 // forward declare of exported functions. 
 extern "C" {
 	CORE_EXPORT_DECL const char* LibDescription();
 	CORE_EXPORT_DECL int LibNumberClasses();
 	CORE_EXPORT_DECL unsigned long LibVersion();
 	CORE_EXPORT_DECL ParaEngine::ClassDescriptor* LibClassDesc(int i);
 	CORE_EXPORT_DECL void LibInit();
 	CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid);
 	CORE_EXPORT_DECL void LibInitParaEngine(ParaEngine::IParaEngineCore* pCoreInterface);
 }
 
 HINSTANCE Instance = NULL;

 ClassDescriptor* HelloWorldPlugin_GetClassDesc();
 typedef ClassDescriptor* (*GetClassDescMethod)();

 GetClassDescMethod Plugins[] = 
 {
 	HelloWorldPlugin_GetClassDesc,
 };


CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
	if(nType == ParaEngine::PluginActType_STATE)
	{
	}
}


/** This has to be unique, change this id for each new plugin.
*/
#define HelloWorld_CLASS_ID Class_ID(0x2b905a29, 0x47b409af)

class HelloWorldPluginDesc :public ClassDescriptor
{
public:
	void* Create(bool loading = FALSE)
	{
		return NULL;
	}

	const char* ClassName()
	{
		return "IHelloWorld";
	}

	SClass_ID SuperClassID()
	{
		return OBJECT_MODIFIER_CLASS_ID;
	}

	Class_ID ClassID()
	{
		return HelloWorld_CLASS_ID;
	}

	const char* Category()
	{
		return "HelloWorld";
	}

	const char* InternalName()
	{
		return "HelloWorld";
	}

	HINSTANCE HInstance()
	{
		extern HINSTANCE Instance;
		return Instance;
	}
};


ClassDescriptor* HelloWorldPlugin_GetClassDesc()
{
	static HelloWorldPluginDesc s_desc;
	return &s_desc;
}

CORE_EXPORT_DECL const char* LibDescription()
{
	return "ParaEngine HelloWorld Ver 1.0.0";
}

CORE_EXPORT_DECL unsigned long LibVersion()
{
	return 1;
}

CORE_EXPORT_DECL int LibNumberClasses()
{
	return sizeof(Plugins) / sizeof(Plugins[0]);
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
	assert(pCoreInterface != NULL);
	g_pCoreInterface = pCoreInterface;
	g_pCoreInterface->GetAppInterface()->WriteToLog("LibInitParaEngine in sqlite.cpp\r\n");
}

CORE_EXPORT_DECL void LibInit()
{
}

/*
#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
#else
void __attribute__((constructor)) DllMain()
#endif
{
#ifdef WIN32
	Instance = hinstDLL;				// Hang on to this DLL's instance handle.
	return (TRUE);
#endif
}
*/

extern "C" {
	/** this is an example of c function calling NPL core interface */
	void WriteLog(const char* zFormat, ...) {
		if(GetCoreInterface()) {
			va_list args;
			va_start(args, zFormat);
			char buffer[2050] = { '\0' };
			int nSize2 = vsnprintf(buffer, 2050, zFormat, args);

			GetCoreInterface()->GetAppInterface()->WriteToLog(buffer);
			va_end(args);
		}
	}
}