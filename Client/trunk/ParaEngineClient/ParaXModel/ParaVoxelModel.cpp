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
#include "BaseCamera.h"
#include "DynamicAttributeField.h"
#include "ParaXModel.h"
#include "effect_file.h"
#include "StringHelper.h"
#include "ParaFile.h"
#include "ViewportManager.h"
#include "ParaVoxelModel.h"

// max octree depth is 12, which is 4096*4096*4096
#define MAX_VOXEL_DEPTH 12

const uint8_t s_oppositeSides[] = { 1, 0, 3, 2, 5, 4 };
const int32_t s_sideOffset_x[] = { -1, 1, 0, 0, 0, 0 };
const int32_t s_sideOffset_y[] = { 0, 0, -1, 1, 0, 0 };
const int32_t s_sideOffset_z[] = { 0, 0, 0, 0, -1, 1 };

const int32_t s_childOffset_x[] = { 0, 1, 0, 1, 0, 1, 0, 1 };
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

inline bool ParaEngine::VoxelOctreeNode::IsChildSameShapeAsParent()
{
	if (IsLeaf())
	{
		auto parentShape = GetVoxelShape();
		return (childVoxelShape[0] == (parentShape & 0x15)) &&
			(childVoxelShape[1] == (parentShape & 0x16)) &&
			(childVoxelShape[2] == (parentShape & 0x19)) &&
			(childVoxelShape[3] == (parentShape & 0x1a)) &&
			(childVoxelShape[4] == (parentShape & 0x25)) &&
			(childVoxelShape[5] == (parentShape & 0x26)) &&
			(childVoxelShape[6] == (parentShape & 0x29)) &&
			(childVoxelShape[7] == (parentShape & 0x2a));
	}
	else
		return false;
}

