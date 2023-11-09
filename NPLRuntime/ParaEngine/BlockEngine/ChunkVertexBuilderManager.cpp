//-----------------------------------------------------------------------------
// Class:	Building vertex in worker thread
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.11.6
/*
UploadPendingChunksToDevice() generally takes only 500 us per chunk. hence function only takes 0.5-2 ms. 
ProcessOneChunk() is most time consuming. takes 20-30ms per chunk (typically 3000-4000 rect faces). We need to break large chunk rebuild into smaller ones. 
*/
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/CSingleton.h"
#include "RenderableChunk.h"
#include "BlockWorld.h"
#include "ParaTime.h"
#include "ChunkVertexBuilderManager.h"
#include "CoroutineThread.h"

using namespace ParaEngine;

// define to output log for debugging. 
// #define PRINT_CHUNK_LOG

ParaEngine::ChunkVertexBuilderManager::ChunkVertexBuilderManager()
	:m_nMaxPendingChunks(4), m_nMaxUploadingChunks(4), m_bChunkThreadStarted(false),
	m_nMaxChunksToUploadPerTick(8), m_nMaxBytesToUploadPerTick(4*1024*1024), m_pBlockWorld(nullptr)
{

}

ParaEngine::ChunkVertexBuilderManager::~ChunkVertexBuilderManager()
{
	
}

ChunkVertexBuilderManager& ParaEngine::ChunkVertexBuilderManager::GetInstance()
{
	return *(CAppSingleton<ChunkVertexBuilderManager>::GetInstance());
}

bool ParaEngine::ChunkVertexBuilderManager::AddChunk(RenderableChunk* pChunk)
{
	std::unique_lock<std::mutex> Lock_(m_mutex);
	if ((int)m_pendingChunks.size() >= m_nMaxPendingChunks || (int)m_pendingUploadChunks.size() >= m_nMaxUploadingChunks)
		return false;
	if (pChunk && !pChunk->IsBuildingBuffer())
	{
		bool bLastReadyOrEmpty = pChunk->IsReadyOrEmpty();
		pChunk->SetChunkBuildState(RenderableChunk::ChunkBuild_RequestRebuild);
		if (bLastReadyOrEmpty)
			pChunk->IsDirtyByBlockChange(pChunk->GetIsDirtyByBlockChange());
		else
			pChunk->IsDirtyByBlockChange(pChunk->IsDirtyByBlockChange() || pChunk->GetIsDirtyByBlockChange());
		m_pendingChunks.push_back(pChunk);
		Lock_.unlock();
		m_chunk_request_signal.notify_one();
		return true;
	}
	return false;
}

void ParaEngine::ChunkVertexBuilderManager::RemovePendingChunks(std::set<RenderableChunk*>* pExcludeList)
{
	std::unique_lock<std::mutex> Lock_(m_mutex);
	if (!m_pendingChunks.empty())
	{
		if (pExcludeList)
		{
			for (auto it = m_pendingChunks.begin(); it != m_pendingChunks.end();)
			{
				if (pExcludeList->find(*it) == pExcludeList->end())
					it = m_pendingChunks.erase(it);
				else
					it++;
			}
		}
		else
		{
			m_pendingChunks.clear();
		}
	}
}

void ParaEngine::ChunkVertexBuilderManager::Cleanup()
{
	{
		std::unique_lock<std::mutex> Lock_(m_mutex);
		m_pendingUploadChunks.clear();
		m_pendingChunks.clear();
	}
#ifndef EMSCRIPTEN_SINGLE_THREAD
	if (m_pBlockWorld && m_bChunkThreadStarted && m_chunk_build_thread.joinable())
	{
		if (!m_pBlockWorld->GetReadWriteLock().HasWriterLock())
		{
			m_bChunkThreadStarted = false;
			m_chunk_request_signal.notify_one();
			m_chunk_build_thread.join();
		}
		else
		{
			PE_ASSERT(m_pBlockWorld->GetReadWriteLock().IsCurrentThreadHasWriterLock());
			Scoped_WriterUnlock<> unlock_(m_pBlockWorld->GetReadWriteLock());
			m_bChunkThreadStarted = false;
			m_chunk_request_signal.notify_one();
			m_chunk_build_thread.join();
		}
	}
#else
		m_bChunkThreadStarted = false;
#endif	
}

