//-----------------------------------------------------------------------------
// Class:	ParaVoxelModel
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2024.1.17
// local model = entity:GetInnerObject():GetPrimaryAsset():GetAttributeObject():GetChildAt(0):GetChild("VoxelModel")
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
#include "ParaVoxelModel.h"

// max octree depth is 12, which is 4096*4096*4096
#define MAX_VOXEL_DEPTH 12

using namespace ParaEngine;
VoxelOctreeNode VoxelOctreeNode::EmptyNode(0x0);
VoxelOctreeNode VoxelOctreeNode::FullNode(0xff);

VoxelOctreeNode::VoxelOctreeNode(uint8_t isBlockMask)
	: isBlockMask(isBlockMask), isChildMask(0), colorRGB(0), baseChunkOffset(0), childMask(0)
{
}

ParaVoxelModel::ParaVoxelModel()
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
	int nMinSize = 0xfe - nMinFreeSize;
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
		if(chunk.GetUsedSize() >= 254)
		{
			pNode->SetBaseChunkOffset(CreateGetFreeChunkIndex());
			auto& newChunk = *(m_chunks[pNode->GetBaseChunkOffset()]);
			auto baseChunkIndex = pNode->GetBaseChunkOffset();
			// create a new chunk and move all existing child nodes to the new chunk. 
			for (int i = 0; i < 8; ++i)
			{
				if (pNode->IsChildAt(i))
				{
					// move child to new chunk
					auto index = newChunk.CreateNode(&chunk[pNode->childOffsets[i]]);
					chunk.erase(pNode->childOffsets[i]);
					pNode->SetChild(i, index);
					newChunk[index].SetBaseChunkOffset(baseChunkIndex);
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
		if (!pNode->IsBlockAt(nChildIndex))
			pChild->MakeEmpty();
		else if(nChildShape!= 0)
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
	DumpOctreeNode(pNode, 0, 0, 0);
}

void ParaEngine::ParaVoxelModel::DumpOctreeNode(VoxelOctreeNode* pNode, int nDepth, int nChunkIndex, int offset)
{
	char tmp[256];
	StringHelper::fast_sprintf(tmp, "Node[%d][%d]: baseChunkOffset %d, blockMask: %d opache:%d shapeMask: #%x  color: #%06x childMask: %x\n",
		nChunkIndex, offset, pNode->GetBaseChunkOffset(),
		pNode->isBlockMask, pNode->IsFullySolid() ? 1 : 0,  pNode->GetVoxelShape(), pNode->GetColor(), pNode->isChildMask);
	OUTPUT_LOG(tmp);

	if(pNode->IsLeaf() && (pNode->IsSolid() || pNode->IsEmpty()))
		return;
	for (int i = 0; i < nDepth; i++)
		tmp[i] = '-';
	tmp[nDepth] = '\0';
	for (int k = 0; k < 8; ++k)
	{
		OUTPUT_LOG("%s", tmp);
		if (pNode->IsChildAt(k))
		{
			OUTPUT_LOG("%d: ", k);
			DumpOctreeNode(GetChildNode(pNode, k), nDepth + 1, pNode->GetBaseChunkOffset(), pNode->childOffsets[k]);
		}
		else
		{
			OUTPUT_LOG("%d: #%x\n", k, pNode->childVoxelShape[k]);
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
	parentNodes[0] = TempVoxelOctreeNodeRef(pNode, 0,0,0,1);
	
	int nChildIndex = 0;
	int nLevel = 1;
	for (; nDepth >= 0; nDepth--, nLevel++)
	{
		uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
		nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
		pNode = CreateGetChildNode(pNode, nChildIndex);
		auto& lastNode = parentNodes[nLevel - 1];
		parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, (lastNode.x<<1) + lx, (lastNode.y << 1) + ly, (lastNode.z << 1) + lz, nLevel);
	}
	if (color > 0)
	{
		// create or set block
		RemoveNodeChildren(pNode, 0xff);
		pNode->SetColor32((uint32_t)color);
		pNode->MakeFullBlock();
		UpdateNode(parentNodes, nLevel);
		UpdateNodeShape(x, y, z, level);
	}
	else
	{
		// delete block
		if (nLevel >= 2) {
			RemoveNodeChildren(parentNodes[nLevel - 2].pNode, 1 << nChildIndex);
			UpdateNode(parentNodes, nLevel-1);
			UpdateNodeShape(x, y, z, level);
		}
		else {
			// for root node
			RemoveNodeChildren(pNode, 0xff);
			pNode->MakeEmpty(); 
			pNode->SetVoxelShape(0);
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

	nDepth--;

	int nChildIndex = 0;
	int nLevel = 1;
	for (; nDepth >= 0; nDepth--, nLevel++)
	{
		uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
		nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
		auto pChild = GetChildNode(pNode, nChildIndex);
		if (pChild)
			pNode = pChild;
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
	}
}

void ParaEngine::ParaVoxelModel::PaintBlockCmd(const char* cmd)
{
	uint32 x, y, z;
	int level;
	int color;
	if (sscanf(cmd, "%d,%d,%d,%d,%d", &x, &y, &z, &level, &color) == 5)
	{
		if(color > 0)
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

void ParaEngine::ParaVoxelModel::UpdateNodeShapeByNeighbour(int32 x, int32 y, int32 z, int level, int side, bool isSolid)
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
						if (!isSolid)
							pNode->childVoxelShape[nChildIndex] |= (1 << side);
						else
							pNode->childVoxelShape[nChildIndex] &= (~(1 << side));
					}
					else
						pNode->childVoxelShape[nChildIndex] = 0;
				}
				return;
			}
		}
		if (nDepth <= 0 && pNode)
		{
			if (pNode->IsSolid())
			{
				if (!isSolid)
					pNode->SetVoxelShape(pNode->GetVoxelShape() | (1 << side));
				else
					pNode->SetVoxelShape(pNode->GetVoxelShape() & (~(1 << side)));
			}
			else
				pNode->SetVoxelShape(0);
		}
	}
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
			if(pChild->IsBlock())
				isBlockMask |= (1 << k);
		}
	}
	pNode->isBlockMask = isBlockMask;
}

