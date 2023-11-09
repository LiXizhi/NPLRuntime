//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2005.12
// Description:	API for terrain
//-----------------------------------------------------------------------------
#pragma once

#include "ParaScriptingCommon.h"
#include "ParaScriptingGlobal.h"
namespace ParaEngine {
	class CTerrainTile;
}

namespace ParaScripting
{
	using namespace std;
	using namespace ParaEngine;

	/**
	* @ingroup ParaTerrain
	* Contains Terrain functions
	*/
	class PE_CORE_DECL ParaTerrain
	{
	public:
		ParaTerrain(void);
		~ParaTerrain(void);
	public:
		/** get the attribute object associated with the global terrain. */
		static ParaAttributeObject GetAttributeObject();
		/** used for API exportation. */
		static void GetAttributeObject_(ParaAttributeObject& output);

		/** get block terrain manager's attribute object. */
		static ParaAttributeObject GetBlockAttributeObject();


		/**
		* get the total number of textures in the texture set of the terrain tile specified by a world position (x,y)
		* One can get each of the textures by calling GetTexture() function.
		*/
		static int GetTextureCount(float x, float y);

		/** get all texture indices in the cell specified by point(x,y)
		* GetTextureCount() returns all textures used by the entire terrain tile (i.e. 500*500).
		* however, each terrain tile is further subdevided into 8*8=64 terrain cell (each cell takes up about 64*64 meters).
		* Alpha channels of a terrain texture is only created on a per cell basis. A single alpha image (128*128) will be created for each used texture in the cell.
		* @param x in world unit
		* @param y in world unit
		* @param out: this should be an empty table to be filled with textures in the cell, so that {[1]=tex_index, [2]=tex_index, ...}.
		*/
		static void GetTexturesInCell(float x, float y, const object& out);

		/**
		* remove the given texture in the cell specified by the point(x,y).
		* @param x in world unit
		* @param y in world unit
		* @param nIndex: Texture index in the current terrain tile. this should be smaller than the total number of textures in the texture set. see GetTextureCount().
		*/
		static bool RemoveTextureInCell(float x, float y, int nIndex);

		/**
		* get the terrain detailed texture by index. Please note that each terrain tile contains its own texture set.
		* the total number of textures can be returned by GetTextureCount().
		* @param nIndex: Texture index in the current terrain tile.  this should be smaller than the total number of textures in the texture set. see GetTextureCount().
		*  there are two reserved negative index for the common and main textures
		*   -1: common texture, which is repeated several times over each terrain tile surface.
		*   -2: main texture, which is chopped and mapped to the entire terrain surface.
		* @return: The texture entity object is returned. The returned object may be invalid if nIndex is invalid.
		*/
		static ParaAssetObject GetTexture(float x, float y, int nIndex);
		/** only used for API exporting. */
		static void GetTexture__(ParaAssetObject* pOut, float x, float y, int nIndex);
		/**
		* replace the texture at the given index. or append a new texture, or delete a texture if TextureAsset or sTextureAsset is NULL.
		* @param nIndex: if it is smaller than the total number of textures in the texture set. see GetTextureCount().
		*  there are two reserved negative index for the common and main textures
		*   -1: common texture, which is repeated several times over each terrain tile surface.
		*   -2: main texture, which is chopped and mapped to the entire terrain surface.
		* @param sTextureAsset: filename. if nil, texture will be removed.
		*/
		static bool ReplaceTexture(float x, float y, int nIndex, ParaAssetObject& TextureAsset);
		static bool ReplaceTexture_(float x, float y, int nIndex, const object& sTextureAsset);

		/**
		* get the attribute object associated with the terrain tile that contains the given point.
		* @param x in world unit
		* @param y in world unit
		* @return
		*/
		static ParaAttributeObject GetAttributeObjectAt(float x, float y);
		/** used for API exportation. */
		static void GetAttributeObjectAt_(ParaAttributeObject& output, float x, float y);

		/**
		* get the terrain height at the specified position.
		* @param x: position in world unit
		* @param y: position in world unit
		* @return: the terrain height.
		*/
		static float GetElevation(float x, float y);

