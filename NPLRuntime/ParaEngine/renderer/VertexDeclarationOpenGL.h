#pragma once
#include <cstdint>
#include <vector>

namespace ParaEngine
{
	struct VertexElement
	{
		bool IsEndDeclare() const;
		uint32_t GetSize() const;
	public:
		uint16_t    Stream;     // Stream index
		uint16_t    Offset;     // Offset in the stream in bytes
		uint8_t     Type;       // Data type (D3DDECLTYPE)
		uint8_t     Method;     // Processing method
		uint8_t     Usage;      // Semantics (D3DDECLUSAGE)
		uint8_t     UsageIndex; // Semantic index
	};

	/**
	 * Cross-platform vertex input layout declaration shared across all ParaRHI backends.
	 */
	class CVertexDeclaration
	{
	public:
		static constexpr uint32_t MAX_STREAMS = 4;
		static constexpr uint32_t MAX_ELEMENTS = 16;

		/** Array of elements terminated with D3DDECL_END() */
		explicit CVertexDeclaration(const VertexElement* elems);
		virtual ~CVertexDeclaration();

		virtual void Release();

		/** Array of elements terminated with D3DDECL_END() */
		void SetVertexElement(const VertexElement* elems);

		const std::vector<VertexElement>& GetElements() const { return m_elements; }
		uint32_t GetStride(uint32_t stream = 0) const { return (stream < MAX_STREAMS) ? m_streamStrides[stream] : 0; }
		uint32_t GetAttributeMask() const { return m_dwAttributes; }
		uint64_t GetHash() const { return m_hash; }

		void* GetBackendHandle() const { return m_backendHandle; }
		void SetBackendHandle(void* handle) { m_backendHandle = handle; }

	protected:
		std::vector<VertexElement> m_elements;
		uint32_t m_streamStrides[MAX_STREAMS];
		uint32_t m_nSize;
		uint32_t m_dwAttributes;
		uint64_t m_hash;
		void* m_backendHandle;
	};
}
