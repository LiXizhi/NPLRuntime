//-----------------------------------------------------------------------------
// Class:	ParaVoxelModel
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2024.1.17
// Desc: Use a very compact octree to represent voxel model. 
// it is very fast to load and save from disk. i.e. memory and disk layout are the same.
// it is also fairly fast to be used in real-time voxel editing. i.e. only neighbouring nodes are updated.  
// 
// e.g.
// local model = entity:GetInnerObject():GetPrimaryAsset():GetAttributeObject():GetChildAt(0):GetChild("VoxelModel")
// model:SetField("MinVoxelPixelSize", 4);
// model:SetField("SetBlock", "7,7,7,8,254");
// model:CallField("DumpOctree");
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneObject.h"
#include "DynamicAttributeField.h"
#include "ParaXModel.h"
#include "effect_file.h"
#include "StringHelper.h"
#include "ViewportManager.h"
#include "ParaVoxelModel.h"

// max octree depth is 12, which is 4096*4096*4096
#define MAX_VOXEL_DEPTH 12

const uint8_t s_oppositeSides[] = { 1, 0, 3, 2, 5, 4 };
const int32_t s_sideOffset_x[] = { -1, 1, 0, 0, 0, 0 };
const int32_t s_sideOffset_y[] = { 0, 0, -1, 1, 0, 0 };
const int32_t s_sideOffset_z[] = { 0, 0, 0, 0, -1, 1 };

const int32_t s_childOffset_x[] = { 0, 1, 0, 1, 0, 1, 0, 1};
const int32_t s_childOffset_y[] = { 0, 0, 1, 1, 0, 0, 1, 1 };
const int32_t s_childOffset_z[] = { 0, 0, 0, 0, 1, 1, 1, 1 };

using namespace ParaEngine;
VoxelOctreeNode VoxelOctreeNode::EmptyNode(0x0);
VoxelOctreeNode VoxelOctreeNode::FullNode(0xff);

VoxelOctreeNode::VoxelOctreeNode(uint8_t isBlockMask)
	: isBlockMask(isBlockMask), isChildMask(0), colorRGB(0), baseChunkOffset(0), offsetAndShape(0)
{
}

// each of the 8 bits represents if a child node is on the side. 
const uint8_t SideSolidMask[6] = { 0x55, 0xaa, 0x33, 0xcc, 0xf, 0xf0 };
bool ParaEngine::VoxelOctreeNode::IsSideSplited(int nSide) {
	uint8_t mask = SideSolidMask[nSide];
	uint8_t sideMask = (mask & isBlockMask);
	return (sideMask) != mask && sideMask != 0;
}

ParaVoxelModel::ParaVoxelModel()
	: m_fMinVoxelPixelSize(4.f)
{
	m_chunks[CreateGetFreeChunkIndex()]->SafeCreateNode(&VoxelOctreeNode::EmptyNode);
}

ParaVoxelModel::~ParaVoxelModel()
{
	for (int i = 0; i < (int)m_chunks.size(); ++i)
	{
		delete m_chunks[i];
	}
	m_chunks.clear();
}


VoxelOctreeNode* ParaEngine::ParaVoxelModel::GetRootNode()
{
	return &((*m_chunks[0])[0]);
}

