//-----------------------------------------------------------------------------
// Class:	Client side light data calculation
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2012.11.26
/*  performance note:
The RefreshLight() function for a single block of second pass can take as long as
10-90ms about (1000 to 15000 queued block to recalculate)
*/
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockWorld.h"
#include "BlockRegion.h"
#include "BlockFacing.h"
#include "BlockCommon.h"
#include "SceneState.h"
#include <boost/bind.hpp>
#include "ParaTime.h"
#include "BlockLightGridClient.h"
#include "BlockFacing.h"

/** whether to use separate thread for light calculation. */
#define ASYNC_LIGHT_CALCULATION

/** define this to enable debug performance log output. */
//#define PRINT_PERF_LOG

namespace ParaEngine
{
	CBlockLightGridClient::CBlockLightGridClient(int32_t chunkCacheDim, CBlockWorld* pBlockWorld)
		: CBlockLightGridBase(pBlockWorld), m_bIsLightThreadStarted(false),
		m_minChunkIdX_ws(-1000), m_minChunkIdZ_ws(-1000), m_maxChunkIdX_ws(-1), m_maxChunkIdZ_ws(-1), m_minLightBlockIdX(-1000), m_maxLightBlockIdX(-1),
		m_minLightBlockIdZ(-1000), m_maxLightBlockIdZ(-1), m_centerChunkIdX_ws(-1), m_centerChunkIdZ_ws(-1), m_max_cells_per_frame(500), m_max_cells_left_per_frame(5000), m_nDirtyBlocksCount(0)
	{
		SetLightGridSize(chunkCacheDim);
#ifdef EMSCRIPTEN_SINGLE_THREAD
		m_light_thread = nullptr;
#endif
	}

	CBlockLightGridClient::~CBlockLightGridClient()
	{
		OnLeaveWorld();
#ifndef EMSCRIPTEN_SINGLE_THREAD
		if (m_light_thread.joinable())
		{
			OUTPUT_LOG("begin exiting light thread ...\n");
			m_light_thread.join();
			OUTPUT_LOG("light thread exited \n");
		}
#else 
	    CoroutineThread::DestroyCoroutineThread(&m_light_thread);
#endif
	}

	void CBlockLightGridClient::OnEnterWorld()
	{
		CBlockLightGridBase::OnEnterWorld();
		std::lock_guard<std::recursive_mutex> Lock_(m_mutex);
		m_minChunkIdX_ws = -1000;
		m_minChunkIdZ_ws = -1000;
		m_maxChunkIdX_ws = -1;
		m_maxChunkIdZ_ws = -1;
		m_minLightBlockIdX = -1000;
		m_maxLightBlockIdX = -1;
		m_minLightBlockIdZ = -1000;
		m_maxLightBlockIdZ = -1;
		m_centerChunkIdX_ws = -1;
		m_centerChunkIdZ_ws = -1;
		m_nDirtyBlocksCount = 0;

		m_dirtyColumns.clear();
		m_loaded_columns.clear();
		m_forced_chunks.clear();
		m_blocksNeedLightRecalcuation.resize(32 * 32 * 32);
	}

	void CBlockLightGridClient::OnLeaveWorld()
	{
		//release memory used by stl
		{
			std::lock_guard<std::recursive_mutex> Lock_(m_mutex);
			m_loaded_columns.clear();
			m_forced_chunks.clear();
			m_dirtyColumns.clear();
			m_nDirtyBlocksCount = 0;
		}

#ifndef EMSCRIPTEN_SINGLE_THREAD
		if (m_pBlockWorld && m_bIsLightThreadStarted && m_light_thread.joinable())
		{
			if (!m_pBlockWorld->GetReadWriteLock().HasWriterLock())
			{
				m_light_thread.join();
			}
			else
			{
				// this code must be called from the main thread which owns the writer lock
				PE_ASSERT(m_pBlockWorld->GetReadWriteLock().IsCurrentThreadHasWriterLock());
				Scoped_WriterUnlock<>  unlock_(m_pBlockWorld->GetReadWriteLock());
				m_light_thread.join();
			}
		}
#else 
	    CoroutineThread::DestroyCoroutineThread(&m_light_thread);
#endif
		{
			std::lock_guard<std::recursive_mutex> Lock_(m_mutex);
			m_dirtyCells.clear();
		}
	}

	void CBlockLightGridClient::OnWorldMove(uint16_t centerChunkX, uint16_t centerChunkZ)
	{
		m_centerChunkIdX_ws = centerChunkX;
		m_centerChunkIdZ_ws = centerChunkZ;
		int32_t chunkCacheRadius = m_nLightGridChunkSize / 2;
		int32_t newChunkStartX = centerChunkX - chunkCacheRadius;
		int32_t newChunkStartZ = centerChunkZ - chunkCacheRadius;

		int32_t deltaChunkX = newChunkStartX - m_minChunkIdX_ws;
		int32_t deltaChunkZ = newChunkStartZ - m_minChunkIdZ_ws;

		if (deltaChunkX == 0 && deltaChunkZ == 0)
			return;

		m_minChunkIdX_ws = newChunkStartX;
		m_minChunkIdZ_ws = newChunkStartZ;
		m_maxChunkIdX_ws = m_minChunkIdX_ws + m_nLightGridChunkSize;
		m_maxChunkIdZ_ws = m_minChunkIdZ_ws + m_nLightGridChunkSize;

		m_minLightBlockIdX = m_minChunkIdX_ws * BlockConfig::g_chunkBlockDim;
		m_minLightBlockIdZ = m_minChunkIdZ_ws * BlockConfig::g_chunkBlockDim;

		m_maxLightBlockIdX = m_minLightBlockIdX + m_nLightGridChunkSize * BlockConfig::g_chunkBlockDim;
		m_maxLightBlockIdZ = m_minLightBlockIdZ + m_nLightGridChunkSize * BlockConfig::g_chunkBlockDim;

		if (abs(deltaChunkX) >= m_nLightGridChunkSize || abs(deltaChunkZ) >= m_nLightGridChunkSize)
		{
			for (int32_t x = 0; x < m_nLightGridChunkSize; x++)
			{
				for (int32_t z = 0; z < m_nLightGridChunkSize; z++)
				{
					int32_t curChunkIdX_ws = m_minChunkIdX_ws + x;
					int32_t curChunkIdZ_ws = m_minChunkIdZ_ws + z;

					if (curChunkIdX_ws >= 0 && curChunkIdX_ws < 0xffff
						&& curChunkIdZ_ws >= 0 && curChunkIdZ_ws < 0xffff)
					{
						if (!IsChunkColumnLoaded(curChunkIdX_ws, curChunkIdZ_ws))
							AddDirtyColumn(curChunkIdX_ws, curChunkIdZ_ws);
					}
				}
			}
		}
		else
		{
			int32_t startChunkX_ws = m_minChunkIdX_ws;
			if (deltaChunkX >= 0)
				startChunkX_ws = startChunkX_ws + m_nLightGridChunkSize - deltaChunkX;

			int32_t startChunkZ_ws = m_minChunkIdZ_ws;
			if (deltaChunkZ >= 0)
				startChunkZ_ws = startChunkZ_ws + m_nLightGridChunkSize - deltaChunkZ;

			//-------update light chunks alone x axis-------------
			for (int16_t x = 0; x < abs(deltaChunkX); x++)
			{
				uint32_t curChunkX_ws = startChunkX_ws + x;

				for (int16_t z = 0; z < m_nLightGridChunkSize; z++)
				{
					int32_t curChunkZ_ws = m_minChunkIdZ_ws + z;
					if (!IsChunkColumnLoaded(curChunkX_ws, curChunkZ_ws))
						AddDirtyColumn(curChunkX_ws, curChunkZ_ws);
				}
			}
			startChunkX_ws = m_minChunkIdX_ws;

			//--------update light chunks alone z axis------------
			for (int16_t z = 0; z < abs(deltaChunkZ); z++)
			{
				int32_t curChunkZ_ws = startChunkZ_ws + z;
				for (int x = 0; x < m_nLightGridChunkSize; x++)
				{
					int32_t curChunkX_ws = m_minChunkIdX_ws + x;
					if (!IsChunkColumnLoaded(curChunkX_ws, curChunkZ_ws))
						AddDirtyColumn(curChunkX_ws, curChunkZ_ws);
				}
			}
		}
	}

