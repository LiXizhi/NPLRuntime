//-----------------------------------------------------------------------------
// Class:	Block World Manager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.2.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockWorld.h"
#include "BlockWorldManager.h"

using namespace ParaEngine;

CBlockWorldManager::CBlockWorldManager()
{

}

ParaEngine::CBlockWorldManager::~CBlockWorldManager()
{

}

CBlockWorldManager* ParaEngine::CBlockWorldManager::GetSingleton()
{
	static CBlockWorldManager g_singleton;
	return &g_singleton;
}

CBlockWorld* ParaEngine::CBlockWorldManager::GetBlockWorld(const std::string& sInstanceName)
{
	auto iter = m_mapBlockWorlds.find(sInstanceName);
	if (iter != m_mapBlockWorlds.end())
	{
		return iter->second;
	}
	return NULL;
}

void ParaEngine::CBlockWorldManager::RemoveBlockWorld(const std::string& sInstanceName)
{
	auto iter = m_mapBlockWorlds.find(sInstanceName);
	if (iter != m_mapBlockWorlds.end())
	{
		SAFE_DELETE(iter->second);
		m_mapBlockWorlds.erase(iter);
	}
}

CBlockWorld* ParaEngine::CBlockWorldManager::CreateBlockWorld(const std::string& sInstanceName)
{
	CBlockWorld* pBlockWorld = new CBlockWorld();
	m_mapBlockWorlds[sInstanceName] = pBlockWorld;
	return pBlockWorld;
}

CBlockWorld* ParaEngine::CBlockWorldManager::CreateGetBlockWorld(const std::string& sInstanceName)
{
	CBlockWorld* pBlockWorld = GetBlockWorld(sInstanceName);
	if (pBlockWorld)
	{
		return pBlockWorld;
	}
	else
	{
		return CreateBlockWorld(sInstanceName);
	}
}

void ParaEngine::CBlockWorldManager::Cleanup()
{
	for (BlockWorldMap_t::iterator itCur = m_mapBlockWorlds.begin(); itCur != m_mapBlockWorlds.end();)
	{
		SAFE_DELETE(itCur->second);
		itCur = m_mapBlockWorlds.erase(itCur);
	}
}

