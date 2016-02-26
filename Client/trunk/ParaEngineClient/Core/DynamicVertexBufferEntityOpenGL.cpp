//----------------------------------------------------------------------
// Class:	Vertex Array
// Authors:	LiXizhi
// Date:	2014.10.8
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#if defined (USE_OPENGL_RENDERER) || defined (USE_NULL_RENDERER)
#include "DynamicVertexBufferEntityOpenGL.h"

using namespace ParaEngine;

DynamicVertexBufferEntityOpenGL::DynamicVertexBufferEntityOpenGL()
	: m_dwBase(0),
	m_dwNextBase(0),
	// better be contains a factor of 3*4=12
	// 24000=800KB, Please note that this number must be a multiple of 3
	m_dwDiscard(24000),
	// normally 1000 vertex is a good choice for current TnL
	m_dwFlush(240),
	// default unit size.
	m_nUnitSize(sizeof(my_vertex))
{

};

HRESULT DynamicVertexBufferEntityOpenGL::RestoreDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;
	m_bIsInitialized = true;
	m_lpVB.resize(m_dwDiscard*m_nUnitSize);
	return S_OK;
}


HRESULT DynamicVertexBufferEntityOpenGL::InvalidateDeviceObjects()
{
	m_bIsInitialized = false;
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
	return nSize;
}

void DynamicVertexBufferEntityOpenGL::Unlock()
{
}

GLuint ParaEngine::DynamicVertexBufferEntityOpenGL::GetBuffer()
{
	return 0;
}

void* ParaEngine::DynamicVertexBufferEntityOpenGL::GetBaseVertexPointer()
{
	return (void*)((&(m_lpVB[0])) + m_dwBase * m_nUnitSize);
}

bool ParaEngine::DynamicVertexBufferEntityOpenGL::IsMemoryBuffer()
{
	return true;
}

DWORD ParaEngine::DynamicVertexBufferEntityOpenGL::GetBaseVertex()
{
	return m_dwBase;
}

#endif


