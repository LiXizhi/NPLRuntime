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
#include "util/regularexpression.h"
#include "NPLRuntimeState.h"


/** @def if defined. we will use the luajit recommended way to open lua states and load library. */
// #define USE_LUAJIT

/** @def this file is loaded before compiling */
#define NPL_META_COMPILER_SRC  "script/ide/System/Compiler/nplc.lua"

/** @def file module is just started being loaded to prevent cyclic reference. */
#define NPL_FILE_MODULE_START_LOADING  -1000
/** @def the given file module is not loaded. */
#define NPL_FILE_MODULE_NOT_LOADED  -999
/** @def the given file module is not found. */
#define NPL_FILE_MODULE_NOT_FOUND  -998

/** global npl table to cache all file modules */
const char _file_mod_[] = "_file_mod_";

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
#ifdef EMSCRIPTEN_SINGLE_THREAD
#define auto_ptr unique_ptr
#endif
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
	:m_nStackSize(-1), m_pState(NULL), m_bOwnLuaState(bCreateState), m_nLastReturnValue(0), m_nDebugTraceLevel(0),
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

int ParaScripting::CNPLScriptingState::GetDebugTraceLevel() const
{
	return m_nDebugTraceLevel;
}

void ParaScripting::CNPLScriptingState::SetDebugTraceLevel(int val)
{
	m_nDebugTraceLevel = val;
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
	PointerWrapper() {};
	PointerWrapper(void * pointer) :m_pointer(pointer) {};
	~PointerWrapper() {};
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
#ifdef CROSS_PLATFORM
	LoadHAPI_WebView();
	LoadHAPI_Platform();
#endif
}

int ParaScripting::CNPLScriptingState::Traceback(lua_State *L)
{
	if (!lua_isstring(L, 1)) { /* Non-string error object? Try metamethod. */
		if (lua_isnoneornil(L, 1) ||
			!luaL_callmeta(L, 1, "__tostring") ||
			!lua_isstring(L, -1))
			return 1;  /* Return non-string error object. */
		lua_remove(L, 1);  /* Replace object by result of __tostring metamethod. */
	}

	// get the "_npl_traceback" global function in the runtime 
	const char _npl_traceback[] = "_npl_traceback";
	lua_pushlstring(L, _npl_traceback, sizeof(_npl_traceback) - 1);
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (lua_isfunction(L, -1))
	{
		lua_insert(L, -2); // put the function before error message string
		// call the function with 1 arguments and 1 result
		int nResult = lua_pcall(L, 1, 1, 0);
		if (nResult != 0)
		{
			const char* errorMsg = lua_tostring(L, -1);
			if (errorMsg != NULL) {
				OUTPUT_LOG("%s <Runtime error>\r\n", errorMsg);
			}
		}
	}
	else
	{
		lua_pop(L, 1);
	}
	return 1;
}

int ParaScripting::CNPLScriptingState::Lua_ProtectedCall(lua_State *L, int nargs, int nresults)
{
	if (GetDebugTraceLevel() > 0)
	{
		int status;
		int base = lua_gettop(L) - nargs;  /* function index */
		lua_pushcfunction(L, ParaScripting::CNPLScriptingState::Traceback);  /* push traceback function */
		lua_insert(L, base);  /* put it under chunk and args */

		status = lua_pcall(L, nargs, nresults, base);

		lua_remove(L, base);  /* remove traceback function */
		/* force a complete garbage collection in case of errors */
		// if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
		return status;
	}
	else
		return lua_pcall(L, nargs, nresults, 0);
}

