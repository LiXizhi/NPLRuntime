//-----------------------------------------------------------------------------
// Class:	NPLRuntimeState
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.22
// Desc:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "IParaEngineApp.h"
#include "PluginManager.h"
#include "PluginAPI.h"
#include "IParaWebService.h"
#include "NPLActivationFile.h"
#include "FileManager.h"
#include "NPLHelper.h"
#include "NPLCommon.h"
#include "NPLRuntime.h"
#include "util/ScopedLock.h"
#include <boost/bind.hpp>
#include "NPLRuntimeState.h"
#ifdef EMSCRIPTEN_SINGLE_THREAD
#define auto_ptr unique_ptr
#endif
/**
for luabind, The main drawback of this approach is that the compilation time will increase for the file
that does the registration, it is therefore recommended that you register everything in the same cpp-file.
*/
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <luabind/luabind.hpp>


/** the maximum number of messages to read from the incoming queue for processing in a single frame rate.
this is usually smaller than DEFAULT_INPUT_QUEUE_SIZE */
#define MAX_MESSAGE_PROCESSED_PER_FRAME		5000

/** whether to use the new NPL mono2 interface. */
#define USE_NPL_MONO2	

#ifdef USE_NPL_MONO2
/** the NPL mono2 plugin dll file name. It will be translated to libNPLMono2.so under linux automatically. */
#ifdef _DEBUG
const char* NPL_MONO_DLL_FILE_PATH = "NPLMono2_d.dll";
#else
const char* NPL_MONO_DLL_FILE_PATH = "NPLMono2.dll";
#endif
/** the NPL mono2 class interface id. */
#define NPL_Mono_CLASS_ID Class_ID(0x3b903b30, 0x57e409be)
#else
/** the NPL mono plugin dll file name. It will be translated to libNPLMono.so under linux automatically. */
const char* NPL_MONO_DLL_FILE_PATH = "NPLMono.dll";
/** the NPL mono class interface id. */
#define NPL_Mono_CLASS_ID Class_ID(0x2b903b29, 0x47e409af)
#endif

/** coroutine name*/
#define COROUTINE_NAME "__co"

/** helper class to mark processing */
class CMarkProcessing {
public:
	CMarkProcessing(bool * pValueBool) :m_pValueBool(pValueBool) { if (m_pValueBool) { *m_pValueBool = true; } }
	~CMarkProcessing() { if (m_pValueBool) { *m_pValueBool = false; } }
	bool * m_pValueBool;
};

NPL::CNPLRuntimeState::CNPLRuntimeState(const string & name, NPLRuntimeStateType type_)
	: m_bUseMessageEvent(false), m_name(name), m_type(type_), m_nFrameMoveCount(0), m_bIsPreemptive(false), m_bPauseAllPreemptiveFunction(false),
	m_current_msg(NULL), m_current_msg_length(0), m_pMonoScriptingState(NULL), m_processed_msg_count(0), m_bIsProcessing(false),
	ParaScripting::CNPLScriptingState(type_ != NPLRuntimeStateType_DLL && type_ != NPLRuntimeStateType_NPL_ExternalLuaState)
{
}

void NPL::CNPLRuntimeState::Init()
{
	if (!IsValid())
		return;

	switch (m_type)
	{
	case NPLRuntimeStateType_NPL_LITE:
		// Load NPL common library
		LoadNPLState();
		break;
	case NPLRuntimeStateType_NPL:
	case NPLRuntimeStateType_NPL_ExternalLuaState:
		// Load NPL common library
		LoadNPLState();
		// load the para scripting library.
		LoadParaLib();
		break;
	case NPLRuntimeStateType_DLL:
		break;
	}

	// signal
	StateLoaded(this);
}

void NPL::CNPLRuntimeState::Reset(const char* onResetScode)
{
	// send the quit message. 
	NPLMessage_ptr msg(new NPLMessage());
	msg->m_type = MSG_TYPE_RESET;
	if (onResetScode != NULL)
	{
		msg->m_code = onResetScode;
	}
	SendMessage(msg, 1);
}

void NPL::CNPLRuntimeState::Reset_Imp()
{
	if (m_type != NPLRuntimeStateType_DLL)
	{
		DestroyState();
		ClearNeuronFiles();
		CreateSetState();
	}
	OUTPUT_LOG("NPL State %s is being reset\n", GetName().c_str());

	// clear the input queue, since we are starting all over again.
	NPLMessage_ptr msg;
	while (m_input_queue.try_pop(msg))
		;
	// remove all timers
	KillTimer(-1);
	// init state again. 
	Init();
}

NPL::CNPLRuntimeState::~CNPLRuntimeState()
{
	Stop();
	SAFE_RELEASE(m_pMonoScriptingState);
	ClearNeuronFiles();
	OUTPUT_LOG("NPL State %s exited\n", GetName().c_str());
}

