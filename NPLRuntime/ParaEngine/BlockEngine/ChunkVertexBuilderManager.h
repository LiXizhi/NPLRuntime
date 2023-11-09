#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "BlockReadWriteLock.h"
#include "IAttributeFields.h"
#include "CoroutineThread.h"

namespace ParaEngine
{
	class CBlockWorld;
	class BlockWorldClient;

	/** for filling chunk vertex in a different thread. this is usually the lighting thread. */
	class ChunkVertexBuilderManager : public IAttributeFields
	{
	public:
		ChunkVertexBuilderManager();
		virtual ~ChunkVertexBuilderManager();
		ATTRIBUTE_DEFINE_CLASS(ChunkVertexBuilderManager);
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(ChunkVertexBuilderManager, GetMaxChunksToUploadPerTick_s, int*)		{ *p1 = cls->GetMaxChunksToUploadPerTick(); return S_OK; }
		ATTRIBUTE_METHOD1(ChunkVertexBuilderManager, SetMaxChunksToUploadPerTick_s, int)	{ cls->SetMaxChunksToUploadPerTick(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ChunkVertexBuilderManager, GetMaxBytesToUploadPerTick_s, int*)		{ *p1 = cls->GetMaxBytesToUploadPerTick(); return S_OK; }
		ATTRIBUTE_METHOD1(ChunkVertexBuilderManager, SetMaxBytesToUploadPerTick_s, int)	{ cls->SetMaxBytesToUploadPerTick(p1); return S_OK; }

		ATTRIBUTE_METHOD1(ChunkVertexBuilderManager, GetPendingChunksCount_s, int*)		{ *p1 = cls->GetPendingChunksCount(); return S_OK; }

	public:
		static ChunkVertexBuilderManager& GetInstance();

		/** only return true, if chunk is added. it will return false, if max pending chunk is reached. */
		bool AddChunk(RenderableChunk* pChunk);

		/** remove all pending chunks except for those in exclude list. usually called form the main render thread. */
		void RemovePendingChunks(std::set<RenderableChunk*>* pExcludeList);
		
		/** call this function in worker thread every tick to process some chunks.
		* @return number of chunks processed.
		*/
		int ProcessOneChunk(Scoped_ReadLock<BlockReadWriteLock>& ReadWriteLock_);

		/** this function should be called from the render thread to upload all pending chunks.  */
		void UploadPendingChunksToDevice();

		void Cleanup();

		int GetPendingChunksCount();

		void StartChunkBuildThread(CBlockWorld* pBlockWorld);

		int GetMaxChunksToUploadPerTick() const;
		void SetMaxChunksToUploadPerTick(int val);
		int GetMaxBytesToUploadPerTick() const;
		void SetMaxBytesToUploadPerTick(int val);

	protected:
		void ChunkBuildThreadProc();
	protected:
		// weak references, no need to release them.
		// chunks that need to be rebuild. 
		std::vector<RenderableChunk*> m_pendingChunks;
		// chunks that needing uploading to device
		std::vector<RenderableChunk*> m_pendingUploadChunks;
		std::mutex m_mutex;
		std::mutex m_queueMutex;
#ifndef EMSCRIPTEN_SINGLE_THREAD
		std::thread m_chunk_build_thread;
#endif
		std::condition_variable m_chunk_request_signal;
		bool m_bChunkThreadStarted;
		CBlockWorld* m_pBlockWorld;
		int m_nMaxPendingChunks;
		int m_nMaxUploadingChunks;
		int m_nMaxChunksToUploadPerTick;
		int m_nMaxBytesToUploadPerTick;
		
		friend class CBlockWorld;
		friend class BlockWorldClient;
	};
}