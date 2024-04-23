#pragma once
#include "BlockWorld.h"
#include "effect_file.h"

namespace ParaEngine
{
	class BlockRegion;
	class RenderableChunk;
	class BlockRenderTask;
	struct BlockHeightValue;
	class CShadowMap;
	class CMultiFrameBlockWorldRenderer;

	/** this is a singleton client side block world instance. It handles rendering in addition to CBlockWorld*/
	class BlockWorldClient : public CBlockWorld
	{
	public:
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_BlockWorldClient;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "BlockWorldClient"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char * sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		//ATTRIBUTE_METHOD1(BlockWorldClient, GetTimeOfDay_s, float*)	{*p1 = cls->GetTimeOfDay(); return S_OK;}
		//ATTRIBUTE_METHOD1(BlockWorldClient, SetTimeOfDay_s, float)	{cls->SetTimeOfDay(p1); return S_OK;}

		
		ATTRIBUTE_METHOD1(BlockWorldClient, GetBlockLightColor_s, Vector3*) {*p1 = cls->GetBlockLightColor().ToVector3(); return S_OK;}
		ATTRIBUTE_METHOD1(BlockWorldClient, SetBlockLightColor_s, Vector3) {LinearColor c(p1.x, p1.y,p1.z,1); cls->SetBlockLightColor(c); return S_OK;}

		ATTRIBUTE_METHOD1(BlockWorldClient, SetPostProcessingScript_s, const char*)	{cls->SetPostProcessingScript(p1); return S_OK;}

