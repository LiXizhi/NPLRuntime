//-----------------------------------------------------------------------------
// Class:	COceanManager
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.11.12
// Algorithm: it combines the Nvidia's Vertex Texture Fetch with my own FFT based ocean simulation.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#include "OcclusionQueryBank.h"
#endif
#include "SceneObject.h"
#include "terrain/GlobalTerrain.h"
#include "terrain/TTerrain.h"
#include "ParaWorldAsset.h"
#include "AutoCamera.h"
#include "OceanManager.h"
#include "2dengine/GUIRoot.h"
#include "SortedFaceGroups.h"
#include "SunLight.h"
#include "BlockEngine/BlockWorldClient.h"
#include <time.h>
#include "memdebug.h"

//#define FILE_FIXED_FUNCTION_OCEAN_TEXTURE	"Texture/ocean.png"
#define FILE_FIXED_FUNCTION_OCEAN_TEXTURE	"Texture/ripple/colored/ripple_a028.dds"

//#define FILE_RIPPLEBUMPMAPPING	"Texture/ripple/ripple_a028.dds"
#define FILE_RIPPLEBUMPMAPPING	"Texture/ripple/WaterBumpMap.dds"

#define FILE_RIPPLE		"Texture/ripple.dds"
#define FILE_WATERFADE	"Texture/water_fade.tga"
#define FILE_SHORELINE	"Texture/shoreline/shoreline_a005.dds"
#define FILE_SUNSETGLOW	"Texture/common/Sunset.dds"
#define FILE_CLOUD1		"Texture/common/ocean_cloud.dds"

#define FILE_SIMPLE_BUMP "Texture/ripple/WaterBumpMap.dds"
#define FILE_SIMPLE_REFLECTION "Texture/ripple/waterReflectMap.dds"

/** interval in seconds between two water reflection maps are rendered */
#define MAX_WATER_REFLECTION_INTERVAL	0.01f

/** it ensures there are maximum of (MAX_OCEAN_BLOCK_COUNT+1)^2 ocean blocks. a value of 4 is common.*/
#define MAX_OCEAN_BLOCK_COUNT	7

/** default wave height in meters */
#define DEFAULT_WAVE_HEIGHT		0.6f

/** default wind speed in meters */
#define DEFAULT_WIND_SPEED	32.f

/** max number of cached terrain tiles. usually 36 is good, which costs 400KB in size when the grid size is 64.*/
#define MAX_CACHE_TERRAIN_TILES		36

/** meters per seconds*/
#define DEFAULT_RIPPLE_SPEED 1.f

/** in radius */
#define DEFAULT_RIPPLE_MAX_RADIUS 1.5f

/** max number of ripples on the surface */
#define MAX_RIPPLE_COUNT	10

/// define this to start testing everything about ocean rendering.
//#define WATER_RENDER_ENABLE_ALL

/**@def whether to use a shoreline texture*/
#define SHORELINE_TEXTURE_ENABLE	false

/** @def 64 is what 2004 year CPU can do at real time(0.0008s per frame). 
* 128 looks slightly better(0.0034s per frame). Test on AMD 1800+
* 32 is for production, with 32*32*2 triangles per ocean tile*/
#define FFT_GRID_SIZE	16

/** @def reflection map width in pixels */
#define MAX_REFLECTION_MAP_WIDTH	512
/** @def reflection map height in pixels */
#define MAX_REFLECTION_MAP_HEIGHT	512
/** @def refraction map width in pixels */
#define MAX_REFRACTION_MAP_WIDTH	512
/** @def reflection map height in pixels */
#define MAX_REFRACTION_MAP_HEIGHT	512

/** view angle overdraw for reflection map. 1.0 means the same as the current camera view angle. */
#define REFLECTION_MAP_OVERDRAW		1.25f
/** view angle overdraw for refraction map. 1.0 means the same as the current camera view angle. */
#define REFRACTION_MAP_OVERDRAW		1.25f

/**@def the clip plane is shifted by the specified meters. 0 means the water surface level */
#define OVERDRAW_DISTANCE_ABOVE	0.2f
/**@def the clip plane is shifted by the specified meters. 0 means the water surface level */
#define OVERDRAW_DISTANCE_BELOW	0.1f

/**@def wave texture speed.  */
#define WAVE_TEXTURE_SPEED	0.05f
/**
* There are three methods used to draw the ocean. 
* - basic vertex color: which requires only vs1.0 and no pixel shader or additional textures
* - PSVS_NEW: which uses in-scattering light and one additional water color index texture
* - PSVS_OLD: which uses bump mapping.
* The last two methods require at least PS 1.1. Please see gaia.1.6 for more information.
*/
// #define PSVS_NEW
// #define PSVS_OLD

/** 
* There are two ocean mesh positioning schemes:
* - fixed around camera: the ocean mesh is centered around the camera and moves with the camera.
* - tiled ocean: the ocean is fixed with the world coordinate system. Define TILED_OCEAN to enable this mode.
*   it will draw more triangles than the fixed camera mode.
*/
#define TILED_OCEAN

/** (1.0f)/sqrt(2.0f) */
#define INVERSE_ROOT_2 ((1.0f)/sqrt(2.0f))

#ifndef PI
#define PI 3.1415926535897932385f
#endif
#ifndef TWO_PI
#define TWO_PI (PI * 2.0f)
#endif



/**
Calculate the closest but lower power of two of a number
twopm = 2**m <= n
@return: TRUE if 2**m == n
*/
int Powerof2(int n,int *m,int *twopm)
{
	if (n <= 1) {
		*m = 0;
		*twopm = 1;
		return(FALSE);
	}

	*m = 1;
	*twopm = 2;
	do {
		(*m)++;
		(*twopm) *= 2;
	} while (2*(*twopm) <= n);

	if (*twopm != n) 
		return(FALSE);
	else
		return(TRUE);
}

inline float Square(float x){return x*x;}

/**
* Generate Gauss values
*/
void GaussRandomPair(float& result_a, float& result_b,
					 float dMean = 0.f, float dStdDeviation=1.0f)
{
	// Algorithm by Dr. Everett (Skip) Carter, Jr.

	float x1, x2, w, y1, y2;

	do {
		x1 = 2.0f * ParaEngine::frand() - 1.0f;
		x2 = 2.0f * ParaEngine::frand() - 1.0f;
		w = x1 * x1 + x2 * x2; 
	} while ( w >= 1.0f );

	w = sqrtf( (-2.0f * logf( w ) ) / w );
	y1= x1 * w;
	y2= x2 * w;
	result_a = dMean+ y1 * dStdDeviation;	// first gauss random
	result_b = dMean+ y2 * dStdDeviation;	// second gauss random
}

namespace ParaEngine
{
	//const DWORD COceanManager::sBaseVertex::FVF = D3DFVF_XYZ | D3DFVF_TEX1;

	const float COceanManager::GRAVITY_CONSTANT = 9.81f;

	//
	// Default parameter values borrowed from
	// Carsten Wenzel's implementation of
	// deep ocean waves 
	//
	const float COceanManager::m_fDefaultWindDirection = ( 0.f/*MATH_PI * 1.f*/ );
	const float COceanManager::m_fDefaultWindSpeed = ( DEFAULT_WIND_SPEED );
	const float COceanManager::m_fDefaultWaveHeight = DEFAULT_WAVE_HEIGHT;//384.f

	// increasing this value will align the waves
	// with the wind direction. The value MUST be
	// a multiple of 2.0 and positive. 
	// ie. range = (2, 2^32) in steps of 2.0f
	const float COceanManager::m_fDefaultDirectionalDependence = ( 4.0f );

	// increasing this value will smooth the waves
	const float COceanManager::m_fDefaultSuppressSmallWavesFactor = ( 0.00005f );


	//////////////////////////////////////////////////////////////////////////
	//
	// ripple related 
	//
	//////////////////////////////////////////////////////////////////////////
	RippleList::RippleList()
	{

	}
	RippleList::~RippleList()
	{
		ClearAll();
	}
	void RippleList::FrameMove(float fDeltaTime)
	{
		// animate the current ripple radius, and remove inactive ones.
		list<RippleItem>::iterator itCur;
		for (itCur=m_ripples.begin();itCur!=m_ripples.end();)
		{
			RippleItem& item = *itCur;
			item.fCurrentRadius += item.fSpeed*fDeltaTime;
			if(item.fCurrentRadius>=item.fMaxRadius)
			{
				itCur = m_ripples.erase(itCur);
			}
			else
			{
				++itCur;
			}
		}
	}

	void RippleList::ClearAll()
	{
		m_ripples.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// COceanManager
	//
	//////////////////////////////////////////////////////////////////////////
	COceanManager::COceanManager(void)
	{
		m_bAnimateFFT = true;
		m_bDisableFogInReflection = false;
		m_ocean_tile_size = 100.f;
		m_ocean_tile_height = 0.3f;
		m_activeBuffer=0;
		m_num_triangle = 0;
		m_bRenderOceanReflection = false;
#ifdef USE_DIRECTX_RENDERER
		m_pBaseBuffer = NULL;
		m_pIndexBuffer = NULL;
		m_pDepthStencilSurface = NULL;
		m_waveReflectionTexture=NULL;
		m_waveReflectionSurface = NULL;
		m_waveRefractionTexture = NULL;
		m_waveRefractionSurface = NULL;
		m_waveReflectionNearTexture = NULL;
		m_waveReflectionNearSurface = NULL;
		m_waveRefractionNearTexture = NULL;
		m_waveRefractionNearSurface = NULL;
		memset(m_pAnimBuffer, NULL, sizeof(LPDIRECT3DVERTEXBUFFER9)*k_total_water_meshes);
		memset(m_pTerrainHeightmapBuffer, NULL, sizeof(LPDIRECT3DVERTEXBUFFER9)*k_total_terrain_meshes);
#endif
		m_fLastReflectionTime = MAX_WATER_REFLECTION_INTERVAL;

		m_colH = NULL;
		m_colN = NULL;
		m_colH0 = NULL;
		m_colAngularFreq = NULL;

		m_fInterpolation = 0.f;
		m_fBumpTime = 0.f;
		m_fTime = 0.f;
		
		m_bUseShorelineTexture = SHORELINE_TEXTURE_ENABLE;
		m_bIsCreated = false;
		m_nMaxRippleNumber = MAX_RIPPLE_COUNT;
		
		SetFFTGrid(FFT_GRID_SIZE);

		SetGlobalWater(false, 0.0f);

		m_refractionMapOverdraw             = REFRACTION_MAP_OVERDRAW;
		m_reflectionMapOverdraw             = REFLECTION_MAP_OVERDRAW;
		m_reflectionTextureWidth            = MAX_REFLECTION_MAP_WIDTH;
		m_reflectionTextureHeight           = MAX_REFLECTION_MAP_HEIGHT;
		m_refractionTextureWidth            = MAX_REFRACTION_MAP_WIDTH;
		m_refractionTextureHeight           = MAX_REFRACTION_MAP_HEIGHT;

		// Initialize clip planes
		m_refractionClipPlaneAboveWater = Plane(0.0f, -1.0f, 0.0f, OVERDRAW_DISTANCE_ABOVE); 
		m_reflectionClipPlaneAboveWater = Plane(0.0f,  1.0f, 0.0f, OVERDRAW_DISTANCE_ABOVE);
		// D3DXPlaneFromPointNormal(&m_reflectionClipPlaneAboveWater, &Vector3(0,-OVERDRAW_DISTANCE_ABOVE,0), &Vector3(0,-1,0));

		m_refractionClipPlaneBelowWater = Plane(0.0f,  1.0f, 0.0f, OVERDRAW_DISTANCE_BELOW);
		m_reflectionClipPlaneBelowWater = Plane(0.0f, -1.0f, 0.0f, OVERDRAW_DISTANCE_BELOW);
		//m_lpVBRipples = NULL;

		m_reflectViewMatrix = false;
		m_underwater = false;
		m_dwTechnique = OCEAN_TECH_FULL;
		m_colorOcean = LinearColor(0.2f,0.3f,0.3f,1.f);
		//m_dwTechnique = OCEAN_TECH_QUAD; // testing this technique

		m_nMaxCacheSize = MAX_CACHE_TERRAIN_TILES;
		m_bDrawShoreline = true;

		for (int i=0; i<R_LAST_ITEM;++i)
		{
			m_ReflectionItems[i] = false;
		}
		m_ReflectionItems[R_SKY_BOX] = true;

		m_useScreenSpaceFog = true;
	}

	COceanManager* COceanManager::GetInstance()
	{
		static COceanManager g_instance;
		return &g_instance;
	}

	bool COceanManager::OceanEnabled()
	{
		return m_bDrawGlobalWater;
	}
	void COceanManager::EnableOcean(bool Enable)
	{
		m_bDrawGlobalWater = Enable;
		if(!m_bDrawGlobalWater)
		{
			m_rippleList.ClearAll();
		}
	}

	bool COceanManager::IsOceanVisible()
	{
		return !m_bRenderOceanReflection;
	}

	void COceanManager::SetRenderTechnique(DWORD dwTechnique)
	{
#ifdef USE_DIRECTX_RENDERER
		DWORD PixelShaderVersion = CGlobals::GetDirectXEngine().m_d3dCaps.PixelShaderVersion;
		DWORD VertexShaderVersion = CGlobals::GetDirectXEngine().m_d3dCaps.VertexShaderVersion;
		if(dwTechnique==OCEAN_TECH_FULL || dwTechnique==OCEAN_TECH_REFLECTION || dwTechnique==OCEAN_TECH_SIMPLE)
		{
			if(PixelShaderVersion<D3DPS_VERSION(2,0) || VertexShaderVersion<D3DVS_VERSION(2,0))
			{
				dwTechnique=OCEAN_TECH_FFT;
			}
			else if(PixelShaderVersion<D3DPS_VERSION(3,0) || VertexShaderVersion<D3DVS_VERSION(3,0))
			{
				m_dwTechnique = OCEAN_TECH_SIMPLE;
				return;
			}
		}
		if(dwTechnique==OCEAN_TECH_FFT)
		{
			if(VertexShaderVersion<D3DVS_VERSION(1,1)){
				dwTechnique=OCEAN_TECH_QUAD;
			}
			else{
				m_dwTechnique = dwTechnique;
				return;
			}
		}
		if(dwTechnique==OCEAN_TECH_CLOUD)
		{
			// if technique is low always disable cloud rendering. 
		}
		m_dwTechnique = dwTechnique;
#endif
	}
	DWORD COceanManager::GetRenderTechnique()
	{
		return m_dwTechnique;
	}

	void COceanManager::SetFFTGrid(int nSize)
	{
		m_grid_size = nSize;
		m_half_grid_size = nSize/2;
		int twopm;
		Powerof2(nSize,&m_log_grid_size,&twopm);
		PE_ASSERT(twopm == nSize);

		DeleteGrid();
		/** create data structures for FFT */
		int tableSize = m_grid_size*m_grid_size;
		m_colH = new sComplex[tableSize];
		m_colN = new sComplex[tableSize];
		m_colH0 = new sComplex[(m_grid_size+1)*(m_grid_size+1)];
		m_colAngularFreq = new float[tableSize];
	}

	void COceanManager::DeleteGrid()
	{
		SAFE_DELETE_ARRAY(m_colH);
		SAFE_DELETE_ARRAY(m_colN);
		SAFE_DELETE_ARRAY(m_colH0);
		SAFE_DELETE_ARRAY(m_colAngularFreq);
	}

	COceanManager::~COceanManager(void)
	{
		Cleanup();
	}

	void COceanManager::CleanupTerrainCache()
	{
		// clean up cached height fields.
		map<int, TerrainTileCacheItem>::iterator itCurCP, itEndCP = m_pCachedTerrains.end();
		for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			delete ((*itCurCP).second.pHeightmap);
		}
		m_pCachedTerrains.clear();
	}