void ParaEngine::ChunkVertexBuilderManager::UploadPendingChunksToDevice()
{
	int nChunkCount = 0;
	int nByteCount = 0;
#ifdef PRINT_CHUNK_LOG
	int64 nFromTime = GetTimeUS();
#endif
	while (true)
	{
		RenderableChunk* pChunk = NULL;
		{
			std::lock_guard<std::mutex> Lock_(m_mutex);
			if (m_pendingUploadChunks.empty())
				break;
			std::stable_sort(m_pendingUploadChunks.begin(), m_pendingUploadChunks.end(), [](RenderableChunk* a, RenderableChunk* b){
				return (a->IsDirtyByBlockChange() && !b->IsDirtyByBlockChange());
			});
			for (auto iter = m_pendingUploadChunks.begin(); iter != m_pendingUploadChunks.end();)
			{
				pChunk = *iter;
				bool bShouldBatchLoadChunk = false;
				int nPendingCount = m_pendingChunks.size();
				bool bDiryBlockBlockChange = pChunk->IsDirtyByBlockChange();
				if (!pChunk->IsDirty() && pChunk->GetChunkViewDistance()<3 && (nPendingCount > 0)
					&& (int)m_pendingUploadChunks.size() < max((int)4, m_nMaxUploadingChunks) )
				{
					// we will handle a very special case here, where pending chunks are neighbors of the uploaded chunks.
					// in such case, we will try to wait until neighbor chunks are also uploaded together 
					// in the same render tick to remove possible visual defects introduced.
					for (RenderableChunk* pPendingChunk : m_pendingChunks)
					{
						auto dPos = pChunk->GetChunkPosWs();
						dPos.Subtract(pPendingChunk->GetChunkPosWs());
						dPos.Abs();
						if ((dPos.x + dPos.y + dPos.z) == 1 && (bDiryBlockBlockChange || pPendingChunk->IsDirtyByBlockChange()))
						{
							bShouldBatchLoadChunk = true;
							break;
						}
					}
					if (!bShouldBatchLoadChunk && !bDiryBlockBlockChange)
					{
						// also compare with current uploading chunks just in case it contains adjacent chunk with dirty blocks.
						for (auto iter1 = m_pendingUploadChunks.begin(); iter1 != iter && iter1 != m_pendingUploadChunks.end(); iter1++)
						{
							auto pChunk1 = *iter1;
							if (pChunk != pChunk1 && pChunk1->IsDirtyByBlockChange())
							{
								auto dPos = pChunk->GetChunkPosWs();
								dPos.Subtract(pChunk1->GetChunkPosWs());
								dPos.Abs();
								if ((dPos.x + dPos.y + dPos.z) == 1)
								{
									bShouldBatchLoadChunk = true;
									break;
								}
							}
						}
					}
				}
				if (!bShouldBatchLoadChunk){
					iter = m_pendingUploadChunks.erase(iter);
					break;
				}
				else{
					pChunk = NULL;
					++iter;
				}
			}
		}
		if (pChunk)
		{
#ifdef PRINT_CHUNK_LOG
			Uint16x3 posChunk = pChunk->GetChunkPosWs();
			OUTPUT_LOG("UploadPendingChunksToDevice: %d %d %d \n", posChunk.x, posChunk.y, posChunk.z);
#endif
			if (!pChunk->IsDirty() && pChunk->GetChunkBuildState() == RenderableChunk::ChunkBuild_RequestBufferUpload)
			{
				// only upload when the chunk is not dirty. 
				pChunk->UploadFromMemoryToDeviceBuffer();
			}
			else
			{
				// if chunk is dirty again, possibly reused when camera moved, we will not upload the buffer. 
				if (pChunk->IsDirty())
				{
					pChunk->ReleaseVertexBuffers();
					pChunk->ClearRenderTasks();
					pChunk->ClearBuilderBuffer();
				}
			}
			std::lock_guard<std::mutex> Lock_(m_mutex);
			pChunk->SetChunkBuildState(RenderableChunk::ChunkBuild_Ready);
			nByteCount += pChunk->GetVertexBufferBytes();
			++nChunkCount;
			if (nChunkCount >= m_nMaxChunksToUploadPerTick || nByteCount >= m_nMaxBytesToUploadPerTick)
				break;
		}
		else
			break;
	}
#ifdef PRINT_CHUNK_LOG
	if (nChunkCount > 0)
		OUTPUT_LOG("UploadPendingChunksToDevice: %d uploaded. Used time:%d us\n", (int)nChunkCount, (int)(GetTimeUS() - nFromTime));
#endif
}

int ParaEngine::ChunkVertexBuilderManager::GetPendingChunksCount()
{
	std::lock_guard<std::mutex> Lock_(m_mutex);
	int nSize = m_pendingChunks.size();
	return nSize;
}

void ParaEngine::ChunkVertexBuilderManager::StartChunkBuildThread(CBlockWorld* pBlockWorld)
{
	if (!m_bChunkThreadStarted)
	{
		Cleanup();
		m_pBlockWorld = pBlockWorld;
		m_bChunkThreadStarted = true;
#ifndef EMSCRIPTEN_SINGLE_THREAD
		m_chunk_build_thread = std::thread(std::bind(&ChunkVertexBuilderManager::ChunkBuildThreadProc, this));
#else
		static CoroutineThread* s_chunk_build_thread = nullptr;
		if (s_chunk_build_thread == nullptr)
		{
			s_chunk_build_thread = CoroutineThread::StartCoroutineThread([this](CoroutineThread* co_thread) -> CO_ASYNC {
				while (true)
				{
					this->ChunkBuildThreadProc();
					CO_AWAIT(co_thread->Sleep(10));
				}
				CO_RETURN;
			}, nullptr);
		}
#endif
	}
}

