//-----------------------------------------------------------------------------
// Class:	ParaEngine wrapper dll
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2010.2.16
//-----------------------------------------------------------------------------

// ParaEngine plug in includes. It is header only, no linker is required. 
#include "PluginAPI.h"

#include "ParaPhysicsWorld.h"

using namespace ParaEngine;

#ifdef WIN32
#define CORE_EXPORT_DECL    __declspec(dllexport)
#else
#define CORE_EXPORT_DECL
#endif

ClassDescriptor* PhysicsBT_GetClassDesc();
typedef ClassDescriptor* (*GetClassDescMethod)();

#define PhysicsBT_CLASS_ID Class_ID(0x2b903a29, 0x47e409cf)

namespace ParaEngine
{
	/** description class */
	class CPhysicsBTClassDesc : public ClassDescriptor
	{
	public:

		void* Create(bool loading = FALSE)
		{
			return new CParaPhysicsWorld();
		}

		const char* ClassName()
		{
			return "IParaPhysics";
		}

		SClass_ID SuperClassID()
		{
			return OBJECT_MODIFIER_CLASS_ID;
		}

		Class_ID ClassID()
		{
			return PhysicsBT_CLASS_ID;
		}

		const char* Category()
		{
			return "Physics";
		}

		const char* InternalName()
		{
			return "Physics";
		}

		HINSTANCE HInstance()
		{
			return Instance;
		}
		static HINSTANCE Instance;
	};
	HINSTANCE CPhysicsBTClassDesc::Instance = NULL;
}

ClassDescriptor* PhysicsBT_GetClassDesc()
{
	static CPhysicsBTClassDesc Desc;
	return &Desc;
}

// following is only compiled if as dynamically linked 
#if !defined(STATIC_LIBRARY)
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
	PhysicsBT_GetClassDesc,
};


CORE_EXPORT_DECL const char* LibDescription()
{
	return "ParaEngine Physics Lib using Bullet Ver 1.0.0";
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
	CPhysicsBTClassDesc::Instance = hinstDLL;				// Hang on to this DLL's instance handle.
	return (TRUE);
#endif
}

#endif