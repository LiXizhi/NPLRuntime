#pragma once
#include "IAttributeFields.h"
#include "TerrainLoadListener.h"
#include "TextureSet.h"
#include "TextureFactory.h"
#include "TTerrain.h"
#include "TerrainGeoMipmapIndices.h"

namespace ParaEngine
{
	struct TextureEntity;
	class TTerrain;
	class CShapeAABB;
	class CShapeFrustum;
}
namespace ParaTerrain
{
	/** @brief You derive a concrete implementation of this interface in order to serve Terrains up to a TerrainLattice.
	*
	* TerrainLattice is a grid of Terrain objects that are stitched together and kept seamless, even under dynamic tessellation.
	* TerrainLattice needs to be fed each of the Terrain objects at each x,y position in the lattice as the user's camera moves around
	* the scene and Terrains become visible. When the camera moves in such a way that a Terrain in the lattice becomes visible, your object's
	* LoadTerrainAt() method will be called. You must load a Terrain by whatever means you like (e.g. using ElevationLoader and TextureLoader objects,
	* or raw arrays of data, etc.) and return that Terrain so that TerrainLattice can make it visible to the user. You may also want to load other
	* application-specific objects at this time (such as the buildings, etc. that populate the loaded Terrain - anything that is specific to THAT Terrain.)
	* When a TerrainObject is no longer visible, your object's UnloadTerrain() method will be called, giving you a chance to dispose of the Terrain 
	* and any application-specific objects.
	*/
	class TerrainLatticeLoader
	{
	public:
		/// \brief Called by the TerrainLattice when a Terrain object in the lattice has entered the visible region and, therefore, needs to be loaded into RAM.
		virtual Terrain * LoadTerrainAt(Terrain *pTerrain, int latticeX, int latticeY, bool useGeoMipmap = false) = 0;
		/// \brief Called by the TerrainLattice when a Terrain object in the lattice is no longer within the visible region and can, therefore, be disposed of, freeing RAM for other visible Terrain objects.
		virtual void UnloadTerrain(int latticeX, int latticeY, Terrain * pTerrain) = 0;
		/// \brief Returns the width in world units of each of the individual Terrain objects in the lattice (they must all be the same width.)
		virtual float GetTerrainWidth() = 0;
		/// \brief Returns the height in world units of each of the individual Terrain objects in the lattice (they must all be the same height.)
		virtual float GetTerrainHeight() = 0;

		/** update the tile config file for the terrain terrain tile x,y. return true if succeeded.
		One needs to SaveWorldConfigFile() in order for the changes to be written to disk.
		@param sTileConfigFile: if "", it will erase the tile. otherwise it will change the config file.
		*/
		virtual bool UpdateTileConfigFile(int x, int y, const string& sTileConfigFile) = 0;

		/** save world config file to disk. */
		virtual bool SaveWorldConfigFile() = 0;
	};

	/// \brief Manages a network of Terrain objects arranged in a grid pattern to model extremely large worlds that couldn't be handled by a 
	/// single Terrain object.

	/// TerrainLattice manages the seams between Terrain objects so that, even during dynamic tessellation, there are no visible cracks between
	/// neighboring Terrain objects in the lattice.
	/// \warning This class is temporarily disabled for the current release, but will be available again soon.
	class TerrainLattice : public IAttributeFields
	{
	public:
		typedef map<int, TerrainTileCacheItem> TerrainTileCacheMap_type;
		TerrainLattice(TerrainLatticeLoader * pLoader, bool useGeoMipmap = false);
		virtual ~TerrainLattice();

		ATTRIBUTE_DEFINE_CLASS(TerrainLattice);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		void Load(char *szBaseName, int maxNumTriangles, int maxBlockSize, float commonRepeats);
		void SetTileSize(float fTileSize);
		/**
		* Get the terrain at the tile location. And load the terrain if it has been cached
		*/
		Terrain *GetTerrain(int positionX, int positionY);
		Terrain *GetTerrainAtPoint(float x, float y);
		void SetCameraPosition(float x, float y, float z);
		void ModelViewMatrixChanged();
		void Render();
		float GetElevation(float x, float y);

