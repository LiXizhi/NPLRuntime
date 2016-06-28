#pragma once
#include "BlockModelProvider.h"
namespace ParaEngine
{
	class CSlopeModelProvider
		:public CLinearModelProvider
	{
	protected:
		BlockModel mEdgeBlockModels[8];
		BlockModel mCornerBlockModels[8];
		BlockModel mCubeBlockModel;
	public:
		CSlopeModelProvider(BlockTemplate* pBlockTemplate);
		virtual ~CSlopeModelProvider();
		virtual BlockModel& GetBlockModel(int nIndex = 0);
		virtual BlockModel& GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks = NULL);
	};
}