int ParaEngine::ParaVoxelModel::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);
	pClass->AddField("SetBlock", FieldType_String, (void*)SetBlock_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("PaintBlock", FieldType_String, (void*)PaintBlock_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("DumpOctree", FieldType_void, (void*)DumpOctree_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("MinVoxelPixelSize", FieldType_Float, (void*)SetMinVoxelPixelSize_s, (void*)GetMinVoxelPixelSize_s, NULL, NULL, bOverride);
	return S_OK;
}

bool ParaVoxelModel::Load(const char* pBuffer, int nCount)
{
	return false;
}

bool ParaVoxelModel::Save(std::vector<char>& output)
{
	return false;
}


inline int ParaEngine::ParaVoxelModel::LevelToDepth(int level)
{
	int nDepth = 0;
	while (level > 1)
	{
		level >>= 1;
		nDepth++;
	}
	return nDepth;
}

int ParaEngine::ParaVoxelModel::CreateGetFreeChunkIndex(int nMinFreeSize)
{
	int nCount = m_chunks.size();
	int nMinSize = MAX_VOXEL_CHUNK_SIZE - nMinFreeSize - 1;
	for (int i = 0; i < nCount; ++i)
	{
		if ((int)m_chunks[i]->GetUsedSize() <= nMinSize)
		{
			return i;
		}
	}
	VoxelChunk* chunk = new VoxelChunk();
	m_chunks.push_back(chunk);
	return m_chunks.size() - 1;
}

VoxelOctreeNode* ParaEngine::ParaVoxelModel::CreateGetChildNode(VoxelOctreeNode* pNode, int nChildIndex)
{
	const uint8_t nChildOffset = pNode->childOffsets[nChildIndex];
	if (!pNode->IsChildAt(nChildIndex))
	{
		const uint8_t nChildShape = nChildOffset;
		// create a new child node
		auto& chunk = *(m_chunks[pNode->GetBaseChunkOffset()]);
		VoxelOctreeNode* pChild = NULL;
		if (chunk.GetFreeSize() < 8)
		{
			// create a new chunk and move all existing child nodes to the new chunk. 
			pNode->SetBaseChunkOffset(CreateGetFreeChunkIndex());
			auto& newChunk = *(m_chunks[pNode->GetBaseChunkOffset()]);
			auto baseChunkIndex = pNode->GetBaseChunkOffset();
			for (int i = 0; i < 8; ++i)
			{
				if (pNode->IsChildAt(i))
				{
					// move child to new chunk
					auto index = newChunk.CreateNode(&chunk[pNode->childOffsets[i]]);
					chunk.erase(pNode->childOffsets[i]);
					pNode->SetChild(i, index);
				}
			}
			auto index = newChunk.CreateNode(&VoxelOctreeNode::FullNode);
			pChild = &(newChunk[index]);
			pChild->SetBaseChunkOffset(baseChunkIndex);
			pNode->SetChild(nChildIndex, index);
		}
		else
		{
			// create in current chunk
			auto index = chunk.CreateNode(&VoxelOctreeNode::FullNode);
			pChild = &(chunk[index]);
			pChild->SetBaseChunkOffset(pNode->GetBaseChunkOffset());
			pNode->SetChild(nChildIndex, index);
		}

		// inherit parent node's color by default
		pChild->SetColor(pNode->GetColor());

		if (!pNode->IsBlockAt(nChildIndex))
			pChild->MakeEmpty();
		else if (nChildShape != 0)
		{
			if (nChildShape & 1) // -x
			{
				pChild->childVoxelShape[0] |= 1;
				pChild->childVoxelShape[2] |= 1;
				pChild->childVoxelShape[4] |= 1;
				pChild->childVoxelShape[6] |= 1;
			}
			if (nChildShape & 2) // +x
			{
				pChild->childVoxelShape[1] |= 2;
				pChild->childVoxelShape[3] |= 2;
				pChild->childVoxelShape[5] |= 2;
				pChild->childVoxelShape[7] |= 2;
			}
			if (nChildShape & 4) // -y
			{
				pChild->childVoxelShape[0] |= 4;
				pChild->childVoxelShape[1] |= 4;
				pChild->childVoxelShape[4] |= 4;
				pChild->childVoxelShape[5] |= 4;
			}
			if (nChildShape & 8) // +y
			{
				pChild->childVoxelShape[2] |= 8;
				pChild->childVoxelShape[3] |= 8;
				pChild->childVoxelShape[6] |= 8;
				pChild->childVoxelShape[7] |= 8;
			}
			if (nChildShape & 16) //-z
			{
				pChild->childVoxelShape[0] |= 16;
				pChild->childVoxelShape[1] |= 16;
				pChild->childVoxelShape[2] |= 16;
				pChild->childVoxelShape[3] |= 16;
			}
			if (nChildShape & 32) // +z
			{
				pChild->childVoxelShape[4] |= 32;
				pChild->childVoxelShape[5] |= 32;
				pChild->childVoxelShape[6] |= 32;
				pChild->childVoxelShape[7] |= 32;
			}
		}
		return pChild;
	}
	else
	{
		return &((*m_chunks[pNode->GetBaseChunkOffset()])[nChildOffset]);
	}
}

VoxelOctreeNode* ParaEngine::ParaVoxelModel::GetChildNode(VoxelOctreeNode* pNode, int nChildIndex)
{
	if (!pNode->IsChildAt(nChildIndex))
	{
		return NULL;
	}
	else
	{
		int nChildOffset = pNode->childOffsets[nChildIndex];
		return &((*m_chunks[pNode->GetBaseChunkOffset()])[nChildOffset]);
	}
}

void ParaEngine::ParaVoxelModel::RemoveNodeChildren(VoxelOctreeNode* pNode, uint8_t isBlockMask)
{
	auto& chunk = *m_chunks[pNode->GetBaseChunkOffset()];
	for (int k = 0; k < 8; ++k)
	{
		if ((isBlockMask & (1 << k)) && pNode->IsChildAt(k))
		{
			auto pChild = GetChildNode(pNode, k);
			RemoveNodeChildren(pChild, 0xff);
			chunk.erase(pNode->childOffsets[k]);
			pNode->RemoveChild(k);
		}
	}
	pNode->isBlockMask &= (~isBlockMask);
}

void ParaEngine::ParaVoxelModel::DumpOctree()
{
	OUTPUT_LOG("dumping ParaVoxelModel %d chunks:\n", (int)m_chunks.size());
	VoxelOctreeNode* pNode = GetRootNode();
	DumpOctreeNode(pNode, 0, 0, 0, 0, 0, 0);
}

void ParaEngine::ParaVoxelModel::DumpOctreeNode(VoxelOctreeNode* pNode, int nDepth, int nChunkIndex, int offset, int x, int y, int z)
{
	char tmp[256];
	StringHelper::fast_sprintf(tmp, "Node[%d][%d]: baseChunkOffset %d, blockMask: #%x opache:%d shapeMask: #%x  color: #%06x childMask: %x\n",
		nChunkIndex, offset, pNode->GetBaseChunkOffset(),
		pNode->isBlockMask, pNode->IsFullySolid() ? 1 : 0, pNode->GetVoxelShape(), pNode->GetColor32(), pNode->isChildMask);
	OUTPUT_LOG(tmp);

	if (pNode->IsLeaf() && (pNode->IsSolid() || pNode->IsEmpty()))
		return;
	for (int i = 0; i < nDepth; i++)
		tmp[i] = '-';
	tmp[nDepth] = '\0';
	
	int nLevel = (int)pow(2, nDepth+1);
	for (int k = 0; k < 8; ++k)
	{
		OUTPUT_LOG("%s", tmp);
		auto nx = x + s_childOffset_x[k];
		auto ny = y + s_childOffset_y[k];
		auto nz = z + s_childOffset_z[k];

		if (pNode->IsChildAt(k))
		{
			OUTPUT_LOG("%d: (%d,%d,%d)/%d ", k, nx, ny, nz, nLevel);
			DumpOctreeNode(GetChildNode(pNode, k), nDepth + 1, pNode->GetBaseChunkOffset(), pNode->childOffsets[k], nx*2, ny * 2, nz * 2);
		}
		else
		{
			OUTPUT_LOG("%d: (%d,%d,%d)/%d #%x %s\n", k, nx,ny,nz, nLevel, pNode->childVoxelShape[k], pNode->IsBlockAt(k) ? "solid" : "");
		}
	}
}

void ParaVoxelModel::SetBlock(uint32 x, uint32 y, uint32 z, int level, int color)
{
	int nDepth = LevelToDepth(level);
	// create get octree node
	VoxelOctreeNode* pNode = GetRootNode();

	nDepth--;
	TempVoxelOctreeNodeRef parentNodes[MAX_VOXEL_DEPTH];
	parentNodes[0] = TempVoxelOctreeNodeRef(pNode, 0, 0, 0, 0);

	int nChildIndex = 0;
	int nLevel = 1;
	for (; nDepth >= 0; nDepth--, nLevel++)
	{
		uint32 lx = (x >> nDepth) & 1, ly = (y >> nDepth) & 1, lz = (z >> nDepth) & 1;
		nChildIndex = lx + (ly << 1) + (lz << 2);
		pNode = CreateGetChildNode(pNode, nChildIndex);
		auto& lastNode = parentNodes[nLevel - 1];
		parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, (lastNode.x << 1) + lx, (lastNode.y << 1) + ly, (lastNode.z << 1) + lz, nLevel, nChildIndex);
	}
	if (color > 0)
	{
		// create or set block
		RemoveNodeChildren(pNode, 0xff);
		pNode->SetColor32((uint32_t)color);
		pNode->MakeFullBlock();
		if (level > 1)
		{
			UpdateNodeParentsSolidityAndColor(parentNodes, nLevel);
			UpdateNodeShape(x, y, z, level);
			MergeNodeAndNeighbours(x, y, z, level);
		}
		else
		{
			// for root node
			pNode->SetVoxelShape(0x3f);
			pNode->childVoxelShape[0] = 0x15;
			pNode->childVoxelShape[1] = 0x16;
			pNode->childVoxelShape[2] = 0x19;
			pNode->childVoxelShape[3] = 0x1a;
			pNode->childVoxelShape[4] = 0x25;
			pNode->childVoxelShape[5] = 0x26;
			pNode->childVoxelShape[6] = 0x29;
			pNode->childVoxelShape[7] = 0x2a;
		}
	}
	else
	{
		// delete block
		if (nLevel >= 2) {
			RemoveNodeChildren(parentNodes[nLevel - 2].pNode, 1 << nChildIndex);
			UpdateNodeParentsSolidityAndColor(parentNodes, nLevel);
			UpdateNodeShape(x, y, z, level);
			MergeNodeAndNeighbours(x, y, z, level);
		}
		else {
			// for root node
			RemoveNodeChildren(pNode, 0xff);
			pNode->MakeEmpty();
			pNode->SetVoxelShape(0);
			pNode->offsetAndShape = 0;
		}
	}
}

void ParaEngine::ParaVoxelModel::SetBlockCmd(const char* cmd)
{
	uint32 x, y, z;
	int level;
	int color;
	if (sscanf(cmd, "%d,%d,%d,%d,%d", &x, &y, &z, &level, &color) == 5)
	{
		SetBlock(x, y, z, level, color);
	}
}

bool ParaEngine::ParaVoxelModel::SetNodeColor(VoxelOctreeNode* pNode, uint32 color)
{
	auto& chunk = *m_chunks[pNode->GetBaseChunkOffset()];
	pNode->SetColor32(color);
	bool isFullySolid = true;
	int nChildCount = 0;
	for (int k = 0; k < 8; ++k)
	{
		if (pNode->IsChildAt(k))
		{
			auto pChild = GetChildNode(pNode, k);
			// merge same color nodes
			if (SetNodeColor(pChild, color)) {
				auto childShape = pChild->GetVoxelShape();
				// TODO: check neaby node shape, if we can remove children.
				RemoveNodeChildren(pNode, 1 << k);
				pNode->isBlockMask |= (1 << k);
				pNode->childVoxelShape[k] = childShape;
			}
			else
				isFullySolid = false;

			nChildCount++;
		}
	}
	return isFullySolid && pNode->IsSolid();
}

void ParaEngine::ParaVoxelModel::PaintBlock(uint32 x, uint32 y, uint32 z, int level, uint32_t color)
{
	int nDepth = LevelToDepth(level);
	VoxelOctreeNode* pNode = GetRootNode();
	TempVoxelOctreeNodeRef parentNodes[MAX_VOXEL_DEPTH];
	parentNodes[0] = TempVoxelOctreeNodeRef(pNode, 0, 0, 0, 0);

	nDepth--;

	int nChildIndex = 0;
	int nLevel = 1;
	for (; nDepth >= 0; nDepth--, nLevel++)
	{
		uint32 lx = (x >> nDepth) & 1, ly = (y >> nDepth) & 1, lz = (z >> nDepth) & 1;
		nChildIndex = lx + (ly << 1) + (lz << 2);
		auto pChild = GetChildNode(pNode, nChildIndex);
		if (pChild) {
			pNode = pChild;
			auto& lastNode = parentNodes[nLevel - 1];
			parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, (lastNode.x << 1) + lx, (lastNode.y << 1) + ly, (lastNode.z << 1) + lz, nLevel, nChildIndex);
		}
		else
		{
			if (nDepth == 0)
			{
				if (pNode->IsBlockAt(nChildIndex) && pNode->GetColor32() != color)
				{
					pNode = CreateGetChildNode(pNode, nChildIndex);
					continue;
				}
			}
			break;
		}
	}
	if (pNode)
	{
		SetNodeColor(pNode, color);

		// also merge parent nodes if they are fully solid and have the same color
		for (int i = nLevel - 1; i >= 0; --i)
		{
			auto pNode = parentNodes[i].pNode;
			auto thisColor = pNode->GetColor();
			if (pNode->IsFullySolid())
			{
				bool isSameColor = true;
				for (int k = 0; k < 8; ++k)
				{
					if (pNode->IsChildAt(k))
					{
						if (thisColor != GetChildNode(pNode, k)->GetColor())
						{
							isSameColor = false;
							break;
						}
					}
				}
				if (isSameColor)
				{
					// TODO: check neaby node shape, if we can remove children.
					RemoveNodeChildren(pNode);
				}
			}
			else
				break;
		}
	}
}

