//----------------------------------------------------------------------
// Class:	CAISimulator
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2004.4.8
// Revised: 2004.4.8
//
// desc: 
// It hold the only instance of the CPOLRuntimeEnv in the application
// game interface should query this it from CAISimulator. It serves as
// a bridge between the game world and the POL language. It contains a 
// group of Host API(or HAPI) for POL to load. 
// 
// AI Simulator controls all the game and player logic of the game world:
// It includes the following aspect.
// (1) NPC and player logic. 
// (2) sequential cut scenes.
// (3) saving and restoring all game states. 
// (4) Network logic is handled transparently within the POL.
//
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "EventsCenter.h"
#include "NPLRuntime.h" // NPL runtime environment
#include "AISimulator.h"
#include "AsyncLoader.h"
#include "ParaWorldAsset.h"
#include "2dengine/GUIRoot.h"
#include "SceneObject.h"
#include "BlockEngine/BlockWorldClient.h"
#include "NPL/NPLWriter.h"
#include "IEnvironmentSim.h"


// @define this to log performance for debugging
// #define LOG_PERF
#ifdef LOG_PERF
#include "ParaTime.h"
#endif

using namespace ParaEngine;


/* list of files */
#define FILE_COLLISION		0
#define FILE_GAMELOOP	1
#define FILE_SHARE			2
#define FILE_NPC1			3
#define FILE_DIALOGBOX		4
#define FILE_TOALLCLIENTS	5
#define FILE_CALLHOST		6
#define FILE_SENSOR			7
#define FILE_OPC1			8

using namespace NPL;

const char *PEBaseModules[] = 
{
	"(gl)script/collision.lua",
	"(gl)script/gameinterface.lua",
	"(gl)script/share.lua",
	"(gl)script/npc1.lua",
	"(gl)script/dialogbox.lua",
	"(gl)network/host/HostToClients.lua",
	"(gl)network/ClientToHost.lua",
	"(gl)script/sensor.lua",
	"(gl)script/opc1.lua"
};

CAISimulator::CAISimulator(void)
:m_sGameloop(PEBaseModules[FILE_GAMELOOP]),m_fGameloopInterval(0.5f),m_bIsCleanedup(false)
{
	m_pRuntimeEnv  = CNPLRuntime::GetInstance();
}

CAISimulator::~CAISimulator(void)
{
}

CAISimulator * CAISimulator::GetSingleton()
{
	static CAISimulator g_instance;
	return &g_instance;
}

void CAISimulator::CleanUp()
{
	if(m_pRuntimeEnv)
		m_pRuntimeEnv->NPL_EnableNetwork(false,NULL, NULL);
	m_bIsCleanedup = true;
}
void CAISimulator::SetGameLoop(const string& scriptName)
{
	if (m_sGameloop != scriptName)
	{
		m_sGameloop = scriptName;
	}
}