	bool CBlockLightGridClient::GetBrightness(Uint16x3& blockId_ws, uint8_t* brightness, int nSize, int nLightType)
	{
		if (brightness == nullptr)
			return false;

		// A mesh asks for up to 27 neighbors of the same vertex. Most samples
		// share one section, including the block and sky channels of each sample.
		const BlockIndex center = CalcLightDataIndex(blockId_ws, false);
		Uint16x3 curBlockId_ws;
		for (int i = 0; i < nSize; i++)
		{
			curBlockId_ws.x = blockId_ws.x + BlockCommon::NeighborOfsTable[i].x;
			curBlockId_ws.y = blockId_ws.y + BlockCommon::NeighborOfsTable[i].y;
			curBlockId_ws.z = blockId_ws.z + BlockCommon::NeighborOfsTable[i].z;

			if (nLightType == 0)
			{
				brightness[i] = 0;
			}
			uint8 nSunLight = 0;
			uint8 nBlockLight = 0;

			const int changedBits = (curBlockId_ws.x ^ blockId_ws.x) | (curBlockId_ws.y ^ blockId_ws.y) | (curBlockId_ws.z ^ blockId_ws.z);
			const bool sameSection = (changedBits & ~15) == 0 && center.m_pChunk && BlockConfig::IsBlockY(curBlockId_ws.y);
			const BlockIndex blockIndex = i == 0 ? center : sameSection
				? BlockIndex(center.m_pChunk, CalcPackedBlockID(curBlockId_ws.x, curBlockId_ws.y, curBlockId_ws.z))
				: CalcLightDataIndex(curBlockId_ws, false);
			if (blockIndex.m_pChunk)
			{
				LightData* pLightData = blockIndex.m_pChunk->GetLightData(blockIndex.m_nChunkBlockIndex);
				if (nLightType != 1)
					nBlockLight = pLightData->GetBrightness(false);
				if (nLightType != 0)
				{
					if (curBlockId_ws.y >= m_nSkyHeight || blockIndex.m_pChunk->CanBlockSeeTheSkyWS(curBlockId_ws.x, curBlockId_ws.y, curBlockId_ws.z))
					{
						nSunLight = 15;
					}
					else if (blockIndex.m_pChunk->HasBlocks())
						nSunLight = pLightData->GetBrightness(true);
					else
						nSunLight = SampleEmptySkyLight(curBlockId_ws);
				}
			}
			else
			{
				if (nLightType != 0)
					nSunLight = SampleEmptySkyLight(curBlockId_ws);
			}

			if (nLightType == -1)
			{
				nSunLight = (uint8_t)(nSunLight * m_pBlockWorld->GetSunIntensity());
				brightness[i] = (nBlockLight > nSunLight ? nBlockLight : nSunLight);
			}
			else if (nLightType == 2)
			{
				brightness[i] = nBlockLight;
				brightness[i + nSize] = nSunLight;
			}
			else if (nLightType == 3)
			{
				brightness[i + nSize] = nBlockLight;
				brightness[i + nSize * 2] = nSunLight;
				nSunLight = (uint8_t)(nSunLight * m_pBlockWorld->GetSunIntensity());
				brightness[i] = (nBlockLight > nSunLight ? nBlockLight : nSunLight);
			}
			else
				brightness[i] = (nLightType == 1) ? nSunLight : nBlockLight;
		}
		return true;
	}

	bool CBlockLightGridClient::IsLightDirty(Uint16x3& blockId_ws)
	{
		uint16_t chunk_ws_x = blockId_ws.x >> 4;
		uint16_t chunk_ws_y = blockId_ws.y >> 4;
		uint16_t chunk_ws_z = blockId_ws.z >> 4;

		uint16_t cx = blockId_ws.x & 0xf;
		uint16_t cy = blockId_ws.y & 0xf;
		uint16_t cz = blockId_ws.z & 0xf;
		uint16_t packedBlockId_cs = cx + (cz << 4) + (cy << 8); // y in higher bits

		uint64_t key = (((uint64_t)chunk_ws_y) << 48) + (((uint64_t)chunk_ws_x) << 32) + (((uint64_t)chunk_ws_z) << 16) + packedBlockId_cs; // chuck y in higher bits. 

		return key == m_processingLightKey || m_dirtyCells.find(key) != m_dirtyCells.end();
	}

	void CBlockLightGridClient::SetLightDirty(Uint16x3& blockId_ws, bool isSunLight, int8 nUpdateRange)
	{
		if (m_suspendLightUpdate || !BlockConfig::IsBlockY(blockId_ws.y))
			return;

		// uint64_t key = ((uint64_t)blockId_ws.x<<32) + ((uint64_t)blockId_ws.y<<16) + blockId_ws.z;

		// make a key: where blocks inside the same 16*16*16 chunk are grouped together. and the higher chunk (y) is sorted in front. 

		uint16_t chunk_ws_x = blockId_ws.x >> 4;
		uint16_t chunk_ws_y = blockId_ws.y >> 4;
		uint16_t chunk_ws_z = blockId_ws.z >> 4;

		uint16_t cx = blockId_ws.x & 0xf;
		uint16_t cy = blockId_ws.y & 0xf;
		uint16_t cz = blockId_ws.z & 0xf;
		uint16_t packedBlockId_cs = cx + (cz << 4) + (cy << 8); // y in higher bits

		uint64_t key = (((uint64_t)chunk_ws_y) << 48) + (((uint64_t)chunk_ws_x) << 32) + (((uint64_t)chunk_ws_z) << 16) + packedBlockId_cs; // chuck y in higher bits. 


		auto got = m_dirtyCells.find(key);
		if (got == m_dirtyCells.end())
		{
			uint8_t sunLightRange, pointLightRange;
			if (isSunLight)
			{
				sunLightRange = nUpdateRange;
				pointLightRange = -1;
			}
			else
			{
				sunLightRange = -1;
				pointLightRange = nUpdateRange;
			}
			m_dirtyCells.insert(std::make_pair(key, Light(blockId_ws, sunLightRange, pointLightRange)));
		}
		else
		{
			if (isSunLight && nUpdateRange > got->second.sunlightUpdateRange)
				got->second.sunlightUpdateRange = nUpdateRange;
			else if (!isSunLight && nUpdateRange > got->second.pointLightUpdateRange)
				got->second.pointLightUpdateRange = nUpdateRange;
		}
	}

	void CBlockLightGridClient::NotifyBlockHeightChanged(uint16_t blockIdX_ws, uint16_t blockIdZ_ws, ChunkMaxHeight& prevBlockHeight)
	{
		auto* region = m_pBlockWorld->GetRegion(blockIdX_ws >> 9, blockIdZ_ws >> 9);
		if (!region) return;
		const int oldTop = prevBlockHeight.GetMaxSolidHeight();
		const int newTop = region->GetHighestBlock(blockIdX_ws & 511, blockIdZ_ws & 511)->GetMaxSolidHeight();
		if (oldTop == newTop) return;
		const int low = (std::min)(oldTop, newTop) + 1;
		const int high = (std::max)(oldTop, newTop);
		QueueSkyInterval(blockIdX_ws, blockIdZ_ws, low, high, true);
	}

