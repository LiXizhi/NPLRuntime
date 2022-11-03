//-----------------------------------------------------------------------------
// Class:	Block Material
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2022.11.3
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockMaterial.h"

using namespace ParaEngine;


CBlockMaterial::CBlockMaterial() 
{
}

CBlockMaterial::CBlockMaterial(const AssetKey& key) :AssetEntity(key) 
{
}

CBlockMaterial::~CBlockMaterial() {
}

CParameterBlock* CBlockMaterial::GetParamBlock(bool bCreateIfNotExist /*= false*/)
{
	return &m_SharedParamBlock;
}