void ParaScripting::CNPLScriptingState::ProcessResult(int nResult, lua_State* L)
{
	if (L == 0)
		L = m_pState;
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

		// get the "__npl_error_callback" global function in the runtime 
		const char error_callback[] = "__npl_error_callback";
		lua_pushlstring(L, error_callback, sizeof(error_callback) - 1);
		lua_gettable(L, LUA_GLOBALSINDEX);
		if (lua_isfunction(L, -1))
		{
			// push error message 
			lua_pushlstring(L, strErrorMsg.c_str(), strErrorMsg.size());
			// call the function with 1 arguments and 0 result
			int nResult = lua_pcall(L, 1, 0, 0);
			if (nResult != 0)
			{
				const char* errorMsg = lua_tostring(L, -1);
				if (errorMsg != NULL) {
					OUTPUT_LOG("%s <Runtime error>\r\n", errorMsg);
				}
				lua_pop(L, 1);
			}
		}
		else
		{
			// pops the element, so that the stack is balanced.
			lua_pop(L, 1);
			// the file does not contain an npl process error function. So we will do nothing.
			OUTPUT_LOG("%s", strErrorMsg.c_str());
		}
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
			if ((((unsigned char)buf[0]) == 0xEF) && (((unsigned char)buf[1]) == 0xBB) && (((unsigned char)buf[2]) == 0xBF))
			{
				buf += 3;
				nSize -= 3;
				nEncodingCode = CP_UTF8;
			}
			else if (((((unsigned char)buf[0]) == 0xFF) && (((unsigned char)buf[1]) == 0xFE)) || ((((unsigned char)buf[0]) == 0xFE) && (((unsigned char)buf[1]) == 0xFF)))
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

	if ((dwFound = ParaEngine::CParaFile::DoesFileExist2(filePath.c_str(), FILE_ON_ZIP_ARCHIVE | FILE_ON_DISK | FILE_ON_SEARCH_PATH)))
	{
		sFileName = filePath;
	}
	else
	{
		// search bin/[path].o version if non-compiled source code version not found. 
		sFileName = std::string("bin/") + filePath;
		sFileName = CParaFile::ChangeFileExtension(sFileName, "o");
		dwFound = ParaEngine::CParaFile::DoesFileExist2(sFileName.c_str(), FILE_ON_ZIP_ARCHIVE | FILE_ON_DISK | FILE_ON_SEARCH_PATH);
	}
#endif
	return dwFound;
}

bool ParaScripting::CNPLScriptingState::IsScriptFileLoaded(const string& filepath)
{
	return (m_loaded_files.find(filepath) != m_loaded_files.end());
}

int ParaScripting::CNPLScriptingState::GetFileLoadStatus(const string& filepath)
{
	auto obj = m_loaded_files.find(filepath);
	return (obj != m_loaded_files.end()) ? obj->second : NPL_FILE_MODULE_NOT_LOADED;
}

void ParaScripting::CNPLScriptingState::SetFileLoadStatus(const string& filepath, int nStatus)
{
	m_loaded_files[filepath] = nStatus;
	std::string sExt = CParaFile::GetFileExtension(filepath);
	if (sExt == "lua")
	{
		m_loaded_files[CParaFile::ChangeFileExtension(filepath, "npl")] = nStatus;
	}
	else if (sExt == "npl")
	{
		m_loaded_files[CParaFile::ChangeFileExtension(filepath, "lua")] = nStatus;
	}
}