void NPL::CNPLRuntimeState::ClearNeuronFiles()
{
	for (auto v : m_act_files_cpp)
	{
		SAFE_RELEASE(v.second);
	}
	for (auto v : m_neuron_files)
	{
		SAFE_DELETE(v.second);
	}
	m_neuron_files.clear();
	m_active_neuron_files.clear();

	m_act_files_cpp.clear();
}

NPL::IMonoScriptingState* NPL::CNPLRuntimeState::GetMonoState()
{
	if (m_pMonoScriptingState)
		return m_pMonoScriptingState;

	using namespace ParaEngine;

	// let us create a new mono scripting state load from the NPLMono.dll plugin. 
	ParaEngine::DLLPlugInEntity* pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->GetPluginEntity(NPL_MONO_DLL_FILE_PATH);
	if (pPluginEntity == 0)
	{
		// load the plug-in if it has never been loaded before. 
		pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->LoadDLL("", NPL_MONO_DLL_FILE_PATH);
	}

	if (pPluginEntity != 0)
	{
		for (int i = 0; i < pPluginEntity->GetNumberOfClasses(); ++i)
		{
			ClassDescriptor* pClassDesc = pPluginEntity->GetClassDescriptor(i);

			if (pClassDesc && pClassDesc->ClassID() == NPL_Mono_CLASS_ID)
			{
				m_pMonoScriptingState = (NPL::IMonoScriptingState*) pClassDesc->Create();
			}
		}
		if (m_pMonoScriptingState == 0)
		{
			OUTPUT_LOG("warn: required Mono class: %d %d can not be found in the dll\n", NPL_Mono_CLASS_ID.PartA(), NPL_Mono_CLASS_ID.PartB());
			for (int i = 0; i < pPluginEntity->GetNumberOfClasses(); ++i)
			{
				ClassDescriptor* pClassDesc = pPluginEntity->GetClassDescriptor(i);

				if (pClassDesc)
				{
					OUTPUT_LOG("Mono class:%d %d in the loaded dll has class name: %s\n", pClassDesc->ClassID().PartA(), pClassDesc->ClassID().PartB(), pClassDesc->ClassName());
				}
			}
		}
	}

	if (m_pMonoScriptingState != 0)
	{
		// Create the mono domain using the same name as the NPLRuntimeState. 
		m_pMonoScriptingState->CreateState(GetName().c_str(), GetNPLStateInterface());
	}
	else
	{
		OUTPUT_LOG("error: can not create Mono state %s from NPLMono.dll \n", GetName().c_str());
	}

	return m_pMonoScriptingState;
}

NPL::CNeuronFileState* NPL::CNPLRuntimeState::GetNeuronFileState(const std::string& filename, bool bCreateIfNotExist)
{
	auto iter = m_neuron_files.find(filename);
	if (iter != m_neuron_files.end())
		return iter->second;
	else if (bCreateIfNotExist)
	{
		auto pFileState = new CNeuronFileState(filename);
		pFileState->SetMaxQueueSize(GetMsgQueueSize());
		m_neuron_files[filename] = pFileState;
		return pFileState;
	}
	return NULL;
}

bool NPL::CNPLRuntimeState::HasDebugHook()
{
	auto L = GetLuaState();
	if (L) {
		auto pHookFunc = lua_gethook(L);
		return pHookFunc != 0;
	}
	return false;
}

bool NPL::CNPLRuntimeState::IsPreemptive()
{
	return m_bIsPreemptive;
}

bool NPL::CNPLRuntimeState::IsAllPreemptiveFunctionPaused() const
{
	return m_bPauseAllPreemptiveFunction;
}

void NPL::CNPLRuntimeState::PauseAllPreemptiveFunction(bool val)
{
	m_bPauseAllPreemptiveFunction = val;
}

void NPL::CNPLRuntimeState::SetPreemptive(bool val)
{
	m_bIsPreemptive = val;
}

int NPL::CNPLRuntimeState::Stop_Async()
{
	if (m_thread.get() != 0)
	{
		// send the quit message. 
		NPLMessage_ptr msg(new NPLMessage());
		msg->m_type = MSG_TYPE_EXIT;
		SendMessage(msg, 1);
	}
	return 0;
}

int NPL::CNPLRuntimeState::Stop()
{
	if (m_thread.get() != 0)
	{
		// send the quit message. 
		Stop_Async();

		m_thread->join();
		m_thread.reset();
	}
	return 0;
}

int NPL::CNPLRuntimeState::Run_Async()
{
	if (m_thread.get() == 0)
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		m_thread.reset(new npl_thread(boost::bind(&NPL::CNPLRuntimeState::Run, shared_from_this())));
#else
		m_thread.reset(CoroutineThread::StartCoroutineThread([this](CoroutineThread*)->CO_ASYNC{
			this->Run();
		}, nullptr));
#endif
	}
	return 0;
}

int NPL::CNPLRuntimeState::Run()
{
	NPLMessage_ptr msg;
	int nRes = 0;
	while (nRes != -1)
	{
		m_input_queue.wait_and_pop(msg);
		nRes = ProcessMsg(msg);
	}
	// this is necessary, because we must finalize mono state before the thread is terminated. 
	// Otherwise there will a exception when application exit via the main thread. 
	SAFE_RELEASE(m_pMonoScriptingState);
	return 0;
}

