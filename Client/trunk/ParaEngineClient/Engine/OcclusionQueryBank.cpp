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


LatentOcclusionQueryBank::LatentOcclusionQueryBank( IDirect3DDevice9 *  pD3DDev)
:m_bIsValid(true), m_nFirstUnusedIndex(0), m_nFirstQueryIndex(0)
{
	//m_Queries.reserve(MAX_QUERIES);
	m_Queries.resize(MAX_QUERIES);

	for(int i=0;i<MAX_QUERIES;i++)
	{
		LPDIRECT3DQUERY9 query = NULL;
		if(FAILED(pD3DDev->CreateQuery(D3DQUERYTYPE_OCCLUSION, &query))) 
		{
			m_bIsValid = false;
			Cleanup();
			return;
		}
		m_Queries[i].query = query;
		m_Queries[i].state = LatentQueryElement::UNUSED;
	}

	// init our head pointer to the first element.  head is always the oldest query in use
	m_head = 0;
}
LatentOcclusionQueryBank::~LatentOcclusionQueryBank()
{
	Cleanup();
}

void LatentOcclusionQueryBank::Cleanup()
{
	m_bIsValid = false;
	int nSize = (int)m_Queries.size();
	for(int i=0;i<nSize;i++)
	{
		SAFE_RELEASE(m_Queries[i].query);
	}
	m_Queries.clear();
}

bool LatentOcclusionQueryBank::IsValid()
{
	return m_bIsValid;
}

//////////////////////////////////////////////////////////////////////////
// Returns the most recent results of outstanding queries.  Once results have
// been encountered the query becomes unused again.  If multiple queries
// finished since the last call, then only the most recent results will be returned.
HRESULT LatentOcclusionQueryBank::GetLatestResults(DWORD *count)
{
	// loop from head to end, and then from beginning to head
	int i = m_head;
	HRESULT rhr = S_FALSE;
	do 
	{
		// only go till we haven't issued yet or we've wrapped around to the head
		if(    m_Queries[i].state != LatentQueryElement::ISSUEDEND || 
			(i!=m_head && (i%MAX_QUERIES) == m_head))
			break;

		// Test the oldest queries first
		DWORD tempCount = 0;
		HRESULT hr = m_Queries[i].query->GetData(&tempCount,sizeof(DWORD),0);

		// if we have a result, then record it and keep testing
		if(hr == S_OK)
		{
			*count = tempCount;
			rhr = S_OK;
			m_Queries[i].state = LatentQueryElement::UNUSED;
		}
		else
			break;

		i++;
	} while(1);

	return rhr;
}

//////////////////////////////////////////////////////////////////////////
// Issue a new begin occlusion query into the pushbuffer.
HRESULT LatentOcclusionQueryBank::BeginNextQuery()
{
	int i = m_head;
	do 
	{
		// only go till we've wrapped around to the head
		if(    (i!=m_head && (i%MAX_QUERIES) == m_head))
			break;

		// first unused query gets begin'd
		if(m_Queries[i].state == LatentQueryElement::UNUSED)
		{
			m_Queries[i].state = LatentQueryElement::ISSUEDBEGIN;
			m_Queries[i].query->Issue(D3DISSUE_BEGIN);
			return S_OK;
		}
		i++;
	}
	while(1);
	return S_FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Issue a new end occlusion query into the pushbuffer
HRESULT LatentOcclusionQueryBank::EndNextQuery()
{
	int i = m_head;
	do 
	{
		// only go till we've wrapped around to the head
		if(    (i!=m_head && (i%MAX_QUERIES) == m_head))
			break;

		// first begin'd query gets end'd
		if(m_Queries[i].state == LatentQueryElement::ISSUEDBEGIN)
		{
			m_Queries[i].state = LatentQueryElement::ISSUEDEND;
			m_Queries[i].query->Issue(D3DISSUE_END);
			return S_OK;
		}
		i++;
	}
	while(1);
	return S_FALSE;
}

uint32 LatentOcclusionQueryBank::GetNumActiveQueries()
{
	uint32 count = 0;
	for(uint32 i=0;i<m_Queries.size();i++)
	{
		if(m_Queries[i].state != LatentQueryElement::UNUSED)
			++count;
	}
	return count;
}

bool LatentOcclusionQueryBank::HasUnusedQuery()
{
	return (m_Queries[m_nFirstUnusedIndex].state == LatentQueryElement::UNUSED);
}

HRESULT LatentOcclusionQueryBank::BeginNewQuery(void* pUserData /*= NULL*/ )
{
	int i = m_nFirstUnusedIndex;
	if(m_Queries[i].state == LatentQueryElement::UNUSED)
	{
		m_Queries[i].state = LatentQueryElement::ISSUEDBEGIN;
		m_Queries[i].pUserData = pUserData;
		m_Queries[i].query->Issue(D3DISSUE_BEGIN);
		return S_OK;
	}
	return S_FALSE;
}

HRESULT LatentOcclusionQueryBank::EndNewQuery()
{
	int i = m_nFirstUnusedIndex;
	if(m_Queries[i].state == LatentQueryElement::ISSUEDBEGIN)
	{
		m_Queries[i].state = LatentQueryElement::ISSUEDEND;
		m_Queries[i].query->Issue(D3DISSUE_END);
		m_nFirstUnusedIndex = (i+1)%MAX_QUERIES;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT LatentOcclusionQueryBank::WaitForFirstResult(DWORD *count, void** pUserData)
{
	// the oldest query
	int i = m_nFirstUnusedIndex;
	HRESULT hr = S_FALSE;
	for (int k=0;k<MAX_QUERIES;++k)
	{
		if( m_Queries[i].state == LatentQueryElement::ISSUEDEND)
		{
			DWORD tempCount = 0;
			while((hr = m_Queries[i].query->GetData(&tempCount,sizeof(DWORD),D3DGETDATA_FLUSH)) == S_FALSE) 
				;
			m_Queries[i].state = LatentQueryElement::UNUSED;
			if(hr == S_OK)
			{
				*count = tempCount;
				if(pUserData!=0)
					*pUserData = m_Queries[i].pUserData;
			}
			m_Queries[i].pUserData = NULL;
			return hr;
		}
		i = (i+1)%MAX_QUERIES;
	}
	return hr;
}

HRESULT LatentOcclusionQueryBank::CheckForFirstResult(DWORD *count, void** pUserData)
{
	// the oldest query
	int i = m_nFirstUnusedIndex;
	HRESULT hr = S_FALSE;
	for (int k=0;k<MAX_QUERIES;++k)
	{
		if( m_Queries[i].state == LatentQueryElement::ISSUEDEND)
		{
			DWORD tempCount = 0;
			HRESULT hr = m_Queries[i].query->GetData(&tempCount,sizeof(DWORD), 0); 
			if(hr == S_OK)
			{
				*count = tempCount;
				if(pUserData!=0)
					*pUserData = m_Queries[i].pUserData;
				m_Queries[i].state = LatentQueryElement::UNUSED;
				m_Queries[i].pUserData = NULL;
			}
			return hr;
		}
		i = (i+1)%MAX_QUERIES;
	}
	return hr;
}

