//-----------------------------------------------------------------------------
// Class:	config
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.2.7
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockConfig.h"

namespace ParaEngine
{
	const int16_t BlockConfig::g_regionBlockDimX = 512;
	const int16_t BlockConfig::g_regionBlockDimY = 256;
	const int16_t BlockConfig::g_regionBlockDimZ = 512;

	const int16_t BlockConfig::g_chunkBlockDim = 16;
	const int16_t BlockConfig::g_chunkBlockCount = 16 * 16 * 16;

	const int16_t BlockConfig::g_regionChunkDimX = g_regionBlockDimX / g_chunkBlockDim; // 32
	const int16_t BlockConfig::g_regionChunkDimY = g_regionBlockDimY / g_chunkBlockDim; // 16
	const int16_t BlockConfig::g_regionChunkDimZ = g_regionBlockDimZ / g_chunkBlockDim; // 32
	const int16_t BlockConfig::g_regionChunkCount = g_regionChunkDimX * g_regionChunkDimY * g_regionChunkDimZ;

	// make this bigger, but no bigger than (65536/6) = 10920, should be multiple of 6.
	// because we need to reference with 16 bits index in a shared index buffer. (2*3 vertices per face)
	const int32_t BlockConfig::g_maxFaceCountPerBatch = 9000;
	//const int32_t BlockConfig::g_maxFaceCountPerBatch = 6; // just for testing 

	const uint8_t BlockConfig::g_maxValidLightValue = 127;
	const uint8_t BlockConfig::g_sunLightValue = 0xf;
	const uint8_t BlockConfig::g_maxLightValue = 0xf;

	const float BlockConfig::g_regionSize = 533.3333f;
	const float BlockConfig::g_chunkSize = BlockConfig::g_regionSize * (1.0f / BlockConfig::g_regionChunkDimX);
	const double BlockConfig::g_dBlockSize = (double)(BlockConfig::g_regionSize) * (1.0 / BlockConfig::g_regionBlockDimX);
	const double BlockConfig::g_dBlockSizeInverse = 1.0 / BlockConfig::g_dBlockSize;
	const float BlockConfig::g_blockSize = (float)(BlockConfig::g_dBlockSize);
	const float BlockConfig::g_half_blockSize = g_blockSize*0.5f;
}