int NPL::CNPLRuntimeState::Process()
{
	// process as many as possible. 
	NPLMessage_ptr msg;
	for (int i = 0; i < MAX_MESSAGE_PROCESSED_PER_FRAME && m_input_queue.try_pop(msg); ++i)
	{
		ProcessMsg(msg);
	}
	return 0;
}

int NPL::CNPLRuntimeState::ProcessMsg(NPLMessage_ptr msg)
{
	if (msg.get() == 0)
		return 0;

	++m_processed_msg_count;
	if (msg->m_type == MSG_TYPE_FILE_ACTIVATION)
	{
		auto pFileState = GetNeuronFileState(msg->m_filename, false);
		if (!pFileState) {
			LoadFile_any(msg->m_filename, false, 0, true);
			pFileState = GetNeuronFileState(msg->m_filename);
		}
		if (pFileState)
		{
			if (pFileState->IsProcessing() || pFileState->IsPreemptive())
			{
				// only push to active queue when it is the first message. 
				if (pFileState->IsEmpty())
					m_active_neuron_files.push_back(pFileState);
				if (!pFileState->PushMessage(msg))
				{
					// TODO: report message queue is full.
				}
			}
			else
			{
				ActivateFile_any(msg->m_filename, msg->m_code.c_str(), (int)msg->m_code.size());
				pFileState->Tick(m_nFrameMoveCount);
			}
		}
	}
	else if (msg->m_type == MSG_TYPE_TICK)
	{
		return FrameMoveTick();
	}
	else if (msg->m_type == MSG_TYPE_FILE_LOAD)
	{
		auto pFileState = GetNeuronFileState(msg->m_filename, false);
		if (!pFileState) {
			LoadFile_any(msg->m_filename, false, 0, true);
		}
	}
	else if (msg->m_type == MSG_TYPE_RESET)
	{
		Reset_Imp();
		if (!msg->m_code.empty())
		{
			DoString(msg->m_code.c_str(), (int)msg->m_code.size());
		}
	}
	else if (msg->m_type == MSG_TYPE_EXIT)
	{
		return -1;
	}
	return 0;
}

static void npl_preemptive_scheduler_hook(lua_State* L, lua_Debug* ar)
{
	(void)ar;
	lua_yield(L, 0);
}