		/** get value of a given terrain region layer
		* @param sLayerName: the layer name
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		*/
		static DWORD GetRegionValue(const char* sLayerName, float x, float y);

		/** get value of a given terrain region layer
		* @param sLayerName: the layer name
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		* @param argb: it can be a string containing "argb", where the sum of them are returned. for example, specify "r" to get only the red channel value. specify "rgb" for the sum of the grey scale image.
		*  if this is empty string "", the 32bits color value is returned as int.
		*/
		static DWORD GetRegionValue4(const char* sLayerName, float x, float y, const char* argb);

		/** get the on load script which created all objects on this terrain that contains the point (x,y) */
		static string GetTerrainOnloadScript(float x, float y);
		/** solely used for API exporting. Not thread-safe */
		static const char * GetTerrainOnloadScript__(float x, float y);

		/** get the height map file name for the terrain tile that contains the point (x,y) */
		static string GetTerrainElevFile(float x, float y);
		/** solely used for API exporting. Not thread-safe */
		static const char* GetTerrainElevFile__(float x, float y);


		/** save modified terrain to disk.
		* @param bHeightMap: true to save height map
		* @param bTextures: true to save textures: alpha maps, etc
		*/
		static void SaveTerrain(bool bHeightMap, bool bTextures);

		/** reload terrain from disk. User will lose changes since last save operation.
		* one can UNDO changes with this function.
		* @param bHeightMap: true to save height map
		* @param bTextures: true to save textures: alpha maps, etc
		*/
		static void ReloadTerrain(bool bHeightMap, bool bTextures);

		/** update terrain. this function is called, when the user changes the terrain surface.
		* @param bForceUpdate: if true it will update the terrain even if the camera does not moves.
		*/
		static void UpdateTerrain();
		static void UpdateTerrain1(bool bForceUpdate);

		/**
		* snap any 2D point on the height map to a vertex position on the height map.
		* in NPL, one should write as below:
		*	local x,y = ParaTerrain.SnapPointToVertexGrid(10,10);
		* @param x arbitrary 2D point on the height map
		* @param y arbitrary 2D point on the height map
		* @param vertex_x [out] vertex position on the height map
		* @param vertex_y [out] vertex position on the height map
		*/
		static void SnapPointToVertexGrid(float x, float y, float* vertex_x, float* vertex_y);

		/** Returns the number of real units between vertices in the terrain's mesh.*/
		static float GetVertexSpacing();

		//////////////////////////////////////////////////////////////////////////
		// Terrain editor
		//////////////////////////////////////////////////////////////////////////

		/** paint on the specified location of the global terrain.
		* @param detailTexture: the texture entity to paint on the terrain. The texture is usually tilable such as road and grass.
		*	if this is "", it means the base layer or layer 0. Since version 0.9.9, the base layer also has an editable alpha mask .
		* @param brushRadius The width of the brush to paint with in DetailTexture layer pixels. There are typically 256 of these pixels across a TextureCell.
		* @param brushIntensity The intensity with which to paint, ranging from 0.0 to 1.0. This determines how opaque the painted "splat" will be.
		* @param maxIntensity The maximum intensity of the painted splat, ranging from 0.0 to 1.0, accounting for mixing with a splat that may already reside on the surface.
		* @param erase Specifies whether to add the splat to the surface or remove existing splat texels already on the surface (pass false to paint and pass true to erase existing splats)
		* @param x The x location of the point to paint on the Terrain's surface in world units.
		* @param y The y location of the point to paint on the Terrain's surface in world units.
		*/
		static void Paint(const char* detailTexture, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y);
		/** @see Paint().
		* @param nDetailTextureID: multi-texture layer ID, beginning from 0.
		* Such as 0,1,2,3. -1,-2 are reserved for special terrain editor textures. -1 means the base layer.
		*/
		static void Paint_(int nDetailTextureID, float brushRadius, float brushIntensity, float maxIntensity, bool erase, float x, float y);

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
		static void DigCircleFlat(float x, float y, float radius, float fFlatPercentage, float factor);

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
		static void Flatten(float x, float y, float radius, int flatten_op, float elevation, float factor);

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
		static void RadialScale(float x, float y, float scale_factor, float min_dist, float max_dist, float smooth_factor);

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
		static void GaussianHill(float x, float y, float radius, float hscale, float standard_deviation, float smooth_factor);

