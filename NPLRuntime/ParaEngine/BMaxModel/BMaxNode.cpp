//-----------------------------------------------------------------------------
// Class:Block max frame node
// Authors:	LiXizhi
// Emails:	lixizhi@yeah.net
// Date:	2015.9.26
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockDirection.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BMaxParser.h"
#include "BMaxNode.h"
using namespace ParaEngine;
const int oppositeSides[6] = { 2, 5, 0, 4, 3, 1 };

ParaEngine::BMaxNode::BMaxNode(BMaxParser* pParser, int16 x_, int16 y_, int16 z_, int32 template_id_, int32 block_data_) :
	m_pParser(pParser), x(x_), y(y_), z(z_), template_id(template_id_), block_data(block_data_), m_color(0), m_nBoneIndex(-1), m_pBlockModel(0), m_bIsSolid(true)
{
	memset(m_facesStatus, faceInvisible, sizeof(m_facesStatus));
}

ParaEngine::BMaxNode::~BMaxNode()
{

}

void ParaEngine::BMaxNode::SetColor(DWORD val)
{
	m_color = val;
}

DWORD ParaEngine::BMaxNode::GetColor()
{
	if (m_color == 0)
	{
		auto node_template = BlockWorldClient::GetInstance()->GetBlockTemplate((uint16)template_id);
		if (node_template)
			SetColor(node_template->GetBlockColor(block_data));
		else
			SetColor(Color::White);
	}
	return m_color;
}

BlockModel * ParaEngine::BMaxNode::GetBlockModel()
{
	return m_pBlockModel;
}

ParaEngine::CParaXModel * BMaxNode::GetParaXModel()
{
	return NULL;
}


bool ParaEngine::BMaxNode::HasTransform()
{
	return false;
}

ParaEngine::Matrix4 ParaEngine::BMaxNode::GetTransform()
{
	return Matrix4::IDENTITY;
}

void ParaEngine::BMaxNode::SetBlockModel(BlockModel* pModel)
{
	m_pBlockModel = pModel;
}

BMaxFrameNode* ParaEngine::BMaxNode::ToBoneNode()
{
	return NULL;
}

bool ParaEngine::BMaxNode::HasBoneWeight()
{
	return m_nBoneIndex >= 0;
}

int ParaEngine::BMaxNode::GetBoneIndex()
{
	return m_nBoneIndex;
}

void ParaEngine::BMaxNode::SetBoneIndex(int nIndex)
{
	m_nBoneIndex = nIndex;
}

BMaxNode* ParaEngine::BMaxNode::GetNeighbour(BlockDirection::Side side)
{
	Int32x3 offset = BlockDirection::GetOffsetBySide(side);
	int nX = x + offset.x;
	int nY = y + offset.y;
	int nZ = z + offset.z;
	return m_pParser->GetBMaxNode(nX, nY, nZ);
}

BMaxNode* ParaEngine::BMaxNode::GetNeighbourByOffset(Vector3 offset)
{
	int nX = x + (int)offset.x;
	int nY = y + (int)offset.y;
	int nZ = z + (int)offset.z;
	return m_pParser->GetBMaxNode(nX, nY, nZ);
}

bool ParaEngine::BMaxNode::isSolid()
{
	return m_bIsSolid;
}

void ParaEngine::BMaxNode::setSolid(bool bValue)
{
	m_bIsSolid = bValue;
}

void ParaEngine::BMaxNode::QueryNeighborBlockData(BMaxNode** pBlockData, int nFrom /*= 0*/, int nTo /*= 26*/)
{
	const Int16x3* neighborOfsTable = BlockCommon::NeighborOfsTable;
	for (int i = nFrom; i <= nTo; ++i)
	{
		Int16x3 curBlockId;
		curBlockId.x = x + neighborOfsTable[i].x;
		curBlockId.y = y + neighborOfsTable[i].y;
		curBlockId.z = z + neighborOfsTable[i].z;

		BMaxNode* pBlock = m_pParser->GetBMaxNode(curBlockId.x, curBlockId.y, curBlockId.z);
		pBlockData[i - nFrom] = pBlock;
	}
}

