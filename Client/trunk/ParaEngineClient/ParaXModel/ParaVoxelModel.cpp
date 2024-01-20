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
	: isBlockMask(isBlockMask), colorRGB{ 0,0,0 }, baseChunkOffset(0), childMask(0xffffffffffffffff)
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

VoxelOctreeNode* ParaEngine::ParaVoxelModel::GetNode(uint32 x, uint32 y, uint32 z, int level)
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

VoxelOctreeNode* ParaEngine::ParaVoxelModel::CreateGetChildNode(VoxelOctreeNode* pNode, int nChildIndex)
{
	int nChildOffset = pNode->childOffsets[nChildIndex];
	if (nChildOffset == 0xff)
	{
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
				if (pNode->childOffsets[i] != 0xff)
				{
					// move child to new chunk
					auto index = newChunk.CreateNode(&chunk[pNode->childOffsets[i]]);
					chunk.erase(pNode->childOffsets[i]);
					pNode->childOffsets[i] = index;
					newChunk[index].SetBaseChunkOffset(baseChunkIndex);
				}
			}
			auto index = newChunk.CreateNode(&VoxelOctreeNode::FullNode);
			pNode->childOffsets[nChildIndex] = index;
			newChunk[index].SetBaseChunkOffset(baseChunkIndex);
			if(!pNode->IsBlockAt(nChildIndex))
				newChunk[index].MakeEmpty();
			
			return &(newChunk[index]);
		}
		else
		{
			// create in current chunk
			auto index = chunk.CreateNode(&VoxelOctreeNode::FullNode);
			pNode->childOffsets[nChildIndex] = index;
			chunk[index].SetBaseChunkOffset(pNode->GetBaseChunkOffset());
			if (!pNode->IsBlockAt(nChildIndex))
				chunk[index].MakeEmpty();
			return &(chunk[index]);
		}
	}
	else
	{
		return &((*m_chunks[pNode->GetBaseChunkOffset()])[nChildOffset]);
	}
}

VoxelOctreeNode* ParaEngine::ParaVoxelModel::GetChildNode(VoxelOctreeNode* pNode, int nChildIndex)
{
	int nChildOffset = pNode->childOffsets[nChildIndex];
	if (nChildOffset == 0xff)
	{
		return NULL;
	}
	else
	{
		return &((*m_chunks[pNode->GetBaseChunkOffset()])[nChildOffset]);
	}
}

