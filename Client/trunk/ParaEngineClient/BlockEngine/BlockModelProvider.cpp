//-----------------------------------------------------------------------------
// Class: Block Model data provider
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2013.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"

#include "BlockModelProvider.h"

using namespace ParaEngine;


BlockModel& ParaEngine::IBlockModelProvider::GetBlockModel( int nIndex /*= 0*/ )
{
	return m_pBlockTemplate->m_block_models[nIndex];
}

BlockModel& ParaEngine::IBlockModelProvider::GetBlockModelByData(uint32 nData /*= 0*/)
{
	return GetBlockModel(0);
}

BlockModel& IBlockModelProvider::GetBlockModel( CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks/*=NULL*/ )
{
	return GetBlockModel(GetModelIndex(pBlockManager, m_pBlockTemplate->GetID(), bx, by, bz, nBlockData, neighborBlocks));
}

int CGrassModelProvider::GetModelIndex( CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/ , Block** neighborBlocks/*=NULL*/ )
{
	// just make it random
	DWORD i = bx + by + bz + block_id;
	return (i + (i>>8) + (i>>16))& 0xf;
}

int CLinearModelProvider::GetModelIndex( CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks/*=NULL*/  )
{
	if(nBlockData<m_nModelCount)
		return nBlockData;
	else
		return 0;
}

BlockModel& ParaEngine::CLinearModelProvider::GetBlockModelByData(uint32 nBlockData)
{
	return m_pBlockTemplate->GetBlockModel(nBlockData);
}

int ParaEngine::CLinearModelProvider::GetModelCount() const
{
	return m_nModelCount;
}
