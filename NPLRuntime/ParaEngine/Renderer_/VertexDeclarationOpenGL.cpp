//-----------------------------------------------------------------------------
// Class:	Vertex Declaration for opengl
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"


using namespace ParaEngine;

#ifdef USE_OPENGL_RENDERER

#include "OpenGLWrapper/GLProgram.h"
#include "VertexDeclarationOpenGL.h"


bool ParaEngine::VertexElement::IsEndDeclare() const
{
	return (Stream == 0xff);
}

uint32_t ParaEngine::VertexElement::GetSize() const
{
	if (Type == D3DDECLTYPE_FLOAT4)
		return 16;
	else if (Type == D3DDECLTYPE_FLOAT3)
		return 12;
	else if (Type == D3DDECLTYPE_FLOAT2)
		return 8;
	else if (Type == D3DDECLTYPE_FLOAT1)
		return 4;
	else if (Type == D3DDECLTYPE_D3DCOLOR)
		return 4;
	else
		return 0;
}


ParaEngine::CVertexDeclaration::CVertexDeclaration(const VertexElement* elems)
	:m_nSize(0), m_dwAttributes(0), m_vao(0)
{
	SetVertexElement(elems);
}

ParaEngine::CVertexDeclaration::~CVertexDeclaration()
{
}



void ParaEngine::CVertexDeclaration::Release()
{
	delete this;
}

#define OFFSET_OF(pVertexStreamZeroData, offset)  ((GLvoid*)(((const char*)(pVertexStreamZeroData))+(offset)))

void ParaEngine::CVertexDeclaration::ApplyAttribute(const void* pVertexStreamZeroData)
{

	int nTextureIndex = 0;
	int nColorIndex = 0;
	for (auto elem : m_elements)
	{
		if (elem.Usage == D3DDECLUSAGE_POSITION)
		{
			if (elem.Type == D3DDECLTYPE_FLOAT3)
				glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, m_nSize, OFFSET_OF(pVertexStreamZeroData, elem.Offset));
			else if (elem.Type == D3DDECLTYPE_FLOAT4)
				glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 4, GL_FLOAT, GL_FALSE, m_nSize, OFFSET_OF(pVertexStreamZeroData, elem.Offset));
		}
		else if (elem.Usage == D3DDECLUSAGE_COLOR)
		{
			// Please note: opengl packed color as RGBA in original byte ordering
			// however directX packed color as BGRA, we will need to swizzle the B and R in color component.
			if (elem.Type == D3DDECLTYPE_D3DCOLOR)
			{
				if (nColorIndex == 0)
					glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, m_nSize, OFFSET_OF(pVertexStreamZeroData, elem.Offset));
				else
					glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_MAX + 0, 4, GL_UNSIGNED_BYTE, GL_TRUE, m_nSize, OFFSET_OF(pVertexStreamZeroData, elem.Offset));
				nColorIndex++;
			}
		}
		else if (elem.Usage == D3DDECLUSAGE_TEXCOORD)
		{
			if (elem.Type == D3DDECLTYPE_FLOAT2){
				glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD + nTextureIndex, 2, GL_FLOAT, GL_FALSE, m_nSize, OFFSET_OF(pVertexStreamZeroData, elem.Offset));
				nTextureIndex++;
			}
		}
		else if (elem.Usage == D3DDECLUSAGE_NORMAL)
		{
			if (elem.Type == D3DDECLTYPE_FLOAT3)
				glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, m_nSize, OFFSET_OF(pVertexStreamZeroData, elem.Offset));
		}
	}

}

void ParaEngine::CVertexDeclaration::EnableAttribute()
{

	GL::enableVertexAttribs(m_dwAttributes);
}

void ParaEngine::CVertexDeclaration::SetVertexElement(const VertexElement* elems)
{
	m_nSize = 0;
	m_dwAttributes = 0;
	m_elements.clear();
	int nTextureIndex = 0;
	int nColorIndex = 0;
	for (int i = 0; i < 10; ++i)
	{
		const VertexElement& elem = elems[i];
		if (!elem.IsEndDeclare())
		{
			m_elements.push_back(elem);
			m_nSize = elem.Offset + elem.GetSize();
			int nAttributeIndex = 0;
			if (elem.Usage == D3DDECLUSAGE_POSITION)
			{
				nAttributeIndex = GLProgram::VERTEX_ATTRIB_POSITION;
			}
			else if (elem.Usage == D3DDECLUSAGE_COLOR)
			{
				if (nColorIndex == 0)
					nAttributeIndex = GLProgram::VERTEX_ATTRIB_COLOR;
				else
					nAttributeIndex = GLProgram::VERTEX_ATTRIB_MAX + 0;
				nColorIndex++;
			}
			else if (elem.Usage == D3DDECLUSAGE_TEXCOORD)
			{
				nAttributeIndex = GLProgram::VERTEX_ATTRIB_TEX_COORD + nTextureIndex;
				nTextureIndex++;
			}
			else if (elem.Usage == D3DDECLUSAGE_NORMAL)
			{
				nAttributeIndex = GLProgram::VERTEX_ATTRIB_NORMAL;
			}
			m_dwAttributes |= (1 << nAttributeIndex);
		}
		else
			break;
	}

}

#endif
