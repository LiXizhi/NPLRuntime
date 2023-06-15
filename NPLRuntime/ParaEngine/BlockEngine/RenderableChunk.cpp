//-----------------------------------------------------------------------------
// Class:	Renderable chunks
// Authors:	LiXizhi, Clayman
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2012.11.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/tss.hpp>
#endif
#include "RenderableChunk.h"
#include "ParaVertexBufferPool.h"
#include "BlockRegion.h"
#include "BlockModel.h"
#include "BlockChunk.h"
#include "BlockWorld.h"
#include "BlockCommon.h"
#include "BlockTessellators.h"
#include "VertexFVF.h"
#include "ChunkVertexBuilderManager.h"


namespace ParaEngine
{
	int RenderableChunk::s_nTotalRenderableChunks = 0;

	RenderableChunk::RenderableChunk()
		:m_pWorld(NULL), m_chunkBuildState(ChunkBuild_empty), m_nDelayedRebuildTick(0), m_nChunkViewDistance(0), m_nViewIndex(0), m_nRenderFrameCount(0), m_nLastVertexBufferBytes(0), m_dwShaderID(-1), m_bIsMainRenderer(true), m_bIsDirtyByBlockChange(true)
	{
		m_isDirty = true;
		m_regionX = -1;
		m_regionZ = -1;
		m_packedChunkID = 0;
		m_totalFaceCount = 0;
		s_nTotalRenderableChunks++;
	}

	RenderableChunk::~RenderableChunk()
	{
		WaitUntilChunkReady();
		ClearBuilderBuffer();
		DeleteDeviceObjects();
		--s_nTotalRenderableChunks;
	}


	bool RenderableChunk::ShouldRebuildRenderBuffer(CBlockWorld* pWorld, bool bNewBuffer /*= true*/, bool bUpdatedBuffer /*= true*/)
	{
		BlockRegion * pOwnerBlockRegion = pWorld->GetRegion(m_regionX, m_regionZ);
		if (!pOwnerBlockRegion || pOwnerBlockRegion->IsLocked())
			return false;
		else
			m_pWorld = pOwnerBlockRegion->GetBlockWorld();

		return ((bNewBuffer && m_isDirty) ||
			(bUpdatedBuffer && pOwnerBlockRegion->IsChunkDirty(m_packedChunkID))) && !IsBuildingBuffer();
	}

	bool RenderableChunk::RebuildRenderBuffer(CBlockWorld* pWorld, bool bAsyncMode)
	{
		BlockRegion * pOwnerBlockRegion = pWorld->GetRegion(m_regionX, m_regionZ);
		if (pOwnerBlockRegion && !pOwnerBlockRegion->IsLocked())
		{
			if (bAsyncMode)
			{
				return ChunkVertexBuilderManager::GetInstance().AddChunk(this);
			}
			else
			{
				BuildRenderGroup();
				m_isDirty = false;
				if (IsMainRenderer())
					pOwnerBlockRegion->SetChunkDirty(m_packedChunkID, false);
				SetChunkBuildState(RenderableChunk::ChunkBuild_Ready);

				// transfer ownership from builder task to render task. 
				m_renderTasks = m_builder_tasks;
				m_builder_tasks.clear();
				ClearBuilderBuffer();
				return true;
			}
		}
		return true;
	}


	void RenderableChunk::FillRenderQueue(CBlockWorld* pWorld, Vector3& renderOfs, float verticalOffset)
	{
		// do not render dirty ones. 
		if (m_isDirty || m_renderTasks.size() == 0)
			return;

		BlockRegion * pOwnerBlockRegion = pWorld->GetRegion(m_regionX, m_regionZ);
		// if (!pOwnerBlockRegion || pOwnerBlockRegion->IsLocked())
		if (!pOwnerBlockRegion)  // we will render, even if it is locked. 
			return;
		else
			m_pWorld = pOwnerBlockRegion->GetBlockWorld();


		uint16_t maxDistValue = 0xfff;
		uint16_t dist = 0;
		bool bGroupByChunk = m_pWorld->IsGroupByChunkBeforeTexture();
		if (bGroupByChunk)
		{
			dist = (uint16_t)GetViewIndex();
		}
		else
		{
			Uint16x3 eyeChunk = pWorld->GetEyeChunkId();
			Uint16x3 curChunk;
			UnpackChunkIndex(m_packedChunkID, curChunk.x, curChunk.y, curChunk.z);

			curChunk.x += pOwnerBlockRegion->m_minChunkId_ws.x;
			curChunk.y += pOwnerBlockRegion->m_minChunkId_ws.y;
			curChunk.z += pOwnerBlockRegion->m_minChunkId_ws.z;

			int16_t deltaX = (int16)eyeChunk.x - (int16)curChunk.x;
			int16_t deltaY = (int16)eyeChunk.y - (int16)curChunk.y;
			int16_t deltaZ = (int16)eyeChunk.z - (int16)curChunk.z;

			//pack distance as 12 bit value
			int32_t dist2 = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;

			int32_t maxDist2 = pWorld->GetActiveChunkDim() * pWorld->GetActiveChunkDim() * 3;
			float r = (float)dist2 / maxDist2;
			if (r > 1)
				r = 1;

			dist = (uint16_t)(r * maxDistValue);
		}

		for (size_t i = 0; i < m_renderTasks.size(); i++)
		{
			BlockRenderTask *task = m_renderTasks[i];
			uint16_t priority = BlockTemplate::g_maxRenderPriority - task->GetTemplate()->GetRenderPriority();

			if (task->GetTemplate()->GetRenderPass() == BlockRenderPass_AlphaBlended)
			{
				//invert distant for alpha blended object
				uint32_t renderOrder = ((maxDistValue - dist) << 16);
				renderOrder += (task->GetTemplateId() + (task->GetTemplate()->GetCategoryID() << 8));
				renderOrder += (priority << 28);
				task->SetRenderOrder(renderOrder);
			}
			else
			{
				uint32_t renderOrder = (priority << 28);
				if (bGroupByChunk)
				{
					renderOrder += (dist << 16);
					renderOrder += (task->GetTemplateId() + (task->GetTemplate()->GetCategoryID() << 8));
				}
				else
				{
					renderOrder += ((task->GetTemplateId() + (task->GetTemplate()->GetCategoryID() << 8)) << 12);
					renderOrder += dist;
				}
				task->SetRenderOrder(renderOrder);
			}

			pWorld->AddRenderTask(task);
		}
	}

