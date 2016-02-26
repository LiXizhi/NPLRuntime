//-----------------------------------------------------------------------------
// Class:	Vertex buffer
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.10
// Desc: cross platform vertex/index buffer. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaVertexBuffer.h"

using namespace ParaEngine;


ParaEngine::ParaVertexBuffer::ParaVertexBuffer()
:m_vertexBuffer(0), m_nBufferSize(0), m_buffer(0), m_bufferType(BufferType_VertexBuffer)
{

}

ParaEngine::ParaVertexBuffer::~ParaVertexBuffer()
{
	
}

ParaEngine::VertexBufferDevicePtr_type ParaEngine::ParaVertexBuffer::GetDevicePointer()
{
	return m_vertexBuffer;
}

ParaEngine::IndexBufferDevicePtr_type ParaEngine::ParaVertexBuffer::GetDevicePointerAsIndexBuffer()
{
	return m_indexBuffer;
}

uint32 ParaEngine::ParaVertexBuffer::GetBufferSize()
{
	return m_nBufferSize;
}


void ParaEngine::ParaVertexBuffer::RendererRecreated()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	SAFE_DELETE_ARRAY(m_buffer);
}

bool ParaEngine::ParaVertexBuffer::CreateBuffer(uint32 nBufferSize, DWORD dwFormat /*= 0*/, DWORD dwUsage /*= 0*/, D3DPOOL dwPool)
{
	ReleaseBuffer();
	m_bufferType = BufferType_VertexBuffer;
	m_nBufferSize = nBufferSize;
#ifdef USE_DIRECTX_RENDERER
	return SUCCEEDED(CGlobals::GetRenderDevice()->CreateVertexBuffer(nBufferSize, dwUsage, dwFormat, dwPool, &m_vertexBuffer, NULL));
#elif defined(USE_OPENGL_RENDERER)
	glGenBuffers(1, &m_vertexBuffer);
	return true;
#else
	return false;
#endif
}


bool ParaEngine::ParaVertexBuffer::CreateIndexBuffer(uint32 nBufferSize, DWORD dwFormat /*= 0*/, DWORD dwUsage /*= 0*/)
{
	ReleaseBuffer();

	m_bufferType = BufferType_IndexBuffer;
	m_nBufferSize = nBufferSize;
#ifdef USE_DIRECTX_RENDERER
	return SUCCEEDED(CGlobals::GetRenderDevice()->CreateIndexBuffer(nBufferSize, 0, (D3DFORMAT)dwFormat, D3DPOOL_MANAGED, &m_indexBuffer, NULL));
#elif defined(USE_OPENGL_RENDERER)
	glGenBuffers(1, &m_vertexBuffer);
	PE_CHECK_GL_ERROR_DEBUG();
	return true;
#else
	return false;
#endif
}


bool ParaEngine::ParaVertexBuffer::CreateMemoryBuffer(uint32 nBufferSize, DWORD dwFormat /*= 0*/, DWORD dwUsage /*= 0*/)
{
	ReleaseBuffer();
	m_bufferType = BufferType_MemoryBuffer;
	m_nBufferSize = nBufferSize;
	if (nBufferSize > 0)
		m_buffer = new char[m_nBufferSize];
	return true;
}

void ParaEngine::ParaVertexBuffer::ReleaseBuffer()
{
	if (m_vertexBuffer != 0)
	{
#ifdef USE_DIRECTX_RENDERER
		if (m_bufferType == BufferType_VertexBuffer){
			SAFE_RELEASE(m_vertexBuffer);
		}
		else{
			SAFE_RELEASE(m_indexBuffer);
		}
#elif defined(USE_OPENGL_RENDERER)
		SAFE_DELETE_ARRAY(m_buffer);
		if (glIsBuffer(m_vertexBuffer))
		{
			glDeleteBuffers(1, &m_vertexBuffer);
			m_vertexBuffer = 0;
		}
#endif
	}
	else if (m_bufferType == BufferType_MemoryBuffer)
	{
		SAFE_DELETE_ARRAY(m_buffer);
	}
}

