//-----------------------------------------------------------------------------
// Class:	All kinds of block tessellation 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.12.22
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockModel.h"
#include "BlockCommon.h"
#include "VertexFVF.h"
#include "BlockChunk.h"
#include "BlockRegion.h"
#include "BlockWorld.h"
#include "VertexFVF.h"
#include "BlockTessellators.h"

using namespace ParaEngine;

ParaEngine::BlockTessellatorBase::BlockTessellatorBase(CBlockWorld* pWorld) 
	: m_pWorld(pWorld), m_pCurBlockTemplate(0), m_pCurBlockModel(0), m_blockId_ws(0, 0, 0), m_nBlockData(0), m_pChunk(0), m_blockId_cs(0,0,0)
{
	memset(neighborBlocks, 0, sizeof(neighborBlocks));
}

void ParaEngine::BlockTessellatorBase::SetWorld(CBlockWorld* pWorld)
{
	if (m_pWorld != pWorld)
	{
		m_pWorld = pWorld;
	}
}

int32 ParaEngine::BlockTessellatorBase::TessellateBlock(BlockChunk* pChunk, uint16 packedBlockId, BlockRenderMethod dwShaderID, BlockVertexCompressed** pOutputData)
{
	return 0;
}

int32_t ParaEngine::BlockTessellatorBase::GetMaxVertexLight(int32_t v1, int32_t v2, int32_t v3, int32_t v4)
{
	int32_t max1 = Math::Max(v1, v2);
	int32_t max2 = Math::Max(v3, v4);
	return Math::Max(max1, max2);
}

uint8 ParaEngine::BlockTessellatorBase::GetMeshBrightness(BlockTemplate * pBlockTemplate, uint8* blockBrightness)
{
	uint8 centerLightness = blockBrightness[rbp_center];
	if (centerLightness > 0 && pBlockTemplate->GetLightOpacity() > 1)
		return Math::Min(centerLightness + pBlockTemplate->GetLightOpacity(), 15);
	else
		return Math::Max(Math::Max(Math::Max(Math::Max(Math::Max(Math::Max(centerLightness, blockBrightness[rbp_nX]), blockBrightness[rbp_pX]), blockBrightness[rbp_nZ]), blockBrightness[rbp_pZ]), blockBrightness[rbp_pY]), blockBrightness[rbp_nY]);
}

int32_t ParaEngine::BlockTessellatorBase::GetAvgVertexLight(int32_t v1, int32_t v2, int32_t v3, int32_t v4)
{
	if ((v2 > 0 || v3 > 0))
	{
		int32_t max1 = Math::Max(v1, v2);
		int32_t max2 = Math::Max(v3, v4);
		return Math::Max(max1, max2);
	}
	else
	{
		return v1;
	}
}


bool ParaEngine::BlockTessellatorBase::UpdateCurrentBlock(BlockChunk* pChunk, uint16 packedBlockId)
{
	Block* pCurBlock = pChunk->GetBlock(packedBlockId);
	if (pCurBlock)
	{
		m_pCurBlockTemplate = pCurBlock->GetTemplate();
		if (m_pCurBlockTemplate)
		{
			m_pChunk = pChunk;
			UnpackBlockIndex(packedBlockId, m_blockId_cs.x, m_blockId_cs.y, m_blockId_cs.z);
			m_blockId_ws.x = pChunk->m_minBlockId_ws.x + m_blockId_cs.x;
			m_blockId_ws.y = pChunk->m_minBlockId_ws.y + m_blockId_cs.y;
			m_blockId_ws.z = pChunk->m_minBlockId_ws.z + m_blockId_cs.z;

			neighborBlocks[rbp_center] = pCurBlock;
			m_nBlockData = pCurBlock->GetUserData();
			m_pCurBlockModel = &(m_pCurBlockTemplate->GetBlockModel(m_pWorld, m_blockId_ws.x, m_blockId_ws.y, m_blockId_ws.z, (uint16)m_nBlockData, neighborBlocks));
			tessellatedModel.ClearVertices();
			return true;
		}
	}
	return false;
}

