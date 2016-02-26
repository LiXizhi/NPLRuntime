#pragma once
#include <map>
#include <string>

namespace ParaEngine
{
	class CBlockWorld;
	
	/** factory class for multiple instances of CBlockWorld */
	class CBlockWorldManager
	{
	public:
		typedef std::map<std::string, CBlockWorld*>  BlockWorldMap_t;

		CBlockWorldManager();
		~CBlockWorldManager();
	public:
		static CBlockWorldManager* GetSingleton();

		/** get block world by name. */
		CBlockWorld* GetBlockWorld(const std::string& sInstanceName);

		void RemoveBlockWorld(const std::string& sInstanceName);

		/** create a new block world by name. */
		CBlockWorld* CreateBlockWorld(const std::string& sInstanceName);

		/** create get a block world */
		CBlockWorld* CreateGetBlockWorld(const std::string& sInstanceName);


		/** called once when application exits */
		void Cleanup();
	private:
		BlockWorldMap_t m_mapBlockWorlds;
	};
}