void ParaEngine::ParaVoxelModel::PaintBlockCmd(const char* cmd)
{
	uint32 x, y, z;
	int level;
	int color;
	if (sscanf(cmd, "%d,%d,%d,%d,%d", &x, &y, &z, &level, &color) == 5)
	{
		if (color > 0)
			PaintBlock(x, y, z, level, color);
		else
			SetBlock(x, y, z, level, color);
	}
}

int ParaVoxelModel::GetBlock(uint32 x, uint32 y, uint32 z, int level)
{
	int nDepth = LevelToDepth(level);
	VoxelOctreeNode* pNode = GetRootNode();

	nDepth--;

	int nChildIndex = 0;
	int nLevel = 1;
	for (; nDepth >= 0; nDepth--, nLevel++)
	{
		uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
		nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
		auto pChildNode = GetChildNode(pNode, nChildIndex);
		if (pChildNode)
			pNode = pChildNode;
		else
		{
			return (pNode->IsBlockAt(nChildIndex)) ? pNode->GetColor32() : 0;
		}
	}
	return pNode->GetColor32();
}


VoxelOctreeNode* ParaEngine::ParaVoxelModel::GetNode(int32 x, int32 y, int32 z, int level)
{
	if (x >= 0 && x < level && y >= 0 && y < level && z >= 0 && z < level)
	{
		int nDepth = LevelToDepth(level);
		VoxelOctreeNode* pNode = GetRootNode();

		nDepth--;

		int nChildIndex = 0;
		int nLevel = 1;
		for (; nDepth >= 0 && pNode != NULL; nDepth--, nLevel++)
		{
			uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
			nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
			pNode = GetChildNode(pNode, nChildIndex);
		}
		return pNode;
	}
	else
		return NULL;
}