	void COceanManager::Cleanup()
	{
		CleanupTerrainCache();
		DeleteGrid();
		m_pRippleBumpMappingTexture.reset();
		m_pShorelineTexture.reset();
		m_pRippleTexture.reset();
		m_pWaterColorTexture.reset();
		m_pWaterFadeTexture.reset();

		m_pSimpleBumpTexture.reset();
		m_pSimpleReflectionTexture.reset();
	}

	void COceanManager::CheckLoadOceanTextures( int nTechnique/*=0*/ )
	{
		if(nTechnique == 0)
		{
			if(m_pWaterColorTexture==0)
			{
				m_pWaterColorTexture = CGlobals::GetAssetManager()->LoadTexture("waterColor", FILE_FIXED_FUNCTION_OCEAN_TEXTURE, TextureEntity::StaticTexture);
				if(m_pWaterColorTexture->SurfaceType == TextureEntity::TextureSequence)
					m_pWaterColorTexture->GetAnimatedTextureInfo()->m_fFPS = 15.f;
			}
		}
		else if(nTechnique == TECH_OCEAN)
		{
			if(m_pRippleBumpMappingTexture == 0)
			{
				m_pRippleBumpMappingTexture = CGlobals::GetAssetManager()->LoadTexture("RippleBumpMapping", FILE_RIPPLEBUMPMAPPING, TextureEntity::StaticTexture);
				if(m_pRippleBumpMappingTexture->SurfaceType == TextureEntity::TextureSequence)
					m_pRippleBumpMappingTexture->GetAnimatedTextureInfo()->m_fFPS = 15.f;
			}

			if(m_bUseShorelineTexture && m_pShorelineTexture==0)
			{
				m_pShorelineTexture = CGlobals::GetAssetManager()->LoadTexture("shoreline", FILE_SHORELINE, TextureEntity::StaticTexture);
			}

			#ifdef ENABLE_FADING	
				/// load fade texture for reflection and refraction texture fading.
				if(m_pWaterFadeTexture==0)
					m_pWaterFadeTexture = CGlobals::GetAssetManager()->LoadTexture("WaterFade",FILE_WATERFADE, TextureEntity::StaticTexture);
			#endif	
		}
		else if(nTechnique == TECH_OCEAN_CLOUD)
		{
			if(m_pSunsetTexture == 0)
				m_pSunsetTexture = CGlobals::GetAssetManager()->LoadTexture("", FILE_SUNSETGLOW, TextureEntity::StaticTexture);
			if(m_pCloudTexture == 0)
				m_pCloudTexture = CGlobals::GetAssetManager()->LoadTexture("", FILE_CLOUD1, TextureEntity::StaticTexture);
		}
		else if(nTechnique == TECH_OCEAN_SIMPLE)
		{
			if(m_pSimpleReflectionTexture == 0)
				m_pSimpleReflectionTexture = CGlobals::GetAssetManager()->LoadTexture("", FILE_SIMPLE_REFLECTION, TextureEntity::StaticTexture);
			
			if(m_pSimpleBumpTexture == 0)
				m_pSimpleBumpTexture = CGlobals::GetAssetManager()->LoadTexture("", FILE_SIMPLE_BUMP, TextureEntity::StaticTexture);
		}
	}

	bool COceanManager::create()
	{
		// set initial parameters
		setOceanParameters( 
			(unsigned long)time(NULL), 
			m_fDefaultWindDirection, 
			m_fDefaultWindSpeed,
			m_fDefaultWaveHeight, 
			m_fDefaultDirectionalDependence,
			m_fDefaultSuppressSmallWavesFactor );

		//
		// prepare for the first frame
		//
		m_fTime = 24.24f; // some arbitrary number
		m_fBumpTime = 0;
		animateHeightTable();
		animateNormalTable();
		horizontalFFT(m_colH );    
		horizontalFFT(m_colN );    
		verticalFFT(m_colH );    
		verticalFFT(m_colN );    
		m_tickCounter = 0;
		
		m_bIsCreated = true;

		if(m_pRippleTexture == 0)
			m_pRippleTexture = CGlobals::GetAssetManager()->LoadTexture("Ripple", FILE_RIPPLE, TextureEntity::StaticTexture);

		return true;
	}

	void COceanManager::SetGlobalWater(bool bEnable, float fWaterLevel)
	{
		EnableOcean(bEnable);
		m_fGlobalWaterLevel = fWaterLevel;
	}

	float COceanManager::GetMaxWaterLevel()
	{
		//TODO: should add the wave height. Currently it is only a fixed 1.0f.
		return m_fGlobalWaterLevel; 
	}
	float COceanManager::GetWaterLevel()
	{
		return m_fGlobalWaterLevel; 
	}
	void COceanManager::SetWaterLevel(float fLevel)
	{
		m_fGlobalWaterLevel = fLevel; 
		m_fLastReflectionTime = MAX_WATER_REFLECTION_INTERVAL;
	}
	bool COceanManager::IsUnderWater()
	{
		return m_underwater;
	}
	bool COceanManager::CreateStripGridIndexBuffer(
		int xVerts,	// width of grid
		int yVerts,	// height of grid
		int xStep,	// horz vertex count per cell
		int yStep,	// vert vertex count per cell
		int stride	// horz vertex count in vbuffer
		)
	{
#ifdef USE_DIRECTX_RENDERER
		int total_strips = 
			yVerts-1;
		int total_indexes_per_strip = 
			xVerts<<1;
		
		// the total number of indice is equal
		// to the number of strips times the
		// indice used per strip plus one
		// degenerate triangle between each strip
		int total_indexes = 
			(total_indexes_per_strip * total_strips) 
			+ (total_strips<<1) - 2;
		m_num_triangle = total_indexes -2;

		/// create and fill the Direct3D index buffer
		LPDIRECT3DDEVICE9 pD3dDevice = CGlobals::GetRenderDevice();
		HRESULT result= pD3dDevice->CreateIndexBuffer(sizeof(unsigned short)*total_indexes,
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_MANAGED,
			&m_pIndexBuffer,
			0);

		if(FAILED(result))
		{
			return false;
		}

		unsigned short* pIndexValues =NULL;
		m_pIndexBuffer->Lock(0,0,(void**)&pIndexValues,0);
		
		unsigned short* index = pIndexValues;
		unsigned short start_vert = 0;
		unsigned short lineStep = yStep*stride;

		for (int j=0;j<total_strips;++j)
		{
			int k=0;
			unsigned short vert=start_vert;

			// create a strip for this row
			for (k=0;k<xVerts;++k)
			{
				*(index++) = vert;
				*(index++) = vert + lineStep;
				vert += xStep;
			}
			start_vert += lineStep;

			if (j+1<total_strips)
			{
				// add a degenerate to attach to 
				// the next row
				*(index++) = (vert-xStep)+lineStep;
				*(index++) = start_vert;
			}
		}
		m_pIndexBuffer->Unlock();
#endif
		return true;
	}

	void COceanManager::InitDeviceObjects()
	{
		if(!m_bIsCreated)
			return;
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DDEVICE9 pD3dDevice = CGlobals::GetRenderDevice();

		/** 
		* Create the base vertex buffer
		*/
		if(m_pBaseBuffer == NULL)
		{
			pD3dDevice->CreateVertexBuffer(m_grid_size*m_grid_size*sizeof(sBaseVertex), 
				D3DUSAGE_WRITEONLY, NULL, 
				D3DPOOL_MANAGED, &(m_pBaseBuffer), NULL);

			sBaseVertex* pBaseVerts = NULL;
			m_pBaseBuffer->Lock(0,0, (void**)&pBaseVerts, 0);

			sBaseVertex* pVertices = pBaseVerts;
			if (pVertices)
			{
				float u = 0.0f;
				float v = 0.0f;
				float uvStep = 
					1.0f/(float) ( m_grid_size - 1 );

				for( int y=0; y<m_grid_size; ++y )
				{
					for( int x=0; x<m_grid_size; ++x )
					{
						pVertices->vPos.x = u*m_ocean_tile_size;
						pVertices->vPos.y = v*m_ocean_tile_size;

						++pVertices;
						u += uvStep;
					}       
					v += uvStep;
					u = 0.0f;
				}
			}
			m_pBaseBuffer->Unlock();
		}
				
		/** 
		* Create the grid strip Index buffer
		*/
		if(m_pIndexBuffer == NULL)
		{
			CreateStripGridIndexBuffer(
				m_grid_size,
				m_grid_size,
				1,1,
				m_grid_size);
		}
		
		/**
		* build the dynamic vertex buffers
		*/
		int i=0;
		for (i=0; i<k_total_water_meshes; ++i)
		{
			if(m_pAnimBuffer[i] == NULL)
			{
				if(FAILED(pD3dDevice->CreateVertexBuffer(m_grid_size*m_grid_size*sizeof(sAnimVertex), 
					D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, NULL, 
					D3DPOOL_DEFAULT, &(m_pAnimBuffer[i]), NULL)))
				{
					OUTPUT_LOG("ocean animation buffer failed to create");
					return;	
				}
				fillVertexBuffer(m_pAnimBuffer[i]);
			}
		}
		/**
		* build the dynamic vertex buffer for terrain.
		*/
		for (i=0; i<k_total_terrain_meshes; ++i)
		{
			if(m_pTerrainHeightmapBuffer[i] == NULL)
			{
				if(FAILED(pD3dDevice->CreateVertexBuffer(m_grid_size*m_grid_size*sizeof(float), 
					D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, NULL, 
					D3DPOOL_DEFAULT, &(m_pTerrainHeightmapBuffer[i]), NULL)))
				{
					OUTPUT_LOG("ocean terrain buffer failed to create");
					return;	
				}
			}
		}
#endif
	}

#define CHECK_RETURN_CODE(text, hr) if(FAILED((hr))){OUTPUT_LOG(text);return;}

