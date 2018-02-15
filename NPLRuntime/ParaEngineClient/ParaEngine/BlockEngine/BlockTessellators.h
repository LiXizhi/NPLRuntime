#pragma once
#include "BlockCommon.h"

namespace ParaEngine
{
	class CBlockWorld;
	class BlockChunk;
	class BlockVertexCompressed;

	/** generate tessellated vertices for a given block in the world. */
	class BlockTessellatorBase
	{
	public:
		BlockTessellatorBase(CBlockWorld* pWorld);

		virtual void SetWorld(CBlockWorld* pWorld);

		int32_t GetAvgVertexLight(int32_t v1, int32_t v2, int32_t v3, int32_t v4);

		/** not used. old algorithm for GetAvgVertexLight */
		int32_t GetMaxVertexLight(int32_t v1, int32_t v2, int32_t v3, int32_t v4);

		/**
		* @param brightness: at least array of 7 values. first is center, the following 6 is neighbours.
		*/
		uint8 GetMeshBrightness(BlockTemplate * pBlockTemplate, uint8* blockBrightness);

		/** calculate ambient occlusion flags for cube model.
		* @return ao_flags 
		*/
		uint32_t CalculateCubeAO();


		/** generate triangles for a given block in a block world, taking all nearby blocks into consideration. 
		* generate block vertex data for rendering in relative chunk space.
		* [threading]: this function is not thread safe, since it internally use global static object as the output.
		* @param pOutputData: generated vertex data. use it immediately and do not keep a reference to it. it will be invalid after the next call to this function.
		* @return rect face count is returned. this value*4 is the number of generated vertices in pOutputData.
		*/
		virtual int32 TessellateBlock(BlockChunk* pChunk, uint16 packedBlockId, BlockRenderMethod dwShaderID, BlockVertexCompressed** pOutputData);

	public:
		Block* neighborBlocks[27];
		uint8_t blockBrightness[27 * 3];
		BlockModel tessellatedModel;

	protected:
		/** update the current block info, including all block position, m_pCurBlockModel, m_nBlockData, etc. will be updated. */
		bool UpdateCurrentBlock(BlockChunk* pChunk, uint16 packedBlockId);

		/** fetch and update nearby (3*3*3=27 blocks) block id and block brightness. 
		* @param nNearbyBlockCount: how many nearby blocks to update. 1 means only the center block, 7 means including 6 neighbors. 
		* by default it is 27, which fetches all blocks in 3*3*3=27 block region. 
		* @param nNearbyLightCount: default it is -1, which means same as nNearbyBlockCount. if 0 it will disable fetching any light data
		*/
		void FetchNearbyBlockInfo(BlockChunk* pChunk, const Uint16x3& packedBlockId, int nNearbyBlockCount = 27, int nNearbyLightCount = -1);

	protected:
		CBlockWorld* m_pWorld;
		BlockTemplate * m_pCurBlockTemplate;
		uint32 m_nBlockData;
		BlockModel* m_pCurBlockModel;
		// world space location. 
		Uint16x3 m_blockId_ws;
		// chunk containing current block. 
		BlockChunk* m_pChunk;
		// chunk space coordinate. 
		Uint16x3 m_blockId_cs;
	};

	/** custom model tessellation like button, stairs, etc.  */
	class BlockGeneralTessellator : public BlockTessellatorBase
	{
	public:
		BlockGeneralTessellator(CBlockWorld* pWorld);;
		/** generate triangles for a given block in a block world, taking all nearby blocks into consideration. */
		virtual int32 TessellateBlock(BlockChunk* pChunk, uint16 packedBlockId, BlockRenderMethod dwShaderID, BlockVertexCompressed** pOutputData);


	protected:
		void TessellateLiquidOrIce(BlockRenderMethod dwShaderID);
		void TessellateStdCube(BlockRenderMethod dwShaderID);
		void TessellateUniformLightingCustomModel(BlockRenderMethod dwShaderID);
		void TessellateSelfLightingCustomModel(BlockRenderMethod dwShaderID);

		
	};
}