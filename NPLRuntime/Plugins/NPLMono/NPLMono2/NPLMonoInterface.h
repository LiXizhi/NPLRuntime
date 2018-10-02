#pragma once

#include <mono/jit/jit.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>
#include <INPLScriptingState.h>

/**
* exposing ParaEngine and NPL API to the mono scripting interface as internal mono calls. 
*/
namespace NPLMonoInterface
{
	/** A wrapper class of NPL related functions. */
	class NPL_wrapper
	{
	public:
		/** This function is for testing NPLMonoInterface.cs
		this is an example of exposing C++ API to the mono runtime. */
		static MonoString* HelloWorld();

		/** 
		Get the current incoming message of a npl_runtime_state. 
		@remark: This function should only be called from activate() function, since activate() function runs in the same thread of its parent npl_runtime_state
		*/
		static MonoString* GetCurrentMsg(void* npl_runtime_state);

		/** activate a local or remote file */
		static void activate(void* npl_runtime_state, MonoString* file_name, MonoString* msg);

		/** same as activate2. except that no npl_runtime_state is specified, the main runtime state is assumed.  */
		static void activate2(MonoString* file_name, MonoString* msg);

		/** accept a given connection. The connection will be regarded as authenticated once accepted. */
		static void accept(MonoString* tid, MonoString* nid);

		/** reject and close a given connection. The connection will be closed once rejected. */
		static void reject(MonoString* tid);
	};


	/** A wrapper class of NPL related functions. */
	class ParaGlobal_wrapper
	{
	public:
		/** this is an example of exposing C++ API to the mono runtime. */
		static void log(MonoString*);
		/** output in application log format with date time */
		static void applog(MonoString*);
	};
}



