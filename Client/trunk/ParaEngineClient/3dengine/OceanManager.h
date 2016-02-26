#pragma once
#include "IAttributeFields.h"
#include "terrain/TTerrain.h"
#include <list>
namespace ParaEngine
{
	using namespace std;

	class TTerrain;
	struct TextureEntity;

	/**
	* a ripple
	*/
	struct RippleItem
	{
		// max radius in meters
		float fMaxRadius;
		// propagation speed
		float fSpeed;
		// position in world coordinate system. 
		Vector3 vPos;

		// current radius, which is automatically updated during frame move 
		float fCurrentRadius;
	};

	/** a list of active RippleItem */
	class RippleList
	{
	public:
		RippleList();
		~RippleList();

		
		/**
		* frame move all active particles, removing inactive ones.
		* @param fDeltaTime 
		*/
		void FrameMove(float fDeltaTime);
	
		/**
		* clear all ripples, called when ocean is disabled. 
		*/
		void ClearAll();

	public:
		/* all active ripples */
		list<RippleItem> m_ripples;
	};

	/**
	* FFT based ocean rendering class. FFT is used to generate the ocean wave mesh. 
	* Bump mapping is used to add high frequency wavelets.
	*/
	class COceanManager : public IAttributeFields
	{
	public:
		COceanManager(void);
		~COceanManager(void);
		static COceanManager* GetInstance();

		enum eProcessingStages
		{
			k_animateHeight = 0,
			k_animateNormal,
			k_heightFFTv,
			k_heightFFTh,
			k_normalFFTv,
			k_normalFFTh,
			k_uploadBuffer,
			k_rotateBuffer,
			k_total_process_stages
		};
		enum eWaterMeshes
		{
			k_total_water_meshes = 3,
			k_total_terrain_meshes = 1,
		};

		/** the base vertex for ocean tile.*/
		struct sBaseVertex
		{
			Vector2	 vPos;
 			//DWORD colour;
			//FLOAT u,v;
			//static const DWORD FVF;
		};

		/** vertex data for animation */
		struct sAnimVertex
		{
			/** the height of the vertex */
			float zPos;
			/** the normal of the vertex */
			Vector2 vNormal;
		};
		/** vertex data for terrain terrain height field.*/
		struct sTerrainVertex
		{
			float fHeight;
		};

		/** 
		* typedef floating-point complex numbers
		* to hold pairs of real 
		* and imaginary numbers
		*/
		struct sComplex
		{
			float real;
			float imag;

