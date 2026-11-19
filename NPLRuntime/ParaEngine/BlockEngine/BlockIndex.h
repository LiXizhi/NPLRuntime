#pragma once
#include "BlockCoordinate.h"
#include "BlockConfig.h"

namespace ParaEngine
{
	class BlockChunk;
	struct BlockIndex
	{
	public:
		BlockIndex(BlockChunk* pChunk, uint16 nChunkBlockIndex) :m_pChunk(pChunk), m_nChunkBlockIndex(nChunkBlockIndex){};
		BlockChunk* m_pChunk;
		uint16 m_nChunkBlockIndex;
	};

	//////////////////////////////////////////////////////////////////////////
	//
	// Helper inline functions for block index 
	//
	//////////////////////////////////////////////////////////////////////////

	//pack region space chunk id to a single value
	inline uint32_t PackChunkIndex(uint16 rx, int16_t ry, uint16 rz)
	{
		return rx + (rz << 5) + (uint32_t(uint16_t(ry)) << 10);
	}

	//parse packed chunk id to region space chunk id
	inline void UnpackChunkIndex(uint32_t index, uint16& ox, int16_t& oy, uint16& oz)
	{
		oy = static_cast<int16_t>(index >> 10);
		index &= 0x3ff;
		oz = index >> 5;
		ox = index & 0x1f;
	}

	//calc packed block id, parameters are in region space
	inline uint16 CalcPackedBlockID(uint16 rx, int16_t ry, uint16 rz)
	{
		uint16 cx = rx & 0xf;
		uint16 cy = ry & 0xf;
		uint16 cz = rz & 0xf;
		return cx + (cz << 4) + (cy << 8);
	}

	inline uint16 CalcPackedBlockID(Uint16x3& blockID_r)
	{
		uint16 cx = blockID_r.x & 0xf;
		uint16 cy = blockID_r.y & 0xf;
		uint16 cz = blockID_r.z & 0xf;
		return cx + (cz << 4) + (cy << 8);
	}

	//pack block position to index, parameters are in chunk space
	inline uint16 PackBlockId(uint16 cx, uint16 cy, uint16 cz)
	{
		return cx + (cz << 4) + (cy << 8);
	}

	//pack block position to index, parameters are in chunk space
	inline uint16_t PackBlockIndex(uint16_t cx, uint16_t cy, uint16_t cz)
	{
		return cx + (cz << 4) + (cy << 8);
	}

	//unpack block index ,parameters are in chunk space
	template<typename LocalY>
	inline void UnpackBlockIndex(uint16 index, uint16& cx, LocalY& cy, uint16& cz)
	{
		cy = index >> 8;
		index &= 0xff;
		cz = index >> 4;
		cx = index & 0xf;
	}

	inline uint32_t CalcPackedChunkID(uint16 blockRX, int16_t blockRY, uint16 blockRZ)
	{
		uint16 chunkIndexX = blockRX >> 4;
		int16_t chunkIndexY = BlockConfig::SectionY(blockRY);
		uint16 chunkIndexZ = blockRZ >> 4;
		return PackChunkIndex(chunkIndexX, chunkIndexY, chunkIndexZ);
	}

	inline uint32_t CalcPackedChunkID(Uint16x3& blockId_r)
	{
		return CalcPackedChunkID(blockId_r.x, blockId_r.y, blockId_r.z);
	}

	//pack region space chunk id to a single value
	inline uint32_t PackChunkIndex(Uint16x3& chunkId_r)
	{
		return PackChunkIndex(chunkId_r.x, chunkId_r.y, chunkId_r.z);
	}

	// pack chunk column in x,z plane
	inline uint16 PackChunkColumnIndex(uint16 x, uint16 z)
	{
		return x + (z << 5);
	}
}