//-----------------------------------------------------------------------------
// Class:	config
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.2.7
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockMaterialManager.h"

using namespace ParaEngine;

CBlockMaterialManager::CBlockMaterialManager()
{
	m_nextMaterialId = 0;
}

CBlockMaterialManager::~CBlockMaterialManager()
{
}

CBlockMaterial* CBlockMaterialManager::CreateBlockMaterial()
{
	return CreateEntity("", std::to_string(GetNextMaterialID())).first;
}

CBlockMaterial* CBlockMaterialManager::GetBlockMaterialByID(int32_t id)
{
	return GetEntity(std::to_string(id));
}

CBlockMaterial* CBlockMaterialManager::CreateGetBlockMaterialByID(int32_t id)
{
	if (id <= 0) 
	{
		id = GetNextMaterialID();
	}
	else
	{
		// 更新nextMaterialId值
		m_nextMaterialId = m_nextMaterialId < id ? id : m_nextMaterialId;
	}
	return CreateEntity("", std::to_string(id)).first;
}