/*
From Mike: http://lua-users.org/lists/lua-l/2011-06/msg00513.html

[Note that lua_sethook() is the only cross-thread-safe and
signal-safe function from the Lua API.]

Also, please note that JIT-compiled code does NOT call hooks by
default. Either disable the JIT compiler or edit src/Makefile:
XCFLAGS= -DLUAJIT_ENABLE_CHECKHOOK
This comes with a speed penalty even hook is not set.
*/
int NPL::CNPLRuntimeState::FrameMoveTick()
{
	if (IsAllPreemptiveFunctionPaused())
		return 0;
	// if debug hook is available, all preemptive function are paused. 
	bool bHasDebugger = HasDebugHook();
	if (bHasDebugger)
		return 0;

	m_nFrameMoveCount++;
	// tick all neuron files with pending messages here. 
	for (auto iter = m_active_neuron_files.begin(); iter != m_active_neuron_files.end(); )
	{
		CNeuronFileState* pFileState = (*iter);
		if (pFileState->PopClearState())
		{
			// clear all message and quit last running coroutine
			if (pFileState->IsProcessing())
			{
				pFileState->SetProcessing(false);
				// stop last coroutine
				lua_State * L = GetLuaState();
				lua_pushstring(L, COROUTINE_NAME);
				lua_rawget(L, LUA_REGISTRYINDEX);
				if (lua_istable(L, -1))
				{
					lua_pushnil(L);
					lua_setfield(L, -2, pFileState->GetFilename().c_str());
				}
				lua_pop(L, 1);
			}
			pFileState->ClearMessageImp();
		}
		if (pFileState->IsProcessing())
		{
			pFileState->SetProcessing(false);
			if (pFileState->IsPreemptive())
			{
				// resume last coroutine task 
				lua_State * L = GetLuaState();

				// create coroutine object such that _G["__co"][GetFileName()] = coroutine.create();
				lua_pushstring(L, COROUTINE_NAME);
				lua_rawget(L, LUA_REGISTRYINDEX);
				if (lua_istable(L, -1))
				{
					lua_getfield(L, -1, pFileState->GetFilename().c_str());
					if (lua_isthread(L, -1))
					{
						lua_State * th = lua_tothread(L, -1);
						int status = lua_status(th);
						if (status == LUA_YIELD)
						{
							// call activate function with preemptive hook. 
							lua_sethook(th, npl_preemptive_scheduler_hook, LUA_MASKCOUNT, pFileState->GetPreemptiveInstructionCount());
							{
								ParaEngine::ScopedBoolean_Lock lock(&m_bIsPreemptive);
								int top = lua_gettop(th);

								int status = lua_resume(th, 0);
								int num_results = lua_gettop(th) - top;
								if (status == LUA_YIELD)
								{
									pFileState->SetProcessing(true);
								}
								else if (status != 0)
								{
									// read error message on stack
									string strErrorMsg;
									const char* errorMsg = lua_tostring(th, -1);
									if (errorMsg != NULL) {
										strErrorMsg = errorMsg;
									}
									strErrorMsg += " in file:";
									strErrorMsg += pFileState->GetFilename();
									switch (status) {
									case LUA_ERRSYNTAX:
										strErrorMsg += " <Syntax error>\r\n";
										break;
									case LUA_ERRMEM:
										strErrorMsg += " <Memory allocation error>\r\n";
										break;
									case LUA_ERRRUN:
										strErrorMsg += " <Runtime error>\r\n";
										break;
									}
									OUTPUT_LOG("%s", strErrorMsg.c_str());
								}
								if (num_results > 0)
									lua_pop(th, num_results);
							}
							// Lua 5.1 support per thread hook, while luajit hook is global to all. 
							// so we need to unhook it for the rest of non-preemptive code.
							lua_sethook(th, npl_preemptive_scheduler_hook, 0, pFileState->GetPreemptiveInstructionCount());
						}
					}
					lua_pop(L, 1);
					if (!pFileState->IsProcessing())
					{
						// clear coroutine from _REG["__co"][filename] = nil, for garbage collection.
						lua_pushnil(L);
						lua_setfield(L, -2, pFileState->GetFilename().c_str());
					}
				}
				lua_pop(L, 1);
			}
			else
			{
				pFileState->SetProcessing(false);
			}
			if (!pFileState->IsProcessing())
			{
				NPLMessage_ptr msg;
				pFileState->PopMessage(msg);
				pFileState->Tick(m_nFrameMoveCount);
			}
		}
		else
		{
			NPLMessage_ptr msg;
			while (!pFileState->IsProcessing() && pFileState->GetMessage(msg))
			{
				pFileState->SetProcessing(false);
				if (pFileState->IsPreemptive())
				{
					DoString(msg->m_code.c_str(), (int)msg->m_code.size());
					lua_State * L = GetLuaState();
					// use coroutine with hooks to simulate preemptive multi-tasking. 
					lua_State* th = lua_newthread(L);
					{
						// call activate function with preemptive hook. 
						lua_sethook(th, npl_preemptive_scheduler_hook, LUA_MASKCOUNT, pFileState->GetPreemptiveInstructionCount());

						const char actTable[] = "__act";
						lua_pushlstring(th, actTable, sizeof(actTable) - 1);
						lua_gettable(th, LUA_GLOBALSINDEX);
						if (lua_istable(th, -1))
						{
							lua_pushlstring(th, pFileState->GetFilename().c_str(), pFileState->GetFilename().size());
							lua_gettable(th, -2);
							if (lua_isfunction(th, -1))
							{
								ParaEngine::ScopedBoolean_Lock lock(&m_bIsPreemptive);
								int top = lua_gettop(th);

								int status = lua_resume(th, 0);
								int num_results = lua_gettop(th) - top;
								if (status == LUA_YIELD)
								{
									pFileState->SetProcessing(true);
								}
								else if (status != 0)
								{
									// read error message on stack
									string strErrorMsg;
									const char* errorMsg = lua_tostring(th, -1);
									if (errorMsg != NULL) {
										strErrorMsg = errorMsg;
									}
									strErrorMsg += " in file:";
									strErrorMsg += pFileState->GetFilename();
									switch (status) {
									case LUA_ERRSYNTAX:
										strErrorMsg += " <Syntax error>\r\n";
										break;
									case LUA_ERRMEM:
										strErrorMsg += " <Memory allocation error>\r\n";
										break;
									case LUA_ERRRUN:
										strErrorMsg += " <Runtime error>\r\n";
										break;
									}
									OUTPUT_LOG("%s", strErrorMsg.c_str());
								}
								if (num_results > 0)
									lua_pop(th, num_results);
							}
						}
						// Lua 5.1 support per thread hook, while luajit hook is global to all. 
						// so we need to unhook it for the rest of non-preemptive code.
						lua_sethook(th, npl_preemptive_scheduler_hook, 0, pFileState->GetPreemptiveInstructionCount());
					}
					if (!pFileState->IsProcessing()) {
						// pop coroutine and leave it to gc
						lua_pop(L, 1);
					}
					else
					{
						// keep a copy of coroutine at _REG["__co"][GetFileName()] = coroutine; so that it can be resumed in the next time slice
						lua_pushstring(L, COROUTINE_NAME);
						lua_rawget(L, LUA_REGISTRYINDEX);
						if (!lua_istable(L, -1)) {
							lua_pop(L, 1);
							lua_newtable(L);
							lua_pushvalue(L, -1);
							lua_setfield(L, LUA_REGISTRYINDEX, COROUTINE_NAME);
						}
						lua_pushvalue(L, -2);
						lua_setfield(L, -2, pFileState->GetFilename().c_str());
						lua_pop(L, 2);
					}
				}
				else
				{
					ActivateFile_any(msg->m_filename, msg->m_code.c_str(), (int)msg->m_code.size());
				}
				if (!pFileState->IsProcessing())
				{
					pFileState->PopMessage(msg);
					pFileState->Tick(m_nFrameMoveCount);
				}
			}
		}
		// when queue is empty, remove from the list. 
		if (pFileState->IsEmpty())
			iter = m_active_neuron_files.erase(iter);
		else
			iter++;
	}
	return 0;
}

