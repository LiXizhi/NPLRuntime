#pragma once

#include <stdint.h>
#include <vector>
#include <queue>
#include <mutex>
#include <unordered_set>
#include <map>
#include <utility>
#include <bitset>
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>
#else
#include "util/CoroutineThread.h"
#endif
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "util/ParaMemPool.h"
#include "BlockReadWriteLock.h"
#include "BlockLightGridBase.h"



namespace ParaEngine
{
	/** block grid on client side. it will only cache around a radius around the current camera eye position to keep memory low. */
	class CBlockLightGridClient : public CBlockLightGridBase
	{
	public:
		typedef FixedSizedAllocator< std::pair<const uint64_t, Light> >	DL_Allocator_BlockLight;
		
		CBlockLightGridClient(int32_t chunkCacheDim, CBlockWorld* pBlockWorld);
		virtual ~CBlockLightGridClient();

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

		/** thread safe: */
		virtual void NotifyBlockHeightChanged(uint16_t blockIdX_ws, uint16_t blockIdZ_ws, ChunkMaxHeight& prevBlockHeight);

		/** thread safe: update all blocks in the given chunk column. A chunk column is all chunks with same x,z*/
		virtual void AddDirtyColumn(uint16_t chunkX_ws, uint16_t chunkZ_ws);
		
		/** thread safe: get the number of remaining dirty column*/
		virtual int GetDirtyColumnCount();

		/** get the number of remaining dirty blocks */
		virtual int GetDirtyBlockCount();

		/** thread safe: set the given column loaded. this function is called, if light data is pre-calculated when loading the world*/
		virtual void SetColumnPreloaded(uint16_t chunkX_ws, uint16_t chunkZ_ws);
	
		/** this is called when chunk column is loaded possibly due to region unload. */
		virtual void SetColumnUnloaded(uint16_t chunkX_ws, uint16_t chunkZ_ws);

		/** Set the light grid chunk size */
		virtual void SetLightGridSize(int nSize);

		/** function to refresh the a given chunk column. if the chunk column is not calculated, it will force adding it to the queue.
		* normally this function is only used to calculate light in far away places for offline usage.
		* this function is not used during real time rendering.
		* @param nChunkWX, nChunkWZ: world chunk position.
		* @return 0 if chunk is loaded. -1 if nearby chunks are not loaded, and we can not do the calculations.
		* return 2 if chunk is still being computed in lighting thread. 
		*/
		virtual int ForceAddChunkColumn(int nChunkWX, int nChunkWZ);

		/** get the number of forced column still in the queue.*/
		virtual int GetForcedChunkColumnCount();

		/** thread safe: */
		virtual bool IsChunkColumnLoaded(int nChunkX, int nChunkZ);
	public:
		/** whether to calculate light in a separate thread. */
		bool IsAsyncLightCalculation() const;
		void SetAsyncLightCalculation(bool val);

		/** thread safe: check to see if the block pos's light is already or being calculated. */
		bool IsChunkColumnLoadedWorldPos(int nWorldX, int nWorldY, int nWorldZ);
		
		/* whether this light block is marked dirty. please note only call this functionin light thread, since there is no lock on this function. */
		bool IsLightDirty(Uint16x3& blockId_ws);
		
	private:
		/*
		* @param nUpdateRange: currently only 0 and 1 are supported. 
		* @return false if block world is exiting. 
		*/
		bool RefreshLight(const Uint16x3& blockId, bool isSunLight, int32 nUpdateRange = 0, Scoped_ReadLock<BlockReadWriteLock>* Lock_= NULL, int* pnCpuYieldCount = NULL);

		void AddPointToAABB(const Uint16x3 &curBlockPos, Int32x3 &minDirtyBlockId_ws, Int32x3 &maxDirtyBlockId_ws);

		void SetChunksDirtyInAABB(Int32x3 & minDirtyBlockId_ws, Int32x3 & maxDirtyBlockId_ws);

		/** only used during load time */
		void EmitSunLight(uint16_t blockIdX_ws,uint16_t blockIdZ_ws, bool bInitialSet=false);