void ParaEngine::BlockTessellatorBase::FetchNearbyBlockInfo(BlockChunk* pChunk, const Uint16x3& blockId_cs, int nNearbyBlockCount, int nNearbyLightCount)
{
	//neighbor block info: excluding the first (center) block, since it has already been fetched. 
	if (nNearbyBlockCount > 1)
	{
		memset(neighborBlocks + 1, 0, sizeof(Block*) * (nNearbyBlockCount - 1));
		pChunk->QueryNeighborBlockData(blockId_cs, neighborBlocks+1, 1, nNearbyBlockCount - 1);
	}
	//neighbor light info
	if (!m_pCurBlockModel->IsUsingSelfLighting())
	{
		nNearbyLightCount = nNearbyLightCount < 0 ? nNearbyBlockCount : nNearbyLightCount;
		memset(blockBrightness, 0, sizeof(uint8_t) * nNearbyLightCount * 3);
		m_pWorld->GetBlockBrightness(m_blockId_ws, blockBrightness, nNearbyLightCount, 3);
	}
}

uint32_t ParaEngine::BlockTessellatorBase::CalculateCubeAO()
{
	uint32_t aoFlags = 0;

	Block* pCurBlock = neighborBlocks[rbp_pXpYpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_xyz;
	}

	pCurBlock = neighborBlocks[rbp_nXpYpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_Nxyz;
	}

	pCurBlock = neighborBlocks[rbp_pXpYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_xyNz;
	}

	pCurBlock = neighborBlocks[rbp_nXpYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
		{
			aoFlags |= BlockModel::evf_NxyNz;
		}
	}

	pCurBlock = neighborBlocks[rbp_pYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_topFront;
	}

	pCurBlock = neighborBlocks[rbp_nXpY];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_topLeft;
	}

	pCurBlock = neighborBlocks[rbp_pXpY];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_topRight;
	}

	pCurBlock = neighborBlocks[rbp_pYpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_topBack;
	}

	pCurBlock = neighborBlocks[rbp_nXnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_LeftFront;
	}

	pCurBlock = neighborBlocks[rbp_nXpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_leftBack;
	}

	pCurBlock = neighborBlocks[rbp_pXnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_rightFont;
	}

	pCurBlock = neighborBlocks[rbp_pXpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_rightBack;
	}

	pCurBlock = neighborBlocks[rbp_pXnYPz];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_xNyz;
	}

	pCurBlock = neighborBlocks[rbp_pXnYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_xNyNz;
	}

	pCurBlock = neighborBlocks[rbp_nXnYPz];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_NxNyz;
	}

	pCurBlock = neighborBlocks[rbp_nXnYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_NxNyNz;
	}

	pCurBlock = neighborBlocks[rbp_nYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_bottomFront;
	}

	pCurBlock = neighborBlocks[rbp_nXnY];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_bottomLeft;
	}

	pCurBlock = neighborBlocks[rbp_pXnY];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_bottomRight;
	}

	pCurBlock = neighborBlocks[rbp_nYpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_bottomBack;
	}	
	return aoFlags;
}

//////////////////////////////////////////////////////////
//
// BlockGeneralTessellator
//
//////////////////////////////////////////////////////////

ParaEngine::BlockGeneralTessellator::BlockGeneralTessellator(CBlockWorld* pWorld) : BlockTessellatorBase(pWorld)
{
}

int32 ParaEngine::BlockGeneralTessellator::TessellateBlock(BlockChunk* pChunk, uint16 packedBlockId, BlockRenderMethod dwShaderID, BlockVertexCompressed** pOutputData)
{
	if (!UpdateCurrentBlock(pChunk, packedBlockId))
		return 0;

	if (m_pCurBlockTemplate->IsMatchAttribute(BlockTemplate::batt_liquid))
	{
		// water, ice or other transparent cube blocks
		// adjacent faces of the same liquid type will be removed. 
		TessellateLiquidOrIce(dwShaderID);
	}
	else
	{
		if (m_pCurBlockModel->IsUsingSelfLighting())
		{
			// like wires, etc. 
			TessellateSelfLightingCustomModel(dwShaderID);
		}
		else if (m_pCurBlockModel->IsUniformLighting())
		{
			// custom models like stairs, slabs, button, torch light, grass, etc. 
			TessellateUniformLightingCustomModel(dwShaderID);
		}
		else
		{
			// standard cube including tree leaves. 
			TessellateStdCube(dwShaderID);
		}
	}
	int nFaceCount = tessellatedModel.GetFaceCount();
	if (nFaceCount > 0 )
	{ 
		tessellatedModel.TranslateVertices(m_blockId_cs.x, m_blockId_cs.y, m_blockId_cs.z);
		*pOutputData = tessellatedModel.GetVertices();
	}
	return nFaceCount;
}


