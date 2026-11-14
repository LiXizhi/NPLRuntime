//----------------------------------------------------------------------
// Class:	Vertex Array
// Authors:	LiXizhi
// Date:	2014.10.8
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#if !defined(USE_NULL_RENDERER)
#include "DynamicVertexBufferEntityOpenGL.h"

using namespace ParaEngine;

DynamicVertexBufferEntityOpenGL::DynamicVertexBufferEntityOpenGL()
	: m_dwDataFormat(0),
	m_dwBase(0),
	m_dwNextBase(0),
	// better be contains a factor of 3*4=12
	// 24000=800KB, Please note that this number must be a multiple of 3
	m_dwDiscard(24000),
	// normally 1000 vertex is a good choice for current TnL
	m_dwFlush(240),
	// default unit size.
	m_nUnitSize(sizeof(my_vertex))
{

}

DynamicVertexBufferEntityOpenGL::~DynamicVertexBufferEntityOpenGL()
{
	InvalidateDeviceObjects();
}

HRESULT DynamicVertexBufferEntityOpenGL::RestoreDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;
	m_bIsInitialized = true;
	m_lpVB.resize(m_dwDiscard*m_nUnitSize);
	if (m_dwDataFormat == DVB_MATRIX)
	{
		if (m_gpuBuffer)
		{
			m_gpuBuffer->ReleaseRHIResource();
			m_gpuBuffer = nullptr;
		}
		auto* pDev = CGlobals::GetRenderDevice();
		if (pDev && m_dwDiscard * m_nUnitSize > 0)
		{
			m_gpuBuffer = pDev->CreateBuffer(m_dwDiscard * m_nUnitSize, false, ERHIBufferUsage::Dynamic | ERHIBufferUsage::WriteOnly, nullptr);
		}
	}
	return S_OK;
}


HRESULT DynamicVertexBufferEntityOpenGL::InvalidateDeviceObjects()
{
	m_bIsInitialized = false;
	if (m_gpuBuffer)
	{
		m_gpuBuffer->ReleaseRHIResource();
		m_gpuBuffer = nullptr;
	}
	m_lockedCount = 0;
	m_lpVB.clear();
	return S_OK;
}
//--------------------------------------------------------------------
/// nSizeOfData: number of vertices to be moved into the vertex buffer.
/// return the number of vertices locked
//--------------------------------------------------------------------
uint32 DynamicVertexBufferEntityOpenGL::Lock(uint32 nSize, void** pVertices)
{
	LoadAsset();

	// Check to see if the entire vertex buffer has been used up yet.
	if (nSize > m_dwDiscard - m_dwNextBase)
	{
		// No space remains. Start over from the beginning 
		//   of the vertex buffer.
		m_dwNextBase = 0;

		// Bug fixed 2008.5.27: if the requested buffer is larger than the total dynamic buffer size, we will break the input. 
		if (nSize > m_dwDiscard)
		{
			nSize = m_dwDiscard;
		}
	}

	// Lock the vertex buffer.
	*pVertices = (void*)((&(m_lpVB[0])) + m_dwNextBase * m_nUnitSize);
	
	// Advance to the next position in the vertex buffer.
	m_dwBase = m_dwNextBase;
	m_dwNextBase += nSize;
	m_lockedCount = nSize;
	return nSize;
}

void DynamicVertexBufferEntityOpenGL::Unlock()
{
	if (m_gpuBuffer != nullptr && m_lockedCount > 0 && !m_lpVB.empty())
	{
		uint32_t offsetBytes = m_dwBase * m_nUnitSize;
		uint32_t sizeBytes = m_lockedCount * m_nUnitSize;
		if (offsetBytes + sizeBytes <= m_lpVB.size())
		{
			m_gpuBuffer->UpdateData(offsetBytes, sizeBytes, (&(m_lpVB[0])) + offsetBytes);
		}
	}
	m_lockedCount = 0;
}

VertexBufferDevicePtr_type ParaEngine::DynamicVertexBufferEntityOpenGL::GetBuffer()
{
	return m_gpuBuffer;
}

void* ParaEngine::DynamicVertexBufferEntityOpenGL::GetBaseVertexPointer()
{
	return (void*)((&(m_lpVB[0])) + m_dwBase * m_nUnitSize);
}

bool ParaEngine::DynamicVertexBufferEntityOpenGL::IsMemoryBuffer()
{
	return m_gpuBuffer == nullptr;
}

DWORD ParaEngine::DynamicVertexBufferEntityOpenGL::GetBaseVertex()
{
	return m_dwBase;
}

#endif


