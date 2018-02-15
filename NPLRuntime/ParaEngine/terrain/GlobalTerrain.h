#pragma once
#include <vector>
#include <stdint.h>
#include "IAttributeFields.h"
#include "IObjectScriptingInterface.h"

namespace ParaEngine
{
	struct TextureEntity;
	class CShapeBox;
	class TTerrain;
	class CShapeAABB;
	class CShapeFrustum;
}
namespace ParaTerrain
{
	using namespace ParaEngine;
	class Terrain;
	class TerrainLattice;
	class CDynamicTerrainLoader;
	class Settings;

	/**
	* this is the interface class for managing global terrain used in ParaEngine.
	* A global terrain can either be single or latticed. The latter will load terrain as tiles 
	* automatically when they are potentially visible.
	* a global terrain's world coordinate (x,y) is defined as below:
	* x>=0, y>=0. A single terrain usually has less than 512*512 vertices and is suitable for terrain size 
	* smaller than 2000*2000meters. A latticed terrain has no theoretical upper limit of terrain size. 
	* each terrain tile may be 500*500meters(or 128*128 elevation vertices) and there may be any number of tiles. 
	* For example, if 64*64 tiles are available on hard disk, then the total terrain size will be 32000*32000 meters
	* Internally, terrain tile is dynamically loaded into a sorted map. So there is a upper limit as for the total
	* number of tiles loaded concurrently. But in most cases, only the 9 tiles around the camera position need to be
	* loaded for rendering. One can specify however tiles should be cached in memory. By default it is set to 16.
	*/
	class CGlobalTerrain : public IAttributeFields, IObjectScriptingInterface
	{
	public:
		CGlobalTerrain(void);
		virtual ~CGlobalTerrain(void);
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CGlobalTerrain;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CGlobalTerrain"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CGlobalTerrain, IsModified_s, bool*)	{*p1 = cls->IsModified(); return S_OK;}