	void CBlockLightGridClient::QueueSkyInterval(uint16_t x, uint16_t z, int low, int high, bool edit)
	{
		auto* region = m_pBlockWorld->GetRegion(x >> 9, z >> 9);
		if (!region) return;
		// Include the two air samples immediately outside a stored section.
		// The existing two-pass solver propagates lateral light from these seeds.
		int nextY = (std::max<int>)(low, BlockConfig::MinBlockY);
		std::vector<Uint16x3> sections;
		const int halo = edit ? 14 : 0;
		const int x0 = (std::max)(0, x - halo) >> 4, x1 = (x + halo) >> 4;
		const int z0 = (std::max)(0, z - halo) >> 4, z1 = (z + halo) >> 4;
		for (int rz = z0 >> 5; rz <= (z1 >> 5); ++rz)
			for (int rx = x0 >> 5; rx <= (x1 >> 5); ++rx)
				if (auto* nearby = m_pBlockWorld->GetRegion(rx, rz))
					nearby->GetOccupiedSections(x0, BlockConfig::SectionY(low - 1), z0, x1, BlockConfig::SectionY(high + 1), z1, sections);
		if (edit) std::sort(sections.begin(), sections.end(), [](const Uint16x3& a, const Uint16x3& b) { return a.y < b.y; });
		for (const auto& pos : sections)
		{
			const int section = pos.y;
			if (!edit)
			{
				auto* cached = region->GetChunk(PackChunkIndex(pos.x & 31, section, pos.z & 31), false);
				if (cached && cached->IsLightingInitialized()) continue;
			}
			const int begin = (std::max)(nextY, section * 16 - 1);
			const int end = (std::min<int>)((std::min)(high, section * 16 + 16), BlockConfig::MaxBlockY);
			if (begin <= end)
			{
				Int32x3 minPos(x, begin, z), maxPos(x, end, z);
				SetChunksDirtyInAABB(minPos, maxPos);
			}
			for (int y = begin; y <= end; ++y)
			{
				Uint16x3 pos(x, y, z);
				SetLightDirty(pos, true, 1);
			}
			if (end >= begin) nextY = end + 1;
			if (nextY > high) break;
		}
	}

	void CBlockLightGridClient::StartLightThread()
	{
		if (!m_bIsLightThreadStarted)
		{
			m_bIsLightThreadStarted = true;
			try
			{
#ifdef EMSCRIPTEN_SINGLE_THREAD
				m_light_thread = CoroutineThread::StartCoroutineThread([this](CoroutineThread* t) -> CO_ASYNC {
					CO_AWAIT(this->LightThreadProc(t));
				}, nullptr);
#else
				m_light_thread = std::thread(std::bind(&CBlockLightGridClient::LightThreadProc, this));
#endif
			}
			catch (...)
			{
				m_bIsLightThreadStarted = false;
				// some other exception (print a diagnostic!)
				OUTPUT_LOG("error: m_light_thread unknown error\n");
			}
		}
	}
#ifdef EMSCRIPTEN_SINGLE_THREAD
	CoroutineThread::Coroutine CBlockLightGridClient::LightThreadProc(CoroutineThread* co_thread)
#else
	void CBlockLightGridClient::LightThreadProc()
#endif
	{
		Scoped_ReadLock<BlockReadWriteLock> lock_(m_pBlockWorld->GetReadWriteLock());

		m_bIsLightThreadStarted = true;
		unsigned int nStartTime = GetTickCount();
		unsigned int nLightCalculationTimeLeft = GetLightCalculationStep();

		int32_t processedCount = 0;
		m_nDirtyBlocksCount = (int)m_dirtyCells.size();

		// #define DISABLE_LIGHTING_CALCULATION_TEST_ONLY
#ifdef DISABLE_LIGHTING_CALCULATION_TEST_ONLY
		m_dirtyColumns.clear();
		m_dirtyCells.clear();
#endif
		// call this function regularly to yield CPU to writer thread only if they are waiting to write data. 
#define CHECK_YIELD_CPU_TO_WRITER   if(m_pBlockWorld->GetReadWriteLock().HasWaitingWritersAndSingleReader()){ \
	m_nDirtyBlocksCount = (int)(m_dirtyCells.size() + processedCount); \
	++nYieldCPUTimes;\
	lock_.unlock(); \
	lock_.lock(); \
	if(!m_pBlockWorld->IsInBlockWorld()){\
		m_bIsLightThreadStarted = false;\
		goto exit_function;\
				}\
						}

