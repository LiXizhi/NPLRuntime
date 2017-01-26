//-----------------------------------------------------------------------------
// Class:	CNPLScriptingState
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.22
// Desc:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "util/os_calls.h"
#ifndef PARAENGINE_MOBILE
#include "util/dl_malloc_include.h"
#endif

#include "NPLStateMemAllocator.h"
#include "ParaScriptingNPL.h"
#include "NPLScriptingState.h"
#include "NPLRuntimeState.h"

/** @def if defined. we will use the luajit recommended way to open lua states and load library. */
// #define USE_LUAJIT

/** @def this file is loaded before compiling */
#define NPL_META_COMPILER_SRC  "script/ide/System/Compiler/nplc.lua"

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

#include "memdebug.h"

string ParaScripting::CNPLScriptingState::m_searchpath;

/** forcing some lua function symbols to be exported even when /opt:ref is used.
This makes decoda lua debugger work for lua script debugging with PDB file. */
PE_CORE_DECL void ForceExportSymbols()
{
	luaL_ref(0, 0);
	luaL_unref(0, 0, 0);
	lua_cpcall(0, 0, 0);
}

/** define this to enable debugging of NPL code in visual studio */
//#define DEBUG_NPL_GLIA_FILE

/** @def all sCode are run in protected mode. */
#define DOSTRING_IN_PROTECTED_MODE

/** @def the max sCode length that can be sent in the package */
#define MAX_CODE_LENGTH	4096

/** @def max glia packet size */
#define MAX_ACTIVATION_PACKET_SIZE (MAX_CODE_LENGTH+1024)

/** max allowed timer due time */
#define MAX_TIMER_DUE_TIME		10000000

/* list of files */
#define FILE_SHARE							0
#define FILE_NPLWEBSERVICECLIENT			1

// TODO:  find a better way
#ifndef MAX_FILENAME_LEN
#define MAX_FILENAME_LEN	256
#endif

/** split registration to save compiling time.
void register_part1(class_<X>& x){
x.def(...);
}
void register_part2(class_<X>& x){
x.def(...);
}
void register_(lua_State* L){
class_<X> x("x");
register_part1(x);
register_part2(x);
module(L) [ x ];
}
*/

ParaScripting::CNPLScriptingState::CNPLScriptingState(bool bCreateState)
	:m_nStackSize(-1), m_pState(NULL), m_bOwnLuaState(bCreateState), m_nLastReturnValue(0),
#ifdef WIN32
m_nMemAllocatorType(MEM_ALLOC_TYPE_DL_MALLOC),
#else
// Xizhi: luajit2 requires using luaL_newstate instead of lua_newstate on 64bits system. so I disabled custom allocator for all linux version. 
m_nMemAllocatorType(MEM_ALLOC_TYPE_SYS_MALLOC),
#endif
m_pMemAlloc(NULL)
{
	bool bIs64Bits = sizeof(void*) > 4;
	if (bIs64Bits)
	{
		// Xizhi: luajit2 requires using luaL_newstate instead of lua_newstate on 64bits system. so I disabled custom allocator for all linux and 64bits version. 
		m_nMemAllocatorType = MEM_ALLOC_TYPE_SYS_MALLOC; 
	}
	if (bCreateState)
	{
		CreateSetState();
	}
}


ParaScripting::CNPLScriptingState::~CNPLScriptingState(void)
{
	PE_ASSERT(m_stack_current_file.size() == 0);
	DestroyState();

	if (m_nMemAllocatorType == MEM_ALLOC_TYPE_POOL_MALLOC)
	{
		SAFE_DELETE(m_pMemAlloc);
	}
#ifndef PARAENGINE_MOBILE
	else if (m_nMemAllocatorType == MEM_ALLOC_TYPE_DL_MALLOC)
	{
		if (m_mspace != 0)
		{
			int nBytesUsed = destroy_mspace(m_mspace);
			OUTPUT_LOG("\t\tNPL thread local memory allocator released %d bytes\n", nBytesUsed);
			m_mspace = 0;
		}
	}
#endif
}

int ParaScripting::CNPLScriptingState::GetLastReturnValue() const
{
	return m_nLastReturnValue;
}

