#pragma once
#include <map>
#include "NPLCommon.h"

namespace NPL
{
	class CNPLStateMemAllocator;
}

namespace ParaScripting
{
	using namespace luabind;

	/**
	* a NPL scripting state (wrapper of lua State), for binding c++ classes to lua.
	* 
	* @note: not thread safe, but can use in multiple thread with a lock. 
	* In a multi-threaded environment, the number of states loaded are limited by system memory. Each state can cost over 1MB memory.
	* 
	* Then call Load*() functions to load ParaEngine classes and APIs to the
	* specified runtime, so that scripts in the runtime can use these classes and functions
	*/
	class CNPLScriptingState 
	{
	public:
		/** NPL runs in its own thread. and constantly reallocate 32, 64, 128, 256, 512 sized objects. */
		enum NPL_MemAllocatorType
		{
			// using the system's default malloc/realloc/free functions, please note that NPL makes heavy use of realloc
			MEM_ALLOC_TYPE_SYS_MALLOC,
			// using fixed sized pool. I thought this one should be fastest, but it turns out to the slowest. 
			// perhaps due to lua memory allocator heavily depends on realloc instead of malloc. 
			MEM_ALLOC_TYPE_POOL_MALLOC,
			// using the Doug Lea memory allocator. This is a fast thread local memory allocator. 
			MEM_ALLOC_TYPE_DL_MALLOC,
		};

		/**
		* @param bCreateState: if false, no interface state is created. 
		* this is usually the case when we wants to create an empty scripting state for DLL runtime state. 
		*/
		CNPLScriptingState(bool bCreateState=true);
		virtual ~CNPLScriptingState(void);

		/** get the last return value from lua_pcall. */
		int GetLastReturnValue() const;
		void SetLastReturnValue(int val);
	public:
		/// get the lua state. 
		lua_State* GetLuaState();

		/// return true if the runtime state is valid
		bool IsValid();

		/// load only NPL related functions.
		void LoadNPLLib();

		/// load only Para related functions.
		void LoadParaLib();

		//////////////////////////////////////////////////////////////////////////
		// following are individual group of HAPI
		//////////////////////////////////////////////////////////////////////////

		/// load functions for scene management
		void LoadHAPI_Globals();
		/// load functions for scene management
		void LoadHAPI_SceneManager();
		/// load functions for resource or assets management
		void LoadHAPI_ResourceManager();
		/// load functions for GUI controls and sounds
		void LoadHAPI_UI();
		/// this function is called by LoadHAPI_UI()
		void LoadHAPI_UI_Extension();
		/// load functions for Audio Engine
		void LoadHAPI_Audio();
		/// load functions for NPL Network Layer
		void LoadHAPI_Network();
		/// load functions for AI
		void LoadHAPI_AI();
		/// load functions for neural parallel language
		void LoadHAPI_NPL();
		/// load jabber related functions
		void LoadHAPI_Jabber();
	public:
		/** get the NPL code buffer and size according to a given file. Please note that BOM is read from the beginning of the file. 
		* if it is a utf8 encoded file with BOM byte mask, only the data section is returned 
		* @return: return the encoding if any. 0 is returned is no BOM encoding found at the beginning of the file. CP_UTF8 is returned if utf8 is found. 
		*/
		static int GetNPLCodeFromFile(ParaEngine::CParaFile* pFile, char** pBuffer, int* pBufferSize);

		/** static function:
		* @param sSearchPath: for example "src/" in android, etc.
		* @param bIsAdding: default to true.
		*/
		static void AddSearchPath(const char* sSearchPath, bool bIsAdding = true);

		/** we will first find if there is an up-to-date compiled version in the script/bin directory. if there is, 
		* we will load the compiled version, otherwise we will use the text version. 
		* @param filePath: the logic file path
		* @param sFileName: the output file path.
		* return uint32 of FileLocation enumeration. 0 if not found. 
		*/
		static uint32 GetScriptDiskPath(const string& filePath, string& sFileName);

		/**
		* whether a given script file is loaded. 
		* @param filePath: the local file path in the following format:
		*		[dir0]/[dir1]/[dir2]/[filename.lua]
		* @return: return true if file is already loaded in the current state.
		*/
		bool IsScriptFileLoaded(const string& filepath);

		/**
		* load a new NPL script file without running it. If the file is already loaded,
		* it will not be loaded again.
		* @param filePath: the local NPL script file path
		* @param bReload: if true, the file will be reloaded even if it is already loaded.
		*    otherwise, the file will only be loaded if it is not loaded yet. 
		* @param L: just in case the NPL.load is called from a different coroutine thread, which is different lua_state(stack) than the default one. 
		* @param bNoReturn: generate no return on lua_state's stack.
		* @return: return the GliaFile reference.
		*/
		bool LoadFile(const string& filePath, bool bReload, lua_State* L = 0, bool bNoReturn = false);

		/** do string in the current state. This function is usually called from the scripting interface.
		* @param sCode: the string to executed. 
		* @param nLength: length in bytes.if this is 0, length will be calculated, but must be smaller than a predefined safe length. If this is positive. any string length is allowed. 
		* @param sFileName: NULL or a file name string that is associated with the code chunk. This is mostly used for debugging. 
		* @param bPopReturnValue: default to true, whether to pop any return values from the sCode. default to true. 
		* @return 0 or the first int return value in the code. 
		*/
		int DoString(const char* sCode, int nLength = 0, const char* sFileName = NULL, bool bPopReturnValue = true);

