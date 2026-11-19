//-----------------------------------------------------------------------------
// Class:	Chunk is a 16*16*16 inside a region
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2013.11.24
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockRegion.h"
#include "BlockCommon.h"
#include "BlockDirection.h"
#include "BlockWorld.h"
#include "BlockChunk.h"

#define INVALID_BLOCK_INDEX		0xffff
#define GET_METERIAL_FACE_KEY(face, index) ((face)*4096+index)
namespace ParaEngine
{
	int BlockChunk::s_total_chunks = 0;

	BlockChunk::BlockChunk(int32_t nPackedChunkId, BlockRegion* pRegion) :
		m_blockIndices(BlockConfig::g_chunkBlockCount, -1), m_nDirty(1), m_emptyBlockSlotIndex(INVALID_BLOCK_INDEX),
		m_ownerBlockRegion(pRegion), m_packedChunkID(nPackedChunkId), m_isBoundaryChunk(0),
		m_uVisGraph(kVisGraphAllConnected), m_opaqueLayerMask(0), m_bVisGraphDirty(false)
	{
		// m_blocks.reserve(4096);
		SetLightingInitialized(false);
		UnpackChunkIndex(m_packedChunkID,m_chunkId_rs.x,m_chunkId_rs.y,m_chunkId_rs.z);
		
		m_minBlockId_rs.x = m_chunkId_rs.x*16;
		m_minBlockId_rs.y = m_chunkId_rs.y*16;
		m_minBlockId_rs.z = m_chunkId_rs.z*16;

		if (pRegion)
		{
			m_minBlockId_ws.x = pRegion->m_minBlockId_ws.x + m_minBlockId_rs.x;
			m_minBlockId_ws.y = m_minBlockId_rs.y;
			m_minBlockId_ws.z = pRegion->m_minBlockId_ws.z + m_minBlockId_rs.z;
		}
		else
		{
			m_minBlockId_ws = m_minBlockId_rs;
		}

		SetBoundaryChunk(m_chunkId_rs.x == 0 || m_chunkId_rs.y == BlockConfig::MinSectionY || m_chunkId_rs.z == 0
			|| m_chunkId_rs.x == BlockConfig::g_regionChunkDimX - 1
			|| m_chunkId_rs.y == BlockConfig::MaxSectionY
			|| m_chunkId_rs.z == BlockConfig::g_regionChunkDimZ - 1);

		m_lightmapArray.resize(BlockConfig::g_chunkBlockCount,LightData());

		s_total_chunks++;
	}

	BlockChunk::~BlockChunk()
	{
		s_total_chunks--;
	}

	void BlockChunk::AddOccupiedCell()
	{
		if (m_occupiedCells++ == 0 && m_ownerBlockRegion) m_ownerBlockRegion->SetSectionOccupied(m_chunkId_rs, true);
	}

	void BlockChunk::ReserveBlocks( int nCount )
	{
		m_blocks.reserve(nCount);
	}

	void BlockChunk::Reset()
	{
		std::fill(m_blockIndices.begin(),m_blockIndices.end(),-1);
		m_blocks.clear();
		if (m_occupiedCells && m_ownerBlockRegion) m_ownerBlockRegion->SetSectionOccupied(m_chunkId_rs, false);
		m_occupiedCells = 0;
		m_lightBlockIndices.clear();
		SetDirty(false);
		SetLightingInitialized(false);
		m_emptyBlockSlotIndex = INVALID_BLOCK_INDEX;
		m_uVisGraph = kVisGraphAllConnected;
		m_opaqueLayerMask = 0;
		m_bVisGraphDirty = false;
		std::fill(m_lightmapArray.begin(),m_lightmapArray.end(),LightData());

	}

	void BlockChunk::ClearAllLight()
	{
		SetLightingInitialized(false);
		std::fill(m_lightmapArray.begin(), m_lightmapArray.end(), LightData());

	}

	void BlockChunk::ClearLightMap()
	{
		ClearAllLight();
	}

	bool BlockChunk::HasBlockMaterial(Uint16x3& blockId_r)
	{
		uint16_t nBlockIndex = CalcPackedBlockID(blockId_r);
		return (m_materialBlockIndices.find(nBlockIndex) != m_materialBlockIndices.end());
	}

	bool BlockChunk::HasBlockMaterial(uint16 nPackedBlockID)
	{
		return (m_materialBlockIndices.find(nPackedBlockID) != m_materialBlockIndices.end());
	}

	int32 BlockChunk::GetBlockFaceMaterial(uint16 nPackedBlockID, int16 nFaceIndex)
	{
		auto iter = m_materialsKeyIdMap.find(GET_METERIAL_FACE_KEY(nFaceIndex, nPackedBlockID));
		return (iter != m_materialsKeyIdMap.end()) ? iter->second : -1;
	}