	void COceanManager::RestoreDeviceObjects()
	{
#ifdef USE_DIRECTX_RENDERER
		HRESULT hr;
		LPDIRECT3DDEVICE9 pD3dDevice = CGlobals::GetRenderDevice();
		int i=0;
		/**
		* Fill the dynamic vertex buffers, if it has not been filled
		*/
		for (i=0; i<k_total_water_meshes; ++i)
		{
			if(m_pAnimBuffer[i] == NULL)
			{
				if(FAILED(pD3dDevice->CreateVertexBuffer(m_grid_size*m_grid_size*sizeof(sAnimVertex), 
					D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, NULL, 
					D3DPOOL_DEFAULT, &(m_pAnimBuffer[i]), NULL)))
				{
					OUTPUT_LOG("ocean animation buffer failed to create");
					return;	
				}
				fillVertexBuffer(m_pAnimBuffer[i]);
			}
		}

		/**
		* build the dynamic vertex buffer for terrain.
		*/
		for (i=0; i<k_total_terrain_meshes; ++i)
		{
			if(m_pTerrainHeightmapBuffer[i] == NULL)
			{
				if(FAILED(pD3dDevice->CreateVertexBuffer(m_grid_size*m_grid_size*sizeof(float), 
					D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, NULL, 
					D3DPOOL_DEFAULT, &(m_pTerrainHeightmapBuffer[i]), NULL)))
				{
					OUTPUT_LOG("ocean terrain buffer failed to create");
					return;	
				}
			}
		}

		if(GetRenderTechnique()>=OCEAN_TECH_REFLECTION)
		{
			int deviceWidth = (int)CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Width;
			int deviceHeight = (int)CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Height;
			// Create the wave reflection and refraction textures for objects
			// that do not penetrate the surface of the water.
			int nWidth = min(deviceWidth, m_reflectionTextureWidth);
			int nHeight = min(deviceHeight, m_reflectionTextureHeight);
			hr = pD3dDevice->CreateTexture(nWidth, 	nHeight, 
				1, D3DUSAGE_RENDERTARGET, 
				D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_waveReflectionTexture, NULL);
			CHECK_RETURN_CODE("CreateTexture m_waveReflectionTexture", hr);  

			hr = m_waveReflectionTexture->GetSurfaceLevel(0, &m_waveReflectionSurface);
			CHECK_RETURN_CODE("GetSurfaceLevel m_waveReflectionSurface", hr);

			hr = pD3dDevice->CreateDepthStencilSurface(nWidth, nHeight, D3DFMT_D16, 
				D3DMULTISAMPLE_NONE, 0, FALSE, &m_pDepthStencilSurface, NULL);
			CHECK_RETURN_CODE("failed creating depth stencil buffer", hr);

#ifdef WATER_RENDER_ENABLE_ALL
			hr = pD3dDevice->CreateTexture(min(deviceWidth, m_refractionTextureWidth), 
				min(deviceHeight, m_refractionTextureHeight), 
				1, D3DUSAGE_RENDERTARGET, 
				D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_waveRefractionTexture, NULL);
			CHECK_RETURN_CODE("CreateTexture m_waveRefractionTexture", hr);

			hr = m_waveRefractionTexture->GetSurfaceLevel(0, &m_waveRefractionSurface);
			CHECK_RETURN_CODE("GetSurfaceLevel m_waveRefractionSurface", hr);

			// Create the second set of refraction and reflection render targets for objects
			// that are penetrating the surface of the water.
			hr = pD3dDevice->CreateTexture(min(deviceWidth, m_reflectionTextureWidth), 
				min(deviceHeight, m_reflectionTextureHeight), 
				1, D3DUSAGE_RENDERTARGET, 
				D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_waveReflectionNearTexture, NULL);
			CHECK_RETURN_CODE("CreateTexture m_waveReflectionNearTexture", hr);  

			hr = m_waveReflectionNearTexture->GetSurfaceLevel(0, &m_waveReflectionNearSurface);
			CHECK_RETURN_CODE("GetSurfaceLevel m_waveReflectionNearSurface", hr);

			hr = pD3dDevice->CreateTexture(min(deviceWidth, m_refractionTextureWidth), 
				min(deviceHeight, m_refractionTextureHeight), 
				1, D3DUSAGE_RENDERTARGET, 
				D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_waveRefractionNearTexture, NULL);
			CHECK_RETURN_CODE("CreateTexture m_waveRefractionNearTexture", hr);

			hr = m_waveRefractionNearTexture->GetSurfaceLevel(0, &m_waveRefractionNearSurface);
			CHECK_RETURN_CODE("GetSurfaceLevel m_waveRefractionNearSurface", hr);
#endif
		}
#endif
	}

	void COceanManager::InvalidateDeviceObjects()
	{
#ifdef USE_DIRECTX_RENDERER
		/** invalid animation buffer */
		if(m_pAnimBuffer)
		{
			for (int i=0; i<k_total_water_meshes; ++i)
			{
				SAFE_RELEASE(m_pAnimBuffer[i]);
			}
		}
		/** invalid animation buffer */
		if(m_pTerrainHeightmapBuffer)
		{
			for (int i=0; i<k_total_terrain_meshes; ++i)
			{
				SAFE_RELEASE(m_pTerrainHeightmapBuffer[i]);
			}
		}

		SAFE_RELEASE(m_pDepthStencilSurface);
		SAFE_RELEASE(m_waveReflectionTexture);
		SAFE_RELEASE(m_waveRefractionTexture);
		SAFE_RELEASE(m_waveReflectionSurface);
		SAFE_RELEASE(m_waveRefractionSurface);
		SAFE_RELEASE(m_waveReflectionNearTexture);
		SAFE_RELEASE(m_waveRefractionNearTexture);
		SAFE_RELEASE(m_waveReflectionNearSurface);
		SAFE_RELEASE(m_waveRefractionNearSurface);
#endif
	}

	void COceanManager::DeleteDeviceObjects()
	{
#ifdef USE_DIRECTX_RENDERER
		SAFE_RELEASE(m_pIndexBuffer);
		SAFE_RELEASE(m_pBaseBuffer);
	
		if(m_pAnimBuffer)
		{
			for (int i=0; i<k_total_water_meshes; ++i)
			{
				SAFE_RELEASE(m_pAnimBuffer[i]);
			}
		}

		if(m_pTerrainHeightmapBuffer)
		{
			for (int i=0; i<k_total_terrain_meshes; ++i)
			{
				SAFE_RELEASE(m_pTerrainHeightmapBuffer[i]);
			}
		}
#endif
	}

	float COceanManager::phillipsSpectrum( float fKx, float fKy )
	{
		// normalize the 2D vector defined by (fKx, fKy)
		float fKLength(sqrtf((fKx*fKx) + (fKy*fKy)));
		fKLength = max(fKLength, 1e-8f);
		float fScale( 1.0f / fKLength );
		fKx *= fScale;
		fKy *= fScale;

		// perform the phillipsSpectrum calculation
		return ( m_fWaveHeight * 
			expf( -1.0f / Square( fKLength * m_fLargestPossibleWave ) - Square( fKLength * m_fSuppressSmallWaves ) ) * 
			powf( fKx * m_fWindX + fKy * m_fWindY, m_fDirectionalDependence ) / powf( fKLength, 4.0f ) );
	}


	void COceanManager::setOceanParameters( 
		unsigned long ulSeed, 
		float fWindDirection, 
		float fWindSpeed,
		float fWaveHeight, 
		float fDirectionalDependence, 
		float fSuppressSmallWavesFactor )
	{
		m_fWindX                    = cos( fWindDirection );    
		m_fWindY                    = sin( fWindDirection );
		m_fWindSpeed                = fWindSpeed;
		m_fWaveHeight               = fWaveHeight;
		m_fDirectionalDependence    = fDirectionalDependence;
		m_fSuppressSmallWavesFactor = fSuppressSmallWavesFactor;

		m_fLargestPossibleWave =  m_fWindSpeed * m_fWindSpeed / GRAVITY_CONSTANT;   
		m_fSuppressSmallWaves  = m_fLargestPossibleWave * m_fSuppressSmallWavesFactor;

		// reset the global random seed
		
		srand(ulSeed);

		int m_half_grid_size = m_grid_size/2;
		// init H0
		int j;
		for( j = -m_half_grid_size;j<= m_half_grid_size; ++j )
		{
			for( int i = -m_half_grid_size;i<= m_half_grid_size; ++i )
			{
				sComplex cmpRnd;
				GaussRandomPair(cmpRnd.real, cmpRnd.imag);

				float scalar = 
					INVERSE_ROOT_2 * 
					sqrtf( phillipsSpectrum(getKx(i), getKy(j)));

				cmpRnd.real *= scalar;
				cmpRnd.imag *= scalar;

				m_colH0[ getIndexH0(i,j) ] = cmpRnd;
			}
		} 

		// init angular frequencies
		for(j= -m_half_grid_size;j< m_half_grid_size; ++j )
		{
			for( int i = -m_half_grid_size;i< m_half_grid_size; ++i )
			{
				float fKLength( sqrtf(Square(getKx(i)) + Square(getKy(j))) );
				m_colAngularFreq[getIndex(i,j)] = getAngularFrequency(fKLength);
			}
		}
	}

	void COceanManager::FFT(sComplex* pCmpTable)
	{
		long nn,i,i1,j,k,i2,l,l1,l2;
		float c1,c2,treal,timag,t1,t2,u1,u2,z;

		nn = m_grid_size;

		sComplex* pCmpi;
		sComplex* pCmpj;
		sComplex* pCmpi1;

		// bit reversal
		i2 = nn >> 1;
		j= 0;
		for(i= 0;i< nn - 1; ++i )
		{
			if(i<j)
			{
				pCmpi = &pCmpTable[i];
				pCmpj = &pCmpTable[j];

				treal = pCmpi->real;
				timag = pCmpi->imag;
				pCmpi->real = pCmpj->real;
				pCmpi->imag = pCmpj->imag;
				pCmpj->real = treal;
				pCmpj->imag = timag;
			}

			k = i2;
			while( k <=j)
			{
				j-= k;
				k >>= 1;
			}

			j+= k;
		}

		// Compute the FFT
		c1 = -1.0f;
		c2 = 0.0f;
		l2 = 1;
		for( l = 0; l < m_log_grid_size; ++l )
		{
			l1 = l2;
			l2 <<= 1;
			u1 = 1.0;
			u2 = 0.0;
			for(j= 0;j< l1; ++j )
			{
				for(i= j;i< nn;i+= l2 )
				{
					i1 =i+ l1;

					pCmpi = &pCmpTable[i];
					pCmpi1 = &pCmpTable[i1];

					t1 = u1 * pCmpi1->real- u2 * pCmpi1->imag;
					t2 = u1 * pCmpi1->imag+ u2 * pCmpi1->real;
					pCmpi1->real = pCmpi->real - t1;
					pCmpi1->imag = pCmpi->imag - t2;
					pCmpi->real += t1;
					pCmpi->imag += t2;
				}

				z =  u1 * c1 - u2 * c2;
				u2 = u1 * c2 + u2 * c1;
				u1 = z;
			}

			c2 = sqrtf( ( 1.0f - c1 ) * 0.5f );
			c1 = sqrtf( ( 1.0f + c1 ) * 0.5f );
		}
	}

	void COceanManager::horizontalFFT(sComplex* pCmpTable )
	{
		// Transform the rows
		for( int j = 0;j< m_grid_size; ++j )
		{
			FFT(&pCmpTable[m_grid_size*j]);
		}
	}

	void COceanManager::verticalFFT(sComplex* pCmpTable )
	{
		static vector<sComplex> tableCopy(m_grid_size);
		tableCopy.resize(m_grid_size);

		// Transform the columns
		for( int i = 0;i< m_grid_size; ++i )
		{
			int j;
			for( j = 0;j< m_grid_size; ++j )
			{
				tableCopy[j] = pCmpTable[ getOffset(i,j) ];
			}

			FFT(&(tableCopy[0]));

			for(j= 0;j< m_grid_size; ++j )
			{
				pCmpTable[ getOffset(i,j) ] = tableCopy[j];
			}
		}
	}
	
	void COceanManager::animateHeightTable()
	{
		for(int j = -m_half_grid_size;
			j< m_half_grid_size; 
			++j )
		{
			for(int i = -m_half_grid_size;
				i< m_half_grid_size; 
				++i )
			{
				float fAngularFreq=	m_colAngularFreq[ getIndex(i,j) ]* m_fTime;
				float fCos=cos(fAngularFreq);
				float fSin=sin(fAngularFreq);

				int indexFFT = getIndexFFT(i,j);
				int indexHO = getIndexH0(i,j);
				int indexHOn = getIndexH0(-i,-j);

				// update the tale of complex numbers 
				// affecting water surface height.
				m_colH[indexFFT].real= 
					( m_colH0[indexHO].real
					+ m_colH0[indexHOn].real ) * fCos -
					( m_colH0[indexHO].imag 
					+ m_colH0[indexHOn].imag ) * fSin;

				m_colH[indexFFT].imag= 
					( m_colH0[indexHO].real
					- m_colH0[indexHOn].real ) * fSin + 
					( m_colH0[indexHO].imag 
					- m_colH0[indexHOn].imag ) * fCos;
			}
		}
	}

