//-----------------------------------------------------------------------------
// Class:	LatentOcclusionQueryBank
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.4.10
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "OcclusionQueryBank.h"

using namespace ParaEngine;

/**@def Max of 30 queries in flight at once */
#define MAX_QUERIES 30

LatentOcclusionQueryBank::LatentOcclusionQueryBank(IRenderDevice* pD3DDev)
	: m_bIsValid(false), m_head(0), m_nFirstUnusedIndex(0), m_nFirstQueryIndex(0)
{
	if (!pD3DDev)
		pD3DDev = CGlobals::GetRenderDevice();

	if (!pD3DDev || !pD3DDev->GetCaps().supportsOcclusionQuery)
		return;

	m_Queries.resize(MAX_QUERIES);

	for (int i = 0; i < MAX_QUERIES; i++)
	{
		IRHIOcclusionQuery* query = pD3DDev->CreateOcclusionQuery();
		if (!query)
		{
			Cleanup();
			return;
		}
		m_Queries[i].query = query;
		m_Queries[i].state = LatentQueryElement::UNUSED;
	}

	m_bIsValid = true;
}

LatentOcclusionQueryBank::~LatentOcclusionQueryBank()
{
	Cleanup();
}

void LatentOcclusionQueryBank::Cleanup()
{
	m_bIsValid = false;
	int nSize = (int)m_Queries.size();
	for (int i = 0; i < nSize; i++)
	{
		ReleaseRHIResource(m_Queries[i].query);
	}
	m_Queries.clear();
}

bool LatentOcclusionQueryBank::IsValid()
{
	return m_bIsValid;
}

HRESULT LatentOcclusionQueryBank::GetLatestResults(DWORD* count)
{
	if (!m_bIsValid || m_Queries.empty())
		return E_FAIL;

	int i = m_head;
	HRESULT rhr = S_FALSE;
	do
	{
		int idx = i % MAX_QUERIES;
		if (m_Queries[idx].state != LatentQueryElement::ISSUEDEND ||
			(i != m_head && idx == m_head))
			break;

		uint64_t samples = 0;
		if (m_Queries[idx].query->GetResult(samples, false))
		{
			if (count)
				*count = (DWORD)samples;
			rhr = S_OK;
			m_Queries[idx].state = LatentQueryElement::UNUSED;
		}
		else
			break;

		i++;
	} while (1);

	return rhr;
}

HRESULT LatentOcclusionQueryBank::BeginNextQuery()
{
	if (!m_bIsValid || m_Queries.empty())
		return E_FAIL;

	int i = m_head;
	do
	{
		int idx = i % MAX_QUERIES;
		if (i != m_head && idx == m_head)
			break;

		if (m_Queries[idx].state == LatentQueryElement::UNUSED)
		{
			m_Queries[idx].state = LatentQueryElement::ISSUEDBEGIN;
			m_Queries[idx].query->Begin();
			return S_OK;
		}
		i++;
	} while (1);
	return S_FALSE;
}

HRESULT LatentOcclusionQueryBank::EndNextQuery()
{
	if (!m_bIsValid || m_Queries.empty())
		return E_FAIL;

	int i = m_head;
	do
	{
		int idx = i % MAX_QUERIES;
		if (i != m_head && idx == m_head)
			break;

		if (m_Queries[idx].state == LatentQueryElement::ISSUEDBEGIN)
		{
			m_Queries[idx].state = LatentQueryElement::ISSUEDEND;
			m_Queries[idx].query->End();
			return S_OK;
		}
		i++;
	} while (1);
	return S_FALSE;
}

uint32 LatentOcclusionQueryBank::GetNumActiveQueries()
{
	uint32 count = 0;
	for (uint32 i = 0; i < m_Queries.size(); i++)
	{
		if (m_Queries[i].state != LatentQueryElement::UNUSED)
			++count;
	}
	return count;
}

bool LatentOcclusionQueryBank::HasUnusedQuery()
{
	if (!m_bIsValid || m_Queries.empty())
		return false;
	return (m_Queries[m_nFirstUnusedIndex].state == LatentQueryElement::UNUSED);
}

HRESULT LatentOcclusionQueryBank::BeginNewQuery(void* pUserData /*= NULL*/)
{
	if (!m_bIsValid || m_Queries.empty())
		return E_FAIL;

	int i = m_nFirstUnusedIndex;
	if (m_Queries[i].state == LatentQueryElement::UNUSED)
	{
		m_Queries[i].state = LatentQueryElement::ISSUEDBEGIN;
		m_Queries[i].pUserData = pUserData;
		m_Queries[i].query->Begin();
		return S_OK;
	}
	return S_FALSE;
}

HRESULT LatentOcclusionQueryBank::EndNewQuery()
{
	if (!m_bIsValid || m_Queries.empty())
		return E_FAIL;

	int i = m_nFirstUnusedIndex;
	if (m_Queries[i].state == LatentQueryElement::ISSUEDBEGIN)
	{
		m_Queries[i].state = LatentQueryElement::ISSUEDEND;
		m_Queries[i].query->End();
		m_nFirstUnusedIndex = (i + 1) % MAX_QUERIES;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT LatentOcclusionQueryBank::WaitForFirstResult(DWORD* count, void** pUserData)
{
	if (!m_bIsValid || m_Queries.empty())
		return E_FAIL;

	int i = m_nFirstUnusedIndex;
	for (int k = 0; k < MAX_QUERIES; ++k)
	{
		if (m_Queries[i].state == LatentQueryElement::ISSUEDEND)
		{
			uint64_t samples = 0;
			while (!m_Queries[i].query->GetResult(samples, true))
			{
			}
			m_Queries[i].state = LatentQueryElement::UNUSED;
			if (count)
				*count = (DWORD)samples;
			if (pUserData != 0)
				*pUserData = m_Queries[i].pUserData;
			m_Queries[i].pUserData = NULL;
			return S_OK;
		}
		i = (i + 1) % MAX_QUERIES;
	}
	return S_FALSE;
}

HRESULT LatentOcclusionQueryBank::CheckForFirstResult(DWORD* count, void** pUserData)
{
	if (!m_bIsValid || m_Queries.empty())
		return E_FAIL;

	int i = m_nFirstUnusedIndex;
	for (int k = 0; k < MAX_QUERIES; ++k)
	{
		if (m_Queries[i].state == LatentQueryElement::ISSUEDEND)
		{
			uint64_t samples = 0;
			if (m_Queries[i].query->GetResult(samples, false))
			{
				if (count)
					*count = (DWORD)samples;
				if (pUserData != 0)
					*pUserData = m_Queries[i].pUserData;
				m_Queries[i].state = LatentQueryElement::UNUSED;
				m_Queries[i].pUserData = NULL;
				return S_OK;
			}
			return S_FALSE;
		}
		i = (i + 1) % MAX_QUERIES;
	}
	return S_FALSE;
}