uint32 ParaEngine::BMaxNode::CalculateCubeAO(BMaxNode** neighborBlocks)
{
	uint32 aoFlags = 0;

	BMaxNode* pCurBlock = neighborBlocks[rbp_pXpYpZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_xyz;
	}

	pCurBlock = neighborBlocks[rbp_nXpYpZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_Nxyz;
	}

	pCurBlock = neighborBlocks[rbp_pXpYnZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_xyNz;
	}

	pCurBlock = neighborBlocks[rbp_nXpYnZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_NxyNz;
	}

	pCurBlock = neighborBlocks[rbp_pYnZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_topFront;
	}

	pCurBlock = neighborBlocks[rbp_nXpY];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_topLeft;
	}

	pCurBlock = neighborBlocks[rbp_pXpY];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_topRight;
	}

	pCurBlock = neighborBlocks[rbp_pYpZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_topBack;
	}

	pCurBlock = neighborBlocks[rbp_nXnZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_LeftFront;
	}

	pCurBlock = neighborBlocks[rbp_nXpZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_leftBack;
	}

	pCurBlock = neighborBlocks[rbp_pXnZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_rightFont;
	}

	pCurBlock = neighborBlocks[rbp_pXpZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_rightBack;
	}

	pCurBlock = neighborBlocks[rbp_pXnYPz];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_xNyz;
	}

	pCurBlock = neighborBlocks[rbp_pXnYnZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_xNyNz;
	}

	pCurBlock = neighborBlocks[rbp_nXnYPz];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_NxNyz;
	}

	pCurBlock = neighborBlocks[rbp_nXnYnZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_NxNyNz;
	}

	pCurBlock = neighborBlocks[rbp_nYnZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_bottomFront;
	}

	pCurBlock = neighborBlocks[rbp_nXnY];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_bottomLeft;
	}

	pCurBlock = neighborBlocks[rbp_pXnY];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_bottomRight;
	}

	pCurBlock = neighborBlocks[rbp_nYpZ];
	if (pCurBlock)
	{
		aoFlags |= BlockModel::evf_bottomBack;
	}
	return aoFlags;
}

int32_t ParaEngine::BMaxNode::GetAvgVertexLight(int32_t v1, int32_t v2, int32_t v3, int32_t v4)
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

void BMaxNode::SetFaceVisible(int nIndex)
{
	m_facesStatus[nIndex] = faceVisibleNotSign;
}

void BMaxNode::SetFaceUsed(int nIndex)
{
	m_facesStatus[nIndex] = faceVisibleSigned;
}

bool BMaxNode::IsFaceNotUse(int nIndex)
{
	return m_facesStatus[nIndex] == faceVisibleNotSign;
}

int ParaEngine::BMaxNode::GetFaceShape(int nIndex) 
{
	if(m_bIsSolid)
		return 0xf;
	BlockTemplate* block_template = BlockWorldClient::GetInstance()->GetBlockTemplate((uint16)template_id);
	return block_template ? block_template->GetFaceShapeDirect(nIndex, block_data) : 0;
}