		while (m_pBlockWorld->IsInBlockWorld())
		{
			// this function is called on each pre-render frame move to update light values if necessary. 
			processedCount = 0;

			int max_cells_left_per_frame = 999999;// m_max_cells_left_per_frame;
			int max_cells_per_frame = 50;//  m_max_cells_per_frame;
			int32_t maxColumnPerFrame = 1;
			int nYieldCPUTimes = 0;
			// how many chunk columns to update every frame move. 

			if ((int)m_dirtyCells.size() < max_cells_per_frame * 2 && (m_dirtyColumns.size() > 0 || !m_forced_chunks.empty()))
			{
				m_closest_chunks.clear();
				ChunkLocation chunkEye((m_minChunkIdX_ws + m_maxChunkIdX_ws) / 2, (m_minChunkIdZ_ws + m_maxChunkIdZ_ws) / 2);

				{
					std::unique_lock<std::recursive_mutex> DirtyColumnLock_(m_mutex);
					for (auto it = m_dirtyColumns.begin(); it != m_dirtyColumns.end();)
					{
						const ChunkLocation& curChunkId_ws = *it;
						uint16_t chunkX_ws = curChunkId_ws.m_chunkX;
						uint16_t chunkZ_ws = curChunkId_ws.m_chunkZ;
						if (chunkX_ws >= m_minChunkIdX_ws && chunkX_ws < m_maxChunkIdX_ws
							&& chunkZ_ws >= m_minChunkIdZ_ws && chunkZ_ws < m_maxChunkIdZ_ws)
						{
							if (m_pBlockWorld->DoChunksNearChunkExist(curChunkId_ws.GetCenterWorldX(), 0, curChunkId_ws.GetCenterWorldZ(), 16))
							{
								// only fully executed insertions may happen, so iterators are still valid. 
								int nDist = curChunkId_ws.DistanceToSquared(chunkEye);
								int nSize = (int)(m_closest_chunks.size());
								if (nSize < maxColumnPerFrame)
									m_closest_chunks.push_back(std::pair<ChunkLocation, int32_t>(curChunkId_ws, nDist));
								else
								{

									for (int i = 0; i < nSize; ++i)
									{
										if (m_closest_chunks[i].second > nDist)
										{
											m_closest_chunks[i] = std::pair<ChunkLocation, int32_t>(curChunkId_ws, nDist);
											break;
										}
									}
								}
								it++;
							}
							else
							{
								it++;
							}
						}
						else
						{
							it = m_dirtyColumns.erase(it);
						}
					}

					int nNumOfForceChunk = 0;
					while (!m_forced_chunks.empty() && (nNumOfForceChunk < maxColumnPerFrame))
					{
						m_closest_chunks.push_back(std::pair<ChunkLocation, int32_t>(m_forced_chunks.back(), 0));
						m_forced_chunks.pop_back();
						nNumOfForceChunk++;
					}

					int nSize = (int)(m_closest_chunks.size());
					for (int i = 0; i < nSize; ++i)
					{
						const ChunkLocation& curChunkId_ws = m_closest_chunks[i].first;
						RemoveDirtyColumn(curChunkId_ws);
					}
				}

				int nSize = (int)(m_closest_chunks.size());
				for (int i = 0; i < nSize; ++i)
				{
					processedCount++;
					const ChunkLocation& curChunkId_ws = m_closest_chunks[i].first;
					uint16_t chunkX_ws = curChunkId_ws.m_chunkX;
					uint16_t chunkZ_ws = curChunkId_ws.m_chunkZ;

					if (!m_pBlockWorld->RefreshAllLightsInColumn(chunkX_ws, chunkZ_ws))
					{
						SetColumnPreloaded(chunkX_ws, chunkZ_ws);
						continue;
					}
					CHECK_YIELD_CPU_TO_WRITER;

					uint16_t chunkBlockX = chunkX_ws * BlockConfig::g_chunkBlockDim;
					uint16_t chunkBlockZ = chunkZ_ws * BlockConfig::g_chunkBlockDim;
					for (int x = 0; x < BlockConfig::g_chunkBlockDim; x++)
					{
						for (int z = 0; z < BlockConfig::g_chunkBlockDim; z++)
						{
							EmitSunLight(chunkBlockX + x, chunkBlockZ + z);
							CHECK_YIELD_CPU_TO_WRITER;
						}
					}
					SetLightingInChunkColumnInitialized(chunkX_ws, chunkZ_ws);

					SetColumnPreloaded(chunkX_ws, chunkZ_ws);
					// RemoveDirtyColumn(curChunkId_ws);
				}
				m_closest_chunks.clear();
			}

			int nDirtyCellCount = (int)m_dirtyCells.size();
			if (nDirtyCellCount > 0)
			{
				processedCount++;
				m_nDirtyBlocksCount = nDirtyCellCount;
				int nBlocksToCalculateThisFrame = max_cells_per_frame;
				if ((nDirtyCellCount - max_cells_left_per_frame) > max_cells_per_frame)
				{
					nBlocksToCalculateThisFrame = nDirtyCellCount - max_cells_left_per_frame;
				}

				if (!ProcessPendingLightCells(nBlocksToCalculateThisFrame, &lock_, &nYieldCPUTimes))
					goto exit_function;
			}

			unsigned int nFinishTime = GetTickCount();
			unsigned int nStepDurationTicks = Math::Max(nFinishTime - nStartTime, (unsigned int)1);

			if (nLightCalculationTimeLeft < nStepDurationTicks)
			{
				nLightCalculationTimeLeft = GetLightCalculationStep();
				// since read/write lock is used, this function will try to notify any waiting writers. 

				lock_.unlock();

				if (m_dirtyCells.size() == 0 && processedCount == 0){
					m_nDirtyBlocksCount = 0;
#if defined(EMSCRIPTEN_SINGLE_THREAD)
					CO_AWAIT(co_thread->Sleep(30));
					// CO_SLEEP(co_thread, 30);
#elif defined(__EMSCRIPTEN__)
					SLEEP(50);
#else
					SLEEP(10);
#endif
				}
				while (IsLightUpdateSuspended() && m_pBlockWorld->IsInBlockWorld()){
#if defined(EMSCRIPTEN_SINGLE_THREAD)
					CO_AWAIT(co_thread->Sleep(30));
					// CO_SLEEP(co_thread, 30);
#elif defined(__EMSCRIPTEN__)
					SLEEP(30);
#else
					SLEEP(1);
#endif
				}
				// since we are using our own read/write lock, this function will block until writers are freed. 
				// in most cases when no writer request, it will lock immediately again, thus utilizing more CPU to process remaining light cells.  
				lock_.lock();
				nStartTime = GetTickCount();
			}
			else
			{
				// sometimes GetTickCount will wrap around INT32, so this prevents it. 
				nStartTime = nFinishTime;
				nLightCalculationTimeLeft = Math::Min(nLightCalculationTimeLeft - nStepDurationTicks, (unsigned int)GetLightCalculationStep());

				if (m_dirtyCells.size() == 0 && processedCount == 0){
					m_nDirtyBlocksCount = 0;
					lock_.unlock();
#if defined(EMSCRIPTEN_SINGLE_THREAD)
					// CO_SLEEP(co_thread, 30);
					CO_AWAIT(co_thread->Sleep(30));
#elif defined(__EMSCRIPTEN__)
					SLEEP(50);
#else
					SLEEP(10);
#endif
					lock_.lock();
				}
			}
		}
exit_function:
		m_nDirtyBlocksCount = 0;
		m_bIsLightThreadStarted = false;
	}

	bool CBlockLightGridClient::ProcessPendingLightCells(int maxCells, Scoped_ReadLock<BlockReadWriteLock>* lock, int* yields)
	{
		while (!m_dirtyCells.empty() && maxCells-- > 0 && !IsLightUpdateSuspended())
		{
			auto it = m_dirtyCells.begin();
			m_processingLightKey = it->first;
			const auto seed = it->second;
			m_dirtyCells.erase(it);
			// A writer can edit this same cell while propagation yields. Leave its
			// newly queued work intact and keep the active neighborhood unsaveable.
			const bool completed = (seed.sunlightUpdateRange < 0 || RefreshLight(seed.blockId, true, seed.sunlightUpdateRange, lock, yields))
				&& (seed.pointLightUpdateRange < 0 || RefreshLight(seed.blockId, false, seed.pointLightUpdateRange, lock, yields));
			m_processingLightKey = UINT64_MAX;
			if (!completed) return false;
		}
		return true;
	}

	void CBlockLightGridClient::UpdateLighting()
	{
		if (!(m_pBlockWorld->IsInBlockWorld()))
		{
			return;
		}

		if (IsAsyncLightCalculation())
		{
			StartLightThread();
		}
		else
		{
#ifndef	ASYNC_LIGHT_CALCULATION
			if (m_dirtyColumns.size() == 0 && m_dirtyCells.size() == 0)
				return;
			// this function is called on each pre-render frame move to update light values if necessary. 
			int32_t processedCount = 0;

			bool bIsCameraMoved = CGlobals::GetSceneState()->m_bCameraMoved;

			int max_cells_left_per_frame = m_max_cells_left_per_frame;
			int max_cells_per_frame = m_max_cells_per_frame;
			int32_t maxColumnPerFrame = 1;

			if (bIsCameraMoved)
			{
				max_cells_per_frame = 50;
				max_cells_left_per_frame = 999999999;

				if ((int)m_dirtyCells.size() > max_cells_per_frame)
					maxColumnPerFrame = 0;
				else
					maxColumnPerFrame = 1;
			}
			else
			{
				/*if((int)m_dirtyCells.size() > max_cells_per_frame)
				{
				maxColumnPerFrame = 0;
				}*/
			}

			// how many chunk columns to update every frame move. 
			// TODO: shall we calculate from near to far. 
			// m_dirtyColumns should use a hash function to sort by distance. 
			// TODO: shall we only calculate when m_dirtyCells is empty. 

			for (auto it = m_dirtyColumns.begin(); it != m_dirtyColumns.end(); )
			{
				const ChunkLocation& curChunkId_ws = *it;
				uint16_t chunkX_ws = curChunkId_ws.m_chunkX;
				uint16_t chunkZ_ws = curChunkId_ws.m_chunkZ;
				if (chunkX_ws >= m_minChunkIdX_ws && chunkX_ws < m_maxChunkIdX_ws
					&& chunkZ_ws >= m_minChunkIdZ_ws && chunkZ_ws < m_maxChunkIdZ_ws)
				{
					if (m_pBlockWorld->DoChunksNearChunkExist(curChunkId_ws.GetCenterWorldX(), 0, curChunkId_ws.GetCenterWorldZ(), 16))
					{
						if (processedCount < maxColumnPerFrame)
						{
							m_pBlockWorld->RefreshAllLightsInColumn(chunkX_ws, chunkZ_ws);

							uint16_t chunkBlockX = chunkX_ws * BlockConfig::g_chunkBlockDim;
							uint16_t chunkBlockZ = chunkZ_ws * BlockConfig::g_chunkBlockDim;
							for (int x = 0; x < BlockConfig::g_chunkBlockDim; x++)
							{
								for (int z = 0; z < BlockConfig::g_chunkBlockDim; z++)
								{
									EmitSunLight(chunkBlockX + x, chunkBlockZ + z);
								}
							}
							SetLightingInChunkColumnInitialized(chunkX_ws, chunkZ_ws);
							processedCount++;
							it = m_dirtyColumns.erase(it);
							SetColumnPreloaded(chunkX_ws, chunkZ_ws);
						}
						else
							it++;
					}
					else
					{
						it++;
					}
				}
				else
				{
					it = m_dirtyColumns.erase(it);
				}
			}


			int nDirtyCellCount = (int)m_dirtyCells.size();
			if (nDirtyCellCount > 0)
			{
				int nBlocksToCalculateThisFrame = max_cells_per_frame;
				if ((nDirtyCellCount - max_cells_left_per_frame) > max_cells_per_frame)
				{
					nBlocksToCalculateThisFrame = nDirtyCellCount - max_cells_left_per_frame;
				}
				// pass 1: first calculate those near the eye center
				/*for(auto it = m_dirtyCells.begin();it != m_dirtyCells.end() && nBlocksToCalculateThisFrame>0;)
				{
				Light& light_data = it->second;
				if(light_data.blockId.x >= m_minLightBlockIdX && light_data.blockId.x <= m_maxLightBlockIdX && light_data.blockId.z >= m_minLightBlockIdZ && light_data.blockId.z <= m_maxLightBlockIdZ)
				{
				-- nBlocksToCalculateThisFrame;
				if(light_data.sunLightValue > 0)
				EmitLight(light_data.blockId,light_data.sunLightValue,true);
				else if(light_data.sunLightValue == 0)
				RefreshLight(light_data.blockId, true);

				if(light_data.pointLightValue > 0)
				EmitLight(light_data.blockId,light_data.pointLightValue, false);
				else if(light_data.pointLightValue == 0)
				RefreshLight(light_data.blockId, false);
				}

				it = m_dirtyCells.erase(it);
				}*/

				// pass 2: calculate everything else in any order. 
				for (auto it = m_dirtyCells.begin(); it != m_dirtyCells.end() && nBlocksToCalculateThisFrame > 0;)
				{
					Light& light_data = it->second;
					if (light_data.blockId.x >= m_minLightBlockIdX && light_data.blockId.x <= m_maxLightBlockIdX && light_data.blockId.z >= m_minLightBlockIdZ && light_data.blockId.z <= m_maxLightBlockIdZ)
					{
						--nBlocksToCalculateThisFrame;
						if (light_data.sunlightUpdateRange > 0)
							EmitLight(light_data.blockId, light_data.sunlightUpdateRange, true);
						else if (light_data.sunlightUpdateRange == 0)
							RefreshLight(light_data.blockId, true);

						if (light_data.pointLightUpdateRange > 0)
							EmitLight(light_data.blockId, light_data.pointLightUpdateRange, false);
						else if (light_data.pointLightUpdateRange == 0)
							RefreshLight(light_data.blockId, false);
					}

					it = m_dirtyCells.erase(it);
				}
			}
#endif
		}
	}

	int CBlockLightGridClient::ForceAddChunkColumn(int chunkX_ws, int chunkZ_ws)
	{
		std::unique_lock<std::recursive_mutex> Lock_(m_mutex);

		if (IsChunkColumnLoaded(chunkX_ws, chunkZ_ws))
			return 0;
		if (!m_pBlockWorld->DoChunksNearChunkExist(chunkX_ws * 16, 0, chunkZ_ws * 16, 16))
		{
			return -1;
		}

		for (const ChunkLocation& chunkPos : m_forced_chunks)
		{
			if (chunkPos.m_chunkX == chunkX_ws && chunkPos.m_chunkZ == chunkZ_ws)
				return 2;
		}
		m_forced_chunks.push_back(ChunkLocation(chunkX_ws, chunkZ_ws));

		return 2;
	}

	int CBlockLightGridClient::GetForcedChunkColumnCount()
	{
		std::unique_lock<std::recursive_mutex> Lock_(m_mutex);
		return m_forced_chunks.size();
	}

	// blocks higher than the highest solid block in the height map can sky. Note, the top most opaque block can not see the sky. 
	bool CBlockLightGridClient::CanBlockSeeTheSky(uint16 x, int16_t y, uint16 z)
	{
		ChunkMaxHeight* pHeight = m_pBlockWorld->GetHighestBlock(x, z);
		if (pHeight)
			return y >= m_nSkyHeight || pHeight->GetMaxSolidHeight() < y;
		else
			return true;
	}

	// Trace only through implicit air. A stored cell supplies the two-pass solver's
	// boundary value. Light loses one level per step, regardless of the world height.
	uint8_t CBlockLightGridClient::SampleEmptySkyLight(const Uint16x3& pos)
	{
		if (pos.y < BlockConfig::MinBlockY) return 0;
		if (CanBlockSeeTheSky(pos.x, pos.y, pos.z)) return 15;
		constexpr int width = 29, plane = width * width, volume = plane * width;
		struct Search {
			std::array<uint16_t, volume> visited{};
			uint16_t generation = 0;
			std::vector<uint16_t> queue;
		};
		static thread_local Search search;
		if (++search.generation == 0)
		{
			search.visited.fill(0);
			++search.generation;
		}
		search.queue.clear();
		const int center = 14 * (plane + width + 1);
		search.queue.push_back(center);
		search.visited[center] = search.generation;
		int best = 0;
		size_t head = 0;
		for (int distance = 1; distance < 15 - best && head < search.queue.size(); ++distance)
		{
			const size_t end = search.queue.size();
			while (head < end)
			{
				const int index = search.queue[head++];
				for (int offset : {-1, 1, -width, width, -plane, plane})
				{
					const int next = index + offset;
					if (search.visited[next] == search.generation) continue;
					search.visited[next] = search.generation;
					const int x = pos.x + next % width - 14;
					const int z = pos.z + next / width % width - 14;
					const int y = pos.y + next / plane - 14;
					if (x < 0 || x > 65535 || z < 0 || z > 65535 || y < BlockConfig::MinBlockY) continue;
					if (CanBlockSeeTheSky(x, y, z)) return 15 - distance;
					Uint16x3 sample(x, y, z);
					const auto cell = CalcLightDataIndex(sample, false);
					if (cell.m_pChunk && cell.m_pChunk->HasBlocks())
						best = (std::max)(best, int(cell.m_pChunk->GetLightData(cell.m_nChunkBlockIndex)->GetBrightness(true)) - distance);
					else
						search.queue.push_back(next);
				}
			}
		}
		return best;
	}

	int32 CBlockLightGridClient::GetSavedLightValue(int32 x, int32 y, int32 z, bool isSunLight)
	{
		if (BlockConfig::IsBlockY(y) && x >= 0 && z >= 0)
		{
			Uint16x3 curBlockId_ws((uint16)x, y, (uint16)z);

			BlockIndex blockIndex = CalcLightDataIndex(curBlockId_ws, false);
			if (blockIndex.m_pChunk)
			{
				auto lightData = GetLightData(blockIndex);
				if (isSunLight)
				{
					if (curBlockId_ws.y >= m_nSkyHeight || blockIndex.m_pChunk->CanBlockSeeTheSkyWS(curBlockId_ws.x, curBlockId_ws.y, curBlockId_ws.z))
					{
						return 15;
					}
					else
						return lightData->GetBrightness(true);
				}
				else
				{
					return lightData->GetBrightness(false);
				}
			}
			else
			{
				return (isSunLight && CanBlockSeeTheSky(curBlockId_ws.x, curBlockId_ws.y, curBlockId_ws.z)) ? 15 : 0;
			}
		}
		return 0;
	}

	int32 CBlockLightGridClient::GetBlockOpacity(int32 x, int32 y, int32 z)
	{
		if (BlockConfig::IsBlockY(y) && x >= 0 && z >= 0)
		{
			BlockTemplate* pTemplate = m_pBlockWorld->GetBlockTemplate((uint16)x, y, (uint16)z);
			return pTemplate ? pTemplate->GetLightOpacity() : 1;
		}
		return 1;
	}

	void CBlockLightGridClient::SetLightValue(uint16_t x, int16_t y, uint16_t z, int nLightValue, bool isSunLight)
	{
		Uint16x3 neighborId(x, y, z);
		if (!BlockConfig::IsBlockY(y)) return;
		auto lightIndex = CalcLightDataIndex(neighborId, false);
		if (!lightIndex.m_pChunk)
		{
			// Clearing a cell in the invalidation pass has no stored data to
			// clear. Direct sunlight is supplied by the height index on every read.
			if (nLightValue == 0 || (isSunLight && CanBlockSeeTheSky(x, y, z))) return;
			lightIndex = CalcLightDataIndex(neighborId, true);
		}
		if (lightIndex.m_pChunk)
			GetLightData(lightIndex)->SetBrightness((uint8)nLightValue, isSunLight);
	}

	int32 CBlockLightGridClient::ComputeLightValue(uint16 x, int16_t y, uint16 z, bool isSunLight /*= false*/)
	{
		if (isSunLight && CanBlockSeeTheSky(x, y, z))
			return 15;
		else
		{
			BlockTemplate* pTemplate = m_pBlockWorld->GetBlockTemplate(x, y, z);
			int block_lightvalue = pTemplate ? pTemplate->GetLightValue() : 0;
			int lightvalue = isSunLight ? 0 : block_lightvalue;
			int block_lightopacity = pTemplate ? pTemplate->GetLightOpacity() : 1;

			if (block_lightopacity >= 15 && block_lightvalue > 0)
			{
				block_lightopacity = 1;
			}

			if (block_lightopacity < 1)
			{
				block_lightopacity = 1;
			}

			if (block_lightopacity >= 15)
			{
				return 0;
			}
			else if (lightvalue >= 14)
			{
				return lightvalue;
			}
			else
			{
				for (int nFaceIndex = 0; nFaceIndex < 6; ++nFaceIndex)
				{
					int32 neighborX = x + BlockFacing::offsetsXForSide[nFaceIndex];
					int32 neighborY = y + BlockFacing::offsetsYForSide[nFaceIndex];
					int32 neighborZ = z + BlockFacing::offsetsZForSide[nFaceIndex];
					int32 newLightValue = GetSavedLightValue(neighborX, neighborY, neighborZ, isSunLight) - block_lightopacity;

					if (newLightValue > lightvalue)
					{
						lightvalue = newLightValue;
					}

					if (lightvalue >= 14)
					{
						return lightvalue;
					}
				}
				return lightvalue;
			}
		}
	}

	// call this function when the block's light value is no longer valid and need to recalculated. 
	// the old light value of the current cell is used to decide which blocks needs to be recalculated. 
	bool CBlockLightGridClient::RefreshLight(const Uint16x3& blockId_ws, bool isSunLight, int32 nUpdateRange, Scoped_ReadLock<BlockReadWriteLock>* Lock_, int* pnCpuYieldCount)
	{
		// call this function regularly to yield CPU to writer thread only if they are waiting to write data. 
#define REFRESH_LIGHT_CHECK_YIELD_CPU_TO_WRITER   if(Lock_ && Lock_->mutex().HasWaitingWritersAndSingleReader()){ \
	++nYieldCPUTimes;\
	Lock_->unlock(); \
	Lock_->lock(); \
	if(!m_pBlockWorld->IsInBlockWorld())\
		return false;\
			}

		// pass 1, invalidate all dirty blocks light value to 0
		// add all blocks whose light value is equal to current source's light value and its affected area to the queue.
		int nQueuedCount = 0;
		int nYieldCPUTimes = 0;
		int lastLightValue = GetSavedLightValue(blockId_ws.x, blockId_ws.y, blockId_ws.z, isSunLight);
		int newLightValue = ComputeLightValue(blockId_ws.x, blockId_ws.y, blockId_ws.z, isSunLight);
		// we will start refresh from second one in the queue, since the first one is already valid. 
		int nStartRefreshIndex = (newLightValue == lastLightValue && nUpdateRange == 1) ? 1 : 0;

		constexpr int kLightStampDim = 37; // covers [-18, +18] around blockId_ws
		constexpr int kLightStampHalf = 18;
		constexpr int kLightStampPlane = kLightStampDim * kLightStampDim;
		constexpr int kLightStampVol = kLightStampPlane * kLightStampDim;
		struct LightQueueStamp
		{
			std::array<uint16_t, kLightStampVol> stamp{};
			uint16_t generation = 0;
			inline uint16_t NextGen()
			{
				if (++generation == 0)
				{
					stamp.fill(0);
					++generation;
				}
				return generation;
			}
		};
		static thread_local LightQueueStamp s_queueStamp;
		auto calcStampIdx = [&](int x, int y, int z) -> int {
			const int dx = x - static_cast<int>(blockId_ws.x) + kLightStampHalf;
			const int dy = y - static_cast<int>(blockId_ws.y) + kLightStampHalf;
			const int dz = z - static_cast<int>(blockId_ws.z) + kLightStampHalf;
			if (static_cast<unsigned>(dx) >= kLightStampDim ||
				static_cast<unsigned>(dy) >= kLightStampDim ||
				static_cast<unsigned>(dz) >= kLightStampDim)
				return -1;
			return dy * kLightStampPlane + dz * kLightStampDim + dx;
		};

		if (newLightValue > lastLightValue)
		{
			m_blocksNeedLightRecalcuation[nQueuedCount++] = LightBlock(blockId_ws, lastLightValue);
		}
		else if (newLightValue == lastLightValue && nUpdateRange == 0)
		{
			++m_lightCellsVisited;
			REFRESH_LIGHT_CHECK_YIELD_CPU_TO_WRITER;
			return true;
		}
		else
		{
			const uint16_t pass1Gen = s_queueStamp.NextGen();
			const int seedStampIdx = calcStampIdx(blockId_ws.x, blockId_ws.y, blockId_ws.z);
			if (seedStampIdx >= 0)
				s_queueStamp.stamp[seedStampIdx] = pass1Gen;
			m_blocksNeedLightRecalcuation[nQueuedCount++] = LightBlock(blockId_ws, lastLightValue);

			int nIndex = 0;
			while (nIndex < nQueuedCount)
			{
				LightBlock& current = m_blocksNeedLightRecalcuation[nIndex++];
				Uint16x3& curBlockPos = current.blockId;

				int32 lightvalue = GetSavedLightValue(curBlockPos.x, curBlockPos.y, curBlockPos.z, isSunLight);
				if (lightvalue == current.brightness)
				{
					// invalidate the light value to 0
					if (lightvalue > 0 && nIndex > nStartRefreshIndex)
						SetLightValue(curBlockPos.x, curBlockPos.y, curBlockPos.z, 0, isSunLight);

					if (lightvalue > 0 && curBlockPos.AbsDistanceTo(blockId_ws) < 17)
					{
						for (int nFaceIndex = 0; nFaceIndex < 6; ++nFaceIndex)
						{
							int32 neighborX = curBlockPos.x + BlockFacing::offsetsXForSide[nFaceIndex];
							int32 neighborY = curBlockPos.y + BlockFacing::offsetsYForSide[nFaceIndex];
							int32 neighborZ = curBlockPos.z + BlockFacing::offsetsZForSide[nFaceIndex];

							if (!BlockConfig::IsBlockY(neighborY) || neighborX < 0 || neighborZ < 0 || neighborX > 65535 || neighborZ > 65535) continue;
							const int sIdx = calcStampIdx(neighborX, neighborY, neighborZ);
							if (sIdx >= 0 && s_queueStamp.stamp[sIdx] == pass1Gen) continue;
							int32 blockOpacity = Math::Max((int32)1, GetBlockOpacity(neighborX, neighborY, neighborZ));
							int32 lastLightValue = GetSavedLightValue(neighborX, neighborY, neighborZ, isSunLight);

							if ((lastLightValue == (lightvalue - blockOpacity)) && !(lastLightValue == 0 && blockOpacity == 15) && nQueuedCount < (int)m_blocksNeedLightRecalcuation.size())
							{
								Uint16x3 neighborPos((uint16)neighborX, (int16_t)neighborY, (uint16)neighborZ);
								if (!isSunLight || !IsLightDirty(neighborPos))
								{
									if (sIdx >= 0)
										s_queueStamp.stamp[sIdx] = pass1Gen;
									m_blocksNeedLightRecalcuation[nQueuedCount++] = LightBlock(neighborPos, (uint8)lastLightValue);
								}
							}
						}
					}
				}
				else
				{
					current.brightness = (int8)lightvalue;
				}
			}
			m_lightCellsVisited += nIndex;
			REFRESH_LIGHT_CHECK_YIELD_CPU_TO_WRITER;
		}

		//pass 2: relight all blocks in the queue generated above. 
		// recalculate all dirty blocks in pass1(m_blocksNeedLightRecalcuation), by setting each block's new light value to be the largest of its 6 neighbors
		// And if its neighbors needs update, also recalculate recursively. 

		Int32x3 minDirtyBlockId_ws(0x7FFF);
		Int32x3 maxDirtyBlockId_ws(-32768);

