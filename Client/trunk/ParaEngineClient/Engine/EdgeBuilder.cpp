//-----------------------------------------------------------------------------
// Class: EdgeBuilder	
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "EdgeBuilder.h"
using namespace ParaEngine;


//--------------------------------------------------------------------
// edge hash class
//--------------------------------------------------------------------
EdgeHash::EdgeHash()
{
    m_v0 = -1;
    m_v1 = -1;
}

EdgeHash::EdgeHash(int v0, int v1)
{
    m_v0 = v0;
    m_v1 = v1;
}

unsigned int EdgeHash::hashCode() const
{
	return (m_v0+m_v1);
}

bool EdgeHash::operator==(const EdgeHash& rhs) const
{
    // return true for any edge that has vertices in equivalent positions
    return ( ( m_v0 == rhs.m_v0 && m_v1 == rhs.m_v1 ) ||
            ( m_v0 == rhs.m_v1 && m_v1 == rhs.m_v0 ) );
}

//--------------------------------------------------------------------
// edge builder class
//--------------------------------------------------------------------

CEdgeBuilder::CEdgeBuilder(void)
{
}

CEdgeBuilder::~CEdgeBuilder(void)
{
}

void CEdgeBuilder::AddEdge( unordered_set <EdgeHash, hash_compare_edge> & edgeTable, DWORD& dwNumEdges, WORD v0, WORD v1 )
{
	// Remove interior edges (which appear in the list twice)
	auto result = edgeTable.insert(EdgeHash(v0,v1));
	
	if(result.second)
		dwNumEdges++;
	else
	{
		edgeTable.erase(result.first);
		dwNumEdges--;
	}
}