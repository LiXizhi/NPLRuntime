#pragma once

#include "BlockCommon.h"
#include "BlockConfig.h"
#include "IAttributeFields.h"

namespace ParaEngine
{
	class CBlockWorld;
	class BlockChunk;
	class LightData;

	/** data structure used when resolving light */
	class LightBlock
	{
	public:
		Uint16x3 blockId;
		int8_t brightness;
		
		LightBlock()
			:blockId(0,0,0),brightness(0)
		{
		}

		LightBlock(uint16_t x,uint16_t y,uint16_t z,uint8_t lightValue)
			:blockId(x,y,z),brightness(lightValue)
		{
		}

		LightBlock(const Uint16x3& pos,uint8_t lightValue)
			:blockId(pos),brightness(lightValue)
		{
		}
	};

	/** base class for block light grid. */
	class CBlockLightGridBase : public IAttributeFields
	{
	public:
		/** per block light data*/
		struct Light
		{
			Uint16x3 blockId;
			// 0 means normal refresh, 1 means force update neighbor within 1 blocks. - 1 to disable
			int8_t sunlightUpdateRange;
			int8_t pointLightUpdateRange;

			Light(Uint16x3& blockId_ws, uint8_t sunValue, uint8_t pointValue)
				:blockId(blockId_ws), sunlightUpdateRange(sunValue), pointLightUpdateRange(pointValue)
			{
			}
		};

		enum CellType
		{
			cp_empty,
			cp_light,
			cp_obstructure,
		};

	public:
		CBlockLightGridBase(CBlockWorld* pBlockWorld);
		virtual ~CBlockLightGridBase();
		ATTRIBUTE_DEFINE_CLASS(CBlockLightGridBase);
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CBlockLightGridBase, GetDirtyColumnCount_s, int*)		{ *p1 = cls->GetDirtyColumnCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockLightGridBase, GetDirtyBlockCount_s, int*)		{ *p1 = cls->GetDirtyBlockCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockLightGridBase, GetLightGridSize_s, int*)		{ *p1 = cls->GetLightGridSize(); return S_OK; }

		ATTRIBUTE_METHOD1(CBlockLightGridBase, GetLightCalculationStep_s, int*)		{ *p1 = cls->GetLightCalculationStep(); return S_OK; }
		ATTRIBUTE_METHOD1(CBlockLightGridBase, SetLightCalculationStep_s, int)		{ cls->SetLightCalculationStep(p1); return S_OK; }

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

		/** only call this function from main thread when you have a write lock on block world to ensure thread safety.
		* @param nUpdateMethod: 0 means normal refresh, 1 means force update neighbor within 1 blocks.  -1 to disable
		*/
		virtual void SetLightDirty(Uint16x3& blockId_ws, bool isSunLight, int8 nUpdateRange = 0);

		virtual void NotifyBlockHeightChanged(uint16_t blockIdX_ws, uint16_t blockIdZ_ws, ChunkMaxHeight& prevBlockHeight);
		
		/** update all blocks in the given chunk column. A chunk column is all chunks with same x,z*/
		virtual void AddDirtyColumn(uint16_t chunkX_ws,uint16_t chunkZ_ws);
		/** set the given column loaded. this function is called, if light data is pre-calculated when loading the world*/
		virtual void SetColumnPreloaded(uint16_t chunkX_ws, uint16_t chunkZ_ws);
		/** this is called when chunk column is loaded possibly due to region unload. */
		virtual void SetColumnUnloaded(uint16_t chunkX_ws, uint16_t chunkZ_ws);

		/** get the number of remaining dirty column*/
		virtual int GetDirtyColumnCount();
		/** get the number of remaining dirty blocks */
		virtual int GetDirtyBlockCount();

		/** get the light grid chunk size */
		virtual int GetLightGridSize();

		/** Set the light grid chunk size */
		virtual void SetLightGridSize(int nSize);

		/** how many lighting to calculate per tick for the lighting thread. 
		* @param nTicks: default to 0. it will stop light calculation when some predefined lighting tasks is finished. 
		* Otherwise, it will only stop either all tasks are finished or nTicks milliseconds have passed since it begins. 
		*/
		virtual void SetLightCalculationStep(uint32 nTicks);
		virtual uint32 GetLightCalculationStep() const;
		
		/** function to refresh the a given chunk column. if the chunk column is not calculated, it will force adding it to the queue.
		* normally this function is only used to calculate light in far away places for offline usage.
		* this function is not used during real time rendering.
		* @param nChunkWX, nChunkWZ: world chunk position.
		* @return 0 if chunk is loaded. -1 if nearby chunks are not loaded, and we can not do the calculations.
		*/
		virtual int ForceAddChunkColumn(int nChunkWX, int nChunkWZ);
		/** get the number of forced column still in the queue.*/
		virtual int GetForcedChunkColumnCount();

		/** thread safe: */
		virtual bool IsChunkColumnLoaded(int nChunkX, int nChunkZ);

	public:
		//ignore all SetLightDirty calls
		void SuspendLightUpdate();

		void ResumeLightUpdate();

		bool IsLightUpdateSuspended();
	
		/** whether to calculate light in a separate thread. */
		bool IsAsyncLightCalculation() const;
		void SetAsyncLightCalculation(bool val);
	protected:
		BlockIndex  CalcLightDataIndex(const Uint16x3& blockId, bool bCreateIfNotExist = true);
		LightData* GetLightData(const BlockIndex& index);
		LightData* GetLightData(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist = true);
		BlockChunk* GetChunk(uint16_t x, uint16_t y, uint16_t z, bool bCreateIfNotExist = true);

	protected:
		bool m_suspendLightUpdate;
		CBlockWorld* m_pBlockWorld;
		int32 m_nLightGridChunkSize;
		uint32 m_nLightCalculationStep;

		/** whether to calculate light in a separate thread. */
		bool m_bIsAsyncLightCalculation;
	};
}