bool ParaEngine::ParaVoxelModel::UpdateNodeShapeByNeighbourAtLevel(int32 x, int32 y, int32 z, int level, int side, bool isBlock)
{
	bool isThisBlock = false;
	if (x >= 0 && x < level && y >= 0 && y < level && z >= 0 && z < level)
	{
		int nDepth = LevelToDepth(level);
		VoxelOctreeNode* pNode = GetRootNode();

		nDepth--;

		int nChildIndex = 0;
		int nLevel = 1;
		for (; nDepth >= 0; nDepth--, nLevel++)
		{
			uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
			nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
			auto pChildNode = GetChildNode(pNode, nChildIndex);
			if (pChildNode)
				pNode = pChildNode;
			else
			{
				if (nDepth == 0)
				{
					if (pNode->IsBlockAt(nChildIndex))
					{
						isThisBlock = true;
						if (!isBlock)
							pNode->childVoxelShape[nChildIndex] |= (1 << side);
						else
							pNode->childVoxelShape[nChildIndex] &= (~(1 << side));
					}
					else
						pNode->childVoxelShape[nChildIndex] = 0;
				}
				else
				{
					isThisBlock = pNode->IsBlockAt(nChildIndex);
				}
				return isThisBlock;
			}
		}
		if (nDepth <= 0 && pNode)
		{
			if (pNode->IsBlock())
			{
				isThisBlock = true;
				if (!isBlock)
					pNode->SetVoxelShape(pNode->GetVoxelShape() | (1 << side));
				else
					pNode->SetVoxelShape(pNode->GetVoxelShape() & (~(1 << side)));
			}
			else
				pNode->SetVoxelShape(0);
		}
	}
	return isThisBlock;
}

void ParaEngine::ParaVoxelModel::UpdateNodeAndChildShapeByNeighbour(int32 x, int32 y, int32 z, int level, int side, bool isSolidOrEmpty)
{
	if (x >= 0 && x < level && y >= 0 && y < level && z >= 0 && z < level)
	{
		int nDepth = LevelToDepth(level);
		VoxelOctreeNode* pNode = GetRootNode();

		nDepth--;
		int nChildIndex = 0;
		int nLevel = 1;
		for (; nDepth >= 0; nDepth--, nLevel++)
		{
			uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
			nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
			auto pChildNode = GetChildNode(pNode, nChildIndex);
			if (pChildNode)
				pNode = pChildNode;
			else
			{
				if (nDepth == 0)
				{
					if (pNode->IsBlockAt(nChildIndex))
					{
						if (!isSolidOrEmpty)
							pNode->childVoxelShape[nChildIndex] |= (1 << side);
						else
							pNode->childVoxelShape[nChildIndex] &= (~(1 << side));
					}
					else
						pNode->childVoxelShape[nChildIndex] = 0;
				}
				// no child nodes, we have finished and return
				return;
			}
		}
		if (nDepth <= 0 && pNode)
		{
			// update all child nodes that are adjacent to the side
			uint8_t sideMask = SideSolidMask[side];

			// create a recursive function to update all child nodes
			std::function<void(VoxelOctreeNode*)> updateChildShape;
			updateChildShape = [&sideMask, &side, &isSolidOrEmpty, &updateChildShape, this](VoxelOctreeNode* pNode) {
				if (pNode->IsBlock())
				{
					if (!isSolidOrEmpty)
						pNode->SetVoxelShape(pNode->GetVoxelShape() | (1 << side));
					else
						pNode->SetVoxelShape(pNode->GetVoxelShape() & (~(1 << side)));

					for (int k = 0; k < 8; ++k)
					{
						if ((sideMask & (1 << k)) && pNode->IsBlockAt(k))
						{
							if (pNode->IsChildAt(k))
							{
								VoxelOctreeNode* pChildNode = GetChildNode(pNode, k);
								updateChildShape(pChildNode);
							}
							else
							{
								if (!isSolidOrEmpty)
									pNode->childVoxelShape[k] |= (1 << side);
								else
									pNode->childVoxelShape[k] &= (~(1 << side));
							}
						}
					}
				}
				else
					pNode->SetVoxelShape(0);
			};
			updateChildShape(pNode);
		}
	}
}

