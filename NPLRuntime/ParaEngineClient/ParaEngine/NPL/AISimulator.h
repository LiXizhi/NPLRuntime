#pragma once
#include "util/mutex.h"
#include <list>
using namespace std;

// declaration
namespace NPL
{
	class CNPLRuntime;
}

namespace ParaEngine
{
	/**
	* AI and AI scripts handlers.
	* It hold the only instance of the CPOLRuntimeEnv in the application
	* game interface should query this it from CAISimulator. It serves as
	* a bridge between the game world and the POL language. It contains a 
	* group of Host API(or HAPI) for POL to load. 
	* 
	* AI Simulator controls all the game and player logic of the game world:
	* It includes the following aspect.
	* (1) NPC and player logic. 
	* (2) sequential cut scenes.
	* (3) saving and restoring all game states. 
	* (4) Network logic is handled transparently within the POL.
	*/
	class CAISimulator
	{
	private:
		/** the default game loop script. */
		string m_sGameloop;
		float m_fGameloopInterval;
		/** the mutex used for AddNPLCommand to make that function thread-safe*/
		mutex m_mutex;
		/// POL runtime environment
		NPL::CNPLRuntime*		m_pRuntimeEnv;
		bool m_bIsCleanedup;
	public:
		/** get a global instance of this class */
		static CAISimulator * GetSingleton();

		/** add an NPL command code to the pending list to be processed in the next simulation cycle. 
		* This function is thread-safe by using a mutex internally.
		*/
		void AddNPLCommand(const string& sCommand);

		/** get the current NPL runtime environment. */
		inline NPL::CNPLRuntime* GetNPLRuntime(){return m_pRuntimeEnv;};

		/** perform final clean up, such as shutting down the network layer. */
		void CleanUp();
		bool IsCleanedUp();

		/// Move all object's mental states to next time 
		void FrameMove(float fElapsedTime = 0);

		/** activate a specified file 
		* @param code: 
		* @param nLength: the code length. if this is 0, length is determined from code, however, it must not exceed 4096 bytes. if it is specified. it can be any code length 
		* @return: same as NPL.activate return value. 
		*/
		int NPLActivate(const char* filepath, const char* code = NULL, int nLength=0);
		/**
		* run a script in the "share.lua" neuron file. 
		* It is equivalent to calling dofile("filename") in the global space of "share.lua".
		* Each time, the file will be parsed and executed. The file is better containing no function definition or 
		* global variables. In other words, the file is better containing only ParaEngine UI commands.
		* @param bReload:  whether reload the file if it already exists.
		*/
		void NPLLoadFile(const char* filename, bool bReload=false);

		/**
		* run a chunk of string in the "share.lua" neuron file. 
		* It is equivalent to calling "strCode" in the global space of "share.lua".
		* @param nLength: default to 0. if it is 0, the strCode must be exceed to the code length limit. 
		*/
		void NPLDoString(const char* strCode, int nLength = 0);

		/** reset the game loop script. the game loop script will be activated every 0.5 seconds 
		* see SetGameLoopInterval() to change the default interval
		* Please keep the game loop concise. The default game loop is ./script/gameinterface.lua
		*/
		void SetGameLoop(const string& scriptName);
		/** set the game loop activation interval. The default value is 0.5 seconds. */
		void SetGameLoopInterval(float fInterval);
	public:

		CAISimulator(void);
		~CAISimulator(void);
	};
}