int NPL::CNPLRuntimeState::SendTick()
{
	// send the quit message. 
	NPLMessage_ptr msg(new NPLMessage());
	msg->m_type = MSG_TYPE_TICK;
	SendMessage(msg, 1);
	return 0;
}


int NPL::CNPLRuntimeState::GetCurrentQueueSize()
{
	return (int)(m_input_queue.size());
}

int NPL::CNPLRuntimeState::GetProcessedMsgCount()
{
	return m_processed_msg_count;
}

template <typename StringType>
bool NPL::CNPLRuntimeState::LoadFile_any(const StringType & filepath, bool bReload, lua_State* L, bool bNoReturn)
{
	if (filepath.empty())
		return true;
	int nSize = (int)filepath.size();
	bool bHasScriptFileExtension = nSize > 5 && filepath[nSize - 4] == '.' &&
		((filepath[nSize - 3] == 'l' && filepath[nSize - 2] == 'u' && filepath[nSize - 1] == 'a') ||
		(filepath[nSize - 3] == 'n' && filepath[nSize - 2] == 'p' && filepath[nSize - 1] == 'l'));

	if (nSize > 2 && filepath[nSize - 1] == '/')
	{
		// if it is a folder, we will add as NPL module
		std::string sOutputFile;
		if (ParaEngine::CGlobals::GetApp()->LoadNPLPackage(filepath.c_str(), &sOutputFile))
		{
			if (!sOutputFile.empty())
			{
				return LoadFile_any(sOutputFile, false, L, bNoReturn);
			}
			return true;
		}
		if (!bNoReturn && L != 0)
		{
			// return false to scripting environment if module is not found. 
			lua_pushboolean(L, 0);
		}
		return false;
	}
	else if (nSize > 5 && !bHasScriptFileExtension)
	{
		if ((filepath[nSize - 3] == 'd' && filepath[nSize - 2] == 'l' && filepath[nSize - 1] == 'l') ||
			(filepath[nSize - 3] == '.' && filepath[nSize - 2] == 's' && filepath[nSize - 1] == 'o'))
		{
			// for *.dll and *.so plug-in files
			if (filepath[nSize - 5] != '*')
			{
				DLL_Plugin_Map_Type::iterator iter = m_dll_plugins_map.find(filepath);
				if (iter == m_dll_plugins_map.end())
				{
					ParaEngine::DLLPlugInEntity* pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->GetPluginEntity(filepath);
					if (pPluginEntity == 0)
					{
						// load the plug-in if it has never been loaded before. 
						pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->LoadDLL("", filepath);
					}
					m_dll_plugins_map[filepath] = pPluginEntity;
				}
			}
			else
			{
				ParaEngine::CGlobals::GetPluginManager()->LoadAllDLLinDirectory(filepath);
			}
			return true;
		}
		else if (filepath[nSize - 3] == '.' && filepath[nSize - 2] == 'c' && filepath[nSize - 1] == 's')
		{
			// for *.cs Mono .NET script files. 
			// CS files are only loaded at activation time. 
			return true;
		}
		else if ((filepath[nSize - 2] == '.' && filepath[nSize - 1] == 'c') || (filepath[nSize - 3] == 'c' && filepath[nSize - 2] == 'p' && filepath[nSize - 1] == 'p' && filepath[nSize - 4] == '.'))
		{
			// *.c and *.cpp files
			auto iter = m_act_files_cpp.find(filepath);
			if (iter == m_act_files_cpp.end())
			{
				// registering by dynamically load the library. 
				RegisterFile(filepath.c_str(), new NPL_C_Func_ActivationFile(filepath));
			}
			return true;
		}
		else if ((filepath[nSize - 3] == 'z' && filepath[nSize - 2] == 'i' && filepath[nSize - 1] == 'p')
			|| (filepath[nSize - 3] == 'p' && filepath[nSize - 2] == 'k' && filepath[nSize - 1] == 'g'))
		{
			// pkg or zip file
			ParaEngine::CFileManager::GetInstance()->OpenArchive(filepath, bReload);
			return true;
		}
	}
	if (filepath[0] == '.' && nSize > 3)
	{
		std::string fullPath;
		// load using relative path to current file path. 
		if (filepath[1] == '/')
		{
			fullPath = ParaEngine::CParaFile::GetParentDirectoryFromPath(GetCurrentFileName(L), 0);
			fullPath.append(filepath.c_str() + 2);
		}
		else if (filepath[1] == '.' && filepath[2] == '/')
		{
			// such as ../../
			fullPath = ParaEngine::CParaFile::GetParentDirectoryFromPath(GetCurrentFileName(L), 1);
			int nOffset = 3;
			while (filepath[nOffset] == '.' && filepath[nOffset + 1] == '.' && filepath[nOffset + 2] == '/')
			{
				fullPath = ParaEngine::CParaFile::GetParentDirectoryFromPath(fullPath, 1);
				nOffset += 3;
			}
			fullPath.append(filepath.c_str() + nOffset);
		}

		if (!fullPath.empty())
		{
			// automatically add file extension if not. 
			if (!bHasScriptFileExtension)
			{
				std::string output, out;
				output = fullPath + ".lua";
				if (GetScriptDiskPath(output, out) != 0)
					fullPath = output;
				else
				{
					output = fullPath + ".npl";
					if (GetScriptDiskPath(output, out) != 0)
						fullPath = output;
				}
			}
			return LoadFile(fullPath, bReload, L, bNoReturn);
		}
	}
	if (!bHasScriptFileExtension)
	{
		// this is file module, we need to use search path
		std::string fileFullPath = GetModuleFilePath(filepath, L);
		if (fileFullPath != filepath)
		{
			return LoadFile(fileFullPath, bReload, L, bNoReturn);
		}
		else
		{
			// file module not found
		}
	}
	return LoadFile(filepath, bReload, L, bNoReturn);
}

