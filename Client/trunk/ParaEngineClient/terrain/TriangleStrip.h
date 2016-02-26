#pragma once
#include "TerrainCommon.h"
#include "Terrain.h"
#include "TerrainBlock.h"
#include "VertexFVF.h"

namespace ParaTerrain
{
	class TriangleStrip
	{
	public:
		TriangleStrip();
		TriangleStrip(const TriangleStrip& ts);
		~TriangleStrip();
		void Render(Terrain * pTerrain);
		inline int GetTriangleNum()
		{
			return (m_bEnabled) ? (m_NumberOfVertices-2) : 0;
		}

		int BuildTriangles(Terrain * pTerrain, terrain_vertex * pIndices, int nStart);
		int BuildTriangles(Terrain * pTerrain, terrain_vertex_normal * pIndices, int nStart);
		inline bool IsEnabled(){return m_bEnabled;}
	private:
		uint32 m_pVertices[6];	// Indices into the terrain vertices
		unsigned char m_NumberOfVertices;
		float fOffsetTexU, fOffsetTexV;
		// in range [0, 8*8)
		int texture_group_id;
		bool m_bEnabled;

		friend class TerrainBlock;
		friend class Terrain;
		friend class TerrainBuffer;
	};
}