#ifdef PRINT_PERF_LOG
		int64 nFromTime = GetTimeUS();
#endif

		const uint16_t pass2InQueueGen = s_queueStamp.NextGen();
		for (int qIdx = 0; qIdx < nQueuedCount; ++qIdx)
		{
			const Uint16x3& qPos = m_blocksNeedLightRecalcuation[qIdx].blockId;
			const int sIdx = calcStampIdx(qPos.x, qPos.y, qPos.z);
			if (sIdx >= 0)
				s_queueStamp.stamp[sIdx] = pass2InQueueGen;
		}

		int nIndex = 0;
		while (nIndex < nQueuedCount)
		{
			LightBlock& current = m_blocksNeedLightRecalcuation[nIndex++];
			Uint16x3& curBlockPos = current.blockId;
			const int curStampIdx = calcStampIdx(curBlockPos.x, curBlockPos.y, curBlockPos.z);
			if (curStampIdx >= 0)
				s_queueStamp.stamp[curStampIdx] = 0;

			int32 lightvalue, newLightValue;

			bool bUpdateNeighbor = false;
			if (nIndex <= nStartRefreshIndex)
			{
				newLightValue = lightvalue = current.brightness;
				bUpdateNeighbor = true;
			}
			else
			{
				lightvalue = GetSavedLightValue(curBlockPos.x, curBlockPos.y, curBlockPos.z, isSunLight);
				newLightValue = ComputeLightValue(curBlockPos.x, curBlockPos.y, curBlockPos.z, isSunLight);
				if (newLightValue != lightvalue)
				{
					SetLightValue(curBlockPos.x, curBlockPos.y, curBlockPos.z, newLightValue, isSunLight);
					AddPointToAABB(curBlockPos, minDirtyBlockId_ws, maxDirtyBlockId_ws);
					bUpdateNeighbor = (newLightValue > lightvalue) && curBlockPos.AbsDistanceTo(blockId_ws) < 17;
				}
				else if (current.brightness != newLightValue)
				{
					// this is required for light value(invalidated to 0, but computed light is also 0) in the first pass. 
					// in such cases, the light value is changed so add to AABB to rebuild the render buffer. 
					AddPointToAABB(curBlockPos, minDirtyBlockId_ws, maxDirtyBlockId_ws);
				}
			}

			if (bUpdateNeighbor && nQueuedCount < (int)(m_blocksNeedLightRecalcuation.size() - 6))
			{
				for (int nFaceIndex = 0; nFaceIndex < 6; ++nFaceIndex)
				{
					int32 neighborX = curBlockPos.x + BlockFacing::offsetsXForSide[nFaceIndex];
					int32 neighborY = curBlockPos.y + BlockFacing::offsetsYForSide[nFaceIndex];
					int32 neighborZ = curBlockPos.z + BlockFacing::offsetsZForSide[nFaceIndex];
					if (!BlockConfig::IsBlockY(neighborY) || neighborX < 0 || neighborZ < 0 || neighborX > 65535 || neighborZ > 65535) continue;
					const int sIdx = calcStampIdx(neighborX, neighborY, neighborZ);
					if (sIdx >= 0 && s_queueStamp.stamp[sIdx] == pass2InQueueGen) continue;
					int32 lastLightValue = GetSavedLightValue(neighborX, neighborY, neighborZ, isSunLight);
					if (lastLightValue < newLightValue)
					{
						Uint16x3 neighborPos((uint16)neighborX, (int16_t)neighborY, (uint16)neighborZ);
						if(!isSunLight || !IsLightDirty(neighborPos))
						{
							if (sIdx >= 0)
								s_queueStamp.stamp[sIdx] = pass2InQueueGen;
							m_blocksNeedLightRecalcuation[nQueuedCount++] = LightBlock(neighborPos, (uint8)lastLightValue);
						}
					}
				}
			}
			REFRESH_LIGHT_CHECK_YIELD_CPU_TO_WRITER;
		}
		m_lightCellsVisited += nQueuedCount;
		SetChunksDirtyInAABB(minDirtyBlockId_ws, maxDirtyBlockId_ws);
		REFRESH_LIGHT_CHECK_YIELD_CPU_TO_WRITER;
		if (pnCpuYieldCount)
		{
			*pnCpuYieldCount += nYieldCPUTimes;
		}