		ATTRIBUTE_METHOD1(BlockWorldClient, SetPostProcessingAlphaScript_s, const char*)	{ cls->SetPostProcessingAlphaScript(p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockWorldClient, GetUseSunlightShadowMap_s, bool*)		{*p1 = cls->GetUseSunlightShadowMap(); return S_OK;}
		ATTRIBUTE_METHOD1(BlockWorldClient, SetUseSunlightShadowMap_s, bool)	{cls->SetUseSunlightShadowMap((BlockRenderMethod)p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(BlockWorldClient, HasSunlightShadowMap_s, bool*)		{ *p1 = cls->HasSunlightShadowMap(); return S_OK; }


		ATTRIBUTE_METHOD1(BlockWorldClient, GetUseWaterReflection_s, bool*)		{*p1 = cls->GetUseWaterReflection(); return S_OK;}
		ATTRIBUTE_METHOD1(BlockWorldClient, SetUseWaterReflection_s, bool)	{cls->SetUseWaterReflection((BlockRenderMethod)p1); return S_OK;}

		ATTRIBUTE_METHOD1(BlockWorldClient, CanUseAdvancedShading_s, bool*)		{*p1 = cls->CanUseAdvancedShading(); return S_OK;}
		
		ATTRIBUTE_METHOD1(BlockWorldClient, IsMovieOutputMode_s, bool*)		{ *p1 = cls->IsMovieOutputMode(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockWorldClient, EnableMovieOutputMode_s, bool)	{ cls->EnableMovieOutputMode((BlockRenderMethod)p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockWorldClient, GetVertexBufferSizeLimit_s, int*)		{ *p1 = cls->GetVertexBufferSizeLimit(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockWorldClient, SetVertexBufferSizeLimit_s, int)	{ cls->SetVertexBufferSizeLimit(p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockWorldClient, IsAsyncChunkMode_s, bool*)		{ *p1 = cls->IsAsyncChunkMode(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockWorldClient, SetAsyncChunkMode_s, bool)	{ cls->SetAsyncChunkMode(p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockWorldClient, GetAlwaysInVertexBufferChunkRadius_s, int*)		{ *p1 = cls->GetAlwaysInVertexBufferChunkRadius(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockWorldClient, SetAlwaysInVertexBufferChunkRadius_s, int)	{ cls->SetAlwaysInVertexBufferChunkRadius(p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockWorldClient, GetMaxVisibleVertexBufferBytes_s, int*)		{ *p1 = cls->GetMaxVisibleVertexBufferBytes(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockWorldClient, SetMaxVisibleVertexBufferBytes_s, int)	{ cls->SetMaxVisibleVertexBufferBytes(p1); return S_OK; }


		ATTRIBUTE_METHOD1(BlockWorldClient, GetNearCameraChunkDist_s, int*)		{ *p1 = cls->GetNearCameraChunkDist(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockWorldClient, SetNearCameraChunkDist_s, int)	{ cls->SetNearCameraChunkDist(p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockWorldClient, GetMaxBufferRebuildPerTick_s, int*)		{ *p1 = cls->GetMaxBufferRebuildPerTick(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockWorldClient, SetMaxBufferRebuildPerTick_s, int)	{ cls->SetMaxBufferRebuildPerTick(p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockWorldClient, GetMaxBufferRebuildPerTick_FarChunk_s, int*)		{ *p1 = cls->GetMaxBufferRebuildPerTick_FarChunk(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockWorldClient, SetMaxBufferRebuildPerTick_FarChunk_s, int)	{ cls->SetMaxBufferRebuildPerTick_FarChunk(p1); return S_OK; }

		ATTRIBUTE_METHOD1(BlockWorldClient, GetUsePointTextureFiltering_s, bool*)	{ *p1 = cls->GetUsePointTextureFiltering(); return S_OK; }
		ATTRIBUTE_METHOD1(BlockWorldClient, SetUsePointTextureFiltering_s, bool)	{ cls->SetUsePointTextureFiltering(p1); return S_OK; }

		//////////////////////////////////////////////////////////////////////////
		//static functions
		//////////////////////////////////////////////////////////////////////////
		static BlockWorldClient* GetInstance();

		//////////////////////////////////////////////////////////////////////////
		//member functions
		//////////////////////////////////////////////////////////////////////////
		BlockWorldClient();
		~BlockWorldClient();

		//init block world
		virtual void EnterWorld(const string& sWorldDir, float x,float y,float z);

		//call this function when leave the block world
		virtual void LeaveWorld();

		//perform culling and fill up m_visibleChunks
		virtual void UpdateVisibleChunks(bool bIsShadowPass = false);
		
		/** clear block render cache */
		virtual void ClearBlockRenderCache();

		void DeleteAllBlocks();

		//////////////////////////////////////////////////////////////////////////
		//render related function

		/** how blocks are preferred to be rendered. */
		virtual void SetBlockRenderMethod(BlockRenderMethod method);

		//perform culling and prepare render group,this function should be called
		//before Render
		void PreRender(bool bIsShadowPass = false);

		void CheckRebuildVisibleChunks(bool bAsyncMode = true, bool bIsShadowPass = false);

		/** render all blocks in render queue using the given pass
		* @param pCurRenderQueue: if NULL, it will fetch current render queue according to nRenderPass
		* @param nRenderMethod: if -1, it will use the this->GetBlockRenderMethod(), otherwise we will render using this specifed render method. 
		* pass any value in enumeration BlockRenderMethod
		*/
		void Render(BlockRenderPass nRenderPass = BlockRenderPass_Opaque, std::vector<BlockRenderTask*>* pCurRenderQueue = NULL, int nRenderMethod = -1);

		/** do deferred lighting */
		void RenderDeferredLighting();

		std::vector<BlockRenderTask*>* GetRenderQueueByPass(BlockRenderPass nRenderPass);

		/** render shadow map pass*/
		void RenderShadowMap();

		//add render task to render queueGe
		virtual void AddRenderTask(BlockRenderTask* pRenderTask);

		//this function can be used to render a damaged block
		void SetDamagedBlock(uint16_t wx,uint16_t wy,uint16_t wz);

		void SetDamagedBlockDegree(float degree);

		/** check whether a specified point is under the water surface.
		first check block world and then check real world
		* @return: return true if point is under water surface.*/
		bool IsPointUnderWater(const Vector3& vPos);

		/** default block emissive light color. It is usually white or yellow light. */
		void SetBlockLightColor(const LinearColor& color);

		/** default block emissive light color. It is usually white or yellow light. */
		LinearColor GetBlockLightColor();

		
		/** used to render selection texture. It is layer on top of block texture 
		@param textureName: if textureName begins with "[0-9]:filename", it will be setting the Nth selection texture
		*/
		void SetSelectionTexture(const char* textureName);

		std::string GetSelectionTexture();

		/** used to render damaged texture. It is layer on top of block texture */
		void SetDamageTexture(const char* textureName);
		std::string GetDamageTexture();

		
		/** get the water block level just nRayLength blocks under the given positions
		* it will also check global real water height and use the higher. 
		* @param nRayLength: cast a ray from x,y,z downward and find the water height. 
		*/
		float GetWaterLevel(float x, float y, float z, int nRayLength=2);

		/// this function currently does nothing
		void InitDeviceObjects();
		void RestoreDeviceObjects();
		/// must call this function when device is lost.
		void DeleteDeviceObjects();
		/// must call this function when device is invalid.
		void InvalidateDeviceObjects();
		void RendererRecreated();

		/** do post rendering processing */
		void DoPostRenderingProcessing(BlockRenderPass nRenderPass = BlockRenderPass_Opaque);

		void RestoreNormalShader();

		void Cleanup();

		/** call this function to set up all render targets for fancy graphics if any. */
		bool PrepareAllRenderTargets(bool bSetRenderTarget = true);

		/** render multi frame block world is enabled. */
		bool DrawMultiFrameBlockWorld();

		/** render the image to the screen, with z test enabled and z-write disabled. As if drawing on the skybox. */
		bool DrawMultiFrameBlockWorldOnSky();

		/** this is usually set for fancy graphics' composite shader in deferred shading. */
		void SetPostProcessingScript(const char* sCallbackScript);
		void SetPostProcessingAlphaScript(const char* sCallbackScript);

		/** check device capabilities for deferred shading. */
		bool CanUseAdvancedShading();

		/** whether to use sun light shadow map */
		void SetUseSunlightShadowMap(bool bEnable);
		bool GetUseSunlightShadowMap();


		/** whether to use sun light shadow map */
		void SetUseWaterReflection(bool bEnable);
		bool GetUseWaterReflection();

		/** prepare shadow casters */
		void PrepareShadowCasters( CShadowMap* pShadowMap );

		/** render selection and damaged blocks*/
		void RenderDynamicBlocks();
		
		//get highest y values of current and neighbor blocks above terrain.
		//return terrain height if no block above ground.
		//@param result: int16_t[6] array contains the result height of {center,x,-x,z,-z,mask} 
		// each bit of mask indicate return value is block height or terrain height, 
		virtual void GetMaxBlockHeightWatchingSky(uint16_t blockX_ws, uint16_t blockZ_ws, ChunkMaxHeight* pResult);

		/** whenever in movie output mode, we will cache all terrain and lighting information. Block chunks are loaded without delay.
		* this mode is only available on PC and turned on manually, since it may slow down rendering for slow PC. And may crash for big scenes running 32bits version due to memory limit.
		*/
		bool IsMovieOutputMode() const;
		void EnableMovieOutputMode(bool val);

		/** for 32 bits version, we need to set the vertex buffer size limit to something like 700MB to prevent running out of 2GB virtual memory. */
		int GetVertexBufferSizeLimit() const;
		void SetVertexBufferSizeLimit(int val);

		/** we will ensure all chunks in this buffer range are also in memory, and not counted in m_nVertexBufferSizeLimit */
		int GetAlwaysInVertexBufferChunkRadius() const;
		void SetAlwaysInVertexBufferChunkRadius(int val);

		/** we will only draw when there are less than this number of vertex visible at the current frame.*/
		int GetMaxVisibleVertexBufferBytes() const;
		void SetMaxVisibleVertexBufferBytes(int val);

		/** if true, if we want to use a separate thread to fill the chunk buffer.
		*/
		bool IsAsyncChunkMode() const;
		void SetAsyncChunkMode(bool val);

		/** near camera chunk is always rendered and cached in vertex buffer. default to 2 chunks from eye position. */
		int GetNearCameraChunkDist() const;
		void SetNearCameraChunkDist(int val);

		/** how many chunks to upload to video memory per tick for near camera chunks. 
		* @sa: GetNearCameraChunkDist
		*/
		int GetMaxBufferRebuildPerTick() const;
		void SetMaxBufferRebuildPerTick(int val);
		
		/** how many chunks to upload to video memory for far aways chunks per tick. 
		* @sa: GetNearCameraChunkDist
		*/
		int GetMaxBufferRebuildPerTick_FarChunk() const;
		void SetMaxBufferRebuildPerTick_FarChunk(int val);

		//helper function to compare render order
		static bool CompareRenderOrder(BlockRenderTask* v0, BlockRenderTask* v1);

		/** whether to use point texture filtering for all ui images rendered. */
		bool GetUsePointTextureFiltering();
		void SetUsePointTextureFiltering(bool bUse);
	protected:
		virtual void UpdateActiveChunk();

	private:
		//Get shared index buffer which will be reused for all render task
		IndexBufferDevicePtr_type GetIndexBuffer();
		VertexDeclarationPtr GetVertexLayout();
		VertexDeclarationPtr GetSelectBlockVertexLayout();

		void BuildSelectionBlockBuffer();

		void RenderSelectionBlock(int nSelectionIndex = BLOCK_GROUP_ID_HIGHLIGHT, float fScaledBlockSize = 1.01f, LinearColor* pColor = NULL, BOOL bColorAnimate = TRUE);

		void RenderWireFrameBlock(int nSelectionIndex = BLOCK_GROUP_ID_WIREFRAME, float fScaledBlockSize = 1.01f, LinearColor* pLineColor = NULL);

		void RenderDamagedBlock();

		int FillSelectBlockVertice(uint16_t nNearbyValue, uint16_t x, uint16_t y, uint16_t z, SelectBlockVertex* blockModel, const Vector3& vOffset, const float fBlockSize, float fUV_Y_Offset = 0.f, float fUV_Y_Size = 1.f);

		int FillSelectBlockInvert(uint16_t nNearbyValue, uint16_t x, uint16_t y, uint16_t z, SelectBlockVertex* blockModel, const Vector3& vOffset, const float fBlockSize, float fUV_Y_Offset = 0.f, float fUV_Y_Size = 1.f);
		
		bool HasSunlightShadowMap();

		void AddToVisibleChunk(RenderableChunk &chunk, int nViewDist, int nRenderFrameCount);
		int ClearActiveChunksToMemLimit(bool bIsShadowPass = false);

		void ClearVisibleChunksToByteLimit(bool bIsShadowPass);

	private:
		static BlockWorldClient* g_pInstance;
		
		static int32_t g_TexPass;
		static int32_t g_twoTexPass;
		static int32_t g_TexNormalMapPass;
		static int32_t g_twoTexNormalMapPass;
		static int32_t g_materialPass;
		static int32_t g_materialFancyPass;
		static int32_t g_transparentBlockPass;
		static int32_t g_waterBlockPass;
		static int32_t g_selectBlockPass;
		static int32_t g_bumpyBlockPass;

		
		bool m_isUnderLiquid;
		
		/** default block emissive light color. It is usually white or yellow light. */
		LinearColor m_vBlockLightColor;

		std::vector<BlockRenderTask*> m_solidRenderTasks;
		std::vector<BlockRenderTask*> m_alphaTestRenderTasks;
		std::vector<BlockRenderTask*> m_alphaBlendRenderTasks;
		std::vector<BlockRenderTask*> m_reflectedWaterRenderTasks;


		std::vector<float> m_selectBlockInstData;
		int32_t m_maxSelectBlockPerBatch;

		asset_ptr<TextureEntity> m_highLightTextures[BLOCK_GROUP_ID_MAX];
		std::vector<SelectBlockVertex>  m_select_block_vertices;
		std::vector<uint16_t>  m_select_block_indices;

		ParaIndexBuffer m_sharedIndexBuffer;

#ifdef USE_DIRECTX_RENDERER
		/** normal render target */
		asset_ptr<TextureEntity> m_render_target_color;
		/** HDR render target */
		asset_ptr<TextureEntity> m_render_target_color_hdr;
		/** depth, block_id, block_light, sun_light*/
		asset_ptr<TextureEntity> m_render_target_block_info;
		LPDIRECT3DSURFACE9       m_render_target_block_info_surface;
		/** 32bits for the red channel, writing depth info to this texture. */
		asset_ptr<TextureEntity> m_render_target_depth_tex;
		LPDIRECT3DSURFACE9       m_render_target_depth_tex_surface;
		/** normal render target */
		asset_ptr<TextureEntity> m_render_target_normal;
		LPDIRECT3DSURFACE9       m_render_target_normal_surface;
		/** depth surface */
		LPDIRECT3DSURFACE9 m_pDepthStencilSurface; 
		LPDIRECT3DSURFACE9 m_pOldRenderTarget; 
		LPDIRECT3DSURFACE9 m_pOldZBuffer; 

		/** block effect used for rendering fancy effect. */
		asset_ptr<CEffectFile> m_block_effect_fancy;
		asset_ptr<CEffectFile> m_normal_mesh_effect_fancy;
		asset_ptr<CEffectFile> m_bmax_model_effect_fancy;
		asset_ptr<CEffectFile> m_terrain_fancy;

		/** 3 kinds of light geometry shader in deferred shading */
		asset_ptr<CEffectFile> m_effect_light_point;
		asset_ptr<CEffectFile> m_effect_light_spot;
		asset_ptr<CEffectFile> m_effect_light_directional;

		/** deferred lighting shader */
		asset_ptr<CEffectFile> m_effect_deferred_lighting;

		/** deferred light geometry for {D3DLIGHT_POINT, D3DLIGHT_SPOT, D3DLIGHT_DIRECTIONAL} */
		asset_ptr<CEffectFile> m_lightgeometry_effects[3];
#endif
		CMultiFrameBlockWorldRenderer* m_pMultiFrameRenderer;

		Uint16x3 m_damagedBlockId;
		asset_ptr<TextureEntity> m_damageTexture;
		float m_damageDegree;

		/** the script to be executed per frame when doing the post processing. One can control the post processing
		* via the GetPostProcessingScene() function */
		std::string m_sPostProcessorCallbackScript;
		/** same as above */
		std::string m_sPostProcessorAlphaCallbackScript;
		
		int m_nMaxBufferRebuildPerTick;
		int m_nNearCameraChunkDist;
		int m_nMaxBufferRebuildPerTick_FarChunk;
		
		/** for 32 bits version, we need to set the vertex buffer size limit to something like 100MB to prevent running out of 2GB virtual memory. */
		int m_nVertexBufferSizeLimit;
		/** we will ensure all chunks in this buffer range are also in memory, and not counted in m_nVertexBufferSizeLimit */
		int m_nAlwaysInVertexBufferChunkRadius;
		/** we will only draw when there are less than this number of vertex visible at the current frame.*/
		int m_nMaxVisibleVertexBufferBytes;
		
		/** number of render buffer created last render frame. */
		int m_nBufferRebuildCountThisTick;

		/** if false, it does not render shadow even in fancy graphic mode. */
		bool m_bUseSunlightShadowMap;

		/** if false, it does not use water reflection even in fancy graphic mode. */
		bool m_bUseWaterReflection;

		/** whenever in movie output mode, we will cache all terrain and lighting information. Block chunks are loaded without delay. 
		* this mode is only available on PC and turned on manually, since it may slow down rendering for slow PC. And may crash for big scenes running 32bits version due to memory limit.  
		*/
		bool m_bMovieOutputMode;

		/** if true, if we want to use a separate thread to fill the chunk buffer.
		*/
		bool m_bAsyncChunkMode;

		/** default to false for all UI images rendered. */
		bool m_bUsePointTextureFiltering;
	};
}