		/**
		* offset in a spherical region
		* @param x: center of the circle in world unit
		* @param y: center of the circle in world unit
		* @param radius: radius of the inner circle in world unit
		*/
		static void Spherical(float x, float y, float radius, float    offset);

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
		static void Roughen_Smooth(float x, float y, float radius, bool roughen, bool big_grid, float factor);

		/** create a ramp (inclined slope) from (x1,y1) to (x2,y2). The ramp's half width is radius.
		* this is usually used to created a slope path connecting a high land with a low land.
		* @param radius: The ramp's half width
		* @param borderpercentage: borderpercentage*radius is how long the ramp boarder is to linearly interpolate with the original terrain. specify 0 for sharp ramp border.
		* @param factor: in range[0,1]. it is the smoothness to merge with other border heights.Specify 1.0 for a complete merge
		*/
		static void Ramp(float x1, float y1, float x2, float y2, float radius, float borderpercentage, float factor);

		/**
		* Add rectangular height field from file to the current terrain.
		* @param x: center of the rect in world unit
		* @param y: center of the rect in world unit
		* @param filename : the raw elevation or gray scale image file that contains the height field.
		* @param nSmoothPixels:  the number of pixels to smooth from the edge of the height field.
		* if this is 0, the original height field will be loaded unmodified. if it is greater than 0, the loaded height field
		* will be smoothed for nSmoothPixels from the edge, where the edge is always 0.
		*/
		static void AddHeightField(float x, float y, const char* filename, int nSmoothPixels);

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
		static void MergeHeightField(float x, float y, const char* filename, int mergeOperation = 0, float weight1 = 1.0, float weight2 = 1.0, int nSmoothPixels = 7);

		/** Update all holes in the terrain tile that contains the input point.
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		*/
		static void UpdateHoles(float x, float y);
		/**
		* Whether the terrain contains a hole at the specified location.
		* Currently, we allow user to load a low resolution hole maps at the beginning of terrain creation.
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		* @return: true if the position specified by (x,y) is inside a terrain hole
		*/
		static bool IsHole(float x, float y);
		/**
		* set a new terrain hole at the specified location.
		* Currently, we will allow user to dynamically dig holes in terrain. After calling this function,
		* the user must manually Call UpdateHoles() to inform that the holes in the terrain has been updated.
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		* @see UpdateHoles();
		*/
		static void SetHole(float x, float y, bool bIsHold);

		/** return true, if the terrain is modified and should be saved. */
		static bool IsModified();

		/** set the content of the terrain modified. the terrain is specified by a 2D point.
		* the on load script will be rebuilt once saving the terrain.
		* @param x: a position on the terrain where content is changed.
		* @param y: a position on the terrain where content is changed.
		* @param bIsModified: true to set modified.
		*/
		static void SetContentModified(float x, float y, bool bIsModified);

		/** set the content of the terrain modified. the terrain is specified by a 2D point.
		* the on load script will be rebuilt once saving the terrain.
		* @param bIsModified: true to set modified.
		* @param dwModifiedBits: this is any combination of TERRAIN_MODIFIED_BITS. Default value is MODIFIED_ON_LOAD_SCRIPT (16)
		enum TERRAIN_MODIFIED_BITS
		{
		MODIFIED_NONE = 0,
		// detailed terrain texture(with mask) has been modified.
		MODIFIED_TEXTURE = 0x1,
		// height map has modified
		MODIFIED_HEIGHTMAP = 0x1<<1,
		// configuration such as base texture, common file, holes, etc has been modified.
		MODIFIED_CONFIGURATION = 0x1<<2,
		// holes have been changed. this should mean the same thing as MODIFIED_CONFIGURATION
		MODIFIED_HOLES = 0x1<<3,
		// if static objects have been modified, so that we will need to update the on load script
		MODIFIED_ON_LOAD_SCRIPT = 0x1<<4,
		MODIFIED_ALL = 0xffff
		};
		*/
		static void SetContentModified4(float x, float y, bool bIsModified, DWORD dwModifiedBits);