	int32 RenderableChunk::GetTotalFaceCount() const
	{
		return m_totalFaceCount;
	}

	void RenderableChunk::BuildRenderGroup()
	{
		if (!m_pWorld)
			return;

		BlockRegion * pOwnerBlockRegion = m_pWorld->GetRegion(m_regionX, m_regionZ);
		if (!pOwnerBlockRegion)
			return;

		ClearRenderTasks();
		ReleaseVertexBuffers();
		BlockChunk* pChunk = pOwnerBlockRegion->GetChunk(m_packedChunkID, false);
		if (!pChunk)
		{
			return;
		}

		//------------------------------------------------------------------------
		//fill instance group
		ResetInstanceGroups();
		int32 totalFaceCount = BuildInstanceGroupsByIdAndData(pChunk);

		if (totalFaceCount <= 0)
		{
			m_totalFaceCount = totalFaceCount;
			return;
		}

		SortAndMergeInstanceGroupsByTexture();

		//----------------------------------------------------------
		//2.create a big buffer to hold all blocks

		uint32_t bufferSize = sizeof(BlockVertexCompressed) * (totalFaceCount * 4);

		BlockRenderMethod dwShaderID = (BlockRenderMethod)GetShaderID();

		m_totalFaceCount = totalFaceCount;
		int32 nFaceCountCompleted = 0;

		const int32 maxFaceCountPerBatch = BlockConfig::g_maxFaceCountPerBatch;

		BlockGeneralTessellator& tessellator = GetBlockTessellator();
		//-------------------------------------------------------------
		//3.fill buffer

		int32 nFreeFaceCountInVertexBuffer = Math::Min(maxFaceCountPerBatch, m_totalFaceCount - nFaceCountCompleted);
		ParaVertexBuffer* pVertexBuffer = RequestVertexBuffer(nFreeFaceCountInVertexBuffer);
		if (!pVertexBuffer)
			return;
		BlockVertexCompressed* pVertices;
		pVertexBuffer->Lock((void**)&pVertices);
		uint32_t vertexOffset = 0;

		std::vector<InstanceGroup* >& instanceGroups = GetInstanceGroups();

		int nSize = (int)instanceGroups.size();
		InstanceGroup* pInstGroup = NULL;
		InstanceGroup* pLastInstGroup = NULL;
		BlockRenderTask *pTask = NULL;
		for (int i = 0; (i < nSize && (pInstGroup = instanceGroups[i])->instances.size()>0); i++)
		{
			BlockTemplate* pTemplate = pInstGroup->m_pTemplate;
			uint32_t nBlockData = pInstGroup->m_blockData;
			std::vector<uint16_t>& instanceGroup = pInstGroup->instances;
			uint32 groupSize = instanceGroup.size();
			uint32 instCount = groupSize;
			int nMaxFaceCountPerInstance = pTemplate->GetBlockModelByData(nBlockData).GetFaceCount();

			if (nFreeFaceCountInVertexBuffer < (int32)pInstGroup->GetFaceCount())
			{
				if (nFreeFaceCountInVertexBuffer < (maxFaceCountPerBatch*0.1))
				{
					pVertexBuffer->Unlock();
					nFreeFaceCountInVertexBuffer = Math::Min(maxFaceCountPerBatch, m_totalFaceCount - nFaceCountCompleted);
					pVertexBuffer = RequestVertexBuffer(nFreeFaceCountInVertexBuffer);
					if (!pVertexBuffer)
						return;
					pVertexBuffer->Lock((void**)&pVertices);
					vertexOffset = 0;
					pLastInstGroup = NULL;
				}
				if (nFreeFaceCountInVertexBuffer < (int32)pInstGroup->GetFaceCount())
				{
					instCount = nFreeFaceCountInVertexBuffer / nMaxFaceCountPerInstance;
				}
				else
				{
					instCount = groupSize;
				}
			}

			//add render task
			if (!pTask || !(pInstGroup->CanShareRenderBufferWith(pLastInstGroup)))
			{
				pTask = BlockRenderTask::CreateTask();
				pTask->Init(pTemplate, nBlockData, vertexOffset, pVertexBuffer->GetDevicePointer(), pChunk->m_minBlockId_ws);
				m_builder_tasks.push_back(pTask);
				pLastInstGroup = pInstGroup;
			}

			int32 batchInstCount = 0;
			int32 unprocessedInstCount = groupSize;
			for (int inst = 0; inst < (int)groupSize; inst++)
			{
				//start a new one if instances can't fit into one batch
				batchInstCount++;
				if (nFreeFaceCountInVertexBuffer < nMaxFaceCountPerInstance)
				{
					pVertexBuffer->Unlock();
					nFreeFaceCountInVertexBuffer = Math::Min(maxFaceCountPerBatch, m_totalFaceCount - nFaceCountCompleted);
					pVertexBuffer = RequestVertexBuffer(nFreeFaceCountInVertexBuffer);
					if (!pVertexBuffer)
						return;
					pVertexBuffer->Lock((void**)&pVertices);
					vertexOffset = 0;

					if (nFreeFaceCountInVertexBuffer < unprocessedInstCount*nMaxFaceCountPerInstance)
					{
						instCount = nFreeFaceCountInVertexBuffer / nMaxFaceCountPerInstance;
					}
					else
					{
						instCount = unprocessedInstCount;
					}
					pTask = BlockRenderTask::CreateTask();
					pTask->Init(pTemplate, nBlockData, vertexOffset, pVertexBuffer->GetDevicePointer(), pChunk->m_minBlockId_ws);
					m_builder_tasks.push_back(pTask);
					pLastInstGroup = pInstGroup;

					batchInstCount = 1;
				}

				//--------------------------------------------------------------
				// assemble block model data
				//--------------------------------------------------------------
				BlockVertexCompressed* pBlockModelVertices = NULL;
				unprocessedInstCount--;
				int32 nFaceCount = tessellator.TessellateBlock(pChunk, instanceGroup[inst], dwShaderID, &pBlockModelVertices);
				if (nFaceCount > 0)
				{
					int32 nVertexCount = nFaceCount * 4;
					if (nFreeFaceCountInVertexBuffer >= nFaceCount)
					{
						memcpy(pVertices, pBlockModelVertices, sizeof(BlockVertexCompressed)*nVertexCount);
						pVertices += nVertexCount;
						vertexOffset += nVertexCount;

						nFaceCountCompleted += nFaceCount;
						pTask->AddRectFace(nFaceCount);
						nFreeFaceCountInVertexBuffer -= nFaceCount;
					}
					else
					{
						// this could happen when block changes when we are still processing it
						OUTPUT_LOG("fatal error: not enough face count in vertex buffer. \n");
					}
				}
			}
		}
		pVertexBuffer->Unlock();

	}

