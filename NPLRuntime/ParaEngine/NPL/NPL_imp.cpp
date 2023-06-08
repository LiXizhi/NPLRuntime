//-----------------------------------------------------------------------------
// Class:	NPL wrapper dll
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2010.2.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "PluginAPI.h"

#include "NPLRuntime.h" // NPL runtime environment
#ifndef PARAENGINE_MOBILE
#include "ParaEngineService.h"
#endif
#include "AsyncLoader.h"
#include "EventsCenter.h"

#include "NPL_imp.h"

using namespace ParaEngine;
using namespace NPL;

#pragma region CNPLClassDesc
/** @def class id*/
#define NPL_CLASS_ID Class_ID(0x2b903b39, 0x47e409af)

/** description class */
class CNPLClassDesc : public ClassDescriptor 
{
public:

	void* Create(bool loading = FALSE) 
	{ 
		static CNPL_imp g_singleton;
		return &g_singleton; 
	}

	const char* ClassName() 
	{ 
		return "NPL"; 
	}

	SClass_ID SuperClassID() 
	{ 
		return OBJECT_MODIFIER_CLASS_ID; 
	}

	Class_ID ClassID() 
	{ 
		return NPL_CLASS_ID; 
	}

	const char* Category() 
	{ 
		return "ParaEngine"; 
	}

	const char* InternalName() 
	{ 
		return "NPL object"; 
	}	

	HINSTANCE HInstance() 
	{ 
		return NULL; 
	}
};

ClassDescriptor* NPL::NPL_GetClassDesc() 
{ 
	static CNPLClassDesc Desc;
	return &Desc; 
}

#pragma endregion CNPLClassDesc

CNPL_imp::CNPL_imp()
:m_fGameloopInterval(0.5f)
{
}

int CNPL_imp::activate(const char * sNPLFilename, const char* sCode, int nCodeLength)
{
	return GetNPLRuntime()->GetMainState()->activate(sNPLFilename, sCode, nCodeLength);
}

int CNPL_imp::StartService(const char* pCommandLine)
{
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN_SINGLE_THREAD)
	ParaEngine::CParaEngineService service;

	service.AcceptKeyStroke(true);

	int exit_code = service.Run((char*)pCommandLine);
	return exit_code;
#else
	return 0;
#endif
}

void CNPL_imp::StopService()
{
}

INPLRuntime* CNPL_imp::GetNPLRuntime()
{
	return CGlobals::GetNPLRuntime();
}

void CNPL_imp::SetGameLoop(const char* scriptName)
{
	m_sGameloop = scriptName;
}

void CNPL_imp::SetGameLoopInterval(float fInterval)
{
	m_fGameloopInterval = fInterval;
}

void CNPL_imp::AddNPLCommand( const char* sCommand, int nLength )
{
	// empty file name
	static const string filename; 
	GetNPLRuntime()->GetMainState()->ActivateLocal(filename.c_str(), sCommand, nLength);
}

//--------------------------------------------------------
/// desc: Move all object's mental states to next time 
/// the following global states are defined specific to this simulator
/// (1) for all NPC type object, "npc_name = object.identifer();" is passed
///     to the neuron file as input.
/// (1.1) for all OPC type object, "opc_name = object.identifer();" is passed
///     to the neuron file as input.
/// (2) for GUI sensor type object, "sensor_name = object.identifer();" is passed
///     to the neuron file as input.
///  (2.1) for GUI sensor type object that accept key strokes, 
///      an additional "keystring = keystrings" is passed as input.
//--------------------------------------------------------
void CNPL_imp::FrameMove(float fElapsedTime)
{
	/* timers */
	static float fGameInterfaceTimer = 1000.0f; /* force execute the first time by giving a large value*/
	fGameInterfaceTimer += fElapsedTime;

	INPLRuntimeState* main_rts_state = GetNPLRuntime()->GetMainState();


	// -- activate game interface module
	if(fGameInterfaceTimer > m_fGameloopInterval ) /* activate every 0.5 seconds */
	{
		fGameInterfaceTimer = 0;
		if(!m_sGameloop.empty())
		{
			main_rts_state->ActivateLocal(m_sGameloop.c_str());
		}
	}

	// fire all unhandled events
	CGlobals::GetEventsCenter()->FireAllUnhandledEvents();

	// Async loaders.
	CAsyncLoader::GetSingleton().ProcessDeviceWorkItems(100, false);

	///-- execute NPL network logic for one pass
	GetNPLRuntime()->Run();
}