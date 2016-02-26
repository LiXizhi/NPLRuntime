// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.

#include "ParaEngine.h"
#include "TriangleFan.h"

using namespace ParaTerrain;

TriangleFan::TriangleFan(const TriangleFan& tf)
{
	memcpy(this, &tf, sizeof(TriangleFan));
}

TriangleFan::TriangleFan()
{
}

TriangleFan::~TriangleFan()
{
}

int TriangleFan::GetTriangleNum()
{
	return m_NumberOfVertices-2;
}

void TriangleFan::Render(Terrain * pTerrain)
{
	//glDrawElements(GL_TRIANGLE_FAN, m_NumberOfVertices, GL_UNSIGNED_INT, m_pVertices);
	/*DirectXPerf::DrawIndexedPrimitiveUP(pd3dDevice, DirectXPerf::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLEFAN, 0, m_NumberOfVertices, m_NumberOfVertices-2, 
			m_pVertices,D3DFMT_INDEX16, pTerrain->m_pVertices, sizeof(FLOAT)*3);*/
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
int TriangleFan::BuildTriangles(Terrain * pTerrain, terrain_vertex * pIndices, int nStart)
{
	int nCur = nStart;
	terrain_vertex* pV;
	terrain_vertex verMinor2, verMinor1;

	int nIndex;
	SEND_VERTEX_NO_NORMAL(0, verMinor1);
	SEND_VERTEX_NO_NORMAL(1, verMinor2);
	SEND_VERTEX_NO_NORMAL(2, verMinor2);

	bool bCCW = false;
	for(int i=3;i<m_NumberOfVertices;++i)
	{
		pV = pIndices+(nCur++);
		*pV = verMinor1;

		pV = pIndices+(nCur++);
		*pV = verMinor2;

		SEND_VERTEX_NO_NORMAL(i, verMinor2);
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

int TriangleFan::BuildTriangles(Terrain * pTerrain, terrain_vertex_normal * pIndices, int nStart)
{
	int nCur = nStart;
	terrain_vertex_normal* pV;
	terrain_vertex_normal verMinor2, verMinor1;

	int nIndex;
	SEND_VERTEX(0, verMinor1);
	SEND_VERTEX(1, verMinor2);
	SEND_VERTEX(2, verMinor2);

	bool bCCW = false;
	for(int i=3;i<m_NumberOfVertices;++i)
	{
		pV = pIndices+(nCur++);
		*pV = verMinor1;

		pV = pIndices+(nCur++);
		*pV = verMinor2;
		
		SEND_VERTEX(i, verMinor2);
	}
	return (m_NumberOfVertices-2);
}