

#pragma once
#include <stdint.h>

namespace ParaEngine
{
	struct BlockConfig
	{
		static const int16_t g_regionBlockDimX;
		static const int16_t g_regionBlockDimY;
		static const int16_t g_regionBlockDimZ;

		static const int16_t g_chunkBlockDim;
		static const int16_t g_chunkBlockCount;

		static const int16_t g_regionChunkDimX;
		static const int16_t g_regionChunkDimY;
		static const int16_t g_regionChunkDimZ;
		static const int16_t g_regionChunkCount;

		static const int32_t g_maxFaceCountPerBatch;

		static const float g_regionSize;
		static const float g_blockSize;
		static const float g_half_blockSize;
		static const double g_dBlockSize;
		static const double g_dBlockSizeInverse;
		static const float g_chunkSize;
		

		static const uint8_t g_maxValidLightValue;
		static const uint8_t g_sunLightValue;
		static const uint8_t g_maxLightValue;
	};
}