	void RenderableChunk::OnLeaveWorld()
	{
		DeleteDeviceObjects();
		ClearBuilderBuffer();
		Reset();
	}

	void RenderableChunk::DeleteDeviceObjects()
	{
		ClearRenderTasks();
		ReleaseVertexBuffers();
	}

	void RenderableChunk::RendererRecreated()
	{
		ClearRenderTasks();
		m_vertexBuffers.clear();
	}

	void RenderableChunk::ReuseChunk(BlockRegion* pOwnerBlockRegion, int16_t packedChunkId_rs)
	{
		ClearChunkData();
		m_nLastVertexBufferBytes = 0;
		if (pOwnerBlockRegion)
		{
			m_regionX = pOwnerBlockRegion->GetRegionX();
			m_regionZ = pOwnerBlockRegion->GetRegionZ();
			m_pWorld = pOwnerBlockRegion->GetBlockWorld();
		}
		else
		{
			m_regionX = -1;
			m_regionZ = -1;
			m_pWorld = NULL;
		}
		m_packedChunkID = packedChunkId_rs;

		if (pOwnerBlockRegion == nullptr || packedChunkId_rs == -1)
		{
			return;
		}

		Uint16x3 chunkId_rs;
		UnpackChunkIndex(m_packedChunkID, chunkId_rs.x, chunkId_rs.y, chunkId_rs.z);

		Uint16x3 minRegionBlockId = pOwnerBlockRegion->m_minBlockId_ws;
		float blockSize = BlockConfig::g_blockSize;

		Vector3 vMinWorld, vMaxWorld;
		vMinWorld.x = minRegionBlockId.x * blockSize;
		vMinWorld.x += chunkId_rs.x * BlockConfig::g_chunkSize;

		vMinWorld.y = minRegionBlockId.y * blockSize;
		vMinWorld.y += chunkId_rs.y * BlockConfig::g_chunkSize + pOwnerBlockRegion->GetBlockWorld()->GetVerticalOffset();

		vMinWorld.z = minRegionBlockId.z  * blockSize;
		vMinWorld.z += chunkId_rs.z * BlockConfig::g_chunkSize;

		vMaxWorld = vMinWorld + Vector3::UNIT_SCALE * (blockSize * BlockConfig::g_chunkBlockDim);

		m_pShapeAABB.SetMinMax(vMinWorld, vMaxWorld);
	}

	void RenderableChunk::Reset()
	{
		m_regionX = -1;
		m_regionZ = -1;
		m_isDirty = false;
		m_nDelayedRebuildTick = 0;
	}

	bool RenderableChunk::IsEmptyChunk()
	{
		return GetChunk() == NULL;
	}

	bool RenderableChunk::IsDirtyByNeighbor()
	{
		auto pChunk = GetChunk();
		return pChunk && pChunk->IsDirtyByNeighbor();
	}

