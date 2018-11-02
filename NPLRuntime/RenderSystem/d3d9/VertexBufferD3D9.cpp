#include "VertexBufferD3D9.h"
#include <cassert>
using namespace ParaEngine;

VertexBufferD3D9::VertexBufferD3D9(IDirect3DVertexBuffer9* pD3DVertexBuffer)
	:m_pD3DVertexBuffer(pD3DVertexBuffer)
{
	assert(m_pD3DVertexBuffer);
}

VertexBufferD3D9::~VertexBufferD3D9()
{
	if (m_pD3DVertexBuffer != nullptr)
	{
		m_pD3DVertexBuffer->Release();
		m_pD3DVertexBuffer = nullptr;
	}
}

IDirect3DVertexBuffer9* ParaEngine::VertexBufferD3D9::GetD3DVertexBuffer() const
{
	assert(m_pD3DVertexBuffer);
	return m_pD3DVertexBuffer;
}