		/** get value of a given terrain region layer
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		*/
		DWORD GetRegionValue(const string& sLayerName, float x, float y);

		/// \brief Returns the surface normal of the terrain at the specified point.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		/// \param normalX Gets filled with the surface normal x component
		/// \param normalY Gets filled with the surface normal y component
		/// \param normalZ Gets filled with the surface normal z component
		void GetNormal(float x, float y, float &normalX, float &normalY, float &normalZ);
		float IntersectRay(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, float fMaxDistance = INFINITY);

		/** Set the height of the lowest visible terrain point. This may be used to render the ocean*/
		void  SetLowestVisibleHeight(float fHeight);
		/** Get the height of the lowest visible terrain point. This may be used to render the ocean*/
		float GetLowestVisibleHeight();

		/** paint detailed texture on the terrain surface */
		void Paint(ParaEngine::TextureEntity* detailTexture, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y);

		/** Flatten the terrain both up and down to the specified elevation, using using the
		* tightness parameter to determine how much the altered points are allowed
		* to deviate from the specified elevation.
		* @param x: center of the circle in world unit
		* @param y: center of the circle in world unit
		* @param radius: radius of the inner circle in world unit
		* @param flatten_op: enum FlattenOperation{
		*	Fill_Op=0, //Flatten the terrain up to the specified elevation
		* 	ShaveTop_Op=1, //Flatten the terrain down to the specified elevation
		* 	Flatten_Op=2, //Flatten the terrain up and down to the specified elevation
		* };
		* @param elevation: the desired height
		* @param factor: value is between [0,1]. 1 means fully transformed; 0 means nothing is changed
		*/
		void Flatten(float x, float y, float radius, int flatten_op, float elevation, float factor);

		/**
		* flatten a land centered at x,y, with a specified radius.
		* Algorithm: (1) flatten a circle with radius same as fFlatPercentage*radius
		*			 (2) smooth the entire region twice.
		* @param x: center of the circle in world unit
		* @param y: center of the circle in world unit
		* @param radius: radius of the inner circle in world unit
		* @param fFlatPercentage: value is between [0,1]. fFlatPercentage*radius is the actual radius that got flattened.
		* @param factor: value is between [0,1]. 1 means fully transformed; 0 means nothing is changed
		*/
		void DigCircleFlat(float x, float y, float radius, float fFlatPercentage = 0.7f, float factor = 0.5f);

		/*
		*  This creates a Gaussian hill at the specified location with the specified parameters.
		*  it actually adds the hill to the original terrain surface.
		*  Here ElevNew(x,y) =
		|(x,y)-(center_x,center_y)| < radius*smooth_factor,	ElevOld(x,y)+hscale*exp(-[(x-center_x)^2+(y-center_y)^2]/(2*standard_deviation^2) ),
		|(x,y)-(center_x,center_y)| > radius*smooth_factor, minimize hill effect.
		* @param center_x: the center of the affected circle. value in the range [0,1]
		* @param center_y: the center of the affected circle.value in the range [0,1]
		* @param radius: the radius of the affected circle.value in the range [0,0.5]
		* @param hscale: scale factor. One can think of it as the maximum height of the Gaussian Hill. this value can be negative
		* @param standard_deviation: standard deviation of the unit height value. should be in the range (0,1).
		*  0.1 is common value. larger than that will just make a flat hill with smoothing.
		* @param smooth_factor: value is between [0,1]. 1 means fully transformed; 0 means nothing is changed
		*/
		void GaussianHill(float x, float y, float radius, float hscale, float standard_deviation = 0.1f, float smooth_factor = 0.5f);