void ParaEngine::BlockGeneralTessellator::TessellateUniformLightingCustomModel(BlockRenderMethod dwShaderID)
{
	int nFetchNearybyCount = 7; //  m_pCurBlockTemplate->IsTransparent() ? 7 : 1;
	FetchNearbyBlockInfo(m_pChunk, m_blockId_cs, nFetchNearybyCount);

	tessellatedModel.CloneVertices(m_pCurBlockTemplate->GetBlockModel(m_pWorld, m_blockId_ws.x, m_blockId_ws.y, m_blockId_ws.z, (uint16)m_nBlockData, neighborBlocks));

	const uint16_t nFaceCount = m_pCurBlockModel->GetFaceCount();

	// custom model does not use AO and does not remove any invisible faces. 
	int32_t max_light = 0;
	int32_t max_sun_light = 0;
	int32_t max_block_light = 0;

	if (dwShaderID == BLOCK_RENDER_FIXED_FUNCTION)
	{
		max_light = GetMeshBrightness(m_pCurBlockTemplate, &(blockBrightness[rbp_center]));

		// not render completely dark
		max_light = Math::Max(max_light, 2);
		float fLightValue = m_pWorld->GetLightBrightnessLinearFloat(max_light);

		for (int face = 0; face < nFaceCount; ++face)
		{
			int nFirstVertex = face * 4;
			for (int v = 0; v < 4; ++v)
			{
				tessellatedModel.SetLightIntensity(nFirstVertex + v, fLightValue);
			}
		}
	}
	else
	{
		max_sun_light = GetMeshBrightness(m_pCurBlockTemplate, &(blockBrightness[rbp_center + nFetchNearybyCount * 2]));
		max_block_light = GetMeshBrightness(m_pCurBlockTemplate, &(blockBrightness[rbp_center + nFetchNearybyCount]));

		uint8 block_lightvalue = m_pWorld->GetLightBrightnessInt(max_block_light);
		uint8 sun_lightvalue = max_sun_light << 4;
		for (int face = 0; face < nFaceCount; ++face)
		{
			int nFirstVertex = face * 4;
			for (int v = 0; v < 4; ++v)
			{
				tessellatedModel.SetVertexLight(nFirstVertex + v, block_lightvalue, sun_lightvalue);
			}
		}
	}
}

void ParaEngine::BlockGeneralTessellator::TessellateSelfLightingCustomModel(BlockRenderMethod dwShaderID)
{
	FetchNearbyBlockInfo(m_pChunk, m_blockId_cs, 19, 0);
	tessellatedModel.CloneVertices(m_pCurBlockTemplate->GetBlockModel(m_pWorld, m_blockId_ws.x, m_blockId_ws.y, m_blockId_ws.z, (uint16)m_nBlockData, neighborBlocks));

	if (m_pCurBlockModel->IsUseAmbientOcclusion())
	{
		uint32 aoFlags = CalculateCubeAO();
		const uint16_t nFaceCount = tessellatedModel.GetFaceCount();
		for (int face = 0; face < nFaceCount; ++face)
		{
			int nIndex = face * 4;
			tessellatedModel.SetVertexShadowFromAOFlags(nIndex, nIndex, aoFlags);
		}
	}
}

