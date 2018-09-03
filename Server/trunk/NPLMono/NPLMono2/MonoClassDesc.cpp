//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: 
//-----------------------------------------------------------------------------

#include "NPLMono.h"

// ParaEngine includes
#include "PluginAPI.h"
#include "IParaEngineApp.h"
#include "MonoScriptingState.h"

using namespace ParaEngine;

// the host application's NPL_Mono2_CLASS_ID must match this one. 
#define NPL_Mono2_CLASS_ID Class_ID(0x3b903b30, 0x57e409be)


/** description class */
class CMonoClassDesc : public ClassDescriptor 
{
public:
	virtual void* Create(bool loading = FALSE)
	{ 
		OUTPUT_LOG("NPLMono2: Creating Mono interface\n");
		CMonoScriptingState* pInstance = new CMonoScriptingState();
		if (pInstance)
		{
			OUTPUT_LOG("NPLMono2: Mono interface created\n");
		}
		else
		{
			OUTPUT_LOG("Error: NPLMono2: failed to create CMonoScriptingState\n");
		}
		return pInstance;
	}

	virtual const char* ClassName()
	{ 
		return "NPLMono2:CMonoScriptingState"; 
	}

	virtual SClass_ID SuperClassID()
	{ 
		return OBJECT_MODIFIER_CLASS_ID; 
	}

	virtual Class_ID ClassID() 
	{
		return NPL_Mono2_CLASS_ID;
	}

	virtual const char* Category()
	{ 
		return "NPLFile"; 
	}

	virtual const char* InternalName()
	{ 
		return "NPLMono2"; 
	}	

	HINSTANCE HInstance() 
	{ 
		extern HINSTANCE Instance;
		return Instance; 
	}
};

ClassDescriptor* NPLMono_GetClassDesc() 
{ 
	static CMonoClassDesc Desc;
	return &Desc; 
}