const std::map <std::string, int32>& ParaScripting::CNPLScriptingState::GetLoadedFiles()
{
	return m_loaded_files;
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

const char* ParaScripting::CNPLScriptingState::GetCurrentFileName(lua_State* L)
{
	if (!m_stack_current_file.empty())
	{
		return m_stack_current_file.top().c_str();
	}
	else
	{
		if (L == 0)
			L = m_pState;

		lua_Debug ar;
		if (lua_getstack(L, 1, &ar))
		{
			/* check function at level */
			if (lua_getinfo(L, "S", &ar) != 0)
				return ar.source;
		}
		return NULL;
	}
}

bool ParaScripting::CNPLScriptingState::LoadFile(const string& filePath, bool bReload, lua_State* L, bool bNoReturn)
{
	if (m_pState == NULL)
		return false;
	if (filePath.empty())
		return true;
	int nFileStatus = GetFileLoadStatus(filePath);
	bool bLoadedBefore = nFileStatus != NPL_FILE_MODULE_NOT_LOADED;
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
			SetFileLoadStatus(filePath, NPL_FILE_MODULE_START_LOADING);


			char* codebuf = NULL;
			int codesize = 0;
			GetNPLCodeFromFile(&file, &codebuf, &codesize);

			if (codesize > 0)
			{
				CFileNameStack pushStack(this, filePath);
				int nSize = (int)sFileName.size();
				if (nSize > 5 && sFileName[nSize - 4] == '.' && sFileName[nSize - 3] == 'n' && sFileName[nSize - 2] == 'p' && sFileName[nSize - 1] == 'l')
				{
					// for *.npl file, invoke meta-compiler NPL.loadstring(code, filename) first. 
					if (!IsScriptFileLoaded(NPL_META_COMPILER_SRC))
					{
						if (!LoadFile(NPL_META_COMPILER_SRC, false, L))
						{
							OUTPUT_LOG("warning: NPL meta compiler not found. \n");
							return false;
						}
					}

					if (L == 0)
						L = m_pState;
					const char actTable[] = "NPL";
					lua_pushlstring(L, actTable, sizeof(actTable) - 1);
					lua_gettable(L, LUA_GLOBALSINDEX);
					if (lua_istable(L, -1))
					{
						std::string funcName = "loadstring";
						lua_pushlstring(L, funcName.c_str(), funcName.size());
						lua_gettable(L, -2);
						lua_remove(L, -2);
						if (lua_isfunction(L, -1))
						{
							int top = lua_gettop(L);
							lua_pushlstring(L, codebuf, codesize);
							lua_pushlstring(L, filePath.c_str(), filePath.size());
							// call the function with 2 arguments and multi result
							int nResult = Lua_ProtectedCall(L, 2, LUA_MULTRET);
							int num_results = lua_gettop(L) - top + 1;
							if (nResult == 0 && num_results > 0)
							{
								int top = lua_gettop(L);
								if (lua_isfunction(L, -1))
								{
									nResult = Lua_ProtectedCall(L, 0, LUA_MULTRET);
									int num_results = lua_gettop(L) - top + 1;
									if (nResult == 0)
									{
										CacheFileModule(filePath, num_results, L);
										num_results = lua_gettop(L) - top + 1;
										if (bNoReturn && num_results > 0) {
											lua_pop(L, num_results);
										}
									}
								}
								else
								{
									lua_pop(L, num_results);
								}
							}
							ProcessResult(nResult, L);
						}
						else
						{
							lua_pop(L, 1);
							OUTPUT_LOG("warning: no NPL.loadstring function not found when compiling %s\n", filePath.c_str());
						}
					}
					else
					{
						lua_pop(L, 1);
					}
				}
				else
				{
					/** for standard lua file, Load and execute the a buffer of code in protected mode ( lua_pcall() )
					Output messages through log interface */
					if (L == 0)
						L = m_pState;
					int nResult = luaL_loadbuffer(L, codebuf, codesize, filePath.c_str());
					if (nResult == 0) {
						int top = lua_gettop(L);
						nResult = Lua_ProtectedCall(L, 0, LUA_MULTRET);
						int num_results = lua_gettop(L) - top + 1;
						if (nResult == 0)
						{
							CacheFileModule(filePath, num_results, L);
							num_results = lua_gettop(L) - top + 1;
							if (bNoReturn && num_results > 0) {
								lua_pop(L, num_results);
							}
						}
					}
					ProcessResult(nResult, L);
				}
			}
		}
		else
		{
			std::string sExt = CParaFile::GetFileExtension(sFileName);
			if (sExt == "npl" || sExt == "lua")
			{
				if (!dwFound) {
					OUTPUT_LOG("warning: script file %s not found\n", sFileName.c_str());
				}
				else {
					OUTPUT_LOG("warning: script file %s found but can not be opened\n", sFileName.c_str());
				}
			}
			SetFileLoadStatus(filePath, NPL_FILE_MODULE_NOT_FOUND);
			nFileStatus = NPL_FILE_MODULE_NOT_FOUND;
		}
	}
	else
	{
		if (!bNoReturn) {
			PopFileModule(filePath, L);
		}
	}

	if (nFileStatus == NPL_FILE_MODULE_NOT_FOUND)
	{
		if (!bNoReturn)
		{
			// return false to scripting environment if module is not found. 
			if (L == 0)
				L = m_pState;
			lua_pushboolean(L, 0);
		}
		return false;
	}
	return true;
}


