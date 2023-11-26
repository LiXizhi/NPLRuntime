//-----------------------------------------------------------------------------
// Class:	Block Render Task
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2013.11.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockRenderTask.h"


namespace ParaEngine
{
	std::vector<BlockRenderTask*> BlockRenderTask::g_renderTaskPool;
	std::map<int, bool> BlockRenderTask::g_emptySlotIndices;
	std::mutex BlockRenderTask::g_renderTaskMutex;

	BlockRenderTask* BlockRenderTask::CreateTask()
	{
		BlockRenderTask* pResult;
		{
			std::lock_guard<std::mutex> lock_(g_renderTaskMutex);
			if (g_emptySlotIndices.size() == 0)
			{
				BlockRenderTask* newTask = new BlockRenderTask();
				g_renderTaskPool.push_back(newTask);
				pResult = newTask;
				pResult->SetInternalIndex(g_renderTaskPool.size() - 1);
			}
			else
			{
				int emptySlotIndex = g_emptySlotIndices.begin()->first;
				pResult = g_renderTaskPool[emptySlotIndex];
				g_emptySlotIndices.erase(g_emptySlotIndices.begin());
			}
		}
		return pResult;
	}

	void BlockRenderTask::ReleaseTask(BlockRenderTask* pRenderTask)
	{
		pRenderTask->Reset();
		std::lock_guard<std::mutex> lock_(g_renderTaskMutex);
		g_emptySlotIndices[pRenderTask->GetInternalIndex()] = true;
	}

	BlockRenderTask::BlockRenderTask()
	{
		memset(this, 0, sizeof(BlockRenderTask));
		m_materialId = -1;
	}

	void BlockRenderTask::Init(BlockTemplate* pTemplate, uint16_t nBlockData, uint32_t vertexOffset, VertexBufferDevicePtr_type pBuffer, Uint16x3& minBlockId_ws)
	{
		m_nBlockData = nBlockData;
		m_nVertexOffset = vertexOffset;
		m_nRectFaceCount = 0;
		m_pVertexBuffer = pBuffer;
		m_renderOrder = 0;
		m_pTemplate = pTemplate;
		m_minBlockId_ws = minBlockId_ws;
		m_materialId = -1;
	}

	void BlockRenderTask::Init(BlockTemplate* pTemplate, uint16_t nBlockData, uint32_t vertexOffset, Uint16x3& minBlockId_ws, int32 nBufferIndex)
	{
		m_nBlockData = nBlockData;
		m_nVertexOffset = vertexOffset;
		m_nRectFaceCount = 0;
		m_pBufferIndex = nBufferIndex;
		m_renderOrder = 0;
		m_pTemplate = pTemplate;
		m_minBlockId_ws = minBlockId_ws;
		m_materialId = -1;
	}

	void BlockRenderTask::Reset()
	{
		m_nVertexOffset = 0;
		m_nRectFaceCount = 0;
		m_pTemplate = nullptr;
		m_nBlockData = 0;
		m_pVertexBuffer = 0;
		m_materialId = -1;
	}

	void BlockRenderTask::ReleaseTaskPool()
	{
		std::lock_guard<std::mutex> lock_(g_renderTaskMutex);

		if (g_renderTaskPool.size() > 0)
		{
			for (uint32_t i = 0; i < g_renderTaskPool.size(); i++)
			{
				delete g_renderTaskPool[i];
			}
			g_renderTaskPool.clear();
		}
		g_emptySlotIndices.clear();
	}

	int32_t BlockRenderTask::GetVertexCount()
	{
		return m_nRectFaceCount * 4;
	}

	int32_t BlockRenderTask::GetPrimitiveCount()
	{
		//12 triangle per block
		return m_nRectFaceCount * 2;
	}

	void BlockRenderTask::AddRectFace(int32 nFaceCount)
	{
		m_nRectFaceCount += (uint16)nFaceCount;
	}

}