	void COceanManager::animateNormalTable()
	{
		for( int j = -m_half_grid_size;j< m_half_grid_size; ++j )
		{
			for( int i = -m_half_grid_size;i< m_half_grid_size; ++i )
			{
				// update normal
				float fKx( getKx(i) );
				float fKy( getKy(j) );

				// Put the two functions for n(Kx) and n(Ky) into one complex array
				// (that is, multiply the n(Ky) by i and add it to n(Kx) ) to do one FFT instead of two.
				// This assumes that the inverse of both transforms are real functions (as it is the case here)!
				// After the FFT the x component of the normal will be stored in the real part, the y component 
				// in the complex part of m_colN respectively.

				int nIndex = getIndexFFT(i,j);
				m_colN[ nIndex ].real= 
					-m_colH[ nIndex ].imag * fKx - m_colH[ nIndex ].real * fKy;
				m_colN[ nIndex ].imag=  
					m_colH[ nIndex ].real * fKx - m_colH[ nIndex ].imag * fKy;
			}
		}
	}
#ifdef USE_DIRECTX_RENDERER
	void COceanManager::fillVertexBuffer(LPDIRECT3DVERTEXBUFFER9 pVertexBuffer)
	{
		sAnimVertex* pStartVertex;
		pVertexBuffer->Lock( 0, 0, (void**)&pStartVertex, D3DLOCK_DISCARD );

		sAnimVertex* pVertices = pStartVertex;
		if (pVertices)
		{
			
			for( int y=0; y<m_grid_size; ++y )
			{
				for( int x=0; x<m_grid_size; ++x )
				{
					pVertices->zPos = getOceanHeight(x, y);
					
					pVertices->vNormal.x = getOceanNormalX(x, y);
					pVertices->vNormal.y = getOceanNormalY(x, y);
					//	pVertices->vNormal.z = 24.0f;

					//	pVertices->vNormal.normalize();

					++pVertices;
				}       
			}

			// make sure all edge data is the same
			sAnimVertex* pLeftVert = pStartVertex;
			sAnimVertex* pTopVert = pStartVertex;
			sAnimVertex* pRightVert = &pStartVertex[m_grid_size-1];
			sAnimVertex* pBottomVert = &pStartVertex[(m_grid_size-1)*m_grid_size];
			sAnimVertex* pEndVert = &pStartVertex[(m_grid_size*m_grid_size)-1];

			pLeftVert->zPos = pRightVert->zPos;
			pLeftVert->vNormal = pRightVert->vNormal;
			pBottomVert->zPos = pRightVert->zPos;
			pBottomVert->vNormal = pRightVert->vNormal;
			pEndVert->zPos = pRightVert->zPos;
			pEndVert->vNormal = pRightVert->vNormal;

			for( int i=0; i<m_grid_size; ++i )
			{
				pLeftVert->zPos = pRightVert->zPos;
				pLeftVert->vNormal = pRightVert->vNormal;
				pTopVert->zPos = pBottomVert->zPos;
				pTopVert->vNormal = pBottomVert->vNormal;

				pTopVert++;
				pBottomVert++;
				pLeftVert += m_grid_size;
				pRightVert += m_grid_size;
			}

			pVertexBuffer->Unlock();
		}
	}
#endif
	void COceanManager::update(float fTimeDelta)
	{
		if(!m_bIsCreated || !m_bDrawGlobalWater)
			return;
#ifdef USE_DIRECTX_RENDERER
		m_fBumpTime += fTimeDelta;
		if(m_fBumpTime>100.f)
			m_fBumpTime -= 100.f;

		if(m_dwTechnique>OCEAN_TECH_QUAD && m_dwTechnique!=OCEAN_TECH_SIMPLE)
		{
			PE_ASSERT(	m_tickCounter >= k_animateHeight);
			PE_ASSERT( m_tickCounter < k_total_process_stages);

			if(!IsAnimateFFT())
				return;

			switch(m_tickCounter)
			{
			case k_animateHeight:
				// TODO: use FRC controller.
				// m_fTime += fTimeDelta * k_total_process_stages;
				m_fTime += 0.033f; 
				animateHeightTable();
				break;
			case k_animateNormal:
				animateNormalTable();
				break;
			case k_heightFFTv:
				verticalFFT(m_colH );    
				break;
			case k_heightFFTh:
				horizontalFFT(m_colH );    
				break;
			case k_normalFFTv:
				verticalFFT(m_colN );    
				break;
			case k_normalFFTh:
				horizontalFFT(m_colN );    
				break;
			case k_uploadBuffer:
				fillVertexBuffer(
					m_pAnimBuffer[m_activeBuffer]);
				break;
			default:
				++m_activeBuffer;
				if (m_activeBuffer 
					>= k_total_water_meshes)
				{
					m_activeBuffer = 0;
				}
				break;
			};

			// compute the interpolation factor for
			// the vertices currently in use
			m_fInterpolation = //0.0f;
				(float)(m_tickCounter+1)
				/(float)(k_total_process_stages);

			++m_tickCounter;
			if (m_tickCounter == k_total_process_stages)
			{
				m_tickCounter = k_animateHeight;
				m_fInterpolation = 0.0f;
			}
		}
#endif
	}

	bool COceanManager::IsPointUnderWater(const Vector3& vPos)
	{
		return m_bDrawGlobalWater && (vPos.y<= GetMaxWaterLevel());
	}

