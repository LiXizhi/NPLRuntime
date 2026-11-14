//-----------------------------------------------------------------------------
// Class:	Cross-platform Vertex Declaration for ParaRHI
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.12 (Unified ParaRHI 2026.11)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "VertexDeclarationOpenGL.h"

using namespace ParaEngine;

namespace
{
	// Canonical attribute slot indices (matching GLProgram::VERTEX_ATTRIB_* bit-for-bit):
	// 0: POSITION        (GLProgram::VERTEX_ATTRIB_POSITION)
	// 1: COLOR0          (GLProgram::VERTEX_ATTRIB_COLOR)
	// 2: COLOR1          (GLProgram::VERTEX_ATTRIB_COLOR1)
	// 3..6: TEXCOORD0..3 (GLProgram::VERTEX_ATTRIB_TEX_COORD .. TEX_COORD3)
	// 7: NORMAL          (GLProgram::VERTEX_ATTRIB_NORMAL)
	// 8: BLENDWEIGHT     (GLProgram::VERTEX_ATTRIB_BLEND_WEIGHT)
	// 9: BLENDINDICES    (GLProgram::VERTEX_ATTRIB_BLEND_INDEX)
	// 10: COLOR2+        (GLProgram::VERTEX_ATTRIB_MAX + 0)
	// 11: TANGENT
	// 12: BINORMAL
	// 13..15: TEXCOORD4..6
	uint32_t MapElementToAttribSlot(const VertexElement& elem, int& inoutTexIndex, int& inoutColorIndex)
	{
		switch (elem.Usage)
		{
		case D3DDECLUSAGE_POSITION:
		case D3DDECLUSAGE_POSITIONT:
			return 0u;
		case D3DDECLUSAGE_COLOR:
		{
			uint32_t slot = (inoutColorIndex <= 1) ? static_cast<uint32_t>(1 + inoutColorIndex) : 10u;
			++inoutColorIndex;
			return slot;
		}
		case D3DDECLUSAGE_TEXCOORD:
		{
			uint32_t idx = (elem.UsageIndex > 0) ? elem.UsageIndex : static_cast<uint32_t>(inoutTexIndex);
			++inoutTexIndex;
			if (idx < 4u)
				return 3u + idx;
			return 13u + ((idx - 4u) < 3u ? (idx - 4u) : 2u);
		}
		case D3DDECLUSAGE_NORMAL:
			return 7u;
		case D3DDECLUSAGE_BLENDWEIGHT:
			return 8u;
		case D3DDECLUSAGE_BLENDINDICES:
			return 9u;
		case D3DDECLUSAGE_TANGENT:
			return 11u;
		case D3DDECLUSAGE_BINORMAL:
			return 12u;
		default:
			return 0u;
		}
	}
}

bool ParaEngine::VertexElement::IsEndDeclare() const
{
	return (Stream == 0xff) || (Type == D3DDECLTYPE_UNUSED);
}

uint32_t ParaEngine::VertexElement::GetSize() const
{
	switch (Type)
	{
	case D3DDECLTYPE_FLOAT1:    return 4;
	case D3DDECLTYPE_FLOAT2:    return 8;
	case D3DDECLTYPE_FLOAT3:    return 12;
	case D3DDECLTYPE_FLOAT4:    return 16;
	case D3DDECLTYPE_D3DCOLOR:  return 4;
	case D3DDECLTYPE_UBYTE4:    return 4;
	case D3DDECLTYPE_SHORT2:    return 4;
	case D3DDECLTYPE_SHORT4:    return 8;
	case D3DDECLTYPE_UBYTE4N:   return 4;
	case D3DDECLTYPE_SHORT2N:   return 4;
	case D3DDECLTYPE_SHORT4N:   return 8;
	case D3DDECLTYPE_USHORT2N:  return 4;
	case D3DDECLTYPE_USHORT4N:  return 8;
	case D3DDECLTYPE_UDEC3:     return 4;
	case D3DDECLTYPE_DEC3N:     return 4;
	case D3DDECLTYPE_FLOAT16_2: return 4;
	case D3DDECLTYPE_FLOAT16_4: return 8;
	default:                    return 0;
	}
}

ParaEngine::CVertexDeclaration::CVertexDeclaration(const VertexElement* elems)
	: m_streamStrides{ 0, 0, 0, 0 }
	, m_nSize(0)
	, m_dwAttributes(0)
	, m_hash(0)
	, m_backendHandle(nullptr)
{
	SetVertexElement(elems);
}

ParaEngine::CVertexDeclaration::~CVertexDeclaration() = default;

void ParaEngine::CVertexDeclaration::Release()
{
	delete this;
}

void ParaEngine::CVertexDeclaration::SetVertexElement(const VertexElement* elems)
{
	for (uint32_t s = 0; s < MAX_STREAMS; ++s)
		m_streamStrides[s] = 0;
	m_nSize = 0;
	m_dwAttributes = 0;
	m_elements.clear();

	uint64_t h = 14695981039346656037ULL; // FNV-1a 64-bit offset basis
	if (!elems)
	{
		m_hash = h;
		return;
	}

	int nTextureIndex = 0;
	int nColorIndex = 0;
	for (uint32_t i = 0; i < MAX_ELEMENTS; ++i)
	{
		const VertexElement& elem = elems[i];
		if (elem.IsEndDeclare())
			break;

		m_elements.push_back(elem);
		uint32_t endOffset = static_cast<uint32_t>(elem.Offset) + elem.GetSize();
		if (elem.Stream < MAX_STREAMS && endOffset > m_streamStrides[elem.Stream])
		{
			m_streamStrides[elem.Stream] = endOffset;
		}

		uint32_t slot = MapElementToAttribSlot(elem, nTextureIndex, nColorIndex);
		if (slot < 32u)
		{
			m_dwAttributes |= (1u << slot);
		}

		const uint8_t* raw = reinterpret_cast<const uint8_t*>(&elem);
		for (size_t b = 0; b < sizeof(VertexElement); ++b)
		{
			h ^= static_cast<uint64_t>(raw[b]);
			h *= 1099511628211ULL;
		}
	}

	m_nSize = m_streamStrides[0];
	m_hash = h;
}
