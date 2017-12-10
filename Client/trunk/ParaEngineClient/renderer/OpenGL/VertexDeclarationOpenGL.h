#pragma once

namespace ParaEngine
{
	struct VertexElement
	{
		//VertexElement();

		bool IsEndDeclare() const;
		uint32 GetSize() const;
	public:
		WORD    Stream;     // Stream index
		WORD    Offset;     // Offset in the stream in bytes
		BYTE    Type;       // Data type
		BYTE    Method;     // Processing method
		BYTE    Usage;      // Semantics
		BYTE    UsageIndex; // Semantic index
	};
	
	/** vertex element*/
	class CVertexDeclaration
	{
	public:
		/**array of elements that ends with D3DDECL_END */
		CVertexDeclaration(const VertexElement* elems);
		virtual ~CVertexDeclaration();
	public:
		virtual void Release();

		void ApplyAttribute(const void* pVertexStreamZeroData = 0);
		void EnableAttribute();

		/**array of elements that ends with D3DDECL_END */
		void SetVertexElement(const VertexElement* elems);
		
	protected:
		std::vector<VertexElement> m_elements;
		uint32 m_nSize;
		uint32 m_dwAttributes;
		//use opengl vertext array object (vao) if possible, same concept as vertex declaration in directX
		GLuint m_vao; 
	};
}

