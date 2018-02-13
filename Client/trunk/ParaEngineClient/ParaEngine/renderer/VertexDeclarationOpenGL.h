#pragma once
#include <cstdint>
#include <vector>
namespace ParaEngine
{
	struct VertexElement
	{
		//VertexElement();

		bool IsEndDeclare() const;
		uint32_t GetSize() const;
	public:
		uint16_t    Stream;     // Stream index
		uint16_t    Offset;     // Offset in the stream in bytes
		uint8_t     Type;       // Data type
		uint8_t     Method;     // Processing method
		uint8_t     Usage;      // Semantics
		uint8_t     UsageIndex; // Semantic index
	};
	
	/** vertex element*/
	class CVertexDeclaration
	{
	public:
		/**array of elements that ends with D3DDECL_END */
		CVertexDeclaration(const VertexElement* elems);
		virtual ~CVertexDeclaration();
	public:

		bool Fuck();
		virtual void Release();

		void ApplyAttribute(const void* pVertexStreamZeroData = 0);
		void EnableAttribute();

		/**array of elements that ends with D3DDECL_END */
		void SetVertexElement(const VertexElement* elems);
		
	protected:
		std::vector<VertexElement> m_elements;
		uint32_t m_nSize;
		uint32_t m_dwAttributes;
		//use opengl vertext array object (vao) if possible, same concept as vertex declaration in directX
		uint32_t m_vao;
	};
}