template <typename StringType>
NPL::NPLReturnCode NPL::CNPLRuntimeState::ActivateFile_any(const StringType& filepath, const char * code /*= NULL*/, int nLength/*=0*/)
{
	CMarkProcessing is_processing_(&m_bIsProcessing);

	int nSize = (int)filepath.size();
	if (nSize > 5 && (filepath[nSize - 3] != 'l' && filepath[nSize - 3] != 'n') /* skip *.lua (npl) file */)
	{
		if ((filepath[nSize - 3] == 'd' && filepath[nSize - 2] == 'l' && filepath[nSize - 1] == 'l') ||
			(filepath[nSize - 3] == '.' && filepath[nSize - 2] == 's' && filepath[nSize - 1] == 'o'))
		{
			// for *.dll and *.so plug-in files
			ParaEngine::DLLPlugInEntity* pPluginEntity = NULL;
			DLL_Plugin_Map_Type::iterator iter = m_dll_plugins_map.find(filepath);
			if (iter == m_dll_plugins_map.end())
			{
				// load the dll file if it has not been loaded before
				ParaEngine::DLLPlugInEntity* pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->GetPluginEntity(filepath);
				if (pPluginEntity == 0)
				{
					// load the plug-in if it has never been loaded before. 
					pPluginEntity = ParaEngine::CGlobals::GetPluginManager()->LoadDLL("", filepath);
				}
				m_dll_plugins_map[filepath] = pPluginEntity;
			}
			else
			{
				pPluginEntity = iter->second;
			}

			if (pPluginEntity != 0)
			{
				// activate the dll file
				CCurrentMessage  push_msg(this, code, nLength);
				return (pPluginEntity->Activate(ParaEngine::PluginActType_STATE, (void*)GetNPLStateInterface()) >= 0) ? NPL_OK : NPL_Error;
			}
			return NPL_FailedToLoadFile;
		}
		else if (filepath[nSize - 3] == '.' && filepath[nSize - 2] == 'c' && filepath[nSize - 1] == 's')
		{
			// for *.cs Mono .NET script files. 
			IMonoScriptingState* pState = GetMonoState();
			if (pState)
			{
				// activate the cs file
				CCurrentMessage  push_msg(this, code, nLength);
				return pState->ActivateFile(filepath, code, nLength);
			}
			return NPL_FailedToLoadFile;
		}
		else if ((filepath[nSize - 2] == '.' && filepath[nSize - 1] == 'c') || (filepath[nSize - 3] == 'c' && filepath[nSize - 2] == 'p' && filepath[nSize - 1] == 'p' && filepath[nSize - 4] == '.'))
		{
			auto iter = m_act_files_cpp.find(filepath);
			if (iter != m_act_files_cpp.end())
			{
				if (iter->second) {
					CCurrentMessage  push_msg(this, code, nLength);
					return iter->second->OnActivate(GetNPLStateInterface());
				}
			}
		}
	}
	return ActivateFile(filepath, code, nLength);
}

NPL::NPLReturnCode NPL::CNPLRuntimeState::Activate_async(const string & filepath, const char * code /*= NULL*/, int nLength/*=0*/, int priority/*=0*/)
{
	NPLMessage_ptr msg(new NPLMessage());
	msg->m_filename = filepath;
	if (code != 0)
	{
		if (nLength > 0)
		{
			msg->m_code.clear();
			msg->m_code.append(code, nLength);
		}
		else
		{
			msg->m_code = code;
		}
	}
	// insert to the input message queue
	return Activate_async(msg, priority);
}

