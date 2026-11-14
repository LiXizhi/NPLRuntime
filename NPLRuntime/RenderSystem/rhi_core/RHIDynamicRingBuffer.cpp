#include "RHIDynamicRingBuffer.h"
#include <string.h>

namespace ParaEngine
{
	RHIDynamicRingBuffer::RHIDynamicRingBuffer(bool isIndexBuffer, uint32_t initialCapacity)
		: m_buffer(nullptr)
		, m_capacity(0)
		, m_cursor(0)
		, m_initialCapacity(initialCapacity)
		, m_isIndexBuffer(isIndexBuffer)
	{
	}

	RHIDynamicRingBuffer::~RHIDynamicRingBuffer()
	{
		Release();
	}

	void RHIDynamicRingBuffer::Release()
	{
		if (m_buffer)
		{
			m_buffer->ReleaseRHIResource();
			m_buffer = nullptr;
		}
		m_capacity = 0;
		m_cursor = 0;
	}

	bool RHIDynamicRingBuffer::Allocate(IRenderDevice* device, uint32_t byteSize, uint32_t alignment, const void* srcData, IRHIBuffer*& outBuffer, uint32_t& outOffset)
	{
		outBuffer = nullptr;
		outOffset = 0;
		if (!device || byteSize == 0 || !srcData)
			return false;

		if (alignment == 0)
			alignment = 4;

		if (!m_buffer || byteSize > m_capacity)
		{
			uint32_t newCap = m_initialCapacity;
			while (newCap < byteSize * 2u)
				newCap *= 2u;

			Release();
			m_buffer = device->CreateBuffer(newCap, m_isIndexBuffer, ERHIBufferUsage::Dynamic | ERHIBufferUsage::Stream | ERHIBufferUsage::WriteOnly, nullptr);
			if (!m_buffer)
				return false;
			m_capacity = newCap;
			m_cursor = 0;
		}

		// Align cursor to stride or power-of-two boundary
		uint32_t alignedOffset = (m_cursor == 0) ? 0 : (((m_cursor + alignment - 1u) / alignment) * alignment);
		ERHIMapMode mapMode = ERHIMapMode::WriteNoOverwrite;
		if (alignedOffset + byteSize > m_capacity)
		{
			alignedOffset = 0;
			mapMode = ERHIMapMode::WriteDiscard;
		}
		else if (alignedOffset == 0)
		{
			mapMode = ERHIMapMode::WriteDiscard;
		}

		void* dst = m_buffer->Map(alignedOffset, byteSize, mapMode);
		if (!dst)
			return false;

		memcpy(dst, srcData, byteSize);
		m_buffer->Unmap();

		m_cursor = alignedOffset + byteSize;
		outBuffer = m_buffer;
		outOffset = alignedOffset;
		return true;
	}
}