	void BlockChunk::ApplyBlockMaterial(uint16 nPackedBlockID, int16 nFaceIndex, int32 nMaterialID)
	{
		Block* pBlock = GetBlock(nPackedBlockID);
		if (pBlock) 
		{
			BlockTemplate* pTemplate = pBlock->GetTemplate();
			if (pTemplate)
			{
				if (pTemplate->IsMatchAttribute(BlockTemplate::batt_liquid)) return ;
				uint16_t x, y, z;
				UnpackBlockIndex(nPackedBlockID, x, y, z);
				x = m_minBlockId_ws.x + x;
				y = m_minBlockId_ws.y + y;
				z = m_minBlockId_ws.z + z;
				BlockModel* pModel = &(pTemplate->GetBlockModel(GetBlockWorld(), x, y, z, (uint16)pBlock->GetUserData(), nullptr));
				if (pModel)
				{
					if (pModel->IsUsingSelfLighting()) return ;
					if (pModel->IsUniformLighting())
					{
						int nFaceCount = pModel->GetFaceCount();
						for (int i = 0; i < nFaceCount; i++)
						{
							auto nMaterialKey = GET_METERIAL_FACE_KEY(i, nPackedBlockID);
							m_materialsKeyIdMap[nMaterialKey] = nMaterialID;
						}
						m_materialBlockIndices[nPackedBlockID] = nFaceCount;
						return;
					}
				}
			}
		}

		auto nMaterialKey = GET_METERIAL_FACE_KEY(nFaceIndex, nPackedBlockID);
		auto iter = m_materialsKeyIdMap.find(nMaterialKey);
		if (iter != m_materialsKeyIdMap.end())
		{
			iter->second = nMaterialID;
		}
		else
		{
			m_materialsKeyIdMap[nMaterialKey] = nMaterialID;
			auto iter = m_materialBlockIndices.find(nPackedBlockID);
			if (iter != m_materialBlockIndices.end())
			{
				iter->second ++;
			}
			else
			{
				m_materialBlockIndices[nPackedBlockID] = 1;
			}
		}
	}

	void BlockChunk::RemoveBlockMaterial(uint16 nPackedBlockID, int16 nFaceIndex)
	{
		Block* pBlock = GetBlock(nPackedBlockID);
		if (pBlock) 
		{
			BlockTemplate* pTemplate = pBlock->GetTemplate();
			if (pTemplate)
			{
				if (pTemplate->IsMatchAttribute(BlockTemplate::batt_liquid)) return ;
				uint16_t x, y, z;
				UnpackBlockIndex(nPackedBlockID, x, y, z);
				x = m_minBlockId_ws.x + x;
				y = m_minBlockId_ws.y + y;
				z = m_minBlockId_ws.z + z;
				BlockModel* pModel = &(pTemplate->GetBlockModel(GetBlockWorld(), x, y, z, (uint16)pBlock->GetUserData(), nullptr));
				if (pModel)
				{
					if (pModel->IsUsingSelfLighting()) return ;
					if (pModel->IsUniformLighting())
					{
						int nFaceCount = pModel->GetFaceCount();
						for (int i = 0; i < nFaceCount; i++)
						{
							auto nMaterialKey = GET_METERIAL_FACE_KEY(i, nPackedBlockID);
							m_materialsKeyIdMap.erase(nMaterialKey);
						}
						m_materialBlockIndices.erase(nPackedBlockID);
						return;
					}
				}
			}
		}

		if (nFaceIndex >= 0)
		{
			m_materialsKeyIdMap.erase(GET_METERIAL_FACE_KEY(nFaceIndex, nPackedBlockID));
			auto iter = m_materialBlockIndices.find(nPackedBlockID);
			if (iter != m_materialBlockIndices.end())
			{
				if (iter->second > 1)
					iter->second = iter->second - 1;
				else
					m_materialBlockIndices.erase(iter);
			}
		}
		else
		{
			for (int16 i = 0; i < 6; ++i)
			{
				m_materialsKeyIdMap.erase(GET_METERIAL_FACE_KEY(i, nPackedBlockID));
			}

			auto iter = m_materialBlockIndices.find(nPackedBlockID);
			if (iter != m_materialBlockIndices.end())
			{
				m_materialBlockIndices.erase(iter);
			}
		}
	}

	void BlockChunk::LoadBlock(uint16_t nBlockIndex, BlockTemplate* pTemplate)
	{
		SetBlockTemplate(nBlockIndex, pTemplate);

		if (pTemplate && pTemplate->IsMatchAttribute(BlockTemplate::batt_light))
		{
			AddLight(nBlockIndex);
		}
		bool isTransparent = pTemplate? pTemplate->IsTransparent() : false;
		UpdateHeightMapAtLoadTime(nBlockIndex, isTransparent);
	}

	int BlockChunk::LoadBlocks(const std::vector<uint16_t>& blockIndices, BlockTemplate* pTemplate)
	{
		int nIndexSize = (int)blockIndices.size();
		if (nIndexSize > 0)
		{
			m_bVisGraphDirty = true;
			uint16 nBlockIndex = blockIndices[0];
			SetBlockTemplate(nBlockIndex, pTemplate);
			Block* pBlock = GetBlock(nBlockIndex);
			uint16 nIndex = m_blockIndices[nBlockIndex];

			bool bIsLightBlock = pTemplate->IsMatchAttribute(BlockTemplate::batt_light);
			bool isTransparent = pTemplate->IsTransparent();
			if (bIsLightBlock)
			{
				AddLight(nBlockIndex);
			}
			UpdateHeightMapAtLoadTime(nBlockIndex, isTransparent);

			for (int i = 1; i < nIndexSize; ++i)
			{
				nBlockIndex = blockIndices[i];
				if (m_blockIndices[nBlockIndex] >=0)
				{
					// untested code: this should only happen when the function is called after load time, which almost never happens. 
					Uint16x3 nPos = GetBlockPosRs(nBlockIndex);
					SetBlockToAir(nPos);
				}
				pBlock->IncreaseInstanceCount();
				AddOccupiedCell();
				m_blockIndices[nBlockIndex] = nIndex;
				
				if (bIsLightBlock)
				{
					AddLight(nBlockIndex);
				}
				UpdateHeightMapAtLoadTime(nBlockIndex, isTransparent);
			}
		}
		
		return nIndexSize;
	}