NPL::NPLReturnCode NPL::CNPLRuntimeState::Loadfile_async(const string & filepath, int priority /*= 0*/)
{
	NPLMessage_ptr msg(new NPLMessage());
	msg->m_filename = filepath;
	msg->m_type = NPL::MSG_TYPE_FILE_LOAD;
	// insert to the input message queue
	return Activate_async(msg, priority);
}

NPL::NPLReturnCode NPL::CNPLRuntimeState::ActivateLocal(const char* filepath, const char * code /*= NULL*/, int nLength/*=0*/, int priority/*=0*/)
{
	NPLMessage_ptr msg(new NPLMessage());
	if (filepath != 0) {
		msg->m_filename = filepath;
	}

	if (code != 0)
	{
		if (nLength > 0)
		{
			msg->m_code.clear();
			msg->m_code.append(code, nLength);
		}
		else
		{
			msg->m_code = code;
		}
	}
	// insert to the input message queue
	return Activate_async(msg, priority);
}

NPL::NPLReturnCode NPL::CNPLRuntimeState::Activate_async(NPLMessage_ptr& msg, int priority)
{
	// insert to the input message queue
	return SendMessage(msg, priority);
}

int NPL::CNPLRuntimeState::activate(const char * sNPLFilename, const char* sCode, int nCodeLength, int priority, int reliability)
{
	return NPL::CNPLRuntime::GetInstance()->Activate(this, sNPLFilename, sCode, nCodeLength, priority, reliability);
}

NPL::NPLReturnCode NPL::CNPLRuntimeState::SendMessage(NPLMessage_ptr& msg, int priority/*=0*/)
{
	// insert to the input message queue
	if (priority <= 0)
	{
		// normal priority, push to back
		return (m_input_queue.try_push(msg) != CNPLMessageQueue::BufferOverFlow) ? NPL_OK : NPL_QueueIsFull;
	}
	else
	{
		// high priority, push to front
		m_input_queue.push_front(msg);
		return NPL_OK;
	}
	return NPL_OK;
}

const char* NPL::CNPLRuntimeState::GetCurrentMsg()
{
	return m_current_msg;
}

int NPL::CNPLRuntimeState::GetCurrentMsgLength()
{
	return m_current_msg_length;
}

void NPL::CNPLRuntimeState::SetCurrentMessage(const char* msg, int nLength)
{
	m_current_msg = msg;
	m_current_msg_length = nLength;
}


int NPL::CNPLRuntimeState::GetTimerCount()
{
	ParaEngine::Lock lock(m_mutex);
	return (int)m_activeTimers.size();
}

bool NPL::CNPLRuntimeState::SetTimer(int nIDEvent, float fElapse, const char* sNeuronFile)
{
	if (nIDEvent > 0 && sNeuronFile != NULL)
	{
		string sFileName, sCode;
		NPLHelper::DevideString(sNeuronFile, sFileName, sCode);

		ParaEngine::Lock lock(m_mutex);
		m_activeTimers[nIDEvent] = NPLTimer_ptr(new NPLTimer(sFileName, sCode, fElapse));
	}
	return false;
}

bool NPL::CNPLRuntimeState::KillTimer(int nIDEvent)
{
	ParaEngine::Lock lock(m_mutex);
	if (nIDEvent > 0)
	{
		m_activeTimers.erase(nIDEvent);
		return true;
	}
	else
	{
		// erase all timers
		m_activeTimers.clear();
	}
	return false;
}

bool NPL::CNPLRuntimeState::ChangeTimer(int nIDEvent, int dueTime, int period)
{
	if (nIDEvent > 0)
	{
		ParaEngine::Lock lock(m_mutex);
		NPLTimer_Pool_Type::iterator it = m_activeTimers.find(nIDEvent);
		if (it != m_activeTimers.end())
		{
			it->second->Change(dueTime, period);
			return true;
		}
	}
	return false;
}

int NPL::CNPLRuntimeState::TickTimers(DWORD nTickCount)
{
	// the main runtime state is processed in the main game thread. 
	{
		// in case the structure is modified by other threads or during processing, we will first dump to a temp queue and then process from the queue. 
		ParaEngine::Lock lock_(m_mutex);
		NPLTimer_Pool_Type::iterator iter, iter_end = m_activeTimers.end();
		for (iter = m_activeTimers.begin(); iter != iter_end; ++iter)
		{
			m_temp_timer_pool.push_back(iter->second);
		}
	}

	int nCount = (int)(m_temp_timer_pool.size());
	if (nCount > 0)
	{
		NPLTimer_TempPool_Type::iterator itCur, itEnd = m_temp_timer_pool.end();
		for (itCur = m_temp_timer_pool.begin(); itCur != itEnd; ++itCur)
		{
			(*itCur)->Tick(shared_from_this(), nTickCount);
		}
		m_temp_timer_pool.clear();
	}

	return nCount;
}