int ParaScripting::CNPLScriptingState::CacheFileModule(const std::string& filename, int nResult, lua_State* L)
{
	int nFileStatus = GetFileLoadStatus(filename);
	if (nResult == 0 && (nFileStatus > 0 || nFileStatus == -1))
	{
		// this could happen when user used NPL.export() instead of return for file module.
		return PopFileModule(filename, L);
	}
	SetFileLoadStatus(filename, nResult);
	if (nResult > 0 || nResult == -1)
	{
		if (L == 0)
			L = m_pState;

		int nLastResultIndex = lua_gettop(L);
		// create the _file_mod_ table if not. 
		lua_pushlstring(L, _file_mod_, sizeof(_file_mod_) - 1);
		lua_rawget(L, LUA_REGISTRYINDEX);

		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			lua_pushlstring(L, _file_mod_, sizeof(_file_mod_) - 1);
			lua_newtable(L);
			lua_rawset(L, LUA_REGISTRYINDEX);

			lua_pushlstring(L, _file_mod_, sizeof(_file_mod_) - 1);
			lua_rawget(L, LUA_REGISTRYINDEX);
		}
		if (lua_istable(L, -1))
		{
			int nFilenameLength = filename.size() - ((filename.size() > 4 && filename[filename.size() - 4] == '.') ? 4 : 0);

			if (nResult == 1)
			{
				// cache the object on top of the stack directly
				lua_pushlstring(L, filename.c_str(), nFilenameLength);
				lua_pushvalue(L, nLastResultIndex);
				lua_rawset(L, -3);
			}
			else if (nResult == -1)
			{
				// to resolve cyclic dependency, we will create an empty table 
				lua_pushlstring(L, filename.c_str(), nFilenameLength);
				lua_newtable(L);
				lua_rawset(L, -3);
			}
			else
			{
				// TODO: save to a table{result1, result2, ...}. currently only the first result is cached. 
				lua_pushlstring(L, filename.c_str(), nFilenameLength);
				lua_pushvalue(L, nLastResultIndex - nResult + 1);
				lua_rawset(L, -3);
			}
			lua_pop(L, 1);
		}
	}
	return 0;
}

int ParaScripting::CNPLScriptingState::PopFileModule(const std::string& filename, lua_State* L)
{
	int nFileStatus = GetFileLoadStatus(filename);
	if (nFileStatus != NPL_FILE_MODULE_NOT_LOADED && nFileStatus != NPL_FILE_MODULE_NOT_FOUND)
	{
		int nResultNum = nFileStatus;
		if (nResultNum == NPL_FILE_MODULE_START_LOADING)
		{
			// if cyclic dependency is detected, we will cache an empty table instead. 
			CacheFileModule(filename, -1, L);
			nResultNum = 1;
		}
		else if (nResultNum == -1)
		{
			nResultNum = 1;
		}

		if (nResultNum > 0)
		{
			if (L == 0)
				L = m_pState;
			int nFilenameLength = filename.size() - ((filename.size() > 4 && filename[filename.size() - 4] == '.') ? 4 : 0);

			// get from the _file_mod_ table
			lua_pushlstring(L, _file_mod_, sizeof(_file_mod_) - 1);
			lua_rawget(L, LUA_REGISTRYINDEX);
			if (lua_istable(L, -1))
			{
				lua_pushlstring(L, filename.c_str(), nFilenameLength);
				lua_rawget(L, -2);
				lua_remove(L, -2);

				if (nResultNum > 1)
				{
					// TODO: unpack the table. currently only first result is returned. 
				}
				return 1;
			}
		}
	}
	return 0;
}

