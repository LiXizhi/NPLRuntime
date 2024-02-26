#pragma once
#include "BlockModelProvider.h"
namespace ParaEngine
{
	class CStairModelProvider
		:public CLinearModelProvider
	{
	protected:
		BlockModel mBlockModels[28];
		
	public:
		CStairModelProvider(BlockTemplate* pBlockTemplate);
		virtual ~CStairModelProvider();
		virtual BlockModel& GetBlockModel(int nIndex = 0);
		virtual BlockModel& GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks = NULL);
	protected:
		void _buildBlockModels();
		

		void cloneAndRotateModels(BlockModel &tempModel, Vector3 *angleArr, BlockModel *outModels, int len, int startOutIdx = 0);

		
		Vector3 calculateModelNormalOfFace(BlockModel &tempModel, int startIdxOfFace);
	};
}