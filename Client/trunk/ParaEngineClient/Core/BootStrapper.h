#pragma once
#include <string>

namespace ParaEngine
{
	using namespace std;
	/**
	* Bootstrapper file is a xml file to be executed at the very beginning of the game engine. 
	* Its main function is to specify the main game loop file to be activated, and optionally the config file to load. 
	Example: 
	<verbatim>
		<?xml version="1.0" ?>
		<MainGameLoop>(gl)script/apps/Aries/main_loop.lua</MainGameLoop>
		<ConfigFile>config/config.safemode.txt</ConfigFile>
	</verbatim>
	Please note: ConfigFile is optional. We can specify a different config file other than config/config.txt to load at startup, such as safe mode. 
	*/
	class CBootStrapper
	{
	protected:
		CBootStrapper(void);
	public:
		~CBootStrapper(void);

		/**
		* since there is only one bootstrapper, it is always accessed through the singleton class.
		* @return 
		*/
		static CBootStrapper* GetSingleton();

		/**
		* load from a given XML file. 
		* @param sXMLfile the path of the file, if this is "", the config/bootstrapper.xml will be used.
		* @return true if success
		*/
		bool LoadFromFile(const string& sXMLfile);
		/**
		* save to a given XML file. 
		* @param sXMLfile the path of the file, if this is "", the config/bootstrapper.xml will be used.
		* @return true if success
		*/
		bool SaveToFile(const string& sXMLfile);

		/** load the default setting. this function is called at the constructor.*/
		void LoadDefault();

	public:
		/** get the game loop file. the game loop file provides the heart beat of the application. 
		* It is also the very first(entry) script to be activated when the application starts up.
		* The default game loop is ./script/gameinterface.lua
		*/
		const string& GetMainLoopFile();
		/** Set the game loop file. the game loop file provides the heart beat of the application. 
		* It is also the very first(entry) script to be activated when the application starts up.
		* The default game loop is ./script/gameinterface.lua
		*/
		void SetMainLoopFile(const string& sMainFile);

		/** get the ParaEngine config file. 
		* The default is "", where the caller can default to config/config.txt
		*/
		const string& GetConfigFile();

		/** set the ParaEngine config file. 
		* The default is "", where the caller can default to config/config.txt
		*/
		void SetConfigFile(const string& sConfigFile);
	
	private:
		/** main game loop file, default to script/gameinterface.lua */
		string m_sMainLoopFile;

		/** ParaEngine config file, default to "" */
		string m_sConfigFile;
	};
}