ParaVoxelModel::ParaVoxelModel()
	: m_fMinVoxelPixelSize(4.f), m_nFirstFreeChunkIndex(0)
{
	VoxelChunk* chunk = new VoxelChunk(1);
	m_chunks.push_back(chunk);
	m_chunks[0]->CreateNode(&VoxelOctreeNode::EmptyNode);
	m_bIsEditable = false;
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
	pClass->AddField("LoadFromFile", FieldType_String, (void*)LoadFromFile_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("SaveToFile", FieldType_String, (void*)SaveToFile_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("SetBlock", FieldType_String, (void*)SetBlock_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("PaintBlock", FieldType_String, (void*)PaintBlock_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("DumpOctree", FieldType_void, (void*)DumpOctree_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("MinVoxelPixelSize", FieldType_Float, (void*)SetMinVoxelPixelSize_s, (void*)GetMinVoxelPixelSize_s, NULL, NULL, bOverride);
	pClass->AddField("Editable", FieldType_Bool, (void*)SetEditable_s, (void*)IsEditable_s, NULL, NULL, bOverride);
	pClass->AddField("RunCommandList", FieldType_String, (void*)RunCommandList_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("run", FieldType_String, (void*)RunCommandList_s, NULL, NULL, NULL, bOverride);
	return S_OK;
}

bool ParaVoxelModel::Load(const char* pBuffer, int nCount)
{
	// read chunk count
	int count = ((uint32_t*)pBuffer)[0];
	pBuffer += 4;
	int nOldSize = (int)m_chunks.size();
	for (int i = 0; i < count; ++i)
	{
		// for each chunk, read chunk size and chunk data
		uint8_t nSize = ((uint8_t*)pBuffer)[0];
		pBuffer += 1;
		if (nOldSize <= i)
			m_chunks.push_back(new VoxelChunk(nSize));
		m_chunks[i]->LoadFromBuffer(pBuffer, nSize);
		pBuffer += nSize * sizeof(VoxelOctreeNode);
	}
	for (int i = count; i < (int)nOldSize; ++i)
	{
		delete m_chunks[i];
	}
	m_chunks.resize(count);
	m_nFirstFreeChunkIndex = 0;
	m_bIsEditable = false;
	return true;
}

bool ParaEngine::ParaVoxelModel::LoadFromFile(const char* filename)
{
	CParaFile file(filename);
	if (!file.isEof())
	{
		//'v'<<24 + 'o'<<16 + 'x'<<8 + '0' + 1;
		uint32_t fileTypeId = file.ReadDWORD();
		if (fileTypeId == 0x766f7830 + 1)
		{
			return Load(file.getBuffer() + 4, file.getSize() - 4);
		}
	}
	return false;
}

bool ParaVoxelModel::Save(std::vector<char>& output)
{
	int nTotalSize = 0;
	int count = (int)m_chunks.size();
	for (int i = 0; i < count; ++i)
	{
		auto& chunk = *(m_chunks[i]);
		nTotalSize += chunk.GetDiskSize() * sizeof(VoxelOctreeNode);
	}
	output.resize(nTotalSize + 4 + count);
	// write chunk count
	char* pData = &output[0];
	((uint32_t*)pData)[0] = count;
	pData += 4;

	for (int i = 0; i < count; ++i)
	{
		// for each chunk, write chunk size and chunk data
		auto& chunk = *(m_chunks[i]);
		uint8_t nSize = chunk.GetDiskSize();
		((uint8_t*)pData)[0] = nSize;
		pData += 1;
		if (nSize > 0)
		{
			memcpy(pData, (char*)(&chunk[0]), nSize * sizeof(VoxelOctreeNode));
			pData += nSize * sizeof(VoxelOctreeNode);
		}
	}
	return true;
}

bool ParaEngine::ParaVoxelModel::SaveToFile(const char* filename)
{
	CParaFile file;
	if (file.CreateNewFile(filename, true))
	{
		std::vector<char> output;
		if (Save(output))
		{
			//'v'<<24 + 'o'<<16 + 'x'<<8 + '0' + 1;
			uint32_t fileTypeId = 0x766f7830 + 1;
			file.WriteDWORD(fileTypeId);
			file.write(&output[0], (int)output.size());
			return true;
		}
	}
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
	int nCount = (int)m_chunks.size();
	int nMinSize = MAX_VOXEL_CHUNK_SIZE - nMinFreeSize - 1;
	for (int i = 0; i < nCount; ++i)
	{
		int nIndex = (m_nFirstFreeChunkIndex + i) % nCount;
		if ((int)m_chunks[nIndex]->GetUsedSize() <= nMinSize)
		{
			m_nFirstFreeChunkIndex = nIndex;
			return nIndex;
		}
	}
	VoxelChunk* chunk = new VoxelChunk();
	m_chunks.push_back(chunk);
	m_nFirstFreeChunkIndex = (int)m_chunks.size() - 1;
	return m_nFirstFreeChunkIndex;
}

VoxelOctreeNode* ParaEngine::ParaVoxelModel::CreateGetChildNode(VoxelOctreeNode* pNode, int nChildIndex)
{
	const uint8_t nChildOffset = pNode->childOffsets[nChildIndex];
	if (!pNode->IsChildAt(nChildIndex))
	{
		const uint8_t nChildShape = nChildOffset;
		// create a new child node
		auto nLastChunkIndex = pNode->GetBaseChunkOffset();
		auto& chunk = *(m_chunks[nLastChunkIndex]);
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
					DeleteNode(nLastChunkIndex, pNode->childOffsets[i]);
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
		pNode->SetSingleShape(false);

		if (!pNode->IsBlockAt(nChildIndex)) {
			pChild->MakeEmpty();
			pChild->SetVoxelShape(0);
		}
		else
		{
			pChild->MakeFullBlock();
			pChild->SetVoxelShape(nChildShape);
			if (nChildShape != 0)
			{
				// assume all child nodes' shape is same as parent node
				pChild->childVoxelShape[0] = nChildShape & 0x15;
				pChild->childVoxelShape[1] = nChildShape & 0x16;
				pChild->childVoxelShape[2] = nChildShape & 0x19;
				pChild->childVoxelShape[3] = nChildShape & 0x1a;
				pChild->childVoxelShape[4] = nChildShape & 0x25;
				pChild->childVoxelShape[5] = nChildShape & 0x26;
				pChild->childVoxelShape[6] = nChildShape & 0x29;
				pChild->childVoxelShape[7] = nChildShape & 0x2a;
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

void ParaEngine::ParaVoxelModel::DeleteNode(uint32_t chunkIndex, uint8_t nodeIndex)
{
	auto chunk = m_chunks[chunkIndex];
	chunk->erase(nodeIndex);
	if (chunkIndex < m_nFirstFreeChunkIndex)
	{
		m_nFirstFreeChunkIndex = chunkIndex;
	}
}

void ParaEngine::ParaVoxelModel::RemoveNodeChildren(VoxelOctreeNode* pNode, uint8_t isBlockMask)
{
	for (int k = 0; k < 8; ++k)
	{
		if ((isBlockMask & (1 << k)) && pNode->IsChildAt(k))
		{
			auto pChild = GetChildNode(pNode, k);
			RemoveNodeChildren(pChild, 0xff);
			DeleteNode(pNode->GetBaseChunkOffset(), pNode->childOffsets[k]);
			pNode->RemoveChild(k);
		}
	}
	pNode->isBlockMask &= (~isBlockMask);
	pNode->SetSingleShape(false);
}

void ParaEngine::ParaVoxelModel::DumpOctree()
{
	OUTPUT_LOG("dumping ParaVoxelModel %d chunks:\n-----------------------\n", (int)m_chunks.size());
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

	if (pNode->IsChildSameShapeAsParent())
		return;
	for (int i = 0; i < nDepth; i++)
		tmp[i] = '-';
	tmp[nDepth] = '\0';

	int nLevel = (int)pow(2, nDepth + 1);
	for (int k = 0; k < 8; ++k)
	{
		OUTPUT_LOG("%s", tmp);
		auto nx = x + s_childOffset_x[k];
		auto ny = y + s_childOffset_y[k];
		auto nz = z + s_childOffset_z[k];

		if (pNode->IsChildAt(k))
		{
			OUTPUT_LOG("%d: (%d,%d,%d)/%d ", k, nx, ny, nz, nLevel);
			DumpOctreeNode(GetChildNode(pNode, k), nDepth + 1, pNode->GetBaseChunkOffset(), pNode->childOffsets[k], nx * 2, ny * 2, nz * 2);
		}
		else
		{
			OUTPUT_LOG("%d: (%d,%d,%d)/%d #%x %s\n", k, nx, ny, nz, nLevel, pNode->childVoxelShape[k], pNode->IsBlockAt(k) ? "solid" : "");
		}
	}
}

void ParaVoxelModel::SetBlock(uint32 x, uint32 y, uint32 z, int level, int color)
{
	SetEditable(true);
	int nDepth = LevelToDepth(level);
	// create get octree node
	VoxelOctreeNode* pNode = GetRootNode();

	nDepth--;
	TempVoxelOctreeNodeRef parentNodes[MAX_VOXEL_DEPTH];
	parentNodes[0] = TempVoxelOctreeNodeRef(pNode, 0, 0, 0, 0);

	int nChildIndex = 0;
	int nLevel = 1;

	if (color >= 0)
	{
		for (; nDepth >= 0; nDepth--, nLevel++)
		{
			uint32 lx = (x >> nDepth) & 1, ly = (y >> nDepth) & 1, lz = (z >> nDepth) & 1;
			nChildIndex = lx + (ly << 1) + (lz << 2);
			pNode = CreateGetChildNode(pNode, nChildIndex);
			auto& lastNode = parentNodes[nLevel - 1];
			parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, (lastNode.x << 1) + lx, (lastNode.y << 1) + ly, (lastNode.z << 1) + lz, nLevel, nChildIndex);
		}

		// create or set block
		RemoveNodeChildren(pNode, 0xff);
		pNode->SetColor32((uint32_t)color);
		pNode->MakeFullBlock();
		if (level > 1)
		{
			UpdateNodeParentsSolidityAndColor(parentNodes, nLevel);
			UpdateNodeShape(x, y, z, level);
			SplitSolidNode(&parentNodes[nLevel - 1]);
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
		for (; nDepth >= 0; nDepth--, nLevel++)
		{
			uint32 lx = (x >> nDepth) & 1, ly = (y >> nDepth) & 1, lz = (z >> nDepth) & 1;
			nChildIndex = lx + (ly << 1) + (lz << 2);
			if (pNode->IsBlockAt(nChildIndex))
			{
				if (nDepth > 0) {
					pNode = CreateGetChildNode(pNode, nChildIndex);
					auto& lastNode = parentNodes[nLevel - 1];
					parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, (lastNode.x << 1) + lx, (lastNode.y << 1) + ly, (lastNode.z << 1) + lz, nLevel, nChildIndex);
				}
				else // if(nDepth == 0)
				{
					// delete child and update node
					RemoveNodeChildren(pNode, 1 << nChildIndex);
					UpdateNodeParentsSolidityAndColor(parentNodes, nLevel);
					UpdateNodeShape(x, y, z, level);
					MergeNodeAndNeighbours(x, y, z, level);
					return;
				}
			}
			else
			{
				// early exit, since it is already empty
				return;
			}
		}

		assert(nLevel == 1);
		// for root node
		RemoveNodeChildren(pNode, 0xff);
		pNode->MakeEmpty();
		pNode->SetVoxelShape(0);
		pNode->offsetAndShape = 0;
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

bool ParaEngine::ParaVoxelModel::SetNodeAndChildColor(VoxelOctreeNode* pNode, uint32 color)
{
	pNode->SetColor32(color);
	bool isFullySolid = true;
	if (!pNode->IsLeaf())
	{
		for (int k = 0; k < 8; ++k)
		{
			if (pNode->IsChildAt(k))
			{
				auto pChild = GetChildNode(pNode, k);
				// merge same color nodes
				if (SetNodeAndChildColor(pChild, color) && pChild->IsChildSameShapeAsParent()) {
					auto childShape = pChild->GetVoxelShape();
					RemoveNodeChildren(pNode, 1 << k);
					pNode->isBlockMask |= (1 << k);
					pNode->childVoxelShape[k] = childShape;
				}
				else
					isFullySolid = false;
			}
		}
	}
	if (isFullySolid && pNode->IsSolid())
	{
		if (pNode->IsLeaf() && pNode->IsChildSameShapeAsParent())
		{
			pNode->SetSingleShape(true);
		}
		return true;
	}
	return false;
}

void ParaEngine::ParaVoxelModel::PaintBlock(uint32 x, uint32 y, uint32 z, int level, uint32_t color)
{
	SetEditable(true);
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
					pNode->SetColor32((uint32_t)color);

					auto& lastNode = parentNodes[nLevel - 1];
					parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, (lastNode.x << 1) + lx, (lastNode.y << 1) + ly, (lastNode.z << 1) + lz, nLevel, nChildIndex);
					continue;
				}
			}
			// early exit if the node is empty, we have nothing to paint on. 
			break;
		}
	}
	if (pNode && nDepth <= 0)
	{
		if (SetNodeAndChildColor(pNode, color))
			UpdateNodeParentsSolidityAndColor(parentNodes, nLevel);
		else
			UpdateNodeParentsColor(parentNodes, nLevel);
		// node merge is done in SetNodeAndChildColor, so there is no need to double udpate here. 
		// MergeNodeAndNeighbours(x, y, z, level);
	}
}

void ParaEngine::ParaVoxelModel::PaintBlockFastMode(uint32 x, uint32 y, uint32 z, int level, uint32_t color)
{
	int nDepth = LevelToDepth(level);
	VoxelOctreeNode* pNode = GetRootNode();
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
			pNode->SetColor32((uint32_t)color);
		}
		else
		{
			if (nDepth == 0)
			{
				if (pNode->IsBlockAt(nChildIndex) && pNode->GetColor32() != color)
				{
					SetEditable(true);
					pNode = CreateGetChildNode(pNode, nChildIndex);
					pNode->SetColor32((uint32_t)color);
				}
			}
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
		if (color >= 0)
			PaintBlock(x, y, z, level, color);
		else
			SetBlock(x, y, z, level, color);
	}
}

void ParaEngine::ParaVoxelModel::RunCommandList(const char* cmd)
{
	static std::string curCmd;
	static int32 curLevel = 1;
	static int32 curColor = 0;
	static int32 offsetX = 0, offsetY = 0, offsetZ = 0;
	auto parseNextCmd = [&]() {
		const char* pos = cmd;
		while (*pos != '\0' && StringHelper::isalphaLowerCase(*pos))
			pos++;
		curCmd.assign(cmd, pos - cmd);
		if (*pos != '\0')
			pos++;
		cmd = pos;
	};
	// "1", "-1", "#ff0000" are all valid values. ' ', ':', ';' are all valid separaters.
	auto parseInteger = [&]() {
		int n = 0;
		const char* pos = cmd;
		bool isPositive = true;
		if (!StringHelper::isdigit(*pos))
		{
			if (*pos == '-') {
				pos++;
				isPositive = false;
			}
			else if (*pos == '#') {
				pos++;
				// parse value in hex
				char c;
				while ((c = *pos) != '\0' && ((c >= 'a' && c <= 'f') || ((c >= '0') && (c <= '9')))) {
					int hex = c - 'a';
					n = n << 4;
					n += (hex >= 0) ? (hex + 10) : (c - '0');
					pos++;
				}
				if (c != '\0' && c != '#')
					pos++;
				cmd = pos;
				return n;
			}
		}
		char c;
		while ((c = *pos) != '\0' && StringHelper::isdigit(c)) {
			n = n * 10 + (c - '0');
			pos++;
		}
		if (c != '\0' && c != '#')
			pos++;
		cmd = pos;
		return isPositive ? n : -n;
	};
	while (*cmd != '\0')
	{
		if (StringHelper::isalphaLowerCase(*cmd))
			parseNextCmd();
		if (curCmd == "setblock") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				uint32 x = parseInteger();
				uint32 y = parseInteger();
				uint32 z = parseInteger();
				int level = parseInteger();
				int color = parseInteger();
				SetBlock(x, y, z, level, color);
			}
		}
		else if (curCmd == "paintblock") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				uint32 x = parseInteger();
				uint32 y = parseInteger();
				uint32 z = parseInteger();
				int level = parseInteger();
				int color = parseInteger();
				PaintBlock(x, y, z, level, color);
			}
		}
		else if (curCmd == "level") {
			curLevel = parseInteger();
		}
		else if (curCmd == "color") {
			curColor = parseInteger();
		}
		else if (curCmd == "offset") {
			offsetX = parseInteger();
			offsetY = parseInteger();
			offsetZ = parseInteger();
		}
		else if (curCmd == "set") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				int32 x = parseInteger();
				int32 y = parseInteger();
				int32 z = parseInteger();
				SetBlock(x, y, z, curLevel, curColor);
			}
		}
		else if (curCmd == "setxyzcolor") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				int32 x = parseInteger();
				int32 y = parseInteger();
				int32 z = parseInteger();
				int color = parseInteger();
				SetBlock(x, y, z, curLevel, color);
			}
		}
		else if (curCmd == "setwithoffset") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				int32 x = parseInteger() + offsetX;
				int32 y = parseInteger() + offsetY;
				int32 z = parseInteger() + offsetZ;
				if (x >= 0 && x < curLevel && y >= 0 && y < curLevel && z >= 0 && z < curLevel)
					SetBlock(x, y, z, curLevel, curColor);
			}
		}
		else if (curCmd == "setrect") {
			uint32 fromX = parseInteger();
			uint32 fromY = parseInteger();
			uint32 fromZ = parseInteger();
			uint32 toX = parseInteger();
			uint32 toY = parseInteger();
			uint32 toZ = parseInteger();

			// make sure the level is large enough to cover the rect
			uint32 level = curLevel;
			uint32 maxSize = max(max(max(fromX, toX), max(fromY, toY)), max(fromZ, toZ)) + 1;
			if (level < maxSize) {
				level = 1 << LevelToDepth(maxSize);
				if (level < maxSize)
					level <<= 1;
			}

			// scan from x to z to y; this is usually the case for how image data is saved. (assume image is on xy, zy, xz plane)
			for (uint32 x = fromX; x <= toX; x++) {
				for (uint32 y = fromY; y <= toY; y++) {
					for (uint32 z = fromZ; z <= toZ; z++)
					{
						SetBlock(x, y, z, level, curColor);
					}
				}
			}
		}
		else if (curCmd == "del") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				int32 x = parseInteger();
				int32 y = parseInteger();
				int32 z = parseInteger();
				SetBlock(x, y, z, curLevel, -1);
			}
		}
		else if (curCmd == "delwithoffset") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				int32 x = parseInteger() + offsetX;
				int32 y = parseInteger() + offsetY;
				int32 z = parseInteger() + offsetZ;
				if (x >= 0 && x < curLevel && y >= 0 && y < curLevel && z >= 0 && z < curLevel)
					SetBlock(x, y, z, curLevel, -1);
			}
		}
		else if (curCmd == "paint") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				int32 x = parseInteger();
				int32 y = parseInteger();
				int32 z = parseInteger();
				PaintBlock(x, y, z, curLevel, curColor);
			}
		}
		else if (curCmd == "paintwithoffset") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				int32 x = parseInteger() + offsetX;
				int32 y = parseInteger() + offsetY;
				int32 z = parseInteger() + offsetZ;
				if (x >= 0 && x < curLevel && y >= 0 && y < curLevel && z >= 0 && z < curLevel)
					PaintBlock(x, y, z, curLevel, curColor);
			}
		}
		else if (curCmd == "paintxyzcolor") {
			while (*cmd != '\0' && !StringHelper::isalphaLowerCase(*cmd))
			{
				int32 x = parseInteger();
				int32 y = parseInteger();
				int32 z = parseInteger();
				int color = parseInteger();
				PaintBlock(x, y, z, curLevel, color);
			}
		}
		else if (curCmd == "paintrect") {
			uint32 fromX = parseInteger();
			uint32 fromY = parseInteger();
			uint32 fromZ = parseInteger();
			uint32 toX = parseInteger();
			uint32 toY = parseInteger();
			uint32 toZ = parseInteger();

			// make sure the level is large enough to cover the rect
			uint32 level = curLevel;
			uint32 maxSize = max(max(max(fromX, toX), max(fromY, toY)), max(fromZ, toZ)) + 1;
			if (level < maxSize) {
				level = 1 << LevelToDepth(maxSize);
				if (level < maxSize)
					level <<= 1;
			}

			if (*cmd == 'd') {
				std::string dataFormat;
				// "data:image/png;base64,"
				dataFormat.assign(cmd, 22);
				cmd += 22;
				if (dataFormat == "data:image/png;base64,")
					dataFormat = "temp.png";
				else if (dataFormat == "data:image/jpg;base64,")
					dataFormat = "temp.jpg";
				else
					dataFormat = "";
				if (!dataFormat.empty())
				{
					std::string buffer = StringHelper::unbase64(cmd, -1);
					int texWidth, texHeight, nBytesPerPixel;
					byte* imageData = NULL;
					if (TextureEntity::LoadImageOfFormat(dataFormat, (char*)(buffer.c_str()), (int)buffer.size(), texWidth, texHeight, &imageData, &nBytesPerPixel))
					{
						if (nBytesPerPixel == 4)
						{
							DWORD* pData = (DWORD*)imageData;
							uint32 x = fromX, y = fromY, z = fromZ;
							while (true) {
								z = fromZ;
								while (true) {
									x = fromX;
									while (true) {
										uint32 color = (*pData) & 0xffffff;
										pData++;
										PaintBlockFastMode(x, y, z, level, color);
										// PaintBlock(x, y, z, level, color);
										if (x == toX)
											break;
										x += (fromX < toX) ? 1 : -1;
									}
									if (z == toZ)
										break;
									z += (fromZ < toZ) ? 1 : -1;
								}
								if (y == toY)
									break;
								y += (fromY < toY) ? 1 : -1;
							}
						}
						SAFE_DELETE_ARRAY(imageData);
					}
				}
			}
			else
			{
				// scan from x to z to y; this is usually the case for how image data is saved. (assume image is on xy, zy, xz plane)
				uint32 x = fromX, y = fromY, z = fromZ;
				while (true) {
					z = fromZ;
					while (true) {
						x = fromX;
						while (true) {
							uint32 color = parseInteger();
							PaintBlock(x, y, z, level, color);
							if (x == toX)
								break;
							x += (fromX < toX) ? 1 : -1;
						}
						if (z == toZ)
							break;
						z += (fromZ < toZ) ? 1 : -1;
					}
					if (y == toY)
						break;
					y += (fromY < toY) ? 1 : -1;
				}
			}
		}
		else {
			break;
		}
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
			// if child and parent share the same shape and color, we can merge them
			if (pNode->IsChildSameShapeAsParent())
			{
				pNode->SetSingleShape(true);
				if (i >= 1 && pNode->GetColor() == parentNodes[i - 1].pNode->GetColor())
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
					break;
			}
			else
				break;
		}
		else if (pNode->IsEmpty() && pNode->IsLeaf() && i >= 1)
		{
			// always merge empty node
			pNode = parentNodes[i - 1].pNode;
			uint8_t k = parentNodes[i].childIndex;
			RemoveNodeChildren(pNode, 1 << k);
			pNode->childVoxelShape[k] = 0;
		}
		else
		{
			break;
		}
	}
}

