#pragma once

#include <string>

namespace ParaTerrain
{
	using namespace std;
	/** world name factory*/
	class CWorldNameFactory
	{
	public:
		CWorldNameFactory();
		/** give the world config file.The world name is derived using the following method.
		* the longest directory name in the given file or just the input. e.g.
		* "sample/worldconfig.txt", "sample/aaa/worldconfig.txt" will generate the world name "sample" and "sample/aaa"*/
		CWorldNameFactory(const string& sWorldConfigFile);
		~CWorldNameFactory();
	private:
		string m_strWorldPath;
		string m_strWorldName;
		//string m_strWorldConfig;
	public:
		/** give the world config file or the world name.The world name is derived using the following method.
		* the longset directory name in the given file or just the input. e.g.
		* "sample/worldconfig.txt", "sample", "sample/aaa/worldconfig.txt" will all generate the world name "sample", "sample", "sample/aaa" */
		void ResetWorldName(const string& sWorldConfigFile);

		/** set world name. if empty, world name will not be used in all sub folders. 
		* this is the default setting for all newly created world. 
		*/
		void SetWorldName(const string& sWorldName);

		/** suppose the given world name is "sample". The generated file name is 
		* "sample/sample.worldconfig.txt"*/
		string GetDefaultWorldConfigName();

		/** suppose the given world name is "sample". tile pos is (0,0) The generated file name is 
		* "sample/config/sample_0_0.config.txt"*/
		string GetTerrainConfigFile(int x, int y);

		/** suppose the given world name is "sample". tile pos is (0,0) The generated file name is 
		* "sample/elev/sample_0_0.raw"*/
		string GetTerrainElevationFile(int x, int y);

		/** suppose the given world name is "sample". tile pos is (0,0) The generated file name is 
		* "sample/script/sample_0_0.onload.lua"*/
		string GetTerrainOnloadFile(int x, int y);

		/** suppose the given world name is "sample". tile pos is (0,0) The generated file name is 
		* "sample/config/sample_0_0.mask"*/
		string GetTerrainMaskFile(int x, int y);

		/** get world name*/
		string GetWorldName();

		/** get world root path. suppose the given world name is "sample". The generated file name is "sample/" */
		string GetWorldDirectory();

		/** get world character path. suppose the given world name is "sample". The generated file name is "sample/character/" */
		string GetWorldCharacterDirectory();

		/** get default file config file path. suppose the given world name is "sample". The generated file name is "sample/flat.txt" */
		string GetDefaultTileConfigName();

		/** suppose the given world name is "sample". tile pos is (0,0) The generated file name is 
		* "sample/elev/sample_0_0_info.raw"*/
		string GetTerrainInfoDataFile(int x, int y);

		/**get block region data file name*/
		string GetBlockRegionFileName(int x,int y,bool usingTempFile);

		string GetBlockGameSaveDir(bool usingTempFile);

		/**get block template file name*/
		string GetBlockTemplateFileName(bool usingTempFile);
	};
}