void ParaEngine::ParaVoxelModel::UpdateNodeShapeByNeighbour(int32 x, int32 y, int32 z, int level, int side, bool isBlock, bool IsSideSplited)
{
	if (x >= 0 && x < level && y >= 0 && y < level && z >= 0 && z < level)
	{
		int nDepth = LevelToDepth(level);
		VoxelOctreeNode* pNode = GetRootNode();

		nDepth--;

		int nChildIndex = 0;
		int nLevel = 1;
		for (; nDepth >= 0; nDepth--, nLevel++)
		{
			uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
			nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
			auto pChildNode = GetChildNode(pNode, nChildIndex);
			if (pChildNode)
				pNode = pChildNode;
			else
			{
				if (nDepth == 0)
				{
					if (pNode->IsBlockAt(nChildIndex))
					{
						if (!isBlock)
							pNode->childVoxelShape[nChildIndex] |= (1 << side);
						else
						{
							if (IsSideSplited)
							{
								pChildNode = CreateGetChildNode(pNode, nChildIndex);
								pNode = pChildNode;
								continue;
							}
							else
							{
								pNode->childVoxelShape[nChildIndex] &= (~(1 << side));
							}
						}
					}
					else
						pNode->childVoxelShape[nChildIndex] = 0;
				}
				else
				{
					if (!isBlock)
					{
						if (pNode->IsBlockAt(nChildIndex))
						{
							// the current block is solid at a higher level of detail than neighbour, while the neighbour block is not solid
							// split the current block until we are at the same level of detail. 
							pChildNode = CreateGetChildNode(pNode, nChildIndex);
							pNode = pChildNode;
							continue;
						}
					}
					else
					{
						if (IsSideSplited)
						{
							if (pNode->IsBlockAt(nChildIndex))
							{
								pChildNode = CreateGetChildNode(pNode, nChildIndex);
								pNode = pChildNode;
								continue;
							}
						}
					}
				}
				return;
			}
		}
		if (nDepth <= 0 && pNode)
		{
			if (pNode->IsBlock())
			{
				if (!isBlock)
					pNode->SetVoxelShape(pNode->GetVoxelShape() | (1 << side));
				else
					pNode->SetVoxelShape(pNode->GetVoxelShape() & (~(1 << side)));
			}
			else
				pNode->SetVoxelShape(0);
		}
	}
}

void ParaEngine::ParaVoxelModel::MergeNodeAndParents(int32 x, int32 y, int32 z, int level)
{
	int nDepth = LevelToDepth(level);
	VoxelOctreeNode* pNode = GetRootNode();
	TempVoxelOctreeNodeRef parentNodes[MAX_VOXEL_DEPTH];
	parentNodes[0] = TempVoxelOctreeNodeRef(pNode, 0, 0, 0, 0);
	nDepth--;
	int nChildIndex = 0;
	int nLevel = 1;
	for (; nDepth >= 0; nDepth--, nLevel++)
	{
		uint32 lx = (x >> nDepth) & 1, ly = (y >> nDepth) & 1, lz = (z >> nDepth) & 1;
		nChildIndex = lx + (ly << 1) + (lz << 2);
		auto pChild = GetChildNode(pNode, nChildIndex);
		if (pChild) {
			pNode = pChild;
			auto& lastNode = parentNodes[nLevel - 1];
			parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, (lastNode.x << 1) + lx, (lastNode.y << 1) + ly, (lastNode.z << 1) + lz, nLevel, nChildIndex);
		}
		else
		{
			break;
		}
	}
	// merge this node and its parents until we reach a node that is not fully solid or empty
	for (int i = nLevel - 1; i >= 0; --i)
	{
		auto pNode = parentNodes[i].pNode;
		if (pNode->IsFullySolid() && pNode->IsLeaf())
		{
			// merge this fully solid node
			auto parentShape = pNode->GetVoxelShape();

			// if child and parent share the same shape, we can merge them
			if ((pNode->childVoxelShape[0] == (parentShape & 0x15)) &&
				(pNode->childVoxelShape[1] == (parentShape & 0x16)) &&
				(pNode->childVoxelShape[2] == (parentShape & 0x19)) &&
				(pNode->childVoxelShape[3] == (parentShape & 0x1a)) &&
				(pNode->childVoxelShape[4] == (parentShape & 0x25)) &&
				(pNode->childVoxelShape[5] == (parentShape & 0x26)) &&
				(pNode->childVoxelShape[6] == (parentShape & 0x29)) &&
				(pNode->childVoxelShape[7] == (parentShape & 0x2a))) 
			{
				auto pChild = pNode;
				pNode = parentNodes[i - 1].pNode;
				uint8_t k = parentNodes[i].childIndex;
				auto childShape = pChild->GetVoxelShape();
				RemoveNodeChildren(pNode, 1 << k);
				pNode->isBlockMask |= (1 << k);
				pNode->childVoxelShape[k] = childShape;
			}
			else
			{
				break;
			}
		}
		else if (pNode->IsEmpty() && pNode->IsLeaf() &&  i >= 1)
		{
			// always merge empty node
			RemoveNodeChildren(parentNodes[i - 1].pNode, 1 << (parentNodes[i].childIndex));
		}
		else
		{
			break;
		}
	}
}

void ParaEngine::ParaVoxelModel::MergeNodeAndNeighbours(int32 x, int32 y, int32 z, int level)
{
	MergeNodeAndParents(x, y, z, level);
	// TODO: merge or split three neighbouring nodes and their child nodes as well. 
}

bool ParaEngine::ParaVoxelModel::IsBlock(int32 x, int32 y, int32 z, int level)
{
	if (x >= 0 && x < level && y >= 0 && y < level && z >= 0 && z < level)
	{
		int nDepth = LevelToDepth(level);
		VoxelOctreeNode* pNode = GetRootNode();

		nDepth--;

		int nChildIndex = 0;
		int nLevel = 1;
		for (; nDepth >= 0; nDepth--, nLevel++)
		{
			uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
			nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
			auto pChildNode = GetChildNode(pNode, nChildIndex);
			if (pChildNode)
				pNode = pChildNode;
			else
			{
				return pNode->IsBlockAt(nChildIndex);
			}
		}
		return pNode->IsBlock();
	}
	else
		return false;
}

bool ParaVoxelModel::RayPicking(const Vector3& origin, const Vector3& dir, Vector3& hitPos, int& hitColor, int level)
{
	return false;
}

void ParaEngine::ParaVoxelModel::SetMinVoxelPixelSize(float fMinVoxelPixelSize)
{
	m_fMinVoxelPixelSize = fMinVoxelPixelSize;
}

