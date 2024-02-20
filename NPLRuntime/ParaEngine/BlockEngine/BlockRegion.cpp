//-----------------------------------------------------------------------------
// Class:	BlockRegion
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2013.10
// desc	: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockRegion.h"
#include "BlockWorld.h"
#include "terrain/Settings.h"
#include "util/StringBuilder.h"
#include "BlockLightGridBase.h"
#include "BlockCommon.h"
#include "BlockDataCodec.h"
#include "NPLRuntime.h"
#include "WorldInfo.h"

#include <zlib.h>
#include <unordered_map>

namespace ParaEngine
{
	/**	we will not use custom data for blocks that has very few instances(less than this value),
	since custom data has 64bits overhead.
	*/
	const int SKIP_CUSTOM_BLOCK_COUNT = 4;

	//////////////////////////////////////////////////////////////////////////
	//BlockRegion
	//////////////////////////////////////////////////////////////////////////
	BlockRegion::BlockRegion(int16_t regionX, int16_t regionZ, CBlockWorld* pBlockWorld)
		:m_bIsModified(false), m_pBlockWorld(pBlockWorld), m_bIsLocked(false), m_nEventAsyncLoadWorldFinished(0), m_nChunksLoaded(0), m_nTotalBytes(0)
	{
		m_regionX = regionX;
		m_regionZ = regionZ;

		m_minChunkId_ws.x = m_regionX * BlockConfig::g_regionChunkDimX;
		m_minChunkId_ws.y = 0;
		m_minChunkId_ws.z = m_regionZ * BlockConfig::g_regionChunkDimZ;

		m_maxChunkId_ws.x = m_minChunkId_ws.x + BlockConfig::g_regionChunkDimX;
		m_maxChunkId_ws.y = BlockConfig::g_regionChunkDimY;
		m_maxChunkId_ws.z = m_minChunkId_ws.z + BlockConfig::g_regionChunkDimX;

		m_minBlockId_ws.x = m_regionX * BlockConfig::g_regionBlockDimX;
		m_minBlockId_ws.y = 0;
		m_minBlockId_ws.z = m_regionZ * BlockConfig::g_regionBlockDimZ;

		m_maxBlockId_ws.x = m_minBlockId_ws.x + BlockConfig::g_regionChunkDimX;
		m_maxBlockId_ws.y = BlockConfig::g_regionBlockDimY;
		m_maxBlockId_ws.z = m_minBlockId_ws.z + BlockConfig::g_regionChunkDimZ;


		int32_t chunkCount = GetChunksCount();
		m_chunks = new BlockChunkPtr[chunkCount];
		memset(m_chunks, 0, sizeof(BlockChunkPtr)*chunkCount);

		m_blockHeightMap.resize(BlockConfig::g_regionBlockDimX * BlockConfig::g_regionBlockDimZ, ChunkMaxHeight(0, 0));

		// TODO:
		// m_biomes.resize(BlockConfig::g_regionBlockDimX * BlockConfig::g_regionBlockDimZ, 0);

		m_chunkTimestamp.resize(BlockConfig::g_regionChunkDimX * BlockConfig::g_regionChunkDimZ, 0);
	}

	int BlockRegion::GetChunksCount()
	{
		return BlockConfig::g_regionChunkDimX * BlockConfig::g_regionChunkDimY *BlockConfig::g_regionChunkDimZ;
	}

	BlockRegion::~BlockRegion()
	{
		if (m_thread.joinable())
			m_thread.join();

		uint32_t nCount = GetChunksCount();
		for (uint32_t i = 0; i < nCount; i++)
		{
			SAFE_DELETE(m_chunks[i]);
		}
		SAFE_DELETE_ARRAY(m_chunks);
	}

	BlockChunk* BlockRegion::CreateNewChunk(uint16_t chunkID)
	{
		BlockChunk* pChunk = m_chunks[chunkID];
		if (!pChunk)
		{
			pChunk = new BlockChunk(chunkID, this);
			m_chunks[chunkID] = pChunk;
		}
		return pChunk;
	}

	BlockChunk* BlockRegion::GetChunk(uint16_t packedChunkID, bool createIfNotExist = true)
	{
		BlockChunk * pChunk = m_chunks[packedChunkID];
		if (pChunk)
			return pChunk;
		else if (createIfNotExist)
		{
			pChunk = CreateNewChunk(packedChunkID);
		}
		return pChunk;
	}

	void BlockRegion::SetChunkDirty(uint16_t packedChunkID, bool isDirty)
	{
		if (!IsLocked())
		{
			BlockChunk* pChunk = GetChunk(packedChunkID, false);
			if (pChunk)
				pChunk->SetDirty(isDirty);
		}
	}

	void BlockRegion::SetChunkDirtyByNeighbor(uint16_t packedChunkID)
	{
		if (!IsLocked())
		{
			BlockChunk* pChunk = GetChunk(packedChunkID, false);
			if (pChunk)
				pChunk->SetDirtyByNeighbor();
		}
	}

	void BlockRegion::SetChunkLightDirty(uint16_t packedChunkID)
	{
		if (!IsLocked())
		{
			BlockChunk* pChunk = GetChunk(packedChunkID, false);
			if (pChunk)
				pChunk->SetLightDirty();
		}
	}

	Block* BlockRegion::GetBlock(uint16_t chunkId, Uint16x3& blockID_r)
	{
		BlockChunk * pChunk = m_chunks[chunkId];
		return (pChunk) ? pChunk->GetBlock(CalcPackedBlockID(blockID_r)) : NULL;
	}

	Block* BlockRegion::GetBlock(uint16_t x_rs, uint16_t y_rs, uint16_t z_rs)
	{
		Uint16x3 blockId_rs(x_rs, y_rs, z_rs);
		uint16_t packedChunkId_rs = CalcPackedChunkID(x_rs, y_rs, z_rs);
		return GetBlock(packedChunkId_rs, blockId_rs);
	}

	bool BlockRegion::SetBlockToAir(uint16_t packedChunkId_rs, Uint16x3& blockId_r)
	{
		BlockChunk * pChunk = m_chunks[packedChunkId_rs];
		if (pChunk)
		{
			return pChunk->SetBlockToAir(blockId_r);
		}
		return false;
	}

	void BlockRegion::SetChunksDirtyByBlockTemplate(uint16_t templateId)
	{
		uint32_t nCount = GetChunksCount();
		for (uint32_t i = 0; i < nCount; i++)
		{
			BlockChunk* pChunk = m_chunks[i];
			if (pChunk)
			{
				pChunk->refreshBlockVisible(templateId);
			}
		}
	}

	void BlockRegion::LoadBlockAsync(uint16_t x_rs, uint16_t y_rs, uint16_t z_rs, uint16_t blockId, uint32_t userData)
	{
		if (IsLocked())
		{
			auto chunkId = CalcPackedChunkID(x_rs, y_rs, z_rs);
			BlockChunk* pChunk = GetChunk(chunkId, true);
			if (pChunk)
			{
				BlockTemplate* pTemplate = GetBlockWorld()->GetBlockTemplate(blockId);
				if (pTemplate)
				{
					auto nIndex = CalcPackedBlockID(x_rs, y_rs, z_rs);
					pChunk->LoadBlock(nIndex, pTemplate);
					pChunk->SetBlockData(nIndex, userData);
				}
				else
				{
					Uint16x3 blockId_rs(x_rs, y_rs, z_rs);
					pChunk->SetBlockToAir(blockId_rs);
				}
				pChunk->SetDirty(true);
			}
		}
	}

	void BlockRegion::SetBlockTemplateByIndex(uint16_t blockX_rs, uint16_t blockY_rs, uint16_t blockZ_rs, BlockTemplate* pTemplate)
	{
		if (IsLocked())
			return;
#ifdef _DEBUG
		/*if (! m_pBlockWorld->GetReadWriteLock().HasWriterLock())
		{
			OUTPUT_LOG("error: writer lock is not acquired when calling block writing functions\n");
			return;
		}*/
#endif
		if (blockY_rs >= BlockConfig::g_regionBlockDimY)
		{
			OUTPUT_LOG("warn: SetBlockTemplateByIndex y is %d which is too big.\n", blockY_rs);
			return;
		}

		bool bLightSuspended = m_pBlockWorld->IsLightUpdateSuspended();

		uint16_t packedChunkId_rs = CalcPackedChunkID(blockX_rs, blockY_rs, blockZ_rs);
		Uint16x3 blockId_rs(blockX_rs, blockY_rs, blockZ_rs);
		if (pTemplate == nullptr)
		{
			if (SetBlockToAir(packedChunkId_rs, blockId_rs))
			{
				//if(!bLightSuspended)
				{
					SetModified();
					SetChunkDirty(packedChunkId_rs, true);
					CheckNeighborChunkDirty(blockId_rs);
					UpdateBlockHeightMap(blockId_rs, true, false);

					blockId_rs.x += m_minBlockId_ws.x;
					blockId_rs.z += m_minBlockId_ws.z;

					m_pBlockWorld->SetLightBlockDirty(blockId_rs, false);
					m_pBlockWorld->SetLightBlockDirty(blockId_rs, true);
					m_pBlockWorld->DeselectBlock(blockId_rs.x, blockId_rs.y, blockId_rs.z);
				}
			}
		}
		else
		{
			bool createIfNotExist = (pTemplate != nullptr);
			BlockChunk* pChunk = GetChunk(packedChunkId_rs, createIfNotExist);
			if (pChunk)
			{
				uint16 nBlockIndex = CalcPackedBlockID(blockId_rs);
				BlockTemplate* pPrevBlockTemplate = pChunk->GetBlockTemplate(nBlockIndex);
				if (pPrevBlockTemplate != pTemplate)
				{
					bool prevIsLight = false;
					if (pPrevBlockTemplate)
						prevIsLight = pPrevBlockTemplate->IsMatchAttribute(BlockTemplate::batt_light);
					bool curIsLight = pTemplate->IsMatchAttribute(BlockTemplate::batt_light);

					pChunk->SetBlock(nBlockIndex, pTemplate, 0);
					SetModified();
					SetChunkDirty(packedChunkId_rs, true);
					CheckNeighborChunkDirty(blockId_rs);
					UpdateBlockHeightMap(blockId_rs, false, pTemplate->IsTransparent());

					//add/remove to/from light table.
					if (prevIsLight && !curIsLight)
					{
						pChunk->RemoveLight(blockId_rs);
					}
					else if (!prevIsLight && curIsLight)
					{
						pChunk->AddLight(blockId_rs);
					}

					blockId_rs.x += m_minBlockId_ws.x;
					blockId_rs.z += m_minBlockId_ws.z;

					if (!bLightSuspended)
					{
						if (curIsLight)
						{
							m_pBlockWorld->SetLightBlockDirty(blockId_rs, false);
						}
						else
						{
							m_pBlockWorld->SetLightBlockDirty(blockId_rs, false);
							if (!(pPrevBlockTemplate && pPrevBlockTemplate->IsMatchAttribute(BlockTemplate::batt_solid) && pTemplate->IsMatchAttribute(BlockTemplate::batt_solid)))
							{
								// update sun light 
								m_pBlockWorld->SetLightBlockDirty(blockId_rs, true);
							}
						}
					}
				}
			}
		}
	}