		/**
		* Activate a local file. The file should be loaded already.
		* @param filepath: pointer to the file path.
		* @return: NPLReturnCode
		*/
		NPL::NPLReturnCode ActivateFile(const string& filepath, const char * code = NULL, int nLength=0);

		/**
		* bind the activation function. Usually, it is for the script function NPL.this(funcActivate).
		* @param nPreemptiveInstructionCount: optional parameter. if omitted, the activate function will
		* run non-preemptive (it is the programmer's job to let the function finish in short time).
		* If a number is specified here, the activate function will be preemptive like in Erlang.
		* When this number of instructions are executed, the activate function will be paused.
		*/
		bool BindFileActivateFunc(const object& funcActivate, const std::string& filename);

		/** get pointer to NPLRuntimeState from the lua state object. it just retrieves from a secret lua_status variable. */
		static NPL::NPLRuntimeState_ptr GetRuntimeStateFromLuaObject(const object& obj);
		static NPL::NPLRuntimeState_ptr GetRuntimeStateFromLuaState(lua_State* L);

		/** get current file name which is being processed now.*/
		const string& GetFileName();
		/** get current file that is being loaded or where the current code is defined. 
		* @param L: just in case the NPL.load is called from a different coroutine thread, which is different lua_state(stack) than the default one.
		*/
		const char* GetCurrentFileName(lua_State* L = 0);

		/**
		* load or restore the runtime state
		* no need to call LoadLuabind(), if the script runtime is created by this function
		@param pLuaState: default to NULL, if not, it will be an external luastate (that we do not own, calling SetOwnLuaState(false) internally)
		*/
		bool CreateSetState(lua_State* pLuaState = NULL);

		/** if true, we will delete the luastate when this class is destroyed. This function should rarely be called. 
		*/
		void SetOwnLuaState(bool bOwn);

		/** set/get exported file module. */
		int NPL_export(lua_State* L = 0);
	protected:

		/** destroy the runtime state
		* careful with this function, it will make the state invalid.
		*/
		void DestroyState();

		/// the function must be called for the lua state before binding anything to it
		void LoadLuabind();

		/** set the current runtime state. this function should be called from the parent class immediately after LoadNPLLib(). */
		void SetRuntimeState(NPL::NPLRuntimeState_ptr runtime_state);

		/**
		* Process return result after calling a function or loading a file in Lua.
		* @param nResult: return result returned by luaL_loadbuffer() or lua_pcall().
		* @param L: just in case the NPL.load is called from a different coroutine thread, which is different lua_state(stack) than the default one.
		*/
		void ProcessResult(int nResult, lua_State* L = 0);

		/** save nResult objects on stack to file modules 
		* @return the number of new result pushed on stack. usually 1 or 0
		*/
		int CacheFileModule(const std::string& filename, int nResult, lua_State* L = 0);

		/** pop file module to stack for a given file. Return true, if file is loaded before or false if not. 
		* @return the number of result pushed on stack. usually 1 or 0
		*/
		int PopFileModule(const std::string& filename, lua_State* L = 0);

		/** get module file path by module name */
		std::string GetModuleFilePath(const std::string& modulename, lua_State* L = 0);

		/** wrapping the m_loaded_files
		*/
		int GetFileLoadStatus(const string& filepath);
		void SetFileLoadStatus(const string& filepath, int nStatus);

	private:
		/** construct this to ensure matching calls to push and pop file name. */
		class CFileNameStack
		{
		public:
			CFileNameStack(CNPLScriptingState* pState, const string& filename):m_pState(pState){
				if(m_pState)
					m_pState->PushFileName(filename);
			}
			~CFileNameStack(){
				if(m_pState)
					m_pState->PopFileName();
			}
			CNPLScriptingState* m_pState;
		};

		/** push the file name of the file which is being activated. 
		* @note: use CFileNameStack instead of calling this directly */
		void PushFileName(const string& filename);

		/** push the file name of the file which is being activated. 
		* @note: use CFileNameStack instead of calling this directly */
		void PopFileName();

	
		void LoadParaScene();
		void LoadParaWorld();

		lua_State* m_pState;
		/* whether we own the luastate. true by default. false if luastate is set externally.*/
		bool m_bOwnLuaState;
		
		/** the default stack size in KB. default to -1, which uses the NPL runtime default, 1024KB. */
		int m_nStackSize;

		/** get the last return value from lua_pcall. */
		int m_nLastReturnValue;
		
		/** whether we will use per thread (state) memory allocator. In network applications that constantly fire NPL file activations, 
		we need to enable this to avoid memory fragmentation and improve execution speed. 
		Because lua is single threaded, memory allocation in lua state has zero overhead (no lock) when using a custom memory allocator. 
		However, it will consume twice as much memory in most cases; and memory locality may not be as good as the system's default memory allocator. 
		@note: default to true
		*/
		NPL_MemAllocatorType m_nMemAllocatorType;

		/** the memory allocator if m_bUseMemAllocator is true. */ 
		union
		{
			NPL::CNPLStateMemAllocator* m_pMemAlloc;
			void* m_mspace;
		};

		/** all loaded files mapping from filename to number of cached objects. 
		* If -1, it means that file is being loaded or something went wrong. 0 means no cached object.  
		*/
		std::map <std::string, int32> m_loaded_files;

		/** a stack of files being loaded. */
		std::stack <std::string> m_stack_current_file;

		/* currently only a single search path is supported. */
		static std::string m_searchpath;
	};
}