void ParaEngine::ParaVoxelModel::MergeNodeAndNeighbours(int32 x, int32 y, int32 z, int level)
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

	// merge this node and parent
	MergeNodeAndParents(x, y, z, level);

	// merge three neighbouring nodes and their parent nodes as well. 
	for (int side = 0; side < 6; ++side)
	{
		uint8_t sideMask = SideSolidMask[side];
		if (sideMask & (1 << nChildIndex))
		{
			int xx = x + s_sideOffset_x[side];
			int yy = y + s_sideOffset_y[side];
			int zz = z + s_sideOffset_z[side];
			MergeNodeAndParents(xx, yy, zz, level);
		}
	}
}

bool ParaEngine::ParaVoxelModel::SplitSolidNode(TempVoxelOctreeNodeRef* node)
{
	if (node->pNode->IsSolid() && node->pNode->IsLeaf())
	{
		// check to split this and child nodes 
		int level = 1 << node->level;
		for (int side = 0; side < 6; ++side)
		{
			int x = node->x + s_sideOffset_x[side];
			int y = node->y + s_sideOffset_y[side];
			int z = node->z + s_sideOffset_z[side];
			if (x >= 0 && x < level && y >= 0 && y < level && z >= 0 && z < level)
			{
				int nDepth = node->level;
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
					else {
						pNode = NULL;
						break;
					}
				}
				if (nDepth <= 0 && pNode)
				{
					// all child nodes that are adjacent to the side
					uint8_t sideMask = SideSolidMask[s_oppositeSides[side]];

					// create a recursive function to update all child nodes
					std::function<bool(VoxelOctreeNode*, int, int, int, int)> splitSolidNode_;
					splitSolidNode_ = [&sideMask, &side, &splitSolidNode_, this](VoxelOctreeNode* pNode, int x, int y, int z, int level) {
						if (pNode->IsBlock())
						{
							int blockCountOnSide = 0;
							bool isSplited = false;
							for (int k = 0; k < 8; ++k)
							{
								if ((sideMask & (1 << k)))
								{
									if (pNode->IsChildAt(k))
									{
										int xx = s_childOffset_x[k] + (x << 1);
										int yy = s_childOffset_y[k] + (y << 1);
										int zz = s_childOffset_z[k] + (z << 1);

										VoxelOctreeNode* pChildNode = GetChildNode(pNode, k);
										isSplited = splitSolidNode_(pChildNode, xx, yy, zz, level << 1) || isSplited;
									}
									if (pNode->IsBlockAt(k))
										blockCountOnSide++;
								}
							}
							if (!isSplited && blockCountOnSide != 0 && blockCountOnSide != 4)
							{
								// split node to this level
								uint8_t nSide = s_oppositeSides[side];
								x += s_sideOffset_x[nSide];
								y += s_sideOffset_y[nSide];
								z += s_sideOffset_z[nSide];

								int dx = s_sideOffset_x[side];
								int dy = s_sideOffset_y[side];
								int dz = s_sideOffset_z[side];

								pNode = GetRootNode();
								TempVoxelOctreeNodeRef parentNodes[MAX_VOXEL_DEPTH];
								parentNodes[0] = TempVoxelOctreeNodeRef(pNode, 0, 0, 0, 0);
								int nLevel = 1;
								int nDepth = LevelToDepth(level);
								nDepth--;
								for (; nDepth >= 0; nDepth--, nLevel++)
								{
									uint32 lx = (x >> nDepth) & 1, ly = (y >> nDepth) & 1, lz = (z >> nDepth) & 1;
									int nChildIndex = lx + (ly << 1) + (lz << 2);
									auto& lastNode = parentNodes[nLevel - 1];
									int xx = (lastNode.x << 1) + lx;
									int yy = (lastNode.y << 1) + ly;
									int zz = (lastNode.z << 1) + lz;
									auto pChild = GetChildNode(pNode, nChildIndex);
									if (!pChild)
									{
										isSplited = true;
										pChild = CreateGetChildNode(pNode, nChildIndex);
										uint8_t mask = SideSolidMask[side];
										// update the side shape of the newly created node's child nodes
										for (int k = 0; k < 8; ++k)
										{
											if ((mask & (1 << k)))
											{
												if (!IsBlock((xx << 1) + s_childOffset_x[k] + dx, (yy << 1) + s_childOffset_y[k] + dy, (zz << 1) + s_childOffset_z[k] + dz, 1 << (nLevel + 1)))
													pChild->childVoxelShape[k] |= (1 << side);
												else
													pChild->childVoxelShape[k] &= (~(1 << side));
											}
										}
										// also update the parent node on the same side
										if (nLevel >= 2)
										{
											auto parentNode = lastNode.pNode;
											for (int k = 0; k < 8; ++k)
											{
												if ((mask & (1 << k)) && !parentNode->IsChildAt(k))
												{
													if (!IsBlock((xx & 0xfffffffe) + s_childOffset_x[k] + dx, (yy & 0xfffffffe) + s_childOffset_y[k] + dy, (zz & 0xfffffffe) + s_childOffset_z[k] + dz, 1 << nLevel))
														parentNode->childVoxelShape[k] |= (1 << side);
													else
														parentNode->childVoxelShape[k] &= (~(1 << side));
												}
											}
										}
									}
									pNode = pChild;
									parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, xx, yy, zz, nLevel, nChildIndex);
								}
								return isSplited;
							}
						}
						return false;
					};
					splitSolidNode_(pNode, x, y, z, level);
				}
			}
		}

		// check to split neighouring nodes
		for (int side = 0; side < 6; ++side)
		{
			// split three neighour node and their parents which are adjacent to the side of this node
			uint8_t sideMask = SideSolidMask[side];
			if (sideMask & (1 << node->childIndex))
			{
				bool isSplited = false;
				uint8_t nSide = s_oppositeSides[side];

				int x = node->x + s_sideOffset_x[side];
				int y = node->y + s_sideOffset_y[side];
				int z = node->z + s_sideOffset_z[side];

				int dx = s_sideOffset_x[nSide];
				int dy = s_sideOffset_y[nSide];
				int dz = s_sideOffset_z[nSide];

				VoxelOctreeNode* pNode = GetRootNode();
				TempVoxelOctreeNodeRef parentNodes[MAX_VOXEL_DEPTH];
				parentNodes[0] = TempVoxelOctreeNodeRef(pNode, 0, 0, 0, 0);
				int nLevel = 1;
				int nDepth = LevelToDepth(level);
				nDepth--;
				for (; nDepth >= 1; nDepth--, nLevel++)
				{
					uint32 lx = (x >> nDepth) & 1, ly = (y >> nDepth) & 1, lz = (z >> nDepth) & 1;
					int nChildIndex = lx + (ly << 1) + (lz << 2);
					auto& lastNode = parentNodes[nLevel - 1];
					int xx = (lastNode.x << 1) + lx;
					int yy = (lastNode.y << 1) + ly;
					int zz = (lastNode.z << 1) + lz;
					if (!pNode->IsBlockAt(nChildIndex))
						break;
					auto pChild = GetChildNode(pNode, nChildIndex);
					if (!pChild)
					{
						isSplited = true;
						pChild = CreateGetChildNode(pNode, nChildIndex);
						uint8_t mask = SideSolidMask[nSide];
						// update the side shape of the newly created node's child nodes
						for (int k = 0; k < 8; ++k)
						{
							if ((mask & (1 << k)))
							{
								if (!IsBlock((xx << 1) + s_childOffset_x[k] + dx, (yy << 1) + s_childOffset_y[k] + dy, (zz << 1) + s_childOffset_z[k] + dz, 1 << (nLevel + 1)))
									pChild->childVoxelShape[k] |= (1 << nSide);
								else
									pChild->childVoxelShape[k] &= (~(1 << nSide));
							}
						}
						// also update the parent node on the same side
						if (nLevel >= 2)
						{
							auto parentNode = lastNode.pNode;
							for (int k = 0; k < 8; ++k)
							{
								if ((mask & (1 << k)) && parentNode->IsBlockAt(k) && !parentNode->IsChildAt(k))
								{
									if (!IsBlock((xx & 0xfffffffe) + s_childOffset_x[k] + dx, (yy & 0xfffffffe) + s_childOffset_y[k] + dy, (zz & 0xfffffffe) + s_childOffset_z[k] + dz, 1 << nLevel))
										parentNode->childVoxelShape[k] |= (1 << nSide);
									else
										parentNode->childVoxelShape[k] &= (~(1 << nSide));
								}
							}
						}
					}
					pNode = pChild;
					parentNodes[nLevel] = TempVoxelOctreeNodeRef(pNode, xx, yy, zz, nLevel, nChildIndex);
				}
			}
		}
	}
	return false;
}