#ifdef PRINT_PERF_LOG
		int64 nCurTime = GetTimeUS();
		if ((nCurTime - nFromTime) > 1000)
		{
			OUTPUT_LOG("Refresh light big duration: %d us QueueCount: %d isSunLight:%d YieldCPUCount:%d\n", (int)(nCurTime - nFromTime), nQueuedCount, isSunLight ? 1 : 0, nYieldCPUTimes);
		}
#endif
		return true;
	}

	void CBlockLightGridClient::EmitSunLight(uint16_t blockIdX_ws, uint16_t blockIdZ_ws, bool bInitialSet)
	{
		if (bInitialSet) return; // Direct sky is sampled from the column height map.
		ChunkMaxHeight heights[6];
		m_pBlockWorld->GetMaxBlockHeightWatchingSky(blockIdX_ws, blockIdZ_ws, heights);
		int high = heights[0].GetMaxHeight();
		int low = heights[0].GetMaxSolidHeight();
		for (int i = 1; i < 5; ++i)
		{
			high = (std::max<int>)(high, heights[i].GetMaxHeight());
			low = (std::min<int>)(low, heights[i].GetMaxSolidHeight());
		}
		if (high >= BlockConfig::MinBlockY)
			QueueSkyInterval(blockIdX_ws, blockIdZ_ws, low, high + 1);
	}

	void CBlockLightGridClient::AddDirtyColumn(uint16_t chunkX_ws, uint16_t chunkZ_ws)
	{
		ChunkLocation chunkPos(chunkX_ws, chunkZ_ws);
		std::unique_lock<std::recursive_mutex> Lock_(m_mutex);
		if (m_dirtyColumns.find(chunkPos) == m_dirtyColumns.end())
		{
			m_dirtyColumns.insert(chunkPos);
			Lock_.unlock();
		}
	}

	void CBlockLightGridClient::RemoveDirtyColumn(const ChunkLocation& curChunkId_ws)
	{
		std::unique_lock<std::recursive_mutex> DirtyColumnLock_(m_mutex);
		m_dirtyColumns.erase(curChunkId_ws);
	}

	int CBlockLightGridClient::GetDirtyColumnCount()
	{
		std::unique_lock<std::recursive_mutex> Lock_(m_mutex);
		return (int)(m_dirtyColumns.size());
	}

	void CBlockLightGridClient::SetColumnPreloaded(uint16_t chunkX_ws, uint16_t chunkZ_ws)
	{
		if (!IsChunkColumnLoaded(chunkX_ws, chunkZ_ws))
		{
			std::lock_guard<std::recursive_mutex> Lock_(m_mutex);
			m_loaded_columns.insert((chunkX_ws << 16) + chunkZ_ws);
			m_dirtyColumns.erase(ChunkLocation(chunkX_ws, chunkZ_ws));
		}
	}

	void CBlockLightGridClient::SetColumnUnloaded(uint16_t chunkX_ws, uint16_t chunkZ_ws)
	{
		std::lock_guard<std::recursive_mutex> Lock_(m_mutex);
		{
			auto iter = m_loaded_columns.find((chunkX_ws << 16) + chunkZ_ws);
			if (iter != m_loaded_columns.end())
			{
				m_loaded_columns.erase(iter);
			}
		}
		ChunkLocation chunkPos(chunkX_ws, chunkZ_ws);
		{
			auto iter = m_dirtyColumns.find(chunkPos);
			if (iter != m_dirtyColumns.end())
			{
				m_dirtyColumns.erase(iter);
			}
		}
	}

	void CBlockLightGridClient::SetLightingInChunkColumnInitialized(uint16_t chunkX_ws, uint16_t chunkZ_ws)
	{
		auto* region = m_pBlockWorld->GetRegion(chunkX_ws >> 5, chunkZ_ws >> 5);
		if (!region) return;
		for (int y : region->GetColumnSections(chunkX_ws, chunkZ_ws))
		{
			BlockChunk* pChunk = m_pBlockWorld->GetChunk(chunkX_ws * 16, y * 16, chunkZ_ws * 16, false);
			if (pChunk)
				pChunk->SetLightingInitialized(true);
		}
	}

	bool CBlockLightGridClient::IsChunkColumnLoadedWorldPos(int nWorldX, int nWorldY, int nWorldZ)
	{
		std::lock_guard<std::recursive_mutex> Lock_(m_mutex);
		return m_loaded_columns.find(((nWorldX >> 4) << 16) + (nWorldZ >> 4)) != m_loaded_columns.end();
	}

	bool CBlockLightGridClient::IsChunkColumnLoaded(int nChunkX, int nChunkZ)
	{
		std::lock_guard<std::recursive_mutex> Lock_(m_mutex);
		return m_loaded_columns.find((nChunkX << 16) + nChunkZ) != m_loaded_columns.end();
	}

	



	void CBlockLightGridClient::SetLightGridSize(int nSize)
	{
		std::lock_guard<std::recursive_mutex> Lock_(m_mutex);
		if (GetLightGridSize() != nSize)
		{
			CBlockLightGridBase::SetLightGridSize(nSize);

			m_minChunkIdX_ws = -1000;
			m_minChunkIdZ_ws = -1000;
			m_maxChunkIdX_ws = -1;
			m_maxChunkIdZ_ws = -1;
			m_minLightBlockIdX = -1000;
			m_minLightBlockIdZ = -1000;
			m_maxLightBlockIdX = -1;
			m_maxLightBlockIdZ = -1;
			if (m_centerChunkIdX_ws > 0 && m_centerChunkIdZ_ws > 0)
			{
				OnWorldMove(m_centerChunkIdX_ws, m_centerChunkIdZ_ws);
			}
		}
	}

	bool CBlockLightGridClient::IsSectionLightPending(int x, int y, int z)
	{
		// Saving holds the world's exclusive lock, including when the worker yields.
		std::lock_guard<std::recursive_mutex> guard(m_mutex);
		for (int dz = -1; dz <= 1; ++dz)
			for (int dx = -1; dx <= 1; ++dx)
			{
				const ChunkLocation column(x + dx, z + dz);
				if (m_dirtyColumns.count(column)) return true;
				for (const auto& active : m_closest_chunks)
					if (active.first.m_chunkX == column.m_chunkX && active.first.m_chunkZ == column.m_chunkZ) return true;
				for (int dy = -1; dy <= 1; ++dy)
				{
					const uint64_t key = (uint64_t(uint16_t(y + dy)) << 48) | (uint64_t(uint16_t(x + dx)) << 32) | (uint64_t(uint16_t(z + dz)) << 16);
					if ((m_processingLightKey & ~uint64_t(0xffff)) == key) return true;
					auto it = m_dirtyCells.lower_bound(key | 0xffff);
					if (it != m_dirtyCells.end() && it->first >= key) return true;
				}
			}
		return false;
	}

	int CBlockLightGridClient::GetDirtyBlockCount()
	{
		return m_nDirtyBlocksCount;
	}

	void CBlockLightGridClient::SetChunksDirtyInAABB(Int32x3 & minDirtyBlockId_ws, Int32x3 & maxDirtyBlockId_ws)
	{
		if (minDirtyBlockId_ws.x > maxDirtyBlockId_ws.x)
			return;
		//extend 1 block in case block happen at the boundary of two chunks
		minDirtyBlockId_ws.x -= 1;
		minDirtyBlockId_ws.y -= 1;
		minDirtyBlockId_ws.z -= 1;
		maxDirtyBlockId_ws.x += 1;
		maxDirtyBlockId_ws.y += 1;
		maxDirtyBlockId_ws.z += 1;

		if (minDirtyBlockId_ws.x < 0)
			minDirtyBlockId_ws.x = 0;
		if (minDirtyBlockId_ws.z < 0)
			minDirtyBlockId_ws.z = 0;
		if (minDirtyBlockId_ws.y < BlockConfig::MinBlockY)
			minDirtyBlockId_ws.y = BlockConfig::MinBlockY;
		if (minDirtyBlockId_ws.y > BlockConfig::MaxBlockY)
			minDirtyBlockId_ws.y = BlockConfig::MaxBlockY;
		if (maxDirtyBlockId_ws.x >= 0xffff)
			maxDirtyBlockId_ws.x = 0xffff - 1;
		if (maxDirtyBlockId_ws.z >= 0xffff)
			maxDirtyBlockId_ws.z = 0xffff - 1;
		if (maxDirtyBlockId_ws.y < BlockConfig::MinBlockY)
			maxDirtyBlockId_ws.y = BlockConfig::MinBlockY;
		if (maxDirtyBlockId_ws.y > BlockConfig::MaxBlockY)
			maxDirtyBlockId_ws.y = BlockConfig::MaxBlockY;

		Uint16x3 startDirtyChunk_ws;
		startDirtyChunk_ws.x = (uint16)(minDirtyBlockId_ws.x >> 4);
		startDirtyChunk_ws.y = (uint16)(minDirtyBlockId_ws.y >> 4);
		startDirtyChunk_ws.z = (uint16)(minDirtyBlockId_ws.z >> 4);

		Uint16x3 endDirtyChunk_ws;
		endDirtyChunk_ws.x = (uint16)(maxDirtyBlockId_ws.x >> 4);
		endDirtyChunk_ws.y = (uint16)(maxDirtyBlockId_ws.y >> 4);
		endDirtyChunk_ws.z = (uint16)(maxDirtyBlockId_ws.z >> 4);

		Uint16x3 curChunk_ws;
		for (uint32_t x = startDirtyChunk_ws.x; x <= endDirtyChunk_ws.x; x++)
		{
			for (int y = startDirtyChunk_ws.y; y <= endDirtyChunk_ws.y; y++)
			{
				for (uint32_t z = startDirtyChunk_ws.z; z <= endDirtyChunk_ws.z; z++)
				{
					Uint16x3 curChunk_ws(x, y, z);
					m_pBlockWorld->SetChunkLightDirty(curChunk_ws);
				}
			}
		}
	}

	void CBlockLightGridClient::AddPointToAABB(const Uint16x3 &curBlockPos, Int32x3 &minDirtyBlockId_ws, Int32x3 &maxDirtyBlockId_ws)
	{
		if (curBlockPos.x > maxDirtyBlockId_ws.x)
			maxDirtyBlockId_ws.x = curBlockPos.x;
		if (curBlockPos.x < minDirtyBlockId_ws.x)
			minDirtyBlockId_ws.x = curBlockPos.x;
		if (curBlockPos.y > maxDirtyBlockId_ws.y)
			maxDirtyBlockId_ws.y = curBlockPos.y;
		if (curBlockPos.y < minDirtyBlockId_ws.y)
			minDirtyBlockId_ws.y = curBlockPos.y;
		if (curBlockPos.z > maxDirtyBlockId_ws.z)
			maxDirtyBlockId_ws.z = curBlockPos.z;
		if (curBlockPos.z < minDirtyBlockId_ws.z)
			minDirtyBlockId_ws.z = curBlockPos.z;
	}

}



