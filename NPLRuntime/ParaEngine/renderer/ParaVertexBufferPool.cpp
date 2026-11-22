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
	: AssetEntity(key), m_nMaxPooledCount(10), m_nFullSizedBufferSize(65535), m_nTotalBufferBytes(0), m_nUnusedBufferBytes(0)
{

}

ParaEngine::ParaVertexBufferPool::ParaVertexBufferPool()
	: m_nMaxPooledCount(10), m_nFullSizedBufferSize(65535), m_nTotalBufferBytes(0), m_nUnusedBufferBytes(0)
{

}

ParaEngine::ParaVertexBufferPool::~ParaVertexBufferPool()
{
	Cleanup();
}

uint32 ParaEngine::ParaVertexBufferPool::RoundUpBucketSize(uint32 nBufferSize) const
{
	if (nBufferSize == 0)
		return 0;
	const uint32 fullSize = GetFullSizedBufferSize();
	if (fullSize > 0 && nBufferSize >= fullSize)
		return fullSize;

	// Use power-of-two multiples of 32 block faces (32 * 4 * 20 = 2560 bytes)
	// so partial chunk VBs reuse device buffers with bounded internal fragmentation.
	uint32 bucket = 2560u;
	while (bucket < nBufferSize)
	{
		if (fullSize > 0 && bucket * 2u >= fullSize)
			return fullSize;
		if (bucket > (0x7fffffffu / 2u))
			return nBufferSize;
		bucket *= 2u;
	}
	return bucket;
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
	int unusedCount = (int)m_unusedFullSizedBuffers.size();
	for (const auto& kv : m_unusedBuckets)
		unusedCount += (int)kv.second.size();
	return GetActiveBufferCount() + unusedCount;
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

	for (auto& kv : m_unusedBuckets)
	{
		for (auto* vertexBuffer : kv.second)
		{
			if (vertexBuffer)
			{
				vertexBuffer->ReleaseBuffer();
				SAFE_DELETE(vertexBuffer);
			}
		}
	}
	m_unusedBuckets.clear();

	for (auto vertexBuffer : m_activeBuffers)
	{
		if (vertexBuffer){
			vertexBuffer->ReleaseBuffer();
			SAFE_DELETE(vertexBuffer);
		}
	}
	m_activeBuffers.clear();
	m_nTotalBufferBytes = 0;
	m_nUnusedBufferBytes = 0;
}

HRESULT ParaEngine::ParaVertexBufferPool::DeleteDeviceObjects()
{
	m_bIsInitialized = false;
	RendererRecreated();
	return S_OK;
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

	for (auto& kv : m_unusedBuckets)
	{
		for (auto* vertexBuffer : kv.second)
		{
			if (vertexBuffer)
			{
				vertexBuffer->RendererRecreated();
				SAFE_DELETE(vertexBuffer);
			}
		}
	}
	m_unusedBuckets.clear();

	for (auto vertexBuffer : m_activeBuffers)
	{
		if (vertexBuffer){
			vertexBuffer->RendererRecreated();
			SAFE_DELETE(vertexBuffer);
		}
	}
	m_activeBuffers.clear();
	m_nTotalBufferBytes = 0;
	m_nUnusedBufferBytes = 0;
	return S_OK;
}

void ParaEngine::ParaVertexBufferPool::TickCache()
{
	const size_t maxUnusedBytes = (size_t)GetMaxPooledCount() * (size_t)(GetFullSizedBufferSize() > 0 ? GetFullSizedBufferSize() : 65536u);
	while (m_nUnusedBufferBytes > maxUnusedBytes && !m_unusedBuckets.empty())
	{
		// Evict largest unused buffers first to reclaim VRAM quickly.
		auto bucketIter = std::prev(m_unusedBuckets.end());
		auto& vec = bucketIter->second;
		if (!vec.empty())
		{
			ParaVertexBuffer* vertexBuffer = vec.back();
			vec.pop_back();
			if (vertexBuffer)
			{
				const size_t sz = vertexBuffer->GetBufferSize();
				m_nTotalBufferBytes = (m_nTotalBufferBytes >= sz) ? (m_nTotalBufferBytes - sz) : 0;
				m_nUnusedBufferBytes = (m_nUnusedBufferBytes >= sz) ? (m_nUnusedBufferBytes - sz) : 0;
				vertexBuffer->ReleaseBuffer();
				SAFE_DELETE(vertexBuffer);
			}
		}
		if (vec.empty())
			m_unusedBuckets.erase(bucketIter);
	}

	if (m_unusedFullSizedBuffers.size() > GetMaxPooledCount())
	{
		int nRemoveCount = (int)m_unusedFullSizedBuffers.size() - (int)GetMaxPooledCount();
		for (int i = 0; i < nRemoveCount && !m_unusedFullSizedBuffers.empty(); ++i)
		{
			auto iter = m_unusedFullSizedBuffers.begin();
			ParaVertexBuffer* vertexBuffer = *iter;
			m_unusedFullSizedBuffers.erase(iter);
			if (vertexBuffer)
			{
				const size_t sz = vertexBuffer->GetBufferSize();
				m_nTotalBufferBytes = (m_nTotalBufferBytes >= sz) ? (m_nTotalBufferBytes - sz) : 0;
				m_nUnusedBufferBytes = (m_nUnusedBufferBytes >= sz) ? (m_nUnusedBufferBytes - sz) : 0;
				vertexBuffer->ReleaseBuffer();
				SAFE_DELETE(vertexBuffer);
			}
		}
	}
}