	void COceanManager::RenderTechnique_Quad()
	{
#ifdef USE_DIRECTX_RENDERER
		CheckLoadOceanTextures(0);
		LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
		static const short pIndexBuffer[] = {
			0,1,2,0,2,3 
		};
		SPRITEVERTEX vertices[4];
		const float ocean_radius_ = 150.f;
		
		static float start_u = 0.f;
		static float start_v = 0.f;
		start_u+=(float)CGlobals::GetSceneState()->dTimeDelta*WAVE_TEXTURE_SPEED;
		if(start_u>1.0f)
			start_u -= 1.0f;
		float repeats = ocean_radius_/5.f; // repeat every 5 meters
		//TODO: animate start of u, v in the range[0,1] by time.
		vertices[0].p.x = ocean_radius_;
		vertices[0].p.z = ocean_radius_;
		vertices[0].tu = start_u + repeats;
		vertices[0].tv = start_v + repeats;

		vertices[1].p.x = -ocean_radius_;
		vertices[1].p.z = ocean_radius_;
		vertices[1].tu = start_u ;
		vertices[1].tv = start_v + repeats;

		vertices[2].p.x = -ocean_radius_;
		vertices[2].p.z = -ocean_radius_;
		vertices[2].tu = start_u;
		vertices[2].tv = start_v;

		vertices[3].p.x = ocean_radius_;
		vertices[3].p.z = -ocean_radius_;
		vertices[3].tu = start_u + repeats;
		vertices[3].tv = start_v;

		Vector3 vRenderOrig =  CGlobals::GetScene()->GetRenderOrigin();
		Vector3 vCenter = CGlobals::GetSceneState()->vEye - vRenderOrig;
		vCenter.y = m_fGlobalWaterLevel-vRenderOrig.y;
		for(int i=0;i<4;++i)
		{
			vertices[i].color = COLOR_ARGB(128,255,255,255);
			vertices[i].p.x+=vCenter.x;
			vertices[i].p.z+=vCenter.z;
			vertices[i].p.y = vCenter.y;
		}

		// set render state
		pd3dDevice->SetTexture(0,m_pWaterColorTexture->GetTexture());
		
		RenderDevice::DrawIndexedPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, D3DPT_TRIANGLELIST, 0, 
			4, 2, pIndexBuffer, D3DFMT_INDEX16,vertices, sizeof(SPRITEVERTEX));
#endif
	}

	void COceanManager::FrameMoveCachedTerrainTiles()
	{
		/** garbage collect other terrain tiles; while frame move all active tiles */
		if((int)m_pCachedTerrains.size() > m_nMaxCacheSize)
		{
			map<int, TerrainTileCacheItem>::iterator itCurCP, oldestCP, itEndCP = m_pCachedTerrains.end();
			oldestCP = itEndCP;
			///  use 0x3fffffff, instead of 0xffffffff prevents the current frame used terrain to be deleted.
			DWORD nHit = 0x3fffffff;
			for( itCurCP = m_pCachedTerrains.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				if((*itCurCP).second.nHitCount < nHit)
				{
					oldestCP = itCurCP;
					nHit = (*itCurCP).second.nHitCount;
				}
				(*itCurCP).second.FrameMove();
			}	
			if(oldestCP != itEndCP)
			{
				delete ((*oldestCP).second.pHeightmap);
				m_pCachedTerrains.erase(oldestCP);
			}
		}
	}
	TTerrain* COceanManager::GetTerrainHeightField(int nID)
	{
		map<int, TerrainTileCacheItem>::iterator iter = m_pCachedTerrains.find(nID);
		if(iter == m_pCachedTerrains.end())
		{
			pair<map<int, TerrainTileCacheItem>::iterator, bool> res = m_pCachedTerrains.insert(pair<int, TerrainTileCacheItem>(nID, TerrainTileCacheItem(new TTerrain())));
			if(res.second ==true)
			{
				/** create a new one */
				TTerrain* pTerrain = (*(res.first)).second.pHeightmap;
				if(pTerrain!=0)
				{
					
					pTerrain->CreatePlane(m_grid_size,0);

					int x=0,y=0;
					GetXYFromTileID(nID, &x,&y);
					CGlobals::GetGlobalTerrain()->GetHeightmapOf((x+0.5f)*m_ocean_tile_size, (y+0.5f)*m_ocean_tile_size, m_ocean_tile_size, pTerrain);
					
					// fixing a bug for rendering ocean above terrain hole. 
					BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
					if(pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
					{
						// tricky: max texture repeat in SM 2.0 is 128. So the minimum water level is -64. 
						if(pTerrain->GetMinHeight() < -64.f)
							pTerrain->SetMinHeight(-64.f); 
					}
					else
					{
						if(pTerrain->GetMinHeight() < -99999.f)
							pTerrain->SetMinHeight(-1000.f); 
					}

					return pTerrain;
				}
			}
			else
			{
				OUTPUT_LOG("error creating terrain tile in ocean manager\n");
				return NULL;
			}
		}
		else
		{
			(*iter).second.OnHit();
			return (*iter).second.pHeightmap;
		}
		return NULL;
	}

	// map<tileid, pixelsVisible>
	static map<int, int> ocean_tile_occlusion_result;

	int COceanManager::RenderTechnique_Simple(SceneState* pSceneState, float x0, float y0, float x1, float y1)
	{
		if(!m_bRenderOceanReflection)
			return 0;
#ifdef USE_DIRECTX_RENDERER
		CBaseCamera * pCamera = CGlobals::GetScene()->GetCurrentCamera();
		LPDIRECT3DDEVICE9 pd3dDevice = pSceneState->m_pd3dDevice;
		Vector3 vRenderOrig =  CGlobals::GetScene()->GetRenderOrigin();
		CEffectFile* pEffectFile = NULL;
		
		// determine how many ocean tiles to draw to fill the camera view,for debugging only
		int nOceanTileRendered = 0;
		CheckLoadOceanTextures(TECH_OCEAN_SIMPLE);
		
		CGlobals::GetEffectManager()->BeginEffect(TECH_OCEAN_SIMPLE);
		pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if(pEffectFile==0)
			return 0;

		if(pSceneState->dTimeDelta > 0)
		{
			m_fBumpTime += (float)(pSceneState->dTimeDelta);
			if(m_fBumpTime>100.f)
				m_fBumpTime -= 100.f;
		}

		//////////////////////////////////////
		// set effect params
		LinearColor OceanColor = CGlobals::GetScene()->GetSunLight().GetSunAmbientHue();
		//OceanColor.r*=m_colorOcean.r;
		//OceanColor.g*=m_colorOcean.g;
		//OceanColor.b*=m_colorOcean.b;
		OceanColor.r = 0.64f;
		OceanColor.g = 0.8f;
		OceanColor.b = 0.96f;
		OceanColor.a = 1.0f;
		pEffectFile->setParameter(CEffectFile::k_ConstVector1, (const float*)OceanColor);
		OceanColor.r*=0.7f;
		OceanColor.g*=0.7f;
		OceanColor.b*=0.7f;
		OceanColor.a = 1.0f;
		pEffectFile->setParameter(CEffectFile::k_ConstVector2, (const float*)OceanColor);

		// direction and speed of the wind. how many U,V per second. Since, time is time % 100; x,y has to be 0.01*n, where n is integer.
		Vector4 vBumpSpeed(0.f, 0.f, 0.f,0.f);
		vBumpSpeed.x = ((float)((int)(m_fWindSpeed*m_fWindX/10.f)))*0.01f;
		vBumpSpeed.y = ((float)((int)(m_fWindSpeed*m_fWindY/10.f)))*0.01f;
		pEffectFile->setParameter(CEffectFile::k_ConstVector3, vBumpSpeed.ptr());

		pEffectFile->setFloat(CEffectFile::k_time, m_fBumpTime);
		//pEffectFile->setInt(CEffectFile::k_specularPower, 20);
		
		pEffectFile->setParameter(CEffectFile::k_uvScaleOffset, 
			(const float*)&Vector4((1.0f /  m_reflectionMapOverdraw), 
			(1.0f / m_reflectionMapOverdraw),
			.5f * (1.0f - (1.0f / m_reflectionMapOverdraw)),
			.5f * (1.0f - (1.0f / m_reflectionMapOverdraw))));
		
		/** for under water surface rendering, we will use NONE culling, 
		* for above water surface rendering, we will use CCW culling. */
		if(m_underwater)
			CGlobals::GetEffectManager()->SetCullingMode(false);
		else
			CGlobals::GetEffectManager()->SetCullingMode(true);

		//////////////////////////////////////////////////////////////
		// unlike regular objects, we do not render through the render
		// queue. Instead, we render on-demand using our internal settings
		if (pEffectFile->begin(true, 0))
		{
			// supply our material to the render method
			pEffectFile->applySurfaceMaterial(&pSceneState->GetGlobalMaterial());

			{
				// set the new render origin
				Vector4 vWorldPos(0,0,0,1.f);
				/** this is for height shift, using the render origin. */
				vWorldPos.x = m_fGlobalWaterLevel; // absolute position is in x component
				vWorldPos.y = m_fGlobalWaterLevel-vRenderOrig.y; // render space position is in y component
				pEffectFile->setParameter(
					CEffectFile::k_worldPos,
					&vWorldPos);
			}
			int totalPasses = pEffectFile->totalPasses();

			for (int i=0; i<totalPasses; ++i)
			{
				if(pEffectFile->BeginPass(i))
				{
					pEffectFile->setTexture(0, m_pSimpleBumpTexture.get());
					pEffectFile->setTexture(1, m_pSimpleReflectionTexture.get());
					
					nOceanTileRendered = 0;

					for (float y=y0; y<y1; y=y+1.0f)
					{
						for (float x=x0; x<x1; x=x+1.0f)
						{
							/** Cull ocean tiles that are not in the view frustum
							*/
							{
								// use global water level as high as the water level of the current tile.
								float fWaterLevel = m_fGlobalWaterLevel;
								int nTileID = (int)(x*1000+y);
								// only render if occlusion test passed. 
								if(ocean_tile_occlusion_result.find(nTileID) == ocean_tile_occlusion_result.end() || ocean_tile_occlusion_result[nTileID]>0)
								{
									#define PT_COUNT	8
									const static Vector3 cubePT[PT_COUNT] = {
										Vector3(0, 0, 0), 
										Vector3(1, 0, 0), 
										Vector3(1, 0, 1), 
										Vector3(0, 0, 1), 
										Vector3(0, 1, 0), 
										Vector3(1, 1, 0), 
										Vector3(1, 1, 1), 
										Vector3(0, 1, 1), 
									};

									Vector3 pt[PT_COUNT];
									for (int i=0; i<PT_COUNT;i++)
									{
										pt[i].x = (cubePT[i].x+ x) * m_ocean_tile_size;
										pt[i].z = (cubePT[i].z+ y) * m_ocean_tile_size;
										pt[i].y = fWaterLevel+(cubePT[i].y-0.5f)/* *m_fWaveHeight */;//? why m_fWaveHeight is 387.f?
										pt[i] -= vRenderOrig; // shift the render origin
									}
									if(pCamera->GetObjectFrustum()->CullPointsWithFrustum(pt, PT_COUNT))
										nOceanTileRendered ++;
									else
										continue;
								}
								else
									continue;
							}

							// use global water level as high as the water level of the current tile.
							float fWaterLevel = m_fGlobalWaterLevel;
							mesh_vertex_plain v[4] = {
								{ Vector3(0, 0, 0), Vector2(0, 0) },
								{ Vector3(0, 0, 1), Vector2(0, 1) },
								{ Vector3(1, 0, 0), Vector2(1, 0) },
								{ Vector3(1, 0, 1), Vector2(1, 1) },
							};
							for (int i=0; i<4;i++)
							{
								v[i].p.x = (v[i].p.x+ x) * m_ocean_tile_size;
								v[i].p.z = (v[i].p.z+ y) * m_ocean_tile_size;
								v[i].p.y = fWaterLevel;
								v[i].p -= vRenderOrig; // shift the render origin
							}

							pEffectFile->setParameter(CEffectFile::k_texCoordOffset, (const float*)&Vector4((float)(((int)x)%8),(float)(((int)y)%8),0.f,0.f));

							pEffectFile->CommitChanges();

							RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, D3DPT_TRIANGLESTRIP, 2, v, sizeof(mesh_vertex_plain));
						}
					}
					pEffectFile->EndPass();
				}
			}
			pEffectFile->end();
		}//if (pEffectFile->begin())

		// restore states
		if(!m_underwater)
			CGlobals::GetEffectManager()->SetCullingMode(false);

		return nOceanTileRendered;
#else
		return 0;
#endif
	}

	int COceanManager::RenderTechnique_FFT_HIGH(SceneState* pSceneState, float x0, float y0, float x1, float y1)
	{
		if(!m_bRenderOceanReflection)
			return 0;
#ifdef USE_DIRECTX_RENDERER
		CBaseCamera * pCamera = CGlobals::GetScene()->GetCurrentCamera();
		LPDIRECT3DDEVICE9 pd3dDevice = pSceneState->m_pd3dDevice;
		Vector3 vRenderOrig =  CGlobals::GetScene()->GetRenderOrigin();
		CEffectFile* pEffectFile = NULL;

		// update FFT if time ticks.
		if(pSceneState->dTimeDelta>0)
			update((float)pSceneState->dTimeDelta);

		// determine how many ocean tiles to draw to fill the camera view,for debugging only
		int nOceanTileRendered = 0;
		bool bUseBumpMap = true;
		CheckLoadOceanTextures(TECH_OCEAN);
		CGlobals::GetEffectManager()->BeginEffect(TECH_OCEAN);
		pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if(pEffectFile==0)
			return 0;
		//////////////////////////////////////
		// set effect params
		bUseBumpMap = m_pRippleBumpMappingTexture!=NULL && m_pRippleBumpMappingTexture->GetTexture()!=NULL;
		//pEffectFile->SetBoolean(0, bUseBumpMap);

		LinearColor OceanColor = CGlobals::GetScene()->GetSunLight().GetSunAmbientHue();
		OceanColor.r*=m_colorOcean.r;
		OceanColor.g*=m_colorOcean.g;
		OceanColor.b*=m_colorOcean.b;
		OceanColor.a = 1.0f;
		pEffectFile->setParameter(CEffectFile::k_ConstVector2, (const float*)OceanColor);
		{
			// direction and speed of the wind. how many U,V per second. Since, time is time % 100; x,y has to be 0.01*n, where n is integer.
			Vector4 vBumpSpeed(-0.02f, 0.f, 0.f,0.f);
			vBumpSpeed.x = ((float)((int)(m_fWindSpeed*m_fWindX/10.f)))*0.01f;
			vBumpSpeed.y = ((float)((int)(m_fWindSpeed*m_fWindY/10.f)))*0.01f;
			pEffectFile->setParameter(CEffectFile::k_ConstVector3, vBumpSpeed.ptr());
		}

		pEffectFile->setParameter(CEffectFile::k_uvScaleOffset, 
			(const float*)&Vector4((1.0f /  m_reflectionMapOverdraw), 
			(1.0f / m_reflectionMapOverdraw),
			.5f * (1.0f - (1.0f / m_reflectionMapOverdraw)),
			.5f * (1.0f - (1.0f / m_reflectionMapOverdraw))));
		if(m_underwater)
		{
			pEffectFile->setParameter(CEffectFile::k_ConstVector1, (const float*)&Vector4(-1.0f, m_fBumpTime, 0.0f, 0.0f));
			pEffectFile->setParameter(CEffectFile::k_fresnelR0, (const float*)&Vector4(0.20f, 0.0f, 0.0f, 0.0f));
			pEffectFile->setParameter(CEffectFile::k_ConstVector0, (const float*)&Vector4(5.0f, 0.65f, 0.65f, 0.0f));
		}
		else
		{
			pEffectFile->setParameter(CEffectFile::k_ConstVector1, (const float*)&Vector4(1.0f, m_fBumpTime, 0.0f, 0.0f));
			pEffectFile->setParameter(CEffectFile::k_fresnelR0, (const float*)&Vector4(0.0977f, 0.0f, 0.0f, 0.0f));
			pEffectFile->setParameter(CEffectFile::k_ConstVector0, (const float*)&Vector4(1.0f, 0.0f, 1.0f, 0.0f));
		}

		/** for under water surface rendering, we will use NONE culling, 
		* for above water surface rendering, we will use CCW culling. */
		if(m_underwater)
			CGlobals::GetEffectManager()->SetCullingMode(false);
		else
			CGlobals::GetEffectManager()->SetCullingMode(true);

		//////////////////////////////////////////////////////////////
		// unlike regular objects, we do not render through the render
		// queue. Instead, we render on-demand using our internal settings
		if (pEffectFile->begin(true, 0))
		{
			// supply our material to the render method
			pEffectFile->applySurfaceMaterial(&pSceneState->GetGlobalMaterial());

			{
				// set the new render origin
				Vector4 vWorldPos(0,0,0,1.f);
				/** this is for height shift, using the render origin. */
				vWorldPos.x = m_fGlobalWaterLevel; // absolute position is in x component
				vWorldPos.y = m_fGlobalWaterLevel-vRenderOrig.y; // render space position is in y component
				pEffectFile->setParameter(
					CEffectFile::k_worldPos,
					&vWorldPos);
			}

			// activate the geometry buffers
			pd3dDevice->SetIndices(m_pIndexBuffer);
			pd3dDevice->SetStreamSource(0, m_pBaseBuffer, 0, sizeof(sBaseVertex));

			int primary_buffer = m_activeBuffer -2;
			int secondary_buffer = m_activeBuffer -1;

			if (primary_buffer <0) primary_buffer += k_total_water_meshes;
			if (secondary_buffer <0) secondary_buffer += k_total_water_meshes;

			pd3dDevice->SetStreamSource(1, m_pAnimBuffer[primary_buffer], 0, sizeof(sAnimVertex));
			pd3dDevice->SetStreamSource(2, m_pAnimBuffer[secondary_buffer], 0, sizeof(sAnimVertex));

			int totalPasses = pEffectFile->totalPasses();

			for (int i=0; i<totalPasses; ++i)
			{
				if(pEffectFile->BeginPass(i))
				{
					pEffectFile->setTexture(0, m_waveReflectionTexture);
					if(bUseBumpMap)
					{
						pEffectFile->setTexture(1, m_pRippleBumpMappingTexture.get());
					}
					if(m_bDrawShoreline && m_bUseShorelineTexture)
					{
						pEffectFile->setTexture(2, m_pShorelineTexture.get());
					}

#ifdef PSVS_NEW
					// texture tex1 : TEXTURE; water lookup table
					pEffectFile->setTexture(1, m_pWaterColorTexture.get());
#elseif ENABLE_FADING
					if(m_pWaterFadeTexture)
						pEffectFile->setTexture(1, m_pWaterFadeTexture.get());
#endif
					nOceanTileRendered = 0;

					for (float y=y0; y<y1; y=y+1.0f)
					{
						for (float x=x0; x<x1; x=x+1.0f)
						{
							/** Cull ocean tiles that are not in the view frustum
							*/
							{
								// use global water level as high as the water level of the current tile.
								float fWaterLevel = m_fGlobalWaterLevel;
								int nTileID = (int)(x*1000+y);
								// only render if occlusion test passed. 
								if(ocean_tile_occlusion_result.find(nTileID) == ocean_tile_occlusion_result.end() || ocean_tile_occlusion_result[nTileID]>0)
								{
#define PT_COUNT	8
									const static Vector3 cubePT[PT_COUNT] = {
										Vector3(0, 0, 0), 
										Vector3(1, 0, 0), 
										Vector3(1, 0, 1), 
										Vector3(0, 0, 1), 
										Vector3(0, 1, 0), 
										Vector3(1, 1, 0), 
										Vector3(1, 1, 1), 
										Vector3(0, 1, 1), 
									};

									Vector3 pt[PT_COUNT];
									for (int i=0; i<PT_COUNT;i++)
									{
										pt[i].x = (cubePT[i].x+ x) * m_ocean_tile_size;
										pt[i].z = (cubePT[i].z+ y) * m_ocean_tile_size;
										pt[i].y = fWaterLevel+(cubePT[i].y-0.5f)/* *m_fWaveHeight */;//? why m_fWaveHeight is 387.f?
										pt[i] -= vRenderOrig; // shift the render origin
									}
									if(pCamera->GetObjectFrustum()->CullPointsWithFrustum(pt, PT_COUNT))
										nOceanTileRendered ++;
									else
										continue;
								}
								else
									continue;
							}

							// set the stream for height field
							if(m_bDrawShoreline)
							{
								int nID = GetTileIDFromXY(int(x), int(y));
								TTerrain* pTerrain = GetTerrainHeightField(nID);
								if(pTerrain!=0)
								{
									// TODO: retrieve from an array to increase parallism between CPU and GPU, here I just use the first one for testing
									LPDIRECT3DVERTEXBUFFER9 pVB = m_pTerrainHeightmapBuffer[0];
									float* pHeightField = NULL;
									if(SUCCEEDED(pVB->Lock( 0, 0, (void**)&pHeightField, D3DLOCK_DISCARD)))
									{
										memcpy(pHeightField, pTerrain->heightfield, pTerrain->GetGridSize()*pTerrain->GetGridSize()*sizeof(float));
										pVB->Unlock();
										pd3dDevice->SetStreamSource(3, pVB, 0, sizeof(float));
									}
								}
							}
							Vector4 scaleOffset(
								m_fInterpolation,
								1.0f/8000.0f,
								x * m_ocean_tile_size-vRenderOrig.x,
								y * m_ocean_tile_size-vRenderOrig.z);

							pEffectFile->setParameter(
								CEffectFile::k_posScaleOffset,
								&scaleOffset);

							pEffectFile->CommitChanges();

							HRESULT hr = RenderDevice::DrawIndexedPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, 
								D3DPT_TRIANGLESTRIP,
								0,
								0,
								m_grid_size*m_grid_size,
								0,
								m_num_triangle);

						}
					}
					pEffectFile->EndPass();
					if(m_bDrawShoreline && pSceneState->m_bCameraMoved)
					{
						FrameMoveCachedTerrainTiles();
					}
				}
			}
			pEffectFile->end();
		}//if (pEffectFile->begin())

		// restore states
		if(!m_underwater)
			CGlobals::GetEffectManager()->SetCullingMode(false);

		return nOceanTileRendered;
#else
		return 0;