int32 VertexVerticalScaleMaskMap[] = {
	3, // evf_NxyNz, //g_topLB 
	1, // evf_Nxyz, //g_topLT 
	0, // evf_xyz, //g_topRT 
	2, // evf_xyNz, //g_topRB 
	-1, //g_frtLB 
	3, // evf_NxyNz, //g_frtLT 
	2, // evf_xyNz, //g_frtRT 
	-1, //g_frtRB 
	-1, //g_btmLB 
	-1, //g_btmLT 
	-1, //g_btmRT 
	-1, //g_btmRB 
	-1, // g_leftLB 
	1, // evf_Nxyz, // g_leftLT 
	3, // evf_NxyNz, // g_leftRT 
	-1, // g_leftRB 
	-1, // g_rightLB
	2, // evf_xyNz, // g_rightLT
	0, // evf_xyz, // g_rightRT
	-1, // g_rightRB
	-1, // g_bkLB 
	0, // evf_xyz, // g_bkLT 
	1, // evf_Nxyz, // g_bkRT 
	-1, // g_bkRB 
};

void ParaEngine::BlockGeneralTessellator::TessellateLiquidOrIce(BlockRenderMethod dwShaderID)
{
	FetchNearbyBlockInfo(m_pChunk, m_blockId_cs, 27);
	
	uint32 aoFlags = 0;
	if (m_pCurBlockModel->IsUseAmbientOcclusion())
	{
		aoFlags = CalculateCubeAO();
	}

	const uint16_t nFaceCount = m_pCurBlockModel->GetFaceCount();
	PE_ASSERT(nFaceCount <= 6);

	bool bHasTopScale = false;
	float TopFaceVerticalScales[] = {1.f, 1.f, 1.f, 1.f};

	DWORD dwBlockColor = m_pCurBlockTemplate->GetDiffuseColor(m_nBlockData);
	const bool bHasColorData = dwBlockColor != Color::White;

	//----------calc vertex lighting----------------
	for (int face = 0; face < nFaceCount; ++face)
	{
		int nFirstVertex = face * 4;

		Block* pCurBlock = neighborBlocks[BlockCommon::RBP_SixNeighbors[face]];

		if (!(pCurBlock && 
			( (pCurBlock->GetTemplate()->IsAssociatedBlockID(m_pCurBlockTemplate->GetID()) 
			// TODO: we should show the face when two transparent color blocks with different color are side by side.
			// However, since we are not doing face sorting anyway, this feature is turned off at the moment. 
			// && pCurBlock->GetTemplate()->GetDiffuseColor(pCurBlock->GetUserData()) == dwBlockColor
			)
			|| (face != 0 && pCurBlock->GetTemplate()->IsFullyOpaque()))))
		{
			int32_t baseIdx = nFirstVertex * 4;
			int32_t v1 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx]];
			int32_t max_light = Math::Max(v1, 2);
			bool bHideFace = false;


			if (face == 0 && !m_pCurBlockTemplate->IsMatchAttribute(BlockTemplate::batt_solid))
			{

				// check top face, just in case we need to scale edge water block according to gravity. 
				for (int v = 0; v < 4; ++v)
				{
					int i = nFirstVertex + v;
					int32_t baseIdx = i * 4;

					// if both of the two adjacent blocks to the edge vertex are empty, we will scale that edge vertex to 0 height.  
					Block* b2;
					Block* b3;
					if (v == 0){
						b2 = neighborBlocks[rbp_nX];
						b3 = neighborBlocks[rbp_nZ];
						if (!((b2 && b2->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid | BlockTemplate::batt_obstruction | BlockTemplate::batt_liquid)) ||
							(b3 && b3->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid | BlockTemplate::batt_obstruction | BlockTemplate::batt_liquid))))
						{
							// BlockModel::evf_NxyNz
							TopFaceVerticalScales[3] = 0.4f; 
							bHasTopScale = true;
						}
						else
						{
							if (!((neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 1]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 1]]->GetTemplate() == m_pCurBlockTemplate) ||
								(neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 2]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 2]]->GetTemplate() == m_pCurBlockTemplate) ||
								(neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 3]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 3]]->GetTemplate() == m_pCurBlockTemplate)))
							{
								// BlockModel::evf_NxyNz
								TopFaceVerticalScales[3] = 0.8f; // surface block is always a little lower
								bHasTopScale = true;
							}
						}
					}
					else if (v == 1){
						b2 = neighborBlocks[rbp_nX];
						b3 = neighborBlocks[rbp_pZ];
						if (!((b2 && b2->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid | BlockTemplate::batt_obstruction | BlockTemplate::batt_liquid)) ||
							(b3 && b3->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid | BlockTemplate::batt_obstruction | BlockTemplate::batt_liquid))))
						{
							// BlockModel::evf_Nxyz
							TopFaceVerticalScales[1] = 0.4f;
							bHasTopScale = true;
						}
						else
						{
							if (!((neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 1]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 1]]->GetTemplate() == m_pCurBlockTemplate) ||
								(neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 2]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 2]]->GetTemplate() == m_pCurBlockTemplate) ||
								(neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 3]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 3]]->GetTemplate() == m_pCurBlockTemplate)))
							{
								// BlockModel::evf_Nxyz
								TopFaceVerticalScales[1] = 0.8f;
								bHasTopScale = true;
							}
						}
					}
					else if (v == 2){
						b2 = neighborBlocks[rbp_pX];
						b3 = neighborBlocks[rbp_pZ];
						if (!((b2 && b2->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid | BlockTemplate::batt_obstruction | BlockTemplate::batt_liquid)) ||
							(b3 && b3->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid | BlockTemplate::batt_obstruction | BlockTemplate::batt_liquid))))
						{
							// BlockModel::evf_xyz
							TopFaceVerticalScales[0] = 0.4f;
							bHasTopScale = true;
						}
						else
						{
							if (!((neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 1]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 1]]->GetTemplate() == m_pCurBlockTemplate) ||
								(neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 2]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 2]]->GetTemplate() == m_pCurBlockTemplate) ||
								(neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 3]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 3]]->GetTemplate() == m_pCurBlockTemplate)))
							{
								// BlockModel::evf_xyz
								TopFaceVerticalScales[0] = 0.8f;
								bHasTopScale = true;
							}
						}
					}
					else if (v == 3){
						b2 = neighborBlocks[rbp_pX];
						b3 = neighborBlocks[rbp_nZ];
						if (!((b2 && b2->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid | BlockTemplate::batt_obstruction | BlockTemplate::batt_liquid)) ||
							(b3 && b3->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid | BlockTemplate::batt_obstruction | BlockTemplate::batt_liquid))))
						{
							// BlockModel::evf_xyNz
							TopFaceVerticalScales[2] = 0.4f;
							bHasTopScale = true;
						}
						else
						{
							if (!((neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 1]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 1]]->GetTemplate() == m_pCurBlockTemplate) ||
								(neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 2]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 2]]->GetTemplate() == m_pCurBlockTemplate) ||
								(neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 3]] && neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx + 3]]->GetTemplate() == m_pCurBlockTemplate)))
							{
								// BlockModel::evf_xyNz
								TopFaceVerticalScales[2] = 0.8f;
								bHasTopScale = true;
							}
						}
					}
				}

				if (!bHasTopScale && (pCurBlock && pCurBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid)))
				{
					bHideFace = true;
				}
			}

			if (!bHideFace)
			{
				for (int v = 0; v < 4; ++v)
				{
					int i = nFirstVertex + v;

					int nIndex = tessellatedModel.AddVertex(*m_pCurBlockModel, i);
					if (bHasTopScale && VertexVerticalScaleMaskMap[i] >= 0)
					{
						float fScale = TopFaceVerticalScales[VertexVerticalScaleMaskMap[i]];
						if (fScale != 1.f)
						{
							tessellatedModel.SetVertexHeightScale(nIndex, fScale);
						}
					}

					if (dwShaderID == BLOCK_RENDER_FIXED_FUNCTION)
					{
						tessellatedModel.SetLightIntensity(nIndex, m_pWorld->GetLightBrightnessLinearFloat(max_light));
					}
					else
					{
						if (m_pCurBlockTemplate->IsMatchAttribute(BlockTemplate::batt_solid))
							tessellatedModel.SetVertexLight(nIndex, m_pWorld->GetLightBrightnessInt(blockBrightness[BlockCommon::NeighborLightOrder[baseIdx] + 27]), blockBrightness[BlockCommon::NeighborLightOrder[baseIdx] + 27 * 2] << 4);
						else
							tessellatedModel.SetVertexLight(nIndex, m_pWorld->GetLightBrightnessInt(blockBrightness[27]), blockBrightness[27 * 2] << 4);
					}

					tessellatedModel.SetVertexShadowFromAOFlags(nIndex, i, aoFlags);
					if (bHasColorData)
					{
						tessellatedModel.SetVertexColor(nIndex, dwBlockColor);
					}
				}
				tessellatedModel.IncrementFaceCount(1);
			}
		}
	}
}