	void BlockChunk::SetBlockTemplate(uint16_t nBlockIndex, BlockTemplate* pTemplate)
	{
		Block* pBlock = GetBlock(nBlockIndex);
		if (pBlock)
		{
			if (pBlock->GetTemplate() != pTemplate)
			{
				SetBlockEmpty(nBlockIndex, *pBlock);
			}
			else
			{
				return;
			}
		}
		m_bVisGraphDirty = true;
		if (pTemplate)
		{
			int16 nIndex = FindBlock(pTemplate);
			if (nIndex >= 0){
				AddOccupiedCell();
				m_blockIndices[nBlockIndex] = nIndex;
				m_blocks[nIndex].IncreaseInstanceCount();
				return;
			}
			else{
				Block* pBlock = CreateBlock(nBlockIndex);
				if (pBlock)
				{
					pBlock->SetTemplate(pTemplate);
					pBlock->IncreaseInstanceCount();
				}
			}
		}
		else
		{
			m_blockIndices[nBlockIndex] = -1;
		}
	}

	void BlockChunk::SetBlockData(uint16_t nBlockIndex, uint32 nData)
	{
		Block* pBlock = GetBlock(nBlockIndex);
		if (pBlock)
		{
			if (pBlock->GetUserData() != nData)
			{
				m_bVisGraphDirty = true;
				BlockTemplate * pTemplate = pBlock->GetTemplate();
				SetBlockEmpty(nBlockIndex, *pBlock);
				
				if (pTemplate)
				{
					int16 nIndex = FindBlock(pTemplate, nData);
					if (nIndex >= 0){
						AddOccupiedCell();
				m_blockIndices[nBlockIndex] = nIndex;
						m_blocks[nIndex].IncreaseInstanceCount();
						return;
					}
					else{
						Block* pBlock = CreateBlock(nBlockIndex);
						if (pBlock)
						{
							pBlock->SetTemplate(pTemplate);
							pBlock->SetUserData(nData);
							pBlock->IncreaseInstanceCount();
						}
					}
				}
			}
		}
	}

	void BlockChunk::SetBlock(uint16_t nBlockIndex, BlockTemplate* pTemplate, uint32 nData)
	{
		Block* pBlock = GetBlock(nBlockIndex);
		if (pBlock)
		{
			if (pBlock->GetTemplate() != pTemplate || pBlock->GetUserData() != nData)
			{
				SetBlockEmpty(nBlockIndex, *pBlock);
			}
			else
			{
				return;
			}
		}
		m_bVisGraphDirty = true;
		if (pTemplate)
		{
			int16 nIndex = FindBlock(pTemplate, nData);
			if (nIndex >= 0){
				AddOccupiedCell();
				m_blockIndices[nBlockIndex] = nIndex;
				m_blocks[nIndex].IncreaseInstanceCount();
				return;
			}
			else{
				Block* pBlock = CreateBlock(nBlockIndex);
				if (pBlock)
				{
					pBlock->SetTemplate(pTemplate);
					pBlock->SetUserData(nData);
					pBlock->IncreaseInstanceCount();
				}
			}
		}
		else
		{
			m_blockIndices[nBlockIndex] = -1;
		}
	}

	BlockTemplate* BlockChunk::GetBlockTemplate(uint16_t nBlockIndex)
	{
		Block* pBlock = GetBlock(nBlockIndex);
		return (pBlock) ? pBlock->GetTemplate() : NULL;
	}

	int16 BlockChunk::FindBlock(BlockTemplate* pTemplate, uint32 nData)
	{
		int nSize = (int)m_blocks.size();
		for (int i = 0; i < nSize; ++i)
		{
			Block& block = m_blocks[i];
			if (block.GetTemplate() == pTemplate && block.GetUserData() == nData && !block.IsEmptySlot())
				return (int16)i;
		}
		return -1;
	}

	int16 BlockChunk::FindBlock(BlockTemplate* pTemplate)
	{
		int nSize = (int)m_blocks.size();
		for (int i = 0; i < nSize; ++i)
		{
			Block& block = m_blocks[i];
			if (block.GetTemplate() == pTemplate && !block.IsEmptySlot())
				return (int16)i;
		}
		return -1;
	}

	Block* BlockChunk::CreateBlock(uint16_t nIndex)
	{
		AddOccupiedCell();
		Block* pResult = NULL;
		if (m_emptyBlockSlotIndex != INVALID_BLOCK_INDEX)
		{
			int32_t nextEmptyBlockSolt = (int32_t)m_blocks[m_emptyBlockSlotIndex].PopEmptySlot();
			pResult = &m_blocks[m_emptyBlockSlotIndex];
			m_blockIndices[nIndex] = m_emptyBlockSlotIndex;
			m_emptyBlockSlotIndex = nextEmptyBlockSolt;
		}
		else
		{
			m_blocks.push_back(Block());
			pResult = &m_blocks.back();
			m_blockIndices[nIndex] = (int16_t)(m_blocks.size() - 1);
		}
		return pResult;
	}


