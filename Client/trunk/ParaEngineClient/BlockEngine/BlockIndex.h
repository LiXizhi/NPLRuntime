#pragma once
#include "BlockCoordinate.h"

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
	inline uint16 PackChunkIndex(uint16 rx, uint16 ry, uint16 rz)
	{
		return rx + (rz << 5) + (ry << 10);
	}

	//parse packed chunk id to region space chunk id
	inline void UnpackChunkIndex(uint16 index, uint16& ox, uint16& oy, uint16& oz)
	{
		oy = index >> 10;
		index &= 0x3ff;
		oz = index >> 5;
		ox = index & 0x1f;
	}

	//calc packed block id, parameters are in region space
	inline uint16 CalcPackedBlockID(uint16 rx, uint16 ry, uint16 rz)
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
	inline void UnpackBlockIndex(uint16 index, uint16& cx, uint16& cy, uint16& cz)
	{
		cy = index >> 8;
		index &= 0xff;
		cz = index >> 4;
		cx = index & 0xf;
	}

	inline uint16 CalcPackedChunkID(uint16 blockRX, uint16 blockRY, uint16 blockRZ)
	{
		uint16 chunkIndexX = blockRX >> 4;
		uint16 chunkIndexY = blockRY >> 4;
		uint16 chunkIndexZ = blockRZ >> 4;
		return chunkIndexX + (chunkIndexZ << 5) + (chunkIndexY << 10);
	}

	inline uint16 CalcPackedChunkID(Uint16x3& blockId_r)
	{
		return (blockId_r.x >> 4) + ((blockId_r.z >> 4) << 5) + ((blockId_r.y >> 4) << 10);
	}

	//pack region space chunk id to a single value
	inline uint16 PackChunkIndex(Uint16x3& chunkId_r)
	{
		return chunkId_r.x + (chunkId_r.z << 5) + (chunkId_r.y << 10);
	}

	// pack chunk column in x,z plane
	inline uint16 PackChunkColumnIndex(uint16 x, uint16 z)
	{
		return x + (z << 5);
	}
}