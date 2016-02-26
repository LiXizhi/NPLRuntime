#pragma once
#include "NPLTypes.h"
#include <string>

namespace NPL
{
	class INPLRuntimeState;

	/**
	* NPLScriptingState interface. Implement this to support additional languages inside NPLRuntimeState. 
	* Please note that we use const std::string& for filename, which means that implementation of this interface 
	* should most probably use std::map<std::string filename, file_instance> for keeping loaded files as well. 
	* It can save us one std::string copy instruction when these API are called. 
	*/
	class INPLScriptingState
	{
	public:
		/** this is something like delete this*/
		virtual void Release() = 0;

		/**get the Mono Domain state. 
		@return MonoDomain *
		*/
		virtual void* GetState() = 0;

		/// return true if the runtime state is valid
		virtual bool IsValid() = 0;

		/**
		* whether a given script file is loaded. 
		* @param filePath: the local file path in the following format:
		*		[dir0]/[dir1]/[dir2]/[filename.lua]
		* @return: return true if file is already loaded in the current state.
		*/
		virtual bool IsScriptFileLoaded(const std::string& filepath) = 0;

		/**
		* load a new CS script file without running it. If the file is already loaded,
		* it will not be loaded again.
		* @param filePath: the local CS script file path, such as "test.dll/test.cs", "test.dll/ParaMono.test.cs", where ParaMono can be a namespace. 
		* @param bReload: if true, the file will be reloaded even if it is already loaded.
		*    otherwise, the file will only be loaded if it is not loaded yet. 
		* @return: return true, if file is loaded. 
		*/
		virtual bool LoadFile(const std::string& filePath, bool bReload) = 0;

		/** do string in the current state. This function is usually called from the scripting interface.
		* If one do not want to implement DoString for security reasons. Just put do nothing and return NPL_OK. 
		* @param sCall: the string to executed. 
		* @param nLength: length in bytes.if this is 0, length will be calculated, but must be smaller than a predefined safe length. If this is positive. any string length is allowed. 
		*/
		virtual int DoString(const char* sCall, int nLength = 0) = 0;

		/**
		* Activate a local file. The file should be loaded already.
		* @param filepath: pointer to the file path.
		* @return: NPLReturnCode
		*/
		virtual NPL::NPLReturnCode ActivateFile(const std::string& filepath, const char * code = NULL, int nLength=0) = 0;

		/**
		* load or restore the runtime state 
		@param name: any runtime state name. 
		@param pState: the NPL runtime state that this mono scripting state belongs to.
		*/
		virtual bool CreateState(const char* name, INPLRuntimeState* pState=NULL) = 0;

		/** destroy the runtime state
		* careful with this function, it will make the state invalid.
		*/
		virtual void DestroyState() = 0;


		/** set the NPL runtime state that this mono scripting state belongs to. 
		* This function is used internally by NPLRuntimeState. */
		virtual void SetNPLRuntimeState(INPLRuntimeState* pState) = 0;

		/** Get the NPL runtime state that this mono scripting state belongs to */
		virtual INPLRuntimeState* GetNPLRuntimeState() = 0;
	};

	/**
	* This is the NPLMono DLL plugin interface.
	* In ParaEngine server side plugin project NPLMono, the class CMonoScriptingState implement this interface. 
	* NPLRuntimeState will use this interface to load and activate files in the mono runtime environment. 
	*
	* The reason that I make mono runtime a plugin, instead of build-in class, is that it is usually only used on the server side
	* and Mono runtime is pretty big both at compile time and run time. 
	*/
	class IMonoScriptingState : public INPLScriptingState
	{
	public:

	};
}