			sComplex& operator = (const sComplex& src)
			{
				real = src.real;
				imag = src.imag;
				return *this;
			}
		};

		
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_COceanManager;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "COceanManager"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(COceanManager, OceanEnabled_s, bool*)	{*p1 = cls->OceanEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, EnableOcean_s, bool)	{cls->EnableOcean(p1); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, GetWaterLevel_s, float*)	{*p1 = cls->GetWaterLevel(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, SetWaterLevel_s, float)	{cls->SetWaterLevel(p1); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, IsUnderWater_s, bool*)	{*p1 = cls->IsUnderWater(); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, GetOceanColor_s, Vector3*)		{*p1 = cls->GetOceanColor().ToVector3(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, SetGetOceanColor_s, Vector3)		{LinearColor c(p1.x, p1.y,p1.z,1); cls->SetGetOceanColor(c); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, GetWindSpeed_s, float*)	{*p1 = cls->GetWindSpeed(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, SetWindSpeed_s, float)	{cls->SetWindSpeed(p1); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, GetWindDirection_s, float*)	{*p1 = cls->GetWindDirection(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, SetWindDirection_s, float)	{cls->SetWindDirection(p1); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, GetRenderTechnique_s, int*)	{*p1 = cls->GetRenderTechnique(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, SetRenderTechnique_s, int)	{cls->SetRenderTechnique(p1); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, IsTerrainReflectionEnabled_s, bool*)	{*p1 = cls->IsTerrainReflectionEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, EnableTerrainReflection_s, bool)	{cls->EnableTerrainReflection(p1); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, IsMeshReflectionEnabled_s, bool*)	{*p1 = cls->IsMeshReflectionEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, EnableMeshReflection_s, bool)	{cls->EnableMeshReflection(p1); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, IsPlayerReflectionEnabled_s, bool*)	{*p1 = cls->IsPlayerReflectionEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, EnablePlayerReflection_s, bool)	{cls->EnablePlayerReflection(p1); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, IsCharacterReflectionEnabled_s, bool*)	{*p1 = cls->IsCharacterReflectionEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, EnableCharacterReflection_s, bool)	{cls->EnableCharacterReflection(p1); return S_OK;}

		ATTRIBUTE_METHOD1(COceanManager, IsAnimateFFT_s, bool*)	{*p1 = cls->IsAnimateFFT(); return S_OK;}
		ATTRIBUTE_METHOD1(COceanManager, SetAnimateFFT_s, bool)	{cls->SetAnimateFFT(p1); return S_OK;}

		ATTRIBUTE_METHOD(COceanManager, CleanupTerrainCache_s)	{cls->CleanupTerrainCache(); return S_OK;}

		/**
		* a color to be multiplied to the sky, giving it a special color hue
		* @return 
		*/
		const LinearColor& GetOceanColor(){return m_colorOcean;};
		/**
		* a color to be multiplied to the sky, giving it a special color hue
		* @param color 
		*/
		void SetGetOceanColor(const LinearColor& color){m_colorOcean= color;};


		/** whether ocean should be drawn and simulated. */
		bool OceanEnabled();
		/** set whether ocean should be drawn and simulated. */
		void EnableOcean(bool Enable);
		
		/** @return: get the water level */
		float GetWaterLevel();
		/** set the water level 
		* @param fLevel: water level */
		void SetWaterLevel(float fLevel);

		enum ReflectionItem
		{
			R_SKY_BOX = 0,
			R_GLOBAL_TERRAIN,
			R_MESH_FRONT_TO_BACK,
			R_MESH_BACK_TO_FRONT,
			R_CHARACTERS,
			R_CURRENT_PLAYER,
			R_MESH_TRANSPARENT,
			R_LAST_ITEM,
		};
		/**
		* Set which group of items will be drawn in the reflection.
		* @param item see ReflectionItem
		* @param bEnabled 
		*/
		void EnableReflectionItem(ReflectionItem item, bool bEnabled);
		bool IsReflectionItemEnabled(ReflectionItem item);

		void EnableTerrainReflection(bool bEnabled){EnableReflectionItem(R_GLOBAL_TERRAIN, bEnabled);};
		bool IsTerrainReflectionEnabled(){return IsReflectionItemEnabled(R_GLOBAL_TERRAIN);};

		void EnableMeshReflection(bool bEnabled){EnableReflectionItem(R_MESH_FRONT_TO_BACK, bEnabled); EnableReflectionItem(R_MESH_BACK_TO_FRONT, bEnabled);};
		bool IsMeshReflectionEnabled(){return IsReflectionItemEnabled(R_MESH_FRONT_TO_BACK);};

		void EnablePlayerReflection(bool bEnabled){EnableReflectionItem(R_CURRENT_PLAYER, bEnabled);};
		bool IsPlayerReflectionEnabled(){return IsReflectionItemEnabled(R_CURRENT_PLAYER);};

		void EnableCharacterReflection(bool bEnabled){EnableReflectionItem(R_CHARACTERS, bEnabled);};
		bool IsCharacterReflectionEnabled(){return IsReflectionItemEnabled(R_CHARACTERS);};


		/** whether to redraw reflection even if the camera does not move.
		* this function is automatically true, if animated characters are enabled for reflection drawing. 
		*/
		bool IsRedrawStillReflection();
		
		/** @return: true if it is under water effect that is being used most recently */
		bool IsUnderWater();

		/** get the wind speed. */
		float GetWindSpeed();
		/** set the wind speed. */
		void SetWindSpeed(float fSpeed);
		/** get the wind Direction. */
		float GetWindDirection();
		/** set the wind Direction. */
		void SetWindDirection(float fWindDirection);

		/**
		* Set the global water drawing attribute.
		* the global water level is just a water surface at a given height near the current camera location
		* @param bEnable: whether to draw global water 
		* @param fWaterLevel: water level in meters. Default value is 0.0f;
		*/
		void SetGlobalWater(bool bEnable, float fWaterLevel = 0.0f);

		/** @return: get the highest water level that is visible. The water level is in world coordinates. */
		float GetMaxWaterLevel();

		
		/**
		* add a ripple at the given position using the default wave parameters of the ocean. 
		* @param vPos: the position of the center of the ripple in world coordinate system.
		*/
		void AddRipple(const Vector3& vPos);
	private:
		/** shall we animate FFT, animating FFT will consume quite a lot of CPU per frame. */
		bool m_bAnimateFFT;
		/** all active ripples on the ocean surface*/
		RippleList m_rippleList;
		int m_nMaxRippleNumber;
		
		/** global water level 
		* the global water level is just a water surface at a given height near the current camera location*/
		float	m_fGlobalWaterLevel;
		/** whether to draw global water */
		bool	m_bDrawGlobalWater;
		/** ocean color: usually something slightly blue. */
		LinearColor m_colorOcean;

		// whether to render ocean reflection texture according to occlusion result. 
		bool m_bRenderOceanReflection;

		int m_grid_size;
		int m_half_grid_size;
		int m_log_grid_size;
		/** Set the FFT resolution. and create all intermediary data structures based on the grid size. 
		* a mesh of (nSize+1)^2 vertices will be created.
		* @param nSize: it must be power of 2. The common value is 64 which is OK for current hardware
		*/
		void SetFFTGrid(int nSize);
		void DeleteGrid();

		float m_fWindX;    
		float m_fWindY;
		float m_fWindSpeed;
		float m_fWaveHeight;
		float m_fDirectionalDependence;
		float m_fSuppressSmallWavesFactor;
		
		float m_fLargestPossibleWave;   
		float m_fSuppressSmallWaves;

		/** tile size in meters */
		float m_ocean_tile_size; 
		/** tile height in meters */
		float m_ocean_tile_height;

		/************************************************************************/
		/* cached terrain tiles for shorelines                                  */
		/************************************************************************/
		bool m_bDrawShoreline;
		/** cached or loaded terrain height maps */
		map <int, TerrainTileCacheItem> m_pCachedTerrains;
		int m_nMaxCacheSize;

		/** get the tile ID which is the key in m_pCachedTerrains, from lattice coordinate */
		static int GetTileIDFromXY(int X, int Y);
		/** get lattice coordinates, from the tile ID which is the key in m_pCachedTerrains 
		* @return: true if ID is valid. */
		static bool GetXYFromTileID(int nTileID, int* X, int* Y);

		/** get the terrain height field buffer by ID
		* @param nID: this is the result of GetTileIDFromXY()
		* @return: return the terrain object. 
		*/
		TTerrain* GetTerrainHeightField(int nID);

		/** this function should be called in each render call per frame, when drawing shorelines.*/
		void FrameMoveCachedTerrainTiles();

		/************************************************************************/
		/* default values                                                       */
		/************************************************************************/
		static const float GRAVITY_CONSTANT;

		/** default Wind parameter values*/
		static const float m_fDefaultWindDirection;
		static const float m_fDefaultWindSpeed;
		static const float m_fDefaultWaveHeight;

		/**
		* increasing this value will align the waves
		* with the wind direction. The value MUST be
		* a multiple of 2.0 and positive. 
		* i.e. range = (2, 2^32) in steps of 2.0f
		*/
		static const float m_fDefaultDirectionalDependence;

		/** increasing this value will smooth the waves */
		static const float m_fDefaultSuppressSmallWavesFactor;

		/** current animation time of the wave */
		float m_fTime;
		/** seconds elapsed, but always in the 0,8 range.  */
		float m_fBumpTime;

		/** used for calculation stage count */
		int m_tickCounter;

		/** [0,1] */
		float m_fInterpolation;

		//////////////////////////////////////////////////////////////////////////
		/// water rendering
		//////////////////////////////////////////////////////////////////////////
		Plane                m_refractionClipPlaneAboveWater;
		Plane                m_reflectionClipPlaneAboveWater;
		Plane                m_refractionClipPlaneBelowWater;
		Plane                m_reflectionClipPlaneBelowWater;

		float                    m_refractionMapOverdraw;
		float                    m_reflectionMapOverdraw;

		int                      m_reflectionTextureWidth;
		int                      m_reflectionTextureHeight;
		int                      m_refractionTextureWidth;
		int                      m_refractionTextureHeight;
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DSURFACE9       m_pDepthStencilSurface;
		LPDIRECT3DTEXTURE9       m_waveReflectionTexture;
		LPDIRECT3DSURFACE9       m_waveReflectionSurface;
		LPDIRECT3DTEXTURE9       m_waveRefractionTexture;
		LPDIRECT3DSURFACE9       m_waveRefractionSurface;
		LPDIRECT3DTEXTURE9       m_waveReflectionNearTexture;
		LPDIRECT3DSURFACE9       m_waveReflectionNearSurface;
		LPDIRECT3DTEXTURE9       m_waveRefractionNearTexture;
		LPDIRECT3DSURFACE9       m_waveRefractionNearSurface;
#endif
		// procedure ripples on the surface. 
		//LPDIRECT3DVERTEXBUFFER9	 m_lpVBRipples; 

		Matrix4               m_projectionMatrix;
		Matrix4               m_reflectionProjectionMatrix;
		Matrix4               m_refractionProjectionMatrix;
		Matrix4               m_worldViewProjectionInverseTransposeMatrix;

		bool                     m_reflectViewMatrix;
		bool					 m_underwater;
		DWORD					 m_dwTechnique;

		/** how many seconds have passed since last water reflection map is rendered. */
		float m_fLastReflectionTime;

		/**
		* set up the rendering matrices for ocean rendering.
		* @param bPostPushMatrices:When matrices are set, push them to the global matrix stack.
		* @param bPrePopMatrices: Before setting matrices, first pop matrix on the global matrix stack.
		*/
		void SetMatrices(bool bPostPushMatrices=false, bool bPrePopMatrices=false);
		
	private:
		bool m_ReflectionItems[R_LAST_ITEM];
		
		/** whether to disable fog when rendering reflection*/
		bool m_bDisableFogInReflection;
		/// whether the ocean has been created.
		bool m_bIsCreated;
		/** high frequency water ripple: bump mapping */
		asset_ptr<TextureEntity> m_pRippleBumpMappingTexture;
		/** shorelines at the edge when ocean surface join the land. They are blended according to the water depth.*/
		asset_ptr<TextureEntity> m_pShorelineTexture;
		/** ripple texture */
		asset_ptr<TextureEntity> m_pRippleTexture;
		/** whether to use shoreline texture. */
		bool m_bUseShorelineTexture;
		/** water color texture */
		asset_ptr<TextureEntity> m_pWaterColorTexture;
		/** Modulate the reflection texture by a texture to fade out at the edges.
		This avoids an objectionable artifact when the reflection map doesn't cover enough area. */
		asset_ptr<TextureEntity> m_pWaterFadeTexture;
		/* only used in TECH_OCEAN_SIMPLE */
		asset_ptr<TextureEntity> m_pSimpleReflectionTexture;
		/* only used in TECH_OCEAN_SIMPLE */
		asset_ptr<TextureEntity> m_pSimpleBumpTexture;
		/* only used in TECH_OCEAN_CLOUD */
		asset_ptr<TextureEntity> m_pCloudTexture;
		/* only used in FILE_SUNSETGLOW */
		asset_ptr<TextureEntity> m_pSunsetTexture;
#ifdef USE_DIRECTX_RENDERER
		/** based vertex buffer*/
		LPDIRECT3DVERTEXBUFFER9 m_pBaseBuffer;
		/** index buffer*/
		LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;
		
		/** an array of cached ocean tile buffers */
		LPDIRECT3DVERTEXBUFFER9 m_pAnimBuffer[k_total_water_meshes];
		LPDIRECT3DVERTEXBUFFER9 m_pTerrainHeightmapBuffer[k_total_terrain_meshes];
#endif
		/** number of triangles in the grid */
		int m_num_triangle;

		/*LPDIRECT3DVERTEXBUFFER9 m_quad_vb;
		LPDIRECT3DINDEXBUFFER9 m_quad_ib;*/

		/** the current active buffer index in m_pAnimBuffer*/
		int m_activeBuffer;

		/** the FFT Ocean Height table for both the spacial and frequency domains.
		* the size of the matrix is m_grid_size^2 */
		sComplex* m_colH;

		/** the FFT Ocean Normal table for both the spacial and frequency domains.
		* the size of the matrix is m_grid_size^2 */
		sComplex* m_colN;

		/** The initial FFT table. I.e. the table, when t=0 */
		sComplex* m_colH0;

		/** an array of m_grid_size^2 floats containing the angular frequency */
		float* m_colAngularFreq;

		/** inline index query functions */
		float getOceanHeight(int x, int y);
		float getOceanNormalX(int x, int y);
		float getOceanNormalY(int x, int y);

		float getKx(int x);
		float getKy(int y);
		float getAngularFrequency(float fKLength);

		int getIndexH0(int x, int y);
		int getIndex(int x, int y);
		int getIndexFFT(int x, int y);
		int getOffset(int x, int y);
		int getOffsetWrap(int x, int y);

		/** compute 2D FFT */
		void FFT(sComplex* pCmpTable);
		/** set ocean parameters */
		void setOceanParameters( 
			unsigned long ulSeed, 
			float fWindDirection, 
			float fWindSpeed,
			float fWaveHeight, 
			float fDirectionalDependence, 
			float fSuppressSmallWavesFactor );
		/** get the Phillips Spectrum of a certain component*/
		float phillipsSpectrum( float fKx, float fKy );
#ifdef USE_DIRECTX_RENDERER
		/** fill the FFT data to a vertex buffer for rendering */
		void fillVertexBuffer(LPDIRECT3DVERTEXBUFFER9 pVertexBuffer);
#endif
		/** create the index buffer for a ocean tile */
		bool CreateStripGridIndexBuffer(
			int xVerts,	// width of grid
			int yVerts,	// height of grid
			int xStep,	// horz vertex count per cell
			int yStep,	// vert vertex count per cell
			int stride	// horz vertex count in vbuffer
			);

		/************************************************************************/
		/* The FFT computational stages                                         */
		/************************************************************************/
		void animateHeightTable();
		void animateNormalTable();
		void horizontalFFT(sComplex* pCmpTable );    
		void verticalFFT(sComplex* pCmpTable );

		// rendering techniques
		/** fixed function quad */
		void RenderTechnique_Quad();
		/** simple shader quad */
		int RenderTechnique_Simple(SceneState* pSceneState, float x0, float y0, float x1, float y1);
		/** complex ocean shader with FFT and ocean depth */
		int RenderTechnique_FFT_HIGH(SceneState* pSceneState, float x0, float y0, float x1, float y1);
		/** render technique cloud */
		int RenderTechnique_Cloud(SceneState* pSceneState, float x0, float y0, float x1, float y1);
		
		/** render occlusion test objects for each ocean tile. */
		int RenderTechnique_OccusionTest(SceneState* pSceneState, float x0, float y0, float x1, float y1);
		/** frame move and render ripples. */
		void RenderRipples(float fTimeDelta);

		/** frame move and render ripples. */
		void RenderUnderwaterEffect(SceneState* pSceneState);

		/** calculate which tile at what location to render. each tile is as big as m_ocean_tile_size */
		void CalculateTileToRender(SceneState* pSceneState, float& x0, float& y0, float& x1, float& y1);
	public:
		/** create the default ocean.
		* the initial ocean wave at time t=0 is prepared.
		* FFT tables are build. But no device objects are created.
		*/
		bool create();

		/** check load ocean textures */
		void CheckLoadOceanTextures(int nTechnique=0);

		/** init device objects.
		* the create() function must be called, before this function is called.*/
		void InitDeviceObjects();
		/** restore device object*/
		void RestoreDeviceObjects();
		/** Invalid device object*/
		void InvalidateDeviceObjects();
		/** delete device objects */
		void DeleteDeviceObjects();
		/** clean up all resource objects */
		void Cleanup();

		/** this function should be called as often as possible to animate the ocean.
		* currently a fixed time delta is used.*/
		void update(float fTimeDelta);

		/** redraw reflection texture only necessary. Call this function at the start of each rendering frame
		* @return whether ocean should be drawn. 
		*/
		bool UpdateReflectionTexture(SceneState & sceneState);

		/** force updating reflection texture in the next render frame. */
		void ForceUpdateOcean();

		/** render the ocean */
		void Render(SceneState* pSceneState);
		/** whether we have reflection texture drawn.  */
		bool RenderReflectionTexture();
		
		enum OCEAN_RENDER_TECHNIQUE{
			OCEAN_TECH_QUAD=0,		// requires just software processing
			OCEAN_TECH_FFT,			// requires just vertex shader 1.1 and no pixel shader
			OCEAN_TECH_REFLECTION,	// requires shader and pixel shader version 3.0
			OCEAN_TECH_FULL,		// requires shader and pixel shader version 3.0
			OCEAN_TECH_SIMPLE,		// requires just vertex and pixel shader 2
			OCEAN_TECH_CLOUD,		// cloud rendering using layered texture. 
		};
		/** obsolete: set the current ocean rendering technique. It will always fall back to a valid device.
		Hence, it may be different from GetRenderTechnique() 
		@param dwTechnique: @see OCEAN_RENDER_TECHNIQUE
		*/
		void SetRenderTechnique(DWORD dwTechnique);
		/** obsolete: get ocean simulation and rendering technique. @see OCEAN_RENDER_TECHNIQUE*/
		DWORD GetRenderTechnique();

		/** check whether a specified point is under the water surface.
		* @return: return true if point is under water surface.*/
		bool IsPointUnderWater(const Vector3& vPos);

		/** return true if draw shoreline */
		bool IsDrawShoreLine();
		/** set whether drawing shoreline */
		void DrawShoreLine(bool bDraw);
		/** clean up terrain height field cache. This is usually called when the user has changed the global terrain */
		void CleanupTerrainCache();

		/** whether the ocean has passed the occlusion test in the last frame*/
		bool IsOceanVisible();
		
		/** shall we animate FFT, animating FFT will consume quite a lot of CPU per frame. */
		bool IsAnimateFFT();
		/** Set whether we shall animate FFT, animating FFT will consume quite a lot of CPU per frame. */
		void SetAnimateFFT(bool bEnable);

	friend class CSceneObject;

	private:
		bool m_useScreenSpaceFog;

	public:
		void EnableScreenSpaceFog(bool value){
			m_useScreenSpaceFog = value;
		}
		bool IsScreenSpaceFog(){return m_useScreenSpaceFog;}
	};
}
