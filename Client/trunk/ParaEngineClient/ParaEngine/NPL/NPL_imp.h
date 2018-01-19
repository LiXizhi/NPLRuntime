#pragma once
#include "INPL.h"

namespace ParaEngine
{
	class ClassDescriptor;
}

namespace NPL
{
	ParaEngine::ClassDescriptor* NPL_GetClassDesc();

	/**
	* implementation of INPL
	*/
	class CNPL_imp : public INPL
	{
	public:
		CNPL_imp();

		virtual INPLRuntime* GetNPLRuntime();

		virtual int activate(const char * sNPLFilename, const char* sCode, int nCodeLength);
		
		virtual int StartService(const char* pCommandLine);

		virtual void StopService();

		virtual void FrameMove(float fElapsedTime);

		/** reset the game loop script. the game loop script will be activated every 0.5 seconds 
		* see SetGameLoopInterval() to change the default interval
		* Please keep the game loop concise. The default game loop is ./script/gameinterface.lua
		*/
		virtual void SetGameLoop(const char* scriptName);

		/** set the game loop activation interval. The default value is 0.5 seconds. */
		virtual void SetGameLoopInterval(float fInterval);

		/** add an NPL command code to the (main state's) pending list to be processed in the next frame move cycle. 
		* [thread safe] This function is thread-safe by using a mutex internally.
		* @param sCommand: command to call in the next frame move. 
		* @param nLength: length in bytes. if 0, we will calculate from the sCommand. 
		*/
		virtual void AddNPLCommand(const char* sCommand, int nLength=0);
	private:
		/** the default game loop script. */
		std::string m_sGameloop;
		
		/** game loop interval */
		float m_fGameloopInterval;
	};
}