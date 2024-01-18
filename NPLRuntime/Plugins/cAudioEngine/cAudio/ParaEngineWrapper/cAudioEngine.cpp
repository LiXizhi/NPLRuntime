//-----------------------------------------------------------------------------
// Class:	ParaEngine wrapper dll
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2010.2.16
//-----------------------------------------------------------------------------
// ParaEngine plug in includes. It is header only, no linker is required. 
#include "PluginAPI.h"

#include "ParaAudioEngine.h"

using namespace ParaEngine;

#ifdef WIN32
#define CORE_EXPORT_DECL    __declspec(dllexport)
#else
#define CORE_EXPORT_DECL
#endif

ClassDescriptor* AudioEngine_GetClassDesc();
typedef ClassDescriptor* (*GetClassDescMethod)();

namespace ParaEngine
{

#define AudioEngine_CLASS_ID Class_ID(0x2b903a29, 0x57e409cf)
	/// description class
	class CAudioEngineClassDesc : public ClassDescriptor
	{
	public:

		void* Create(bool loading = FALSE)
		{
			return new CParaAudioEngine();
		}

		const char* ClassName()
		{
			return "IParaAudioEngine";
		}

		SClass_ID SuperClassID()
		{
			return OBJECT_MODIFIER_CLASS_ID;
		}

		Class_ID ClassID()
		{
			return AudioEngine_CLASS_ID;
		}

		const char* Category()
		{
			return "AudioEngine";
		}

		const char* InternalName()
		{
			return "AudioEngine";
		}

		HINSTANCE HInstance()
		{
			return Instance;
		}

		static HINSTANCE Instance;
	};

	HINSTANCE CAudioEngineClassDesc::Instance = NULL;
}

ClassDescriptor* AudioEngine_GetClassDesc()
{
	static CAudioEngineClassDesc Desc;
	return &Desc;
}

// following is only compiled if as dynamically linked 
#if !defined(AUDIO_STATICLIB)

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
#ifdef __cplusplus
}   /* extern "C" */
#endif

GetClassDescMethod Plugins[] =
{
	AudioEngine_GetClassDesc,
};

CORE_EXPORT_DECL const char* LibDescription()
{
	return "ParaEngine Audio Engine Lib using cAudio & OpenAL Ver 1.0.0";
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

CORE_EXPORT_DECL void LibInit()
{
}

CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
}

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
#else
void __attribute__ ((constructor)) DllMain()
#endif
{
	// TODO: dll start up code here
#ifdef WIN32
	CAudioEngineClassDesc::Instance = hinstDLL;				// Hang on to this DLL's instance handle.
	return (TRUE);
#endif
}
#endif