		/** light thread proc */
#ifdef EMSCRIPTEN_SINGLE_THREAD
		CoroutineThread::Coroutine LightThreadProc(CoroutineThread* co_thread);
#else
		void LightThreadProc();
#endif	

		void SetLightingInChunkColumnInitialized(uint16_t chunkX_ws, uint16_t chunkBlockZ);

		void RemoveDirtyColumn(const ChunkLocation& curChunkId_ws);

		/** start the light thread. only call this function, when there is something to calculate. 
		* the light thread will automatically exit when there is no work to do or the parent block world is not entered. 
		*/
		void StartLightThread();

		/** quick update sunlight according to height map here(without emitting sunlight) */ 
		void DoQuickSunLightValues(int chunkX, int chunkZ);
		/** only do when it has not been done */
		void CheckDoQuickSunLightValues(int chunkX, int chunkZ);
		
		/* compute light value according to the nearby 6 blocks. */
		int32 ComputeLightValue(uint16 x, uint16 y, uint16 z, bool isSunLight = false);
		/* Returns whether a block above this one can reach to the sky (by checking the height map) */
		bool CanBlockSeeTheSky(uint16 x, uint16 y, uint16 z);
		/** get brightness of the given block.*/
		int32 GetSavedLightValue(int32 x, int32 y, int32 z, bool isSunLight);
		/** get opacity of the given block. */
		int32 GetBlockOpacity(int32 x, int32 y, int32 z);
		void SetLightValue(uint16_t x, uint16_t y, uint16_t z, int nLightValue, bool isSunLight);
		
	public:
		/** max number of cells(blocks) to calculate per frame. Smaller value gives better frame rate during frame load time. */
		int m_max_cells_per_frame;
		/** max number of cells(blocks) to left un-calculated per frame. When a scene is first loaded, there can be large number of blocks to calculate.
		and it is better finish loading them at start up time, instead of spreading into many frames. So this value is usually some big value. */
		int m_max_cells_left_per_frame;
	private:
		//first cached block
		int32_t m_minLightBlockIdX;
		int32_t m_minLightBlockIdZ;
		//last cached block
		int32_t m_maxLightBlockIdX;
		int32_t m_maxLightBlockIdZ;

		//fist cached chunk
		int32_t m_minChunkIdX_ws;
		int32_t m_minChunkIdZ_ws;
		int32_t m_maxChunkIdX_ws;
		int32_t m_maxChunkIdZ_ws;

		int32_t m_centerChunkIdX_ws;
		int32_t m_centerChunkIdZ_ws;
		// dirty blocks since last frame's calculation
		uint32  m_nDirtyBlocksCount;

		// temporary array used by Relight function
		std::vector<LightBlock> m_blocksNeedLightRecalcuation;

		std::map<uint64_t, Light, std::greater<uint64_t>, DL_Allocator_BlockLight> m_dirtyCells;

		typedef std::unordered_set<ChunkLocation, ChunkLocation::ChunkLocationHasher>  ChunkLocationSet_type;
		ChunkLocationSet_type m_dirtyColumns;
		/** this is just a temporary structure to get the closest to eye unprocessed chunks */
		std::vector< std::pair<ChunkLocation, int32_t> > m_closest_chunks;
		/** chunk column that must be computed regardless of its locations. this is usually for rendering far away blocks in other renderers.*/
		std::vector< ChunkLocation > m_forced_chunks;

		std::set<uint32_t> m_loaded_columns;
		/** whether DoQuickSunLightValues is called for this column */
		ChunkLocationSet_type m_quick_loaded_columns;

		/** whether light thread is working. */
		bool m_bIsLightThreadStarted;

		/** Thread used for light calculation and some other parallel task. */
#ifdef EMSCRIPTEN_SINGLE_THREAD
		CoroutineThread* m_light_thread;
#else
		std::thread m_light_thread;
#endif

		/** whether to calculate light in a separate thread. */
		bool m_bIsAsyncLightCalculation;

		std::recursive_mutex m_mutex;
	};
}