		/**
		* offset in a spherical region
		* @param x: center of the circle in world unit
		* @param y: center of the circle in world unit
		* @param radius: radius of the inner circle in world unit
		*/
		void Spherical(float x, float y, float radius, float    offset);

		/**
		* Note: terrain data should be in normalized space with height in the range [0,1].
		* Picks a point and scales the surrounding terrain in a circular manner.
		* Can be used to make all sorts of circular shapes. Still needs some work.
		*  radial_scale: pick a point (center_x, center_y) and scale the points
		*      where distance is mindist<=distance<=maxdist linearly.  The formula
		*      we'll use for a nice sloping smoothing factor is (-cos(x*3)/2)+0.5.
		* @param x: center of the circle in world unit
		* @param y: center of the circle in world unit
		* @param scale_factor: height of the scaled portion in world unit. This value can be negative.
		*/
		void RadialScale(float x, float y, float scale_factor, float min_dist, float max_dist, float smooth_factor = 0.5f, int frequency = 1);

		/**
		square filter for sharpening and smoothing.
		Use neighbour-averaging to roughen or smooth the height field. The factor
		determines how much of the computed roughening is actually applied to the
		height field. In it's default invocation, the 4 directly neighboring
		squares are used to calculate the roughening. If you select big sampling grid,
		all 8 neighboring cells will be used.
		* @param x: center of the circle in world unit
		* @param y: center of the circle in world unit
		* @param radius: radius of the inner circle in world unit
		* @param roughen: true for sharpening, false for smoothing.
		* @param big_grid: true for 8 neighboring cells, false for 4.
		* @param factor: value is between [0,1]. 1 means fully transformed; 0 means nothing is changed
		*/
		void Roughen_Smooth(float x, float y, float radius, bool roughen = false, bool big_grid = false, float factor = 0.5f);

		/**
		* Add rectangular height field from file to the current terrain.  This is just a shortcut for MergeHeightField()
		* @param x: center of the rect in world unit
		* @param y: center of the rect in world unit
		* @param filename : the raw elevation or gray scale image file that contains the height field.
		* @param nSmoothPixels:  the number of pixels to smooth from the edge of the height field.
		* if this is 0, the original height field will be loaded unmodified. if it is greater than 0, the loaded height field
		* will be smoothed for nSmoothPixels from the edge, where the edge is always 0.
		*/
		void AddHeightField(float x, float y, const char* filename, int nSmoothPixels = 7);
		/**
		* merge a rectangular height field from file to the current terrain.
		* @param x: center of the rect in world unit
		* @param y: center of the rect in world unit
		* @param filename : the raw elevation or gray scale image file that contains the height field.
		* @param MergeOperation: {
		Addition=0,
		Subtract=1,
		Multiplication=2,
		Division=3,
		Minimum=4,
		Maximum=5,
		};
		* @param weight1: the destination merging weight
		* @param weight2: the source file merging weight
		* @param nSmoothPixels:  the number of pixels to smooth from the edge of the height field.
		* if this is 0, the original height field will be loaded unmodified. if it is greater than 0, the loaded height field
		* will be smoothed for nSmoothPixels from the edge, where the edge is always 0.
		*/
		void MergeHeightField(float x, float y, const char* filename, int mergeOperation = 0, float weight1 = 1.0, float weight2 = 1.0, int nSmoothPixels = 7);

		/** create a ramp (inclined slope) from (x1,y1) to (x2,y2). The ramp's half width is radius.
		* this is usually used to created a slope path connecting a high land with a low land.
		* @param radius: The ramp's half width
		* @param borderpercentage: borderpercentage*radius is how long the ramp boarder is to linearly interpolate with the original terrain. specify 0 for sharp ramp border.
		* @param factor: in range[0,1]. it is the smoothness to merge with other border heights.Specify 1.0 for a complete merge
		*/
		void Ramp(float x1, float y1, float x2, float y2, float radius, float borderpercentage = 0.5f, float factor = 1.0f);

