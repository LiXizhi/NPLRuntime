//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: exposing ParaEngine and NPL API to the mono scripting interface as internal mono calls. 
//-----------------------------------------------------------------------------
#include "NPLMono.h"

// ParaEngine includes
#include "PluginAPI.h"
#include "INPLRuntime.h"
#include "IParaEngineApp.h"

#include "marshal_wrapper.h"
#include "NPLMonoInterface.h"

using namespace NPLMonoInterface;

MonoString* NPL_wrapper::HelloWorld() 
{
	return mono_string_new (mono_domain_get (), "All your monos belong to ParaEngine!");
}

MonoString* NPL_wrapper::GetCurrentMsg(void* npl_runtime_state)
{
	if(npl_runtime_state!=0)
	{
		const char* sMsg = ((NPL::INPLRuntimeState*)npl_runtime_state)->GetCurrentMsg();
		if(sMsg)
			return mono_string_new (mono_domain_get (), sMsg);
	}
	return NULL;
}

void NPL_wrapper::activate(void* npl_runtime_state, MonoString* file_name, MonoString* msg)
{
	if(npl_runtime_state!=0 && file_name!=0)
	{
		mono_string_utf8 sFileName(file_name);
		mono_string_utf8 sMsg(msg);
		((NPL::INPLRuntimeState*)npl_runtime_state)->activate(sFileName.c_str(), sMsg.c_str());
	}
}

void NPL_wrapper::activate2(MonoString* file_name, MonoString* msg)
{
	if(file_name!=0)
	{
		mono_string_utf8 sFileName(file_name);
		mono_string_utf8 sMsg(msg);
		GetCoreInterface()->GetAppInterface()->GetNPLRuntime()->Activate(NULL, sFileName.c_str(), sMsg.c_str());
	}
}

void NPL_wrapper::accept(MonoString* tid, MonoString* nid)
{
	mono_string_utf8 tid_(tid);
	mono_string_utf8 nid_(nid);
	GetCoreInterface()->GetAppInterface()->GetNPLRuntime()->accept(tid_.c_str(), nid_.c_str());
}

void NPL_wrapper::reject(MonoString* tid)
{
	mono_string_utf8 tid_(tid);
	GetCoreInterface()->GetAppInterface()->GetNPLRuntime()->reject(tid_.c_str());
}

void ParaGlobal_wrapper::log(MonoString* str) 
{
	if(str!=0)
	{
		mono_string_utf8 str_(str);
		GetCoreInterface()->GetAppInterface()->WriteToLog(str_.c_str());
	}
}

void ParaGlobal_wrapper::applog(MonoString* str) 
{
	if(str!=0)
	{
		mono_string_utf8 str_(str);
		GetCoreInterface()->GetAppInterface()->AppLog(str_.c_str());
	}
}