void ParaScripting::CNPLScriptingState::SetLastReturnValue(int val)
{
	m_nLastReturnValue = val;
}

void ParaScripting::CNPLScriptingState::DestroyState()
{
	if (m_pState != NULL)
	{
		m_loaded_files.clear();

		if (m_bOwnLuaState)
			lua_close(m_pState);
		m_pState = NULL;
	}
}

bool ParaScripting::CNPLScriptingState::CreateSetState(lua_State* pLuaState)
{
	if (m_pState != NULL)
		DestroyState();
	if (pLuaState == NULL)
	{
#if !defined(PARAENGINE_MOBILE)
		if (m_nMemAllocatorType == MEM_ALLOC_TYPE_POOL_MALLOC)
		{
			if (m_pMemAlloc == 0)
			{
				m_pMemAlloc = new NPL::CNPLStateMemAllocator();
			}
			m_pState = lua_newstate(NPL::npl_mem_alloc, m_pMemAlloc);
		}
		else if (m_nMemAllocatorType == MEM_ALLOC_TYPE_DL_MALLOC)
		{
			if (m_mspace == 0)
				m_mspace = create_mspace(0, 0);
			m_pState = lua_newstate(NPL::npl_mem_dl_alloc, m_mspace);
		}
		else
#endif
		{
			m_pState = luaL_newstate();
		}
		SetOwnLuaState(true);
	}
	else
	{
		m_pState = pLuaState;
		SetOwnLuaState(false);
	}

	if (m_pState == NULL)
		return false;

	LoadLuabind();
	/// make this a reasonable size
	if (m_nStackSize > 0)
		lua_checkstack(m_pState, m_nStackSize);

	if (!pLuaState)
	{
		luaL_openlibs(m_pState);
		//#ifdef USE_LUAJIT
		//	luaL_openlibs(m_pState);
		//#else
		//	/// enable base functions
		//	luaopen_base(m_pState);
		//	/// packages
		//	luaopen_package(m_pState);
		//	/// enable string functions in the script file.
		//	luaopen_string(m_pState);
		//	/// enable math functions.
		//	luaopen_math(m_pState);
		//	/// open the debugging lib
		//	luaopen_debug(m_pState);
		//	/// open the table lib
		//	luaopen_table(m_pState);
		//	
		//#endif
	}
	return true;
}

class PointerWrapper
{
public:
	PointerWrapper(){};
	PointerWrapper(void * pointer) :m_pointer(pointer){};
	~PointerWrapper(){};
	void * m_pointer;
};

void ParaScripting::CNPLScriptingState::LoadLuabind()
{
	if (m_pState != NULL)
	{
		using namespace luabind;
		luabind::open(m_pState);
	}
}

lua_State* ParaScripting::CNPLScriptingState::GetLuaState()
{
	return m_pState;
}

bool ParaScripting::CNPLScriptingState::IsValid()
{
	return (m_pState != NULL);
}

void ParaScripting::CNPLScriptingState::LoadNPLLib()
{
	if (m_pState == 0)
	{
		OUTPUT_LOG("can not load modules to NPL state because it is not yet initialized\n");
		return;
	}

	/// load NPL
	LoadHAPI_NPL();
}

void ParaScripting::CNPLScriptingState::LoadParaLib()
{
	if (m_pState == 0)
	{
		OUTPUT_LOG("can not load modules to NPL state because it is not yet initialized\n");
		return;
	}
	/// load all Para libs
	LoadHAPI_Globals();
	LoadHAPI_SceneManager();
	LoadHAPI_ResourceManager();
	LoadHAPI_UI();
	LoadHAPI_Audio();
	LoadHAPI_Network();
	LoadHAPI_AI();
}