		/** Update all holes in the terrain tile that contains the input point.
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		*/
		void UpdateHoles(float x, float y);
		/**
		* Whether the terrain contains a hole at the specified location.
		* Currently, we allow user to load a low resolution hole maps at the beginning of terrain creation.
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		* @return: true if the position specified by (x,y) is inside a terrain hole
		*/
		bool IsHole(float x, float y);
		/**
		* set a new terrain hole at the specified location.
		* Currently, we will allow user to dynamically dig holes in terrain. After calling this function,
		* the user must manually Call UpdateHoles() to inform that the holes in the terrain has been updated.
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		* @see UpdateHoles();
		*/
		void SetHole(float x, float y, bool bIsHold);

		void InitDeviceObjects();
		void DeleteDeviceObjects();
		void InvalidateDeviceObjects();

		/// \brief Returns the height of the vertex closest to the specified point.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		float GetVertexElevation(float x, float y);
		/// \brief Sets the elevation (z-coordinate) in real units of the nearest vertex to the specified point.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		/// \param newElevation The new z-value (elevation) of the vertex to be set.
		void SetVertexElevation(float x, float y, float newElevation, bool recalculate_geometry = true);

		/**
		* @return
		*/
		float GetVertexSpacing() const;
		/**
		* snap the input point position to the nearest vertex gird point.
		* @param x: [in|out]
		* @param y: [in|out]
		* @return: return true if the terrain vertex exists, otherwise return false.
		*/
		bool SnapPointToVertexGrid(float& x, float& y);

		/** set how many terrain tile(including height fields, etc) are cached in memory.
		* The actual cached tile number may be smaller or larger than this value,
		* since we ensure that a tile that is very recently used is never removed from cache.
		* the default value is 18. The smallest value is 18, the largest value is 4000
		*/
		int GetMaxTileCacheSize();

		/** Get how many terrain tile(including height fields, etc) are cached in memory.
		* The actual cached tile number may be smaller or larger than this value,
		* since we ensure that a tile that is very recently used is never removed from cache.
		* the default value is 18. The smallest value is 18, the largest value is 4000
		*/
		void SetMaxTileCacheSize(int nNum);

		/** resize all texture mask width */
		void ResizeTextureMaskWidth(int nWidth);

		/** mark the global config as modified, so that when saving the world, the config file will be saved to disk. */
		void SetGlobalConfigModified(bool bEnable = true);
	private:
		/**
		* copy the terrain data to the specified height field for later processing.
		* @param pHeightField: the height field data.
		* @param x: center x
		* @param y: center y
		* @param fRadius: half the size of the rect.
		* @param bNormalize: true to normalize the terrain height to [0,1] range.
		*/
		bool  Copy_Rect_to_HeightField(ParaEngine::TTerrain* pHeightField, float x, float y, float fRadius, bool bNormalize = true);
		/**
		* copy height field back to the terrain.
		* @param pHeightField: the height field data.
		* @param x: center x
		* @param y: center y
		* @param bRecalculateGeometry: true to recalculate affected terrain geometry
		* @param bRecoverHeightData: whether to transform normalized heightfield data back to original state.
		*/
		bool  Apply_HeightField_to_Rect(ParaEngine::TTerrain* pHeightField, float x, float y, bool bRecalculateGeometry, bool bRecoverHeightData = true);
		/** recalculate terrain geometry in the specified rectangular region.
		* @param x: center x
		* @param y: center y
		* @param fRadius: half the size of the rect.
		*/
		void  RecalculateTerrainGeometry(float x, float y, float fRadius);

