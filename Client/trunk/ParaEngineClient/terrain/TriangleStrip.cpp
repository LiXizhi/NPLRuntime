// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.


#include "ParaEngine.h"
#include "TriangleStrip.h"

using namespace ParaTerrain;

TriangleStrip::TriangleStrip()
:m_bEnabled(false),texture_group_id(-1)
{
}
TriangleStrip::TriangleStrip(const TriangleStrip& ts)
{
	if(ts.m_bEnabled == true)
	{
		memcpy(this, &ts, sizeof(TriangleStrip));
	}
	else
		m_bEnabled = false;
}
TriangleStrip::~TriangleStrip()
{
}

void TriangleStrip::Render(Terrain * pTerrain)
{
	/*if (m_bEnabled)
		glDrawElements(GL_TRIANGLE_STRIP, m_NumberOfVertices, GL_UNSIGNED_INT, m_pVertices);*/
	/*if (m_bEnabled)
	{
			DirectXPerf::DrawIndexedPrimitiveUP(pd3dDevice, DirectXPerf::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLESTRIP, 0, m_NumberOfVertices, m_NumberOfVertices-2, 
			m_pVertices,D3DFMT_INDEX16, pTerrain->m_pVertices, sizeof(FLOAT)*3);
	}*/
}

/** send a vertex to locked video memory */
#define SEND_VERTEX_NO_NORMAL(i, vertex) \
	nIndex = m_pVertices[i];\
	pTerrain->GetRenderVertex(nIndex, vertex.v);\
	pTerrain->GetTexCord(nIndex, vertex.tu1, vertex.tv1, vertex.tu2, vertex.tv2);\
	vertex.tu1 -= fOffsetTexU;\
	vertex.tv1 -= fOffsetTexV;\
	pV = pIndices+(nCur++);\
	(*pV) = vertex;

// build the triangle list indice for this triangular strip.
// pIndices is the place where the indice are saved
// nStart is the first position in pIndices where the indice are saved
// return the number of triangles built
int TriangleStrip::BuildTriangles(Terrain * pTerrain, terrain_vertex * pIndices, int nStart)
{
	int nCur = nStart;
	terrain_vertex* pV;
	terrain_vertex verMinor2, verMinor1;

	int nIndex;
	SEND_VERTEX_NO_NORMAL(0, verMinor1);
	SEND_VERTEX_NO_NORMAL(1, verMinor2);
	SEND_VERTEX_NO_NORMAL(2, verMinor1);

	bool bCCW = false;
	for(int i=3;i<m_NumberOfVertices;++i)
	{
		if((bCCW = !bCCW))
		{
			pV = pIndices+(nCur++);
			*pV = verMinor1;

			pV = pIndices+(nCur++);
			*pV = verMinor2;
		}
		else
		{
			pV = pIndices+(nCur++);
			*pV = verMinor2;

			pV = pIndices+(nCur++);
			*pV = verMinor1;
		}
		verMinor2 = verMinor1;

		SEND_VERTEX_NO_NORMAL(i, verMinor1);
	}
	return (m_NumberOfVertices-2);
}

/** send a vertex to locked video memory */
#define SEND_VERTEX(i, vertex) \
	nIndex = m_pVertices[i];\
	pTerrain->GetRenderVertex(nIndex, vertex.v);\
	pTerrain->GetRenderNorm(nIndex, vertex.n);\
	pTerrain->GetTexCord(nIndex, vertex.tu1, vertex.tv1, vertex.tu2, vertex.tv2);\
	vertex.tu1 -= fOffsetTexU;\
	vertex.tv1 -= fOffsetTexV;\
	pV = pIndices+(nCur++);\
	(*pV) = vertex;

int TriangleStrip::BuildTriangles(Terrain * pTerrain, terrain_vertex_normal * pIndices, int nStart)
{
	int nCur = nStart;
	terrain_vertex_normal* pV;
	terrain_vertex_normal verMinor2, verMinor1;
	
	int nIndex;
	SEND_VERTEX(0, verMinor1);
	SEND_VERTEX(1, verMinor2);
	SEND_VERTEX(2, verMinor1);

	bool bCCW = false;
	for(int i=3;i<m_NumberOfVertices;++i)
	{
		if((bCCW = !bCCW))
		{
			pV = pIndices+(nCur++);
			*pV = verMinor1;

			pV = pIndices+(nCur++);
			*pV = verMinor2;
		}
		else
		{
			pV = pIndices+(nCur++);
			*pV = verMinor2;

			pV = pIndices+(nCur++);
			*pV = verMinor1;
		}
		verMinor2 = verMinor1;

		SEND_VERTEX(i, verMinor1);
	}
	return (m_NumberOfVertices-2);

#ifdef OLD_CODE
	// this is for performance test. 
	//int c=0;for (int k=0;k<5000;++k){c+=k;}if(c<10) m_NumberOfVertices = 0;
	//return (m_NumberOfVertices-2);

	/*terrain_vertex_normal vTest;
	for(i=3;i<m_NumberOfVertices;++i)
	{
		pV = pIndices+(nCur++);
		nIndex = m_pVertices[i];
		*pV = vTest;
		pTerrain->GetTexCord(nIndex, pV->tu1, pV->tv1, pV->tu2, pV->tv2);

		pV = pIndices+(nCur++);
		nIndex = m_pVertices[i];
		*pV = vTest;
		pTerrain->GetTexCord(nIndex, pV->tu1, pV->tv1, pV->tu2, pV->tv2);

		pV = pIndices+(nCur++);
		nIndex = m_pVertices[i];
		*pV = vTest;
		pTerrain->GetTexCord(nIndex, pV->tu1, pV->tv1, pV->tu2, pV->tv2);
	}
	return (m_NumberOfVertices-2);*/

	int nCur = nStart;
	int i;
	terrain_vertex_normal* pV;
	int nIndex;
	for(i=0;i<3;++i)
	{
		pV = pIndices+(nCur++);
		nIndex = m_pVertices[i];
		pTerrain->GetRenderVertex(nIndex, (pV->v));
		pTerrain->GetTexCord(nIndex, pV->tu1, pV->tv1, pV->tu2, pV->tv2);
		pV->tu1 -= fOffsetTexU;
		pV->tv1 -= fOffsetTexV;
	}

	bool bCCW = false;
	for(i=3;i<m_NumberOfVertices;++i)
	{
		if(bCCW = !bCCW)
		{
			pV = pIndices+(nCur++);
			*pV = *(pV-1);

			pV = pIndices+(nCur++);
			*pV = *(pV-3);
		}
		else
		{
			pV = pIndices+(nCur++);
			*pV = *(pV-3);

			pV = pIndices+(nCur++);
			*pV = *(pV-2);
		}

		pV = pIndices+(nCur++);
		nIndex = m_pVertices[i];

		pTerrain->GetRenderVertex(nIndex, (pV->v));
		pTerrain->GetTexCord(nIndex, pV->tu1, pV->tv1, pV->tu2, pV->tv2);
		pV->tu1 -= fOffsetTexU;
		pV->tv1 -= fOffsetTexV;
	}
	return (m_NumberOfVertices-2);
#endif
}