		ATTRIBUTE_METHOD1(CGlobalTerrain, TerrainRenderingEnabled_s, bool*)	{*p1 = cls->TerrainRenderingEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, EnableTerrainRendering_s, bool)	{cls->EnableTerrainRendering(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CGlobalTerrain, IsTerrainEngineEnabled_s, bool*)	{ *p1 = cls->IsTerrainEngineEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CGlobalTerrain, EnableTerrainEngine_s, bool)	{ cls->EnableTerrainEngine(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGlobalTerrain, IsVerbose_s, bool*)	{*p1 = cls->IsVerbose(); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetVerbose_s, bool)	{cls->SetVerbose(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CGlobalTerrain, IsEditor_s, bool*)	{*p1 = cls->IsEditor(); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetEditor_s, bool)	{cls->SetEditor(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CGlobalTerrain, GetTessellateMethod_s, int*)	{*p1 = cls->GetTessellateMethod(); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetTessellateMethod_s, int)	{cls->SetTessellateMethod(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CGlobalTerrain, UseNormals_s, bool*)	{*p1 = cls->UseNormals(); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetUseNormals_s, bool)	{cls->SetUseNormals(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CGlobalTerrain, GetTextureMaskWidth_s, int*)	{*p1 = cls->GetTextureMaskWidth(); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetTextureMaskWidth_s, int)	{cls->SetTextureMaskWidth(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CGlobalTerrain, GetDefaultHeight_s, float*)	{ *p1 = cls->GetDefaultHeight(); return S_OK; }
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetDefaultHeight_s, float)	{ cls->SetDefaultHeight(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGlobalTerrain, IsGeoMipmapTerrain_s, bool*)	{*p1 =cls->IsGeoMipmapTerrain(); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetGeoMipmapTerrain_s, bool)	{cls->SetGeoMipmapTerrain(p1); return S_OK;}
		 
		ATTRIBUTE_METHOD1(CGlobalTerrain, IsEditorMode_s, bool*)	{*p1 =cls->IsEditorMode(); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetEditorMode_s, bool)	{cls->SetEditorMode(p1); return S_OK;}
		       
		ATTRIBUTE_METHOD1(CGlobalTerrain, IsSlopeCollision_s, bool*)	{*p1 =cls->IsSlopeCollision(); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetSlopeCollision_s, bool)	{cls->SetSlopeCollision(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CGlobalTerrain, SetBlockSelectionTexture_s, const char*)	{cls->SetBlockSelectionTexture(p1); return S_OK;}
		ATTRIBUTE_METHOD1(CGlobalTerrain, SetBlockDamageTexture_s, const char*)	{cls->SetBlockDamageTexture(p1); return S_OK;}
	public:
		/**
		* get the attribute object associated with the terrain tile that contains the given point.
		* @param x in world unit
		* @param y in world unit
		* @return
		*/
		IAttributeFields* GetAttributeObjectAt(float x, float y);

		/**Specifies whether or not Demeter should output diagnostic information at runtime.*/
		bool IsVerbose();
		void SetVerbose(bool bEnable);

		/// This makes memory usage much higher but makes dynamically editing the terrain easier 
		/// for applications such as terrain editors.
		void SetEditor(bool isEditor);
		/// brief Indicates whether or not SetEditor(true) has been called.
		bool IsEditor();

		/// This setting determines which algorithm Demeter uses to simplify terrain blocks. Some
		/// settings are faster but less general and vice versa. The default setting is TM_NEW.
		void SetTessellateMethod(int method);
		/// Gets the current tessellation heuristic method.
		int GetTessellateMethod();

		/// Turning this option on will make Demeter use more memory and will slow rendering slightly on most hardware.
		/// You will definitely need to turn this setting on if you want to use lighting on your Terrain objects instead
		/// of "baking" lighting into the terrain textures.
		void SetUseNormals(bool bUseNormals);
		/// Indicates whether or not normals are on as set by calling SetUseNormals().
		bool UseNormals();
		 
		
		/** used to render damaged texture. It is layer on top of block texture */
		void SetBlockDamageTexture(const char* textureName);
		/** used to render selection texture. It is layer on top of block texture */
		void SetBlockSelectionTexture(const char* textureName);

	public:
		/// clean up the global terrain
		void Cleanup();
		/// this function currently does nothing
		void InitDeviceObjects();
		void RestoreDeviceObjects();
		/// must call this function when device is lost.
		void DeleteDeviceObjects();
		/// must call this function when device is invalid.
		void InvalidateDeviceObjects();

		/** get the settings object to set and get terrain global parameters. Such as normal, etc.*/
		Settings* GetSettings();

		/** Get the bounding box from the last tessellation result. The returned box is always in world coordinate. 
		* please note: the box is bounding box of all smallest boxes in the terrain quads which are in the detailed 
		* ranged. The detailed range is usually set 30 meters from current eye position in the ground (xz) plane. 
		* This bounding box is usually used as the terrain's shadow receiver's bounding box.
		*/
		void GetBoundingBox(ParaEngine::CShapeBox * BBox);

		/**
		* Get the bounding boxes for a given view frustum. The returned boxes are always in world coordinate with render origin applied and transform by modelView
		* @param boxes [in|out] 
		* @param modelView: if not null, all returned boxes will be transformed by it, before testing with the frustum 
		* @param frustum: the view frustum to test against.
		* @param nMaxBoxesNum: maximum number of boxes returned. Since it internally uses a breadth first traversal of the terrain quad tree,
		*	larger boxes will return when there are more boxes in the frustum than nMaxBoxesNum
		* @param nSmallestBoxStride what is the stride size of the smallest boxes returned. 
		*/
		void GetBoundingBoxes(vector<ParaEngine::CShapeAABB>& boxes, const Matrix4* modelView, const ParaEngine::CShapeFrustum& frustum, int nMaxBoxesNum=1024, int nSmallestBoxStride=4);

		/**
		* Set the camera position, so that terrain tiles around it are automatically loaded. 
		* It is only useful for latticed terrain.
		*/
		void SetCameraPosition(float x, float y);

		/** should be called when the camera moves to re-tessellate the terrain and rebuild the terrain buffer. 
		* However, if RenderAuto() is called. There is no need to call this function.
		* @param bForceUpdate if true, the terrain will be re-tessellate even if the camera is not moved. 
		*/
		void Update(bool bForceUpdate=false);
		/** Render last scene assuming that the camera has not moved.*/
		void Render();
		/** Automatically rebuild the terrain buffer and the render the terrain. Internally it calls Update() and Render().*/
		//void RenderAuto();

		/** get elevation at the world position. */
		float GetElevation(float x, float y);

		/** get value of a given terrain region layer 
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		*/
		DWORD GetRegionValue(const string& sLayerName, float x, float y);

		/**
		* Set elevation at the nearest vertex to the specified point.
		* @param x world x coordinate
		* @param y world y coordinate
		* @param fHeight absolute height
		*/
		void SetVertexElevation(float x, float y,float fHeight);
		/**
		* Returns the height of the vertex closest to the specified point.
		* @param x The x location of the point on the Terrain's surface in world units.
		* @param y The y location of the point on the Terrain's surface in world units.
		* @return 
		*/
		float GetVertexElevation(float x, float y);
		
		/** Get the height of the lowest visible terrain point. This may be used to render the ocean*/
		float GetLowestVisibleHeight();

		/**
		* This method makes use of the terrain's quad tree to optimize the ray-tracing.
		* If it is latticed terrain, only the 9 tiles around the camera position are used for ray-tracing.
		* \return The distance from the starting point to the intersection. A negative value is returned if the ray misses the Terrain.
		* \param startX The starting point of the ray.
		* \param startY The starting point of the ray.
		* \param startZ The starting point of the ray.
		* \param dirX The direction of the ray - this vector should be normalized.
		* \param dirY The direction of the ray - this vector should be normalized.
		* \param dirZ The direction of the ray - this vector should be normalized.
		* \param intersectX Filled with the intersection point of this ray with the Terrain surface.
		* \param intersectY Filled with the intersection point of this ray with the Terrain surface.
		* \param intersectZ Filled with the intersection point of this ray with the Terrain surface.
		* @param fMaxDistance: if negative it means the ray length is infinity. otherwise it means ray length. 
		*/
		float IntersectRay(float startX, float startY, float startZ, float dirX, float dirY, float dirZ, float &intersectX, float &intersectY, float &intersectZ, float fMaxDistance = 999999999.0f);

		/// \brief Returns the surface normal of the terrain at the specified point.
		/// \param x The x location of the point on the Terrain's surface in world units.
		/// \param y The y location of the point on the Terrain's surface in world units.
		/// \param normalX Gets filled with the surface normal x component
		/// \param normalY Gets filled with the surface normal y component
		/// \param normalZ Gets filled with the surface normal z component
		void GetNormal(float x, float y, float &normalX, float &normalY, float &normalZ);

		/**
		* Create and set the single tile based global terrain from height map and texture files.
		* this function can be called multiple times, in which cases previously loaded single terrain will be discarded.
		* @param sHeightmapfile: the height map used to create the terrain. It must be sized to 2*2*...*2 pixels for both height and width. 
		*		so usually it is 1024*1024, 2048*2048, etc.
		* @param fTerrainSize: the actual terrain size in the game
		* @param bSwapvertical: if one want to swap the height map data vertically. 
		* @param sMainTextureFile: texture to be mapped to entire terrain
		* @param sCommonTextureFile: texture to be tiles to the entire terrain to add some details.
		* @param nMaxBlockSize: When doing LOD with the height map, the max block size must be smaller than this one. 
		*		This will be (nMaxBlockSize*nMaxBlockSize) sized region on the height map.
		* @param fDetailThreshold: we will use a LOD block to approximate the terrain at its location, if the 
		*		block is smaller than fDetailThreshold pixels when projected to the 2D screen.
		*/
		void CreateSingleTerrain(const char* sHeightmapfile, 
			float fTerrainSize, float fElevscale, int bSwapvertical, 
			const char*  sMainTextureFile, const char* sCommonTextureFile, 
			int nMaxBlockSize, float fDetailThreshold);
		/**
		* create a terrain from a configuration file. There are two kind of configuration files. 
		* the first line of the configuration file decides which configuration to use. 
		* - Configuration file for a single terrain. The file specification can be found at Terrain.h
		* - Configuration file for a lattice based terrain. The file specification can be found at Terrain.h
		* The first line always contains "type = lattice"
		*/
		void CreateTerrain(const char * sConfigFileName);

		/** paint on the specified location of the global terrain.
		* @param detailTexture: the texture entity to paint on the terrain. The texture is usually tilable such as road and grass.
		*	if this is NULL, it means the base layer or layer 0. Since version 0.9.9, the base layer also has an editable alpha mask .
		* @param brushRadius The width of the brush to paint with in DetailTexture layer pixels. There are typically 256 of these pixels across a TextureCell.
		* @param brushIntensity The intensity with which to paint, ranging from 0.0 to 1.0. This determines how opaque the painted "splat" will be.
		* @param maxIntensity The maximum intensity of the painted splat, ranging from 0.0 to 1.0, accounting for mixing with a splat that may already reside on the surface.
		* @param erase Specifies whether to add the splat to the surface or remove existing splat texels already on the surface (pass false to paint and pass true to erase existing splats)
		* @param x The x location of the point to paint on the Terrain's surface in world units.
		* @param y The y location of the point to paint on the Terrain's surface in world units.
		*/
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
		void Flatten (float x,float y,float radius, int flatten_op, float elevation,float factor);

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
		void DigCircleFlat(float x, float y, float radius, float fFlatPercentage=0.7f, float factor=0.5f);

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
		void GaussianHill (float x,float y,float radius,float hscale,float standard_deviation=0.1f,float smooth_factor=0.5f);

		/**
		* offset in a spherical region
		* @param x: center of the circle in world unit
		* @param y: center of the circle in world unit
		* @param radius: radius of the inner circle in world unit
		*/
		void Spherical ( float x,float y,float radius, float    offset);

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
		void RadialScale (float x, float y,  float scale_factor, float min_dist,float max_dist, float smooth_factor=0.5f, int frequency=1);

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
		void Roughen_Smooth (float x, float y, float radius,bool roughen=false, bool big_grid=false,float factor=0.5f);

		/** create a ramp (inclined slope) from (x1,y1) to (x2,y2). The ramp's half width is radius. 
		* this is usually used to created a slope path connecting a high land with a low land. 
		* @param radius: The ramp's half width
		* @param borderpercentage: borderpercentage*radius is how long the ramp boarder is to linearly interpolate with the original terrain. specify 0 for sharp ramp border.
		* @param factor: in range[0,1]. it is the smoothness to merge with other border heights.Specify 1.0 for a complete merge
		*/
		void Ramp(float x1, float y1, float x2, float y2, float radius, float borderpercentage=0.5f, float factor=1.0f);

		/**
		* Add rectangular height field from file to the current terrain.  
		* @param x: center of the rect in world unit
		* @param y: center of the rect in world unit
		* @param filename : the raw elevation or gray scale image file that contains the height field. 
		* @param nSmoothPixels:  the number of pixels to smooth from the edge of the height field. 
		* if this is 0, the original height field will be loaded unmodified. if it is greater than 0, the loaded height field 
		* will be smoothed for nSmoothPixels from the edge, where the edge is always 0. 
		*/
		void AddHeightField(float x, float y, const char* filename, int nSmoothPixels=7);

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
		void MergeHeightField(float x, float y, const char* filename, int mergeOperation=0,  float weight1=1.0, float weight2=1.0, int nSmoothPixels=7);

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

		/** get the on load script which created all objects on this terrain tile which contains the point x,y in global coordinate system*/
		string GetOnloadScript(float x, float y);

		/** set the content of the terrain modified. the terrain is specified by a 2D point.
		* the on load script will be rebuilt once saving the terrain.
		* @param x, y: save all loaded content modified. 
		* @param bIsModified: true to set modified.
		* @param dwModifiedBits: this is any combination of TERRAIN_MODIFIED_BITS. Default value is MODIFIED_ON_LOAD_SCRIPT
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
		void SetContentModified(float x, float y, bool bIsModified = true, DWORD dwModifiedBits=16);

		/** set all loaded terrain tile content modified. */
		void SetAllLoadedModified(bool bIsModified = true, DWORD dwModifiedBits=16);

		/** get the height map file name for the terrain tile that contains the point (x,y) */
		string GetTerrainElevFile(float x, float y);
		/** save modified terrain to disk. 
		* @param bHeightMap: true to save height map
		* @param bTextures: true to save textures: alpha maps, etc
		*/
		void SaveTerrain(bool bHeightMap, bool bTextures);

		/** return true, if the terrain is modified and should be saved. */
		bool IsModified();

		/** Enable both global and local lighting. Turn off lighting will greatly improve performance, such as on slower computers*/
		void EnableLighting(bool bEnable);

		/** enable/disable global real world terrain engine. If disabled CreateTerrain does not take effect. */
		bool IsTerrainEngineEnabled() const;
		void EnableTerrainEngine(bool val);

		/**
		* true if terrain is rendered.
		* @return 
		*/
		bool TerrainRenderingEnabled();

		/**
		* specify whether to whether terrain. 
		* @param bEnable 
		*/
		void EnableTerrainRendering(bool bEnable);

		/**
		* get the height map of a given square region in the global terrain. 
		* The resolution of the obtained height map is determined by the pOut parameter.
		* @param fCenterX global world position of the center of the square region.
		* @param fCenterY global world position of the center of the square region.
		* @param fSize width or height of the square region. 
		* @param pOut [in|out] the height map is to be filled with height map of the terrain in the specified region. 
		* @return true if succeed.
		*/
		bool GetHeightmapOf(float fCenterX, float fCenterY, float fSize, TTerrain* pOut);

		/**
		* set the height map of a given square region in the global terrain.
		* @note: the center location will be aligned to the closest terrain vertex before the height map is applied.
		* hence it is good practice to call SnapToTerrainVertex() and only use the exact vertex position as the center.
		* @param fCenterX global world position of the center of the square region.
		* @param fCenterY global world position of the center of the square region.
		* @param fSize width or height of the square region. 
		* @param pIn [in] the height map is to be filled with height map of the terrain in the specified region. 
		* @return true if succeed.
		*/
		bool SetHeightmapOf(float fCenterX, float fCenterY, float fSize, TTerrain* pIn);

		/**
		* snap any 2D point on the height map to a vertex position on the height map.
		* @param x arbitrary 2D point on the height map
		* @param y arbitrary 2D point on the height map
		* @param vertex_x [out] vertex position on the height map
		* @param vertex_y [out] vertex position on the height map
		*/
		void SnapPointToVertexGrid(float x, float y, float* vertex_x, float* vertex_y);

		/** Returns the number of real units between vertices in the terrain's mesh.*/
		float GetVertexSpacing() const;

		/**
		* get the total number of textures in the texture set of the terrain tile specified by a world position (x,y)
		* One can get each of the textures by calling GetTexture() function. 
		*/
		int GetTextureCount(float x, float y);

		/** get all texture indices in the cell specified by point(x,y) 
		* GetTextureCount() returns all textures used by the entire terrain tile (i.e. 500*500).
		* however, each terrain tile is further subdevided into 8*8=64 terrain cell (each cell takes up about 64*64 meters). 
		* Alpha channels of a terrain texture is only created on a per cell basis. A single alpha image (128*128) will be created for each used texture in the cell. 
		* @param x in world unit
		* @param y in world unit
		* @param indices: this will be filled with texture indices which are used in the cell. 
		*/
		void GetTexturesInCell(float x, float y, std::vector<int>& indices);

		/**
		* remove the given texture in the cell specified by the point(x,y).
		* @param x in world unit
		* @param y in world unit
		* @param nIndex: Texture index in the current terrain tile. this should be smaller than the total number of textures in the texture set. see GetTextureCount().
		*/
		bool RemoveTextureInCell(float x, float y, int nIndex);

		/**
		* get the terrain detailed texture by index. Please note that each terrain tile contains its own texture set. 
		* the total number of textures can be returned by GetTextureCount().
		* @param nIndex: this should be smaller than the total number of textures in the texture set. see GetTextureCount().
		*  there are two reserved negative index for the common and main textures
		*   -1: common texture, which is repeated several times over each terrain tile surface.
		*   -2: main texture, which is chopped and mapped to the entire terrain surface.
		* @return: The texture entity object is returned. The returned object may be invalid if nIndex is invalid. 
		*/
		TextureEntity* GetTexture(float x, float y, int nIndex);
		/**
		* replace the texture at the given index.
		* @param nIndex: this should be smaller than the total number of textures in the texture set. see GetTextureCount().
		*  there are two reserved negative index for the common and main textures
		*   -1: common texture, which is repeated several times over each terrain tile surface.
		*   -2: main texture, which is chopped and mapped to the entire terrain surface.
		*/
		bool ReplaceTexture(float x, float y, int nIndex, TextureEntity* TextureAsset);

		/** get the terrain at point no matter it is single or latticed. Return NULL if there is no terrain object at the position. */
		Terrain * GetTerrainAtPoint(float x,float y);

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

		/** set whether we will encode terrain related files. default to true.
		* By enabling path encoding, terrain related files like "worlddir/worldfile.txt" will be saved as "%WORLD%/worldfile.txt", thus 
		* even the entire world directory changes, the world files can still be found using path variables. Path encoding needs to be disabled when you are creating a template world.
		*/
		void SetEnablePathEncoding(bool bEnable){ m_bEnablePathEncoding = bEnable;}

		/** get whether we will encode terrain related files. default to true.
		* By enabling path encoding, terrain related files like "worlddir/worldfile.txt" will be saved as "%WORLD%/worldfile.txt", thus 
		* even the entire world directory changes, the world files can still be found using path variables. Path encoding needs to be disabled when you are creating a template world.
		*/
		bool GetEnablePathEncoding(){ return  m_bEnablePathEncoding;}

		/** get the default texture mask file resolution. Although the low level implementation allows us to special a different resolution per cell and per detail texture. 
		* this is the only exposed API to change the default mask resolution. Default to 128. the larger, the more details, yet also increases file size of *.mask file. 
		* 256, 126, 64 are all reasonable values. 
		*/
		void SetTextureMaskWidth(int nWidth);
		int GetTextureMaskWidth();

		void SwitheTerrainLodStyle(bool useGeoMipmap);
		
		/** the default terrain height, normally it is 0*/
		float GetDefaultHeight() const;
		void SetDefaultHeight(float val);

	private:
		/** should be called when the camera moves to re-tessellate the terrain and rebuild the terrain buffer. 
		* However, if RenderAuto() is called. There is no need to call this function.*/
		void ModelViewMatrixChanged();
	private:
		/** the default terrain height, normally it is 0*/
		float			m_fDefaultHeight;
		Terrain*		m_pTerrainSingle;
		TerrainLattice*	m_pTerrainLattice;
		CDynamicTerrainLoader* m_pTerrainLoader;
		enum Terrain_type{
			SINGLE_TERRAIN,
			LATTICED_TERRAIN
		} m_nTerrainType;
		
		/** if false, the terrain will not be drawn. both tessellation and rendering */
		bool	m_bRenderTerrain;
		/** enable/disable global real world terrain engine. If disabled CreateTerrain does not take effect. */
		bool	m_bEnableTerrainEngine;
		
		/** whether we will encode terrain related files. default to true.
		* By enabling path encoding, terrain related files like "worlddir/worldfile.txt" will be saved as "%WORLD%/worldfile.txt", thus 
		* even the entire world directory changes, the world files can still be found using path variables. Path encoding needs to be disabled when you are creating a template world.
		*/
		bool	m_bEnablePathEncoding;

		//GeoMipmapCode 
		bool m_useGeoMipmap;
		bool m_enableSlopeCollision;
		bool m_isEditorMode;
		bool m_isSlopeMeshVisible;
		bool m_isWaypointRegionVisible;
		
		Vector3		m_matTerrainEye;
		Vector3		m_vTerrainRenderOffset;
		
	public:
		static const float g_fTerrainInvisibleHeight;

		const Vector3& GetMatTerrainEye() const;
		const Vector3& GetTerrainRenderOffset() const;

		//this function must be called before CreateTerrain,we don't support change
		//terrain rendering style at runtime -.-    --clayman 2012.2.21

		void SetGeoMipmapTerrain(bool enable);

		bool IsGeoMipmapTerrain()
		{
			return m_useGeoMipmap;
		}

		void SetEditorMode(bool enable);

		bool IsEditorMode()
		{
			return m_isEditorMode;
		}

		void SetSlopeCollision(bool enable)
		{
			m_enableSlopeCollision = enable;
		}

		bool IsSlopeCollision()
		{
			return m_enableSlopeCollision;
		}

		void SetSlopeMeshVisible(bool);

		bool IsSlopeMeshVisible()
		{
			return m_isSlopeMeshVisible;
		}

		void SetWaypointRegionVisible(bool enable);

		bool IsWaypointRegionVisible()
		{
			return m_isWaypointRegionVisible;
		}
			
		void PaintTerrainData(float x,float y,float radius,unsigned int data,unsigned int dataMask,unsigned int bitOffset);

		uint32 GetTerrainData(float x,float y,uint32 dataMask,uint8 bitOffset);

		void SetVisibleDataMask(uint32 mask,uint8 bitOffset);

		bool IsWalkable(float x,float y, Vector3& oNormal) const;

	};

}