		void Tessellate();
		Terrain * CreateTerrainTile(int positionX, int positionY);
		ParaTerrain::DIRECTION GetOppositeDirection(ParaTerrain::DIRECTION direction);
		Terrain *GetTerrainRelative(Terrain * pTerrain, int positionX, int positionY);
		Terrain *GetTerrainRelative(Terrain * pTerrain, ParaTerrain::DIRECTION direction);
		Terrain *LoadTerrain(int index);
		TerrainLatticeLoader *m_pLoader;
		//int m_WidthTerrains, m_HeightTerrains;
		int m_WidthActiveTerrains, m_HeightActiveTerrains;
		float m_TerrainWidth, m_TerrainHeight;
		/// the height of the lowest visible terrain point. This may be used to render the ocean.
		float m_fLowestVisibleHeight;

		/** cached or loaded terrain tiles */
		TerrainTileCacheMap_type m_pCachedTerrains;
		int m_nMaxCacheSize;

		int m_CurrentTerrainIndex[9];

		/** the bounding box from the last tessellation result. The box is always in world coordinate. */
		CShapeBox		m_BoundingBox;
		/** whether the global config file is modified. */
		bool m_bIsGlobalConfigModified;
	public:
		/** Get the bounding box from the last tessellation result. The returned box is always in world coordinate. */
		const CShapeBox& GetBoundingBox() const;
		/**
		* Get the bounding boxes for a given view frustum. The returned boxes are always in world coordinate with render origin applied  and transform by modelView
		* @param boxes [in|out]
		* @param modelView: if not null, all returned boxes will be transformed by it, before testing with the frustum
		* @param frustum: the view frustum to test against.
		* @param nMaxBoxesNum: maximum number of boxes returned. Since it internally uses a breadth first traversal of the terrain quad tree,
		*	larger boxes will return when there are more boxes in the frustum than nMaxBoxesNum
		* @param nSmallestBoxStride what is the stride size of the smallest boxes returned.
		*/
		void GetBoundingBoxes(vector<ParaEngine::CShapeAABB>& boxes, const Matrix4* modelView, const ParaEngine::CShapeFrustum& frustum, int nMaxBoxesNum = 1024, int nSmallestBoxStride = 4);

		/** get the tile ID which is the key in m_pCachedTerrains, from lattice coordinate */
		static int GetTileIDFromXY(int X, int Y);
		/** get lattice coordinates, from the tile ID which is the key in m_pCachedTerrains
		* @return: true if ID is valid. */
		static bool GetXYFromTileID(int nTileID, int* X, int* Y);

		/** set all loaded terrain tile content modified. */
		void SetAllLoadedModified(bool bIsModified = true, DWORD dwModifiedBits = 16);

		/** save modified terrain to disk.
		* @param bHeightMap: true to save height map
		* @param bTextures: true to save textures: alpha maps, etc
		*/
		void SaveTerrain(bool bHeightMap, bool bTextures);
		/** return true, if the terrain is modified and should be saved. */
		bool IsModified();

	public:
		bool m_useGeoMipmap;
		void SwitchLodStyle(bool useGeoMipmap);
		bool IsWalkable(float x, float y, Vector3& oNormal);
		void SetTerrainInfo(float x, float y, float radius, uint32 data, uint32 bitMask, uint32 bitOffset);
		uint32 GetTerrainInfo(float x, float y, uint32 bitMask, uint32 bitOffset);
		void SetEditorMode(bool enable);
		void SetVisibleDataMask(uint32 mask, uint8 bitOffset);
	private:
		ParaIndexBuffer m_pIndices;
		
		GeoMipmapIndicesGroup m_geoMipmapIndices;
		int m_terrainBlockXCount;
		float m_camPosX;
		float m_camPosY;
		int m_Level0ChunkCount;;
		int m_Level1ChunkCount;

		void CreateGeoMipmapIndexBuffer();
		void GetIndexInfo(int level, GeoMipmapChunkType type);
		void CalcGeoMipmapLod(float camX, float camY);
		int CalcBlockLod(int* lodBounds, int blockIndexX, int blockIndexY, int maxLod);

		void SetVertexInfo(float x, float y, uint32 data, uint32 bitMask, uint32 bitOffset);
	};
}