		/** set all loaded terrain tile content modified. This is the refered way to perform a save all operation */
		static void SetAllLoadedModified(bool bIsModified, DWORD dwModifiedBits);


		//////////////////////////////////////////////////////////////////////////
		//
		// global effect and light settings
		//
		//////////////////////////////////////////////////////////////////////////

		/** Enable both global and local lighting. Turn off lighting will greatly improve performance, such as on slower computers*/
		static void EnableLighting(bool bEnable);

		//////////////////////////////////////////////////////////////////////////
		//
		//Edit additional terrin data e.g walkable region,waypoint region
		//
		//////////////////////////////////////////////////////////////////////////
		static void PaintTerrainData(float x, float y, float radius, unsigned int data, unsigned int dataMask, unsigned int bitOffset);

		static uint32 GetTerrainData(float x, float y, uint32 dataMask, unsigned char bitOffset);

		static void SetVisibleDataMask(uint32 mask, unsigned char bitOffset);


		//////////////////////////////////////////////////////////////////////////
		//
		//Block terrain api
		//
		//////////////////////////////////////////////////////////////////////////

		//call this function to start block world
		//params are camera position, if camera is attached to object then it should be attached object position
		static void EnterBlockWorld(float x, float y, float z);

		//call this when leave block world
		//this function will release most resource(buffer,cached data) used by block engine.
		static void LeaveBlockWorld();

		static bool IsInBlockWorld();

		static void SaveBlockWorld(bool saveToTemp);

		static void DeleteAllBlocks();

		//set block template
		//x,y,z should be positive value
		//templateId may contain two template values packed as template0 + (template1<<16)
		//use 0 to delete a block.
		static void SetBlockTemplateId(float x, float y, float z, uint16_t templateId);

		static void SetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z, uint32_t templateId);

		static uint32_t GetBlockTemplateId(float x, float y, float z);

		static uint32_t GetBlockTemplateIdByIdx(uint16_t x, uint16_t y, uint16_t z);

		//user data is used to store per instance data e.g
		//for two template block it means the blend factor of two template
		//for model object it means the facing and scale value
		//one can pack one or more value into a single uint32 value
		//and unpack them according to pre-defined rules according to block type
		static void SetBlockUserData(float x, float y, float z, uint32_t data);

