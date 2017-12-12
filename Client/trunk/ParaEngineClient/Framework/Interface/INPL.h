#pragma once
//-----------------------------------------------------------------------------
// Copyright (C) 2007 - 2010 ParaEngine Corporation, All Rights Reserved.
// Date:	2010.2
// Description:	API for NPL interface 
//-----------------------------------------------------------------------------
#include "baseinterface.h"

namespace NPL
{
	class INPLRuntime;
	/** NPL root interface
	* It can be used by plug-in dlls to access the NPL interface. 
	* Call GetNPLRuntime() to get the NPL runtime interface
	* One can simply start the NPL service in a different thread, or use the GetNPLRuntime() interface to frame move in the main thread. 
	*/
	class INPL : ParaEngine::BaseInterface
	{
	public:
		/** the core NPL runtime interface. change global settings of the NPL runtime as well as managing runtime state. */
		virtual INPLRuntime* GetNPLRuntime() = 0;

		/** if one wants to Run NPL runtime in the main thread. Call this function such as 30 times per second. 
		* @param fElapsedTime: time (seconds) elapsed since last call. 
		*/
		virtual void FrameMove(float fElapsedTime) = 0;

		/** reset the game loop script. the game loop script will be activated every 0.5 seconds 
		* see SetGameLoopInterval() to change the default interval
		* Please keep the game loop concise. The default game loop is ./script/gameinterface.lua
		*/
		virtual void SetGameLoop(const char* scriptName) = 0;
		
		/** set the game loop activation interval. The default value is 0.5 seconds. */
		virtual void SetGameLoopInterval(float fInterval) = 0;

		/** add an NPL command code to the (main state's) pending list to be processed in the next frame move cycle. 
		* [thread safe] This function is thread-safe by using a mutex internally.
		* @param sCommand: command to call in the next frame move. 
		* @param nLength: length in bytes. if 0, we will calculate from the sCommand. 
		*/
		virtual void AddNPLCommand(const char* sCommand, int nLength=0) = 0;

		/**
		* start the NPL service. This function does not return until finished. 
		* we may call it inside the main function of an exe or in a different thread. 
		* @param pCommandLine: e.g. "bootstrapper=\"config/bootstrapper_emptyshell.xml\""
		*/
		virtual int StartService(const char* pCommandLine) = 0;

		/**
		* stop the NPL service. Sending the stop signal. It does not return until service is stopped. 
		*/
		virtual void StopService() = 0;

		/** [thread safe]
		* activate the specified file. It can either be local or remote file. 
		* 
		* @param pState: the source runtime state that initiated this activation. If pState is NULL, the main runtime state is used.
		* @param sNPLFileName: 
		* a globally unique name of a NPL file name instance. 
		* The string format of an NPL file name is like below. 
		* [(sRuntimeStateName|gl)][sNID:]sRelativePath[@sDNSServerName]
		* 
		* the following is a list of all valid file name combinations: 
		*	"user001@paraengine.com:script/hello.lua"	-- a file of user001 in its default gaming thread
		*	"(world1)server001@paraengine.com:script/hello.lua"		-- a file of server001 in its thread world1
		*	"(worker1)script/hello.lua"			-- a local file in the thread worker1
		*	"(gl)script/hello.lua"			-- a glia (local) file in the current runtime state's thread
		*	"script/hello.lua"			-- a file in the current thread. For a single threaded application, this is usually enough.
		*	"(worker1)NPLRouter.dll"			-- activate a C++ or C# dll. Please note that, in windows, it looks for NPLRonter.dll; in linux, it looks for ./libNPLRouter.so 
		*	"plugin/libNPLRouter.dll"			-- almost same as above, it is recommented to remove the heading 'lib' when loading. In windows, it looks for plugin/NPLRonter.dll; in linux, it looks for ./plugin/libNPLRouter.so
		* 
		* @param sCode: it is a chunk of pure data table init code that would be transmitted to the destination file. 
		* @note: pure data table is defined as table consisting of only string, number and other table of the above type. 
		*   NPL.activate function also accepts ParaFileObject typed message data type. ParaFileObject will be converted to base64 string upon transmission. There are size limit though of 10MB.
		*   one can also programmatically check whether a script object is pure date by calling NPL.SerializeToSCode() function. Please note that data types that is not pure data in sCode will be ignored instead of reporting an error.
		* @return: NPLReturnCode. 0 means succeed. 
		*/
		virtual int activate(const char * sNPLFilename, const char* sCode, int nCodeLength = 0) = 0;
		
	};
}
