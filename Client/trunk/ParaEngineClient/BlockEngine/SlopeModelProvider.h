#pragma once
#include "BlockModelProvider.h"
namespace ParaEngine
{
	class CSlopeModelProvider
		:public CLinearModelProvider
	{
	protected:
		BlockModel mEdgeBlockModels[8];
		BlockModel mOuterCornerBlockModels[8];
		BlockModel mInnerCornerBlockModels[8];
		BlockModel mHEdgeBlockModels[8];
		BlockModel mInnerCornerBlockModels2[8];

		//锥形，6个方向各有4个翻滚角，共24个(包含上面的mOuterCornerBlockModels) https://i.bmp.ovh/imgs/2022/08/25/42439e0218bc849d.png
		BlockModel mOutCornerModels_1[24];
		//锥形，直角可以指向8个顶点 https://i.bmp.ovh/imgs/2022/08/25/16c1e6198ea3c3c1.png
		BlockModel mOutCornerModels_2[8];
	public:
		CSlopeModelProvider(BlockTemplate* pBlockTemplate);
		virtual ~CSlopeModelProvider();
		virtual BlockModel& GetBlockModel(int nIndex = 0);
		virtual BlockModel& GetBlockModel(CBlockWorld* pBlockManager, uint16_t block_id, uint16_t bx, uint16_t by, uint16_t bz, uint16_t nBlockData = 0, Block** neighborBlocks = NULL);
	protected:
		void _buildEdgeBlockModels();
		void _buildOuterCornerBlockModels();
		void _buildInnerCornerBlockModels();
		void _buildHEdgeBlockModels();
		void _buildInnerCornerBlockModels2();
		void _buildOutCornerModels_1();
		void _buildOutCornerModels_2();

		void cloneAndRotateModels(BlockModel &tempModel, Vector3 *angleArr, BlockModel *outModels, int len);
	};
}