float ParaEngine::ParaVoxelModel::GetMinVoxelPixelSize()
{
	return m_fMinVoxelPixelSize;
}

void ParaVoxelModel::Optimize()
{

}

void ParaEngine::ParaVoxelModel::OptimizeNode(VoxelOctreeNode* pNode)
{
	if (pNode->IsLeaf())
		return;

	int solidCount = 0;
	uint8_t isBlockMask = 0;
	for (int k = 0; k < 8; ++k)
	{
		auto pChild = GetChildNode(pNode, k);
		if (pChild)
		{
			OptimizeNode(pChild);
			if (pChild->IsBlock())
				isBlockMask |= (1 << k);
		}
	}
	pNode->isBlockMask = isBlockMask;
}


void ParaEngine::ParaVoxelModel::UpdateNodeParentsSolidityAndColor(TempVoxelOctreeNodeRef nodes[], int nNodeCount)
{
	int32_t fullySolidBlockColor = -1;
	for (int i = nNodeCount - 1; i >= 0; --i)
	{
		auto pNode = nodes[i].pNode;
		if (pNode->IsSolid() && pNode->IsLeaf()) {
			fullySolidBlockColor = pNode->GetColor();
		}
		else
		{
			uint8_t isBlockMask = 0;
			uint16_t color[3] = { 0,0,0 };
			int nChildCount = 0;
			int blockCount = 0;
			bool isChildFullySolid = true;
			for (int k = 0; k < 8; ++k)
			{
				auto pChild = GetChildNode(pNode, k);
				if (pChild)
				{
					color[0] += pChild->GetColor0();
					color[1] += pChild->GetColor1();
					color[2] += pChild->GetColor2();
					nChildCount++;
					if (pChild->IsBlock())
						isBlockMask |= (1 << k);
					isChildFullySolid = isChildFullySolid && pChild->IsFullySolid();
				}
				else if (pNode->IsBlockAt(k))
				{
					blockCount++;
					isBlockMask |= (1 << k);
				}
			}
			pNode->isBlockMask = isBlockMask;
			pNode->SetFullySolid(isChildFullySolid && pNode->IsSolid());

			if (nChildCount > 0) {
				// average on rgb color separately
				if (blockCount == 0)
				{
					// if the node has both child nodes and solid blocks, the color is the color of the non-child blocks.
					// if the node has only child nodes and no blocks, the color is the average of color of the its child blocks.
					pNode->SetColor0((uint8)(color[0] / nChildCount));
					pNode->SetColor1((uint8)(color[1] / nChildCount));
					pNode->SetColor2((uint8)(color[2] / nChildCount));
				}
			}
			if (!pNode->IsLeaf())
				fullySolidBlockColor = -1;
		}
	}
}

// not used
void ParaEngine::ParaVoxelModel::UpdateNode(TempVoxelOctreeNodeRef nodes[], int nNodeCount)
{
	/*
	int32_t fullySolidBlockColor = -1;
	for (int i = nNodeCount - 1; i >= 0; --i)
	{
		auto pNode = nodes[i].pNode;
		if (pNode->IsSolid() && pNode->IsLeaf()) {
			fullySolidBlockColor = pNode->GetColor();
		}
		else
		{
			uint8_t isBlockMask = 0;
			uint16_t color[3] = { 0,0,0 };
			int nChildCount = 0;
			int blockCount = 0;
			bool isChildFullySolid = true;
			for (int k = 0; k < 8; ++k)
			{
				auto pChild = GetChildNode(pNode, k);
				if (pChild)
				{
					color[0] += pChild->GetColor0();
					color[1] += pChild->GetColor1();
					color[2] += pChild->GetColor2();
					nChildCount++;
					if (pChild->IsBlock())
						isBlockMask |= (1 << k);
					isChildFullySolid = isChildFullySolid && pChild->IsFullySolid();
				}
				else if (pNode->IsBlockAt(k))
				{
					blockCount++;
					isBlockMask |= (1 << k);
				}
			}
			pNode->isBlockMask = isBlockMask;
			pNode->SetFullySolid(isChildFullySolid && pNode->IsSolid());

			if (nChildCount > 0) {
				// average on rgb color separately
				if (blockCount == 0)
				{
					// if the node has both child nodes and solid blocks, the color is the color of the non-child blocks.
					// if the node has only child nodes and no blocks, the color is the average of color of the its child blocks.
					pNode->SetColor0((uint8)(color[0] / nChildCount));
					pNode->SetColor1((uint8)(color[1] / nChildCount));
					pNode->SetColor2((uint8)(color[2] / nChildCount));
				}
				auto thisColor = pNode->GetColor();
				for (int k = 0; k < 8; ++k)
				{
					auto pChild = GetChildNode(pNode, k);
					if (pChild && pChild->IsLeaf() && pChild->IsSolid() && thisColor == pChild->GetColor())
					{
						auto childShape = pChild->GetVoxelShape();
						RemoveNodeChildren(pNode, 1 << k);
						pNode->isBlockMask |= (1 << k);
						pNode->childVoxelShape[k] = childShape;
					}
				}
			}
			if (!pNode->IsLeaf())
				fullySolidBlockColor = -1;
		}
	}
	*/
}

inline uint8_t ParaEngine::ParaVoxelModel::GetOppositeSide(uint8_t nSide)
{
	return s_oppositeSides[nSide];
}

void ParaEngine::ParaVoxelModel::UpdateNodeShapeAtLevel(VoxelOctreeNode* pNode, uint32 x, uint32 y, uint32 z, int level)
{
	bool isBlock = pNode->IsBlock();
	// update this block's shape
	uint8_t shape = 0;

	for (int k = 0; k < 6; ++k)
	{
		auto nx = x + s_sideOffset_x[k];
		auto ny = y + s_sideOffset_y[k];
		auto nz = z + s_sideOffset_z[k];
		bool isNeighbourBlock = UpdateNodeShapeByNeighbourAtLevel(nx, ny, nz, level, s_oppositeSides[k], isBlock);
		shape |= (isNeighbourBlock ? 0 : (1 << k));
	}
	if(isBlock)
		pNode->SetVoxelShape(shape);
}