bool ParaEngine::ParaVertexBuffer::Lock(void** ppData, uint32 offsetToLock /*= 0*/, uint32 sizeToLock /*= 0*/, DWORD dwFlag)
{
	if (m_vertexBuffer != 0 && m_nBufferSize>0)
	{
#ifdef USE_DIRECTX_RENDERER
		if (m_bufferType == BufferType_VertexBuffer)
			m_vertexBuffer->Lock(offsetToLock, sizeToLock, ppData, dwFlag);
		else
			m_indexBuffer->Lock(offsetToLock, sizeToLock, ppData, dwFlag);
#elif defined(USE_OPENGL_RENDERER)
		SAFE_DELETE_ARRAY(m_buffer);
		m_buffer = new char[m_nBufferSize];
		*ppData = (void*)m_buffer;
#endif
		return true;
	}
	else if (m_bufferType == BufferType_MemoryBuffer)
	{
		*ppData = (void*)m_buffer;
	}
	return false;
}

void ParaEngine::ParaVertexBuffer::Unlock()
{
	if (m_bufferType == BufferType_MemoryBuffer)
		return;
#ifdef USE_DIRECTX_RENDERER
	if (m_vertexBuffer != 0)
	{
		if (m_bufferType == BufferType_VertexBuffer)
			m_vertexBuffer->Unlock();
		else
			m_indexBuffer->Unlock();
	}
#elif defined(USE_OPENGL_RENDERER)
	if (m_buffer != 0)
	{
		if (m_bufferType == BufferType_VertexBuffer)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, m_nBufferSize, m_buffer, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		else if (m_bufferType == BufferType_IndexBuffer)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nBufferSize, m_buffer, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		PE_CHECK_GL_ERROR_DEBUG();
		SAFE_DELETE_ARRAY(m_buffer);
	}
#endif
}

bool ParaEngine::ParaVertexBuffer::IsValid() const
{
	return m_vertexBuffer != 0 || (m_bufferType == BufferType_MemoryBuffer && m_buffer!=0);
}

char* ParaEngine::ParaVertexBuffer::GetMemoryPointer()
{
	return m_buffer;
}

void ParaEngine::ParaVertexBuffer::UploadMemoryBuffer(const char* pBuffer, int32 nBufSize)
{
	nBufSize = nBufSize > 0 ? nBufSize : GetBufferSize();
#ifdef USE_OPENGL_RENDERER
	if (m_bufferType == BufferType_VertexBuffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, nBufSize, pBuffer, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else if (m_bufferType == BufferType_IndexBuffer)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, nBufSize, pBuffer, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	PE_CHECK_GL_ERROR_DEBUG();
#else
	char * pData = NULL;
	if(Lock((void**)&pData, 0, 0))
	{
		memcpy(pData, pBuffer, nBufSize);
		Unlock();
	}
#endif
}

ParaEngine::IndexBufferDevicePtr_type ParaEngine::ParaIndexBuffer::GetDevicePointer()
{
	return GetDevicePointerAsIndexBuffer();
}

bool ParaEngine::ParaIndexBuffer::CreateBuffer(uint32 nBufferSize, DWORD dwFormat /*= 0*/, DWORD dwUsage /*= 0*/)
{
	return CreateIndexBuffer(nBufferSize, dwFormat, dwUsage);
}

bool ParaEngine::ParaMemoryBuffer::CreateBuffer(uint32 nBufferSize, DWORD dwFormat /*= 0*/, DWORD dwUsage /*= 0*/)
{
	return CreateMemoryBuffer(nBufferSize, dwFormat, dwUsage);
}

char* ParaEngine::ParaMemoryBuffer::GetDevicePointer()
{
	return m_buffer;
}