	void BlockRegion::RefreshBlockTemplateByIndex(uint16_t blockX_rs, uint16_t blockY_rs, uint16_t blockZ_rs, BlockTemplate* pTemplate)
	{
		if (IsLocked())
			return;

		if (blockY_rs >= BlockConfig::g_regionBlockDimY)
		{
			OUTPUT_LOG("warn: SetBlockTemplateByIndex y is %d which is too big.\n", blockY_rs);
			return;
		}

		bool bLightSuspended = m_pBlockWorld->IsLightUpdateSuspended();

		uint16_t packedChunkId_rs = CalcPackedChunkID(blockX_rs, blockY_rs, blockZ_rs);
		Uint16x3 blockId_rs(blockX_rs, blockY_rs, blockZ_rs);

		//on remove
		SetModified();
		SetChunkDirty(packedChunkId_rs, true);
		CheckNeighborChunkDirty(blockId_rs);
		UpdateBlockHeightMap(blockId_rs, true, false);

		blockId_rs.x += m_minBlockId_ws.x;
		blockId_rs.z += m_minBlockId_ws.z;

		m_pBlockWorld->SetLightBlockDirty(blockId_rs, false);
		m_pBlockWorld->SetLightBlockDirty(blockId_rs, true);
		m_pBlockWorld->DeselectBlock(blockId_rs.x, blockId_rs.y, blockId_rs.z);

		//reset blockId_rs
		blockId_rs.x -= m_minBlockId_ws.x;
		blockId_rs.z -= m_minBlockId_ws.z;

		//on add
		BlockChunk* pChunk = GetChunk(packedChunkId_rs, false);
		if (pChunk)
		{
			uint16 nBlockIndex = CalcPackedBlockID(blockId_rs);
			BlockTemplate* pPrevBlockTemplate = pChunk->GetBlockTemplate(nBlockIndex);
			bool prevIsLight = false;
			if (pPrevBlockTemplate)
				prevIsLight = pPrevBlockTemplate->IsMatchAttribute(BlockTemplate::batt_light);
			bool curIsLight = pTemplate->IsMatchAttribute(BlockTemplate::batt_light);


			SetModified();
			SetChunkDirty(packedChunkId_rs, true);
			CheckNeighborChunkDirty(blockId_rs);
			UpdateBlockHeightMap(blockId_rs, false, pTemplate->IsTransparent());

			//add/remove to/from light table.
			if (prevIsLight && !curIsLight)
			{
				pChunk->RemoveLight(blockId_rs);
			}
			else if (!prevIsLight && curIsLight)
			{
				pChunk->AddLight(blockId_rs);
			}

			blockId_rs.x += m_minBlockId_ws.x;
			blockId_rs.z += m_minBlockId_ws.z;

			if (!bLightSuspended)
			{
				if (curIsLight)
				{
					m_pBlockWorld->SetLightBlockDirty(blockId_rs, false);
				}
				else
				{
					m_pBlockWorld->SetLightBlockDirty(blockId_rs, false);
					if (!(pPrevBlockTemplate && pPrevBlockTemplate->IsMatchAttribute(BlockTemplate::batt_solid) && pTemplate->IsMatchAttribute(BlockTemplate::batt_solid)))
					{
						// update sun light 
						m_pBlockWorld->SetLightBlockDirty(blockId_rs, true);
					}
				}
			}
		}
	}

	bool BlockRegion::IntersectBlock(int16_t x, int16_t y, int16_t z, uint32_t filter)
	{
		if (!IsLocked())
		{
			Uint16x3 blockId(x, y, z);
			uint16_t chunkId = CalcPackedChunkID(x, y, z);
			Block* pBlock = GetBlock(chunkId, blockId);

			if (pBlock)
			{
				uint16_t templateId = pBlock->GetTemplateId();
				templateId &= 0xffff;

				const BlockTemplate* temp = m_pBlockWorld->GetBlockTemplate(templateId);
				if (temp && (temp->GetAttFlag() & filter) > 0)
					return true;
			}
		}
		return false;
	}

	uint32_t BlockRegion::GetBlockTemplateIdByIndex(int16_t x, int16_t y, int16_t z)
	{
		if (!IsLocked())
		{
			Uint16x3 blockId(x, y, z);
			uint16_t chunkId = CalcPackedChunkID(x, y, z);
			Block* pBlock = GetBlock(chunkId, blockId);
			return (pBlock == nullptr) ? 0 : pBlock->GetTemplate()->GetID();
		}
		else
		{

			Uint16x3 blockId(x, y, z);
			uint16_t chunkId = CalcPackedChunkID(x, y, z);
			Block* pBlock = GetBlock(chunkId, blockId);
			Block* pBlock1 = GetBlock(chunkId, blockId);
			// This is safe in 99.99% times, we will fetch 2 times, just in case the other thread is assigning the integer
			if (pBlock1 == pBlock)
				return (pBlock == nullptr) ? 0 : pBlock->GetTemplate()->GetID();
		}
		return 0;
	}

	BlockTemplate* BlockRegion::GetBlockTemplateByIndex(int16_t x, int16_t y, int16_t z)
	{
		if (!IsLocked())
		{
			Uint16x3 blockId(x, y, z);
			uint16_t chunkId = CalcPackedChunkID(x, y, z);
			Block* pBlock = GetBlock(chunkId, blockId);
			if (pBlock)
				return pBlock->GetTemplate();
		}
		else
		{
			Uint16x3 blockId(x, y, z);
			uint16_t chunkId = CalcPackedChunkID(x, y, z);
			// This is safe in 99.99% times, we will fetch 2 times, just in case the other thread is assigning the integer
			Block* pBlock = GetBlock(chunkId, blockId);
			Block* pBlock1 = GetBlock(chunkId, blockId);
			if (pBlock1 == pBlock && pBlock)
				return pBlock->GetTemplate();
		}
		return nullptr;
	}

	void BlockRegion::SetBlockUserDataByIndex(int16_t x, int16_t y, int16_t z, uint32_t data)
	{
		if (!IsLocked())
		{
			Uint16x3 blockId(x, y, z);
			uint16_t chunkId = CalcPackedChunkID(x, y, z);
			BlockChunk* pChunk = GetChunk(chunkId, false);
			if (pChunk)
			{
				uint16 nBlockIndex = CalcPackedBlockID(x, y, z);
				pChunk->SetBlockData(nBlockIndex, data);
				SetModified();
				BlockTemplate* pTemplate = pChunk->GetBlockTemplate(nBlockIndex);
				if (pTemplate && pTemplate->IsMatchAttribute(BlockTemplate::batt_cubeModel))
					SetChunkDirty(chunkId, true);
			}
		}
	}

	uint32_t BlockRegion::GetBlockUserDataByIndex(int16_t x, int16_t y, int16_t z)
	{
		if (!IsLocked())
		{
			Uint16x3 blockId(x, y, z);
			uint16_t chunkId = CalcPackedChunkID(x, y, z);
			Block* pBlock = GetBlock(chunkId, blockId);
			if (pBlock != nullptr)
				return pBlock->GetUserData();
		}
		return 0;
	}