	bool BlockChunk::RecycleBlock(uint16 nIndex, Block& block)
	{
		if (block.IsEmptySlot())
		{
			block.PushEmptySlotIndex(m_emptyBlockSlotIndex);
			m_emptyBlockSlotIndex = nIndex;
			return true;
		}
		return false;
	}

	bool BlockChunk::SetBlockToAir(Uint16x3& blockId_r)
	{
		uint16_t nIndex = CalcPackedBlockID(blockId_r);
		int32_t blockIndex = m_blockIndices[nIndex];
		if(blockIndex >= 0)
		{
			if (HasBlockMaterial(nIndex))
			{
				RemoveBlockMaterial(nIndex, -1);
			}

			Block& block = m_blocks[blockIndex];

			if(block.GetTemplate()->IsMatchAttribute(BlockTemplate::batt_light))
			{
				RemoveLight(blockId_r);
			}

			SetBlockEmpty(nIndex, block);
			return true;
		}
		return false;
	}

	Block* BlockChunk::GetBlock(uint16_t nIndex)
	{
		int32_t blockIndex = m_blockIndices[nIndex];
		return (blockIndex > -1) ? &m_blocks[blockIndex] : NULL;
	}

	bool BlockChunk::RemoveLight( Uint16x3& blockId_r )
	{
		uint16_t nBlockIndex = CalcPackedBlockID(blockId_r);

		auto iter = m_lightBlockIndices.find(nBlockIndex);
		if(iter!=m_lightBlockIndices.end())
		{
			m_lightBlockIndices.erase(iter);
			return true;
		}
		return false;
	}

	void BlockChunk::AddLight( Uint16x3& blockId_r )
	{
		uint16_t nBlockIndex = CalcPackedBlockID(blockId_r);
		m_lightBlockIndices.insert(nBlockIndex);
	}

	void BlockChunk::AddLight(uint16 nPackedBlockID)
	{
		m_lightBlockIndices.insert(nPackedBlockID);
	}

	bool BlockChunk::IsVisibleBlock( int32_t index,Block* pBlock )
	{
		if(!pBlock)
		{
			pBlock = GetBlock(index);
			if(!pBlock)
			{
				return false;
			}
		}

		auto blockTemplate = pBlock->GetTemplate();
		if (blockTemplate->IsMatchAttribute(BlockTemplate::batt_invisible))
		{
			return false;
		}

		uint16_t blockIdX_cs,blockIdY_cs,blockIdZ_cs;
		UnpackBlockIndex(index,blockIdX_cs,blockIdY_cs,blockIdZ_cs);
		const int16_t maxIndex = BlockConfig::g_chunkBlockDim - 1;

		BlockTemplate* pTemplate = pBlock->GetTemplate();
		if(pTemplate->IsMatchAttribute(BlockTemplate::batt_liquid))
		{
			//hide block if it's surrounded by same kind of liquid
			if(blockIdX_cs==0 || blockIdY_cs==0 || blockIdZ_cs == 0 || maxIndex==blockIdX_cs || maxIndex==blockIdY_cs || maxIndex==blockIdZ_cs)
			{
				//boundary block
				Block* neighborBlocks[6] = {0,0,0,0,0,0};
				Uint16x3 blockId_cs(blockIdX_cs, blockIdY_cs, blockIdZ_cs);
				QueryNeighborBlockData(blockId_cs, neighborBlocks, 1, 6);

				for(int i=0; i<6; ++i)
				{
					Block* pBlock = neighborBlocks[i];
					if(!pBlock || pBlock->GetTemplate() != pTemplate)
						return true;
				}
				return false;
			}
			else
			{
				//hide block if it's surrounded by same kind of liquid
				uint16_t curX = blockIdX_cs - 1;
				uint16_t curY = blockIdY_cs;
				uint16_t curZ = blockIdZ_cs;
				Block* pBlock = NULL;

				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || pBlock->GetTemplate() != pTemplate)
					return true;

				curX = blockIdX_cs + 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || pBlock->GetTemplate() != pTemplate)
					return true;

				curX = blockIdX_cs;
				curY = blockIdY_cs - 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || pBlock->GetTemplate() != pTemplate)
					return true;