ParaVertexBuffer* ParaEngine::ParaVertexBufferPool::CreateBuffer(uint32 nBufferSize, DWORD dwFormat /*= 0*/, DWORD dwUsage /*= 0*/, EPoolType dwPool)
{
	if (nBufferSize == 0)
		return NULL;
	const uint32 bucketSize = RoundUpBucketSize(nBufferSize);
	auto bucketIter = m_unusedBuckets.find(bucketSize);
	if (bucketIter != m_unusedBuckets.end())
	{
		auto& vec = bucketIter->second;
		while (!vec.empty())
		{
			ParaVertexBuffer* vertexBuffer = vec.back();
			vec.pop_back();
			const size_t sz = vertexBuffer ? vertexBuffer->GetBufferSize() : 0;
			m_nUnusedBufferBytes = (m_nUnusedBufferBytes >= sz) ? (m_nUnusedBufferBytes - sz) : 0;
			if (vertexBuffer && vertexBuffer->IsValid())
			{
				if (vec.empty())
					m_unusedBuckets.erase(bucketIter);
				m_activeBuffers.insert(vertexBuffer);
				return vertexBuffer;
			}
			else if (vertexBuffer)
			{
				m_nTotalBufferBytes = (m_nTotalBufferBytes >= sz) ? (m_nTotalBufferBytes - sz) : 0;
				SAFE_DELETE(vertexBuffer);
			}
		}
		m_unusedBuckets.erase(bucketIter);
	}

	ParaVertexBuffer* vertexBuffer = new ParaVertexBuffer();
	if (vertexBuffer)
	{
		m_activeBuffers.insert(vertexBuffer);
		vertexBuffer->CreateBuffer(bucketSize, dwFormat, dwUsage, dwPool);
		m_nTotalBufferBytes += bucketSize;
	}
	return vertexBuffer;
}

void ParaEngine::ParaVertexBufferPool::ReleaseBuffer(ParaVertexBuffer* pBuffer)
{
	if (!pBuffer)
		return;
	auto iter = m_activeBuffers.find(pBuffer);
	if (iter != m_activeBuffers.end())
		m_activeBuffers.erase(iter);

	if (pBuffer->IsValid() && pBuffer->GetBufferSize() > 0)
	{
		const uint32 sz = pBuffer->GetBufferSize();
		m_unusedBuckets[sz].push_back(pBuffer);
		m_nUnusedBufferBytes += sz;
	}
	else
	{
		const size_t sz = pBuffer->GetBufferSize();
		m_nTotalBufferBytes = (m_nTotalBufferBytes >= sz) ? (m_nTotalBufferBytes - sz) : 0;
		pBuffer->ReleaseBuffer();
		SAFE_DELETE(pBuffer);
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
		m_nUnusedBufferBytes = 0;
		for (auto vertexBuffer : m_activeBuffers)
		{
			m_nTotalBufferBytes += vertexBuffer->GetBufferSize();
		}
		for (auto vertexBuffer : m_unusedFullSizedBuffers)
		{
			m_nTotalBufferBytes += vertexBuffer->GetBufferSize();
			m_nUnusedBufferBytes += vertexBuffer->GetBufferSize();
		}
		for (const auto& kv : m_unusedBuckets)
		{
			for (auto* vertexBuffer : kv.second)
			{
				if (vertexBuffer)
				{
					m_nTotalBufferBytes += vertexBuffer->GetBufferSize();
					m_nUnusedBufferBytes += vertexBuffer->GetBufferSize();
				}
			}
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
