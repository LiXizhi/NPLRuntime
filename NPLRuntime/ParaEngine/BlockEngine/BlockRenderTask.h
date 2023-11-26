#pragma once

#include <stdint.h>
#include <vector>
#include <mutex>
#include "BlockCommon.h"
#include "BlockTemplate.h"

namespace ParaEngine
{
	class BlockTemplate;

	class BlockRenderTask
	{
	public:
		static BlockRenderTask* CreateTask();

		static void ReleaseTask(BlockRenderTask* pRenderTask);

		static void ReleaseTaskPool();

		inline uint16_t GetTemplateId()
		{
			return m_pTemplate->GetID();
		}

		inline uint16_t GetUserData()
		{
			return m_nBlockData;
		}

		inline BlockTemplate* GetTemplate()
		{
			return m_pTemplate;
		}

		void SetVertexBuffer(VertexBufferDevicePtr_type val) {
			m_pVertexBuffer = val;
		}

		inline VertexBufferDevicePtr_type GetVertexBuffer()
		{
			return m_pVertexBuffer;
		}

		inline int32_t GetVertexOfs()
		{
			//24 vertex per block
			return m_nVertexOffset;
		}

		inline int32_t GetIndexOfs()
		{
			//24 vertex per block
			return m_nVertexOffset / 4 * 6;
		}

		int32_t GetVertexCount();

		int32_t GetPrimitiveCount();

		inline void SetRenderOrder(uint32_t renderOrder)
		{
			m_renderOrder = renderOrder;
		}

		inline Uint16x3& GetMinBlockPos() { return m_minBlockId_ws; }

		//render order is a uint_32 value can be used to sort render object.
		//it compose of  render priority,material id and relative distance
		//for solid object bit usage are: render priority(4bit) + material id(16bit) + dist(12bit)
		//for transparent object:render priority(4bit) + dist(12bit) + material id(16bit)
		inline int32_t GetRenderOrder()
		{
			return m_renderOrder;
		}

		void AddRectFace(int32 nFaceCount);

		void Init(BlockTemplate* pTemplate, uint16_t nBlockData, uint32_t vertexOffset,
			VertexBufferDevicePtr_type pBuffer, Uint16x3& minBlockId_ws);
		void Init(BlockTemplate* pTemplate, uint16_t nBlockData, uint32_t vertexOffset,
			Uint16x3& minBlockId_ws, int32 nBufferIndex);

		int32 GetBufferIndex() const { return m_pBufferIndex; }
		void SetBufferIndex(int32 val) { m_pBufferIndex = val; }

		int32_t GetMaterialId() const { return m_materialId; }
		void SetMaterialId(int32_t materialId) { m_materialId = materialId; }
	private:
		static std::vector<BlockRenderTask*> g_renderTaskPool;
		static std::map<int, bool> g_emptySlotIndices;
		static std::mutex g_renderTaskMutex;
		uint32_t m_renderOrder;
		uint32_t m_nVertexOffset;
		uint16_t m_nRectFaceCount;
		int16_t m_internalIndex;

		Uint16x3 m_minBlockId_ws;

		BlockTemplate* m_pTemplate;
		uint16_t m_nBlockData;

		int32_t m_materialId;
		// all are weak references: do not release it
		union {
			VertexBufferDevicePtr_type m_pVertexBuffer;
			int32 m_pBufferIndex;
		};


		BlockRenderTask();

		void Reset();

		inline void SetInternalIndex(int32_t index)
		{
			m_internalIndex = index;
		}

		int32_t GetInternalIndex()
		{
			return m_internalIndex;
		}
	};
}