int ParaEngine::BMaxNode::TessellateBlock(BlockModel* tessellatedModel)
{
	int bone_index = GetBoneIndex();

	BlockTemplate* block_template = BlockWorldClient::GetInstance()->GetBlockTemplate((uint16)template_id);
	
	if (block_template == 0 || block_template->isSolidBlock() || block_template->GetID() == BMaxParser::BoneBlockId)
	{
		// standard cube 
		DWORD dwBlockColor = GetColor();

		//neighbor blocks
		const int nNearbyBlockCount = 27;
		BMaxNode* neighborBlocks[nNearbyBlockCount];
		neighborBlocks[rbp_center] = this;
		memset(neighborBlocks + 1, 0, sizeof(BMaxNode*) * (nNearbyBlockCount - 1));
		QueryNeighborBlockData(neighborBlocks + 1, 1, nNearbyBlockCount - 1);

		//ao
		uint32 aoFlags = (block_template && block_template->GetID() != BMaxParser::MetalBlockId) ? CalculateCubeAO(neighborBlocks) : 0;

		// model position offset
		BlockVertexCompressed* pVertices = tessellatedModel->GetVertices();
		int count = tessellatedModel->GetVerticesCount();
		const Vector3& vCenter = m_pParser->GetCenterPos();
		Vector3 vOffset((float)x - vCenter.x, (float)y, (float)z - vCenter.z);
		for (int k = 0; k < count; k++)
		{
			pVertices[k].OffsetPosition(vOffset);
			pVertices[k].SetBlockColor(dwBlockColor);
		}

		for (int face = 0; face < 6; ++face)
		{
			int nFirstVertex = face * 4;

			BMaxNode* pCurBlock = neighborBlocks[BlockCommon::RBP_SixNeighbors[face]];

			// we will preserve the face when two bones does not belong to the same bone
			if (!pCurBlock || (pCurBlock->GetBoneIndex() != bone_index || (!pCurBlock->isSolid() && pCurBlock->GetFaceShape(oppositeSides[face]) != 0xf)))
			{
				if (aoFlags > 0)
				{
					for (int v = 0; v < 4; ++v)
					{
						int i = nFirstVertex + v;

						int nShadowLevel = 0;
						if ((nShadowLevel = tessellatedModel->CalculateCubeVertexAOShadowLevel(i, aoFlags)) != 0)
						{
							Color color(dwBlockColor);
							float fShadow = (255 - nShadowLevel) / 255.f;
							color.r = (uint8)(color.r * fShadow);
							color.g = (uint8)(color.g * fShadow);
							color.b = (uint8)(color.b * fShadow);
							tessellatedModel->SetVertexColor(i, (DWORD)color);
						}
					}
				}
				SetFaceVisible(face);
			}
		}
	}
	else
	{
		// metal blocks, and custom models like stairs, slabs, button, torch light, etc. 
		// they do not have ambient occlusion shadows on them. 
		tessellatedModel->ClearVertices();
		BlockModel& blockModel = block_template->GetBlockModelByData(this->block_data);

		DWORD dwBlockColor = GetColor();
		tessellatedModel->SetUniformLighting(true);
		
		const Vector3& vCenter = m_pParser->GetCenterPos();
		Vector3 vOffset((float)x - vCenter.x, (float)y, (float)z - vCenter.z);

		if (blockModel.HasFaceShape())
		{
			// for slope, stairs, slab which have face shape precalculated,
			// We can do a more precision hidden face removal with standard solid cubes and other face shape enabled blocks.
			const uint16_t nFaceCount = blockModel.GetFaceCount();
			
			//neighbor blocks
			const int nNearbyBlockCount = 7;
			BMaxNode* neighborBlocks[nNearbyBlockCount];
			neighborBlocks[rbp_center] = this;
			memset(neighborBlocks + 1, 0, sizeof(BMaxNode*) * (nNearbyBlockCount - 1));
			QueryNeighborBlockData(neighborBlocks + 1, 1, nNearbyBlockCount - 1);

			for (int face = 0; face < nFaceCount; ++face)
			{
				int nFirstVertex = face * 4;
				if (blockModel.GetVertices()[nFirstVertex].IsEmptyFace())
					continue;
				bool bRemoveFace = false;
				if (face < 6)
				{
					auto pCurBlock = neighborBlocks[BlockCommon::RBP_SixNeighbors[face]];
					bRemoveFace = !(!pCurBlock || blockModel.GetFaceShape(face) == 0 || (!pCurBlock->isSolid() &&
						(pCurBlock->GetFaceShape(oppositeSides[face]) != blockModel.GetFaceShape(face))));
				}
				else
				{
					int nFaceId = blockModel.GetVertices()[nFirstVertex].GetCubeFaceId();
					if (nFaceId >= 0 && blockModel.GetFaceShape(nFaceId) != 0)
					{
						auto pCurBlock = neighborBlocks[BlockCommon::RBP_SixNeighbors[nFaceId]];
						if (pCurBlock)
						{
							auto neighbourFaceShape = pCurBlock->GetFaceShape(oppositeSides[nFaceId]);
							if (neighbourFaceShape == 0xf || neighbourFaceShape == blockModel.GetFaceShape(nFaceId))
							{
								bRemoveFace = true;
							}
						}
					}
				}
				if (!bRemoveFace)
				{
					for (int v = 0; v < 4; ++v)
					{
						int i = nFirstVertex + v;
						int nIndex = tessellatedModel->AddVertex(blockModel, i);
						tessellatedModel->GetVertices()[nIndex].OffsetPosition(vOffset);
						tessellatedModel->GetVertices()[nIndex].SetBlockColor(dwBlockColor);
					}
					tessellatedModel->IncrementFaceCount(1);
				}
			}
		}
		else
		{
			// for other transparent models, we will render without hidden face removal
			tessellatedModel->CloneVertices(blockModel);
			BlockVertexCompressed* pVertices = tessellatedModel->GetVertices();
			int count = tessellatedModel->GetVerticesCount();
			for (int k = 0; k < count; k++)
			{
				pVertices[k].OffsetPosition(vOffset);
				pVertices[k].SetBlockColor(dwBlockColor);
			}
		}
	}

	return tessellatedModel->GetVerticesCount();
}