bool ParaEngine::ParaVoxelModel::HasHolesOnSide(int32 x, int32 y, int32 z, int level, int side)
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
				// no child nodes, we have finished and return
				return false;
			}
		}
		if (nDepth <= 0 && pNode)
		{
			// update all child nodes that are adjacent to the side
			uint8_t sideMask = SideSolidMask[side];

			// create a recursive function to update all child nodes
			std::function<bool(VoxelOctreeNode*)> hasHolesOnSide_;
			hasHolesOnSide_ = [&sideMask, &side, &hasHolesOnSide_, this](VoxelOctreeNode* pNode) {
				if (pNode->IsBlock())
				{
					int blockCountOnSide = 0;
					for (int k = 0; k < 8; ++k)
					{
						if ((sideMask & (1 << k)))
						{
							if (pNode->IsChildAt(k))
							{
								VoxelOctreeNode* pChildNode = GetChildNode(pNode, k);
								if (hasHolesOnSide_(pChildNode))
									return true;
							}
							if (pNode->IsBlockAt(k))
								blockCountOnSide++;
						}
					}
					return blockCountOnSide != 0 && blockCountOnSide != 4;
				}
				return false;
			};
			return hasHolesOnSide_(pNode);
		}
	}
	return false;
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

void ParaEngine::ParaVoxelModel::SetEditable(bool bEditable)
{
	if (m_bIsEditable != bEditable)
	{
		m_bIsEditable = bEditable;
		if (m_bIsEditable)
		{
			// resize all chunks to full size
			for (auto& chunk : m_chunks)
			{
				chunk->MakeEditable();
			}
		}
	}
}

