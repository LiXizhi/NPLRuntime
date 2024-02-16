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
#include "BlockTessellateFastCutConfig.h"
#include <map>
#include <set>
#include <boost/container/small_vector.hpp>

using namespace ParaEngine;
const int oppositeSides[6] = { 2, 5, 0, 4, 3, 1 };

ParaEngine::BlockTessellatorBase::BlockTessellatorBase(CBlockWorld* pWorld)
	: m_pWorld(pWorld), m_pCurBlockTemplate(0), m_pCurBlockModel(0), m_blockId_ws(0, 0, 0), m_nBlockData(0), m_pChunk(0), m_blockId_cs(0, 0, 0)
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

int32 ParaEngine::BlockTessellatorBase::TessellateBlock(BlockChunk* pChunk, uint16 packedBlockId, BlockRenderMethod dwShaderID, BlockVertexCompressed** pOutputData, int materialId)
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
			m_packedBlockId = packedBlockId;
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
		pChunk->QueryNeighborBlockData(blockId_cs, neighborBlocks + 1, 1, nNearbyBlockCount - 1);
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

	if (m_pCurBlockTemplate->IsMatchAttribute(BlockTemplate::batt_invisible))
		return aoFlags;

	Block* pCurBlock = neighborBlocks[rbp_pXpYpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_xyz;
	}

	pCurBlock = neighborBlocks[rbp_nXpYpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_Nxyz;
	}

	pCurBlock = neighborBlocks[rbp_pXpYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_xyNz;
	}

	pCurBlock = neighborBlocks[rbp_nXpYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_NxyNz;
	}

	pCurBlock = neighborBlocks[rbp_pYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_topFront;
	}

	pCurBlock = neighborBlocks[rbp_nXpY];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_topLeft;
	}

	pCurBlock = neighborBlocks[rbp_pXpY];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_topRight;
	}

	pCurBlock = neighborBlocks[rbp_pYpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_topBack;
	}

	pCurBlock = neighborBlocks[rbp_nXnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_LeftFront;
	}

	pCurBlock = neighborBlocks[rbp_nXpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_leftBack;
	}

	pCurBlock = neighborBlocks[rbp_pXnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_rightFont;
	}

	pCurBlock = neighborBlocks[rbp_pXpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_rightBack;
	}

	pCurBlock = neighborBlocks[rbp_pXnYPz];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_xNyz;
	}

	pCurBlock = neighborBlocks[rbp_pXnYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_xNyNz;
	}

	pCurBlock = neighborBlocks[rbp_nXnYPz];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_NxNyz;
	}

	pCurBlock = neighborBlocks[rbp_nXnYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_NxNyNz;
	}

	pCurBlock = neighborBlocks[rbp_nYnZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_bottomFront;
	}

	pCurBlock = neighborBlocks[rbp_nXnY];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_bottomLeft;
	}

	pCurBlock = neighborBlocks[rbp_pXnY];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
			aoFlags |= BlockModel::evf_bottomRight;
	}

	pCurBlock = neighborBlocks[rbp_nYpZ];
	if (pCurBlock)
	{
		if (pCurBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
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

int32 ParaEngine::BlockGeneralTessellator::TessellateBlock(BlockChunk* pChunk, uint16 packedBlockId, BlockRenderMethod dwShaderID, BlockVertexCompressed** pOutputData, int materialId)
{
	if (!UpdateCurrentBlock(pChunk, packedBlockId))
		return 0;

	if (m_pCurBlockTemplate->IsMatchAttribute(BlockTemplate::batt_liquid))
	{
		// water, ice or other transparent cube blocks
		// adjacent faces of the same liquid type will be removed. 
		TessellateLiquidOrIce(dwShaderID, materialId);
	}
	else
	{
		if (m_pCurBlockModel->IsUsingSelfLighting())
		{
			// like wires, etc. 
			TessellateSelfLightingCustomModel(dwShaderID, materialId);
		}
		else if (m_pCurBlockModel->IsUniformLighting())
		{
			// custom models like stairs, slabs, button, torch light, grass, etc. 
			TessellateUniformLightingCustomModel(dwShaderID, materialId);
		}
		else
		{
			// standard cube including tree leaves. 
			TessellateStdCube(dwShaderID, materialId);
		}
	}
	int nFaceCount = tessellatedModel.GetFaceCount();
	if (nFaceCount > 0)
	{
		tessellatedModel.TranslateVertices(m_blockId_cs.x, m_blockId_cs.y, m_blockId_cs.z);
		*pOutputData = tessellatedModel.GetVertices();
	}
	return nFaceCount;
}

bool checkFaceContain(Vector2 *rectSelf,Vector2 *rectTemp) {
	//判断self的每个顶点是否都在temp矩形内部
	for (int i = 0; i < 4; i++) {
		Vector2 & ptSelf = rectSelf[i];
		int num = 4;
		int acc = 0;
		Vector2 dir_0 = ptSelf - rectTemp[0];
		Vector2 dir_1 = ptSelf - rectTemp[1];
		Vector2 dir_2 = ptSelf - rectTemp[2];
		Vector2 dir_3 = ptSelf - rectTemp[3];

		float cross_0 = dir_0.crossProduct(dir_1);
		float cross_1 = dir_1.crossProduct(dir_2);
		float cross_2 = dir_2.crossProduct(dir_3);
		float cross_3 = dir_3.crossProduct(dir_0);

		if (abs(cross_0) < 0.01f) num--; else acc += cross_0 > 0 ? 1 : -1;
		if (abs(cross_1) < 0.01f) num--; else acc += cross_1 > 0 ? 1 : -1;
		if (abs(cross_2) < 0.01f) num--; else acc += cross_2 > 0 ? 1 : -1;
		if (abs(cross_3) < 0.01f) num--; else acc += cross_3 > 0 ? 1 : -1;

		if (acc != num && acc != -num) {
			return false;
		}
	}
	return true;
}


void ParaEngine::BlockGeneralTessellator::TessellateUniformLightingCustomModel(BlockRenderMethod dwShaderID, int materialId)
{
	if (m_pChunk->GetBlockFaceMaterial(m_packedBlockId, 0) != materialId) return;

	int nFetchNearybyCount = 7; //  m_pCurBlockTemplate->IsTransparent() ? 7 : 1;
	FetchNearbyBlockInfo(m_pChunk, m_blockId_cs, nFetchNearybyCount);
	const uint16_t nFaceCount = m_pCurBlockModel->GetFaceCount();

	// custom model does not use AO
	int32_t max_light = 0;
	int32_t max_sun_light = 0;
	int32_t max_block_light = 0;

	DWORD dwBlockColor = m_pCurBlockTemplate->GetDiffuseColor(m_nBlockData);
	const bool bHasColorData = dwBlockColor != Color::White;

	if (m_pCurBlockModel->HasFaceShape())
	{
		// for slope, stairs, slab which have face shape precalculated,
		// We can do a more precision hidden face removal with standard solid cubes and other face shape enabled blocks.
		if (dwShaderID == BLOCK_RENDER_FIXED_FUNCTION)
		{
			max_light = GetMeshBrightness(m_pCurBlockTemplate, &(blockBrightness[rbp_center]));

			// not render completely dark
			max_light = Math::Max(max_light, 2);
			float fLightValue = m_pWorld->GetLightBrightnessLinearFloat(max_light);
			for (int face = 0; face < nFaceCount; ++face)
			{
				int nFirstVertex = face * 4;
				bool bRemoveFace = false;
				if (face < 6)
				{
					Block* pCurBlock = neighborBlocks[BlockCommon::RBP_SixNeighbors[face]];
					bRemoveFace = !(!pCurBlock || m_pCurBlockModel->GetFaceShape(face) == 0 || (pCurBlock->GetTemplate()->GetLightOpacity() < 15 && 
						(pCurBlock->GetFaceShape(oppositeSides[face]) != m_pCurBlockModel->GetFaceShape(face))));
				}
				if (!bRemoveFace)
				{
					for (int v = 0; v < 4; ++v)
					{
						int i = nFirstVertex + v;
						int nIndex = tessellatedModel.AddVertex(*m_pCurBlockModel, i);
						tessellatedModel.SetLightIntensity(nIndex, fLightValue);
						if (bHasColorData)
							tessellatedModel.SetVertexColor(nIndex, dwBlockColor);
					}
					tessellatedModel.IncrementFaceCount(1);
				}
			}
		}
		else
		{
			max_sun_light = GetMeshBrightness(m_pCurBlockTemplate, &(blockBrightness[rbp_center + nFetchNearybyCount * 2]));
			max_block_light = GetMeshBrightness(m_pCurBlockTemplate, &(blockBrightness[rbp_center + nFetchNearybyCount]));

			int curModelId = m_pCurBlockTemplate->GetID();
			auto modelId = BlockTessellateFastCutCfg::GetModelIDFromModelName(m_pCurBlockTemplate->GetModelName());

			uint8 block_lightvalue = m_pWorld->GetLightBrightnessInt(max_block_light);
			uint8 sun_lightvalue = max_sun_light << 4;

			for (int face = 0; face < nFaceCount; ++face)
			{
				int nFirstVertex = face * 4;
				bool bRemoveFace = false;
				if (face < 6)
				{
					Block* pCurBlock = neighborBlocks[BlockCommon::RBP_SixNeighbors[face]];
					bRemoveFace = !(!pCurBlock || m_pCurBlockModel->GetFaceShape(face) == 0 || (pCurBlock->GetTemplate()->GetLightOpacity() < 15 &&
						(pCurBlock->GetFaceShape(oppositeSides[face]) != m_pCurBlockModel->GetFaceShape(face))));
				}
				if (!bRemoveFace)
				{
					for (int v = 0; v < 4; ++v)
					{
						int i = nFirstVertex + v;
						int nIndex = tessellatedModel.AddVertex(*m_pCurBlockModel, i);
						tessellatedModel.SetVertexLight(nIndex, block_lightvalue, sun_lightvalue);
						if (bHasColorData)
							tessellatedModel.SetVertexColor(nIndex, dwBlockColor);
					}
					tessellatedModel.IncrementFaceCount(1);
				}
			}
		}
		return;
	}

    // for other models, that does not remove hidden surfaces. 
	tessellatedModel.CloneVertices(m_pCurBlockTemplate->GetBlockModel(m_pWorld, m_blockId_ws.x, m_blockId_ws.y, m_blockId_ws.z, (uint16)m_nBlockData, neighborBlocks));

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
				int nIndex = nFirstVertex + v;
				tessellatedModel.SetLightIntensity(nIndex, fLightValue);

				if (bHasColorData)
				{
					tessellatedModel.SetVertexColor(nIndex, dwBlockColor);
				}
			}
		}
	}
	else
	{
		max_sun_light = GetMeshBrightness(m_pCurBlockTemplate, &(blockBrightness[rbp_center + nFetchNearybyCount * 2]));
		max_block_light = GetMeshBrightness(m_pCurBlockTemplate, &(blockBrightness[rbp_center + nFetchNearybyCount]));

		int curModelId = m_pCurBlockTemplate->GetID();
		auto modelId = BlockTessellateFastCutCfg::GetModelIDFromModelName(m_pCurBlockTemplate->GetModelName());

		uint8 block_lightvalue = m_pWorld->GetLightBrightnessInt(max_block_light);
		uint8 sun_lightvalue = max_sun_light << 4;
		
		int tempFaceCount = nFaceCount;
// obsoleted, now we use face shape instead
#ifdef USE_OLD_DATA_MAPPING
		int cur_id_data = m_nBlockData;
		cur_id_data = cur_id_data & 0xff;

#ifdef USE_CUT_CFG
		if (modelId > 0) {
			// here we use small_vector instead of std::vector to avoid heap memory allocations.
			boost::container::small_vector<int32, 20>  facesNeedCut;
			for (int dir = rbp_pX; dir <= rbp_nZ; ++dir)
			{
				Block* tempBlock = neighborBlocks[dir];
				if (tempBlock)
				{
					int temp_id_data = tempBlock->GetUserData();
					temp_id_data = temp_id_data & 0xff; // TODO: 16 bits color blocks or solid cubes?
					BlockModel& tempModel = tempBlock->GetTemplate()->GetBlockModel(temp_id_data);
					auto neighbourModelId = BlockTessellateFastCutCfg::GetModelIDFromModelName(tempBlock->GetTemplate()->GetModelName());
					if (neighbourModelId > 0)
					{
						int intKey = modelId * 10000000 + cur_id_data * 100000 + dir * 1000 + neighbourModelId * 100 + temp_id_data * 1;

						auto* curInfo = BlockTessellateFastCutCfg::GetCutInfo(intKey);
						if (curInfo != NULL) {
							auto& faces = curInfo->faces;

							for (int i = 0; i < 10 && faces[i]>0; i++) {
								facesNeedCut.push_back(faces[i] - 1);
							}
						}
						//OUTPUT_LOG("\"%s_%d_%d__%s_%d = %d\",\n", curModelName.c_str(), cur_id_data, dir, tempModelName.c_str(), temp_id_data, selfFace);
					}
				}
			}
			std::sort(facesNeedCut.begin(), facesNeedCut.end());
			auto& verts = tessellatedModel.Vertices();
			for (auto iter = facesNeedCut.rbegin(); iter != facesNeedCut.rend(); iter++) {
				int selfFace = *iter;
				int start = selfFace * 4;   //去掉这个面的四个顶点，并前移数组
				for (int v = start; v < tempFaceCount * 4 - 4; v++) {
					verts[v] = verts[v + 4];
				}
				for (int v = 0; v < 4; v++) {
					verts.pop_back();
				}
				tempFaceCount--;
			}
			tessellatedModel.SetFaceCount(tempFaceCount);
		}
		
#else
		const std::string &curModelName = m_pCurBlockTemplate->GetModelName();
		if (curModelName=="slab"||curModelName=="stairs"||curModelName=="slope") {
			boost::container::small_vector<int32, 20> facesNeedCut;
			const static Vector3 normals[7] = {
				Vector3(),

				Vector3(1,0,0),
				Vector3(-1,0,0),
				Vector3(0,1,0),
				Vector3(0,-1,0),
				Vector3(0,0,1),
				Vector3(0,0,-1),
			};
			int cur_id_data = m_nBlockData;
			cur_id_data = cur_id_data & 0xff;

			static boost::container::small_vector<int32, 20> selfFaces;//当前方块正对邻居的面
			static boost::container::small_vector<int32, 20> tempFaces;//邻居正对当前方块的面

			static Vector2 selfRect[4], tempRect[4];

			for (int dir = rbp_pX; dir <= rbp_nZ; ++dir)//遍历6个方向的邻居
			{
				Block* tempBlock = neighborBlocks[dir];
				//这个面有个实体邻居
				if (tempBlock)
				{
					int temp_id_data = tempBlock->GetUserData();
					temp_id_data = temp_id_data & 0xff;
					BlockModel& tempModel = tempBlock->GetTemplate()->GetBlockModel(temp_id_data);
					const string &tempModelName = tempBlock->GetTemplate()->GetModelName();
					
					selfFaces.clear();
					tempFaces.clear();

					for (int selfFace = tempFaceCount - 1; selfFace >= 0; selfFace--) { //遍历自己所有面
						int nFirstVertex = selfFace * 4;
						Vector3  &selfNormal = tessellatedModel.Vertices()[nFirstVertex].GetNormal();
						float angle = selfNormal.angleBetween(normals[dir]);
						if (abs(angle) < 0.01f) { //找到正对着邻居的一面（可能有多个，比如stair）
							selfFaces.push_back(selfFace);
						}
					}
					bool isNeighborCube = !tempModel.IsUniformLighting() && tempBlock->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid);

					if (!(isNeighborCube || tempModelName == "slab" || tempModelName == "stairs" || tempModelName == "slope")) {
						continue;
					}
					for (int tempFace = 0; tempFace < tempModel.GetFaceCount(); tempFace++) {
						int nTempFirstVertex = tempFace * 4;
						float angle = tempModel.Vertices()[nTempFirstVertex].GetNormal().angleBetween(normals[dir]);
						if (abs(abs(angle) - 3.141592f) < 0.01f) {
							tempFaces.push_back(tempFace);
						}
					}
					for (size_t i = 0; i < selfFaces.size(); i++) {
						int selfFace = selfFaces[i];
						for (size_t j = 0; j < tempFaces.size(); j++) {
							int tempFace = tempFaces[j];
							//判断两个面是否是重叠
							bool isRealNeighbor = false;
							BlockVertexCompressed &fristVert = tessellatedModel.Vertices()[selfFace * 4];//当前方块此面的第一个顶点
							BlockVertexCompressed &tempVert = tempModel.Vertices()[tempFace * 4];//邻居方块此面的第一个顶点

							if (dir == rbp_pY) {
								if (abs(tempVert.position[1] + 1.0f - fristVert.position[1]) < 1e-03) {
									isRealNeighbor = true;

									for (int i = 0; i < 4; i++) {
										BlockVertexCompressed &_vert = tessellatedModel.Vertices()[selfFace * 4 + i];
										selfRect[i][0] = _vert.position[0];
										selfRect[i][1] = _vert.position[2];

										BlockVertexCompressed &_vert2 = tempModel.Vertices()[tempFace * 4 + i];
										tempRect[i][0] = _vert2.position[0];
										tempRect[i][1] = _vert2.position[2];
									}


								}
							}
							else if (dir == rbp_nY) {
								if (abs(tempVert.position[1] - 1.0f - fristVert.position[1]) < 1e-03) {
									isRealNeighbor = true;

									for (int i = 0; i < 4; i++) {
										BlockVertexCompressed &_vert = tessellatedModel.Vertices()[selfFace * 4 + i];
										selfRect[i][0] = _vert.position[0];
										selfRect[i][1] = _vert.position[2];

										BlockVertexCompressed &_vert2 = tempModel.Vertices()[tempFace * 4 + i];
										tempRect[i][0] = _vert2.position[0];
										tempRect[i][1] = _vert2.position[2];
									}
								}
							}
							else if (dir == rbp_pX) {
								if (abs(tempVert.position[0] + 1.0f - fristVert.position[0]) < 1e-03) {
									isRealNeighbor = true;

									for (int i = 0; i < 4; i++) {
										BlockVertexCompressed &_vert = tessellatedModel.Vertices()[selfFace * 4 + i];
										selfRect[i][0] = _vert.position[1];
										selfRect[i][1] = _vert.position[2];

										BlockVertexCompressed &_vert2 = tempModel.Vertices()[tempFace * 4 + i];
										tempRect[i][0] = _vert2.position[1];
										tempRect[i][1] = _vert2.position[2];
									}
								}
							}
							else if (dir == rbp_nX) {
								if (abs(tempVert.position[0] - 1.0f - fristVert.position[0]) < 1e-03) {
									isRealNeighbor = true;

									for (int i = 0; i < 4; i++) {
										BlockVertexCompressed &_vert = tessellatedModel.Vertices()[selfFace * 4 + i];
										selfRect[i][0] = _vert.position[1];
										selfRect[i][1] = _vert.position[2];

										BlockVertexCompressed &_vert2 = tempModel.Vertices()[tempFace * 4 + i];
										tempRect[i][0] = _vert2.position[1];
										tempRect[i][1] = _vert2.position[2];
									}
								}
							}
							else if (dir == rbp_pZ) {
								if (abs(tempVert.position[2] + 1.0f - fristVert.position[2]) < 1e-03) {
									isRealNeighbor = true;

									for (int i = 0; i < 4; i++) {
										BlockVertexCompressed &_vert = tessellatedModel.Vertices()[selfFace * 4 + i];
										selfRect[i][0] = _vert.position[0];
										selfRect[i][1] = _vert.position[1];

										BlockVertexCompressed &_vert2 = tempModel.Vertices()[tempFace * 4 + i];
										tempRect[i][0] = _vert2.position[0];
										tempRect[i][1] = _vert2.position[1];
									}
								}
							}
							else if (dir == rbp_nZ) {
								if (abs(tempVert.position[2] - 1.0f - fristVert.position[2]) < 1e-03) {
									isRealNeighbor = true;

									for (int i = 0; i < 4; i++) {
										BlockVertexCompressed &_vert = tessellatedModel.Vertices()[selfFace * 4 + i];
										selfRect[i][0] = _vert.position[0];
										selfRect[i][1] = _vert.position[1];

										BlockVertexCompressed &_vert2 = tempModel.Vertices()[tempFace * 4 + i];
										tempRect[i][0] = _vert2.position[0];
										tempRect[i][1] = _vert2.position[1];
									}
								}
							}
							if (isRealNeighbor) {
								isRealNeighbor = checkFaceContain(selfRect, tempRect);
							}

							//isRealNeighbor = true;
							if (isRealNeighbor) {//判断这个面确实是跟邻居重合的
								facesNeedCut.push_back(selfFace);
								auto neighbourModelId = BlockTessellateFastCutCfg::GetModelIDFromModelName(tempBlock->GetTemplate()->GetModelName());
								if (isNeighborCube) {
									temp_id_data = 0;
									neighbourModelId = BlockTessellateFastCutCfg::GetModelIDFromModelName("solidCube");
								}
								int intKey = modelId * 10000000 + cur_id_data * 100000 + dir * 1000 + neighbourModelId * 100 + temp_id_data;
								OUTPUT_LOG("\"%d = %d\",\n", intKey,selfFace);

								//OUTPUT_LOG("\"%s_%d_%d__%s_%d = %d\",\n", curModelName.c_str(), cur_id_data, dir, tempModelName.c_str(), temp_id_data, selfFace);

								break;//当前方块的当前面已经剪裁，达到目的了，邻居后面的不用再遍历了
							}

							if (tempModel.GetFaceCount() == 6) {//邻居只有6个面的话，那应该只有一个面是与我相邻，后面的不用找了
								break;
							}
						}
						if (nFaceCount == 6) {//如果自己只有6个面，那么只有一个面会与邻居相邻，后面的不用找了
							break;
						}
					}
				}
			}

			std::sort(facesNeedCut.begin(), facesNeedCut.end());
			for (auto iter = facesNeedCut.rbegin(); iter != facesNeedCut.rend(); iter++) {
				int selfFace = *iter;
				int start = selfFace * 4;//去掉这个面的四个顶点，并前移数组
				for (int v = start; v < tempFaceCount * 4 - 4; v++) {
					tessellatedModel.Vertices()[v] = tessellatedModel.Vertices()[v + 4];
				}
				for (int v = 0; v < 4; v++) {
					tessellatedModel.Vertices().pop_back();
				}
				tempFaceCount--;
			}

			tessellatedModel.SetFaceCount(tempFaceCount);
		}
		