void ParaEngine::ParaVoxelModel::UpdateNode(TempVoxelOctreeNodeRef nodes[], int nNodeCount)
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
}


const uint8_t s_oppositeSides[] = { 1, 0, 3, 2, 5, 4 };
const int32_t s_sideOffset_x[] = { -1, 1, 0, 0, 0, 0 };
const int32_t s_sideOffset_y[] = { 0, 0, -1, 1, 0, 0 };
const int32_t s_sideOffset_z[] = { 0, 0, 0, 0, -1, 1 };

inline uint8_t ParaEngine::ParaVoxelModel::GetOppositeSide(uint8_t nSide)
{
	return s_oppositeSides[nSide];
}

void ParaEngine::ParaVoxelModel::UpdateNodeShape(uint32 x, uint32 y, uint32 z, int level)
{
	bool isBlock = IsBlock(x, y, z, level);
	{
		// update this block's shape
		uint8_t shape = 0;
		int nDepth = LevelToDepth(level);
		VoxelOctreeNode* pNode = GetRootNode();
		nDepth--;

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
			uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
			nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
			auto pChildNode = GetChildNode(pNode, nChildIndex);
			if (pChildNode)
				pNode = pChildNode;
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
		if (nDepth <= 0 && pNode)
			pNode->SetVoxelShape(shape);
	}
	{
		// update six neighbour block's shape
		for (int k = 0; k < 6; ++k)
		{
			auto nx = x + s_sideOffset_x[k];
			auto ny = y + s_sideOffset_y[k];
			auto nz = z + s_sideOffset_z[k];
			UpdateNodeShapeByNeighbour(nx, ny, nz, level, s_oppositeSides[k], isBlock);
		}
	}

	if (level > 1) {
		UpdateNodeShape(x >> 1, y >> 1, z >> 1, level >> 1);
	}
}

void ParaVoxelModel::Draw(SceneState* pSceneState)
{
	CBaseObject* pBaseObj = pSceneState->GetCurrentSceneObject();
	if (pBaseObj != NULL) pBaseObj->ApplyMaterial();


	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	int start = 0;
	int indexCount = 0;

	bmax_vertex* vb_vertices = NULL;
	bmax_vertex* ov = NULL;
	int nNumLockedVertice;
	int nNumFinishedVertice = 0;
	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_NORM_DIF);
	pd3dDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, sizeof(bmax_vertex));

	do
	{
		indexCount = 3;
		if ((nNumLockedVertice = pBufEntity->Lock((indexCount - nNumFinishedVertice),
			(void**)(&vb_vertices))) > 0)
		{
			int nLockedNum = nNumLockedVertice / 3;

			bmax_vertex origVertices[] = {
				{ Vector3(1, 0.5, 0), Vector3(0, 1, 0), 0xffffffff },
				{ Vector3(0, 0.5, 1), Vector3(0, 1, 0), 0xffffffff },
				{ Vector3(1, 0.5, 1), Vector3(0, 1, 0), 0xffffffff }
			};
			
			for (int i = 0; i < nLockedNum; ++i)
			{
				int nVB = 3 * i;
				for (int k = 0; k < 3; ++k, ++nVB)
				{
					uint16 a = k;
					bmax_vertex& out_vertex = vb_vertices[nVB];
					ov = origVertices + a;
					out_vertex.p = ov->p;
					out_vertex.n = ov->n;
					out_vertex.color = ov->color;
				}
			}
			
			pBufEntity->Unlock();

			if (pBufEntity->IsMemoryBuffer())
				RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, D3DPT_TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
			else
				RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, D3DPT_TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

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
}