#endif
	}


	int COceanManager::RenderTechnique_Cloud( SceneState* pSceneState, float x0, float y0, float x1, float y1 )
	{
		if(!m_bRenderOceanReflection)
			return 0;
#ifdef USE_DIRECTX_RENDERER
		CBaseCamera * pCamera = CGlobals::GetScene()->GetCurrentCamera();
		LPDIRECT3DDEVICE9 pd3dDevice = pSceneState->m_pd3dDevice;
		Vector3 vRenderOrig =  CGlobals::GetScene()->GetRenderOrigin();
		CEffectFile* pEffectFile = NULL;


		// determine how many ocean tiles to draw to fill the camera view,for debugging only
		int nOceanTileRendered = 0;
		bool bUseBumpMap = true;
		CheckLoadOceanTextures(TECH_OCEAN_CLOUD);
		CGlobals::GetEffectManager()->BeginEffect(TECH_OCEAN_CLOUD);
		pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if(pEffectFile==0)
			return 0;
		//////////////////////////////////////
		// set effect params
		
		LinearColor OceanColor = CGlobals::GetScene()->GetSunLight().GetSunAmbientHue();
		OceanColor.r*=m_colorOcean.r;
		OceanColor.g*=m_colorOcean.g;
		OceanColor.b*=m_colorOcean.b;
		OceanColor.a = 1.0f;
		pEffectFile->setParameter(CEffectFile::k_ConstVector2, (const float*)OceanColor);
		{
			// direction and speed of the wind. how many U,V per second. Since, time is time % 100; x,y has to be 0.01*n, where n is integer.
			Vector4 vBumpSpeed(-0.02f, 0.f, 0.f,0.f);
			vBumpSpeed.x = ((float)((int)(m_fWindSpeed*m_fWindX/10.f)))*0.01f;
			vBumpSpeed.y = ((float)((int)(m_fWindSpeed*m_fWindY/10.f)))*0.01f;
			pEffectFile->setParameter(CEffectFile::k_ConstVector3, vBumpSpeed.ptr());
		}

		pEffectFile->setParameter(CEffectFile::k_ConstVector1, Vector4(0.f, m_fBumpTime / 100.f, m_fBumpTime / 100.f, 0.0f).ptr());
		
		/** for under water surface rendering, we will use NONE culling, 
		* for above water surface rendering, we will use CCW culling. */
		if(m_underwater)
			CGlobals::GetEffectManager()->SetCullingMode(false);
		else
			CGlobals::GetEffectManager()->SetCullingMode(true);

		//////////////////////////////////////////////////////////////
		// unlike regular objects, we do not render through the render
		// queue. Instead, we render on-demand using our internal settings
		if (pEffectFile->begin(true, 0))
		{
			// supply our material to the render method
			pEffectFile->applySurfaceMaterial(&pSceneState->GetGlobalMaterial());

			{
				// set the new render origin
				Vector4 vWorldPos(0,0,0,1.f);
				/** this is for height shift, using the render origin. */
				vWorldPos.x = m_fGlobalWaterLevel; // absolute position is in x component
				vWorldPos.y = m_fGlobalWaterLevel-vRenderOrig.y; // render space position is in y component
				pEffectFile->setParameter(
					CEffectFile::k_worldPos,
					&vWorldPos);
			}

			// activate the geometry buffers
			pd3dDevice->SetIndices(m_pIndexBuffer);
			pd3dDevice->SetStreamSource(0, m_pBaseBuffer, 0, sizeof(sBaseVertex));
			
			int totalPasses = pEffectFile->totalPasses();

			for (int i=0; i<totalPasses; ++i)
			{
				if(pEffectFile->BeginPass(i))
				{
					pEffectFile->setTexture(0, m_pSunsetTexture.get());
					pEffectFile->setTexture(1, m_pCloudTexture.get());
					
					nOceanTileRendered = 0;

					for (float y=y0; y<y1; y=y+1.0f)
					{
						for (float x=x0; x<x1; x=x+1.0f)
						{
							/** Cull ocean tiles that are not in the view frustum
							*/
							{
								// use global water level as high as the water level of the current tile.
								float fWaterLevel = m_fGlobalWaterLevel;
								int nTileID = (int)(x*1000+y);
								// only render if occlusion test passed. 
								if(ocean_tile_occlusion_result.find(nTileID) == ocean_tile_occlusion_result.end() || ocean_tile_occlusion_result[nTileID]>0)
								{
#define PT_COUNT	8
									const static Vector3 cubePT[PT_COUNT] = {
										Vector3(0, 0, 0), 
										Vector3(1, 0, 0), 
										Vector3(1, 0, 1), 
										Vector3(0, 0, 1), 
										Vector3(0, 1, 0), 
										Vector3(1, 1, 0), 
										Vector3(1, 1, 1), 
										Vector3(0, 1, 1), 
									};

									Vector3 pt[PT_COUNT];
									for (int i=0; i<PT_COUNT;i++)
									{
										pt[i].x = (cubePT[i].x+ x) * m_ocean_tile_size;
										pt[i].z = (cubePT[i].z+ y) * m_ocean_tile_size;
										pt[i].y = fWaterLevel+(cubePT[i].y-0.5f)/* *m_fWaveHeight */;//? why m_fWaveHeight is 387.f?
										pt[i] -= vRenderOrig; // shift the render origin
									}
									if(pCamera->GetObjectFrustum()->CullPointsWithFrustum(pt, PT_COUNT))
										nOceanTileRendered ++;
									else
										continue;
								}
								else
									continue;
							}

							// set the stream for height field
							if(m_bDrawShoreline)
							{
								int nID = GetTileIDFromXY(int(x), int(y));
								TTerrain* pTerrain = GetTerrainHeightField(nID);
								if(pTerrain!=0)
								{
									// TODO: retrieve from an array to increase parallism between CPU and GPU, here I just use the first one for testing
									LPDIRECT3DVERTEXBUFFER9 pVB = m_pTerrainHeightmapBuffer[0];
									float* pHeightField = NULL;
									if(SUCCEEDED(pVB->Lock( 0, 0, (void**)&pHeightField, D3DLOCK_DISCARD)))
									{
										memcpy(pHeightField, pTerrain->heightfield, pTerrain->GetGridSize()*pTerrain->GetGridSize()*sizeof(float));
										pVB->Unlock();
										pd3dDevice->SetStreamSource(1, pVB, 0, sizeof(float));
									}
								}
							}
							Vector4 scaleOffset(
								m_fInterpolation,
								1.0f/8000.0f,
								x * m_ocean_tile_size-vRenderOrig.x,
								y * m_ocean_tile_size-vRenderOrig.z);

							pEffectFile->setParameter(
								CEffectFile::k_posScaleOffset,
								&scaleOffset);

							pEffectFile->CommitChanges();

							HRESULT hr = RenderDevice::DrawIndexedPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, 
								D3DPT_TRIANGLESTRIP,
								0,
								0,
								m_grid_size*m_grid_size,
								0,
								m_num_triangle);

						}
					}
					pEffectFile->EndPass();
					if(m_bDrawShoreline && pSceneState->m_bCameraMoved)
					{
						FrameMoveCachedTerrainTiles();
					}
				}
			}
			pEffectFile->end();
		}//if (pEffectFile->begin())

		// restore states
		if(!m_underwater)
			CGlobals::GetEffectManager()->SetCullingMode(false);

		return nOceanTileRendered;
#else
		return 0;