#endif
#endif
		for (int face = 0; face < tempFaceCount; ++face)
		{
			int nFirstVertex = face * 4;
			for (int v = 0; v < 4; ++v)
			{
				int nIndex = nFirstVertex + v;
				tessellatedModel.SetVertexLight(nIndex, block_lightvalue, sun_lightvalue);
				if (bHasColorData)
				{
					tessellatedModel.SetVertexColor(nIndex, dwBlockColor);
				}
			}
		}
	}
}

void ParaEngine::BlockGeneralTessellator::TessellateSelfLightingCustomModel(BlockRenderMethod dwShaderID, int materialId)
{
	FetchNearbyBlockInfo(m_pChunk, m_blockId_cs, 19, 0);
	tessellatedModel.CloneVertices(m_pCurBlockTemplate->GetBlockModel(m_pWorld, m_blockId_ws.x, m_blockId_ws.y, m_blockId_ws.z, (uint16)m_nBlockData, neighborBlocks));

	DWORD dwBlockColor = m_pCurBlockTemplate->GetDiffuseColor(m_nBlockData);
	const bool bHasColorData = dwBlockColor != Color::White;

	if (m_pCurBlockModel->IsUseAmbientOcclusion())
	{
		uint32 aoFlags = CalculateCubeAO();
		const uint16_t nFaceCount = tessellatedModel.GetFaceCount();
		for (int face = 0; face < nFaceCount; ++face)
		{
			int nIndex = face * 4;
			tessellatedModel.SetVertexShadowFromAOFlags(nIndex, nIndex, aoFlags);
			if (bHasColorData)
			{
				tessellatedModel.SetVertexColor(nIndex, dwBlockColor);
			}
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

void ParaEngine::BlockGeneralTessellator::TessellateLiquidOrIce(BlockRenderMethod dwShaderID, int materialId)
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
	float TopFaceVerticalScales[] = { 1.f, 1.f, 1.f, 1.f };

	DWORD dwBlockColor = m_pCurBlockTemplate->GetDiffuseColor(m_nBlockData);
	const bool bHasColorData = dwBlockColor != Color::White;

	//----------calc vertex lighting----------------
	for (int face = 0; face < nFaceCount; ++face)
	{
		// if (m_pChunk->GetBlockFaceMaterial(m_packedBlockId, face) != materialId) continue;

		int nFirstVertex = face * 4;

		Block* pCurBlock = neighborBlocks[BlockCommon::RBP_SixNeighbors[face]];

		if (!(pCurBlock &&
			((pCurBlock->GetTemplate()->IsAssociatedBlockID(m_pCurBlockTemplate->GetID())
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
					if (v == 0) {
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
					else if (v == 1) {
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
					else if (v == 2) {
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
					else if (v == 3) {
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

void ParaEngine::BlockGeneralTessellator::TessellateStdCube(BlockRenderMethod dwShaderID, int materialId)
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
	const bool bHasColorData = dwBlockColor != Color::White;

	int tileSize = m_pCurBlockTemplate->getTileSize();
	float uvScale = (tileSize == 1) ? 1.0f : (1.0f / tileSize);

	auto packedBlockId = CalcPackedBlockID(m_blockId_cs);
	bool bHasBlockMaterial = m_pChunk->HasBlockMaterial(packedBlockId);

	for (int face = 0; face < nFaceCount; ++face)
	{
		int nFirstVertex = face * 4;

		Block* pCurBlock = neighborBlocks[BlockCommon::RBP_SixNeighbors[face]];

		if (((!pCurBlock) || (pCurBlock->GetTemplate()->GetLightOpacity() < 15 && (pCurBlock->GetFaceShape(oppositeSides[face]) != 0xf))) &&
			/** we will skip standard material if there is a block material */
			((materialId < 0 && (!bHasBlockMaterial || m_pChunk->GetBlockFaceMaterial(packedBlockId, (int16)face) < 0)) ||
			/** we will only output the give block material */
			(materialId > 0 && bHasBlockMaterial && m_pChunk->GetBlockFaceMaterial(packedBlockId, (int16)face) == materialId)))
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
				if (pCurBlock1 && pCurBlock1->GetTemplate()->IsMatchAttributes(BlockTemplate::batt_solid | BlockTemplate::batt_invisible, BlockTemplate::batt_solid))
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

				if (m_pCurBlockTemplate->IsMatchAttribute(BlockTemplate::batt_pos_tiling) || m_pCurBlockTemplate->IsMatchAttribute(BlockTemplate::batt_random_tiling))
				{
					BlockVertexCompressed* vert = tessellatedModel.GetVertices() + nIndex;
					Vector2 tran;
					if (m_pCurBlockTemplate->IsMatchAttribute(BlockTemplate::batt_pos_tiling))
						tran = Vector2((float)(m_blockId_cs.x % tileSize), (float)(m_blockId_cs.z % tileSize));
					else
						tran = Vector2((float)(m_nBlockData % tileSize), (float)(m_nBlockData / tileSize));
					float u, v;
					vert->GetTexcoord(u, v);
					vert->SetTexcoord((tran.x + u) * uvScale, (tran.y + v) * uvScale);
				}
			}
			tessellatedModel.IncrementFaceCount(1);
		}
	}
}