void ParaEngine::ParaVoxelModel::RemoveNodeChildren(VoxelOctreeNode* pNode, uint8_t isBlockMask)
{
	auto& chunk = *m_chunks[pNode->GetBaseChunkOffset()];
	for (int k = 0; k < 8; ++k)
	{
		if ((isBlockMask & (1 << k)) && pNode->childOffsets[k] != 0xff)
		{
			auto pChild = GetChildNode(pNode, k);
			RemoveNodeChildren(pChild, 0xff);
			chunk.erase(pNode->childOffsets[k]);
			pNode->childOffsets[k] = 0xff;
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
	StringHelper::fast_sprintf(tmp, "Node[%d][%d]: baseChunkOffset %d, blockMask: %d color: #%06x\n", nChunkIndex, offset, pNode->GetBaseChunkOffset(), pNode->isBlockMask, pNode->GetColor());
	OUTPUT_LOG(tmp);

	if(pNode->IsLeaf())
		return;
	for (int i = 0; i < nDepth; i++)
		tmp[i] = '-';
	tmp[nDepth] = '\0';
	for (int k = 0; k < 8; ++k)
	{
		OUTPUT_LOG("%s", tmp);
		if (pNode->childOffsets[k] != 0xff)
		{
			OUTPUT_LOG("%d: ", k);
			DumpOctreeNode(GetChildNode(pNode, k), nDepth + 1, pNode->GetBaseChunkOffset(), pNode->childOffsets[k]);
		}
		else
		{
			OUTPUT_LOG("%d:\n", k);
		}
	}
}

void ParaVoxelModel::SetBlock(uint32 x, uint32 y, uint32 z, int level, int color)
{
	int nDepth = LevelToDepth(level);
	// create get octree node
	VoxelOctreeNode* pNode = GetRootNode();
	
	nDepth--;
	VoxelOctreeNode* parentNodes[MAX_VOXEL_DEPTH];
	parentNodes[0] = pNode;
	
	int nChildIndex = 0;
	int nLevel = 1;
	for (; nDepth >= 0; nDepth--, nLevel++)
	{
		uint32 lx = x >> nDepth, ly = y >> nDepth, lz = z >> nDepth;
		nChildIndex = (lx & 1) + ((ly & 1) << 1) + ((lz & 1) << 2);
		pNode = CreateGetChildNode(pNode, nChildIndex);
		parentNodes[nLevel] = pNode;
	}
	if (color > 0)
	{
		// create or set block
		RemoveNodeChildren(pNode, 0xff);
		pNode->SetColor((uint32_t)color);
		pNode->MakeFullBlock();
		UpdateNode(parentNodes, nLevel);
	}
	else
	{
		// delete block
		if (nLevel >= 2) {
			RemoveNodeChildren(parentNodes[nLevel - 2], 1 << nChildIndex);
			UpdateNode(parentNodes, nLevel-1);
		}
		else {
			// for root node
			RemoveNodeChildren(pNode, 0xff);
			pNode->MakeEmpty(); 
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
	pNode->SetColor(color);
	bool isFullySolid = true;
	int nChildCount = 0;
	for (int k = 0; k < 8; ++k)
	{
		if (pNode->childOffsets[k] != 0xff)
		{
			auto pChild = GetChildNode(pNode, k);
			// merge same color nodes
			if (SetNodeColor(pChild, color))
				RemoveNodeChildren(pNode, 1<<k);
			else
				isFullySolid = false;
			
			nChildCount++;
		}
	}
	return isFullySolid && pNode->IsSolid();
}

void ParaEngine::ParaVoxelModel::PaintBlock(uint32 x, uint32 y, uint32 z, int level, uint32_t color)
{
	VoxelOctreeNode* pNode = GetNode(x, y, z, level);
	if(pNode)
		SetNodeColor(pNode, color);
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
	VoxelOctreeNode* pNode = GetNode(x, y, z, level);
	return (pNode && !pNode->IsEmpty()) ? pNode->GetColor() : -1;
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

void ParaEngine::ParaVoxelModel::UpdateNode(VoxelOctreeNode* nodes[], int nNodeCount)
{
	int fullySolidBlockColor = -1;
	for (int i = nNodeCount - 1; i >= 0; --i)
	{
		auto pNode = nodes[i];
		if (pNode->IsFullySolid()) {
			fullySolidBlockColor = pNode->GetColor();
			continue;
		}
		uint8_t isBlockMask = 0;
		uint32_t color = 0;
		uint32_t thisColor = pNode->GetColor();
		int nChildCount = 0;
		int blockCount = 0;
		for (int k = 0; k < 8; ++k)
		{
			auto pChild = GetChildNode(pNode, k);
			if (pChild)
			{
				color += pChild->GetColor();
				nChildCount++;
				if (pChild->IsBlock())
					isBlockMask |= (1 << k);
			}
			else if (pNode->IsBlockAt(k))
			{
				blockCount++;
				isBlockMask |= (1 << k);
				color += thisColor;
			}
		}
		pNode->isBlockMask = isBlockMask;
		if (nChildCount > 0) {
			// average on rgb color separately
			blockCount += nChildCount;
			color = (((color>>16) / blockCount) << 16) + (((color>>8) / blockCount) << 8) + ((color / blockCount));
			pNode->SetColor(color);
			if (fullySolidBlockColor == pNode->GetColor() && pNode->IsSolid())
			{
				bool bIsFullySolid = true;
				for (int k = 0; k < 8; ++k)
				{
					auto pChild = GetChildNode(pNode, k);
					if (pChild && !pChild->IsFullySolid())
					{
						bIsFullySolid = false;
						break;
					}
				}
				if (bIsFullySolid)
				{
					RemoveNodeChildren(pNode, 0xff);
					pNode->MakeFullBlock();
				}
				else
					fullySolidBlockColor = -1;
			}
			else
				fullySolidBlockColor = -1;
		}
		else
			fullySolidBlockColor = -1;
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