void ParaScripting::CNPLScriptingState::ProcessResult(int nResult)
{
	lua_State* L = m_pState;
	if (nResult != 0)
	{
		// read error message on stack
		string strErrorMsg;
		const char* errorMsg = lua_tostring(L, -1);
		if (errorMsg != NULL) {
			strErrorMsg = errorMsg;
		}

		/// report error to log;
		switch (nResult) {
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
		// remove the error message from the stack.
		lua_pop(L, 1);
		OUTPUT_LOG("%s", strErrorMsg.c_str());

		/* TODO: do this in a thread-safe way
		if(ParaEngine::CGlobals::WillGenReport())
		{
		ParaEngine::CGlobals::GetReport()->SetString("Script Error", strErrorMsg.c_str());
		}*/
	}
}

int ParaScripting::CNPLScriptingState::GetNPLCodeFromFile(ParaEngine::CParaFile* pFile, char** pBuffer, int* pBufferSize)
{
	int nEncodingCode = 0;
	if (pFile)
	{
		int nSize = pFile->getSize();
		char* buf = pFile->getBuffer();

		if (nSize >= 2)
		{
			if ((((byte)buf[0]) == 0xEF) && (((byte)buf[1]) == 0xBB) && (((byte)buf[2]) == 0xBF))
			{
				buf += 3;
				nSize -= 3;
				nEncodingCode = CP_UTF8;
			}
			else if (((((byte)buf[0]) == 0xFF) && (((byte)buf[1]) == 0xFE)) || ((((byte)buf[0]) == 0xFE) && (((byte)buf[1]) == 0xFF)))
			{
				buf += 2;
				nSize -= 2;
				nEncodingCode = CP_UTF8;
			}
		}
		if (pBuffer)
			*pBuffer = buf;
		if (pBufferSize)
			*pBufferSize = nSize;
	}
	return nEncodingCode;
}

uint32 ParaScripting::CNPLScriptingState::GetScriptDiskPath(const string& filePath, string& sFileName)
{
#ifdef PARAENGINE_MOBILE
	uint32 dwFound = FILE_NOT_FOUND;
	// for mobile version, always use source version first then compiled version in bin/ directory. 
	if (!(dwFound = ParaEngine::CParaFile::DoesFileExist2(filePath.c_str(), (FILE_ON_ZIP_ARCHIVE | FILE_ON_DISK | FILE_ON_SEARCH_PATH))))
	{
		// then check if there is bin/*.o in zip archive, this is NPL convention
		sFileName = std::string("bin/") + filePath;
		sFileName = CParaFile::ChangeFileExtension(sFileName, "o");
		if (!(dwFound = ParaEngine::CParaFile::DoesFileExist2(sFileName.c_str(), FILE_ON_ZIP_ARCHIVE)))
		{
			// finally, check if there *.luac file in the same directory 
			// (this is cocos' convention)
			sFileName = CParaFile::ChangeFileExtension(filePath, "luac");
			if (!(dwFound = ParaEngine::CParaFile::DoesFileExist2(sFileName.c_str(), (FILE_ON_ZIP_ARCHIVE | FILE_ON_DISK | FILE_ON_SEARCH_PATH))))
			{
				sFileName = filePath;
			}
		}
	}
	else
	{
		sFileName = filePath;
	}
#else
	uint32 dwFound = FILE_NOT_FOUND;

	if (!CParaFile::GetDevDirectory().empty())
	{
		dwFound = ParaEngine::CParaFile::DoesFileExist2(filePath.c_str(), FILE_ON_DISK | FILE_ON_SEARCH_PATH);
		if (dwFound)
		{
			sFileName = filePath;
			return dwFound;
		}
	}

	// 2008.5.7: we will first find if there is an up to date compiled version in the bin directory. if there is, 
	// we will load the compiled version, otherwise we will use the text version. 
	sFileName = std::string("bin/") + filePath;
	sFileName = CParaFile::ChangeFileExtension(sFileName, "o");

	if ((dwFound = ParaEngine::CParaFile::DoesFileExist2(sFileName.c_str(), FILE_ON_ZIP_ARCHIVE | FILE_ON_DISK | FILE_ON_SEARCH_PATH)))
	{
		time_t srcTime, binTime;
		uint32 dwFoundSrc = FILE_NOT_FOUND;
		if ((dwFoundSrc = ParaEngine::CParaFile::DoesFileExist2(filePath.c_str(), FILE_ON_ZIP_ARCHIVE | FILE_ON_DISK | FILE_ON_SEARCH_PATH))
			&& (!ParaEngine::GetLastFileWriteTime(filePath.c_str(), srcTime) || !ParaEngine::GetLastFileWriteTime(sFileName.c_str(), binTime) || (srcTime > binTime)))
		{
			// use src version, if source version exist and up to date. 
			sFileName = filePath;
			dwFound = dwFoundSrc;
		}
	}
	else
	{
		dwFound = ParaEngine::CParaFile::DoesFileExist2(filePath.c_str(), FILE_ON_ZIP_ARCHIVE | FILE_ON_DISK | FILE_ON_SEARCH_PATH);
		sFileName = filePath;
	}
#endif
	return dwFound;
}

bool ParaScripting::CNPLScriptingState::IsScriptFileLoaded(const string& filepath)
{
	return (m_loaded_files.find(filepath) != m_loaded_files.end());
}

void ParaScripting::CNPLScriptingState::PushFileName(const string& filename)
{
	m_stack_current_file.push(filename);
}

void ParaScripting::CNPLScriptingState::PopFileName()
{
	m_stack_current_file.pop();
}

const string& ParaScripting::CNPLScriptingState::GetFileName()
{
	return !m_stack_current_file.empty() ? m_stack_current_file.top() : CGlobals::GetString(0);
}

bool ParaScripting::CNPLScriptingState::LoadFile(const string& filePath, bool bReload)
{
	if (m_pState == NULL)
		return false;
	if (filePath.empty())
		return true;
	bool bLoadedBefore = IsScriptFileLoaded(filePath);
	if (!bLoadedBefore || bReload)
	{
		// if the file is not loaded or reload it true, try loading the glia file first.
		// only find in local files using ParaFile interface
		string sFileName;
		uint32 dwFound = GetScriptDiskPath(filePath, sFileName);

		ParaEngine::CParaFile file;
		if (dwFound && file.OpenFile(sFileName.c_str(), true, NULL, false, dwFound))
		{
			// if the file is not loaded before, add a new GliaFile with the name filePath to the loaded glia file list.
			// this is done before the file is actually loaded to prevent recursive loading, which may lead to C stack overflow.
			if (!bLoadedBefore)
			{
				m_loaded_files.insert(filePath);
			}
			char* codebuf = NULL;
			int codesize = 0;
			GetNPLCodeFromFile(&file, &codebuf, &codesize);

			if (codesize>0)
			{
				CFileNameStack pushStack(this, filePath);
				int nSize = (int)sFileName.size();
				if (nSize > 5 && sFileName[nSize - 4] == '.' && sFileName[nSize - 3] == 'n' && sFileName[nSize - 2] == 'p' && sFileName[nSize - 1] == 'l')
				{
					// for *.npl file, invoke meta-compiler NPL.loadstring(code, filename) first. 
					if (!IsScriptFileLoaded(NPL_META_COMPILER_SRC))
					{
						if (!LoadFile(NPL_META_COMPILER_SRC, false))
						{
							OUTPUT_LOG("warning: NPL meta compiler not found. \n");
							return false;
						}
					}

					const char actTable[] = "NPL";
					lua_pushlstring(m_pState, actTable, sizeof(actTable) - 1);
					lua_gettable(m_pState, LUA_GLOBALSINDEX);
					if (lua_istable(m_pState, -1))
					{
						std::string funcName = "loadstring";
						lua_pushlstring(m_pState, funcName.c_str(), funcName.size());
						lua_gettable(m_pState, -2);
						if (lua_isfunction(m_pState, -1))
						{
							int top = lua_gettop(m_pState);
							lua_pushlstring(m_pState, codebuf, codesize);
							lua_pushlstring(m_pState, filePath.c_str(), filePath.size());
							// call the function with 2 arguments and multi result, with no error handling routine
							int nResult = lua_pcall(m_pState, 2, LUA_MULTRET, 0);
							int num_results = lua_gettop(m_pState) - top + 1;
							if (nResult == 0 && num_results > 0)
							{
								int top = lua_gettop(m_pState);
								if (lua_isfunction(m_pState, -1)) 
								{
									nResult = lua_pcall(m_pState, 0, LUA_MULTRET, 0);
									int num_results = lua_gettop(m_pState) - top + 1;
									if (nResult == 0 && num_results > 0)
									{
										lua_pop(m_pState, num_results);
									}
								}
								else 
								{
									lua_pop(m_pState, num_results);
								}
							}
							ProcessResult(nResult);
						}
						else
						{
							OUTPUT_LOG("warning: no NPL.loadstring function not found when compiling %s\n", filePath.c_str());
							lua_pop(m_pState, 1);
						}
					}
					// pops the element, so that the stack is balanced.
					lua_pop(m_pState, 1);
				}
				else
				{
					/** for standard lua file, Load and execute the a buffer of code in protected mode ( lua_pcall() )
					Output messages through log interface */

					int nResult = luaL_loadbuffer(m_pState, codebuf, codesize, filePath.c_str());
					if (nResult == 0) {
						nResult = lua_pcall(m_pState, 0, LUA_MULTRET, 0);
					}
					ProcessResult(nResult);
				}
			}
		}
		else
		{
			OUTPUT_LOG("warning: script file %s not found\n", sFileName.c_str());
			return false;
		}
	}
	return true;
}

int ParaScripting::CNPLScriptingState::DoString(const char* sCall, int nLength, const char* sFileName, bool bPopReturnValue)
{
	if (m_pState == NULL)
	{
		return 0;
	}

	/// do sCall as a string
	/// Assume caller is authorized, since it is from the local environment. This may lack security, and is only valid for authorized callers. 
	int nResult = 1;
	int nReturnValue = 0;
	if ((sCall != NULL) && sCall[0] != '\0')
	{
#ifdef DOSTRING_IN_PROTECTED_MODE
		/** Load and execute the a buffer of code in protected mode ( lua_pcall() )
		Output messages through log interface */

		if (nLength <= 0)
		{
			nLength = strnlen(sCall, MAX_CODE_LENGTH);
			if (nLength >= MAX_CODE_LENGTH)
			{
				nResult = LUA_ERRRUN;
				OUTPUT_LOG("warning: sCode exceed the length limit of %d\r\n", MAX_CODE_LENGTH);
				nLength = 0; // prevent the sCode to be executed. 
			}
		}

		if (nLength>0)
		{
			nResult = luaL_loadbuffer(m_pState, sCall, nLength, sFileName);
			if (nResult == 0) {
				int top = lua_gettop(m_pState);
				nResult = lua_pcall(m_pState, 0, LUA_MULTRET, 0);
				int num_results = lua_gettop(m_pState) - top + 1;
				if (nResult == 0 && num_results > 0)
				{
					if (num_results == 1)
					{
						// check for return value
						if (lua_isnumber(m_pState, -1))
						{
							nReturnValue = (int)lua_tonumber(m_pState, -1);
							SetLastReturnValue(nReturnValue);
						}
					}
					if (bPopReturnValue)
						lua_pop(m_pState, num_results);
				}
			}

			ProcessResult(nResult);
		}
#else
		// Note: use loadbuffer and pcall instead of lua_dostring() is safer.
		nResult = lua_dostring(L, sCall);
#endif
		if (nResult != 0)
		{
			OUTPUT_LOG("Error in sCode:%s\n", sCall);
			// error
			nResult = 0;
		}
	}
	return nReturnValue;
}

NPL::NPLReturnCode ParaScripting::CNPLScriptingState::ActivateFile(const string& filepath, const char * code /*= NULL*/, int nLength/*=0*/)
{
	if (m_pState == NULL)
		return NPL::NPL_RuntimeState_NotReady;
	DoString(code, nLength);

	NPL::NPLReturnCode nRes = NPL::NPL_OK;
	if (filepath.empty())
	{
		return nRes;
	}

	lua_State* L = m_pState;

	/** the activation function is stored in __glia global variable,
	* indexed by the the file name. There activation function may be nil.  */

	/** get the "__act" global table in the runtime */
	const char actTable[] = "__act";
	lua_pushlstring(L, actTable, sizeof(actTable)-1);
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (lua_istable(L, -1))
	{
		/** get activation function of the file name*/
		lua_pushlstring(L, filepath.c_str(), filepath.size());
		lua_gettable(L, -2);
		if (lua_isfunction(L, -1))
		{
			int top = lua_gettop(L);
			// call the function with 0 arguments and 0 result, with no error handling routine
			int nResult = lua_pcall(L, 0, LUA_MULTRET, 0);
			int num_results = lua_gettop(L) - top + 1;
			if (nResult == 0 && num_results > 0)
			{
				// check for return value
				if (num_results == 1)
				{
					int nReturnValue = 0;
					if (lua_isnumber(L, -1))
					{
						nReturnValue = (int)lua_tonumber(L, -1);
						SetLastReturnValue(nReturnValue);
					}
				}
				
				lua_pop(L, num_results);
			}
			ProcessResult(nResult);

			if (nResult != 0){
				// error
				nRes = NPL::NPL_FailedToLoadFile;
			}
			// else {//success}
		}
		else
		{
			OUTPUT_LOG("warning: no activate function found for %s\n", filepath.c_str());
			lua_pop(L, 1);
		}
	}
	// else {//the file does not contain any activate function. So we will do nothing.}

	// pops the element, so that the stack is balanced.
	lua_pop(L, 1);

	return nRes; // success
}

bool ParaScripting::CNPLScriptingState::BindFileActivateFunc(const luabind::object& funcActivate, const std::string& filename)
{
	if (type(funcActivate) == LUA_TFUNCTION)
	{
		object tabGlobal = luabind::globals(funcActivate.interpreter());
		object tabAct = tabGlobal["__act"];

		/// create the activate table, if it does not exist
		if (!(tabAct.is_valid()) || (type(tabAct) != LUA_TTABLE))
		{
			tabAct = newtable(funcActivate.interpreter());
			tabGlobal["__act"] = tabAct;
		}

		/// add the current file name to the __activate table
		if (!filename.empty())
			tabAct[filename] = funcActivate;
		else
			tabAct[GetFileName()] = funcActivate;
		return true;
	}
	return false;
}

//namespace luabind
//{
//	template <>
//	struct default_converter<uint64>
//		: native_converter_base<uint64>
//	{
//		static int compute_score(lua_State* L, int index)
//		{
//			return lua_type(L, index) == LUA_TNUMBER ? 0 : -1;
//		}
//
//		uint64 from(lua_State* L, int index)
//		{
//			return (uint64)(lua_tonumber(L, index));
//		}
//
//		void to(lua_State* L, uint64 const& x)
//		{
//			lua_pushnumber(L, x);
//		}
//	};
//
//	template <>
//	struct default_converter<uint64 const&>
//		: default_converter<uint64>
//	{};
//}

NPL::NPLRuntimeState_ptr ParaScripting::CNPLScriptingState::GetRuntimeStateFromLuaObject(const luabind::object& obj)
{
	// read from registry
	lua_State * L = obj.interpreter();
	const char rts_name[] = "__rts__";
	lua_pushlstring(L, rts_name, sizeof(rts_name)-1);
	lua_rawget(L, LUA_REGISTRYINDEX);
	NPL::CNPLRuntimeState* pState = (NPL::CNPLRuntimeState*)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return (pState) ? pState->shared_from_this() : NPL::NPLRuntimeState_ptr();
}

void ParaScripting::CNPLScriptingState::SetRuntimeState(NPL::NPLRuntimeState_ptr runtime_state)
{
	luabind::object tabGlobal = luabind::globals(m_pState);
	// save the runtime state (rts) to a hidden variable. 
	tabGlobal["__rts__"] = ParaNPLRuntimeState(runtime_state);

	// save void* to registry for access later on. 
	lua_pushstring(m_pState, "__rts__");
	lua_pushlightuserdata(m_pState, (void*)(runtime_state.get()));
	lua_rawset(m_pState, LUA_REGISTRYINDEX);
}

void ParaScripting::CNPLScriptingState::AddSearchPath(const char* sSearchPath_, bool bIsAdding /*= true*/)
{
	std::string sSearchPath = sSearchPath_;
	if (sSearchPath.size() > 0 && sSearchPath[sSearchPath.size() - 1] != '/')
		sSearchPath += "/";
	m_searchpath = bIsAdding ? sSearchPath : "";
}

void ParaScripting::CNPLScriptingState::SetOwnLuaState(bool bOwn)
{
	m_bOwnLuaState = bOwn;
}