void ParaEngine::BlockGeneralTessellator::TessellateStdCube(BlockRenderMethod dwShaderID)
{
	FetchNearbyBlockInfo(m_pChunk, m_blockId_cs, 27);

	uint32 aoFlags = 0;
	if (m_pCurBlockModel->IsUseAmbientOcclusion())
	{
		aoFlags = CalculateCubeAO();
	}

	const uint16_t nFaceCount = m_pCurBlockModel->GetFaceCount();
	PE_ASSERT(nFaceCount <= 6);

	DWORD dwBlockColor = m_pCurBlockTemplate->GetDiffuseColor(m_nBlockData);
	const bool bHasColorData = dwBlockColor!=Color::White;

	for (int face = 0; face < nFaceCount; ++face)
	{
		int nFirstVertex = face * 4;

		Block* pCurBlock = neighborBlocks[BlockCommon::RBP_SixNeighbors[face]];

		if (!pCurBlock || (pCurBlock->GetTemplate()->GetLightOpacity() < 15))
		{
			for (int v = 0; v < 4; ++v)
			{
				int i = nFirstVertex + v;
				int32_t baseIdx = i * 4;

				int32_t max_light = 0;
				int32_t max_sun_light = 0;
				int32_t max_block_light = 0;

				if (dwShaderID == BLOCK_RENDER_FIXED_FUNCTION)
				{
					int32_t v1 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx]];
					int32_t v2 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx + 1]];
					int32_t v3 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx + 2]];
					int32_t v4 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx + 3]];
					max_light = GetAvgVertexLight(v1, v2, v3, v4);
				}
				else
				{
					int32_t v1 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx] + 27];
					int32_t v2 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx + 1] + 27];
					int32_t v3 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx + 2] + 27];
					int32_t v4 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx + 3] + 27];
					max_block_light = GetAvgVertexLight(v1, v2, v3, v4);

					v1 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx] + 27 * 2];
					v2 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx + 1] + 27 * 2];
					v3 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx + 2] + 27 * 2];
					v4 = blockBrightness[BlockCommon::NeighborLightOrder[baseIdx + 3] + 27 * 2];

					max_sun_light = GetAvgVertexLight(v1, v2, v3, v4);
				}

				Block* pCurBlock1 = neighborBlocks[BlockCommon::NeighborLightOrder[baseIdx]];
				if (pCurBlock1 && pCurBlock1->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid))
				{
					// simulate ao but not render completely dark. 
					max_light -= 3;
					max_sun_light -= 3;
					max_block_light -= 3;
				}

				int nIndex = tessellatedModel.AddVertex(*m_pCurBlockModel, i);
				if (dwShaderID == BLOCK_RENDER_FIXED_FUNCTION)
				{
					max_light = Math::Max(max_light, 2);
					tessellatedModel.SetLightIntensity(nIndex, m_pWorld->GetLightBrightnessLinearFloat(max_light));
				}
				else
				{
					max_sun_light = Math::Max(max_sun_light, 0);
					max_block_light = Math::Max(max_block_light, 0);
					tessellatedModel.SetVertexLight(nIndex, m_pWorld->GetLightBrightnessInt(max_block_light), max_sun_light << 4);
				}

				tessellatedModel.SetVertexShadowFromAOFlags(nIndex, i, aoFlags);
				if (bHasColorData)
				{
					tessellatedModel.SetVertexColor(nIndex, dwBlockColor);
				}
			}
			tessellatedModel.IncrementFaceCount(1);
		}
	}
}
