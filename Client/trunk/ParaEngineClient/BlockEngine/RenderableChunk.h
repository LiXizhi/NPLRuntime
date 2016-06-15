#pragma once
#include "BlockRenderTask.h"
#include "BlockCommon.h"
#include "ShapeAABB.h"
#include "BlockReadWriteLock.h"

namespace ParaEngine
{
	class BlockRegion;
	class BlockRenderTask;
	class Block;
	class BlockChunk;
	class CBlockWorld;
	class ParaVertexBufferPool;
	class BlockGeneralTessellator;

	class RenderableChunk
	{
	public:
		enum ChunkBuildState
		{
			// chunk vertex buffer is ready on device
			ChunkBuild_Ready,
			// main thread is requesting to rebuild vertex buffer
			ChunkBuild_RequestRebuild,
			// chunk data is rebuilding in (possibly in another thread). 
			ChunkBuild_Rebuilding,
			// building thread has finished, and request the main thread to upload the buffer
			ChunkBuild_RequestBufferUpload,
			// initial state: chunk has never been built before. 
			ChunkBuild_empty,
		};


		RenderableChunk();
		~RenderableChunk();

		void ReuseChunk(BlockRegion* m_ownerBlockRegion,int16_t chunkIndex);

		void ClearChunkData();

		void ReleaseVertexBuffers();

		void Reset();

		/** every frame, we will ensure some new buffer is updated, and some updated buffer is updated as well. 
		* @param bNewBuffer: if true, we will update buffer that has not been created yet. 
		* @param bUpdatedBuffer: if true, we will update buffer within which some light is updated. 
		* return true if the buffer is updated. and false, it is not dirty. 
		*/
		bool ShouldRebuildRenderBuffer(CBlockWorld* pWorld, bool bNewBuffer = true, bool bUpdatedBuffer = true);

		/** every frame, we will ensure some new buffer is updated, and some updated buffer is updated as well.
		* @param bAsyncMode: true to update in a separate thread.
		*/
		bool RebuildRenderBuffer(CBlockWorld* pWorld, bool bAsyncMode);


		/** fill the render queue with buffer data. please note, it does not fill if the buffer has not been created before. */
		void FillRenderQueue(CBlockWorld* pWorld);

		void OnLeaveWorld();

		void DeleteDeviceObjects();

		void ClearRenderTasks();

		void RendererRecreated();
		bool IsEmptyChunk();

		inline uint16_t GetChunkIndex(){return m_packedChunkID;}

		/** chunk position in world space.  */
		Int16x3 GetChunkPosWs();

		/** get the associated raw chunk data */
		BlockChunk* GetChunk();

		void FillRenderQueue(CBlockWorld* pWorld,Vector3& renderOfs,float verticalOffset);

		/** only dirty if chunk is first loaded or reused and all render buffer content can not be used to render
		*/
		void SetChunkDirty(bool isDirty)
		{
			m_isDirty = isDirty;
		}

		/** only dirty if chunk is first loaded or reused and all render buffer content can not be used to render
		* this has NOTHING to do when the actual block or light content of the chunk has changed or not.
		*/
		bool IsDirty() const { 
			return m_isDirty; 
		}

		bool IsDirtyByNeighbor();
		/** whether this chunk is made dirty by actual block changes. */
		bool GetIsDirtyByBlockChange();

		/** whether the chunk intersect with the given sphere. used in fast view culling. */
		bool IsIntersect(CShapeSphere& sphere);

		/** return true if the nearby 4 chunk columns have been generated (and loaded) */
		bool IsNearbyChunksLoaded();

		RenderableChunk::ChunkBuildState GetChunkBuildState() const;
		bool IsBuildingBuffer() const;
		bool IsReadyOrEmpty() const;
		void SetChunkBuildState(RenderableChunk::ChunkBuildState val);

		static void StaticInit();
		static void StaticRelease();
		static ParaVertexBufferPool* GetVertexBufferPool();
		
		/** only used when the chunk is destroyed. do not call this function is other cases.*/
		void WaitUntilChunkReady();

		/** call this function to generate all vertices to memory buffer.  
		* this functions take 20-30ms to complete. Better run from another thread. 
		* @param Lock_: if this function is called from chunk builder thread, we need to pass the lock, 
		* so that we only process nMaxBlockPerStep of blocks per step and yield CPU to main render thread. 
		* @param pnCpuYieldCount: how many times we have yield CPU to writer thread. 
		* @return total face count is returned.
		*/
		void RebuildRenderBufferToMemory(Scoped_ReadLock<BlockReadWriteLock>* Lock_, int* pnCpuYieldCount);
		void ClearBuilderBuffer();
		/** only call in main thread */
		void UploadFromMemoryToDeviceBuffer();

		/* total number of faces. */
		int32 GetTotalFaceCount() const;

		/** how many render ticks that this chunk has been delayed from buffer rebuilding.
		If a chunk has been delayed for too long, we will force it to rebuild even it is farther from the camera than other dirty chunks with smaller delayed ticks. */
		int32 GetDelayedRebuildTick() const;
		void SetDelayedRebuildTick(int32 val);

		/** distance to current camera in chunk space. */
		int16 GetChunkViewDistance() const;
		void SetChunkViewDistance(int16 val);

		/** each chunk has a unique view index, the smaller the closer to the current view center. */
		int16 GetViewIndex() const;
		void SetViewIndex(int16 val);

