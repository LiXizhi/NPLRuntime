//-----------------------------------------------------------------------------
// Class: Carpet Model data provider
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2015.10
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockConfig.h"
#include "BlockCommon.h"
#include "BlockChunk.h"
#include "BlockTemplate.h"
#include "ShapeAABB.h"
#include "CarpetModelProvider.h"

using namespace ParaEngine;

ParaEngine::CCarpetModelProvider::CCarpetModelProvider(BlockTemplate* pBlockTemplate)
	: CLinearModelProvider(pBlockTemplate, 22)
{
	const BlockModel& basemodel = pBlockTemplate->GetBlockModel();
	for (int i = 0; i < m_nModelCount; i++)
	{
		m_block_models[i] = basemodel;
	}

	char sName[] = "plate0";
	for (int i = 0; i < 6; i++)
	{
		sName[5] = '0' + i;
		m_block_models[i].LoadModel(sName);
	}

	// local id: 1: posX, 0: negX, 3:posZ, 4:negZ
	const int id[] = { 3, 4, 1, 5 };
	const int nextid[] = { 2, 3, 1, 0};
	const int id_to_face[] = { 3, 4, 1, 5 };

	// one plate on the ground, the other on one of the four side
	for (int i = 6; i < 10; i++)
	{
		int nLocalId = (i - 6);
		// AABB uses the plate on the ground 
		m_block_models[i] = m_block_models[0];
		m_block_models[i].RemoveFace(id_to_face[nLocalId] * 4);

		// add the 5 faces from the side plate model
		const BlockModel& sidemodel = m_block_models[id[nLocalId]];
		m_block_models[i].IncrementFaceCount(5);
		
		CShapeAABB aabb = sidemodel.GetAABB();
		aabb.Extend(Vector3(BlockConfig::g_half_blockSize, BlockConfig::g_half_blockSize, BlockConfig::g_half_blockSize));
		m_block_models[i].SetAABB(aabb.GetMin(), aabb.GetMax());

		for (int j = 0; j < 24; j++)
		{
			// excluding the bottom face
			if (!(j >= BlockModel::g_btmLB && j <= BlockModel::g_btmRB))
				m_block_models[i].AddVertex(sidemodel, j);
		}
	}
	// one plate on the ceiling, the other on one of the four side
	for (int i = 10; i < 14; i++)
	{
		int nLocalId = (i - 10);
		// AABB uses the plate on the ceiling
		m_block_models[i] = m_block_models[2];
		m_block_models[i].RemoveFace(id_to_face[nLocalId] * 4);

		// add the 5 faces from the side plate model
		const BlockModel& sidemodel = m_block_models[id[nLocalId]];
		m_block_models[i].IncrementFaceCount(5);
		
		CShapeAABB aabb = sidemodel.GetAABB();
		aabb.Extend(Vector3(BlockConfig::g_half_blockSize, BlockConfig::g_half_blockSize, BlockConfig::g_half_blockSize));
		m_block_models[i].SetAABB(aabb.GetMin(), aabb.GetMax());

		for (int j = 0; j < 24; j++)
		{
			// excluding the bottom face
			if (!(j >= BlockModel::g_topLB && j <= BlockModel::g_topRB))
				m_block_models[i].AddVertex(sidemodel, j);
		}
	}
	// one plate on the ground, the other adjacent two on the four sides
	for (int i = 14; i < 18; i++)
	{
		int nLocalId1 = (i - 14);
		int nLocalId2 = nextid[nLocalId1];
		// AABB uses the plate on the ground 
		m_block_models[i] = m_block_models[0];
		m_block_models[i].SetIsCubeAABB(true);
		// add the 5 faces from the two side plate models
		for (int side = 0; side < 2; side++)
		{
			int nLocalId = (side == 0) ? nLocalId1 : nLocalId2;
			int nLocalIdOther = (side == 0) ? nLocalId2 : nLocalId1;
			m_block_models[i].RemoveFace(id_to_face[nLocalId] * 4);

			const BlockModel& sidemodel = m_block_models[id[nLocalId]];
			m_block_models[i].IncrementFaceCount(4);
			for (int j = 0; j < 24; j++)
			{
				// excluding the bottom face
				if (j >= BlockModel::g_btmLB && j <= BlockModel::g_btmRB)
					continue;
				else if ((j >= id_to_face[nLocalIdOther] * 4 && j < (id_to_face[nLocalIdOther] + 1) * 4))
					continue;
				else
					m_block_models[i].AddVertex(sidemodel, j);
			}
		}
	}
	// one plate on the ceiling, the other adjacent two on the four sides
	for (int i = 18; i < 22; i++)
	{
		int nLocalId1 = (i - 18);
		int nLocalId2 = nextid[nLocalId1];
		// AABB uses the plate on the ceiling 
		m_block_models[i] = m_block_models[2];
		m_block_models[i].SetIsCubeAABB(true);
		// add the 5 faces from the two side plate models
		for (int side = 0; side < 2; side++)
		{
			int nLocalId = (side == 0) ? nLocalId1 : nLocalId2;
			int nLocalIdOther = (side == 0) ? nLocalId2 : nLocalId1;
			m_block_models[i].RemoveFace(id_to_face[nLocalId] * 4);
			const BlockModel& sidemodel = m_block_models[id[nLocalId]];

			m_block_models[i].IncrementFaceCount(4);
			for (int j = 0; j < 24; j++)
			{
				// excluding the bottom face
				if (j >= BlockModel::g_btmLB && j <= BlockModel::g_btmRB)
					continue;
				else if ((j >= id_to_face[nLocalIdOther] * 4 && j < (id_to_face[nLocalIdOther] + 1) * 4))
					continue;
				else
					m_block_models[i].AddVertex(sidemodel, j);
			}
		}
	}
}

ParaEngine::CCarpetModelProvider::~CCarpetModelProvider()
{

}

BlockModel& ParaEngine::CCarpetModelProvider::GetBlockModel(int nIndex /*= 0*/)
{
	return (nIndex<m_nModelCount) ? m_block_models[nIndex] : m_block_models[0];
}

BlockModel& ParaEngine::CCarpetModelProvider::GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData /*= 0*/, Block** neighborBlocks /*= NULL*/)
{
	return (nBlockData<m_nModelCount) ? m_block_models[nBlockData] : m_block_models[0];
}
