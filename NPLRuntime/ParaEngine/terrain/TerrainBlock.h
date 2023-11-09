#pragma once
#include "TerrainCommon.h"
#include "util/ParaMemPool.h"
#include "TriangleStrip.h"
#include "TriangleFan.h"
#include "Terrain.h"
#include "Triangle.h"
#include "ShapeRay.h"
#include <queue>

#include "TerrainGeoMipmapIndices.h"

namespace ParaTerrain
{
	using namespace ParaEngine;
	class TerrainBlock;
	class TriangleStrip;
	typedef TerrainBlock* TerrainBlockPtr;
	typedef std::deque<TerrainBlockPtr,  FixedSizedAllocator<TerrainBlockPtr> >	deque_TerrainBlockPtr_Type;
	typedef std::queue<TerrainBlockPtr, deque_TerrainBlockPtr_Type> queue_TerrainBlockPtr_Type;

	/** it represents the quad tree node of LOD terrain data*/
	class TerrainBlock
	{
	public:
		TerrainBlock(TerrainBlock * pParent,bool useGeoMipmap = false);
		TerrainBlock(int homeVertex, int stride, Terrain * pTerrain, TerrainBlock * pParent,bool useGeoMipmap = false);
		 ~TerrainBlock();
		 /** pre-compute data in this block. This is a recursive function.*/
		 void CalculateGeometry(Terrain * pTerrain);
		/** generate triangles for rendering in higher LOD. This is a recursive function. */
		void Tessellate(uint32 *pCountStrips, Terrain * pTerrain);
		/** repair cracks in this block and child blocks. This is a recursive function.*/
		void RepairCracks(Terrain * pTerrain, uint32 *pCountFans);

		bool IsActive();

		inline int GetStride()
		{
			return m_Stride;
		}
		inline int GetHomeIndex()
		{
			return m_HomeIndex;
		}
		inline bool IsHole()
		{
			return m_bIsHole;
		}
		/** the amount of vertex error introduced if this block is simplified as just two triangles */
		inline float GetMaxError(){return m_fMaxError;}

		inline float GetMaxElevation(){return m_MaxElevation;}
		inline float GetMinElevation(){return m_MinElevation;}

		void EnableStrip(Terrain * pTerrain, bool bEnabled);
		
		void VertexChanged(Terrain * pTerrain);
		void VertexChanged(Terrain * pTerrain, int index1);
		void VertexChanged(Terrain * pTerrain, int index1, int index2);
		void IntersectRay(const CShapeRay & ray, Vector3 & intersectionPoint, float &lowestDistance, const Terrain * pTerrain);

		/// update holes in the terrain. This will set the m_bIsHold attribute of the terrain block. This function will 
		/// recursively call itself. Call this function when the terrain block has already been built.
		/// return true if any of its child block contains a hole. 
		bool UpdateHoles(Terrain * pTerrain);

private:
		TriangleStrip* CreateGetTriangleStrip(int nIndex, Terrain * pTerrain);
		TriangleFan* CreateGetTriangleFan(int nIndex, Terrain * pTerrain);

		void CreateTriangleStrip(uint32 *pCount, Terrain * pTerrain);
		/** return true if child nodes should be tessellated. used for breadth first traversal*/
		bool Tessellate_NonRecursive(uint32 *pCountStrips, Terrain * pTerrain);

		/** the amount of vertex error introduced if this block is simplified as just two triangles. we will compare each vertices in the region with the two planes of the simplified triangles. */
		void ComputeMaxVertexError(Terrain * pTerrain);

		TerrainBlock * m_pParent;
		TerrainBlock **m_pChildren;
		int m_HomeIndex;
		float m_MinElevation, m_MaxElevation;
		/** the amount of vertex error introduced if this block is simplified as just two triangles */
		float m_fMaxError;
		int m_nTriangleStripIndex;
		// in range [0, 8*8]; range [0, 8*8) matches the m_texture_cell_id. last one is reserved for blocks that are completely in the fog.
		short int m_texture_group_id;
		// in range [0, 8*8)
		short int m_texture_cell_id;
		// this is always 64
		short int m_texture_fog_id;

		/* on which fog side this block is on. 
		enum Side
		{
		NO_SIDE,
		POSITIVE_SIDE,
		NEGATIVE_SIDE,
		BOTH_SIDE
		};
		*/
		short int m_fog_side;
		/** whether this block in in distance fog. this function is only valid since last tessellation call.*/
		inline bool IsInFogRadius() {return m_texture_group_id == m_texture_cell_id; }

		/* on which fog side this block is on. 
		enum Side
		{
		NO_SIDE,
		POSITIVE_SIDE,
		NEGATIVE_SIDE,
		BOTH_SIDE
		};
		*/
		inline short int GetFogSide() {return m_fog_side; }

		/** indicating the current frustum state of this block. this function is only valid since last tessellation call.
		* 2 requires more testing(meaning either intersecting)
		* 1 is fast accept(meaning the box is fully inside the frustum)
		* 0 is fast reject
		*/
		short int m_frustum_state;
	public:
		inline short int GetFrustumState() {return m_frustum_state; }
	private:
		float m_texture_tile_x;
		float m_texture_tile_y;
		
#if _USE_RAYTRACING_SUPPORT_ > 0
		Triangle *m_pTriangles;
#endif
		short int m_Stride;
		/// whether this block and all of its child block is a hole.
		bool m_bIsHole:1;
		/// whether the block is a hole. 
		bool m_bHasHole:1;
		bool m_bIsActive:1;
		bool m_bChildrenActive:1;

		friend class Terrain;

		//geoMipmapCode
	public:
		IndexInfo* m_indexInfo;
		bool m_useGeoMipmap;
		int m_chunkCountX;

		int m_lodLevel;
		GeoMipmapChunkType m_chunkType;

		void SetLod(int level,GeoMipmapChunkType type);
		int GetLodLevel(){return m_lodLevel;}
		GeoMipmapChunkType GetChunkType(){return m_chunkType;}

	private:
			TerrainBlock(){};
			void InitMipmapBlock(int homeVertex,int stride,Terrain* pTerrain,TerrainBlock* pParent,int nCol,int nRow);
			void TessellateGeoMipmap(Terrain* pTerrain);
			void CalculateGeometryGeoMipmap(Terrain * pTerrain);
			int GetChildChunkCount()
			{
				if(m_useGeoMipmap)
					return m_chunkCountX*m_chunkCountX;
				else
					return 4;
			};
			TerrainBlock* GetChildBlock(int index);
			int GetTextureGroupId(){return m_texture_group_id;}
			bool IsInFog(){return m_texture_group_id == m_texture_fog_id;}
			void IntersectRayGeoMipmap(const CShapeRay & ray, Vector3 & intersectionPoint, float &lowestDistance, const Terrain * pTerrain);
			void IntersectRayTriangle(const CShapeRay & ray, Vector3 & intersectionPoint, float &lowestDistance, const Terrain * pTerrain,int startVertex);
			bool PointInTriangle(Triangle& tri,Vector3& point);
	};
}
