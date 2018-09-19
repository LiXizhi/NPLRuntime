//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
//-----------------------------------------------------------------------------
#include "NPLMono.h"

// ParaEngine includes
#include "PluginAPI.h"
#include "ParaScriptingGlobal.h"
#include "MonoScriptingState.h"
#include "NPLMonoInterface.h"

/*
* We add our API internal call, so that C# code can call us back.
*/
void Add_NPL_Internal_Call()
{
	
	// NPL class
	mono_add_internal_call ("NPL::HelloWorld", (const void*)(NPLMonoInterface::NPL_wrapper::HelloWorld));
	mono_add_internal_call ("NPL::GetCurrentMsg", (const void*)(NPLMonoInterface::NPL_wrapper::GetCurrentMsg));
	mono_add_internal_call ("NPL::activate", (const void*)(NPLMonoInterface::NPL_wrapper::activate));
	mono_add_internal_call ("NPL::activate2", (const void*)(NPLMonoInterface::NPL_wrapper::activate2));
	mono_add_internal_call ("NPL::accept", (const void*)(NPLMonoInterface::NPL_wrapper::accept));
	mono_add_internal_call ("NPL::reject", (const void*)(NPLMonoInterface::NPL_wrapper::reject));


	// ParaGlobal class
	mono_add_internal_call ("ParaGlobal::log", (const void*)(NPLMonoInterface::ParaGlobal_wrapper::log));
	mono_add_internal_call ("ParaGlobal::applog", (const void*)(NPLMonoInterface::ParaGlobal_wrapper::applog));

}