		static void SetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z, uint32_t data);

		static uint32_t GetBlockUserData(float x, float y, float z);

		static uint32_t GetBlockUserDataByIdx(uint16_t x, uint16_t y, uint16_t z);

		/** set block material from a block's face.
		* @param nFaceIndex: if -1, we will remove all materials from the block
		* @param nMaterialID: global unique material id in CBlockMaterialManager
		*/
		static bool SetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId, int32_t nMaterial);

		/** get block's material of a given face
		* @param nFaceIndex: if -1, we will remove all materials from the block
		* @return -1 if not found
		*/
		static int32_t GetBlockMaterial(uint16_t x, uint16_t y, uint16_t z, int16_t nFaceId);
		
		//ray orignial should be positive value
		//ray direction should be normalized value
		//@return:
		//result["x"] = pickResult.X;
		//result["y"] = pickResult.Y;
		//result["z"] = pickResult.Z;
		//result["blockX"] = pickResult.BlockX;
		//result["blockY"] = pickResult.BlockY;
		//result["blockZ"] = pickResult.BlockZ;
		//result["side"] = pickResult.Side;
		//result["length"] = pickResult.Distance;
		//side value : 0 negativeX,1 positiveX,2 NZ,3 PZ,4 NY, 5PY
		//length > fMaxDistance when no collision detected
		static object Pick(float rayX, float rayY, float rayZ, float dirX, float dirY, float dirZ, float fMaxDistance, const object& result, uint32_t filter = 0xffffffff);
		static object MousePick(float fMaxDistance, const object& result, uint32_t filter = 0xffffffff);

		/**
		@param params: it can be attFlag of int type.
		*/
		static bool RegisterBlockTemplate(uint16_t templateId, const object& params);

		static void SetTemplateTexture(uint16_t templateId, const char* fileName);

		static void SetBlockWorldYOffset(float offset);

		/** get block in [startChunk,endChunk]
		* @param startChunkY, endChunkY: if negative, and startChunkY == endChunkY, -startChunkY will be used as verticalSectionFilter(a bitwise filter).
		* @return {count,x{},y{},z{},tempId{}}
		*/
		static object GetBlocksInRegion(int32_t startChunkX, int32_t startChunkY, int32_t startChunkZ, int32_t endChunkX, int32_t endChunkY, int32_t endChunkZ,
			uint32_t matchType, const object& result);

		//get visible chunk region
		//@return : world space chunk id {minX,minY,minZ,maxX,maxY,maxZ}
		static object GetVisibleChunkRegion(const object& result);

		static void SetBlockWorldSunIntensity(float value);

		/** add/remove block to/from highlight block list
		@param x,y,z: world space block id;
		@param isSelect : true to select block, false to de-select block
		@param nGroupID: group id. 0 for highlight 1 for wireframe.
		*/
		static void SelectBlock(uint16_t x, uint16_t y, uint16_t z, bool isSelect);
		static void SelectBlock1(uint16_t x, uint16_t y, uint16_t z, bool isSelect, int nGroupID);

		/**
		@param nGroupID: 0 for animated selection, 1 for wire frame selection.  -1 for all
		*/
		static void DeselectAllBlock1(int nGroupID);

		static void DeselectAllBlock();


		//set damage block id
		//@param x,y,z :world space block id;
		static void SetDamagedBlock(uint16_t x, uint16_t y, uint16_t z);

		//set damage block degree
		//@param damageDegree: [0,1] 0 means undamaged block,1 full damaged block
		static void SetDamageDegree(float damagedDegree);

		/** find a block in the side direction that matched filter from block(x,y,z)
		* this function can be used to check for free space upward or download
		* @param side: 4 is top.  5 is bottom.
		* @param attrFilter:  attribute to match. 0 means air. default to any block
		* @param nCategoryID: -1 means any category_id.  default to -1
		* @return -1 if not found. otherwise distance to the first block that match in the side direction is returned.
		*/
		static int FindFirstBlock(uint16_t x, uint16_t y, uint16_t z, uint16_t nSide = 4, uint32_t max_dist = 32, uint32_t attrFilter = 0xffffffff, int nCategoryID = -1);

		/** get the y pos of the first block of nBlockID, start searching from x, y, z in the side direction
		*/
		static int GetFirstBlock(uint16_t x, uint16_t y, uint16_t z, int nBlockId, uint16_t nSide = 4, uint32_t max_dist = 32);

		/** get the time stamp of for the given chunk column
		0 means not available, >1 means loaded before
		*/
		static int32_t GetChunkColumnTimeStamp(uint32_t chunkX, uint32_t chunkZ);

		/** set chunk column time stamp. usually 0 for non-generated. 1 for generated.
		this is usually called by world generators, so that we will not generate again next time we load the world. */
		static void SetChunkColumnTimeStamp(uint32_t chunkX, uint32_t chunkZ, uint32_t nTimeStamp);

		/**
		* @param verticalSectionFilter: default to 0xffff.  each bit is for one of the 16 vertical sections.
		*/
		static const std::string& GetMapChunkData(uint32_t chunkX, uint32_t chunkZ, bool bIncludeInit, uint32_t verticalSectionFilter);

		static object ApplyMapChunkData(uint32_t chunkX, uint32_t chunkZ, uint32_t verticalSectionFilter, const std::string& chunkData, const object& out);

		/** get block id and userdata at the given block position. */
		static void GetBlockFullData(uint16_t x, uint16_t y, uint16_t z, uint16_t* pId, uint32_t* pUserData);

		/** this function is only valid, when the block region is locked. this function is extremely fast. and it does not do light calculation. */
		static void LoadBlockAsync(uint16_t x, uint16_t y, uint16_t z, uint16_t blockId, uint32_t userData);
	};

}