	bool BlockRegion::SetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId, int32_t nMaterial)
	{
		if (!IsLocked())
		{
			Uint16x3 blockId(x, y, z);
			uint16_t chunkId = CalcPackedChunkID(x, y, z);
			BlockChunk* pChunk = GetChunk(chunkId, false);
			if (pChunk)
			{
				uint16 nBlockIndex = CalcPackedBlockID(x, y, z);
				pChunk->ApplyBlockMaterial(nBlockIndex, nFaceId, nMaterial);
				SetModified();
				SetChunkDirty(chunkId, true);
			}
		}
		return false;
	}


	int32_t BlockRegion::GetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId)
	{
		if (!IsLocked())
		{
			Uint16x3 blockId(x, y, z);
			uint16_t chunkId = CalcPackedChunkID(x, y, z);
			BlockChunk* pChunk = GetChunk(chunkId, false);
			if (pChunk)
			{
				uint16 nBlockIndex = CalcPackedBlockID(x, y, z);
				return pChunk->GetBlockFaceMaterial(nBlockIndex, nFaceId);
			}
		}
		return -1;
	}

	ChunkMaxHeight* BlockRegion::GetHighestBlock(uint16_t blockIdX_rs, uint16_t blockIdZ_rs)
	{
		return &m_blockHeightMap[blockIdX_rs + (blockIdZ_rs << 9)];
	}


	void BlockRegion::GetBlocksInChunk(uint16_t chunkX_ws, uint16_t chunkZ_ws, uint32_t verticalSectionFilter, uint32_t matchtype, const luabind::adl::object& luaTable, int32_t& blockCount)
	{
		if (IsLocked())
			return;
		luabind::adl::object xTable = luaTable["x"];
		luabind::adl::object yTable = luaTable["y"];
		luabind::adl::object zTable = luaTable["z"];
		luabind::adl::object templateTable = luaTable["tempId"];
		luabind::adl::object dataTable = luaTable["data"];

		uint16_t chunkX_rs = chunkX_ws & 0x1f;
		uint16_t chunkZ_rs = chunkZ_ws & 0x1f;

		uint16_t startBlockIdX_ws = chunkX_ws << 4;
		uint16_t startBlockIdZ_ws = chunkZ_ws << 4;

		for (uint16_t y = 0; y <= 16; y++)
		{
			if ((verticalSectionFilter & (1 << y)) != 0)
			{
				uint16_t startBlockIdY_ws = y << 4;
				uint16_t packedChunkId_rs = PackChunkIndex(chunkX_rs, y, chunkZ_rs);
				BlockChunk * pChunk = m_chunks[packedChunkId_rs];
				if (pChunk)
				{
					uint32_t nCount = pChunk->m_blockIndices.size();
					for (uint32_t i = 0; i < nCount; i++)
					{
						int32_t blockIdx = pChunk->m_blockIndices[i];
						if (blockIdx >= 0)
						{
							Block& curBlock = pChunk->GetBlockByIndex(blockIdx);
							if (curBlock.GetTemplate() && curBlock.GetTemplate()->IsMatchAttribute(matchtype))
							{
								uint16_t rx, ry, rz;
								UnpackBlockIndex(i, rx, ry, rz);
								blockCount++;
								xTable[blockCount] = (int32_t)rx + startBlockIdX_ws;
								yTable[blockCount] = (int32_t)ry + startBlockIdY_ws;
								zTable[blockCount] = (int32_t)rz + startBlockIdZ_ws;
								templateTable[blockCount] = curBlock.GetTemplateId();
								dataTable[blockCount] = curBlock.GetUserData();
							}
						}
					}
				}
			}
		}
	}

	void BlockRegion::GetBlocksInChunk(uint16_t chunkX_ws, uint16_t chunkZ_ws, uint16_t startChunkY, uint16_t endChunkY,
		uint32_t matchtype, const luabind::adl::object& luaTable, int32_t& blockCount)
	{
		if (IsLocked())
			return;
		luabind::adl::object xTable = luaTable["x"];
		luabind::adl::object yTable = luaTable["y"];
		luabind::adl::object zTable = luaTable["z"];
		luabind::adl::object templateTable = luaTable["tempId"];
		luabind::adl::object dataTable = luaTable["data"];

		uint16_t chunkX_rs = chunkX_ws & 0x1f;
		uint16_t chunkZ_rs = chunkZ_ws & 0x1f;

		uint16_t startBlockIdX_ws = chunkX_ws << 4;
		uint16_t startBlockIdZ_ws = chunkZ_ws << 4;

		for (uint16_t y = startChunkY; y <= endChunkY; y++)
		{
			uint16_t startBlockIdY_ws = y << 4;
			uint16_t packedChunkId_rs = PackChunkIndex(chunkX_rs, y, chunkZ_rs);
			BlockChunk * pChunk = m_chunks[packedChunkId_rs];
			if (pChunk)
			{
				uint32_t nCount = pChunk->m_blockIndices.size();
				for (uint32_t i = 0; i < nCount; i++)
				{
					int32_t blockIdx = pChunk->m_blockIndices[i];
					if (blockIdx >= 0)
					{
						Block& curBlock = pChunk->GetBlockByIndex(blockIdx);
						if (curBlock.GetTemplate() && curBlock.GetTemplate()->IsMatchAttribute(matchtype))
						{
							uint16_t rx, ry, rz;
							UnpackBlockIndex(i, rx, ry, rz);
							blockCount++;
							xTable[blockCount] = (int32_t)rx + startBlockIdX_ws;
							yTable[blockCount] = (int32_t)ry + startBlockIdY_ws;
							zTable[blockCount] = (int32_t)rz + startBlockIdZ_ws;
							templateTable[blockCount] = curBlock.GetTemplateId();
							dataTable[blockCount] = curBlock.GetUserData();
						}
					}
				}
			}
		}
	}


	void BlockRegion::RefreshAllLightsInColumn(uint16_t chunkX_ws, uint16_t chunkZ_ws)
	{
		if (IsLocked())
			return;
		uint16_t chunkX_rs = chunkX_ws - m_minChunkId_ws.x;
		uint16_t chunkZ_rs = chunkZ_ws - m_minChunkId_ws.z;

		Uint16x3 curBlock;
		for (int y = 0; y < BlockConfig::g_regionChunkDimY; y++)
		{
			uint16_t packedChunkId_rs = PackChunkIndex(chunkX_rs, y, chunkZ_rs);
			BlockChunk* pChunk = m_chunks[packedChunkId_rs];
			if (pChunk)
			{
				uint16_t cx_min = chunkX_rs << 4;
				uint16_t cy_min = y << 4;
				uint16_t cz_min = chunkZ_rs << 4;

				std::set<uint16_t>::iterator itCur, itEnd = pChunk->m_lightBlockIndices.end();
				for (itCur = pChunk->m_lightBlockIndices.begin(); itCur != itEnd; itCur++)
				{
					uint16_t nLightIndex = (*itCur);
					uint16_t cx, cy, cz;
					UnpackBlockIndex(nLightIndex, cx, cy, cz);

					cx += cx_min;
					cy += cy_min;
					cz += cz_min;
					curBlock.x = cx + m_minBlockId_ws.x;
					curBlock.y = cy;
					curBlock.z = cz + m_minBlockId_ws.z;

					m_pBlockWorld->SetLightBlockDirty(curBlock, false);
				}
			}
		}
	}

	void BlockRegion::CheckNeighborChunkDirty(Uint16x3& blockId_rs)
	{
#ifdef USE_STRICT_CHECKING
		int16_t blockX_cs = blockId_rs.x & 0xf;
		int16_t blockY_cs = blockId_rs.y & 0xf;
		int16_t blockZ_cs = blockId_rs.z & 0xf;

		uint16_t maxChunkBlockId = BlockConfig::g_chunkBlockDim - 1;
		uint16_t maxRegionBlockId = BlockConfig::g_regionBlockDimX - 1;

		//non-boundary block
		if (blockX_cs != 0 && blockX_cs != maxChunkBlockId && blockY_cs != 0 && blockY_cs != maxChunkBlockId
			&& blockZ_cs != 0 && blockZ_cs != maxChunkBlockId)
			return;
		int32_t BlockX_ws = blockId_rs.x + m_minBlockId_ws.x;
		int32_t BlockY_ws = blockId_rs.y + m_minBlockId_ws.y;
		int32_t BlockZ_ws = blockId_rs.z + m_minBlockId_ws.z;
		Uint16x3 ChunkId_ws(BlockX_ws >> 4, BlockY_ws >> 4, BlockZ_ws >> 4);

		Uint16x3 dirtyChunks[8];
		int nDirtyCount = 0;
		const Int16x3* neighborOfsTable = BlockCommon::NeighborOfsTable;
		for (int i = 1; i <= 26; ++i)
		{
			const Int16x3& vDelta = neighborOfsTable[i];
			Uint16x3 curBlockId_ws(BlockX_ws + vDelta.x, BlockY_ws + vDelta.y, BlockZ_ws + vDelta.z);
			Uint16x3 curChunkId_ws(curBlockId_ws.x >> 4, curBlockId_ws.y >> 4, curBlockId_ws.z >> 4);
			if (curChunkId_ws != ChunkId_ws)
			{
				bool bHasUpdated = false;
				for (int nCount = 0; nCount < nDirtyCount; ++nCount)
				{
					if (dirtyChunks[nCount] == curChunkId_ws)
					{
						bHasUpdated = true;
						break;
					}
				}
				if (!bHasUpdated)
				{
					if (nDirtyCount < 8) {
						dirtyChunks[nDirtyCount] = curBlockId_ws;
						nDirtyCount++;
					}
					SetNeighborChunkDirty(curChunkId_ws);
				}
			}
		}
#else
		uint16_t blockX_cs = blockId_rs.x & 0xf;
		uint16_t blockY_cs = blockId_rs.y & 0xf;
		uint16_t blockZ_cs = blockId_rs.z & 0xf;

		uint16_t maxChunkBlockId = BlockConfig::g_chunkBlockDim - 1;
		uint16_t maxRegionBlockId = BlockConfig::g_regionBlockDimX - 1;

		//non-boundary block
		if (blockX_cs != 0 && blockX_cs != maxChunkBlockId && blockY_cs != 0 && blockY_cs != maxChunkBlockId
			&& blockZ_cs != 0 && blockZ_cs != maxChunkBlockId)
			return;

		int32_t chunkIdX_ws = (blockId_rs.x >> 4) + m_minChunkId_ws.x;
		int32_t chunkIdY_ws = (blockId_rs.y >> 4);
		int32_t chunkIdZ_ws = (blockId_rs.z >> 4) + m_minChunkId_ws.z;

		int32_t neighborChunkX_ws = chunkIdX_ws;
		if (blockX_cs == 0)
			neighborChunkX_ws -= 1;
		else if (blockX_cs == maxChunkBlockId)
			neighborChunkX_ws += 1;

		if (neighborChunkX_ws < 0)
			neighborChunkX_ws = 0;
		if (neighborChunkX_ws > 0xfffe)
			neighborChunkX_ws = 0xfffe;

		int32_t neighborChunkZ_ws = chunkIdZ_ws;
		if (blockZ_cs == 0)
			neighborChunkZ_ws -= 1;
		else if (blockZ_cs == maxChunkBlockId)
			neighborChunkZ_ws += 1;

		if (neighborChunkZ_ws < 0)
			neighborChunkZ_ws = 0;
		if (neighborChunkZ_ws > 0xfffe)
			neighborChunkZ_ws = 0xfffe;

		int32_t neighborChunkY_ws = chunkIdY_ws;
		if (blockY_cs == 0)
			neighborChunkY_ws -= 1;
		if (blockY_cs == maxChunkBlockId)
			neighborChunkY_ws += 1;

		if (neighborChunkY_ws < 0)
			neighborChunkY_ws = 0;
		if (neighborChunkY_ws > (BlockConfig::g_regionChunkDimY - 1))
			neighborChunkY_ws = BlockConfig::g_regionChunkDimY - 1;

		Uint16x3 curNeighbor_ws;

		if (neighborChunkX_ws != chunkIdX_ws)
		{
			if (neighborChunkY_ws != chunkIdY_ws)
			{
				if (neighborChunkZ_ws != chunkIdZ_ws)
				{
					// xyz
					curNeighbor_ws.x = (uint16_t)neighborChunkX_ws;
					curNeighbor_ws.y = (uint16_t)chunkIdY_ws;
					curNeighbor_ws.z = (uint16_t)chunkIdZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);

					curNeighbor_ws.x = (uint16_t)chunkIdX_ws;
					curNeighbor_ws.y = (uint16_t)neighborChunkY_ws;
					curNeighbor_ws.z = (uint16_t)chunkIdZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);

					curNeighbor_ws.x = (uint16_t)chunkIdX_ws;
					curNeighbor_ws.y = (uint16_t)chunkIdY_ws;
					curNeighbor_ws.z = (uint16_t)neighborChunkZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);
				}
				else
				{
					//xy
					curNeighbor_ws.x = (uint16_t)neighborChunkX_ws;
					curNeighbor_ws.y = (uint16_t)chunkIdY_ws;
					curNeighbor_ws.z = (uint16_t)chunkIdZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);

					curNeighbor_ws.x = (uint16_t)chunkIdX_ws;
					curNeighbor_ws.y = (uint16_t)neighborChunkY_ws;
					curNeighbor_ws.z = (uint16_t)chunkIdZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);

					curNeighbor_ws.x = (uint16_t)neighborChunkX_ws;
					curNeighbor_ws.y = (uint16_t)neighborChunkY_ws;
					curNeighbor_ws.z = (uint16_t)chunkIdZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);
				}
			}
			else
			{
				if (neighborChunkZ_ws != chunkIdZ_ws)
				{
					//xz
					curNeighbor_ws.x = (uint16_t)neighborChunkX_ws;
					curNeighbor_ws.y = (uint16_t)chunkIdY_ws;
					curNeighbor_ws.z = (uint16_t)chunkIdZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);

					curNeighbor_ws.x = (uint16_t)chunkIdX_ws;
					curNeighbor_ws.y = (uint16_t)chunkIdY_ws;
					curNeighbor_ws.z = (uint16_t)neighborChunkZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);

					curNeighbor_ws.x = (uint16_t)neighborChunkX_ws;
					curNeighbor_ws.y = (uint16_t)chunkIdY_ws;
					curNeighbor_ws.z = (uint16_t)neighborChunkZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);
				}
				else
				{
					//x
					curNeighbor_ws.x = (uint16_t)neighborChunkX_ws;
					curNeighbor_ws.y = (uint16_t)chunkIdY_ws;
					curNeighbor_ws.z = (uint16_t)chunkIdZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);
				}
			}

		}
		else
		{
			if (neighborChunkY_ws != chunkIdY_ws)
			{
				if (neighborChunkZ_ws != chunkIdZ_ws)
				{
					// yz
					curNeighbor_ws.x = (uint16_t)chunkIdX_ws;
					curNeighbor_ws.y = (uint16_t)neighborChunkY_ws;
					curNeighbor_ws.z = (uint16_t)chunkIdZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);

					curNeighbor_ws.x = (uint16_t)chunkIdX_ws;
					curNeighbor_ws.y = (uint16_t)chunkIdY_ws;
					curNeighbor_ws.z = (uint16_t)neighborChunkZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);

					curNeighbor_ws.x = (uint16_t)chunkIdX_ws;
					curNeighbor_ws.y = (uint16_t)neighborChunkY_ws;
					curNeighbor_ws.z = (uint16_t)neighborChunkZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);
				}
				else
				{
					//y
					curNeighbor_ws.x = (uint16_t)chunkIdX_ws;
					curNeighbor_ws.y = (uint16_t)neighborChunkY_ws;
					curNeighbor_ws.z = (uint16_t)chunkIdZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);
				}
			}
			else
			{
				if (neighborChunkZ_ws != chunkIdZ_ws)
				{
					//z
					curNeighbor_ws.x = (uint16_t)chunkIdX_ws;
					curNeighbor_ws.y = (uint16_t)chunkIdY_ws;
					curNeighbor_ws.z = (uint16_t)neighborChunkZ_ws;
					SetNeighborChunkDirty(curNeighbor_ws);
				}
			}
		}