bool ParaEngine::ParaVoxelModel::IsEditable()
{
	return m_bIsEditable;
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
					if (pChild->IsBlock())
					{
						color[0] += pChild->GetColor0();
						color[1] += pChild->GetColor1();
						color[2] += pChild->GetColor2();
						nChildCount++;
						isBlockMask |= (1 << k);
					}
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

void ParaEngine::ParaVoxelModel::UpdateNodeParentsColor(TempVoxelOctreeNodeRef nodes[], int nNodeCount)
{
	for (int i = nNodeCount - 1; i >= 0; --i)
	{
		auto pNode = nodes[i].pNode;
		if (!pNode->IsLeaf() && !(pNode->HasNonChildNodeBlock()))
		{
			uint16_t color[3] = { 0,0,0 };
			int nChildCount = 0;
			for (int k = 0; k < 8; ++k)
			{
				auto pChild = GetChildNode(pNode, k);
				if (pChild)
				{
					if (pChild->IsBlock())
					{
						color[0] += pChild->GetColor0();
						color[1] += pChild->GetColor1();
						color[2] += pChild->GetColor2();
						nChildCount++;
					}
				}
			}
			if (nChildCount > 0) {
				// if the node has both child nodes and solid blocks, the color is the color of the non-child blocks.
				// if the node has only child nodes and no blocks, the color is the average of color of the its child blocks.
				pNode->SetColor0((uint8)(color[0] / nChildCount));
				pNode->SetColor1((uint8)(color[1] / nChildCount));
				pNode->SetColor2((uint8)(color[2] / nChildCount));
			}
		}
	}
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
	if (isBlock)
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
		auto& parent = parentNodes[i];
		UpdateNodeShapeAtLevel(parent.pNode, parent.x, parent.y, parent.z, 1 << parent.level);
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
	int lod = (int)std::log2f(fScreenWidth / m_fMinVoxelPixelSize / ((abs(fCameraObjectDist) + 0.001f) / fScaling));
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

	// calculate the max lod depth according to the distance from camera to the object
	Matrix4 mat = CGlobals::GetWorldMatrixStack().SafeGetTop();
	Vector3 scaleX(mat.m[0][0], mat.m[0][1], mat.m[0][2]);
	float fScaling = scaleX.length();
	int nMaxDrawDepth = GetLodDepth(pSceneState->GetCameraToCurObjectDistance(), fScaling);

	// if there are many chunks, we will test compute the three sides are facing towards the camera
	// and we will disable back facing surface rendering for all nodes in the octree.
	uint8_t exShapeMask = 0x3f;
	if (m_chunks.size() > 1 && nMaxDrawDepth > 2)
	{
		// world view matrix. 	
		mat *= CGlobals::GetViewMatrixStack().SafeGetTop();
		Vector3 vNormalX(-1, 0, 0);
		Vector3 vNormalY(0, -1, 0);
		Vector3 vNormalZ(0, 0, -1);
		Matrix4 matRot = mat;
		matRot.RemoveScaling();
		auto quatRot = matRot.extractQuaternion();
		vNormalX = quatRot * vNormalX;
		vNormalY = quatRot * vNormalY;
		vNormalZ = quatRot * vNormalZ;

		// min, max point in camera space
		Vector3 vMin(-0.5, 0, -0.5);
		Vector3 vMax(0.5, 0, 0.5);
		vMin = vMin * mat;
		vMax = vMax * mat;
		bool sides[6];
		// check if the camera(0,0,0) is on the positve side of the 6 planes
		sides[0] = vNormalX.dotProduct(vMin) < 0;
		sides[1] = vNormalX.dotProduct(vMax) > 0;
		sides[2] = vNormalY.dotProduct(vMin) < 0;
		sides[3] = vNormalY.dotProduct(vMax) > 0;
		sides[4] = vNormalZ.dotProduct(vMin) < 0;
		sides[5] = vNormalZ.dotProduct(vMax) > 0;
		exShapeMask = 0;
		if (sides[0] != sides[1])
			exShapeMask |= sides[0] ? 1 : 2;
		else
			exShapeMask |= 0x3;
		if (sides[2] != sides[3])
			exShapeMask |= sides[2] ? 4 : 8;
		else
			exShapeMask |= 0xc;
		if (sides[4] != sides[5])
			exShapeMask |= sides[4] ? 0x10 : 0x20;
		else
			exShapeMask |= 0x30;
		// TODO: for special case like 0x3, 0xc, 0x30, we need to check if the camera is inside the child node of the octree 
		// until parallel sides belongs to the same side of the camera.
		// however, since voxel model is usually small, we can ignore this case for now.
	}

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
					RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
				else
					RenderDevice::DrawPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);

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


	auto drawVoxelShape = [&indexCount, &drawBatched, &exShapeMask](uint8_t shape, float x, float y, float z, float size, uint32_t color) {
		DWORD dwColor = color | 0xff000000;
		Vector3 origin(x, y, z);
		for (int k = 0; k < 6; ++k)
		{
			if (shape & exShapeMask & (1 << k))
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
			auto color = pNode->GetColor32();
			if (pNode->IsSingleShape())
			{
				drawVoxelShape(pNode->GetVoxelShape(), x, y, z, size, color);
				return;
			}
			size = size / 2.f;

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