void NPL::CNPLRuntimeState::LoadNPLState()
{
	// load common lib
	LoadNPLLib();
	// set the runtime
	SetRuntimeState(shared_from_this());
}

NPL::INPLRuntime* NPL::CNPLRuntimeState::GetNPLRuntime()
{
	return (NPL::INPLRuntime*)(ParaEngine::CGlobals::GetNPLRuntime());
}

void NPL::CNPLRuntimeState::WriteLog(const char* text, int nTextLen/*=0*/, int nLogType /*= 0*/)
{
	if (text == 0)
		return;
	if (nTextLen <= 0)
	{
		nTextLen = strlen(text);
	}
	if (nLogType == 0)
	{
		ParaEngine::CLogger::GetSingleton().Write(text, nTextLen);
	}
	else
	{
		ParaEngine::CLogger::GetSingleton().WriteFormated_WithTime("%s", text);
	}
}

std::string& NPL::CNPLRuntimeState::GetStringBuffer(int nIndex /*= 0*/)
{
	if (nIndex < (int)m_string_buffers.size())
		return m_string_buffers[nIndex];
	else
	{
		m_string_buffers.resize(nIndex + 1);
		return m_string_buffers[nIndex];
	}
}

int NPL::CNPLRuntimeState::GetMsgQueueSize()
{
	return m_input_queue.capacity();
}

void NPL::CNPLRuntimeState::SetMsgQueueSize(int nSize /*= 500*/)
{
	if (nSize > 0)
	{
		m_input_queue.set_capacity(nSize);
	}
}

void NPL::CNPLRuntimeState::RegisterFile(const char* sFilename_, INPLActivationFile* pFileHandler /*= NULL*/)
{
	std::string sFilename = sFilename_;
	auto iter = m_act_files_cpp.find(sFilename);
	if (iter != m_act_files_cpp.end())
	{
		if (iter->second == pFileHandler)
			return;
		else {
			SAFE_RELEASE(iter->second);
			m_act_files_cpp.erase(iter);
		}
	}

	if (pFileHandler)
	{
		pFileHandler->addref();
		m_act_files_cpp[sFilename] = pFileHandler;
	}
}

const std::string& NPL::CNPLRuntimeState::GetIdentifier()
{
	return m_name;
}

void NPL::CNPLRuntimeState::call(const char * sNPLFilename, const char* sCode, int nCodeLength /*= 0*/)
{
	NPL::NPLFileName filename(sNPLFilename);
	LoadFile_any(filename.sRelativePath, false, 0, true);
	ActivateFile_any(filename.sRelativePath, sCode, nCodeLength);
}

void NPL::CNPLRuntimeState::WaitForMessage(int nMessageCount)
{
	m_input_queue.wait(nMessageCount);
}

NPL::NPLMessage_ptr NPL::CNPLRuntimeState::PeekMessage(int nIndex)
{
	return m_input_queue.peek(nIndex);
}

NPL::NPLMessage_ptr NPL::CNPLRuntimeState::PopMessageAt(int nIndex)
{
	NPLMessage_ptr msg;
	m_input_queue.try_pop_at(nIndex, msg);
	return msg;
}

int NPL::CNPLRuntimeState::InstallFields(ParaEngine::CAttributeClass* pClass, bool bOverride)
{
	using namespace ParaEngine;
	IAttributeFields::InstallFields(pClass, bOverride);
	pClass->AddField("ProcessedMsgCount", FieldType_Int, (void*)0, (void*)GetProcessedMsgCount_s, NULL, NULL, bOverride);
	pClass->AddField("CurrentQueueSize", FieldType_Int, (void*)0, (void*)GetCurrentQueueSize_s, NULL, NULL, bOverride);
	pClass->AddField("TimerCount", FieldType_Int, (void*)0, (void*)GetTimerCount_s, NULL, NULL, bOverride);
	pClass->AddField("MsgQueueSize", FieldType_Int, (void*)SetMsgQueueSize_s, (void*)GetMsgQueueSize_s, NULL, NULL, bOverride);
	pClass->AddField("HasDebugHook", FieldType_Bool, (void*)0, (void*)HasDebugHook_s, NULL, NULL, bOverride);
	pClass->AddField("IsPreemptive", FieldType_Bool, (void*)0, (void*)IsPreemptive_s, NULL, NULL, bOverride);
	pClass->AddField("PauseAllPreemptiveFunction", FieldType_Bool, (void*)PauseAllPreemptiveFunction_s, (void*)IsAllPreemptiveFunctionPaused_s, NULL, NULL, bOverride);
	pClass->AddField("filename", FieldType_String, (void*)0, (void*)GetFileName_s, NULL, NULL, bOverride);
	pClass->AddField("DebugTraceLevel", FieldType_Int, (void*)SetDebugTraceLevel_s, (void*)GetDebugTraceLevel_s, NULL, NULL, bOverride);
	return S_OK;
}