#endif
	}

	void BlockRegion::SetNeighborChunkDirty(Uint16x3& neighborChunkId_ws)
	{
		if (neighborChunkId_ws.x >= m_minChunkId_ws.x
			&& neighborChunkId_ws.x < (m_minChunkId_ws.x + BlockConfig::g_regionChunkDimX)
			&& neighborChunkId_ws.z >= m_minChunkId_ws.z
			&& neighborChunkId_ws.z < (m_minChunkId_ws.z + BlockConfig::g_regionChunkDimZ))
		{
			Uint16x3 chunkId_rs;
			chunkId_rs.x = neighborChunkId_ws.x & 0x1f;
			chunkId_rs.y = neighborChunkId_ws.y;
			chunkId_rs.z = neighborChunkId_ws.z & 0x1f;

			uint16_t packChunkId_rs = PackChunkIndex(chunkId_rs);
			SetChunkDirtyByNeighbor(packChunkId_rs);
		}
		else
		{
			m_pBlockWorld->SetChunkDirty(neighborChunkId_ws, true);
		}
	}

	void BlockRegion::UpdateBlockHeightMap(Uint16x3& blockId_rs, bool isRemove, bool isTransparent)
	{
		ChunkMaxHeight& blockHeight = m_blockHeightMap[blockId_rs.x + (blockId_rs.z << 9)];
		ChunkMaxHeight prevHeight = blockHeight;

		if (isRemove)
		{
			if (blockId_rs.y >= blockHeight.GetMaxSolidHeight())
			{
				uint16_t chunkX_rs = blockId_rs.x >> 4;
				uint16_t chunkY_rs = blockId_rs.y >> 4;
				uint16_t chunkZ_rs = blockId_rs.z >> 4;

				Uint16x3 curBlockId_cs;
				curBlockId_cs.x = blockId_rs.x & 0xf;
				curBlockId_cs.z = blockId_rs.z & 0xf;

				blockHeight.ClearHeight();
				//find highest block below current block
				for (int y = chunkY_rs; y >= 0; y--)
				{
					uint16_t packedChunkId_rs = PackChunkIndex(chunkX_rs, y, chunkZ_rs);
					BlockChunk * pChunk = m_chunks[packedChunkId_rs];
					if (pChunk)
					{
						for (int cy = BlockConfig::g_chunkBlockDim - 1; cy >= 0; cy--)
						{
							curBlockId_cs.y = cy;
							uint16_t packedBlockId_cs = PackBlockId(curBlockId_cs.x, curBlockId_cs.y, curBlockId_cs.z);
							int32_t blockIdx = pChunk->m_blockIndices[packedBlockId_cs];
							if (blockIdx >= 0)
							{
								Block& block = pChunk->GetBlockByIndex(blockIdx);
								uint16_t wy = y * BlockConfig::g_chunkBlockDim + cy;
								if (!(block.GetTemplate()->IsTransparent()))
								{
									blockHeight.SetHeight(wy, false);
									m_pBlockWorld->NotifyBlockHeightMapChanged(blockId_rs.x + m_minBlockId_ws.x, blockId_rs.z + m_minBlockId_ws.z, prevHeight);
									return;
								}
								else
								{
									blockHeight.AddBlockHeight(wy, true);
								}
							}
						}
					}
				}
				m_pBlockWorld->NotifyBlockHeightMapChanged(blockId_rs.x + m_minBlockId_ws.x, blockId_rs.z + m_minBlockId_ws.z, prevHeight);
			}
		}
		else
		{
			if (blockId_rs.y < BlockConfig::g_regionBlockDimY)
			{
				blockHeight.AddBlockHeight(blockId_rs.y, isTransparent);
				m_pBlockWorld->NotifyBlockHeightMapChanged(blockId_rs.x + m_minBlockId_ws.x, blockId_rs.z + m_minBlockId_ws.z, prevHeight);
			}
		}
	}


	void BlockRegion::SaveToFile(const char* sFilename)
	{
		if (IsLocked())
			return;
		Scoped_WriteLock<BlockReadWriteLock> lock_(m_pBlockWorld->GetReadWriteLock());

		if (!IsModified())
			return;

		SetModified(false);

		std::string fileName = m_pBlockWorld->GetWorldInfo().GetBlockRegionFileName(m_regionX, m_regionZ, true);
		if (sFilename != 0 && sFilename[0] != '\0')
		{
			fileName = sFilename;
		}

		CParaFile cfile, memFile;

		if (cfile.CreateNewFile(fileName.c_str(), true) && memFile.OpenFile("<memory>", false))
		{
			//'b'<<24 + 'l'<<16 + 'o'<<8 + 'c' + 1;
			uint32_t fileTypeId = 0x626c6f63 + 1;
			cfile.WriteDWORD(fileTypeId);

			//version 1.00
			uint16_t version = 0x0100;
			cfile.WriteWORD(version);

			uint32_t regionId = (m_regionX << 16) + m_regionZ;
			memFile.WriteDWORD(regionId);

			std::vector<std::pair<uint16_t, uint16_t>> blockData;
			std::vector<std::pair<uint16_t, uint16_t>> blockMetaData;
			std::vector<uint8_t> blockLightMapData;
			std::map<uint16_t, std::vector<uint16_t> > blockIDToIndex;

			StringBuilder strBuilder(1024);
			uint32_t dataItemSize = sizeof(std::pair<uint16_t, uint16_t>);
			DWORD data_mask = 0;
			data_mask |= ChunkDataMask_HasCustomData;
			memFile.WriteDWORD(dataItemSize | data_mask);

			{
				strBuilder.clear();
				if (CIntegerEncoder::IsSameIntegerBetter(m_chunkTimestamp))
				{
					memFile.WriteEncodedUInt(ChunkCustomDataType_TimeStamp_SameInteger);
					CIntegerEncoder::EncodeSameInteger(strBuilder, m_chunkTimestamp);
				}
				else
				{
					memFile.WriteEncodedUInt(ChunkCustomDataType_TimeStamp);
					CIntegerEncoder::EncodeIntArray(strBuilder, m_chunkTimestamp);
				}
				memFile.WriteEncodedUInt(strBuilder.size());
				memFile.write(&(strBuilder[0]), strBuilder.size());
			}
			{
				// TODO: biomes of each 512*512 chunk column. 
			}
			{
				// TODO: heightmap of each 512*512 chunk column. 
			}
			if(!GetTag().empty())
			{
				memFile.WriteEncodedUInt(ChunkCustomDataType_Tag);
				memFile.WriteEncodedUInt((int)GetTag().size());
				memFile.write(GetTag().c_str(), (int)GetTag().size());
			}
			{
				memFile.WriteEncodedUInt(ChunkCustomDataType_Materials);
				int chunksCount = GetChunksCount();
				uint32_t totalSize = 0;
				std::unordered_map<uint16_t, std::shared_ptr<std::unordered_map<uint32_t, std::shared_ptr<std::vector<uint16_t>>>>> chunkMaterialKeyMap;
				for (int i = 0 ; i < chunksCount; i++)
				{
					BlockChunk* pChunk = m_chunks[i];
					if (!pChunk) continue;
					std::shared_ptr<std::unordered_map<uint32_t, std::shared_ptr<std::vector<uint16_t>>>> materialKeyMap = std::make_shared<std::unordered_map<uint32_t, std::shared_ptr<std::vector<uint16_t>>>>();
					chunkMaterialKeyMap.insert(std::make_pair(i, materialKeyMap));
					totalSize += sizeof(uint16_t) + sizeof(uint32_t);  // chunk index size + chunk list length size
					for (const auto& it : pChunk->m_materialsKeyIdMap)
					{
						uint16_t materialKey = (uint16_t)(it.first);
						uint32_t materialId = (uint32_t)(it.second);
						const auto keyListIt = materialKeyMap->find(materialId);
						std::shared_ptr<std::vector<uint16_t>> keyList = nullptr;
						if (keyListIt == materialKeyMap->end())
						{
							keyList = std::make_shared<std::vector<uint16_t>>();
							materialKeyMap->insert(std::make_pair(materialId, keyList));
							totalSize += sizeof(uint32_t) + sizeof(uint32_t); // material id size + key list length size
						}	
						else
						{
							keyList = keyListIt->second;
						}
						keyList->push_back(materialKey);
						totalSize += sizeof(uint16_t);  // material key size;
					}
				}
				memFile.WriteEncodedUInt(totalSize);
				for (const auto& chunkMaterialKeyIt : chunkMaterialKeyMap)
				{
					uint16_t chunkIndex = chunkMaterialKeyIt.first;
					const auto& materialKeyMap = chunkMaterialKeyIt.second;
					uint32_t chunkSize = materialKeyMap->size();
					memFile.write(&chunkIndex, sizeof(uint16_t));
					memFile.write(&chunkSize, sizeof(uint32_t));
					for (const auto& materialKeyIt : (*materialKeyMap))
					{
						uint32_t materialId = materialKeyIt.first;
						const auto keyList = materialKeyIt.second;
						uint32_t keyListSize = keyList->size();
						memFile.write(&materialId, sizeof(uint32_t));
						memFile.write(&keyListSize, sizeof(uint32_t));
						memFile.write(keyList->data(), keyListSize * sizeof(uint16_t));
					}
				}
			}
			memFile.WriteEncodedUInt(ChunkCustomDataType_ChunksData);

			// saving for all 32*32*16 chunks
			uint32_t nCount = GetChunksCount();
			for (uint32_t i = 0; i < nCount; i++)
			{
				BlockChunk * pChunk = m_chunks[i];
				if (!pChunk)
					continue;
				// first 32bits stores chunkid and data mask
				DWORD dwChunkID = i;
				// any bitwise field of ChunkDataMask
				DWORD dwChunkDataMask = 0;

				blockIDToIndex.clear();
				blockData.clear();
				blockMetaData.clear();
				blockLightMapData.clear();

				// prepare and pre-process data into memory

				uint16_t sizeCount = (uint16_t)pChunk->m_blockIndices.size();
				for (uint16_t j = 0; j < sizeCount; j++)
				{
					int32_t blockIdx = pChunk->m_blockIndices[j];
					if (blockIdx > -1)
					{
						Block& block = pChunk->GetBlockByIndex(blockIdx);
						auto& block_indices = blockIDToIndex[block.GetTemplateId()];
						block_indices.push_back(j);

						if (block.GetUserData() != 0)
						{
							blockMetaData.push_back(make_pair(j, (uint16_t)(block.GetUserData())));
						}
					}
				}

				int nCustomDataCount = 0;
				for (auto itCur = blockIDToIndex.begin(); itCur != blockIDToIndex.end(); ++itCur)
				{
					int nBlockCount = itCur->second.size();
					if (nBlockCount < SKIP_CUSTOM_BLOCK_COUNT  && nBlockCount >0)
					{
						// we will not use custom data for blocks that has very few instances,
						// since custom data has 64bits overhead. 
						uint16_t nBlockID = itCur->first;

						auto& block_indices = itCur->second;
						for (auto itBlockCur = block_indices.begin(); itBlockCur != block_indices.end(); ++itBlockCur)
						{
							blockData.push_back(make_pair(*itBlockCur, nBlockID));
						}
						block_indices.clear();
					}
					else
					{
						// in most cases, we will use custom data to store block id and indices
						nCustomDataCount++;
					}
				}

				// whether saving light map value
				if (m_pBlockWorld->IsSaveLightMap() && m_pBlockWorld->GetLightGrid().GetDirtyBlockCount() == 0)
				{
					uint32_t nCount = pChunk->m_lightmapArray.size();
					bool isAllZero = false;
					blockLightMapData.resize(nCount);
					for (uint32_t i = 0; i < nCount; i++)
					{
						LightData& lightValue = pChunk->m_lightmapArray[i];
						if (!isAllZero && !lightValue.IsZero())
							isAllZero = false;
						blockLightMapData[i] = lightValue.GetBrightness(false) | (lightValue.GetBrightness(true) << 4);
					}
					if (!isAllZero)
					{
						nCustomDataCount++;
					}
					else
					{
						blockLightMapData.clear();
					}
				}

				if (nCustomDataCount > 0)
				{
					dwChunkDataMask |= ChunkDataMask_HasCustomData;
				}
				if (blockData.size() > 0)
				{
					dwChunkDataMask |= ChunkDataMask_HasBlockData;
				}
				if (blockMetaData.size() > 0)
				{
					dwChunkDataMask |= ChunkDataMask_HasMaskData;
				}

				// skip this chunk if it is fully empty
				if (dwChunkDataMask == 0)
					continue;

				// write id and data mask
				memFile.WriteDWORD(dwChunkID | dwChunkDataMask);


				// custom data
				if (nCustomDataCount > 0)
				{
					memFile.WriteEncodedUInt(nCustomDataCount);

					// write block id and indices
					for (auto itCur = blockIDToIndex.begin(); itCur != blockIDToIndex.end(); ++itCur)
					{
						uint16_t nBlockID = itCur->first;
						auto& block_indices = itCur->second;
						int nCount = block_indices.size();
						if (nCount > 0)
						{
							if (nCount > 10 && CIntegerEncoder::IsSkipOneBetter(block_indices))
							{
								strBuilder.clear();
								CIntegerEncoder::EncodeSkipOne(strBuilder, block_indices);
								memFile.WriteEncodedUInt(ChunkCustomDataType_Blocks_SkipOne);
								memFile.WriteEncodedUInt(nBlockID);
								memFile.WriteEncodedUInt(strBuilder.size());
								memFile.write(&(strBuilder[0]), strBuilder.size());
							}
							else if (nCount > 1)
							{
								strBuilder.clear();
								CIntegerEncoder::EncodeIntDeltaArray(strBuilder, block_indices);
								memFile.WriteEncodedUInt(ChunkCustomDataType_Blocks_Delta);
								memFile.WriteEncodedUInt(nBlockID);
								memFile.WriteEncodedUInt(strBuilder.size());
								memFile.write(&(strBuilder[0]), strBuilder.size());
							}
							else
							{
								// this is never used
								memFile.WriteEncodedUInt(ChunkCustomDataType_Blocks);
								memFile.WriteEncodedUInt(nBlockID);
								memFile.WriteEncodedUInt(block_indices.size());
								memFile.write(&(block_indices[0]), block_indices.size() * sizeof(uint16_t));
							}
						}
					}

					// write light map
					if (!blockLightMapData.empty())
					{
						memFile.WriteEncodedUInt(ChunkCustomDataType_LightValues);
						memFile.WriteDWORD(blockLightMapData.size());
						memFile.write(&(blockLightMapData[0]), blockLightMapData.size());
					}
				}

				if (blockData.size() > 0 || ((dwChunkDataMask & ChunkDataMask_HasCustomData) == 0))
				{
					//block count
					memFile.WriteDWORD(blockData.size());
					//block id data
					if (blockData.size() > 0)
					{
						std::pair<uint16_t, uint16_t>* pBlockData = &blockData[0];
						memFile.write(pBlockData, blockData.size() * dataItemSize);
					}
				}

				if (blockMetaData.size() > 0)
				{
					//block count
					memFile.WriteDWORD(blockMetaData.size());
					// block meta data
					std::pair<uint16_t, uint16_t>* pBlockData = &blockMetaData[0];
					memFile.write(pBlockData, blockMetaData.size() * dataItemSize);
				}
			}
			auto pStringBuilder = static_cast<StringBuilder*>(memFile.GetHandlePtr());

			// save compressed data
			do
			{
				auto uncompressedSize = pStringBuilder->size();

				// 
				if (uncompressedSize > 1024)
				{
					char* compressedData = new char[uncompressedSize];
					size_t compressedSize = 0;

					z_stream stream;
					stream.next_in = (Bytef*)pStringBuilder->str();
					stream.avail_in = (uInt)uncompressedSize;

					stream.next_out = (Bytef*)compressedData;
					stream.avail_out = (uInt)uncompressedSize;

					stream.zalloc = (alloc_func)0;
					stream.zfree = (free_func)0;
					stream.opaque = (voidpf)0;

					int err = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
					if (err != Z_OK)
					{
						OUTPUT_LOG("could not compress data");
						delete[] compressedData;
						break;
					}

					err = deflate(&stream, Z_FINISH);
					if (err != Z_STREAM_END)
					{
						deflateEnd(&stream);

						OUTPUT_LOG("could not compress data");
						delete[] compressedData;
						break;
					}
					compressedSize = stream.total_out;
					deflateEnd(&stream);

					cfile.WriteDWORD(uncompressedSize); // uncompressedSize
					cfile.WriteDWORD(compressedSize); // compressedSize;
					cfile.write(compressedData, compressedSize);

					delete[] compressedData;
				}
				else // so small, write uncompressed data
				{
					cfile.WriteDWORD(uncompressedSize); // uncompressedSize
					cfile.WriteDWORD(0); // compressedSize;
					cfile.write(pStringBuilder->str(), pStringBuilder->size());
				}


			} while (false);


			cfile.close();
			memFile.close();

			GetBlockWorld()->OnSaveBlockRegion(m_regionX, m_regionZ);
		}
	}

	void BlockRegion::ParserFile(CParaFile* pFile)
	{
		uint32_t regionId = pFile->ReadDWORD();

		uint32_t data_mask = pFile->ReadDWORD();
		uint32_t dataItemSize = data_mask & 0xffff;

		m_pBlockWorld->SuspendLightUpdate();

		if ((data_mask & ChunkDataMask_HasCustomData) > 0)
		{
			// read all custom data until we meet ChunkCustomDataType_ChunksData
			DWORD nCustomDataType;
			while (!pFile->isEof() && ((nCustomDataType = pFile->ReadEncodedUInt()) != ChunkCustomDataType_ChunksData))
			{
				switch (nCustomDataType)
				{
				case ChunkCustomDataType_Biomes:
				case ChunkCustomDataType_Biomes_SameInteger:
					break;
				case ChunkCustomDataType_TimeStamp:
				case ChunkCustomDataType_TimeStamp_SameInteger:
				{
					uint32_t nByteCount = pFile->ReadEncodedUInt();
					m_chunkTimestamp.clear();

					if (nCustomDataType == ChunkCustomDataType_TimeStamp_SameInteger)
						CIntegerEncoder::DecodeSameInteger(*pFile, m_chunkTimestamp, nByteCount);
					else
						CIntegerEncoder::DecodeIntArray(*pFile, m_chunkTimestamp, nByteCount);
					if (m_chunkTimestamp.size() != 32 * 32)
					{
						m_chunkTimestamp.resize(32 * 32);
						OUTPUT_LOG("error:invalid m_chunkTimestamp \n");
						return;
					}
					break;
				}
				case ChunkCustomDataType_Tag:
				{
					uint32_t nByteCount = pFile->ReadEncodedUInt();
					if (nByteCount > 0)
					{
						m_sTag.resize(nByteCount);
						pFile->read(&(m_sTag[0]), nByteCount);
					}
					break;
				}
				case ChunkCustomDataType_Materials:
				{
					uint32_t nByteCount = pFile->ReadEncodedUInt();
					while (nByteCount > 0)
					{
						uint16_t chunkIndex = 0;
						uint32_t materialKeyCount = 0;
						pFile->read(&chunkIndex, sizeof(uint16_t));
						pFile->read(&materialKeyCount, sizeof(uint32_t));
						nByteCount = nByteCount - sizeof(uint16_t) - sizeof(uint32_t);
						auto pChunk = GetChunk(chunkIndex, true);
						for (uint32_t i = 0; i < materialKeyCount; i++)
						{
							uint32_t materialId = 0;
							uint32_t keyCount = 0;
							pFile->read(&materialId, sizeof(uint32_t));
							pFile->read(&keyCount, sizeof(uint32_t));
							std::vector<uint16_t> keys;
							keys.resize(keyCount);
							pFile->read(keys.data(), keyCount * sizeof(uint16_t));
							nByteCount = nByteCount - sizeof(uint32_t) - sizeof(uint32_t) - keyCount * sizeof(uint16_t);
							for (uint32_t j = 0; j < keyCount; j++)
							{	
								uint16_t key = keys[j];
								uint16_t blockIndex = key % 4096;
								uint16_t faceIndex = key / 4096;
								pChunk->ApplyBlockMaterial(blockIndex, (int16_t)faceIndex, materialId);
							}
						}
					}
					break;
				}
				default:
				{
					uint32_t nByteCount = pFile->ReadEncodedUInt();
					pFile->seekRelative(nByteCount);
					OUTPUT_LOG("warn: unknown block region file format in custom data \n");
					break;
				}
				}
			}
		}
		else
		{ // compatible with old formats

		  // timestamp of 32*32 chunk columns. chunk column will be generated if there is no timestamp. 
		  // loading a file without timestamp will be set to default timestamp of 1. 
			std::fill(m_chunkTimestamp.begin(), m_chunkTimestamp.end(), 1);
		}

		std::vector<std::pair<uint16_t, uint16_t>> blockData;
		std::vector<uint16_t> blockIndices;
		std::vector<uint8_t> blockLightMap;

		while (!pFile->isEof())
		{
			uint32_t chunkId_and_dataMask = pFile->ReadDWORD();
			uint32_t chunkId = chunkId_and_dataMask & (0x7fffffff);

			m_nChunksLoaded++;

			uint16_t chunkX, chunkY, chunkZ;
			UnpackChunkIndex(chunkId, chunkX, chunkY, chunkZ);
			blockLightMap.clear();
			uint16_t offset_x = (chunkX << 4);
			uint16_t offset_y = (chunkY << 4);
			uint16_t offset_z = (chunkZ << 4);

			if ((chunkId_and_dataMask & ChunkDataMask_HasCustomData) > 0)
			{
				int nCustomDataCount = pFile->ReadEncodedUInt();
				for (int c = 0; c < nCustomDataCount; ++c)
				{
					DWORD nCustomDataType = pFile->ReadEncodedUInt();
					switch (nCustomDataType)
					{
					case ChunkCustomDataType_LightValues:
					{
						uint32_t blockCount = pFile->ReadDWORD();

						blockLightMap.resize(blockCount);
						pFile->read(&blockLightMap[0], blockCount);
						break;
					}
					case ChunkCustomDataType_Blocks:
					case ChunkCustomDataType_Blocks_SkipOne:
					case ChunkCustomDataType_Blocks_Delta:
					{
						uint16_t nBlockID = (uint16_t)pFile->ReadEncodedUInt();
						uint16_t nBlockCount = (uint16_t)pFile->ReadEncodedUInt();

						if (nCustomDataType == ChunkCustomDataType_Blocks_SkipOne)
						{
							blockIndices.clear();
							CIntegerEncoder::DecodeSkipOne(*pFile, blockIndices, nBlockCount);
							nBlockCount = (uint16_t)blockIndices.size();
						}
						else if (nCustomDataType == ChunkCustomDataType_Blocks_Delta)
						{
							blockIndices.clear();
							CIntegerEncoder::DecodeIntDeltaArray(*pFile, blockIndices, nBlockCount);
							nBlockCount = (uint16_t)blockIndices.size();
						}
						else if (nCustomDataType == ChunkCustomDataType_Blocks)
						{
							blockIndices.resize(nBlockCount);
							pFile->read(&blockIndices[0], nBlockCount * sizeof(uint16_t));
						}
						if (nBlockCount > 0)
						{
							BlockTemplate *pTemplate = m_pBlockWorld->GetBlockTemplate(nBlockID);
							if (pTemplate)
							{
								BlockChunk* pChunk = GetChunk(chunkId, true);
								if (pChunk)
								{
									pChunk->LoadBlocks(blockIndices, pTemplate);
								}
							}
						}
					}
					break;
					case ChunkCustomDataType_Biomes:
					{
						// TODO: 
					}
					break;
					default:
					{
						OUTPUT_LOG("error:unknown block region file format in custom data \n");
						return;
					}
					}
				}
			}

			if ((chunkId_and_dataMask & ChunkDataMask_HasBlockData) > 0 ||
				(chunkId_and_dataMask & ChunkDataMask_HasCustomData) == 0)
			{
				uint32_t blockCount = pFile->ReadDWORD();

				blockData.resize(blockCount);
				if (blockData.size() > 0)
				{
					std::pair<uint16_t, uint16_t> *pData = &blockData[0];
					pFile->read(pData, dataItemSize*blockCount);
				}

				BlockChunk* pChunk = GetChunk(chunkId, true);
				if (pChunk)
				{
					pChunk->ReserveBlocks(pChunk->GetBlockCount() + blockCount);
					for (uint32_t i = 0; i < blockCount; i++)
					{
						std::pair<uint16_t, uint16_t>& curBlockData = blockData[i];
						BlockTemplate *pTemplate = m_pBlockWorld->GetBlockTemplate(curBlockData.second);
						if (pTemplate)
						{
							pChunk->LoadBlock(curBlockData.first, pTemplate);
						}
					}
				}
			}

			if ((chunkId_and_dataMask & ChunkDataMask_HasMaskData) > 0)
			{
				uint32_t blockCount = pFile->ReadDWORD();

				blockData.resize(blockCount);
				std::pair<uint16_t, uint16_t> *pData = &blockData[0];
				pFile->read(pData, dataItemSize*blockCount);

				BlockChunk* pChunk = GetChunk(chunkId);
				if (pChunk)
				{
					for (uint32_t i = 0; i < blockCount; i++)
					{
						std::pair<uint16_t, uint16_t>& curBlockData = blockData[i];
						pChunk->SetBlockData(curBlockData.first, curBlockData.second);
					}
				}
			}
			if (!blockLightMap.empty())
			{
				BlockChunk* pChunk = GetChunk(chunkId, true);
				uint32_t blockCount = blockLightMap.size();
				for (uint32_t i = 0; i < blockCount; i++)
				{
					uint8_t v = blockLightMap[i];
					pChunk->m_lightmapArray[i].LoadBrightness((v & 0xf0) >> 4, v & 0xf);
				}
				m_pBlockWorld->GetLightGrid().SetColumnPreloaded((m_regionX << 5) + chunkX, (m_regionZ << 5) + chunkZ);
			}
		}

		m_nEventAsyncLoadWorldFinished = 1;

		// TODO: initialize sunlight on height map?  
		m_pBlockWorld->ResumeLightUpdate();
	}

	void BlockRegion::ParserFile1_0(CParaFile* pFile)
	{
		size_t uncompressedSize = pFile->ReadDWORD();
		size_t compressedSize = pFile->ReadDWORD();

		if (compressedSize == 0)
		{
			// uncompressed data
			ParserFile(pFile);
		}
		else
		{
			// compressed data
			char* compressedData = new char[compressedSize];
			char* uncompressedData = new char[uncompressedSize];
			auto readSize = pFile->read(compressedData, compressedSize);

			if (readSize != compressedSize)
			{
				OUTPUT_LOG("error: failed to read compressed data from file in BlockRegion parser %d (%d)\n", readSize, compressedSize);
				delete[] compressedData;
				delete[] uncompressedData;
				return;
			}

			z_stream zstream;
			zstream.opaque = Z_NULL;
			zstream.zalloc = Z_NULL;
			zstream.zfree = Z_NULL;
			zstream.data_type = Z_BINARY;

			// http://hewgill.com/journal/entries/349-how-to-decompress-gzip-stream-with-zlib
			inflateInit(&zstream);

			zstream.next_in = reinterpret_cast<Bytef*>(compressedData);
			zstream.avail_in = compressedSize;

			zstream.avail_out = uncompressedSize;
			zstream.next_out = reinterpret_cast<Bytef*>(uncompressedData);

			const int ret = inflate(&zstream, Z_FINISH);

			if (ret != Z_STREAM_END && ret != Z_OK)
			{
				OUTPUT_LOG("error: failed to decompress data in BlockRegion\n");

				delete[] compressedData;
				delete[] uncompressedData;

				return;
			}

			// terminate zlib
			inflateEnd(&zstream);

			CParaFile memFile(uncompressedData, uncompressedSize);

			ParserFile(&memFile);

			delete[] compressedData;
			delete[] uncompressedData;
		}
	}

	void BlockRegion::LoadFromFile(const char* sFilename)
	{
		CParaFile* pFile = nullptr;
		std::string fileName = m_pBlockWorld->GetWorldInfo().GetBlockRegionFileName(m_regionX, m_regionZ, true);
		if (sFilename != 0 && sFilename[0] != '\0')
		{
			fileName = sFilename;
		}

		CParaFile gameSaveFile;
		CParaFile tempFile;
		SetModified(false);
		tempFile.OpenAssetFile(fileName.c_str(), true, ParaTerrain::Settings::GetInstance()->GetMediaPath());
		if (!tempFile.isEof())
			pFile = &tempFile;

		if (!pFile)
		{
			fileName = m_pBlockWorld->GetWorldInfo().GetBlockRegionFileName(m_regionX, m_regionZ, false);
			gameSaveFile.OpenAssetFile(fileName.c_str(), true, ParaTerrain::Settings::GetInstance()->GetMediaPath());
			if (!gameSaveFile.isEof())
				pFile = &gameSaveFile;
		}

		if (pFile == nullptr)
		{
			m_nEventAsyncLoadWorldFinished = 2;
			OUTPUT_LOG("error: failed to load region file %s. It may not exist.\n", fileName.c_str());
			return;
		}

		uint32_t fileTypeId = pFile->ReadDWORD();

		OUTPUT_LOG("load region file %s. version: %x\n", fileName.c_str(), fileTypeId);

		if (fileTypeId == 0x626c6f63)
		{
			ParserFile(pFile);
		}
		else if (fileTypeId == 0x626c6f63 + 1)
		{
			uint16_t version = pFile->ReadWORD();
			switch (version)
			{
			case 0x0100:
				ParserFile1_0(pFile);
				break;
			default:
				break;
			}
		}

		if (m_nEventAsyncLoadWorldFinished == 0)
		{
			m_nEventAsyncLoadWorldFinished = 2;
			OUTPUT_LOG("error: failed to load region file %s. It may be corrupted or version not supported.\n", fileName.c_str());
		}
	}

	void BlockRegion::DeleteAllBlocks()
	{
		while (IsLocked())
		{
			SLEEP(10);
		}
				
		uint32_t nCount = GetChunksCount();
		for (uint32_t i = 0; i < nCount; i++)
		{
			SAFE_DELETE(m_chunks[i]);
		}
		std::fill(m_blockHeightMap.begin(), m_blockHeightMap.end(), ChunkMaxHeight(0, 0));

		std::fill(m_biomes.begin(), m_biomes.end(), 0);

		std::fill(m_chunkTimestamp.begin(), m_chunkTimestamp.end(), 0);

		// clear all light
		for (uint16 cx = m_minChunkId_ws.x; cx < m_maxChunkId_ws.x; cx++)
		{
			for (uint16 cz = m_minChunkId_ws.z; cz < m_maxChunkId_ws.z; cz++)
			{
				if (m_pBlockWorld->GetLightGrid().IsChunkColumnLoaded(cx, cz))
				{
					m_pBlockWorld->GetLightGrid().SetColumnUnloaded(cx, cz);
					m_pBlockWorld->GetLightGrid().AddDirtyColumn(cx, cz);
				}
			}
		}
		//  clear all active chunks
		m_pBlockWorld->ClearBlockRenderCache();
	}

	void BlockRegion::SetChunkColumnTimeStamp(uint16_t x_rs, uint16_t z_rs, uint16_t nTimeStamp)
	{
		uint16 nIndex = PackChunkColumnIndex(x_rs >> 4, z_rs >> 4);
		if (nIndex < m_chunkTimestamp.size())
		{
			int nOldValue = m_chunkTimestamp[nIndex];
			m_chunkTimestamp[nIndex] = (unsigned char)nTimeStamp;

			if (nOldValue == 0 && nTimeStamp > 0)
			{
				m_pBlockWorld->RefreshChunkColumn(m_regionX * 32 + (x_rs >> 4), m_regionZ * 32 + (z_rs >> 4));
			}
		}
		else
		{
			OUTPUT_LOG("error: BlockRegion::SetChunkColumnTimeStamp\n");
			PE_ASSERT(FALSE);
		}
	}


	uint16_t BlockRegion::GetChunkColumnTimeStamp(uint16_t x_rs, uint16_t z_rs)
	{
		uint16 nIndex = PackChunkColumnIndex(x_rs >> 4, z_rs >> 4);
		if (nIndex < m_chunkTimestamp.size())
		{
			return m_chunkTimestamp[nIndex];
		}
		return 0;
	}

	bool BlockRegion::IsChunkDirty(int16_t packedChunkID)
	{
		BlockChunk* pChunk = GetChunk(packedChunkID, false);
		return pChunk && pChunk->IsDirty();
	}

	bool BlockRegion::IsModified()
	{
		return m_bIsModified;
	}

	void BlockRegion::SetModified(bool bModified)
	{
		m_bIsModified = bModified;
	}

	CBlockWorld* BlockRegion::GetBlockWorld()
	{
		return m_pBlockWorld;
	}

	const std::string& BlockRegion::GetMapChunkData(uint32_t chunkX_ws, uint32_t chunkZ_ws, bool bIncludeInit, uint32_t verticalSectionFilter)
	{
		uint16_t chunkX_rs = chunkX_ws & 0x1f;
		uint16_t chunkZ_rs = chunkZ_ws & 0x1f;

		static StringBuilder outputStream;
		outputStream.clear();
		// append version format
		outputStream.append("chunkV1");
		uint32_t nChunkSize = 0;
		int nChunkSizeLocation = outputStream.size();
		outputStream.appendBinary((uint32)nChunkSize);

		CSameIntegerEncoder<uint16_t> blockIdEncoder(&outputStream);
		CSameIntegerEncoder<uint32_t> blockDataEncoder(&outputStream);
		for (uint16_t y = 0; y < 16; y++)
		{
			if ((verticalSectionFilter & (1 << y)) != 0)
			{
				outputStream.appendBinary((uint32)y);
				int32 nBlockCount = 0;
				int nBlockCountIndex = outputStream.size();
				outputStream.appendBinary((uint32)nBlockCount);
				uint16_t startBlockIdY_ws = y << 4;
				uint16_t packedChunkId_rs = PackChunkIndex(chunkX_rs, y, chunkZ_rs);
				BlockChunk * pChunk = m_chunks[packedChunkId_rs];
				if (pChunk)
				{
					blockIdEncoder.Reset();
					uint32_t nCount = pChunk->m_blockIndices.size();
					for (uint32_t i = 0; i < nCount; i++)
					{
						int32_t blockIdx = pChunk->m_blockIndices[i];
						if (blockIdx >= 0)
						{
							Block& curBlock = pChunk->GetBlockByIndex(blockIdx);
							blockIdEncoder.Append(curBlock.GetTemplateId());
							nBlockCount++;
						}
						else
						{
							blockIdEncoder.Append(0);
						}
					}
					blockIdEncoder.Finalize();

					blockDataEncoder.Reset();
					for (uint32_t i = 0; i < nCount; i++)
					{
						int32_t blockIdx = pChunk->m_blockIndices[i];
						if (blockIdx >= 0)
						{
							Block& curBlock = pChunk->GetBlockByIndex(blockIdx);
							blockDataEncoder.Append(curBlock.GetUserData());
						}
						else
						{
							blockDataEncoder.Append(0);
						}
					}
					blockDataEncoder.Finalize();
				}
				else
				{
					blockIdEncoder.Reset();
					blockIdEncoder.Append(0, 4096);
					blockIdEncoder.Finalize();
					blockDataEncoder.Reset();
					blockDataEncoder.Append(0, 4096);
					blockDataEncoder.Finalize();
				}
				outputStream.WriteAt(nBlockCountIndex, nBlockCount);
			}
		}
		outputStream.WriteAt(nChunkSizeLocation, outputStream.length() - nChunkSizeLocation - 4);
		static std::string g_str;
		g_str.assign(outputStream.c_str(), outputStream.length());
		return g_str;
	}

	void BlockRegion::ApplyMapChunkData(uint32_t chunkX, uint32_t chunkZ, uint32_t verticalSectionFilter, const std::string& chunkData, const luabind::adl::object& output)
	{
		uint16_t chunkX_rs = chunkX & 0x1f;
		uint16_t chunkZ_rs = chunkZ & 0x1f;

		bool IsChunkColumnFirstLoaded = GetChunkColumnTimeStamp(chunkX_rs << 4, chunkZ_rs << 4) <= 1;
		CParaFile file((char*)(chunkData.c_str()), chunkData.size(), false);
		char sVersion[8];
		file.read(sVersion, 7);
		sVersion[7] = '\0';
		int nModifiedCount = 0;
		bool bLightSuspended = false;
		if (strcmp(sVersion, "chunkV1") == 0)
		{
			static std::vector<uint16_t> sBlockId(4096);
			static std::vector<uint32_t> sBlockData(4096);

			lua_State* L = output.interpreter();
			luabind::adl::object removeQueue = luabind::newtable(L);
			output["remove"] = removeQueue;
			luabind::adl::object addQueue = luabind::newtable(L);
			output["add"] = addQueue;
			luabind::adl::object addDataQueue = luabind::newtable(L);
			output["addData"] = addDataQueue;
			luabind::adl::object modDataQueue = luabind::newtable(L);
			output["modData"] = modDataQueue;

			if (IsChunkColumnFirstLoaded)
			{
				if (!GetBlockWorld()->IsLightUpdateSuspended())
				{
					GetBlockWorld()->SuspendLightUpdate();
					bLightSuspended = true;
				}
			}

			int nChunkSize = (int)file.ReadDWORD();
			while (!file.isEof())
			{
				uint32_t chunkY_rs = (uint32_t)file.ReadDWORD();
				if (chunkY_rs < 16)
				{
					uint32_t nBlockCount = (uint32_t)file.ReadDWORD();

					if (nBlockCount > 0)
					{
						assert(nBlockCount <= 4096);
						sBlockId.clear();
						sBlockData.clear();
						CSameIntegerDecoder<uint16_t>::DecodeSameIntegerOfCount(file, sBlockId, 4096);
						CSameIntegerDecoder<uint32_t>::DecodeSameIntegerOfCount(file, sBlockData, 4096);
						assert(sBlockId.size() == 4096 && sBlockData.size() == 4096);

						uint16_t packedChunkId_rs = PackChunkIndex(chunkX_rs, chunkY_rs, chunkZ_rs);
						BlockChunk * pChunk = GetChunk(packedChunkId_rs, false);
						if (pChunk == NULL)
						{
							pChunk = GetChunk(packedChunkId_rs, true);
						}
						if (IsChunkColumnFirstLoaded && pChunk)
						{
							uint32_t nCount = pChunk->m_blockIndices.size();
							pChunk->ReserveBlocks(nBlockCount);
							for (uint32_t i = 0; i < nCount; i++)
							{
								if (sBlockId[i] > 0)
								{
									BlockTemplate* pTemplate = GetBlockWorld()->GetBlockTemplate(sBlockId[i]);
									pChunk->LoadBlock(i, pTemplate);
									pChunk->SetBlockData(i, sBlockData[i]);
									nModifiedCount++;
								}
							}
						}
						else if (pChunk)
						{
							uint32_t nCount = pChunk->m_blockIndices.size();
							uint16_t chunkX_ws = (chunkX_rs << 4);
							uint16_t chunkY_ws = (chunkY_rs << 4);
							uint16_t chunkZ_ws = (chunkZ_rs << 4);
							for (uint32_t i = 0; i < nCount; i++)
							{
								int32_t blockIdx = pChunk->m_blockIndices[i];
								if (sBlockId[i] == 0)
								{
									if (blockIdx >= 0)
									{
										// delete old block
										Block& curBlock = pChunk->GetBlockByIndex(blockIdx);
										if (curBlock.GetTemplate() && curBlock.GetTemplate()->IsMatchAttribute(BlockTemplate::batt_onload))
										{
											removeQueue[i * 16 + chunkY_rs] = curBlock.GetTemplateId();
										}

										uint16_t blockX, blockY, blockZ;
										UnpackBlockIndex(i, blockX, blockY, blockZ);
										uint16_t regionBlockX = chunkX_ws + blockX;
										uint16_t regionBlockY = chunkY_ws + blockY;
										uint16_t regionBlockZ = chunkZ_ws + blockZ;
										SetBlockTemplateByIndex(regionBlockX, regionBlockY, regionBlockZ, NULL);
										nModifiedCount++;
									}
								}
								else
								{
									if (blockIdx >= 0)
									{
										Block& curBlock = pChunk->GetBlockByIndex(blockIdx);
										bool bSameBlockId = curBlock.GetTemplateId() == sBlockId[i];
										bool bSameBlockData = curBlock.GetUserData() == sBlockData[i];
										if (!bSameBlockId || !bSameBlockData)
										{
											nModifiedCount++;
											BlockTemplate* pTemplate = GetBlockWorld()->GetBlockTemplate(sBlockId[i]);
											if (pTemplate)
											{
												uint16_t blockX, blockY, blockZ;
												UnpackBlockIndex(i, blockX, blockY, blockZ);
												uint16_t regionBlockX = chunkX_ws + blockX;
												uint16_t regionBlockY = chunkY_ws + blockY;
												uint16_t regionBlockZ = chunkZ_ws + blockZ;

												if (!bSameBlockId)
												{
													if (curBlock.GetTemplate())
													{
														if (curBlock.GetTemplate()->IsMatchAttribute(BlockTemplate::batt_onload))
														{
															removeQueue[i * 16 + chunkY_rs] = curBlock.GetTemplateId();
															if (pTemplate->IsMatchAttribute(BlockTemplate::batt_onload))
															{
																addQueue[i * 16 + chunkY_rs] = sBlockId[i];
																if (sBlockData[i] != 0)
																	addDataQueue[i * 16 + chunkY_rs] = sBlockData[i];
															}
														}
													}
													else
													{
														OUTPUT_LOG("fatal error: apply chunk to invalid index\n");
													}
													SetBlockTemplateByIndex(regionBlockX, regionBlockY, regionBlockZ, pTemplate);
													SetBlockUserDataByIndex(regionBlockX, regionBlockY, regionBlockZ, sBlockData[i]);
												}
												else
												{
													if (!bSameBlockData)
													{
														modDataQueue[i * 16 + chunkY_rs] = sBlockData[i];
														SetBlockUserDataByIndex(regionBlockX, regionBlockY, regionBlockZ, sBlockData[i]);
													}
												}
											}
										}
									}
									else
									{
										// create the new block at given index
										BlockTemplate* pTemplate = GetBlockWorld()->GetBlockTemplate(sBlockId[i]);
										if (pTemplate)
										{
											nModifiedCount++;
											if (pTemplate->IsMatchAttribute(BlockTemplate::batt_onload))
											{
												addQueue[i * 16 + chunkY_rs] = sBlockId[i];
												if (sBlockData[i] != 0)
												{
													addDataQueue[i * 16 + chunkY_rs] = sBlockData[i];
												}
											}
											uint16_t blockX, blockY, blockZ;
											UnpackBlockIndex(i, blockX, blockY, blockZ);
											uint16_t regionBlockX = chunkX_ws + blockX;
											uint16_t regionBlockY = chunkY_ws + blockY;
											uint16_t regionBlockZ = chunkZ_ws + blockZ;
											SetBlockTemplateByIndex(regionBlockX, regionBlockY, regionBlockZ, pTemplate);
											if (sBlockData[i] != 0)
											{
												SetBlockUserDataByIndex(regionBlockX, regionBlockY, regionBlockZ, sBlockData[i]);
											}
										}
									}
								}
							}
						}
						else
						{
							OUTPUT_LOG("error: ApplyMapChunkData failed because chunk does not exist.\n");
						}
					}
					else
					{
						CSameIntegerDecoder<uint16_t>::SkipDecodeSameIntegerOfCount(file, 4096);
						CSameIntegerDecoder<uint32_t>::SkipDecodeSameIntegerOfCount(file, 4096);

						uint16_t packedChunkId_rs = PackChunkIndex(chunkX_rs, chunkY_rs, chunkZ_rs);
						BlockChunk * pChunk = GetChunk(packedChunkId_rs, false);
						if (pChunk)
						{
							// clear this chunk if has any data
							uint32_t nCount = pChunk->m_blockIndices.size();
							uint16_t chunkX_ws = (chunkX_rs << 4);
							uint16_t chunkY_ws = (chunkY_rs << 4);
							uint16_t chunkZ_ws = (chunkZ_rs << 4);
							for (uint32_t i = 0; i < nCount; i++)
							{
								int32_t blockIdx = pChunk->m_blockIndices[i];
								if (blockIdx >= 0)
								{
									// delete old block
									nModifiedCount++;
									Block& curBlock = pChunk->GetBlockByIndex(blockIdx);
									if (curBlock.GetTemplate() && curBlock.GetTemplate()->IsMatchAttribute(BlockTemplate::batt_onload))
									{
										removeQueue[i * 16 + chunkY_rs] = curBlock.GetTemplateId();
									}
									uint16_t blockX, blockY, blockZ;
									UnpackBlockIndex(i, blockX, blockY, blockZ);
									uint16_t regionBlockX = chunkX_ws + blockX;
									uint16_t regionBlockY = chunkY_ws + blockY;
									uint16_t regionBlockZ = chunkZ_ws + blockZ;
									SetBlockTemplateByIndex(regionBlockX, regionBlockY, regionBlockZ, NULL);
								}
							}
						}
					}
				}
			}
		}
		if (IsChunkColumnFirstLoaded && nModifiedCount > 0)
		{
			SetChunkColumnTimeStamp(chunkX_rs << 4, chunkZ_rs << 4, 2);
			if (!m_pBlockWorld->RefreshChunkColumn(m_minChunkId_ws.x + chunkX_rs, m_minChunkId_ws.z + chunkZ_rs))
				m_pBlockWorld->GetLightGrid().SetColumnUnloaded(m_minChunkId_ws.x + chunkX_rs, m_minChunkId_ws.z + chunkZ_rs);
			/* NOT needed
			// refresh nearby chunk column if any, this fix a bug of lighting
			for (int i = -1; i <= 1; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					if (i != 0 && j != 0){
						if(!m_pBlockWorld->RefreshChunkColumn(m_minChunkId_ws.x + chunkX_rs + i, m_minChunkId_ws.z + chunkZ_rs + j))
							m_pBlockWorld->GetLightGrid().SetColumnUnloaded(m_minChunkId_ws.x + chunkX_rs + i, m_minChunkId_ws.z + chunkZ_rs + j);
					}
				}
			}*/
		}
		if (bLightSuspended)
			GetBlockWorld()->ResumeLightUpdate();
	}

	void BlockRegion::Load()
	{
		if (GetBlockWorld() && !IsLocked())
		{
			if (GetBlockWorld()->OnBeforeLoadBlockRegion(GetRegionX(), GetRegionZ()) != 0)
				return;

			if (!GetBlockWorld()->IsRemote())
			{
				// it is important that server mode always use sync mode or there may be empty chunks on client side. 
				if (GetBlockWorld()->IsUseAsyncLoadWorld() && !GetBlockWorld()->IsServerWorld())
				{
					OUTPUT_LOG("Block loading region %d %d in Async mode\n", m_regionX, m_regionZ);
					m_bIsLocked = true;
					m_thread = std::thread(std::bind(&BlockRegion::LoadWorldThreadFunc, this));
				}
				else
				{
					OUTPUT_LOG("Block loading region %d %d in sync mode\n", m_regionX, m_regionZ);
					Scoped_WriteLock<BlockReadWriteLock> lock_(m_pBlockWorld->GetReadWriteLock());
					LoadFromFile();
					OnLoadWorldFinished();
				}
			}
			else
			{
				// for remote world, we will disable time stamp loading. 
				for (uint16 cx = 0; cx < 512; cx += 16)
				{
					for (uint16 cz = 0; cz < 512; cz += 16)
					{
						// SetChunkColumnTimeStamp(cx, cz, 1);
						SetChunkColumnTimeStamp(cx, cz, 0);
					}
				}
				GetBlockWorld()->OnLoadBlockRegion(GetRegionX(), GetRegionZ());
			}
		}
	}

	void BlockRegion::LoadWorldThreadFunc()
	{
		LoadFromFile();
	}

	void BlockRegion::OnFrameMove()
	{
		if (m_nEventAsyncLoadWorldFinished != 0)
		{
			OnLoadWorldFinished();
		}
	}

	void BlockRegion::SetChunksLoaded(uint32 val)
	{
		m_nChunksLoaded = val;
	}

	void BlockRegion::ClearAllLight()
	{
		uint32_t nCount = GetChunksCount();
		for (uint32_t i = 0; i < nCount; i++)
		{
			BlockChunk * pChunk = m_chunks[i];
			if (pChunk)
			{
				pChunk->ClearAllLight();
			}
		}
		for (uint16 cx = m_minChunkId_ws.x; cx < m_maxChunkId_ws.x; cx++)
		{
			for (uint16 cz = m_minChunkId_ws.z; cz < m_maxChunkId_ws.z; cz++)
			{
				if (m_pBlockWorld->GetLightGrid().IsChunkColumnLoaded(cx, cz))
				{
					m_pBlockWorld->GetLightGrid().SetColumnUnloaded(cx, cz);
					m_pBlockWorld->GetLightGrid().AddDirtyColumn(cx, cz);
				}
			}
		}
	}

	void BlockRegion::RefreshLightChunkColumns(uint16_t chunkX_ws, uint16_t chunkZ_ws, uint16_t chunksLength /*= 1*/)
	{
		for (uint16 x = 0; x < chunksLength; x++)
		{
			for (uint16 z = 0; z < chunksLength; z++)
			{
				uint16 rx = chunkX_ws - m_minChunkId_ws.x + x;
				uint16 rz = chunkZ_ws - m_minChunkId_ws.z + z;
				if (rx < 32 && rz < 32)
				{
					for (uint16 ry = 0; ry < 16; ry++)
					{
						uint16_t nIndex = PackBlockIndex(rx, ry, rz);
						BlockChunk * pChunk = m_chunks[nIndex];
						if (pChunk)
						{
							pChunk->ClearLightMap();
						}
					}
					uint16_t cx = chunkX_ws + x;
					uint16_t cz = chunkZ_ws + z;
					if (m_pBlockWorld->GetLightGrid().IsChunkColumnLoaded(cx, cz))
					{
						m_pBlockWorld->GetLightGrid().SetColumnUnloaded(cx, cz);
						m_pBlockWorld->GetLightGrid().AddDirtyColumn(cx, cz);
					}
				}
			}
		}
	}

	uint32 BlockRegion::GetChunksLoaded() const
	{
		return m_nChunksLoaded;
	}

	void BlockRegion::OnLoadWorldFinished()
	{
		if (m_nEventAsyncLoadWorldFinished != 0)
		{
			Scoped_WriteLock<BlockReadWriteLock> lock_(m_pBlockWorld->GetReadWriteLock());

			// unlock this region, since it is now fully loaded. 
			m_bIsLocked = false;

			m_pBlockWorld->SuspendLightUpdate();

			if (m_nEventAsyncLoadWorldFinished == 1)
			{
				m_nEventAsyncLoadWorldFinished = 0;
				int nSize = m_chunkTimestamp.size();
				for (int i = 0; i < nSize; ++i)
				{
					if (m_chunkTimestamp[i] == 0)
					{
						uint16_t z = i >> 5;
						uint16_t x = i - (z << 5);
						m_pBlockWorld->OnGenerateTerrain(m_regionX, m_regionZ, x, z);
					}
				}
			}
			else if (m_nEventAsyncLoadWorldFinished == 2)
			{
				m_nEventAsyncLoadWorldFinished = 0;
				// region file not created before, we will need to call full generator
				m_pBlockWorld->OnGenerateTerrain(m_regionX, m_regionZ);
			}
			m_nEventAsyncLoadWorldFinished = 0;

			m_pBlockWorld->ResumeLightUpdate();

			Int16x3 minChunkId = m_minChunkId_ws;
			Int16x3 maxChunkId = m_maxChunkId_ws;
			bool bHasDirtyColumns = false;
			for (uint16 cx = m_minChunkId_ws.x; cx < m_maxChunkId_ws.x; cx++)
			{
				for (uint16 cz = m_minChunkId_ws.z; cz < m_maxChunkId_ws.z; cz++)
				{
					if (m_pBlockWorld->IsChunkColumnInActiveRange(cx, cz))
					{
						m_pBlockWorld->GetLightGrid().AddDirtyColumn(cx, cz);
						bHasDirtyColumns = true;
					}
				}
			}
			if (bHasDirtyColumns)
			{
				m_pBlockWorld->SetVisibleChunkDirty(true);
			}
			m_pBlockWorld->OnLoadBlockRegion(GetRegionX(), GetRegionZ());
		}
	}

	void BlockRegion::OnUnloadWorld()
	{
		for (uint16 cx = m_minChunkId_ws.x; cx < m_maxChunkId_ws.x; cx++)
		{
			for (uint16 cz = m_minChunkId_ws.z; cz < m_maxChunkId_ws.z; cz++)
			{
				m_pBlockWorld->GetLightGrid().SetColumnUnloaded(cx, cz);
			}
		}
	}

	void BlockRegion::GetCenterBlockWs(Uint16x3* pPos)
	{
		*pPos = m_minBlockId_ws;
		pPos->x += BlockConfig::g_regionBlockDimX / 2;
		pPos->z += BlockConfig::g_regionBlockDimZ / 2;
	}

	const Uint16x3& BlockRegion::GetMinBlockWs()
	{
		return m_minBlockId_ws;
	}

	const Uint16x3& BlockRegion::GetMaxBlockWs()
	{
		return m_maxBlockId_ws;
	}


	const std::string& BlockRegion::GetIdentifier()
	{
		return m_sName;
	}

	void BlockRegion::SetIdentifier(const std::string& sID)
	{
		m_sName = sID;
	}

	int BlockRegion::GetTotalBytes()
	{
		if (m_nTotalBytes == 0)
			CalculateTotalBytes();
		return m_nTotalBytes;
	}


	void BlockRegion::CalculateTotalBytes()
	{
		int nBytes = 0;
		uint32_t nCount = GetChunksCount();
		for (uint32_t i = 0; i < nCount; i++)
		{
			BlockChunk * pChunk = m_chunks[i];
			if (pChunk)
			{
				nBytes += pChunk->GetTotalBytes();
			}
		}
		m_nTotalBytes = nBytes
			+ sizeof(BlockRegion)
			+ sizeof(BlockChunkPtr) * GetChunksCount()
			+ sizeof(unsigned char) * m_chunkTimestamp.size()
			+ sizeof(unsigned char) * m_biomes.size()
			+ sizeof(ChunkMaxHeight) * m_blockHeightMap.size()
			;
	}


	bool BlockRegion::IsLocked()
	{
		return m_bIsLocked;
	}

	void BlockRegion::SetLocked(bool bLocked)
	{
		if (bLocked)
		{
			BeginWrite();
			m_bIsLocked = bLocked;
		}
		else
		{
			m_bIsLocked = bLocked;
			EndWrite();
		}
	}

	void BlockRegion::BeginRead()
	{
		m_readWriteLock.BeginRead();
	}

	void BlockRegion::EndRead()
	{
		m_readWriteLock.EndRead();
	}

	void BlockRegion::BeginWrite()
	{
		m_readWriteLock.BeginWrite();
	}

	void BlockRegion::EndWrite()
	{
		m_readWriteLock.EndWrite();
	}

	ParaEngine::BlockReadWriteLock& BlockRegion::GetReadWriteLock()
	{
		return m_readWriteLock;
	}

	const std::string& BlockRegion::GetTag()
	{
		return m_sTag;
	}

	void BlockRegion::SetTag(const std::string& sTag)
	{
		m_sTag = sTag;
	}

	int BlockRegion::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		IAttributeFields::InstallFields(pClass, bOverride);
		pClass->AddField("RegionX", FieldType_Int, (void*)0, (void*)GetRegionX_s, NULL, NULL, bOverride);
		pClass->AddField("RegionZ", FieldType_Int, (void*)0, (void*)GetRegionZ_s, NULL, NULL, bOverride);
		pClass->AddField("ChunksLoaded", FieldType_Int, (void*)SetChunksLoaded_s, (void*)GetChunksLoaded_s, NULL, NULL, bOverride);
		pClass->AddField("TotalBytes", FieldType_Int, (void*)0, (void*)GetTotalBytes_s, NULL, NULL, bOverride);
		pClass->AddField("IsModified", FieldType_Bool, (void*)SetModified_s, (void*)IsModified_s, NULL, NULL, bOverride);
		pClass->AddField("SaveToFile", FieldType_String, (void*)SaveToFile_s, (void*)0, NULL, NULL, bOverride);
		pClass->AddField("LoadFromFile", FieldType_String, (void*)LoadFromFile_s, (void*)0, NULL, NULL, bOverride);
		pClass->AddField("Tag", FieldType_String, (void*)SetTag_s, (void*)GetTag_s, NULL, NULL, bOverride);
		pClass->AddField("DeleteAllBlocks", FieldType_void, (void*)DeleteAllBlocks_s, NULL, NULL, "", bOverride);
		pClass->AddField("IsLocked", FieldType_Bool, (void*)SetLocked_s, (void*)IsLocked_s, NULL, NULL, bOverride);
		pClass->AddField("ClearAllLight", FieldType_void, (void*)ClearAllLight_s, NULL, NULL, "", bOverride);
		pClass->AddField("RefreshLightChunkColumns", FieldType_Vector3, (void*)RefreshLightChunkColumns_s, (void*)0, NULL, NULL, bOverride);

		return S_OK;
	}
}