#pragma once

#include "BlockModelProvider.h"

namespace ParaEngine
{

	/** for horizontal wires */
	class CWireModelProvider : public IBlockModelProvider
	{
	public:
		CWireModelProvider(BlockTemplate* pBlockTemplate);;
		virtual ~CWireModelProvider(){};

		virtual BlockModel& GetBlockModel(int nIndex = 0);
		virtual BlockModel& GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks = NULL);
	private:
		BlockModel m_block_model;
		BlockModel m_block_model_cube;
	};

}