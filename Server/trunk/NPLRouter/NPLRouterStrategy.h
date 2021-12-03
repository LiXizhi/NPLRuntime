#pragma once
#include <string>
#include <vector>
#include <map>
#include <util/Mutex.hpp>

//user table count 72X12
#define USER_TABLE_COUNT	864

namespace NPL
{
	typedef struct { int iTableIndex; int iNid; } DBRouterTable;
    class NPLRouterStrategy
	{
	private:
		bool bIsLoadConfig;
		std::vector<DBRouterTable> m_MyDBRouterTables;
		int m_nUserTableCount;
		double dlResponseNid;
		string strGameServerScript;
		string strDBServerScript;
#ifdef WIN32
		void* logger;
#else
		ParaEngine::CServiceLogger_ptr logger;
#endif
		/** protecting config file. */
		ParaEngine::Mutex m_mutex;
	public:
		NPLRouterStrategy()
		{
			bIsLoadConfig=false;
			dlResponseNid = 0;
			m_nUserTableCount = 0;
		};
		~NPLRouterStrategy(){};

		/** the singleton instance */
		static NPLRouterStrategy* GetInstance();

		void LoadConfigInfo(const string& sMsg);
		int GetDBServerNID(double dlUserNID);
		void DoRouterStrategy(int nType, void* pVoid);
		//bool ActivateFileWithTimeout(int iTimeout);
	};

}