int ParaEngine::ChunkVertexBuilderManager::ProcessOneChunk(Scoped_ReadLock<BlockReadWriteLock>& ReadWriteLock_)
{
	RenderableChunk* pChunkToBuild = NULL;
	{
		std::lock_guard<std::mutex> Lock_(m_mutex);
		if (m_pendingChunks.size() == 0)
			return 0;
		for (auto itCur = m_pendingChunks.begin(); itCur != m_pendingChunks.end();)
		{
			RenderableChunk* pChunk = *itCur;
			if (pChunk)
			{
				if (!pChunk->IsBuildingBuffer())
				{
					itCur = m_pendingChunks.erase(itCur);
					continue;
				}
				else if (pChunkToBuild == NULL)
				{
					pChunk->SetChunkBuildState(RenderableChunk::ChunkBuild_Rebuilding);
					pChunkToBuild = pChunk;
				}
			}
			itCur++;
		}
	}
	if (pChunkToBuild)
	{
#ifdef PRINT_CHUNK_LOG
		int64 nFromTime = GetTimeUS();
#endif
		int nCpuYieldCount = 0;
		pChunkToBuild->RebuildRenderBufferToMemory(&ReadWriteLock_, &nCpuYieldCount);
#ifdef PRINT_CHUNK_LOG
		Uint16x3 posChunk = pChunkToBuild->GetChunkPosWs();
		OUTPUT_LOG("chunk rebuild: %d %d %d time: %d us cpu yieldtime: %d face count:%d\n", (int)posChunk.x, (int)posChunk.y, (int)posChunk.z, (int)(GetTimeUS() - nFromTime), (int)nCpuYieldCount, (int)(pChunkToBuild->GetTotalFaceCount()));
#endif
		{
			std::lock_guard<std::mutex> Lock_(m_mutex);
			m_pendingChunks.erase(std::remove(m_pendingChunks.begin(), m_pendingChunks.end(), pChunkToBuild), m_pendingChunks.end());
			// move from pending chunks to upload chunks. 
			pChunkToBuild->SetChunkBuildState(RenderableChunk::ChunkBuild_RequestBufferUpload);
			m_pendingUploadChunks.push_back(pChunkToBuild);
		}
		return 1;
	}
	return 0;
}

void ParaEngine::ChunkVertexBuilderManager::ChunkBuildThreadProc()
{
	Scoped_ReadLock<BlockReadWriteLock> lock_(m_pBlockWorld->GetReadWriteLock());
	while (m_bChunkThreadStarted && m_pBlockWorld->IsInBlockWorld())
	{
		if (ProcessOneChunk(lock_) == 0)
		{

			if (m_pendingChunks.empty())
			{
				lock_.unlock();
				{
					std::unique_lock<std::mutex> QueueLock_(m_queueMutex);
					if (m_pendingChunks.empty())
					{
#ifndef EMSCRIPTEN_SINGLE_THREAD
						m_chunk_request_signal.wait(QueueLock_);
#else
						return;
#endif
					}
				}
				lock_.lock();
			}
		}
	}
	lock_.unlock();
	{
		std::unique_lock<std::mutex> Lock_(m_mutex);
		m_pendingUploadChunks.clear();
		m_pendingChunks.clear();
	}
}

int ParaEngine::ChunkVertexBuilderManager::GetMaxChunksToUploadPerTick() const
{
	return m_nMaxChunksToUploadPerTick;
}

void ParaEngine::ChunkVertexBuilderManager::SetMaxChunksToUploadPerTick(int val)
{
	m_nMaxChunksToUploadPerTick = val;
}

int ParaEngine::ChunkVertexBuilderManager::GetMaxBytesToUploadPerTick() const
{
	return m_nMaxBytesToUploadPerTick;
}

void ParaEngine::ChunkVertexBuilderManager::SetMaxBytesToUploadPerTick(int val)
{
	m_nMaxBytesToUploadPerTick = val;
}


int ParaEngine::ChunkVertexBuilderManager::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);

	pClass->AddField("MaxChunksToUploadPerTick", FieldType_Int, (void*)SetMaxChunksToUploadPerTick_s, (void*)GetMaxChunksToUploadPerTick_s, NULL, NULL, bOverride);
	pClass->AddField("MaxBytesToUploadPerTick", FieldType_Int, (void*)SetMaxBytesToUploadPerTick_s, (void*)GetMaxBytesToUploadPerTick_s, NULL, NULL, bOverride);
	pClass->AddField("PendingChunksCount", FieldType_Int, (void*)0, (void*)GetPendingChunksCount_s, NULL, NULL, bOverride);

	return S_OK;
}

