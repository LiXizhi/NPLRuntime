#pragma once
#include <stdint.h>
#include "Framework/Interface/Render/IRenderDevice.h"

namespace ParaEngine
{
	/**
	 * Per-device streaming ring buffer for zero-allocation DrawPrimitiveUP / DrawIndexedPrimitiveUP.
	 */
	class RHIDynamicRingBuffer
	{
	public:
		explicit RHIDynamicRingBuffer(bool isIndexBuffer, uint32_t initialCapacity = 256 * 1024);
		~RHIDynamicRingBuffer();

		RHIDynamicRingBuffer(const RHIDynamicRingBuffer&) = delete;
		RHIDynamicRingBuffer& operator=(const RHIDynamicRingBuffer&) = delete;

		void Release();

		/**
		 * Uploads transient vertex/index data into the streaming ring buffer and outputs
		 * the underlying IRHIBuffer and aligned byte offset.
		 */
		bool Allocate(IRenderDevice* device, uint32_t byteSize, uint32_t alignment, const void* srcData, IRHIBuffer*& outBuffer, uint32_t& outOffset);

	private:
		IRHIBuffer* m_buffer;
		uint32_t m_capacity;
		uint32_t m_cursor;
		uint32_t m_initialCapacity;
		bool m_isIndexBuffer;
	};
}