		/** when this chunk should be rendered last time. we will likely to remove old chunks when memory is small. */
		int GetRenderFrameCount() const;
		void SetRenderFrameCount(int val);

		/** get the current size of all of its vertex buffer. */
		int GetVertexBufferBytes();
		/** this is usually same as GetVertexBufferBytes(), except that when we clear vertex buffer, 
		this size remain unchanged until the chunk is reused. It will internally call GetVertexBufferBytes() to refresh size*/
		int GetLastBufferBytes();

		/** total number of renderable chunks in memory. */
		static int GetTotalRenderableChunks();

		std::vector<BlockRenderTask*> GetRenderTasks();
	
		const CShapeBox& GetShapeAABB() const;
		
		/** this determined how the vertex buffer format is.
		* this is -1, pWorld->GetBlockRenderMethod() is used. otherwise it is one of the BlockRenderMethod enumeration
		*/
		int GetShaderID() const;
		void SetShaderID(int val);

		/** for main renderer (default to true), we will set chuck dirty to false, whenever the buffer is rebuilt. */
		bool IsMainRenderer() const;
		void SetIsMainRenderer(bool val);

		bool IsDirtyByBlockChange() const;
		void IsDirtyByBlockChange(bool val);
	public:
		struct InstanceGroup
		{
			BlockTemplate* m_pTemplate;
			uint32_t m_blockData;
			uint32_t m_nFaceCount;

			//packedBlockId
			std::vector<uint16_t> instances;
			InstanceGroup() :m_pTemplate(NULL), m_blockData(0), m_nFaceCount(0){}
			inline void reset(){
				m_pTemplate = 0;
				m_nFaceCount = 0;
				instances.clear();
			}
			inline bool isEmpty() const {
				return !(m_pTemplate || instances.size() > 0);
			}
			inline void AddFace(uint32_t nCount){
				m_nFaceCount += nCount;
			}
			inline void AddInstance(uint16_t nIndex, uint32_t nFaceCount = 0){
				instances.push_back(nIndex);
				AddFace(nFaceCount);
			}
			inline uint32_t GetFaceCount() const { return m_nFaceCount; }
			void operator+=(InstanceGroup& right){
				AddFace(right.GetFaceCount());
				instances.insert(instances.end(), right.instances.begin(), right.instances.end());
			}
			/** whether this instance group can share vertex buffer with the other instance, providing that both instance are from the same chunk.
			* this allows us to batch different blocks sharing same texture(altas) into one draw call.
			* @param pOther: if NULL, it always return false;
			*/
			bool CanShareRenderBufferWith(InstanceGroup* pOther = NULL);
		};
		/** this function returns thread local data */
		std::vector<InstanceGroup* >& GetInstanceGroups();

		/** mapping from a hashed value of block_template id and template data if any. */
		std::map<int32_t, int>& GetInstanceMap();
		
		static void StaticReleaseInstGroup(std::vector<RenderableChunk::InstanceGroup* >* pInstances);

		/** get thread local block tessellator */
		BlockGeneralTessellator& GetBlockTessellator();

	protected:
		void BuildRenderGroup();

		void ResetInstanceGroups();

		int32 BuildInstanceGroupsByIdAndData(BlockChunk* pChunk);
		void SortAndMergeInstanceGroupsByTexture();

		/** each rectangle face is 2 triangles or 4 vertices. */
		ParaVertexBuffer* RequestVertexBuffer(int32 nFaceCountInVertexBuffer);
		ParaMemoryBuffer RequestMemoryBuffer(int32 nFaceCountInVertexBuffer, int32* pBufferIndex);

	protected:
		std::vector<BlockRenderTask*> m_builder_tasks;
		std::vector<ParaMemoryBuffer> m_memoryBuffers; 
		
	private:
		/** total number of renderable chunks in memory. */
		static int s_nTotalRenderableChunks;
		CBlockWorld* m_pWorld;
		int16_t m_regionX, m_regionZ;
		int16_t m_packedChunkID;
		bool m_isDirty;
		int32 m_nRenderFrameCount;
		int32 m_nLastVertexBufferBytes;
		/** how many render ticks that this chunk has been delayed from buffer rebuilding. 
		If a chunk has been delayed for too long, we will force it to rebuild even it is farther from the camera than other dirty chunks with smaller delayed ticks. */
		int32 m_nDelayedRebuildTick;
		/** distance to current camera in chunk space. */
		int16 m_nChunkViewDistance;
		/** each chunk has a unique view index, the smaller the closer to the current view center. */
		int16 m_nViewIndex;
		
		ChunkBuildState m_chunkBuildState;
		
		/** this determined how the vertex buffer format is.
		* this is -1, pWorld->GetBlockRenderMethod() is used. otherwise it is one of the BlockRenderMethod enumeration
		*/
		int m_dwShaderID;

		std::vector<BlockRenderTask*> m_renderTasks;
		std::vector<ParaVertexBuffer*> m_vertexBuffers;
		CShapeBox m_pShapeAABB;

		int32 m_totalFaceCount;

		/** for main renderer (default to true), we will set chuck dirty to false, whenever the buffer is rebuilt. */
		bool m_bIsMainRenderer : 1;
		/** whether dirty by block change when this chunk is added.*/
		bool m_bIsDirtyByBlockChange : 1;
	};
}
