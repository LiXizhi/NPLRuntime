#pragma once
#include "Terrain.h"
#include "TerrainBlock.h"
#include "VertexFVF.h"

namespace ParaTerrain
{
	class TriangleFan
	{
	public:
		TriangleFan();
		TriangleFan(const TriangleFan& tf);
		~TriangleFan();
		void Render(Terrain * pTerrain);
		int BuildTriangles(Terrain * pTerrain, terrain_vertex * pIndices, int nStart);
		int BuildTriangles(Terrain * pTerrain, terrain_vertex_normal * pIndices, int nStart);
		int GetTriangleNum();
	private:
		int m_pVertices[MAX_VERTICES_PER_FAN];	// Indices into the terrain vertices
		// TBD: It is a pretty awful waste of memory to preallocate MAX_VERTICES_PER_FAN vertices for every triangle fan,
		// when in most cases only a few vertices are needed. However, dynamically allocating these vertices
		// during every tessellation is not an option either because it causes huge performance problems and
		// badly fragments memory. Any good ideas for this?
		unsigned char m_NumberOfVertices;
		float fOffsetTexU, fOffsetTexV;
		// in range [0, 8*8)
		int texture_group_id;

		friend class TerrainBlock;
		friend class Terrain;
		friend class TerrainBuffer;
	};
}