std::string ParaScripting::CNPLScriptingState::GetModuleFilePath(const std::string& modulename, lua_State* L /*= 0*/)
{
	std::string fileExt = CParaFile::GetFileExtension(modulename);
	if (fileExt == "npl" || fileExt == "lua")
	{
		return modulename;
	}
	else
	{
		if (L == 0)
			L = m_pState;

		std::string filename = regex_replace(modulename, regex("\\."), "/");

		std::string parentModName;
		std::string subModPath;
		std::string testPath;
		smatch result;
		if (regex_search(filename, result, regex("^([^/]+)/?(.*)$")))
		{
			parentModName = result.str(1);
			subModPath = result.str(2);
			if (subModPath.empty())
				subModPath = parentModName;

			const char* pCurFilename = GetCurrentFileName(L);
			std::string currentFilename;
			if (pCurFilename)
				currentFilename = pCurFilename;
			std::string output;
			// Test1: if the current file is from npl_packages/[foldername], we will first try
			// npl_packages/[foldername]/npl_mod/[modulename]
			if (regex_search(currentFilename, result, regex("^(npl_packages/[\\w_]+/)")))
			{
				testPath = result.str(1) + "npl_mod/" + parentModName + "/" + subModPath;
				std::string out;
				output = testPath + ".lua";
				if (GetScriptDiskPath(output, out) != 0)
					return output;
				output = testPath + ".npl";
				if (GetScriptDiskPath(output, out) != 0)
					return output;
				// Test 1.1: if the current file is from npl_packages/[foldername], we will try again for 
				// npl_packages/[foldername]/npl_packages/[modulename]/npl_mod/[modulename]
				{
					testPath = result.str(1) + std::string("npl_packages/") + parentModName + "/npl_mod/" + parentModName + "/" + subModPath;
					std::string out;
					output = testPath + ".lua";
					if (GetScriptDiskPath(output, out) != 0)
						return output;
					output = testPath + ".npl";
					if (GetScriptDiskPath(output, out) != 0)
						return output;
				}
			}

			// Test2: we will try npl_mod/[modulename]
			{
				testPath = std::string("npl_mod/") + parentModName + "/" + subModPath;
				std::string out;
				output = testPath + ".lua";
				if (GetScriptDiskPath(output, out) != 0)
					return output;
				output = testPath + ".npl";
				if (GetScriptDiskPath(output, out) != 0)
					return output;
			}

			// finally, we will try npl_packages/[modulename]/npl_mod/[modulename]
			{
				testPath = std::string("npl_packages/") + parentModName + "/npl_mod/" + parentModName + "/" + subModPath;
				std::string out;
				output = testPath + ".lua";
				if (GetScriptDiskPath(output, out) != 0)
					return output;
				output = testPath + ".npl";
				if (GetScriptDiskPath(output, out) != 0)
					return output;
			}
		}
	}
	return modulename;
}


int ParaScripting::CNPLScriptingState::NPL_export(lua_State* L)
{
	if (L == 0)
		L = m_pState;
	std::string filename(GetCurrentFileName(L));
	// number of arguments
	int n = lua_gettop(L);
	if (n == 0)
	{
		int nResultNum = GetFileLoadStatus(filename);
		if (nResultNum == 0)
		{
			CacheFileModule(filename, -1, L);
		}

		// create or get the file module
		return PopFileModule(filename, L);
	}
	else if (n == 1)
	{
		CacheFileModule(filename, n, L);
		return n;
	}
	else if (n > 1)
	{
		// TODO: cache to an array table. 
		CacheFileModule(filename, n, L);
		return n;
	}
	return 0;
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

		if (nLength > 0)
		{
			nResult = luaL_loadbuffer(m_pState, sCall, nLength, sFileName);
			if (nResult == 0) {
				int top = lua_gettop(m_pState);
				nResult = Lua_ProtectedCall(m_pState, 0, LUA_MULTRET);
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

			ProcessResult(nResult, m_pState);
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
	lua_pushlstring(L, actTable, sizeof(actTable) - 1);
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (lua_istable(L, -1))
	{
		/** get activation function of the file name*/
		lua_pushlstring(L, filepath.c_str(), filepath.size());
		lua_gettable(L, -2);
		if (lua_isfunction(L, -1))
		{
			int top = lua_gettop(L);
			// call the function with 0 arguments and 0 result
			int nResult = Lua_ProtectedCall(L, 0, LUA_MULTRET);
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
			ProcessResult(nResult, L);

			if (nResult != 0) {
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
	if (luabind::type(funcActivate) == LUA_TFUNCTION)
	{
		object tabGlobal = luabind::globals(funcActivate.interpreter());
		object tabAct = tabGlobal["__act"];

		/// create the activate table, if it does not exist
		if (!(tabAct.is_valid()) || (luabind::type(tabAct) != LUA_TTABLE))
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
	lua_pushlstring(L, rts_name, sizeof(rts_name) - 1);
	lua_rawget(L, LUA_REGISTRYINDEX);
	NPL::CNPLRuntimeState* pState = (NPL::CNPLRuntimeState*)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return (pState) ? pState->shared_from_this() : NPL::NPLRuntimeState_ptr();
}

NPL::NPLRuntimeState_ptr ParaScripting::CNPLScriptingState::GetRuntimeStateFromLuaState(lua_State* L)
{
	const char rts_name[] = "__rts__";
	lua_pushlstring(L, rts_name, sizeof(rts_name) - 1);
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
