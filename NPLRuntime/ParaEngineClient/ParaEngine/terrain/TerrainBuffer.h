#pragma once
#include "util/ParaMemPool.h"

namespace ParaTerrain
{
	using namespace ParaEngine;
	class Terrain;

	/**
	* it represents a texture batch of many triangles.
	*/
	struct TextureGroup
	{
	public:
		/** the index of the first triangle vertex in the m_pVertexBuffer buffer.*/
		int nStartIndex;
		/** The total number of triangles in this group.*/
		int nNumTriangles;
	};

	typedef FixedSizedAllocator<TextureGroup>	TextureGroup_Allocator;
	typedef std::vector <TextureGroup> TextureGroups_Type;

	/**
	* it contains all the necessary dynamic information to render the terrain for the current frame.
	* terrain buffer is only rebuilt when the model view matrix has changed, otherwise the old terrain buffer
	* is used for rendering the terrain.
	* E.g. The buffer contains triangle groups sorted by textures.
	* either low-res and high-res textures are specified.
	*/
	class TerrainBuffer
	{
	public:
		/** the terrain object for which this buffer is built */
		Terrain* m_pTerrain;
		/** current camera model view matrix */
		Matrix4 matModelview;

		/** the dynamic vertex buffer which hold a triangle vertex array */
		ParaVertexBuffer m_pVertexBuffer;

		/** number of reserved triangles in buffer */
		int m_nBufferSize;

		/** total number of vertices */
		int m_nNumOfTriangles;

		/** groups of object ordered by texture */
		TextureGroups_Type m_textureGroups;
	public:
		TerrainBuffer();
		~TerrainBuffer();
	public:

		VertexBufferDevicePtr_type GetVertexBufferDevicePtr();

		/** called internally by the terrain object*/
		void DeleteDeviceObjects();
		/** rebuild the terrain buffer from last tessellation data of the terrain object.
		* these data are mainly triangle fans and strips.
		*/
		void RebuildBuffer();

		/** return true if the buffer contains valid data for rendering */
		bool HasData();

		//GeoMipmapCode
		void BuildGeoMipmapBuffer();
		int GetChunkVertexOffset(int x, int y);
	};
}