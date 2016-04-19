//-----------------------------------------------------------------------------
// Class:	Vertex buffer Pool
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.17
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaVertexBuffer.h"
#include "ParaVertexBufferPool.h"

using namespace ParaEngine;

// std::map<const std::string, ParaVertexBufferPool*> ParaEngine::ParaVertexBufferPool::s_pools;

ParaEngine::ParaVertexBufferPool::ParaVertexBufferPool(const AssetKey& key)
	: AssetEntity(key), m_nMaxPooledCount(10), m_nFullSizedBufferSize(65535), m_nTotalBufferBytes(0)
{

}

ParaEngine::ParaVertexBufferPool::ParaVertexBufferPool()
	: m_nMaxPooledCount(10), m_nFullSizedBufferSize(65535), m_nTotalBufferBytes(0)
{

}

ParaEngine::ParaVertexBufferPool::~ParaVertexBufferPool()
{
	Cleanup();
}

uint32 ParaEngine::ParaVertexBufferPool::GetFullSizedBufferSize() const
{
	return m_nFullSizedBufferSize;
}

void ParaEngine::ParaVertexBufferPool::SetFullSizedBufferSize(uint32 val)
{
	m_nFullSizedBufferSize = val;
}

uint32 ParaEngine::ParaVertexBufferPool::GetMaxPooledCount() const
{
	return m_nMaxPooledCount;
}

void ParaEngine::ParaVertexBufferPool::SetMaxPooledCount(uint32 val)
{
	m_nMaxPooledCount = val;
}

int ParaEngine::ParaVertexBufferPool::GetActiveBufferCount() const
{
	return (int)m_activeBuffers.size();
}

int ParaEngine::ParaVertexBufferPool::GetTotalBufferCount() const
{
	return GetActiveBufferCount() + (int)(m_unusedFullSizedBuffers.size());
}

void ParaEngine::ParaVertexBufferPool::Cleanup()
{
	for (auto vertexBuffer : m_unusedFullSizedBuffers)
	{
		if (vertexBuffer)
		{
			vertexBuffer->ReleaseBuffer();
			SAFE_DELETE(vertexBuffer);
		}
	}
	m_unusedFullSizedBuffers.clear();

	for (auto vertexBuffer : m_activeBuffers)
	{
		if (vertexBuffer){
			vertexBuffer->ReleaseBuffer();
			SAFE_DELETE(vertexBuffer);
		}
	}
	m_activeBuffers.clear();
	m_nTotalBufferBytes = 0;
}


HRESULT ParaEngine::ParaVertexBufferPool::RendererRecreated()
{
	for (auto vertexBuffer : m_unusedFullSizedBuffers)
	{
		if (vertexBuffer)
		{
			vertexBuffer->RendererRecreated();
			SAFE_DELETE(vertexBuffer);
		}
	}
	m_unusedFullSizedBuffers.clear();

	for (auto vertexBuffer : m_activeBuffers)
	{
		if (vertexBuffer){
			vertexBuffer->RendererRecreated();
			SAFE_DELETE(vertexBuffer);
		}
	}
	m_activeBuffers.clear();
	m_nTotalBufferBytes = 0;
	return S_OK;
}

void ParaEngine::ParaVertexBufferPool::TickCache()
{
	if (m_unusedFullSizedBuffers.size() > GetMaxPooledCount())
	{
		int nRemoveCount = m_unusedFullSizedBuffers.size() - GetMaxPooledCount();
		for (int i = 0; i < nRemoveCount; ++i)
		{
			auto vertexBuffer = m_unusedFullSizedBuffers.begin();
			if ((*vertexBuffer))
			{
				m_nTotalBufferBytes -= (*vertexBuffer)->GetBufferSize();
				(*vertexBuffer)->ReleaseBuffer();
				m_unusedFullSizedBuffers.erase(vertexBuffer);
			}
		}
	}
}