void ParaEngine::ParaVoxelModel::UpdateNodeShape(uint32 x, uint32 y, uint32 z, int level)
{
	bool isBlock = IsBlock(x, y, z, level);
	// update this block's shape 
	uint8_t shape = 0;
	int nDepth = LevelToDepth(level);
	VoxelOctreeNode* pNode = GetRootNode();
	TempVoxelOctreeNodeRef parentNodes[MAX_VOXEL_DEPTH];
	parentNodes[0] = TempVoxelOctreeNodeRef(pNode, 0, 0, 0, 0);
	nDepth --;

	if (isBlock)
	{
		for (int k = 0; k < 6; ++k)
		{
			auto nx = x + s_sideOffset_x[k];
			auto ny = y + s_sideOffset_y[k];
			auto nz = z + s_sideOffset_z[k];
			shape |= (IsBlock(nx, ny, nz, level) ? 0 : (1 << k));
		}
	}

	int nChildIndex = 0;
	int nLevel = 1;
	for (; nDepth >= 0; nDepth--, nLevel++)
	{
		uint32 lx = (x >> nDepth) & 1, ly = (y >> nDepth) & 1, lz = (z >> nDepth) & 1;
		nChildIndex = lx + (ly << 1) + (lz << 2);
		auto pChildNode = GetChildNode(pNode, nChildIndex);
		if (pChildNode) {
			pNode = pChildNode;
			auto& lastNode = parentNodes[nLevel - 1];
			parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, (lastNode.x << 1) + lx, (lastNode.y << 1) + ly, (lastNode.z << 1) + lz, nLevel, nChildIndex);
		}
		else
		{
			if (nDepth == 0)
			{
				pNode->childVoxelShape[nChildIndex] = shape;
			}
			pNode = NULL;
			break;
		}
	}
	if (nDepth <= 0 && pNode) {
		pNode->SetVoxelShape(shape);
		if (pNode->IsLeaf() && pNode->IsSolid())
		{
			// assume all child nodes' shape is same as parent node
			pNode->childVoxelShape[0] = (shape & 0x15);
			pNode->childVoxelShape[1] = (shape & 0x16);
			pNode->childVoxelShape[2] = (shape & 0x19);
			pNode->childVoxelShape[3] = (shape & 0x1a);
			pNode->childVoxelShape[4] = (shape & 0x25);
			pNode->childVoxelShape[5] = (shape & 0x26);
			pNode->childVoxelShape[6] = (shape & 0x29);
			pNode->childVoxelShape[7] = (shape & 0x2a);
		}
	}
	// update six neighbour nodess shape and all of their child nodes
	for (int k = 0; k < 6; ++k)
	{
		auto nx = x + s_sideOffset_x[k];
		auto ny = y + s_sideOffset_y[k];
		auto nz = z + s_sideOffset_z[k];
		UpdateNodeAndChildShapeByNeighbour(nx, ny, nz, level, s_oppositeSides[k], isBlock);
		// bool isSideSplited = pNode != 0 ? pNode->IsSideSplited(k) : false;
		// UpdateNodeShapeByNeighbour(nx, ny, nz, level, s_oppositeSides[k], isBlock, isSideSplited);
	}

	// update all parent nodes' shape and their neighbour nodes at the same level
	for (int i = nLevel - 1; i >= 0; --i)
	{
		auto & parent = parentNodes[i];
		UpdateNodeShapeAtLevel(parent.pNode, parent.x, parent.y, parent.z, 1<<parent.level);
	}
}

const bmax_vertex cubeVertices[36] = {
	// left face
	{{0,0,1}, {-1,0,0}, 0},
	{{0,1,1}, {-1,0,0}, 0},
	{{0,1,0}, {-1,0,0}, 0},
	{{0,0,1}, {-1,0,0}, 0},
	{{0,1,0}, {-1,0,0}, 0},
	{{0,0,0}, {-1,0,0}, 0},
	// right face
	{{1,0,0}, {1,0,0}, 0},
	{{1,1,0}, {1,0,0}, 0},
	{{1,1,1}, {1,0,0}, 0},
	{{1,0,0}, {1,0,0}, 0},
	{{1,1,1}, {1,0,0}, 0},
	{{1,0,1}, {1,0,0}, 0},
	// bottom face
	{{0,0,1}, {0,-1,0}, 0},
	{{0,0,0}, {0,-1,0}, 0},
	{{1,0,0}, {0,-1,0}, 0},
	{{0,0,1}, {0,-1,0}, 0},
	{{1,0,0}, {0,-1,0}, 0},
	{{1,0,1}, {0,-1,0}, 0},
	// top face
	{{0,1,0}, {0,1,0}, 0},
	{{0,1,1}, {0,1,0}, 0},
	{{1,1,1}, {0,1,0}, 0},
	{{0,1,0}, {0,1,0}, 0},
	{{1,1,1}, {0,1,0}, 0},
	{{1,1,0}, {0,1,0}, 0},
	// front face
	{{0,0,0}, {0,0,-1}, 0},
	{{0,1,0}, {0,0,-1}, 0},
	{{1,1,0}, {0,0,-1}, 0},
	{{0,0,0}, {0,0,-1}, 0},
	{{1,1,0}, {0,0,-1}, 0},
	{{1,0,0}, {0,0,-1}, 0},
	// back face
	{{0,0,1}, {0,0,1}, 0},
	{{1,0,1}, {0,0,1}, 0},
	{{1,1,1}, {0,0,1}, 0},
	{{0,0,1}, {0,0,1}, 0},
	{{1,1,1}, {0,0,1}, 0},
	{{0,1,1}, {0,0,1}, 0},
};


int ParaEngine::ParaVoxelModel::GetLodDepth(float fCameraObjectDist, float fScaling)
{
	float fScreenWidth = (float)CGlobals::GetViewportManager()->GetWidth();
	int lod = (int)std::log2f(fScreenWidth / m_fMinVoxelPixelSize / ((abs(fCameraObjectDist)+0.001f) / fScaling));
	if (lod <= 1)
		lod = 1;
	else if (lod > 10)
		lod = 10;
	return lod;
}

