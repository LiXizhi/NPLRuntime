//-----------------------------------------------------------------------------
// Class:	CGlobals
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Terrain.h"
#include "TerrainBuffer.h"

using namespace ParaTerrain;

bool ParaTerrain::TerrainBuffer::HasData()
{
	return (m_pVertexBuffer) && (m_nNumOfTriangles > 0);
}

void ParaTerrain::TerrainBuffer::DeleteDeviceObjects()
{
	m_nBufferSize = 0;
	m_pVertexBuffer.ReleaseBuffer();
}

ParaTerrain::TerrainBuffer::~TerrainBuffer()
{
	DeleteDeviceObjects();
}

ParaTerrain::TerrainBuffer::TerrainBuffer()
{
	m_pTerrain = NULL;
	m_nNumOfTriangles = 0;
	m_nBufferSize = 0;
}

ParaEngine::VertexBufferDevicePtr_type ParaTerrain::TerrainBuffer::GetVertexBufferDevicePtr()
{
	return m_pVertexBuffer.GetDevicePointer();
}

void TerrainBuffer::BuildGeoMipmapBuffer()
{
	int terrainVertexWidth = m_pTerrain->m_WidthVertices;
	int blockVertexWidth = m_pTerrain->m_MaximumVisibleBlockSize + 1;
	int blockCountX = (terrainVertexWidth - 1) / (blockVertexWidth - 1);
	int blockCount = blockCountX * blockCountX;
	int vertexCount = blockVertexWidth * blockVertexWidth * blockCount;
	int bufferSize;

	bool useNormal = Settings::GetInstance()->UseNormals();
	if (useNormal)
		bufferSize = sizeof(terrain_vertex_normal)* vertexCount;
	else
		bufferSize = sizeof(terrain_vertex)* vertexCount;

	HRESULT hr = E_FAIL;
	void *pBufferData;

	if (!m_pVertexBuffer)
	{
		//create a static buffer and we assume the vertex number won't change during runtime
		if (m_pVertexBuffer.CreateBuffer(bufferSize, 0, 0))
		{
			if (m_pVertexBuffer.Lock((void**)&pBufferData, 0, 0))
				hr = S_OK;
		}
	}
	else
	{
		if (m_pVertexBuffer.Lock((void**)&pBufferData, 0, 0, D3DLOCK_DISCARD))
			hr = S_OK;
	}

	if (SUCCEEDED(hr))
	{
		if (useNormal)
		{
			terrain_vertex_normal* pVertexArr = (terrain_vertex_normal*)pBufferData;
			terrain_vertex_normal* temp = pVertexArr;
			int idx = 0;
			blockVertexWidth = blockVertexWidth - 1;
			for (int i = 0; i < blockCountX; i++)
			{
				for (int j = 0; j < blockCountX; j++)
				{
					int blockStartIdx = i * terrainVertexWidth * blockVertexWidth + j * blockVertexWidth;
					for (int y = 0; y < blockVertexWidth + 1; y++)
					{
						for (int x = 0; x < blockVertexWidth + 1; x++)
						{
							int tempIdx = blockStartIdx + terrainVertexWidth * y + x;
							terrain_vertex_normal vertex;
							Vector3& vec = m_pTerrain->m_pVertices[tempIdx];
							vertex.v.x = vec.x;
							vertex.v.y = vec.z;
							vertex.v.z = vec.y;

							m_pTerrain->GetRenderNorm(tempIdx, (vertex.n));
							m_pTerrain->GetTexCord(tempIdx, vertex.tu1, vertex.tv1, vertex.tu2, vertex.tv2);
							vertex.tu1 -= j;
							vertex.tv1 -= i;
							pVertexArr[idx++] = vertex;
						}
					}

				}
			}
		}
		else
		{
			int idx = 0;
			blockVertexWidth = blockVertexWidth - 1;
			terrain_vertex* pVertexArr = (terrain_vertex*)pBufferData;
			for (int i = 0; i < blockCountX; i++)
			{
				for (int j = 0; j < blockCountX; j++)
				{
					int blockStartIdx = i * terrainVertexWidth * blockVertexWidth + j * blockVertexWidth;
					for (int y = 0; y < blockVertexWidth + 1; y++)
					{
						int currentIdx = blockStartIdx + terrainVertexWidth * y;
						for (int x = 0; x < blockVertexWidth + 1; x++)
						{
							int tempIdx = blockStartIdx + terrainVertexWidth * y + x;
							terrain_vertex vertex;
							Vector3& vec = m_pTerrain->m_pVertices[tempIdx];
							vertex.v.x = vec.x;
							vertex.v.y = vec.z;
							vertex.v.z = vec.y;
							m_pTerrain->GetTexCord(tempIdx, vertex.tu1, vertex.tv1, vertex.tu2, vertex.tv2);
							vertex.tu1 -= j;
							vertex.tv1 -= i;
							pVertexArr[idx++] = vertex;
						}
					}
				}
			}
		}
		m_pVertexBuffer.Unlock();
	}
}