	bool RenderableChunk::GetIsDirtyByBlockChange()
	{
		auto pChunk = GetChunk();
		return pChunk && pChunk->IsDirtyByBlockChange();
	}

	bool RenderableChunk::IsIntersect(CShapeSphere& sphere)
	{
		return !IsEmptyChunk() && m_pShapeAABB.Intersect(sphere);
	}

	void RenderableChunk::StaticInit()
	{

	}

	void RenderableChunk::StaticRelease()
	{
		BlockRenderTask::ReleaseTaskPool();
	}

	BlockChunk* RenderableChunk::GetChunk()
	{
		if (m_regionX >= 0 && m_pWorld)
		{
			BlockRegion * pOwnerBlockRegion = m_pWorld->GetRegion(m_regionX, m_regionZ);
			if (pOwnerBlockRegion)
				return pOwnerBlockRegion->GetChunk(m_packedChunkID, false);
		}
		return NULL;
	}

	bool RenderableChunk::IsNearbyChunksLoaded()
	{
		BlockChunk* pChunk = GetChunk();
		return pChunk && pChunk->IsNearbyChunksLoaded();
	}


	void RenderableChunk::ClearBuilderBuffer()
	{
		for (auto& task : m_builder_tasks)
			BlockRenderTask::ReleaseTask(task);
		m_builder_tasks.clear();

		for (auto& buf : m_memoryBuffers)
		{
			buf.ReleaseBuffer();
		}
		m_memoryBuffers.clear();
	}

	void RenderableChunk::ClearRenderTasks()
	{
		for (uint32_t i = 0; i < m_renderTasks.size(); i++)
			BlockRenderTask::ReleaseTask(m_renderTasks[i]);
		m_renderTasks.clear();
	}

	void RenderableChunk::ReleaseVertexBuffers()
	{
		ParaVertexBufferPool* pPool = GetVertexBufferPool();

		for (auto vertexBuffer : m_vertexBuffers)
		{
			pPool->ReleaseBuffer(vertexBuffer);
		}
		m_vertexBuffers.clear();
	}

	ParaVertexBufferPool* RenderableChunk::GetVertexBufferPool()
	{
		static std::string s_pool_name = "chunk";
		return CVertexBufferPoolManager::GetInstance().CreateGetPool(s_pool_name);
	}

	ParaVertexBuffer* RenderableChunk::RequestVertexBuffer(int32 nFreeFaceCountInVertexBuffer)
	{
		ParaVertexBuffer* pVertexBuffer = GetVertexBufferPool()->CreateBuffer(nFreeFaceCountInVertexBuffer * sizeof(BlockVertexCompressed) * 4, block_vertex::FVF, D3DUSAGE_WRITEONLY);
		if (pVertexBuffer && pVertexBuffer->IsValid())
		{
			m_vertexBuffers.push_back(pVertexBuffer);
			return pVertexBuffer;
		}
		else
		{
			OUTPUT_LOG("warn: failed to CreateVertexBuffer of size %d for renderable chunk\n", nFreeFaceCountInVertexBuffer);
			return NULL;
		}
	}

	void RenderableChunk::StaticReleaseInstGroup(std::vector<RenderableChunk::InstanceGroup* >* pInstances)
	{
		for (int i = 0; i < (int)pInstances->size(); i++)
		{
			SAFE_DELETE((*pInstances)[i]);
		}
		pInstances->clear();
		delete pInstances;
	}

	BlockGeneralTessellator& RenderableChunk::GetBlockTessellator()
	{
		thread_local static BlockGeneralTessellator tessellator(m_pWorld);
		tessellator.SetWorld(m_pWorld);
		return tessellator;
	}

	bool RenderableChunk::IsDirtyByBlockChange() const
	{
		return m_bIsDirtyByBlockChange;
	}

	void RenderableChunk::IsDirtyByBlockChange(bool val)
	{
		m_bIsDirtyByBlockChange = val;
	}

	std::vector<RenderableChunk::InstanceGroup* >& RenderableChunk::GetInstanceGroups()
	{
		thread_local static std::vector<InstanceGroup*> tls_instanceGroups;
		return tls_instanceGroups;
	}

	std::map<int64_t, int>& RenderableChunk::GetInstanceMap()
	{
		thread_local static std::map<int64_t, int> tls_instance_map;
		return tls_instance_map;
	}

	void RenderableChunk::ResetInstanceGroups()
	{
		std::vector<InstanceGroup* >& instanceGroups = GetInstanceGroups();
		int nInstanceSize = (int)instanceGroups.size();
		for (int i = 0; i < nInstanceSize; i++)
		{
			InstanceGroup* group = (instanceGroups[i]);
			if (!group->isEmpty())
				group->reset();
			else
				break;
		}
		if (nInstanceSize == 0)
			instanceGroups.push_back(new InstanceGroup());

		GetInstanceMap().clear();
	}

