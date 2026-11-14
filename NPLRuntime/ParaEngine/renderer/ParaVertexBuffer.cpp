//-----------------------------------------------------------------------------
// Class:	Vertex buffer
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.10
// Desc: cross platform vertex/index buffer. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Render/IRenderDevice.h"
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

bool ParaEngine::ParaVertexBuffer::CreateBuffer(uint32 nBufferSize, DWORD dwFormat /*= 0*/, DWORD dwUsage /*= 0*/, EPoolType dwPool)
{
	(void)dwFormat;
	(void)dwPool;
	ReleaseBuffer();
	m_bufferType = BufferType_VertexBuffer;
	m_nBufferSize = nBufferSize;
	auto pd3dDevice = CGlobals::GetRenderDevice();
	if (!pd3dDevice || nBufferSize == 0)
		return false;
	ERHIBufferUsage usage = ERHIBufferUsage::Static;
	if (dwUsage & D3DUSAGE_DYNAMIC)
		usage |= ERHIBufferUsage::Dynamic;
	if (dwUsage & D3DUSAGE_WRITEONLY)
		usage |= ERHIBufferUsage::WriteOnly;
	m_vertexBuffer = pd3dDevice->CreateBuffer(nBufferSize, false, usage, nullptr);
	return m_vertexBuffer != nullptr;
}


bool ParaEngine::ParaVertexBuffer::CreateIndexBuffer(uint32 nBufferSize, DWORD dwFormat /*= 0*/, DWORD dwUsage /*= 0*/)
{
	(void)dwFormat;
	ReleaseBuffer();

	m_bufferType = BufferType_IndexBuffer;
	m_nBufferSize = nBufferSize;
	auto pd3dDevice = CGlobals::GetRenderDevice();
	if (!pd3dDevice || nBufferSize == 0)
		return false;
	ERHIBufferUsage usage = ERHIBufferUsage::Static;
	if (dwUsage & D3DUSAGE_DYNAMIC)
		usage |= ERHIBufferUsage::Dynamic;
	if (dwUsage & D3DUSAGE_WRITEONLY)
		usage |= ERHIBufferUsage::WriteOnly;
	m_indexBuffer = pd3dDevice->CreateBuffer(nBufferSize, true, usage, nullptr);
	return m_indexBuffer != nullptr;
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
		SAFE_DELETE_ARRAY(m_buffer);
		m_vertexBuffer->ReleaseRHIResource();
		m_vertexBuffer = nullptr;
	}
	else if (m_bufferType == BufferType_MemoryBuffer)
	{
		SAFE_DELETE_ARRAY(m_buffer);
	}
}

bool ParaEngine::ParaVertexBuffer::Lock(void** ppData, uint32 offsetToLock /*= 0*/, uint32 sizeToLock /*= 0*/, DWORD dwFlag)
{
	if (!ppData)
		return false;
	if (m_vertexBuffer != 0 && m_nBufferSize > 0)
	{
		ERHIMapMode mode = ERHIMapMode::Write;
		if (dwFlag & D3DLOCK_READONLY)
			mode = ERHIMapMode::Read;
		else if (dwFlag & D3DLOCK_DISCARD)
			mode = ERHIMapMode::WriteDiscard;
		else if (dwFlag & D3DLOCK_NOOVERWRITE)
			mode = ERHIMapMode::WriteNoOverwrite;
		uint32_t lockBytes = (sizeToLock > 0) ? sizeToLock : (m_nBufferSize > offsetToLock ? (m_nBufferSize - offsetToLock) : 0);
		*ppData = m_vertexBuffer->Map(offsetToLock, lockBytes, mode);
		return *ppData != nullptr;
	}
	else if (m_bufferType == BufferType_MemoryBuffer && m_buffer != nullptr)
	{
		*ppData = (void*)(m_buffer + offsetToLock);
		return true;
	}
	return false;
}

void ParaEngine::ParaVertexBuffer::Unlock()
{
	if (m_bufferType == BufferType_MemoryBuffer)
		return;
	if (m_vertexBuffer != nullptr)
	{
		m_vertexBuffer->Unmap();
	}
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
	if (m_bufferType != BufferType_MemoryBuffer && m_vertexBuffer != nullptr && pBuffer != nullptr && nBufSize > 0)
	{
		m_vertexBuffer->UpdateData(0, static_cast<uint32_t>(nBufSize), pBuffer);
		return;
	}
	char * pData = NULL;
	if(Lock((void**)&pData, 0, 0))
	{
		memcpy(pData, pBuffer, nBufSize);
		Unlock();
	}
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