				curY = blockIdY_cs + 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || pBlock->GetTemplate() != pTemplate)
					return true;

				curY = blockIdY_cs;
				curZ = blockIdZ_cs - 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || pBlock->GetTemplate() != pTemplate)
					return true;

				curZ = blockIdZ_cs + 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || pBlock->GetTemplate() != pTemplate)
					return true;

				return false;
			}
		}
		else
		{
			if(blockIdX_cs==0 || blockIdY_cs==0 || blockIdZ_cs == 0 || maxIndex==blockIdX_cs || maxIndex==blockIdY_cs || maxIndex==blockIdZ_cs)
			{
				Block* neighborBlocks[6] = {0,0,0,0,0,0};
				Uint16x3 blockId_cs(blockIdX_cs, blockIdY_cs, blockIdZ_cs);
				QueryNeighborBlockData(blockId_cs, neighborBlocks, 1, 6);

				if(!pTemplate->IsMatchAttribute(BlockTemplate::batt_transparent))
				{
					// for non-transparent solid blocks, if all six nearby blocks are solid and non-transparent, we will make it invisible. 
					for(int i=0; i<6; ++i)
					{
						Block* pBlock = neighborBlocks[i];
						if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid) || pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_transparent)))
							return true;
					}
				}
				else
				{
					// for solid-transparent blocks(life tree-leaves), if all six nearby blocks are solid, we will make it invisible. 
					for(int i=0; i<6; ++i)
					{
						Block* pBlock = neighborBlocks[i];
						if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid)))
							return true;
					}
				}
				return false;
			}
			else if(!pTemplate->IsMatchAttribute(BlockTemplate::batt_transparent))
			{
				// for non-transparent solid blocks, if all six nearby blocks are solid and non-transparent, we will make it invisible. 
				uint16_t curX = blockIdX_cs - 1;
				uint16_t curY = blockIdY_cs;
				uint16_t curZ = blockIdZ_cs;

				Block * pBlock;

				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid) || pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_transparent)))
					return true;

				curX = blockIdX_cs + 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid) || pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_transparent)))
					return true;

				curX = blockIdX_cs;
				curY = blockIdY_cs - 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid) || pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_transparent)))
					return true;

				curY = blockIdY_cs + 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid) || pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_transparent)))
					return true;

				curY = blockIdY_cs;
				curZ = blockIdZ_cs - 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid) || pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_transparent)))
					return true;

				curZ = blockIdZ_cs + 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid) || pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_transparent)))
					return true;
			}
			else
			{
				// for solid-transparent blocks(life tree-leaves), if all six nearby blocks are solid, we will make it invisible. 
				uint16_t curX = blockIdX_cs - 1;
				uint16_t curY = blockIdY_cs;
				uint16_t curZ = blockIdZ_cs;
				Block * pBlock;

				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid)))
					return true;

				curX = blockIdX_cs + 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid)))
					return true;

				curX = blockIdX_cs;
				curY = blockIdY_cs - 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid)))
					return true;

				curY = blockIdY_cs + 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid)))
					return true;

				curY = blockIdY_cs;
				curZ = blockIdZ_cs - 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid)))
					return true;

				curZ = blockIdZ_cs + 1;
				pBlock = GetBlock(PackBlockId(curX,curY,curZ));
				if(!pBlock || (!pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_solid)))
					return true;
			}

			//hide block if it's surrounded by soildBlock
			return false;
		}
	}

	void BlockChunk::QueryNeighborBlockData( const Uint16x3& blockId_cs,Block** pBlockData,int nFrom /*= 0*/, int nTo /*= 26*/ )
	{
		const Int16x3* neighborOfsTable = BlockCommon::NeighborOfsTable;

		//best case all neighbor blocks are in same chunk
		if((blockId_cs.x != 0 && (blockId_cs.x != BlockConfig::g_chunkBlockDim - 1)
			&& (blockId_cs.y != 0) && (blockId_cs.y != BlockConfig::g_chunkBlockDim - 1)
			&& (blockId_cs.z != 0) && blockId_cs.z != BlockConfig::g_chunkBlockDim - 1))
		{
			for(int i= nFrom; i<=nTo; ++i)
			{
				Uint16x3 curBlockId;
				curBlockId.x = blockId_cs.x + neighborOfsTable[i].x;
				curBlockId.y = blockId_cs.y + neighborOfsTable[i].y;
				// no need to test again, since blockId_cs is already within the chunk boundary
				// curBlockId.y = curBlockId.y == 0xffff ? 0 : curBlockId.y;
				curBlockId.z = blockId_cs.z + neighborOfsTable[i].z;

				Block* pBlock = GetBlock(PackBlockId(curBlockId.x,curBlockId.y,curBlockId.z));
				pBlockData[i-nFrom] = pBlock;
			}
		}
		else
		{
			//ok all neighbors are still in same region
			if(!IsBoundaryChunk())
			{
				Uint16x3 blockId_rs;
				blockId_rs.x = m_minBlockId_rs.x + blockId_cs.x;
				blockId_rs.y = m_minBlockId_rs.y + blockId_cs.y;
				blockId_rs.z = m_minBlockId_rs.z + blockId_cs.z;

				for(int i= nFrom; i<=nTo; ++i)
				{
					Uint16x3 curBlockId_rs;
					curBlockId_rs.x = blockId_rs.x + neighborOfsTable[i].x;
					curBlockId_rs.y = blockId_rs.y + neighborOfsTable[i].y;
					//curBlockId_rs.y = curBlockId_rs.y == 0xffff ? 0 : curBlockId_rs.y;
					if(!BlockConfig::IsBlockY(curBlockId_rs.y))
					{
						continue;
					}
					curBlockId_rs.z = blockId_rs.z + neighborOfsTable[i].z;

					int32_t packedChunkId_rs = CalcPackedChunkID(curBlockId_rs);
					BlockChunk* pChunk = m_ownerBlockRegion->GetChunk(packedChunkId_rs, false);
					if(pChunk)
					{
						Block* pBlock = pChunk->GetBlock(CalcPackedBlockID(curBlockId_rs.x, curBlockId_rs.y, curBlockId_rs.z));
						pBlockData[i-nFrom] = pBlock;
					}
				}
			}
			else
			{
				//worse case neighbors may cross region
				Uint16x3 blockId_ws;
				blockId_ws.x = m_minBlockId_ws.x + blockId_cs.x;
				blockId_ws.y = m_minBlockId_ws.y + blockId_cs.y;
				blockId_ws.z = m_minBlockId_ws.z + blockId_cs.z;

				CBlockWorld* pWorld = GetBlockWorld();
				for(int i= nFrom; i<=nTo; ++i)
				{
					Uint16x3 curBlockId_ws;
					curBlockId_ws.x = blockId_ws.x + neighborOfsTable[i].x;
					curBlockId_ws.y = blockId_ws.y + neighborOfsTable[i].y;
					//curBlockId_ws.y = curBlockId_ws.y == 0xffff ? 0 : curBlockId_ws.y;
					if(!BlockConfig::IsBlockY(curBlockId_ws.y))
					{
						continue;
					}
					curBlockId_ws.z = blockId_ws.z + neighborOfsTable[i].z;

					pBlockData[i - nFrom] = pWorld->GetBlock(curBlockId_ws.x, curBlockId_ws.y, curBlockId_ws.z);
				}
			}
		}
	}

	LightData* BlockChunk::GetLightData( uint16_t nIndex )
	{
		return &(m_lightmapArray[nIndex]);
	}

	bool BlockChunk::IsInfluenceBySunLight()
	{
		uint32_t nCount = m_lightmapArray.size();
		for(uint32_t i = 0;i<nCount;i++)
		{
			if(m_lightmapArray[i].IsInfluencedBySun())
				return true;
		}
		return false;
	}

	bool BlockChunk::IsNearbyChunksLoaded()
	{
		CBlockWorld* pWorld =  GetBlockWorld();
		return ((m_minBlockId_ws.x < 16 || pWorld->GetChunkColumnTimeStamp(m_minBlockId_ws.x - 16, m_minBlockId_ws.z) > 0) &&
				pWorld->GetChunkColumnTimeStamp(m_minBlockId_ws.x + 16, m_minBlockId_ws.z) > 0 && 
				(m_minBlockId_ws.z < 16 || pWorld->GetChunkColumnTimeStamp(m_minBlockId_ws.x, m_minBlockId_ws.z-16) > 0) &&
				pWorld->GetChunkColumnTimeStamp(m_minBlockId_ws.x, m_minBlockId_ws.z+16) > 0);
	}

	CBlockWorld* BlockChunk::GetBlockWorld()
	{
		return m_ownerBlockRegion->GetBlockWorld();
	}

	uint32 BlockChunk::GetBlockCount()
	{
		return (uint32)m_blocks.size();
	}

	void BlockChunk::UpdateHeightMapAtLoadTime(uint16_t nBlockIndex, bool isTransparent)
	{
		uint16 rx, ry, rz;
		UnpackBlockIndex(nBlockIndex, rx, ry, rz);
		ChunkMaxHeight* pBlockHeight = m_ownerBlockRegion->GetHighestBlock(m_minBlockId_rs.x + rx, m_minBlockId_rs.z + rz);
		int rs_y = m_minBlockId_rs.y + ry;
		if (pBlockHeight)
		{
			pBlockHeight->AddBlockHeight(rs_y, isTransparent);
		}
	}

	Block& BlockChunk::GetBlockByIndex(uint16_t nBlockIndex)
	{
		return m_blocks[nBlockIndex];
	}

	void BlockChunk::SetBlockEmpty(uint16_t nBlockIndex, Block& block)
	{
		int16 nIndex = m_blockIndices[nBlockIndex];
		if (nIndex != -1)
		{
			m_bVisGraphDirty = true;
			if (--m_occupiedCells == 0 && m_ownerBlockRegion) m_ownerBlockRegion->SetSectionOccupied(m_chunkId_rs, false);
			m_blockIndices[nBlockIndex] = -1;
			if (block.DecreaseInstanceCount() == 0)
			{
				RecycleBlock((uint16)nIndex, block);
			}
		}
	}

	Uint16x3 BlockChunk::GetBlockPosRs(uint16 nBlockIndex)
	{
		Uint16x3 blockId_rs;
		UnpackBlockIndex(nBlockIndex, blockId_rs.x, blockId_rs.y, blockId_rs.z);
		return m_minBlockId_rs + blockId_rs;
	}

	std::vector<Uint16x3> BlockChunk::refreshBlockVisible(uint16_t blockTemplateId)
	{
		int nIndexSize = (int)m_blockIndices.size();
		if (nIndexSize > 0)
		{
			for (int blockIndex = 1; blockIndex < nIndexSize; ++blockIndex)
			{
				if (m_blockIndices[blockIndex] >= 0)
				{
					Block& block = m_blocks[m_blockIndices[blockIndex]];
					BlockTemplate* blockTemplate = block.GetTemplate();
					if (blockTemplate && blockTemplate->GetID() == blockTemplateId)
					{
						Uint16x3 blockId_rs = GetBlockPosRs(blockIndex);
						m_ownerBlockRegion->RefreshBlockTemplateByIndex(blockId_rs.x, blockId_rs.y, blockId_rs.z, blockTemplate);
						m_ownerBlockRegion->CheckNeighborChunkDirty(blockId_rs);
						SetDirty(true);
						m_bVisGraphDirty = true;
					}
				}
			}
		}

		std::vector<Uint16x3> rets;
		return rets;
	}

	void BlockChunk::SetDirty(bool val)
	{
		m_nDirty = val ? (m_nDirty==0 ? 1 : (m_nDirty | 0x1)) : 0;
	}


	void BlockChunk::SetLightDirty()
	{
		m_nDirty |= 0x4;
	}

	void BlockChunk::SetDirtyByNeighbor()
	{
		m_nDirty |= 0x2;
	}

	bool BlockChunk::IsDirtyByNeighbor()
	{
		return (m_nDirty & 0x2) && (m_nDirty & 0x1) == 0;
	}

	bool BlockChunk::IsDirtyByLight()
	{
		return (m_nDirty & 0x4) && (m_nDirty & 0x1) == 0;
	}

	bool BlockChunk::IsDirtyByBlockChange()
	{
		return (m_nDirty & 0x1) != 0;
	}

	void BlockChunk::SetLightingInitialized(bool bInitialized)
	{
		m_lightingInitialized = bInitialized;
	}

	bool BlockChunk::IsLightingInitialized()
	{
		return m_lightingInitialized;
	}

	bool BlockChunk::CanBlockSeeTheSkyWS(uint16 x, int16_t y, uint16 z)
	{
		ChunkMaxHeight* pHeight = m_ownerBlockRegion->GetHighestBlock(x & 0x1ff, z & 0x1ff);
		if (pHeight)
			return pHeight->GetMaxSolidHeight() < y;
		else
			return true;
	}

	int BlockChunk::GetTotalChunksInMemory()
	{
		return s_total_chunks;
	}

	int BlockChunk::GetTotalBytes()
	{
		return (sizeof(BlockChunk) + (sizeof(LightData) + sizeof(int16))*(16 * 16 * 16)) + sizeof(Block) * GetBlockCount() + sizeof(uint16) * m_lightBlockIndices.size();
	}

	namespace
	{
		static constexpr uint16_t kVisPairMaskTable[6][6] = {
			{ 0x0000u, 0x0001u, 0x0002u, 0x0004u, 0x0008u, 0x0010u },
			{ 0x0001u, 0x0000u, 0x0020u, 0x0040u, 0x0080u, 0x0100u },
			{ 0x0002u, 0x0020u, 0x0000u, 0x0200u, 0x0400u, 0x0800u },
			{ 0x0004u, 0x0040u, 0x0200u, 0x0000u, 0x1000u, 0x2000u },
			{ 0x0008u, 0x0080u, 0x0400u, 0x1000u, 0x0000u, 0x4000u },
			{ 0x0010u, 0x0100u, 0x0800u, 0x2000u, 0x4000u, 0x0000u },
		};
	}

	uint16_t BlockChunk::GetVisEdgeMask(int faceA, int faceB)
	{
		if ((unsigned)faceA >= 6u || (unsigned)faceB >= 6u)
			return 0;
		return kVisPairMaskTable[faceA][faceB];
	}

	bool BlockChunk::GetBestOpaqueLayerSpan(uint16_t layerMask, int& outMinY, int& outMaxY)
	{
		if (layerMask == 0)
			return false;

		int bestStart = -1;
		int bestLen = 0;
		int curStart = -1;
		int curLen = 0;

		for (int cy = 0; cy < 16; ++cy)
		{
			if ((layerMask & (1u << cy)) != 0)
			{
				if (curLen == 0)
					curStart = cy;
				++curLen;
				if (curLen >= bestLen)
				{
					bestLen = curLen;
					bestStart = curStart;
				}
			}
			else
			{
				curLen = 0;
			}
		}

		if (bestLen <= 0)
			return false;

		outMinY = bestStart;
		outMaxY = bestStart + bestLen - 1;
		return true;
	}

	void BlockChunk::UpdateVisibilityGraph()
	{
		// Each pair of section faces has 256 disjoint paths between them.
		// Fewer occupied cells cannot seal a face or form an opaque layer.
		if (m_occupiedCells < 256 || m_blockIndices.size() < 4096)
		{
			m_uVisGraph = kVisGraphAllConnected;
			m_opaqueLayerMask = 0;
			m_bVisGraphDirty = false;
			return;
		}

		uint64_t visited[64] = {};
		int solidCount = 0;
		const int16_t* indices = m_blockIndices.data();
		const Block* blockPool = m_blocks.data();
		const int poolSize = (int)m_blocks.size();

		for (int idx = 0; idx < 4096; ++idx)
		{
			int16_t poolIdx = indices[idx];
			if (poolIdx >= 0 && poolIdx < poolSize)
			{
				BlockTemplate* pTemplate = const_cast<Block&>(blockPool[poolIdx]).GetTemplate();
				if (pTemplate &&
					pTemplate->IsMatchAttribute(BlockTemplate::batt_solid) &&
					!pTemplate->IsMatchAttribute(BlockTemplate::batt_transparent | BlockTemplate::batt_invisible))
				{
					++solidCount;
					visited[idx >> 6] |= (1ULL << (idx & 63));
				}
			}
		}

		if (solidCount == 0)
		{
			m_uVisGraph = kVisGraphAllConnected;
			m_opaqueLayerMask = 0;
			m_bVisGraphDirty = false;
			return;
		}

		if (solidCount == 4096)
		{
			m_uVisGraph = kVisGraphFullyOpaque;
			m_opaqueLayerMask = 0xFFFFu;
			m_bVisGraphDirty = false;
			return;
		}

		uint16_t layerMask = 0;
		for (int cy = 0; cy < 16; ++cy)
		{
			const int w = cy << 2;
			if ((visited[w] & visited[w + 1] & visited[w + 2] & visited[w + 3]) == ~0ULL)
				layerMask |= static_cast<uint16_t>(1u << cy);
		}
		m_opaqueLayerMask = layerMask;

		static constexpr uint64_t kMidLayerBoundaryMasks[4] = {
			0x800180018001FFFFULL,
			0x8001800180018001ULL,
			0x8001800180018001ULL,
			0xFFFF800180018001ULL,
		};

		uint16_t visGraph = 0;
		uint16_t queue[4096];

		for (int wordIdx = 0; wordIdx < 64 && visGraph != kVisGraphAllConnected; ++wordIdx)
		{
			const uint64_t boundaryMask = (wordIdx < 4 || wordIdx >= 60)
				? ~0ULL
				: kMidLayerBoundaryMasks[wordIdx & 3];

			uint64_t seeds = (~visited[wordIdx]) & boundaryMask;
			while (seeds != 0)
			{
				int bitPos = 0;
				{
					uint64_t tmp = seeds;
					while ((tmp & 1ULL) == 0)
					{
						tmp >>= 1;
						++bitPos;
					}
				}
				const uint64_t seedBit = 1ULL << bitPos;
				seeds &= ~seedBit;
				if ((visited[wordIdx] & seedBit) != 0)
					continue;

				visited[wordIdx] |= seedBit;
				int qHead = 0;
				int qTail = 0;
				queue[qTail++] = static_cast<uint16_t>((wordIdx << 6) | bitPos);

				uint8_t touchedFaces = 0;
				while (qHead < qTail)
				{
					const uint16_t cur = queue[qHead++];
					const uint16_t cx = cur & 0xFu;
					const uint16_t cz = (cur >> 4) & 0xFu;
					const uint16_t cy = cur >> 8;

					if (cx == 15) touchedFaces |= (1u << BlockDirection::left);
					if (cx == 0)  touchedFaces |= (1u << BlockDirection::right);
					if (cz == 15) touchedFaces |= (1u << BlockDirection::front);
					if (cz == 0)  touchedFaces |= (1u << BlockDirection::back);
					if (cy == 15) touchedFaces |= (1u << BlockDirection::top);
					if (cy == 0)  touchedFaces |= (1u << BlockDirection::bottom);

					auto tryPush = [&](uint16_t n) {
						uint64_t& w = visited[n >> 6];
						const uint64_t b = 1ULL << (n & 63);
						if ((w & b) == 0)
						{
							w |= b;
							queue[qTail++] = n;
						}
					};

					if (cx < 15) tryPush(cur + 1);
					if (cx > 0)  tryPush(cur - 1);
					if (cz < 15) tryPush(cur + 16);
					if (cz > 0)  tryPush(cur - 16);
					if (cy < 15) tryPush(cur + 256);
					if (cy > 0)  tryPush(cur - 256);
				}

				if (touchedFaces == 0x3Fu)
				{
					visGraph = kVisGraphAllConnected;
					break;
				}

				for (int a = 0; a < 5; ++a)
				{
					if ((touchedFaces & (1u << a)) == 0)
						continue;
					for (int b = a + 1; b < 6; ++b)
					{
						if ((touchedFaces & (1u << b)) != 0)
							visGraph |= kVisPairMaskTable[a][b];
					}
				}

				seeds &= ~visited[wordIdx];
			}
		}

		m_uVisGraph = visGraph;
		m_bVisGraphDirty = false;
	}

	uint16_t BlockChunk::GetVisibilityGraph()
	{
		if (m_bVisGraphDirty)
			UpdateVisibilityGraph();
		return m_uVisGraph;
	}

	uint16_t BlockChunk::GetOpaqueLayerMask()
	{
		if (m_bVisGraphDirty)
			UpdateVisibilityGraph();
		return m_opaqueLayerMask;
	}
	
	void LightData::SetBrightness( uint8_t value,bool isSunLight )
	{
		uint16_t light = (value & BlockConfig::g_maxValidLightValue);
		if(isSunLight)
		{
			m_value = (m_value & 0x0F) | (light << 4);
		}
		else
		{
			m_value = (m_value & 0xF0) | light;
		}
	}

	uint8_t LightData::GetBrightness( bool isSunLight )
	{
		if(isSunLight)
		{
			return (uint8_t)((m_value & 0xF0) >> 4);
		}
		else
		{
			return (uint8_t)(m_value & 0x0f);
		}
	}

	void LightData::LoadBrightness(uint8_t sun, uint8_t torch)
	{
		m_value = (sun << 4) + torch;
	}

	uint8_t LightData::GetMaxBrightness(CBlockWorld* pBlockWorld)
	{
		uint8_t v0 = m_value & 0x0f;
		uint8_t v1 = ((m_value & 0xf0) >> 4);

		uint8_t sun = (uint8_t)(v1 * pBlockWorld->GetSunIntensity());
		return (v0 > sun ? v0 : sun);
	}

	int32 Block::GetTextureIndex()
	{
		if (m_pTemplate)
			return m_pTemplate->GetBlockModelByData(m_blockData).GetTextureIndex();
		else
			return 0;
	}

}
