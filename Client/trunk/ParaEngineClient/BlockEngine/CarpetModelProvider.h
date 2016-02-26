#pragma once

#include "BlockModelProvider.h"

namespace ParaEngine
{

	/** a thin layer wrapping blocks */
	class CCarpetModelProvider : public CLinearModelProvider
	{
	public:
		CCarpetModelProvider(BlockTemplate* pBlockTemplate);
		virtual ~CCarpetModelProvider();

		/** get the default block model.
		* @param nIndex: model index default to 0
		*/
		virtual BlockModel& GetBlockModel(int nIndex = 0);
		virtual BlockModel& GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks = NULL);
	public:
		BlockModel m_block_models[22];
	};

}