#endif
	}

	int COceanManager::RenderTechnique_OccusionTest(SceneState* pSceneState, float x0, float y0, float x1, float y1)
	{
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DDEVICE9 pd3dDevice = pSceneState->m_pd3dDevice;
		Vector3 vRenderOrig =  CGlobals::GetScene()->GetRenderOrigin();
		CEffectFile* pEffectFile = NULL;

		/** render occlusion rough plane, and we will use the visibility of the previous frame to draw the current frame detailed ocean tiles. */
		LatentOcclusionQueryBank* pQueries = CGlobals::GetAssetManager()->GetOcclusionQueryBank(1);
		if(	pQueries != 0 )
		{
			CGlobals::GetEffectManager()->BeginEffect(TECH_OCCLUSION_TEST);
			CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
			if ( pEffectFile != 0)
			{
				// check occlusion query result from the the last render frame. 
				DWORD pixelsVisible = 0;
				int tileID = 0;
				HRESULT hr = E_FAIL;

				while((hr = pQueries->CheckForFirstResult(&pixelsVisible, (void**)(&tileID))) == S_OK)
				{
					ocean_tile_occlusion_result[tileID] = pixelsVisible;
				}

				/*if(pEffectFile->begin(true, 0))
				{
				if(pEffectFile->BeginPass(0))
				{*/
				for (float y=y0; y<y1; y=y+1.0f)
				{
					for (float x=x0; x<x1; x=x+1.0f)
					{
						// use global water level as high as the water level of the current tile.
						float fWaterLevel = m_fGlobalWaterLevel;
						mesh_vertex_plain v[4] = {
							{ Vector3(0, 0, 0), Vector2(0,0)}, 
							{ Vector3(0, 0, 1), Vector2(0, 0) },
							{ Vector3(1, 0, 0), Vector2(0, 0) },
							{ Vector3(1, 0, 1), Vector2(0, 0) },
						};
						for (int i=0; i<4;i++)
						{
							v[i].p.x = (v[i].p.x+ x) * m_ocean_tile_size;
							v[i].p.z = (v[i].p.z+ y) * m_ocean_tile_size;
							v[i].p.y = fWaterLevel;
							v[i].p -= vRenderOrig; // shift the render origin
						}
						int nTileID = (int)(x*1000+y);
						// wait on the result of the first query, to ensure there is always an unused query in the bank.
						HRESULT hr = pQueries->BeginNewQuery((void*)nTileID);

						if(hr == S_OK)
						{
							// render a quad for the ocean tile occlustion testing. 
							RenderDevice::DrawPrimitiveUP(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, D3DPT_TRIANGLESTRIP, 2, v, sizeof(mesh_vertex_plain));
							hr = pQueries->EndNewQuery();
						}
						else
							break;
					}
				}
				/*
				pEffectFile->EndPass();
				}
				pEffectFile->end();
				}*/

				while((hr = pQueries->CheckForFirstResult(&pixelsVisible, (void**)(&tileID))) == S_OK)
				{
					ocean_tile_occlusion_result[tileID] = pixelsVisible;
				}

				// check if none is visible. 
				bool bRenderOceanReflection_lastframe = m_bRenderOceanReflection;
				m_bRenderOceanReflection = false;
				for (float y=y0; y<y1; y=y+1.0f)
				{
					for (float x=x0; x<x1; x=x+1.0f)
					{
						int nTileID = (int)(x*1000+y);
						if(ocean_tile_occlusion_result.find(nTileID) == ocean_tile_occlusion_result.end() || ocean_tile_occlusion_result[nTileID]>0)
						{
							m_bRenderOceanReflection = true;
							break;
						}
					}
				}
				if(!bRenderOceanReflection_lastframe && m_bRenderOceanReflection)
					ForceUpdateOcean();
			}
		}
		else
		{
			ocean_tile_occlusion_result.clear();
			m_bRenderOceanReflection = true;
		}
#endif
		return S_OK;
	}

	void COceanManager::RenderUnderwaterEffect(SceneState* pSceneState)
	{
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DDEVICE9 pd3dDevice = pSceneState->m_pd3dDevice;
		Vector3 vRenderOrig =  CGlobals::GetScene()->GetRenderOrigin();
		CEffectFile* pEffectFile = NULL;

		/** for under water effect, we will 
		* - blend the ocean color to the entire screen.
		* - display an animated texture in front of the screen. The texture may contain some bubbles, etc.
		* - blur the image with post-processing. Maybe need to copy back buffer using StrechRect, then apply a blur type pixel shader. 
		* - using fog or vertex based fog, etc.*/
		if(m_underwater)
		{
			//////////////////////////////////////////////////////////////////////////
			//
			// Render under water effect
			//
			//////////////////////////////////////////////////////////////////////////
			CGlobals::GetEffectManager()->BeginEffect(TECH_OCEAN_UNDERWATER);

			/** draw a blue square in front of the screen. */
			pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
			if(pEffectFile == 0)
			{
				//////////////////////////////////////////////////////////////////////////
				// fixed programming pipeline
				D3DVIEWPORT9 curViewport;
				CGlobals::GetRenderDevice()->GetViewport(&curViewport);
				FLOAT sx = (FLOAT)curViewport.Width;
				FLOAT sy = (FLOAT)curViewport.Height;

				UNDERWATER_VERTEX v[4];
				v[0].p = Vector4(  0, sy, 0.0f, 1.0f );
				v[1].p = Vector4(  0,  0, 0.0f, 1.0f );
				v[2].p = Vector4( sx, sy, 0.0f, 1.0f );
				v[3].p = Vector4( sx,  0, 0.0f, 1.0f );

				LinearColor underwaterColor = CGlobals::GetScene()->GetFogColor();
				// make it a little blue than the fog color.
				underwaterColor.r *= 0.8f*m_colorOcean.r;
				underwaterColor.g *= m_colorOcean.g;
				underwaterColor.b *= 1.1f*m_colorOcean.b;
				underwaterColor.a = 0.6f;
				DWORD dwColor = underwaterColor;

				for(int i=0;i<4;i++)
				{
					v[i].color = dwColor;
				}

				// Set render states (disable z-buffering, enable stencil, disable fog, and
				// turn on alpha blending)

				RenderDevice::DrawPrimitiveUP( pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_MESH,  D3DPT_TRIANGLESTRIP, 2, v, sizeof(UNDERWATER_VERTEX) );
			}
		}
#endif
	}

	void COceanManager::CalculateTileToRender(SceneState* pSceneState, float& x0, float& y0, float& x1, float& y1)
	{
		x0 = 0;
		x1 = 0;
		y0 = 0;
		y1 = 0;
		CBaseCamera * pCamera = CGlobals::GetScene()->GetCurrentCamera();
		Vector3 vEye = pCamera->GetEyePosition();
		
#ifdef TILED_OCEAN
		/** static tiled ocean. 
		* There may be too many triangles to draw and is not efficient without LOD.
		* but it has no visual flaws as in the camera fixed ocean tile method */
		if(pCamera)
		{
			//TODO: this is wrong. 
			Vector3 vCenter = vEye;
			// the ocean radius should be so big to include the fog range. 
			// 50 is used as half of m_ocean_tile_size
			float fRadius = (float)((int)(CGlobals::GetScene()->GetFogEnd()/50)*50);
			fRadius = max(m_ocean_tile_size, fRadius);

			// this ensure there are maximum of (MAX_OCEAN_BLOCK_COUNT+1)^2 ocean blocks.
			fRadius = min(m_ocean_tile_size*MAX_OCEAN_BLOCK_COUNT-1.f, fRadius); 
			// align the world rect to our ocean tile grid
			x0 = floorf((vCenter.x-fRadius)/m_ocean_tile_size);
			x1 = ceilf((vCenter.x+fRadius)/m_ocean_tile_size);
			y0 = floorf((vCenter.z-fRadius)/m_ocean_tile_size);
			y1 = ceilf((vCenter.z+fRadius)/m_ocean_tile_size);
		}
#else
		/** camera fixed ocean tile:
		* it only draws a single tile that moves with the camera. It is fast, yet it has visual flaws when the camera moves 
		* The ocean appears to move with the camera */
		if(pCamera)
		{
			Vector3 vCenter = vEye;
			x0 = vCenter.x/m_ocean_tile_size-0.5f;
			x1 = x0+1.0f;
			y0 = vCenter.z/m_ocean_tile_size-0.5f;
			y1 = y0+1.0f;
		}
#endif
	}

	void COceanManager::Render(SceneState* pSceneState)
	{
		if(!m_bIsCreated)
			return;
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DDEVICE9 pd3dDevice = pSceneState->m_pd3dDevice;
		PERF1("ocean_render");

		/** check whether the camera eye is under the water surface or not*/
		CBaseCamera * pCamera = CGlobals::GetScene()->GetCurrentCamera();
		Vector3 vEye = pCamera->GetEyePosition();

		BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
		m_underwater = pBlockWorldClient->IsPointUnderWater(vEye);
		
		//
		// Render ocean surface
		//
		CEffectFile* pEffectFile = NULL;
		if(OceanEnabled())
		{
			if(m_dwTechnique == OCEAN_TECH_SIMPLE)
				pEffectFile = CGlobals::GetEffectManager()->CheckLoadEffect(TECH_OCEAN_SIMPLE);
			else if(m_dwTechnique == OCEAN_TECH_CLOUD)
				pEffectFile = CGlobals::GetEffectManager()->CheckLoadEffect(TECH_OCEAN_CLOUD);
			else if(m_dwTechnique != OCEAN_TECH_QUAD)
				pEffectFile = CGlobals::GetEffectManager()->CheckLoadEffect(TECH_OCEAN);

			if(pEffectFile == 0)
			{
				if(m_dwTechnique == OCEAN_TECH_CLOUD)
				{
					return;
				}
				// fixed programming pipeline
				CGlobals::GetEffectManager()->BeginEffect(TECH_OCEAN);
				RenderTechnique_Quad();
			}
			else
			{
				// Render using effect file
				float x0, y0, x1, y1;
				// get tiles to render
				CalculateTileToRender(pSceneState, x0, y0, x1, y1);

				// occlusion testing first
				RenderTechnique_OccusionTest(pSceneState, x0, y0, x1, y1);

				// Render FFT ocean here 
				int nOceanTileRendered = 0;

				// render with FFT high shader
				if(m_dwTechnique == OCEAN_TECH_SIMPLE)
					nOceanTileRendered = RenderTechnique_Simple(pSceneState, x0, y0, x1, y1);
				else if(m_dwTechnique == OCEAN_TECH_CLOUD)
					nOceanTileRendered = RenderTechnique_Cloud(pSceneState, x0, y0, x1, y1);
				else
					nOceanTileRendered = RenderTechnique_FFT_HIGH(pSceneState, x0, y0, x1, y1);

				if(CGlobals::WillGenReport())
					CGlobals::GetReport()->SetValue("ocean tile", (m_bRenderOceanReflection)?nOceanTileRendered:-nOceanTileRendered);// if negative value, it means that occlusion testing is not passed.
			}

			// render ripples if any
			if(m_dwTechnique != OCEAN_TECH_CLOUD)
			{
				RenderRipples((float)pSceneState->dTimeDelta);

				// render under water effect if camera is under water
				if(m_useScreenSpaceFog)
					RenderUnderwaterEffect(pSceneState);
			}
		}
		else if(pBlockWorldClient->IsInBlockWorld())
		{
			RenderRipples((float)pSceneState->dTimeDelta);

			// render under water effect if camera is under water
			if(m_useScreenSpaceFog)
				RenderUnderwaterEffect(pSceneState);
		}
#endif
	}

	inline float COceanManager::getOceanHeight( int x, int y )
	{
		int index = getOffsetWrap(x, y);
		return(-m_colH[index].real);
	}

	inline float COceanManager::getOceanNormalX( int x, int y )
	{
		int index = getOffsetWrap(x, y);
		return( m_colN[index].real );
	}

	inline float COceanManager::getOceanNormalY( int x, int y )
	{
		int index = getOffsetWrap(x, y);
		return( m_colN[index].imag );
	}

	inline float COceanManager::getKx( int x )
	{
		return (TWO_PI * x );
	}

	inline float COceanManager::getKy( int y )
	{
		return (TWO_PI * y );
	}

	inline float COceanManager::getAngularFrequency(float fKLength)
	{
		// special version for deep water
		return( sqrtf( GRAVITY_CONSTANT * fKLength ) );

		// Note: If we wanted to take the water depth into account, we could
		// use the slower equation:
		// sqrtf( GRAVITY_CONSTANT * fKLength * tanhf( fKLength * depth ) )
	}

	inline int COceanManager::getIndexH0( int x, int y )
	{
		return( x + m_half_grid_size + ( m_grid_size + 1 ) * ( y + m_half_grid_size) );
	}

	inline int COceanManager::getIndex( int x, int y )
	{
		return( x + ( m_half_grid_size ) + m_grid_size * ( y + m_half_grid_size) );
	}

	inline int COceanManager::getIndexFFT( int x, int y )
	{
		// get the appropriate index to store data in the right order for the 2d fft
		// (that is, dc is stored at (0, 0) and not at ( m_half_grid_size, m_half_grid_size ) !!!)
		return( ( x & ( m_grid_size - 1 ) ) + m_grid_size * ( y & ( m_grid_size - 1 ) ) );
	}

	inline int COceanManager::getOffset( int x, int y )
	{
		return( x + m_grid_size * y );
	}

	inline int COceanManager::getOffsetWrap( int x, int y )
	{
		return( getOffset( x & ( m_grid_size - 1 ), y & ( m_grid_size - 1 ) ) );
	}

	//-----------------------------------------------------------------------------
	// 
	// Render the scene into the reflection texture
	//
	//-----------------------------------------------------------------------------
	bool  COceanManager::RenderReflectionTexture(void)
	{
		if(!m_bRenderOceanReflection)
			return false;

#ifdef USE_DIRECTX_RENDERER
		if( GetRenderTechnique() != OCEAN_TECH_FULL)
			return false;
		m_fLastReflectionTime = 0.f;
		PERF1("render_reflection");

		LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		CSceneObject* pScene = CGlobals::GetScene();

		Plane clipPlane;
		Plane transformedClipPlane;
		float     reflectionMapFieldOfView;

		//////////////////////////////////////////////////////////////////////////
		// Render to the reflection map
		LPDIRECT3DSURFACE9 pOldRenderTarget =  CGlobals::GetDirectXEngine().GetRenderTarget();
		CGlobals::GetDirectXEngine().SetRenderTarget(0, m_waveReflectionSurface);

		// set depth surface
		LPDIRECT3DSURFACE9 pOldZBuffer = NULL;
		if(FAILED(pd3dDevice->GetDepthStencilSurface(&pOldZBuffer)))
		{
			OUTPUT_LOG("GetDepthStencilSurface failed\r\n");
			return false;
		}
		pd3dDevice->SetDepthStencilSurface( m_pDepthStencilSurface );

		// Compute the field of view and use it
		CBaseCamera* pCamera = pScene->GetCurrentCamera();
		reflectionMapFieldOfView = 2.0f * atan((tan(pCamera->GetFieldOfView() / 2.0f)) * m_reflectionMapOverdraw);
		ParaMatrixPerspectiveFovLH(&m_reflectionProjectionMatrix, reflectionMapFieldOfView, pCamera->GetAspectRatio(), pCamera->GetNearPlane(), pCamera->GetFarPlane());

		CGlobals::GetProjectionMatrixStack().push(m_reflectionProjectionMatrix);
		m_reflectViewMatrix = true;
		SetMatrices(true, false);
		if(m_bDisableFogInReflection)
			pEffectManager->EnableFog(false);

		pCamera->UpdateFrustum();

		// Set up the clip plane to draw the correct geometry
		if(m_underwater)
		{
			clipPlane = m_reflectionClipPlaneBelowWater;
			// correct coordinate system
			Vector3 vRenderOrig =  pScene->GetRenderOrigin();
			clipPlane.d += (m_fGlobalWaterLevel - vRenderOrig.y);

			if(!m_useScreenSpaceFog)
			{
				pScene->ApplyWaterFogParam();
			}
		}
		else
		{
			clipPlane = m_reflectionClipPlaneAboveWater;
			// correct coordinate system
			Vector3 vRenderOrig =  pScene->GetRenderOrigin();
			clipPlane.d -= (m_fGlobalWaterLevel - vRenderOrig.y);
		}
		

		// we will render SKYBOX and TERRAIN using fixed function pipeline. So this is always false.
		// in case we use a programmable pipeline for skybox and terrain, set the transformed clipping plane to clip space.
		// When the programmable pipeline is used the plane equations are assumed to be in the clipping space 
		// transform the plane into view space
		Matrix4 tempMatrix = CGlobals::GetViewMatrixStack().SafeGetTop();
		tempMatrix *= CGlobals::GetProjectionMatrixStack().SafeGetTop();
		// For some reason, DX requires that the inverse and transpose matrix is used. 
		tempMatrix = tempMatrix.inverse();
		tempMatrix = tempMatrix.transpose();
		transformedClipPlane = clipPlane.PlaneTransform(tempMatrix);

		
		pEffectManager->SetClipPlane(0, (const float*)&clipPlane, false);
		pEffectManager->SetClipPlane(0, (const float*)&transformedClipPlane, true);
		pEffectManager->EnableClipPlane(true);

		// D3DCLEAR_ZBUFFER is doomed. Hence, reflection must be rendered before the main thing is rendered. 
		pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
			0x00ffffff&(DWORD)(pScene->IsSceneEnabled()? pScene->GetFogColor(): pScene->GetBackGroundColor()), 1.0f, 0L);

		//////////////////////////////////////////////////////////////////////////
		//
		// Render the actual scene: 
		//
		//////////////////////////////////////////////////////////////////////////

		/*bool bUseReflection = pEffectManager->IsReflectionRenderingEnabled();
		if(bUseReflection)
			pEffectManager->EnableReflectionRendering(false);*/

		// reverse cull mode, since we made the reflection. This should be done for both fixed function and programmable pipeline.
		//CGlobals::GetSceneState()->m_dwD3D_CULLMODE = D3DCULL_CW; 
		// TODO: maybe first rendering terrain then rendering sky box will reduce fill rate ?
		//pd3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
		if(m_ReflectionItems[R_SKY_BOX])
			pScene->RenderSelection(RENDER_SKY_BOX);
		pEffectManager->EnableClipPlane(true);
		if(m_ReflectionItems[R_GLOBAL_TERRAIN])
		{
			if(CGlobals::GetGlobalTerrain()->TerrainRenderingEnabled())
			{
				// since the camera settings is changed, we need to update the terrain buffer before drawing
				CGlobals::GetGlobalTerrain()->Update(); // quite CPU time consuming.
				if(pScene->IsShadowMapEnabled())
				{
					pScene->SetShadow(false);
					pScene->RenderSelection(RENDER_GLOBAL_TERRAIN);
					pScene->SetShadow(true);
				}
				else
				{
					pScene->RenderSelection(RENDER_GLOBAL_TERRAIN);
				}
			}
		}
		
		if(m_ReflectionItems[R_CHARACTERS])
		{
			pScene->RenderSelection(RENDER_CHARACTERS);
		}
		else if(m_ReflectionItems[R_CURRENT_PLAYER])
		{
			pScene->RenderSelection(RENDER_PLAYER);
			/** TODO: only render when it is above or below water surface. */
			/*
			CBipedObject * pObj = pScene->GetCurrentPlayer();
			if(pObj)
			{
			Vector3 vPos;
			pObj->GetPosition(&vPos);
			if( (m_underwater && GetMaxWaterLevel()>(vPos.y-2.0f)) || 
			(!m_underwater && GetMaxWaterLevel()<vPos.y) )
			{
			pScene->RenderSelection(CSceneObject::RENDER_PLAYER);
			}
			}
			*/
		}
		
		if(m_ReflectionItems[R_MESH_FRONT_TO_BACK])
		{
			/** TODO: only render when it is above or below water surface. */
			pScene->RenderSelection(RENDER_MESH_FRONT_TO_BACK);
		}

		if(m_ReflectionItems[R_MESH_BACK_TO_FRONT])
		{
			/** TODO: only render when it is above or below water surface. */
			// TODO: WHY not render smaller mesh ? pScene->RenderSelection(CSceneObject::RENDER_MESH_BACK_TO_FRONT);
			pScene->RenderSelection(RENDER_MESH_BACK_TO_FRONT);
		}

		if(m_ReflectionItems[R_MESH_TRANSPARENT])
		{
			pScene->RenderSelection(RENDER_MESH_TRANSPARENT);
		}
		
		if(!CGlobals::GetSceneState()->GetFaceGroups()->IsEmpty())
		{
			// NOT TESTED: translucent face groups.
			if(m_ReflectionItems[R_MESH_TRANSPARENT])
			{
				pScene->RenderSelection(RENDER_TRANSLUCENT_FACE_GROUPS);
			}
			CGlobals::GetSceneState()->GetFaceGroups()->Clear();
		}

		//////////////////////////////////////////////////////////////////////////
		// Restore state for the normal pipeline
		pEffectManager->EnableClipPlane(false);
		if(m_bDisableFogInReflection)
			pEffectManager->EnableFog(pScene->IsFogEnabled());
		//CGlobals::GetSceneState()->m_dwD3D_CULLMODE = D3DCULL_CCW;
		CGlobals::GetProjectionMatrixStack().pop();
		m_reflectViewMatrix = false;

		SetMatrices(false, true);
		pCamera->UpdateFrustum();

		// restore old depth surface
		pd3dDevice->SetDepthStencilSurface( pOldZBuffer);
		SAFE_RELEASE(pOldZBuffer);

		//////////////////////////////////////////////////////////////////////////
		// Restore the old render target: i.e. the backbuffer
		HRESULT hr = CGlobals::GetDirectXEngine().SetRenderTarget(0, pOldRenderTarget);
		/*if(bUseReflection)
			pEffectManager->EnableReflectionRendering(true);*/
		PE_ASSERT(hr == D3D_OK);