void CAISimulator::SetGameLoopInterval(float fInterval)
{
	m_fGameloopInterval = fInterval;
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
void CAISimulator::FrameMove(float fElapsedTime)
{
	/* timers */
	static float g_fGameInterfaceTimer = 1000.0f; /* force execute the first time by giving a large value*/
	g_fGameInterfaceTimer += fElapsedTime;

	/* setting parameters for HAPI*/
	if(CGlobals::GetEnvSim() == NULL || m_pRuntimeEnv  == NULL)
		return;
	
	NPLRuntimeState_ptr main_rts_state = CGlobals::GetNPLRuntime()->GetMainRuntimeState();

	// lock for entire main thread frame move 
	Scoped_WriteLock<BlockReadWriteLock> lock_(BlockWorldClient::GetInstance()->GetReadWriteLock());

	// Async loaders. this must be readwrite lock protected, since the url callback will invoke NPL script. 
	CAsyncLoader::GetSingleton().ProcessDeviceWorkItems(100, false);

#ifdef LOG_PERF
	CParaTimeInterval simulatorInterval;
#endif
	// -- activate game interface module
	if(g_fGameInterfaceTimer > m_fGameloopInterval ) /* activate every 0.5 seconds */
	{
		g_fGameInterfaceTimer = 0;
		// send empty message
		if (!m_sGameloop.empty()) {
			m_pRuntimeEnv->NPL_Activate(main_rts_state, m_sGameloop.c_str(), NPL::CNPLWriter::GetNilMessage().c_str(), (int)NPL::CNPLWriter::GetNilMessage().size());
		}
	}

	// fire all unhandled events
	CGlobals::GetEventsCenter()->FireAllUnhandledEvents();

	int nSize = 0;

	//////////////////////////////////////////////////////////////////////////
	//
	// process all queued scene scripts: all may be run in a sandbox
	//
	//////////////////////////////////////////////////////////////////////////
	nSize = (int)CGlobals::GetScene()->GetScripts().size();
	if(nSize>0)
	{
		const char* sandbox = CGlobals::GetWorldInfo()->GetScriptSandBox();
		NPLRuntimeState_ptr scene_rts_state;
		if(sandbox!=NULL)
		{
			scene_rts_state =  CGlobals::GetNPLRuntime()->CreateGetRuntimeState(sandbox);
		}
		if(scene_rts_state.get() == NULL)
		{
			scene_rts_state = main_rts_state;
		}
		if(scene_rts_state.get() != NULL)
		{
			for(int i=0;i<nSize;i++)
			{
				auto& sc =  CGlobals::GetScene()->GetScripts()[i];
				// tricky note: since any individual script may delete object, we call activate to postpone execution until the script loop is finished. 
				scene_rts_state->Activate_async(sc.m_srcFile, sc.m_code.c_str(), (int)sc.m_code.size());
			}
		}
		CGlobals::GetScene()->GetScripts().ClearAll();
	}


	//////////////////////////////////////////////////////////////////////////
	//
	// process all queued GUI scripts
	//
	//////////////////////////////////////////////////////////////////////////

	nSize = (int)CGlobals::GetGUI()->m_scripts.size();
	if(nSize>0)
	{
		for(int i=0;i<nSize;i++)
		{
			GUI_SCRIPT& sc = CGlobals::GetGUI()->m_scripts[i];
			// tricky note: since any individual script may delete object, we call activate to postpone execution until the script loop is finished. 
			main_rts_state->Activate_async(sc.m_srcFile, sc.m_code.c_str(), (int)sc.m_code.size());
		}
		CGlobals::GetGUI()->m_scripts.ClearAll();
	}


	///-- execute NPL network logic for one pass
	m_pRuntimeEnv->Run();
#ifdef LOG_PERF
	simulatorInterval.Print("AI Simulater");
#endif
}

void ParaEngine::CAISimulator::AddNPLCommand( const string& sCommand )
{
	// empty file name
	static const string filename; 
	CGlobals::GetNPLRuntime()->GetMainRuntimeState()->Activate_async(filename, sCommand.c_str(), (int)sCommand.size());
}

bool ParaEngine::CAISimulator::IsCleanedUp()
{
	return m_bIsCleanedup;
}

void CAISimulator::NPLLoadFile(const char* filename, bool bReload)
{
	// TODO: should add local UI receiver?
	// TODO: should put in a queue rather than execute immediately?
	m_pRuntimeEnv->NPL_LoadFile(m_pRuntimeEnv->GetMainRuntimeState(), filename, bReload);
}

void CAISimulator::NPLDoString(const char* strCode, int nLength)
{
	// TODO: should add local UI receiver?
	// TODO: should put in a queue rather than execute immediately?
	//m_pRuntimeEnv->NPL_Activate(PEBaseModules[FILE_SHARE], strCode);
	m_pRuntimeEnv->GetMainRuntimeState()->DoString(strCode, nLength);
}

/// activate a local neuron file by its file path in the local directory
int CAISimulator::NPLActivate(const char* filepath, const char* code, int nLength)
{
	return m_pRuntimeEnv->NPL_Activate(m_pRuntimeEnv->GetMainRuntimeState(), filepath, code, nLength);
}