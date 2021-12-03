//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: 
//-----------------------------------------------------------------------------
#include "NPLRouter.h"
#include "PluginAPI.h"

using namespace ParaEngine;

#define NPL_ROUTER_CLASS_ID Class_ID(0x2b903b29, 0x47e409cf)

/** TODO: implementation here */
class CRouterImp
{
public:
	CRouterImp(){};
	~CRouterImp(){};
};


/** description class */
class CRouterClassDesc : public ClassDescriptor 
{
public:

	void* Create(bool loading = FALSE) 
	{ 
		return new CRouterImp(); 
	}

	const char* ClassName() 
	{ 
		return "HelloWorld"; 
	}

	SClass_ID SuperClassID() 
	{ 
		return OBJECT_MODIFIER_CLASS_ID; 
	}

	Class_ID ClassID() 
	{ 
		return NPL_ROUTER_CLASS_ID; 
	}

	const char* Category() 
	{ 
		return "NPL File"; 
	}

	const char* InternalName() 
	{ 
		return "NPL Router"; 
	}	

	HINSTANCE HInstance() 
	{ 
		extern HINSTANCE Instance;
		return Instance; 
	}
};

ClassDescriptor* NPLRouter_GetClassDesc() 
{ 
	static CRouterClassDesc Desc;
	return &Desc; 
}