int TerrainBuffer::GetChunkVertexOffset(int x, int y)
{
	if (m_pTerrain->m_useGeoMipmap)
	{
		int chunkWidth = m_pTerrain->GetMaximumVisibleBlockSize() + 1;
		int chunkCountX = (m_pTerrain->m_WidthVertices - 1) / (chunkWidth - 1);
		return ((y * chunkCountX + x) * chunkWidth * chunkWidth);
	}
	else
	{
		return (y * m_pTerrain->m_WidthVertices * m_pTerrain->m_MaximumVisibleBlockSize +
			x * m_pTerrain->m_MaximumVisibleBlockSize);
	}
}

/** this is the most time consuming task compared with tessellation and repair crack */
void TerrainBuffer::RebuildBuffer()
{
	int i, j;
	if ((m_pTerrain->m_CountStrips + m_pTerrain->m_CountFans) == 0)
	{
		m_nNumOfTriangles = 0;
		return;
	}

	// clear all texture groups 
	int nTextureGroupCount = m_pTerrain->m_NumberOfTextureTiles + 1;
	m_textureGroups.resize(nTextureGroupCount);
	for (i = 0; i<nTextureGroupCount; ++i)
	{
		TextureGroup& texGroup = m_textureGroups[i];
		texGroup.nStartIndex = 0;
		texGroup.nNumTriangles = 0;
	}

	/// add get the total number of triangles for each texture group
	int nCount = (int)m_pTerrain->m_CountStrips;

	// #define TEST_REBUILD_PERFORMANCE
#ifdef TEST_REBUILD_PERFORMANCE
	/** this is the most time consuming task compared with tessellation and repair crack.
	* a single tile rebuild can take as long as 0.0011 seconds on 2.5GHZ PC.
	*/
	if (nCount<200)
		return;
	PERF1("RebuildTerrainBuffer");
#endif

	// we need to group triangle strips and fans by texture ID
	for (i = 0; i < nCount; ++i)
	{
		TriangleStrip& tri = m_pTerrain->m_pTriangleStrips[i];
		if (tri.IsEnabled())
		{
			TextureGroup& texGroup = m_textureGroups[tri.texture_group_id];
			texGroup.nNumTriangles += tri.GetTriangleNum();
		}
	}
	nCount = (int)m_pTerrain->m_CountFans;
	for (i = 0; i < nCount; ++i)
	{
		TriangleFan& tri = m_pTerrain->m_pTriangleFans[i];
		TextureGroup& texGroup = m_textureGroups[tri.texture_group_id];
		texGroup.nNumTriangles += tri.GetTriangleNum();
	}

	/// set the start triangle index of each texture group
	int nStartIndex = 0;

	for (i = 0; i<nTextureGroupCount; ++i)
	{
		TextureGroup& texGroup = m_textureGroups[i];
		texGroup.nStartIndex = nStartIndex;
		nStartIndex += texGroup.nNumTriangles;
		texGroup.nNumTriangles = 0;
	}
	m_nNumOfTriangles = nStartIndex;

	bool bUseNormal = Settings::GetInstance()->UseNormals();
	//
	// Create a vertex buffer that contains the vertex data sorted by texture cell group
	//
	if (m_nNumOfTriangles>0)
	{
		int result = E_FAIL;
		void *pVertices = NULL;

		int nNewBufferSize = 0;
		if (!bUseNormal)
			nNewBufferSize = 3 * m_nNumOfTriangles*sizeof(terrain_vertex);
		else
			nNewBufferSize = 3 * m_nNumOfTriangles*sizeof(terrain_vertex_normal);

		//PERF_BEGIN("TerraCreateBuf");
		if (m_nBufferSize<nNewBufferSize || (!m_pVertexBuffer))
		{
			/** recreate buffer, if the old buffer is too small.*/
			DeleteDeviceObjects();
			
			if (m_pVertexBuffer.CreateBuffer(nNewBufferSize, bUseNormal ? terrain_vertex_normal::FVF : terrain_vertex::FVF, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DPOOL_DEFAULT))
			{
				m_nBufferSize = nNewBufferSize;
				if(m_pVertexBuffer.Lock((void**)&pVertices, 0,0, D3DLOCK_DISCARD))
					result = S_OK;
			}
		}
		else
		{
			/** reuse the old buffer */
			if (m_pVertexBuffer.Lock((void**)&pVertices, 0, nNewBufferSize, D3DLOCK_DISCARD))
				result = S_OK;
		}
		if (SUCCEEDED(result))
		{
			if (!bUseNormal)
			{
				// do not use normal
				terrain_vertex * pVertexArray = (terrain_vertex *)pVertices;
				nCount = (int)m_pTerrain->m_CountStrips;
				for (j = 0; j < nCount; ++j)
				{
					TriangleStrip* pTri = m_pTerrain->GetTriStrip(j);
					if (pTri->IsEnabled())
					{
						TextureGroup& texGroup = m_textureGroups[pTri->texture_group_id];
						int nTriCount = (texGroup.nStartIndex + texGroup.nNumTriangles) * 3;
						texGroup.nNumTriangles += pTri->BuildTriangles(m_pTerrain, pVertexArray, nTriCount);
					}
				}
				nCount = (int)m_pTerrain->m_CountFans;
				for (j = 0; j < nCount; ++j)
				{
					TriangleFan* pTri = m_pTerrain->GetTriFan(j);
					TextureGroup& texGroup = m_textureGroups[pTri->texture_group_id];
					int nTriCount = (texGroup.nStartIndex + texGroup.nNumTriangles) * 3;
					texGroup.nNumTriangles += pTri->BuildTriangles(m_pTerrain, pVertexArray, nTriCount);
				}
			}
			else
			{
#ifdef TEST_REBUILD_PERFORMANCE
				PERF1("build_triangles");
				// the following build triangles are the most time consuming task : Avg: 0.0011s on 2.5GHZ PC
#endif

				//////////////////////////////////////////////////////////////////////////
				// render with normal
				m_pTerrain->GenerateTerrainNormal(false);

				terrain_vertex_normal * pVertexArray = (terrain_vertex_normal *)pVertices;
				nCount = (int)m_pTerrain->m_CountStrips;
				for (j = 0; j < nCount; ++j)
				{
					TriangleStrip* pTri = m_pTerrain->GetTriStrip(j);
					if (pTri->IsEnabled())
					{
						TextureGroup& texGroup = m_textureGroups[pTri->texture_group_id];
						int nTriCount = (texGroup.nStartIndex + texGroup.nNumTriangles) * 3;
						texGroup.nNumTriangles += pTri->BuildTriangles(m_pTerrain, pVertexArray, nTriCount);
					}
				}
				nCount = (int)m_pTerrain->m_CountFans;
				for (j = 0; j < nCount; ++j)
				{
					TriangleFan* pTri = m_pTerrain->GetTriFan(j);
					TextureGroup& texGroup = m_textureGroups[pTri->texture_group_id];
					int nTriCount = (texGroup.nStartIndex + texGroup.nNumTriangles) * 3;
					texGroup.nNumTriangles += pTri->BuildTriangles(m_pTerrain, pVertexArray, nTriCount);
				}
			}

			m_pVertexBuffer.Unlock();
		}
		else
		{
			m_nNumOfTriangles = 0;
		}
	}
}