#endif
		return true;

	}
	//-----------------------------------------------------------------------------
	// 
	// Setup up the transforms for fixed function.
	// Setup constants for vertex shaders.        
	//
	//-----------------------------------------------------------------------------
	void COceanManager::SetMatrices(bool bPostPushMatrices, bool bPrePopMatrices)
	{
#ifdef USE_DIRECTX_RENDERER
		// pop matrices
		if(bPrePopMatrices)
		{
			CGlobals::GetViewMatrixStack().pop();
		}

		LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
		Matrix4 worldViewMatrix;
		Matrix4 worldInverseTransposeMatrix;
		Matrix4 viewMatrix;
		Matrix4 viewProjectionMatrix;
		Matrix4 worldViewProjectionMatrix;

		// Reflect the view matrix if asked
		if(m_reflectViewMatrix)
		{
			/*Matrix4 reflectionMatrix;

			Vector3 vRenderOrig =  CGlobals::GetScene()->GetRenderOrigin();

			D3DXMatrixReflect(&reflectionMatrix, &Plane(0.0f, 1.0f, 0.0f, -(m_fGlobalWaterLevel-vRenderOrig.y)));

			viewMatrix = reflectionMatrix * CGlobals::GetViewMatrixStack().SafeGetTop();*/

			// put the camera at the opposite side of the water plane. 
			// TODO: currently it only works when the original camera up axis is (0,1,0)
			// the reflection map should be flipped in x axis in image space.We will do this in shader.
			CAutoCamera* pCamera = (CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera());
			Vector3 vUp(0,-1,0);
			DVector3 vEye = pCamera->GetEyePosition();
			vEye.y = m_fGlobalWaterLevel*2-vEye.y;
			DVector3 vLookAt = pCamera->GetLookAtPosition();
			vLookAt.y = m_fGlobalWaterLevel*2-vLookAt.y;
			
			pCamera->ComputeViewMatrix( &viewMatrix, &vEye, &vLookAt, &vUp );
		}
		else
		{
			viewMatrix = CGlobals::GetViewMatrixStack().SafeGetTop();
		}

		// push matrices
		if(bPostPushMatrices)
		{
			CGlobals::GetViewMatrixStack().push(viewMatrix);
		}

		CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true,true, true);
#endif
	}

	int COceanManager::GetTileIDFromXY(int x, int y)
	{
		if(x>=0)
		{
			int r = x+abs(y)+1;
			return r*r*4+r+y;
		}
		else
		{
			int r = abs(x)-y+1;
			return r*r*4+3*r+y;
		}
	}

	bool COceanManager::GetXYFromTileID(int nTileID, int* X, int* Y)
	{
		int x,y;
		if(nTileID<0)
			return false;
		int nBase = (int)sqrt((float)nTileID); // 2*r
		int nLeft = nTileID - nBase*nBase;
		int r = nBase/2;
		if(nLeft<nBase)
		{
			y = nLeft - r;
			x = r-abs(y)-1;
		}
		else
		{
			y = nLeft - 3*r;
			x = -(r-abs(y)-1);
		}
		(*X) = x;
		(*Y) = y;
		return true;
	}

	bool COceanManager::IsDrawShoreLine()
	{
		return m_bDrawShoreline;
	}

	void COceanManager::DrawShoreLine(bool bDraw)
	{
		m_bDrawShoreline = bDraw;
	}
	void COceanManager::AddRipple(const Vector3& vPos)
	{
		if(((int)m_rippleList.m_ripples.size())>=m_nMaxRippleNumber)
		{
			m_rippleList.m_ripples.erase(m_rippleList.m_ripples.begin());
		}
		RippleItem item;
		item.vPos = vPos;
		item.fSpeed = DEFAULT_RIPPLE_SPEED;
		item.fMaxRadius = DEFAULT_RIPPLE_MAX_RADIUS;
		item.fCurrentRadius = 0.1f;
		m_rippleList.m_ripples.push_back(item);
	}
	void COceanManager::RenderRipples(float fTimeDelta)
	{
#ifdef USE_DIRECTX_RENDERER
		m_rippleList.FrameMove(fTimeDelta);
		int nTotalVertices = (int)m_rippleList.m_ripples.size()*6; 

		// TODO: Render ripples
		if(nTotalVertices==0)
			return;

		CGlobals::GetEffectManager()->BeginEffect(TECH_WATER_RIPPLE);
		CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if(pEffectFile==0)
			return;
		
		LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
		EffectManager* pEffectManager = CGlobals::GetEffectManager();

		DynamicVertexBufferEntity* pBufEntity =  CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_TEX1_DIF);
		pd3dDevice->SetStreamSource( 0, pBufEntity->GetBuffer(), 0, pBufEntity->m_nUnitSize );
		
		Vector3 vRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();

		list<RippleItem>::const_iterator iterCur=m_rippleList.m_ripples.begin(), itEnd = m_rippleList.m_ripples.end();

		SPRITEVERTEX* vb_vertices;
		int nNumLockedVertice;
		int nNumFinishedVertice = 0;
		// the first triangle is 0, the second is 1.
		int nTriID = 0;
		// because we still uses Z-buffer, we will offset the ripple a little bit to avoid intersection with the wave.
#define WAVE_RIPPLE_OFFSET	0.15f
		float fHeightOffset = m_underwater ? -WAVE_RIPPLE_OFFSET: WAVE_RIPPLE_OFFSET;
		while(iterCur!=itEnd)
		{
			if( (nNumLockedVertice = pBufEntity->Lock((nTotalVertices - nNumFinishedVertice), 
				(void**)(&vb_vertices))) > 0)
			{
				int nLockedNum = nNumLockedVertice/3;
				
				for(int i=0;i<nLockedNum;++i)
				{
					const RippleItem& item = *iterCur;
					float fRadius = item.fCurrentRadius;
					float fRatio = fRadius/item.fMaxRadius;
					DWORD nColor = LinearColor(1,1,1,1-pow(fRadius/item.fMaxRadius,2));
					int nVB = 3*i;
					Vector3 vPos = item.vPos - vRenderOrigin;
					vPos.y+=fHeightOffset;
					
					if(nTriID==0)
					{ // first triangle in the quad particle

						vb_vertices[nVB].p = vPos + Vector3(fRadius,0,fRadius);
						vb_vertices[nVB].tu = 1.f;
						vb_vertices[nVB].tv = 1.f;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = vPos + Vector3(-fRadius,0,-fRadius);
						vb_vertices[nVB].tu = 0;
						vb_vertices[nVB].tv = 0;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = vPos + Vector3(fRadius,0,-fRadius);
						vb_vertices[nVB].tu = 1.f;
						vb_vertices[nVB].tv = 0;
						vb_vertices[nVB].color = nColor;
					}
					else
					{
						vb_vertices[nVB].p = vPos + Vector3(fRadius,0,fRadius);
						vb_vertices[nVB].tu = 1.f;
						vb_vertices[nVB].tv = 1.f;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = vPos + Vector3(-fRadius,0,fRadius);
						vb_vertices[nVB].tu = 0;
						vb_vertices[nVB].tv = 1.f;
						vb_vertices[nVB].color = nColor;
						++nVB;
						vb_vertices[nVB].p = vPos + Vector3(-fRadius,0,-fRadius);
						vb_vertices[nVB].tu = 0;
						vb_vertices[nVB].tv = 0;
						vb_vertices[nVB].color = nColor;
					}
					if(++nTriID >= 2)
					{
						nTriID = 0;
						iterCur++;
					}
				}
				pBufEntity->Unlock();

				int totalPasses = pEffectFile->totalPasses();

				if (pEffectFile->begin(true, 0))
				{
					for (int i=0; i<totalPasses; ++i)
					{
						if(pEffectFile->BeginPass(i))
						{
							pEffectFile->setTexture(0, m_pRippleTexture.get());

							RenderDevice::DrawPrimitive( pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLELIST,pBufEntity->m_dwBase,nLockedNum);
							pEffectFile->EndPass();
						}
					}
					pEffectFile->end();
				}
				
				if((nTotalVertices - nNumFinishedVertice) > nNumLockedVertice)
					nNumFinishedVertice += nNumLockedVertice;
				else
					break;
			}
			else 
				break;
		}
#endif
	}

	float COceanManager::GetWindSpeed()
	{
		return m_fWindSpeed;
	}

	void COceanManager::SetWindSpeed(float fSpeed)
	{
		m_fWindSpeed = fSpeed;
	}

	float COceanManager::GetWindDirection()
	{
		if(fabs(m_fWindX)<0.001f)
			return 3.14f;
		else
		{
			return (float)atan(m_fWindY/m_fWindX);
		}
	}
	
	void COceanManager::SetWindDirection(float fWindDirection)
	{
		m_fWindX                    = cos( fWindDirection );
		m_fWindY                    = sin( fWindDirection );
	}


	int COceanManager::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		IAttributeFields::InstallFields(pClass, bOverride);

		pClass->AddField("DrawOcean", FieldType_Bool, (void*)EnableOcean_s, (void*)OceanEnabled_s, NULL, NULL, bOverride);
		pClass->AddField("WaterLevel", FieldType_Float, (void*)SetWaterLevel_s, (void*)GetWaterLevel_s, NULL, NULL, bOverride);
		pClass->AddField("OceanColor", FieldType_Vector3, (void*)SetGetOceanColor_s, (void*)GetOceanColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), "", bOverride);
		pClass->AddField("UnderWater", FieldType_Bool, NULL, (void*)IsUnderWater_s, NULL, NULL, bOverride);
		pClass->AddField("WindSpeed", FieldType_Float, (void*)SetWindSpeed_s, (void*)GetWindSpeed_s, NULL, NULL, bOverride);
		pClass->AddField("WindDirection", FieldType_Float, (void*)SetWindDirection_s, (void*)GetWindDirection_s, NULL, NULL, bOverride);
		pClass->AddField("RenderTechnique", FieldType_Int, (void*)SetRenderTechnique_s, (void*)GetRenderTechnique_s, NULL, NULL, bOverride);

		pClass->AddField("EnableTerrainReflection", FieldType_Bool, (void*)EnableTerrainReflection_s, (void*)IsTerrainReflectionEnabled_s, NULL, NULL, bOverride);
		pClass->AddField("EnableMeshReflection", FieldType_Bool, (void*)EnableMeshReflection_s, (void*)IsMeshReflectionEnabled_s, NULL, NULL, bOverride);
		pClass->AddField("EnablePlayerReflection", FieldType_Bool, (void*)EnablePlayerReflection_s, (void*)IsPlayerReflectionEnabled_s, NULL, NULL, bOverride);
		pClass->AddField("EnableCharacterReflection", FieldType_Bool, (void*)EnableCharacterReflection_s, (void*)IsCharacterReflectionEnabled_s, NULL, NULL, bOverride);
		pClass->AddField("IsAnimateFFT", FieldType_Bool, (void*)SetAnimateFFT_s, (void*)IsAnimateFFT_s, NULL, NULL, bOverride);
		pClass->AddField("CleanupTerrainCache", FieldType_void, (void*)CleanupTerrainCache_s, NULL, NULL, NULL, bOverride);
		return S_OK;
	}

	void COceanManager::EnableReflectionItem( ReflectionItem item, bool bEnabled )
	{
		if (item>=0 && item<R_LAST_ITEM)
		{
			m_ReflectionItems[item] = bEnabled;
		}
	}

	bool COceanManager::IsReflectionItemEnabled( ReflectionItem item )
	{
		if (item>=0 && item<R_LAST_ITEM)
		{
			return m_ReflectionItems[item];
		}
		return false;
	}

	bool COceanManager::IsRedrawStillReflection()
	{
		return m_ReflectionItems[R_CHARACTERS] || m_ReflectionItems[R_CURRENT_PLAYER];
	}

	void COceanManager::ForceUpdateOcean()
	{
		m_fLastReflectionTime = MAX_WATER_REFLECTION_INTERVAL;
	}

	bool COceanManager::UpdateReflectionTexture(SceneState & sceneState)
	{
#ifdef USE_DIRECTX_RENDERER
		bool bSimulateOcean = !(sceneState.m_bSkipOcean) && OceanEnabled() && 
			(IsPointUnderWater(sceneState.vEye) /* draw if underwater*/ ||  
			!CGlobals::GetGlobalTerrain()->TerrainRenderingEnabled() || /* draw ocean if terrain is not rendered. */
			GetMaxWaterLevel()>CGlobals::GetGlobalTerrain()->GetLowestVisibleHeight() /* draw if water is above lowest land*/ ||
			CGlobals::GetGlobalTerrain()->GetLowestVisibleHeight() == CGlobals::GetGlobalTerrain()->g_fTerrainInvisibleHeight /* draw if land is completely invisible */);

		if(bSimulateOcean && GetRenderTechnique() >= COceanManager::OCEAN_TECH_REFLECTION)
		{
			if(sceneState.m_bCameraMoved || IsRedrawStillReflection())
			{
				RenderReflectionTexture();
			}
			else
			{
				if(m_fLastReflectionTime!=0)
				{
					RenderReflectionTexture();
				}
			}
		}
#endif
		return true;
	}

	bool COceanManager::IsAnimateFFT()
	{
		return m_bAnimateFFT;
	}

	void COceanManager::SetAnimateFFT( bool bEnable )
	{
		m_bAnimateFFT = bEnable;
	}
}

