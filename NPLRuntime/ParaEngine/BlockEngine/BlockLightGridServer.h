#pragma once

#include "BlockLightGridBase.h"

namespace ParaEngine
{
	/** block grid on server side. 
	* unlike like Client, it will compute to completion immediately. 
	*/
	class CBlockLightGridServer : public CBlockLightGridBase
	{
	public:
		CBlockLightGridServer(CBlockWorld* pBlockWorld);
		virtual ~CBlockLightGridServer();

	public:
		virtual void OnEnterWorld();
		virtual void OnLeaveWorld();
		virtual void OnWorldMove(uint16_t centerChunkX, uint16_t centerChunkZ);
		virtual void UpdateLighting();

		//get light brightness of current and 26 nearby block
		// @param brightness: pointer to uint8_t blockBrightness[27];
		// @param nSize: if 1, it will only get the center block.
		// @param nLightType: -1 means Max light. 0 means emissive block light, 1 means sun light. 2 means both. where brightness[i] is block light and brightness[i+nSize] is sunlight 
		// @return : true if exist.  false if there is no block at the position. 
		virtual bool GetBrightness(Uint16x3& blockId, uint8_t* brightness, int nSize = 27, int nLightType = -1);

		virtual void SetLightDirty(Uint16x3& blockId_ws, bool isSunLight);

		/** update all blocks in the given chunk column. A chunk column is all chunks with same x,z*/
		virtual void AddDirtyColumn(uint16_t chunkX_ws, uint16_t chunkZ_ws);

		/** get the number of remaining dirty column*/
		virtual int GetDirtyColumnCount();

	private:
		//this is the core function to spread light
		void EmitLight(const Uint16x3& blockId, uint8_t brightness, bool isSunLight = false);

		//recalc light when remove light or block state changed 
		void RefreshLight(const Uint16x3& blockId);
		void RefreshLight(const Uint16x3& blockId, bool isSunLight);

		/** only used during load time */
		void EmitSunLight(uint16_t blockIdX_ws, uint16_t blockIdZ_ws, bool bInitialSet = false);

		BlockIndex CalcLightDataIndex(const Uint16x3& blockId);

		LightData* GetLightData(const BlockIndex& index);
		LightData* GetLightData(uint16_t x, uint16_t y, uint16_t z);
		BlockChunk* GetChunk(uint16_t x, uint16_t y, uint16_t z);

	public:
		/** max number of cells(blocks) to calculate per frame. Smaller value gives better frame rate during frame load time. */
		int m_max_cells_per_frame;
		/** max number of cells(blocks) to left un-calculated per frame. When a scene is first loaded, there can be large number of blocks to calculate.
		and it is better finish loading them at start up time, instead of spreading into many frames. So this value is usually some big value. */
		int m_max_cells_left_per_frame;
	private:
		
	};
}