#pragma once

#include "./terrain/WorldNameFactory.h"
#include <string>
namespace ParaEngine
{
	using namespace ParaTerrain;
	/**
	* @ingroup ParaEngine
	* Providing information about the current game world
	*/
	class CWorldInfo : public CWorldNameFactory
	{
	public:
		CWorldInfo(void);
		~CWorldInfo(void);
		/** give the world config file.The world name is derived using the following method.
		* the first directory name in the given file or just the input. e.g.
		* "sample/worldconfig.txt", "sample/aaa/worldconfig.txt" will all generate the world name "sample" */
		CWorldInfo(const string& sWorldConfigFile);
		/** a singleton class */
		static CWorldInfo& GetSingleton();
	public:
		/**
		* Paraworld is told to execute in the given sandbox. 
		(1)	ParaWorld namespace supports a sandbox mode, which can be turned on and off on demand. Once turned on, all scripts from the current game world will be executed in a separate and newly created script runtime environment. 
		(2)	Sandbox mode is an isolated mode that does not have any link with the glia file environment.
		(3)	The world scripts protected by the sandbox runtime environment includes: terrain tile onload script, biped event handler scripts, such as character onload, onclick events. 
		(4)	The sandbox environment includes the following functions that could be used: ParaScene, ParaUI namespace functions. It also explicitly disabled the following functions:
		a)	Dofile()
		b)	Io, ParaIO, Exec
		c)	Require(),NPL.load, NPL.activate, NPL.download: cut off any way to manually load a file. It adds
		d)	Log
		e)	Download some file to replace local file.
		f)	Changing the Enter sand box function or almost any function to some fake function.
		(5)	The following attack methods should be prevented by the sandbox environment
		a)	Execute or load any external application
		b)	Write to any file, including log file
		c)	Compromise data or functions in the glia file environment. Such as, changing and hooking the string method
		d)	Compromise the sandbox itself and then affect in the next sandbox entity.
		(6)	glia file environment does not have a sandbox mode. Because I found that any global sandbox mode implementation has a way to hack in, and I give up any measure of protecting the glia file environment. Sandbox protection for the world file is enough because that is the only source file that may not be provided by ParaEngine. In order to run any other code not provided by the ParaEngine, the user should have been well informed of the danger. But so far, there is no need to have a world that should inform the user. Because most world functions are supported in the world sandbox.

		* @param sNeuronFileName: script file name. Use NPL.CreateNeuronFile() to create a sandbox neuron file
		* if you do not want to use a sandbox, please specify NULL or "". 
		*/
		void SetScriptSandBox(const char* sSandboxNeuronFile);

		/** please see SetScriptSandBox() 
		* @return: it may return NULL if the sandbox does not exist. 
		*/
		const char* GetScriptSandBox();

	private:
		std::string m_sScriptSandbox;
	};

}