ParaVertexBuffer* ParaEngine::ParaVertexBufferPool::CreateBuffer(uint32 nBufferSize, DWORD dwFormat /*= 0*/, DWORD dwUsage /*= 0*/, D3DPOOL dwPool)
{
	if (nBufferSize == 0)
		return NULL;
	if (m_unusedFullSizedBuffers.size() > 0 && nBufferSize == GetFullSizedBufferSize())
	{
		for (auto iter = m_unusedFullSizedBuffers.begin(); iter != m_unusedFullSizedBuffers.end(); )
		{
			ParaVertexBuffer* vertexBuffer = *iter;
			if (vertexBuffer->IsValid())
			{
				m_unusedFullSizedBuffers.erase(iter);
				m_activeBuffers.insert(vertexBuffer);
				return vertexBuffer;
			}
			else
			{
				// this should never happen. 
				iter = m_unusedFullSizedBuffers.erase(iter);
				SAFE_DELETE(vertexBuffer);
			}
		}
	}
	ParaVertexBuffer* vertexBuffer = new ParaVertexBuffer();
	if (vertexBuffer)
	{
		m_activeBuffers.insert(vertexBuffer);
		vertexBuffer->CreateBuffer(nBufferSize, dwFormat, dwUsage, dwPool);
		m_nTotalBufferBytes += nBufferSize;
	}
	return vertexBuffer;
}

void ParaEngine::ParaVertexBufferPool::ReleaseBuffer(ParaVertexBuffer* pBuffer)
{
	if (pBuffer && pBuffer->IsValid() && pBuffer->GetBufferSize() == GetFullSizedBufferSize())
	{
		m_unusedFullSizedBuffers.insert(pBuffer);
		auto iter = m_activeBuffers.find(pBuffer);
		if (iter != m_activeBuffers.end())
		{
			m_activeBuffers.erase(iter);
		}
	}
	else
	{
		auto iter = m_activeBuffers.find(pBuffer);
		if (iter != m_activeBuffers.end())
		{
			ParaVertexBuffer* vertexBuffer = (*iter);
			if (vertexBuffer)
			{
				m_nTotalBufferBytes -= vertexBuffer->GetBufferSize();
				vertexBuffer->ReleaseBuffer();
				SAFE_DELETE(vertexBuffer);
			}
			m_activeBuffers.erase(iter);
		}
	}
}

void ParaEngine::CVertexBufferPoolManager::TickCache()
{
	for (auto itCurCP = m_items.begin(); itCurCP != m_items.end(); ++itCurCP)
	{
		itCurCP->second->TickCache();
	}
}

ParaVertexBufferPool* ParaEngine::CVertexBufferPoolManager::CreateGetPool(const std::string& name)
{
	pair<ParaVertexBufferPool*, bool> res = CreateEntity(name, name);
	return res.first;
}

CVertexBufferPoolManager& ParaEngine::CVertexBufferPoolManager::GetInstance()
{
	static CVertexBufferPoolManager s_instance;
	return s_instance;
}

size_t ParaEngine::CVertexBufferPoolManager::GetVertexBufferPoolTotalBytes()
{
	size_t nTotalBytes = 0;
	for (auto itCurCP = m_items.begin(); itCurCP != m_items.end(); ++itCurCP)
	{
		nTotalBytes += itCurCP->second->GetTotalBufferBytes();
	}
	return nTotalBytes;
}

size_t ParaEngine::ParaVertexBufferPool::GetTotalBufferBytes(bool bRecalculate)
{
	if (!bRecalculate)
		return m_nTotalBufferBytes;
	else
	{
		m_nTotalBufferBytes = 0;
		for (auto vertexBuffer : m_activeBuffers)
		{
			m_nTotalBufferBytes += vertexBuffer->GetBufferSize();
		}
		for (auto vertexBuffer : m_unusedFullSizedBuffers)
		{
			m_nTotalBufferBytes += vertexBuffer->GetBufferSize();
		}
		return m_nTotalBufferBytes;
	}
}


int ParaEngine::ParaVertexBufferPool::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	AssetEntity::InstallFields(pClass, bOverride);

	pClass->AddField("FullSizedBufferSize", FieldType_Int, (void*)SetFullSizedBufferSize_s, (void*)GetFullSizedBufferSize_s, NULL, NULL, bOverride);
	pClass->AddField("MaxPooledCount", FieldType_Int, (void*)SetMaxPooledCount_s, (void*)GetMaxPooledCount_s, NULL, NULL, bOverride);
	pClass->AddField("ActiveBufferCount", FieldType_Int, (void*)0, (void*)GetActiveBufferCount_s, NULL, NULL, bOverride);
	pClass->AddField("TotalBufferCount", FieldType_Int, (void*)0, (void*)GetTotalBufferCount_s, NULL, NULL, bOverride);
	pClass->AddField("TotalBufferBytes", FieldType_Int, (void*)0, (void*)GetTotalBufferBytes_s, NULL, NULL, bOverride);
	return S_OK;
}
