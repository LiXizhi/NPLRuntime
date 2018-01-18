#include "IndexBufferD3D9.h"
#include <cassert>
using namespace ParaEngine;

IndexBufferD3D9::IndexBufferD3D9(IDirect3DIndexBuffer9* pD3DIndexBuffer)
	:m_pD3DIndexBuffer(pD3DIndexBuffer)
{
	assert(m_pD3DIndexBuffer);
}

IndexBufferD3D9::~IndexBufferD3D9()
{
	if (m_pD3DIndexBuffer != nullptr)
	{
		m_pD3DIndexBuffer->Release();
		m_pD3DIndexBuffer = nullptr;
	}
}

IDirect3DIndexBuffer9* ParaEngine::IndexBufferD3D9::GetD3DIndexBuffer() const
{
	assert(m_pD3DIndexBuffer);
	return m_pD3DIndexBuffer;
}