void ParaVoxelModel::Draw(SceneState* pSceneState)
{
	CBaseObject* pBaseObj = pSceneState->GetCurrentSceneObject();
	if (pBaseObj != NULL) pBaseObj->ApplyMaterial();

	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	int indexCount = 0;

	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_NORM_DIF);
	pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, sizeof(bmax_vertex));

	static std::vector<bmax_vertex> vertices;
	// how many vertices per draw call at most. 
#define VoxelBatchSize 1024
	vertices.resize(VoxelBatchSize + 36);

	auto drawBatched = [&]() {
		if (indexCount == 0)
			return;
		int nNumLockedVertice;
		int nNumFinishedVertice = 0;
		bmax_vertex* vb_vertices = NULL;
		do
		{
			if ((nNumLockedVertice = pBufEntity->Lock((indexCount - nNumFinishedVertice),
				(void**)(&vb_vertices))) > 0)
			{
				int nLockedNum = nNumLockedVertice / 3;
				memcpy(vb_vertices, &vertices[nNumFinishedVertice], sizeof(bmax_vertex) * nNumLockedVertice);
				pBufEntity->Unlock();

				if (pBufEntity->IsMemoryBuffer())
					pd3dDevice->DrawPrimitiveUP(EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
				else
					pd3dDevice->DrawPrimitive(EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

				if ((indexCount - nNumFinishedVertice) > nNumLockedVertice)
				{
					nNumFinishedVertice += nNumLockedVertice;
				}
				else
					break;
			}
			else
				break;
		} while (1);
		indexCount = 0;
	};

	Matrix4 mat = CGlobals::GetWorldMatrixStack().SafeGetTop();
	Vector3 scaleX(mat.m[0][0], mat.m[0][1], mat.m[0][2]);
	float fScaling = scaleX.length();
	int nMaxDrawDepth = GetLodDepth(pSceneState->GetCameraToCurObjectDistance(), fScaling);

	auto drawVoxelShape = [&indexCount, &drawBatched](uint8_t shape, float x, float y, float z, float size, uint32_t color) {
		DWORD dwColor = color | 0xff000000;
		Vector3 origin(x, y, z);
		for (int k = 0; k < 6; ++k)
		{
			if (shape & (1 << k))
			{
				int srcIndex = k * 6;
				for (int i = 0; i < 6; ++i)
				{
					auto& v = cubeVertices[srcIndex + i];
					vertices[indexCount].p = v.p * size + origin;
					vertices[indexCount].n = v.n;
					vertices[indexCount].color = dwColor;
					indexCount++;
				}
			}
		}
		if (indexCount >= VoxelBatchSize)
			drawBatched();
	};

	std::function<void(VoxelOctreeNode*, float, float, float, float, int)> drawNode;
	drawNode = [&drawNode, &nMaxDrawDepth, &drawVoxelShape, this](VoxelOctreeNode* pNode, float x, float y, float z, float size, int nDepth)
	{
		if (nDepth >= nMaxDrawDepth)
		{
			if (pNode->GetVoxelShape() != 0)
				drawVoxelShape(pNode->GetVoxelShape(), x, y, z, size, pNode->GetColor32());
		}
		else
		{
			if (pNode->IsLeaf() && pNode->IsSolid() && pNode->GetVoxelShape() != 0)
			{
				drawVoxelShape(pNode->GetVoxelShape(), x, y, z, size, pNode->GetColor32());
				return;
			}

			size = size / 2.f;
			auto color = pNode->GetColor32();
			if (pNode->IsChildAt(0))
				drawNode(GetChildNode(pNode, 0), x, y, z, size, nDepth + 1);
			else if (pNode->childVoxelShape[0] != 0)
				drawVoxelShape(pNode->childVoxelShape[0], x, y, z, size, color);
			if (pNode->IsChildAt(1))
				drawNode(GetChildNode(pNode, 1), x + size, y, z, size, nDepth + 1);
			else if (pNode->childVoxelShape[1] != 0)
				drawVoxelShape(pNode->childVoxelShape[1], x + size, y, z, size, color);
			if (pNode->IsChildAt(2))
				drawNode(GetChildNode(pNode, 2), x, y + size, z, size, nDepth + 1);
			else if (pNode->childVoxelShape[2] != 0)
				drawVoxelShape(pNode->childVoxelShape[2], x, y + size, z, size, color);
			if (pNode->IsChildAt(3))
				drawNode(GetChildNode(pNode, 3), x + size, y + size, z, size, nDepth + 1);
			else if (pNode->childVoxelShape[3] != 0)
				drawVoxelShape(pNode->childVoxelShape[3], x + size, y + size, z, size, color);
			if (pNode->IsChildAt(4))
				drawNode(GetChildNode(pNode, 4), x, y, z + size, size, nDepth + 1);
			else if (pNode->childVoxelShape[4] != 0)
				drawVoxelShape(pNode->childVoxelShape[4], x, y, z + size, size, color);
			if (pNode->IsChildAt(5))
				drawNode(GetChildNode(pNode, 5), x + size, y, z + size, size, nDepth + 1);
			else if (pNode->childVoxelShape[5] != 0)
				drawVoxelShape(pNode->childVoxelShape[5], x + size, y, z + size, size, color);
			if (pNode->IsChildAt(6))
				drawNode(GetChildNode(pNode, 6), x, y + size, z + size, size, nDepth + 1);
			else if (pNode->childVoxelShape[6] != 0)
				drawVoxelShape(pNode->childVoxelShape[6], x, y + size, z + size, size, color);
			if (pNode->IsChildAt(7))
				drawNode(GetChildNode(pNode, 7), x + size, y + size, z + size, size, nDepth + 1);
			else if (pNode->childVoxelShape[7] != 0)
				drawVoxelShape(pNode->childVoxelShape[7], x + size, y + size, z + size, size, color);
		}
	};
	drawNode(GetRootNode(), -0.5f, 0, -0.5f, 1.f, 0);
	drawBatched();
}