	int32 RenderableChunk::BuildInstanceGroupsByIdAndData(BlockChunk* pChunk)
	{
		int32 totalFaceCount = 0;
		int32 cachedGroupIdx = 0;
		uint16_t nSize = (uint16_t)pChunk->m_blockIndices.size();
		std::map < int64_t, int > & instance_map = GetInstanceMap();
		std::vector<InstanceGroup* >& instanceGroups = GetInstanceGroups();

		for (uint16_t i = 0; i < nSize; i++)
		{
			Block* pBlock = pChunk->GetBlock(i);
			if (!pBlock)
				continue;

			if (pBlock->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_cubeModel)
				&& pChunk->IsVisibleBlock(i, pBlock))
			{
				//find correct group,nearby blocks may use the same template,so we 
				//compare it with current group first.
				uint32 nBlockID = pBlock->GetTemplate()->GetID();
				uint32 nBlockData = pBlock->GetTemplate()->HasColorData() ? 0 : pBlock->GetUserData();
				BlockModel& blockmodel = pBlock->GetTemplate()->GetBlockModelByData(nBlockData);
				if (nBlockData > 0)
					nBlockID = ((nBlockData << 12) | nBlockID);
				auto curIndex = instance_map.find(nBlockID);
				if (curIndex != instance_map.end())
				{
					cachedGroupIdx = curIndex->second;
				}
				else // if(m_instanceGroups[cachedGroupIdx].m_pTemplate != pBlock->GetTemplate())
				{
					for (uint32_t j = 0; j < instanceGroups.size(); j++)
					{
						if (instanceGroups[j]->m_pTemplate == pBlock->GetTemplate() && instanceGroups[j]->m_blockData == nBlockData)
						{
							cachedGroupIdx = j;
							break;
						}
						else if (instanceGroups[j]->m_pTemplate == 0)
						{
							//template doesn't match any group 
							instanceGroups[j]->m_pTemplate = pBlock->GetTemplate();
							instanceGroups[j]->m_blockData = nBlockData;
							cachedGroupIdx = j;
							break;
						}
					}

					if (instanceGroups[cachedGroupIdx]->m_pTemplate != pBlock->GetTemplate() || instanceGroups[cachedGroupIdx]->m_blockData != nBlockData)
					{
						instanceGroups.push_back(new InstanceGroup());
						cachedGroupIdx = instanceGroups.size() - 1;
						instanceGroups[cachedGroupIdx]->m_pTemplate = pBlock->GetTemplate();
						instanceGroups[cachedGroupIdx]->m_blockData = nBlockData;
					}
					instance_map[nBlockID] = cachedGroupIdx;
				}

				uint32 nFaceCount = blockmodel.GetFaceCount();
				instanceGroups[cachedGroupIdx]->AddInstance(i, nFaceCount);
				totalFaceCount += nFaceCount;

				if (!pChunk->HasBlockMaterial(i)) continue;

				// 按照材质分类
				for (uint32_t j = 0; j < nFaceCount; j++)
				{
					int32_t materialId = pChunk->GetBlockFaceMaterial(i, j);
					if (materialId > 0)
					{
						int64_t materialInstanceKey = ((int64_t)materialId) << 32;
						auto curIndex = instance_map.find(materialInstanceKey);
						if (curIndex != instance_map.end())
						{
							cachedGroupIdx = curIndex->second;
						}
						else 
						{
							for (uint32_t j = 0; j < instanceGroups.size(); j++)
							{
								if (instanceGroups[j]->m_materialId == materialId || instanceGroups[j]->m_pTemplate == 0)
								{
									cachedGroupIdx = j;
									break;
								}
							}

							if (instanceGroups[cachedGroupIdx]->m_materialId != materialId)
							{
								if (instanceGroups[cachedGroupIdx]->m_pTemplate != 0)
								{
									cachedGroupIdx = instanceGroups.size();
									instanceGroups.push_back(new InstanceGroup());
								}
								instanceGroups[cachedGroupIdx]->m_pTemplate = pBlock->GetTemplate();
								instanceGroups[cachedGroupIdx]->m_blockData = nBlockData;
								instanceGroups[cachedGroupIdx]->m_materialId = materialId;
							}
							instance_map[materialInstanceKey] = cachedGroupIdx;
						}
						// 同一个方块不重复加
						if (instanceGroups[cachedGroupIdx]->instances.size() == 0 || instanceGroups[cachedGroupIdx]->instances.back() != i)
						{
							instanceGroups[cachedGroupIdx]->AddInstance(i, 1);
							totalFaceCount += 1;
						}
					}
				}
			}
		}
		return totalFaceCount;
	}

	RenderableChunk::ChunkBuildState RenderableChunk::GetChunkBuildState() const
	{
		return m_chunkBuildState;
	}

	void RenderableChunk::SetChunkBuildState(RenderableChunk::ChunkBuildState val)
	{
		m_chunkBuildState = val;
		if (m_chunkBuildState == ChunkBuild_Ready)
			SetDelayedRebuildTick(0);
	}

	void RenderableChunk::WaitUntilChunkReady()
	{
		while (m_chunkBuildState == ChunkBuild_Rebuilding || m_chunkBuildState == ChunkBuild_RequestRebuild)
		{
			SLEEP(1);
		}
	}


	void RenderableChunk::RebuildRenderBufferToMemory(Scoped_ReadLock<BlockReadWriteLock>* Lock_, int* pnCpuYieldCount)
	{
		// call this function regularly to yield CPU to writer thread only if they are waiting to write data. 
#define CHECK_YIELD_CPU_TO_WRITER   if(Lock_ && Lock_->mutex().HasWaitingWriters()){ \
	nCpuYieldCount++;\
	Lock_->unlock(); \
	Lock_->lock(); \
	if(!m_pWorld->IsInBlockWorld() || m_isDirty)\
		return;\
}

		if (GetChunkBuildState() != ChunkBuild_Rebuilding)
			return;
		if (!m_pWorld)
			return;
		BlockRegion * pOwnerBlockRegion = m_pWorld->GetRegion(m_regionX, m_regionZ);
		if (!pOwnerBlockRegion)
			return;

		Scoped_ReadLock<BlockReadWriteLock> regionLock_(pOwnerBlockRegion->GetReadWriteLock());

		// make chunk not dirty anymore, safe to call, since main thread has write lock that will 
		// not modify the block world when this function is being executed. 
		m_isDirty = false;

		if (IsMainRenderer())
			pOwnerBlockRegion->SetChunkDirty(m_packedChunkID, false);

		ClearBuilderBuffer();

		BlockChunk* pChunk = pOwnerBlockRegion->GetChunk(m_packedChunkID, false);
		if (!pChunk)
		{
			return;
		}
		int nCpuYieldCount = 0;
		//------------------------------------------------------------------------
		//fill instance group
		ResetInstanceGroups();
		CHECK_YIELD_CPU_TO_WRITER;
		int32 totalFaceCount = BuildInstanceGroupsByIdAndData(pChunk);
		CHECK_YIELD_CPU_TO_WRITER;
		if (totalFaceCount <= 0)
		{
			m_totalFaceCount = totalFaceCount;
			return;
		}

		SortAndMergeInstanceGroupsByTexture();
		CHECK_YIELD_CPU_TO_WRITER;
		//----------------------------------------------------------
		//2.create a big buffer to hold all blocks

		uint32_t bufferSize = sizeof(BlockVertexCompressed) * (totalFaceCount * 4);

		BlockRenderMethod dwShaderID = (BlockRenderMethod)GetShaderID();

		m_totalFaceCount = totalFaceCount;
		int32 nFaceCountCompleted = 0;

		const int32 maxFaceCountPerBatch = BlockConfig::g_maxFaceCountPerBatch;
		//-------------------------------------------------------------
		//3.fill buffer
		BlockGeneralTessellator& tessellator = GetBlockTessellator();
		int32 nFreeFaceCountInVertexBuffer = Math::Min(maxFaceCountPerBatch, m_totalFaceCount - nFaceCountCompleted);
		int32 nMemoryBufferIndex = 0;
		ParaVertexBuffer memoryBuffer = RequestMemoryBuffer(nFreeFaceCountInVertexBuffer, &nMemoryBufferIndex);

		BlockVertexCompressed* pVertices;
		memoryBuffer.Lock((void**)&pVertices);
		uint32_t vertexOffset = 0;
		std::vector<InstanceGroup* >& instanceGroups = GetInstanceGroups();
		int nSize = (int)instanceGroups.size();
		InstanceGroup* pInstGroup = NULL;
		InstanceGroup* pLastInstGroup = NULL;
		BlockRenderTask *pTask = NULL;
		for (int i = 0; (i < nSize && (pInstGroup = instanceGroups[i])->instances.size()>0); i++)
		{
			BlockTemplate* pTemplate = pInstGroup->m_pTemplate;
			uint32_t nBlockData = pInstGroup->m_blockData;
			std::vector<uint16_t>& instanceGroup = pInstGroup->instances;
			uint32 groupSize = instanceGroup.size();
			uint32 instCount = groupSize;
			int32_t materialId = pInstGroup->m_materialId;
			int nMaxFaceCountPerInstance = pTemplate->GetBlockModelByData(nBlockData).GetFaceCount();

			if (nFreeFaceCountInVertexBuffer < (int32)pInstGroup->GetFaceCount())
			{
				if (nFreeFaceCountInVertexBuffer < (maxFaceCountPerBatch*0.1))
				{
					memoryBuffer.Unlock();
					nFreeFaceCountInVertexBuffer = Math::Min(maxFaceCountPerBatch, m_totalFaceCount - nFaceCountCompleted);
					memoryBuffer = RequestMemoryBuffer(nFreeFaceCountInVertexBuffer, &nMemoryBufferIndex);
					memoryBuffer.Lock((void**)&pVertices);
					vertexOffset = 0;
					pLastInstGroup = NULL;
				}
				if (nFreeFaceCountInVertexBuffer < (int32)pInstGroup->GetFaceCount())
				{
					instCount = nFreeFaceCountInVertexBuffer / nMaxFaceCountPerInstance;
				}
				else
				{
					instCount = groupSize;
				}
			}

			//add render task
			if (!pTask || !(pInstGroup->CanShareRenderBufferWith(pLastInstGroup)))
			{
				pTask = BlockRenderTask::CreateTask();
				pTask->Init(pTemplate, nBlockData, vertexOffset, pChunk->m_minBlockId_ws, nMemoryBufferIndex);
				pTask->SetMaterialId(pInstGroup->m_materialId);
				m_builder_tasks.push_back(pTask);
				pLastInstGroup = pInstGroup;
			}

			int32 batchInstCount = 0;
			int32 unprocessedInstCount = groupSize;
			for (int inst = 0; inst < (int)groupSize; inst++)
			{
				//start a new one if instances can't fit into one batch
				batchInstCount++;
				if (nFreeFaceCountInVertexBuffer < nMaxFaceCountPerInstance)
				{
					memoryBuffer.Unlock();
					nFreeFaceCountInVertexBuffer = Math::Min(maxFaceCountPerBatch, m_totalFaceCount - nFaceCountCompleted);
					memoryBuffer = RequestMemoryBuffer(nFreeFaceCountInVertexBuffer, &nMemoryBufferIndex);
					memoryBuffer.Lock((void**)&pVertices);
					vertexOffset = 0;

					if (nFreeFaceCountInVertexBuffer < unprocessedInstCount*nMaxFaceCountPerInstance)
					{
						instCount = nFreeFaceCountInVertexBuffer / nMaxFaceCountPerInstance;
					}
					else
					{
						instCount = unprocessedInstCount;
					}
					pTask = BlockRenderTask::CreateTask();
					pTask->Init(pTemplate, nBlockData, vertexOffset, pChunk->m_minBlockId_ws, nMemoryBufferIndex);
					m_builder_tasks.push_back(pTask);
					pLastInstGroup = pInstGroup;

					batchInstCount = 1;
				}

				//--------------------------------------------------------------
				// assemble block model data
				//--------------------------------------------------------------
				BlockVertexCompressed* pBlockModelVertices = NULL;
				unprocessedInstCount--;
				int32 nFaceCount = tessellator.TessellateBlock(pChunk, instanceGroup[inst], dwShaderID, &pBlockModelVertices, materialId);
				if (nFaceCount > 0)
				{
					int32 nVertexCount = nFaceCount * 4;
					if (nFreeFaceCountInVertexBuffer >= nFaceCount)
					{
						memcpy(pVertices, pBlockModelVertices, sizeof(BlockVertexCompressed)*nVertexCount);
						pVertices += nVertexCount;
						vertexOffset += nVertexCount;

						nFaceCountCompleted += nFaceCount;
						pTask->AddRectFace(nFaceCount);
						nFreeFaceCountInVertexBuffer -= nFaceCount;
					}
					else
					{
						// this could happen when block changes when we are still processing it. 
						OUTPUT_LOG("fatal error: not enough face count in vertex buffer. \n");
					}
				}
				CHECK_YIELD_CPU_TO_WRITER;
			}
		}
		memoryBuffer.Unlock();
		if (pnCpuYieldCount)
			*pnCpuYieldCount = nCpuYieldCount;
	}

	ParaMemoryBuffer RenderableChunk::RequestMemoryBuffer(int32 nFaceCountInVertexBuffer, int32* pBufferIndex)
	{
		ParaMemoryBuffer memBuffer;
		if (memBuffer.CreateBuffer(nFaceCountInVertexBuffer * sizeof(BlockVertexCompressed) * 4))
		{
			if (pBufferIndex)
				*pBufferIndex = m_memoryBuffers.size();
			m_memoryBuffers.push_back(memBuffer);
		}
		else
		{
			OUTPUT_LOG("warn: failed to RequestMemoryBuffer of size %d for renderable chunk\n", nFaceCountInVertexBuffer);
		}
		return memBuffer;
	}

	void RenderableChunk::UploadFromMemoryToDeviceBuffer()
	{
		ReleaseVertexBuffers();
		ClearRenderTasks();

		if (!m_memoryBuffers.empty())
		{
			for (ParaMemoryBuffer& memBuffer : m_memoryBuffers)
			{
				ParaVertexBuffer* pBuffer = RequestVertexBuffer(memBuffer.GetBufferSize() / (sizeof(BlockVertexCompressed) * 4));
				if (pBuffer)
					pBuffer->UploadMemoryBuffer(memBuffer.GetMemoryPointer());
			}

			m_renderTasks.clear();
			m_renderTasks.reserve(m_builder_tasks.size());
			for (BlockRenderTask* pTask : m_builder_tasks)
			{
				if ((int)m_vertexBuffers.size() > pTask->GetBufferIndex())
				{
					// transfer render task ownership from builder task to render task. 
					pTask->SetVertexBuffer(m_vertexBuffers[pTask->GetBufferIndex()]->GetDevicePointer());
					m_renderTasks.push_back(pTask);
				}
				else
				{
					BlockRenderTask::ReleaseTask(pTask);
				}
			}
			m_builder_tasks.clear();
			ClearBuilderBuffer();
		}
	}

	bool RenderableChunk::IsBuildingBuffer() const
	{
		return m_chunkBuildState == ChunkBuild_Rebuilding || m_chunkBuildState == ChunkBuild_RequestRebuild;
	}

	bool RenderableChunk::IsReadyOrEmpty() const
	{
		return m_chunkBuildState == ChunkBuild_Ready || m_chunkBuildState == ChunkBuild_empty;
	}

	Int16x3 RenderableChunk::GetChunkPosWs()
	{
		Uint16x3 curChunk;
		UnpackChunkIndex(m_packedChunkID, curChunk.x, curChunk.y, curChunk.z);
		return Int16x3(((m_regionX << 5) + curChunk.x), curChunk.y, ((m_regionZ << 5) + curChunk.z));
	}

	int32 RenderableChunk::GetDelayedRebuildTick() const
	{
		return m_nDelayedRebuildTick;
	}

	void RenderableChunk::SetDelayedRebuildTick(int32 val)
	{
		m_nDelayedRebuildTick = val;
	}

	int16 RenderableChunk::GetChunkViewDistance() const
	{
		return m_nChunkViewDistance;
	}

	void RenderableChunk::SetChunkViewDistance(int16 val)
	{
		m_nChunkViewDistance = val;
	}

	int16 RenderableChunk::GetViewIndex() const
	{
		return m_nViewIndex;
	}

	void RenderableChunk::SetViewIndex(int16 val)
	{
		m_nViewIndex = val;
	}

	void RenderableChunk::SortAndMergeInstanceGroupsByTexture()
	{
		std::vector<InstanceGroup* >& instanceGroups = GetInstanceGroups();
		auto itEnd = std::find_if(instanceGroups.begin(), instanceGroups.end(), [](InstanceGroup * pInst) {
			return pInst->GetFaceCount() == 0;
		});
		// sort by id and then by texture and then by face count. 
		std::sort(instanceGroups.begin(), itEnd, [](InstanceGroup* l, InstanceGroup* r) {
			if (l->m_pTemplate != r->m_pTemplate)
			{
				int nOrder1 = l->m_pTemplate->GetRenderPriority();
				int nOrder2 = r->m_pTemplate->GetRenderPriority();
				if (nOrder1 != nOrder2)
					return nOrder1 < nOrder2;
				else
				{
					return l->m_pTemplate->GetID() < r->m_pTemplate->GetID();
				}
			}
			else
			{
				return l->m_blockData < r->m_blockData;
			}
		});

		/* not needed any more, since we are already merging it during render task rebuild.
		// merge groups with identical textures.
		int nSize = (int)m_instanceGroups.size() - 1;
		for (int i = 0; i < nSize && m_instanceGroups[i]->GetFaceCount()>0; i++)
		{
			InstanceGroup* curInstance = m_instanceGroups[i];
			InstanceGroup* nextInstance = m_instanceGroups[i + 1];
			if (nextInstance->GetFaceCount() > 0)
			{
				if (curInstance->m_pTemplate == nextInstance->m_pTemplate &&
					curInstance->m_pTemplate->GetBlockModelByData(curInstance->m_blockData).GetTextureIndex() == nextInstance->m_pTemplate->GetBlockModelByData(nextInstance->m_blockData).GetTextureIndex())
				{
					// now merge the two instance group.
					// use the block data with larger face count for buffer size estimation.
					curInstance->m_blockData = nextInstance->m_blockData;
					(*curInstance) += (*nextInstance);
					nextInstance->reset();
					for (int j = i + 1; j < nSize; j++)
					{
						m_instanceGroups[j] = m_instanceGroups[j + 1];
						m_instanceGroups[j + 1] = nextInstance;
					}
					i--;
				}
			}
		}
		*/
	}

	int RenderableChunk::GetTotalRenderableChunks()
	{
		return s_nTotalRenderableChunks;
	}

	std::vector<BlockRenderTask*> RenderableChunk::GetRenderTasks()
	{
		return m_renderTasks;
	}

	const CShapeBox& RenderableChunk::GetShapeAABB() const
	{
		return m_pShapeAABB;
	}

	int RenderableChunk::GetShaderID() const
	{
		return (m_dwShaderID < 0) ? m_pWorld->GetBlockRenderMethod() : m_dwShaderID;
	}

	void RenderableChunk::SetShaderID(int val)
	{
		m_dwShaderID = val;
	}

	bool RenderableChunk::IsMainRenderer() const
	{
		return m_bIsMainRenderer;
	}

	void RenderableChunk::SetIsMainRenderer(bool val)
	{
		m_bIsMainRenderer = val;
	}

	void RenderableChunk::ClearChunkData()
	{
		SetChunkDirty(true);
		m_nDelayedRebuildTick = 0;

		if (!IsBuildingBuffer())
		{
			// clear all buffers
			ClearRenderTasks();
			ClearBuilderBuffer();
		}
		else
		{
			OUTPUT_LOG("warn: chunk is rebuilding when clearing it\n");
		}
		ReleaseVertexBuffers();
		m_chunkBuildState = ChunkBuild_empty;
	}

	int RenderableChunk::GetRenderFrameCount() const
	{
		return m_nRenderFrameCount;
	}

	void RenderableChunk::SetRenderFrameCount(int val)
	{
		m_nRenderFrameCount = val;
	}

	int RenderableChunk::GetVertexBufferBytes()
	{
		int nTotalBytes = 0;
		for (auto vertexBuffer : m_vertexBuffers)
		{
			nTotalBytes += (int)vertexBuffer->GetBufferSize();
		}
		return nTotalBytes;
	}

	int RenderableChunk::GetLastBufferBytes()
	{
		int nTotalBytes = GetVertexBufferBytes();
		if (nTotalBytes > 0)
		{
			m_nLastVertexBufferBytes = nTotalBytes;
		}
		return m_nLastVertexBufferBytes;
	}

	bool RenderableChunk::InstanceGroup::CanShareRenderBufferWith(InstanceGroup* pOther /*= NULL*/)
	{
		if (pOther == NULL)
			return false;
		else
		{
			if (m_pTemplate->GetRenderPass() != pOther->m_pTemplate->GetRenderPass())
				return false;
			else if (m_materialId != pOther->m_materialId)
			{
				return false;
			}		
			else
			{
				// TODO: further test textures and categories, etc. 
				return (m_pTemplate->GetTexture0(m_blockData) == pOther->m_pTemplate->GetTexture0(pOther->m_blockData))
					&& (m_pTemplate->GetNormalMap() == pOther->m_pTemplate->GetNormalMap());
			}
		}
	}


}
