//-----------------------------------------------------------------------------
// Class:	Block Manager
// Authors:	LiXizhi, Clayman
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2012.11.26
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "EffectManager.h"
#include "OceanManager.h"
#include "ParaVertexBufferPool.h"
#ifdef USE_DIRECTX_RENDERER
#include "ShadowMap.h"
#include "DirectXEngine.h"
#endif
#include "SunLight.h"
#include "BaseCamera.h"
#include "ParaWorldAsset.h"
#include "terrain/GlobalTerrain.h"
#include "WorldInfo.h"
#include "BlockCommon.h"
#include "BlockRegion.h"
#include "terrain/Settings.h"
#include "IBatchedElementDraw.h"
#include "AISimulator.h"
#include "NPLRuntime.h"
#include "BlockLightGridClient.h"
#include "ParaEngineSettings.h"
#include "ViewportManager.h"
#include "LightObject.h"
#include "NPLHelper.h"
#include "util/os_calls.h"
#include "ChunkVertexBuilderManager.h"
#include "MultiFrameBlockWorldRenderer.h"
#include "BlockWorldClient.h"

/** default block light color. due to shader optimization, there can only be one color for all emissive blocks. 
* ARBG: it is usually white or yellow(red) color. */
// #define DEFAULT_BLOCK_LIGHT_COLOR 0xffffffff
#define DEFAULT_BLOCK_LIGHT_COLOR 0xffffffff

// define to output log for debugging. 
//#define PRINT_CHUNK_LOG

#ifdef PRINT_CHUNK_LOG
#include "ParaTime.h"
#endif


namespace ParaEngine
{
	BlockWorldClient* BlockWorldClient::g_pInstance = NULL;
	int32_t BlockWorldClient::g_TexPass = 0;
	int32_t BlockWorldClient::g_twoTexPass = 1;
	int32_t BlockWorldClient::g_TexNormalMapPass = 2;
	int32_t BlockWorldClient::g_twoTexNormalMapPass = 3; // not used. 
#ifdef PARAENGINE_MOBILE
	int32_t BlockWorldClient::g_transparentBlockPass = 2;
	int32_t BlockWorldClient::g_waterBlockPass = 2;
	int32_t BlockWorldClient::g_bumpyBlockPass = 1;
#else
	int32_t BlockWorldClient::g_transparentBlockPass = 3;
	int32_t BlockWorldClient::g_waterBlockPass = 4;
	int32_t BlockWorldClient::g_bumpyBlockPass = 5;
#endif
	
	int32_t BlockWorldClient::g_selectBlockPass = 1;


	inline int64_t GetBlockSparseIndex(int64_t bx,int64_t by,int64_t bz)
	{
		return by*30000*30000+bx*30000+bz;
	}

	//////////////////////////////////////////////////////////////////////////
	//BlockWorldClient
	//////////////////////////////////////////////////////////////////////////
	BlockWorldClient::BlockWorldClient()
		:m_maxSelectBlockPerBatch(80), m_isUnderLiquid(false), m_vBlockLightColor(DEFAULT_BLOCK_LIGHT_COLOR), 
		m_nBufferRebuildCountThisTick(0), 
		m_nVertexBufferSizeLimit(100 * 1024 * 1024), 
		m_nMaxVisibleVertexBufferBytes(100 * 1024 * 1024),
		m_nAlwaysInVertexBufferChunkRadius(2),
		m_pMultiFrameRenderer(NULL),
#ifdef USE_DIRECTX_RENDERER
		m_pDepthStencilSurface(NULL), m_render_target_block_info_surface(NULL), m_render_target_depth_tex_surface(NULL), m_render_target_normal_surface(NULL), m_pOldRenderTarget(NULL), m_pOldZBuffer(NULL),
#endif
		m_bUseSunlightShadowMap(true), m_bUseWaterReflection(true), m_bMovieOutputMode(false), m_bAsyncChunkMode(true)
	{
		g_pInstance = this;
		m_damageDegree = 0;
	
#if defined(PARAENGINE_CLIENT) || defined(WIN32)
		m_nMaxBufferRebuildPerTick = 4;
		m_nNearCameraChunkDist = 8;
		m_nMaxBufferRebuildPerTick_FarChunk = 2;
		m_nVertexBufferSizeLimit = 200 * 1024 * 1024;
		m_nMaxVisibleVertexBufferBytes = (100 * 1024 * 1024);
#else
		m_nMaxBufferRebuildPerTick = 4;
		m_nNearCameraChunkDist = 6;
		m_nMaxBufferRebuildPerTick_FarChunk = 2;
		m_nVertexBufferSizeLimit = 100 * 1024 * 1024;
		m_nMaxVisibleVertexBufferBytes = (50 * 1024 * 1024);
#endif
		
#ifdef PARAENGINE_MOBILE
		SetGroupByChunkBeforeTexture(true);
#endif

		SAFE_DELETE(m_pLightGrid);
		m_pLightGrid = new CBlockLightGridClient(m_activeChunkDim + 2, this);
		// ensure that all vertices can be indexed by uint16 index buffer. 
		PE_ASSERT((BlockConfig::g_maxFaceCountPerBatch * 4) <= 0xffff);
		RenderableChunk::GetVertexBufferPool()->SetFullSizedBufferSize(BlockConfig::g_maxFaceCountPerBatch * sizeof(BlockVertexCompressed) * 4);
		// 6MB at most
		RenderableChunk::GetVertexBufferPool()->SetMaxPooledCount(6000000 / RenderableChunk::GetVertexBufferPool()->GetFullSizedBufferSize());

		m_pMultiFrameRenderer = new CMultiFrameBlockWorldRenderer(this);
	}

	BlockWorldClient::~BlockWorldClient()
	{
		DeleteDeviceObjects();
		SAFE_DELETE(m_pMultiFrameRenderer);
		for(int i=0;i<(int)(m_activeChunks.size());++i)
		{
			SAFE_DELETE(m_activeChunks[i]);
		}
		m_activeChunks.clear();
		ChunkVertexBuilderManager::GetInstance().Cleanup();

		RenderableChunk::StaticRelease();
		
	}

	void BlockWorldClient::EnterWorld(const string& sWorldDir, float x,float y,float z)
	{
		if (m_isInWorld)
			return;
		ChunkVertexBuilderManager::GetInstance().StartChunkBuildThread(this);
		Scoped_WriteLock<BlockReadWriteLock> lock_(GetReadWriteLock());
		CBlockWorld::EnterWorld(sWorldDir, x, y, z);
	}


	void BlockWorldClient::Cleanup()
	{
		DeleteDeviceObjects();
		for (int i = 0; i < BLOCK_GROUP_ID_MAX; ++i)
		{
			m_highLightTextures[i].reset();
		}
	}

	void BlockWorldClient::LeaveWorld()
	{
		if(!m_isInWorld)
			return;
		ChunkVertexBuilderManager::GetInstance().Cleanup();
		Scoped_WriteLock<BlockReadWriteLock> lock_(GetReadWriteLock());
		Cleanup();
		CBlockWorld::LeaveWorld();
	}

	void BlockWorldClient::DeleteAllBlocks()
	{
		Scoped_WriteLock<BlockReadWriteLock> lock_(GetReadWriteLock());
		for(std::map<int,BlockRegion*>::iterator iter = m_regionCache.begin();iter != m_regionCache.end();iter++)
		{
			iter->second->DeleteAllBlocks();
		}
		
		for(uint32_t i=0;i<m_activeChunks.size();i++)
		{
			m_activeChunks[i]->SetChunkDirty(true);
		}

		m_isVisibleChunkDirty = true;
	}
 
	void BlockWorldClient::PreRender(bool bIsShadowPass)
	{
		if (!m_isInWorld)
			return;
		Vector3 camWorldPos = CGlobals::GetSceneState()->vEye;
		Uint16x3 camBlockPos;
		BlockCommon::ConvertToBlockIndex(camWorldPos.x, camWorldPos.y, camWorldPos.z, camBlockPos.x, camBlockPos.y, camBlockPos.z);
		SetEyeBlockId(camBlockPos);
		Block* eye_block = GetBlock(camBlockPos.x, camBlockPos.y, camBlockPos.z);
		if (eye_block && eye_block->GetTemplate())
		{
			m_isUnderLiquid = eye_block->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_liquid);
		}
		else
			m_isUnderLiquid = false;

		if (!bIsShadowPass)
			GetLightGrid().UpdateLighting();

		//perform culling and refresh render buffers if dirty. 
		m_nBufferRebuildCountThisTick = 0;
		if (m_isVisibleChunkDirty || (GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER && GetUseSunlightShadowMap()))
			UpdateVisibleChunks(bIsShadowPass);

		m_solidRenderTasks.clear();
		m_alphaTestRenderTasks.clear();
		m_alphaBlendRenderTasks.clear();
		m_reflectedWaterRenderTasks.clear();

		//fill render task
		Vector3 renderOfs = CGlobals::GetScene()->GetRenderOrigin();
		float verticalOffset = GetVerticalOffset();
		if (!bIsShadowPass)
			ChunkVertexBuilderManager::GetInstance().UploadPendingChunksToDevice();
		if (IsRenderBlocks())
		{
			// please note m_visibleChunks is already arranged in a spiral pattern around the eye. 
			// hence the closer ones get updated sooner. 

			m_nBufferRebuildCountThisTick = 0;

			CheckRebuildVisibleChunks(m_bAsyncChunkMode, bIsShadowPass);
			ClearVisibleChunksToByteLimit(bIsShadowPass);

#ifdef PRINT_CHUNK_LOG
			static int64 g_lastTime = GetTimeUS();
			int64 curTime = GetTimeUS();
			if (m_nBufferRebuildCountThisTick > 0){
				OUTPUT_LOG("deltaTime: %d ms: chunk buffer update this tick %d\n", (int)(curTime - g_lastTime) / 1000, (int)m_nBufferRebuildCountThisTick);
			}
			g_lastTime = curTime;
#endif
			for (uint32 i = 0; i < m_visibleChunks.size(); i++)
			{
				m_visibleChunks[i]->FillRenderQueue(this, renderOfs, verticalOffset);
			}
		}

		//sort data
		if(!IsGroupByChunkBeforeTexture())
		{
			if (m_solidRenderTasks.size() > 0)
				sort(m_solidRenderTasks.begin(), m_solidRenderTasks.end(), CompareRenderOrder);
		}

		if(m_alphaTestRenderTasks.size() > 0)
			sort(m_alphaTestRenderTasks.begin(),m_alphaTestRenderTasks.end(),CompareRenderOrder);

		if (m_alphaBlendRenderTasks.size() > 0)
			sort(m_alphaBlendRenderTasks.begin(), m_alphaBlendRenderTasks.end(), CompareRenderOrder);

		if (m_reflectedWaterRenderTasks.size() > 0)
			sort(m_reflectedWaterRenderTasks.begin(), m_reflectedWaterRenderTasks.end(), CompareRenderOrder);
	}


	void BlockWorldClient::RenderShadowMap()
	{
#ifdef USE_DIRECTX_RENDERER
		// only render shadow map for fancy shader mode. 
		if( (GetBlockRenderMethod() !=  BLOCK_RENDER_FANCY_SHADER) || 
			! GetUseSunlightShadowMap())
			return;
		// prepare shadow pass 
		PreRender(true);
		// make sure all shaders are replaced. 
		PrepareAllRenderTargets(false);

		if (m_alphaTestRenderTasks.size() == 0 && m_solidRenderTasks.size() == 0 && m_alphaBlendRenderTasks.size() == 0)
			return;

		if(m_block_effect_fancy == 0)
			return;
		m_block_effect_fancy->LoadAsset();

		const float fBlockSize = BlockConfig::g_blockSize;
		Vector3 renderOfs  = CGlobals::GetScene()->GetRenderOrigin();
		float verticalOffset = GetVerticalOffset();

		Uint16x3 renderBlockOfs;
		renderBlockOfs.x = (uint16_t)(renderOfs.x/fBlockSize);
		renderBlockOfs.z = (uint16_t)(renderOfs.z/fBlockSize);

		Vector3 renderBlockOfs_remain;
		renderBlockOfs_remain.x = renderOfs.x - renderBlockOfs.x * fBlockSize;
		renderBlockOfs_remain.z = renderOfs.z - renderBlockOfs.z * fBlockSize;

		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		RenderDevicePtr pDevice = CGlobals::GetRenderDevice();

		IScene* pScene = CGlobals::GetEffectManager()->GetScene();
		CBaseCamera* pCamera = pScene->GetCurrentCamera();
		Matrix4 matViewProj;
		if (pCamera)
		{
			const Matrix4& pView = (CGlobals::GetEffectManager()->GetViewTransform());
			const Matrix4& pProj = (CGlobals::GetEffectManager()->GetProjTransform());
			matViewProj = (pView) * (pProj);
		}

		CEffectFile* pEffect = NULL;
		pEffectManager->BeginEffect(TECH_BLOCK_FANCY, &pEffect);
		if(pEffect != 0 && pEffect->begin(false))
		{
			VertexDeclarationPtr pVertexLayout =  GetVertexLayout();
			pDevice->SetVertexDeclaration(pVertexLayout);

			// turn off alpha blending to enable early-Z on modern graphic cards. 
			pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

			//////////////////////////////////////////////////////////////////////////
			// set the wave time parameter
			double time = CGlobals::GetGameTime(); 
			// in case it loses accuracy, we will subtract integer number of 2*PI from time
			// time -= ((int)(time / (2*MATH_PI)))*(2*MATH_PI);
			time = (int)(time*1000) % 1000000;
			pEffect->setParameter(CEffectFile::k_ConstVector1, Vector4((float)time, 0.f, 0.f, 0.f).ptr());

			IDirect3DIndexBuffer9* pIndexBuffer = GetIndexBuffer();
			pDevice->SetIndices(pIndexBuffer);

			for (int nRenderPass = 0; nRenderPass<=BlockRenderPass_AlphaBlended; nRenderPass++)
			{
				std::vector<BlockRenderTask*>* pCurRenderQueue = GetRenderQueueByPass((BlockRenderPass)nRenderPass);

				IDirect3DVertexBuffer9* pCurVB = NULL;
				uint16_t curTamplerId = 0;
				int32_t curPass = -1;
				D3DCULL culling = D3DCULL_CCW;
				IDirect3DTexture9* pCurTex0 = NULL;

				if(pCurRenderQueue->size() > 0)
				{
					for(uint32_t i=0;i<pCurRenderQueue->size();i++)
					{
						BlockRenderTask* pRenderTask = (*pCurRenderQueue)[i];
						IDirect3DVertexBuffer9* pVB = pRenderTask->GetVertexBuffer();

						if(pVB != pCurVB)
						{
							pDevice->SetStreamSource(0,pVB,0,sizeof(BlockVertexCompressed));
							pCurVB = pVB;
						}
						int32_t passId = 0;
						if(curTamplerId != pRenderTask->GetTemplateId())
						{
							BlockTemplate* pTempate = pRenderTask->GetTemplate();
							
							if(pTempate->IsShadowCaster())
							{
								passId = 0;
							}
							
							if(curPass != passId)
							{
								if(curPass > -1)
									pEffect->EndPass();

								if(pEffect->BeginPass(passId))
								{
									curPass = passId;
								}
								else
								{
									curPass = -1;
									continue;
								}
							}

							TextureEntity* pTexEntity = pTempate->GetTexture0(pRenderTask->GetUserData());
							if(pTexEntity && pTexEntity->GetTexture()!=pCurTex0)
							{
								pCurTex0 = pTexEntity->GetTexture();
								pDevice->SetTexture(0,pCurTex0);
							}
							
							// culling mode
							if(pTempate->GetBlockModel().IsDisableFaceCulling())
							{
								if(culling != D3DCULL_NONE)
								{
									pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
									culling = D3DCULL_NONE;
								}
							}
							else if (nRenderPass != BlockRenderPass_Opaque && m_isUnderLiquid
								&& pRenderTask->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_liquid))
							{
								if(culling != D3DCULL_CW)
								{
									pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
									culling = D3DCULL_CW;
								}
							}
							else
							{
								if(culling != D3DCULL_CCW)
								{
									pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
									culling = D3DCULL_CCW;
								}
							}
						}

						Matrix4 vWorldMatrix(Matrix4::IDENTITY);
						Uint16x3& vMinPos = pRenderTask->GetMinBlockPos();
						vWorldMatrix._11 = vWorldMatrix._22 = vWorldMatrix._33 = fBlockSize;
						vWorldMatrix._41 = (vMinPos.x - renderBlockOfs.x)*fBlockSize - renderBlockOfs_remain.x;
						vWorldMatrix._42 = vMinPos.y * fBlockSize - renderOfs.y + verticalOffset;
						vWorldMatrix._43 = (vMinPos.z - renderBlockOfs.z)*fBlockSize - renderBlockOfs_remain.z;

						vWorldMatrix = vWorldMatrix * matViewProj;

						pEffect->setMatrix(CEffectFile::k_worldViewProjMatrix, &vWorldMatrix);

						pEffect->CommitChanges();

						pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, pRenderTask->GetVertexOfs(),
							pRenderTask->GetVertexCount(),pRenderTask->GetIndexOfs(),pRenderTask->GetPrimitiveCount());
					}
				}
				if(curPass > -1)
					pEffect->EndPass();

				pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
			}
			// turn blending on again
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			pEffect->end();
		}
#endif
	}

	void BlockWorldClient::Render(BlockRenderPass nRenderPass, std::vector<BlockRenderTask*>* pCurRenderQueue, int nRenderMethod)
	{
		if(!m_isInWorld)
			return;
	
		BlockRenderMethod dwRenderMethod = (nRenderMethod < 0) ? GetBlockRenderMethod() : (BlockRenderMethod)nRenderMethod;
		
		// no need to lock 
		if (pCurRenderQueue == 0)
			pCurRenderQueue = GetRenderQueueByPass(nRenderPass);
		
		RenderDevicePtr pDevice = CGlobals::GetRenderDevice();

		if(pCurRenderQueue->size() > 0)
		{
			const float fBlockSize = BlockConfig::g_blockSize;
			Vector3 renderOfs  = CGlobals::GetScene()->GetRenderOrigin();
			float verticalOffset = GetVerticalOffset();
			
			Uint16x3 renderBlockOfs;
			renderBlockOfs.x = (uint16_t)(renderOfs.x/fBlockSize);
			renderBlockOfs.z = (uint16_t)(renderOfs.z/fBlockSize);

			Vector3 renderBlockOfs_remain;
			renderBlockOfs_remain.x = renderOfs.x - renderBlockOfs.x * fBlockSize;
			renderBlockOfs_remain.z = renderOfs.z - renderBlockOfs.z * fBlockSize;

			EffectManager* pEffectManager = CGlobals::GetEffectManager();
			
			CEffectFile* pEffect = NULL;
			if (dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
			{
				if (nRenderPass != BlockRenderPass_AlphaBlended)
				{
					if (!PrepareAllRenderTargets())
					{
						// try a lower shader other than fancy 
						SetBlockRenderMethod(BLOCK_RENDER_FAST_SHADER);
						return;
					}

					pEffectManager->BeginEffect(TECH_BLOCK_FANCY, &pEffect);
					if (pEffect == 0)
					{
						// try a lower shader other than fancy ;
						SetBlockRenderMethod(BLOCK_RENDER_FAST_SHADER);
						return;
					}
				}
				else
				{
					// use ordinary shader for alpha blended blocks. 
					pEffectManager->BeginEffect(TECH_BLOCK, &pEffect);
				}
			}
			else if (dwRenderMethod == BLOCK_RENDER_FAST_SHADER)
			{
				pEffectManager->BeginEffect(TECH_BLOCK, &pEffect);
			}
			else if (dwRenderMethod == BLOCK_RENDER_FIXED_FUNCTION)
			{
				pEffectManager->BeginEffect(TECH_BLOCK, &pEffect);
			}

			// CEffectFile* pEffect = pEffectManager->GetCurrentEffectFile();

			if (nRenderPass == BlockRenderPass_AlphaBlended || nRenderPass == BlockRenderPass_ReflectedWater)
			{
				pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
			else
			{
				if (nRenderPass == BlockRenderPass_AlphaTest)
					pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

				// turn off alpha blending to enable early-Z on modern graphic cards. 
				pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
				pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			}

			if ( pEffect == 0)
			{
#ifdef USE_DIRECTX_RENDERER
				if (dwRenderMethod != BLOCK_RENDER_FIXED_FUNCTION)
				{
					SetBlockRenderMethod(BLOCK_RENDER_FIXED_FUNCTION);
					return;
				}
				//////////////////////////////////////////////////////////////////////////
				// render using fixed function pipeline

				pDevice->SetFVF(block_vertex::FVF);

				IDirect3DIndexBuffer9* pIndexBuffer =  GetIndexBuffer();
				pDevice->SetIndices(pIndexBuffer);

				IDirect3DVertexBuffer9* pCurVB = NULL;
				uint16_t curTamplerId = 0;
				int32_t curPass = -1;
				D3DCULL culling = D3DCULL_CCW;
				IDirect3DTexture9* pCurTex0 = NULL;
				IDirect3DTexture9* pCurTex1 = NULL;
				IDirect3DTexture9* pCurTex2 = NULL;

				pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE );
				pDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_DIFFUSE | D3DTA_ALPHAREPLICATE );
				
				for(uint32_t i=0;i<pCurRenderQueue->size();i++)
				{
					BlockRenderTask* pRenderTask = (*pCurRenderQueue)[i];
					IDirect3DVertexBuffer9* pVB = pRenderTask->GetVertexBuffer();
					if(pVB != pCurVB)
					{
						pDevice->SetStreamSource(0,pVB,0,sizeof(BlockVertexCompressed));
						pCurVB = pVB;
					}
					int32_t passId;
					if(curTamplerId != pRenderTask->GetTemplateId())
					{
						BlockTemplate* pTempate = pRenderTask->GetTemplate();
						if(pTempate->IsMatchAttribute(BlockTemplate::batt_twoTexture))
							passId = g_twoTexPass;
						else if(pTempate->IsMatchAttribute(BlockTemplate::batt_transparent))
							passId = g_transparentBlockPass;
						else
							passId = g_TexPass;

						if(curPass != passId)
						{
							curPass = passId;
							if(passId == 0)
							{
								pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
							}
							else if(passId == 1)
							{
								pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
							}
							else if(passId == 2)
							{
								pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
							}
							else if(passId == 3)
							{
								pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
							}
							else
							{
								curPass = -1;
								continue;
							}
						}

						TextureEntity* pTexEntity = pTempate->GetTexture0(pRenderTask->GetUserData());
						if(pTexEntity && pTexEntity->GetTexture()!=pCurTex0)
						{
							pCurTex0 = pTexEntity->GetTexture();
							pDevice->SetTexture(0,pCurTex0);
						}

						pTexEntity = pTempate->GetTexture1();
						if(pTexEntity && pTexEntity->GetTexture()!=pCurTex1)
						{
							pCurTex1 = pTexEntity->GetTexture();
							pDevice->SetTexture(1,pCurTex1);
						}

						/* fixed function never use normal map
						pTexEntity = pTempate->GetNormalMap();
						if(pTexEntity && pTexEntity->GetTexture()!=pCurTex2)
						{
							pCurTex2 = pTexEntity->GetTexture();
							pDevice->SetTexture(2,pCurTex2);
						}*/


						// culling mode 
						if(pTempate->GetBlockModel().IsDisableFaceCulling())
						{
							if(culling != D3DCULL_NONE)
							{
								pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
								culling = D3DCULL_NONE;
							}
						}
						else if (nRenderPass != BlockRenderPass_Opaque && m_isUnderLiquid
							&& pRenderTask->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_liquid))
						{
							if(culling != D3DCULL_CW)
							{
								pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
								culling = D3DCULL_CW;
							}
						}
						else
						{
							if(culling != D3DCULL_CCW)
							{
								pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
								culling = D3DCULL_CCW;
							}
						}
					}


					Matrix4 vWorldMatrix(Matrix4::IDENTITY);
					
					Uint16x3& vMinPos = pRenderTask->GetMinBlockPos();
					vWorldMatrix._11 = vWorldMatrix._22 = vWorldMatrix._33 = fBlockSize;
					vWorldMatrix._41 = (vMinPos.x - renderBlockOfs.x)*fBlockSize - renderBlockOfs_remain.x;
					vWorldMatrix._42 = vMinPos.y * fBlockSize - renderOfs.y + verticalOffset;
					vWorldMatrix._43 = (vMinPos.z - renderBlockOfs.z)*fBlockSize - renderBlockOfs_remain.z;

					pDevice->SetTransform(D3DTS_WORLD, vWorldMatrix.GetPointer());

					RenderDevice::DrawIndexedPrimitive(pDevice, RenderDevice::DRAW_PERF_TRIANGLES_TERRAIN, D3DPT_TRIANGLELIST, 0, pRenderTask->GetVertexOfs(),
						pRenderTask->GetVertexCount(),pRenderTask->GetIndexOfs(),pRenderTask->GetPrimitiveCount());
				}
				
				pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
				pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
				pDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
#endif
			}
			else if(pEffect != 0 && pEffect->begin(false))
			{
#ifdef USE_OPENGL_RENDERER
				if (nRenderPass == BlockRenderPass_AlphaTest)
				{
					// ParaEngine's opengl renderer currently does not support shared uniforms between passes, so do this. 
					// Remove this: when you support it. 
					pEffect->BeginPass(g_transparentBlockPass);
					pEffect->EndPass();
				}
				CGlobals::GetEffectManager()->applyFogParameters();
#endif
				

				VertexDeclarationPtr pVertexLayout = GetVertexLayout();
				pDevice->SetVertexDeclaration(pVertexLayout);

				if (dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
				{
					//////////////////////////////////////////////////////////////////////////
					// set the wave time parameter
					double time = CGlobals::GetGameTime(); 
					// in case it loses accuracy, we will subtract integer number of 2*PI from time
					// time -= ((int)(time / (2*MATH_PI)))*(2*MATH_PI);
					time = (int)(time*1000) % 1000000;
					pEffect->setParameter(CEffectFile::k_ConstVector1, Vector4((float)time, 0.f, 0.f, 0.f).ptr());
				}
				else
				{
					Vector3 vDir = -CGlobals::GetScene()->GetSunLight().GetSunDirection();
					Vector4 vDir_(vDir.x, vDir.y, vDir.z, 1.0f);
					pEffect->setParameter(CEffectFile::k_sunVector, &vDir_);
				}
				
				IndexBufferDevicePtr_type pIndexBuffer =  GetIndexBuffer();
				pDevice->SetIndices(pIndexBuffer);

				VertexBufferDevicePtr_type pCurVB = 0;
				uint16_t curTamplerId = 0;
				int32_t curPass = -1;
				D3DCULL culling = D3DCULL_CCW;
				DeviceTexturePtr_type pCurTex0 = 0;
				DeviceTexturePtr_type pCurTex1 = 0;
				DeviceTexturePtr_type pCurTex2 = 0;

				IScene* pScene = CGlobals::GetEffectManager()->GetScene();
				CBaseCamera* pCamera = pScene->GetCurrentCamera();
				Matrix4 matViewProj;
				if (pCamera)
				{
					const Matrix4& pView = (CGlobals::GetEffectManager()->GetViewTransform());
					const Matrix4& pProj = (CGlobals::GetEffectManager()->GetProjTransform());
					matViewProj = (pView) * (pProj);
				}

				/** block light params and sun intensity*/
				Vector4 vLightParams(m_vBlockLightColor.r, m_vBlockLightColor.g, m_vBlockLightColor.b, m_sunIntensity);
				pEffect->setParameter(CEffectFile::k_ConstVector0, (const void*)(&vLightParams));
				for(uint32_t i=0;i<pCurRenderQueue->size();i++)
				{
					BlockRenderTask* pRenderTask = (*pCurRenderQueue)[i];
					VertexBufferDevicePtr_type pVB = pRenderTask->GetVertexBuffer();
					if (pVB != pCurVB)
					{
						pDevice->SetStreamSource(0,pVB,0,sizeof(BlockVertexCompressed));
						pCurVB = pVB;
					}
					int32_t passId;
					if(curTamplerId != pRenderTask->GetTemplateId())
					{
						BlockTemplate* pTempate = pRenderTask->GetTemplate();
						if(pTempate->IsMatchAttribute(BlockTemplate::batt_twoTexture))
							passId = g_twoTexPass;
						else if (nRenderPass == BlockRenderPass_AlphaTest)
						{
							passId = g_transparentBlockPass;
						}
						else if (nRenderPass == BlockRenderPass_AlphaBlended || nRenderPass == BlockRenderPass_ReflectedWater)
						{
							passId = g_TexPass;
							if (pTempate->GetCategoryID() == 8 && dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
							{
								passId = g_waterBlockPass;
							}
						}
						else if (pTempate->GetNormalMap() && dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
						{
							passId = g_bumpyBlockPass;
						}
						else
							passId = g_TexPass;
							

						
						if(curPass != passId)
						{
							if(curPass > -1)
								pEffect->EndPass();

							if(pEffect->BeginPass(passId))
							{
								curPass = passId;

								pCurTex0 = 0;
								pCurTex1 = 0;
								pCurTex2 = 0;
								// pEffect->setMatrix(CEffectFile::k_viewProjMatrix, &matViewProj);
								// pDevice->SetVertexShaderConstantF(0,&matViewProj._11,4);
							}
							else
							{
								curPass = -1;
								continue;
							}
						}

						TextureEntity* pTexEntity = pTempate->GetTexture0(pRenderTask->GetUserData());
						if(pTexEntity && pTexEntity->GetTexture()!=pCurTex0)
						{
							pCurTex0 = pTexEntity->GetTexture();
							pDevice->SetTexture(0,pCurTex0);
						}

						pTexEntity = pTempate->GetTexture1();
						if(pTexEntity && pTexEntity->GetTexture()!=pCurTex1)
						{
							pCurTex1 = pTexEntity->GetTexture();
							pDevice->SetTexture(1,pCurTex1);
						}

						pTexEntity = pTempate->GetNormalMap();
						if(pTexEntity && pTexEntity->GetTexture()!=pCurTex2)
						{
							pCurTex2 = pTexEntity->GetTexture();
							if (dwRenderMethod  == BLOCK_RENDER_FANCY_SHADER){
								pDevice->SetTexture(2,pCurTex2);
							}
						}

						// culling mode
						if(pTempate->GetBlockModel().IsDisableFaceCulling())
						{
							if(culling != D3DCULL_NONE)
							{
								pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
								culling = D3DCULL_NONE;
							}
						}
						else if (nRenderPass != BlockRenderPass_Opaque && m_isUnderLiquid
							&& pRenderTask->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_liquid))
						{
							if(culling != D3DCULL_CW)
							{
								pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
								culling = D3DCULL_CW;
							}
						}
						else
						{
							if(culling != D3DCULL_CCW)
							{
								pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
								culling = D3DCULL_CCW;
							}
						}
					}

					// 
					Matrix4 vWorldMatrix(Matrix4::IDENTITY);
					Uint16x3& vMinPos = pRenderTask->GetMinBlockPos();
					vWorldMatrix._11 = vWorldMatrix._22 = vWorldMatrix._33 = fBlockSize;
					vWorldMatrix._41 = (vMinPos.x - renderBlockOfs.x)*fBlockSize - renderBlockOfs_remain.x;
					vWorldMatrix._42 = vMinPos.y * fBlockSize - renderOfs.y + verticalOffset;
					vWorldMatrix._43 = (vMinPos.z - renderBlockOfs.z)*fBlockSize - renderBlockOfs_remain.z;
					
					Matrix4 mWorldViewProj;
					mWorldViewProj = vWorldMatrix * matViewProj;

					pEffect->setMatrix(CEffectFile::k_worldViewProjMatrix, &mWorldViewProj);
					if(pEffect->isMatrixUsed(CEffectFile::k_worldViewMatrix))
					{
						Matrix4 mWorldView;
						ParaMatrixMultiply(&mWorldView, &vWorldMatrix, &matViewProj);
						pEffect->setMatrix(CEffectFile::k_worldViewMatrix, &mWorldView);
					}
					{
						// set the new render origin
						Vector4 vWorldPos(0,0,0,1.f);
						/** this is for height shift, using the render origin. */
						Vector3 vRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();
						vWorldPos.x = vRenderOrigin.x + vWorldMatrix._41;
						vWorldPos.y = vRenderOrigin.y + vWorldMatrix._42;
						vWorldPos.z = vRenderOrigin.z + vWorldMatrix._43;
						pEffect->setParameter(CEffectFile::k_worldPos, &vWorldPos);
					}

					pEffect->CommitChanges();

					RenderDevice::DrawIndexedPrimitive(pDevice, D3DPT_TRIANGLELIST, D3DPT_TRIANGLELIST, 0, pRenderTask->GetVertexOfs(),
						pRenderTask->GetVertexCount(), pRenderTask->GetIndexOfs(), pRenderTask->GetPrimitiveCount());
				}

				if(curPass > -1)
					pEffect->EndPass();
				
				pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);

				if (dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
				{
					// disable render target again
				}

				pEffect->end();
				if (pCurVB != 0)
				{
					// unbind buffer if any, this fixed a crash bug with OpenGL multi-frame renderer
					// where the same stream source is used both in multi-frame renderer and main renderer. 
					pDevice->SetStreamSource(0, 0, 0, 0);
				}
			}
			
			if (nRenderPass != BlockRenderPass_AlphaBlended)
			{
				if (nRenderPass == BlockRenderPass_AlphaTest)
					pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
				// turn blending on again
				pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
		}
		if (nRenderPass == BlockRenderPass_AlphaBlended)
		{
			if (dwRenderMethod != BLOCK_RENDER_FANCY_SHADER)
			{
				RenderDynamicBlocks();
			}
		}
	}

	void BlockWorldClient::RenderWireFrameBlock(int nSelectionIndex, float fScaling, LinearColor* pLineColor)
	{
		auto& selectedBlocks = m_selectedBlockMap[nSelectionIndex];
		auto& selectedBlockMap = selectedBlocks.m_blocks;
		
		if(selectedBlockMap.size() == 0)
			return;

		SceneState* sceneState = CGlobals::GetSceneState();
		IBatchedElementDraw* pBatchedElementDraw = sceneState->GetBatchedElementDrawer();
		if (pBatchedElementDraw == 0)
			return;

		Vector3 pVecBounds[8];
		int nNumVertices;


		const float fBlockSize = BlockConfig::g_blockSize;
		Vector3 renderOfs  = CGlobals::GetScene()->GetRenderOrigin();
		float verticalOffset = GetVerticalOffset();

		int32_t renderBlockOfs_x = (int32_t)(renderOfs.x/fBlockSize);
		int32_t renderBlockOfs_y = (int32_t)(renderOfs.y/fBlockSize);
		int32_t renderBlockOfs_z = (int32_t)(renderOfs.z/fBlockSize);

		Vector3 renderBlockOfs_remain;
		renderBlockOfs_remain.x = renderOfs.x - renderBlockOfs_x * fBlockSize;
		renderBlockOfs_remain.y = renderOfs.y - renderBlockOfs_y * fBlockSize;
		renderBlockOfs_remain.z = renderOfs.z - renderBlockOfs_z * fBlockSize;

		// scale the block a little to avoid z fighting. 
		float fScaledBlockSize = BlockConfig::g_blockSize*fScaling;

		if (pBatchedElementDraw)
		{
			PARAVECTOR4 color(0.2f,0.2f,0.2f, 0.7f);

			if (pLineColor)
			{
				color = *((PARAVECTOR4*)pLineColor);
			}
			const float fLineWidth = -2.f;

			std::map<int64_t, Uint16x3>::iterator itCur, itEnd = selectedBlockMap.end();
			for(itCur = selectedBlockMap.begin(); itCur!=itEnd; itCur++)
			{
				Uint16x3& BlockPos = (*itCur).second;
				uint16_t x, y, z;
				x = BlockPos.x;
				y = BlockPos.y;
				z = BlockPos.z;

				BlockTemplate* pBlockTemplate = GetBlockTemplate(x,y,z);
				if(pBlockTemplate == 0)
					 pBlockTemplate = GetBlockTemplate(1);
				if(pBlockTemplate)
				{
					int nLenCount = 12;
					CShapeAABB aabb;
					pBlockTemplate->GetAABB(this, x, y, z, &aabb);

					Vector3 vOffset((x  - renderBlockOfs_x) * fBlockSize, (y  - renderBlockOfs_y) * fBlockSize + verticalOffset, (z  - renderBlockOfs_z) * fBlockSize);
					vOffset -= renderBlockOfs_remain;
					
					aabb.GetCenter() += vOffset;
					aabb.GetExtents() *= fScaling;

					BlockModel::GetBoundingBoxVertices(aabb, pVecBounds, &nNumVertices);
					{
						static const int pIndexBuffer[] = {
							0,1,2,3,0, // bottom
							4,5,6,7,4, // top
							5,1,6,2,3,7, // sides
						};
						
						Vector3 pVertexList[16];
						for(int i=0;i<16; i++)
						{
							pVertexList[i] = pVecBounds[pIndexBuffer[i]] ;
						}

						pBatchedElementDraw->AddThickLines((PARAVECTOR3*)pVertexList, 10, color, fLineWidth);
						pBatchedElementDraw->AddThickLines((PARAVECTOR3*)pVertexList + 10, 2, color, fLineWidth);
						pBatchedElementDraw->AddThickLines((PARAVECTOR3*)pVertexList + 12, 2, color, fLineWidth);
						pBatchedElementDraw->AddThickLines((PARAVECTOR3*)pVertexList + 14, 2, color, fLineWidth);
					}
				}
			}
		}
	}

	void BlockWorldClient::RenderSelectionBlock(int nSelectionIndex, float fScaledBlockSize,  LinearColor* pColor, BOOL bColorAnimate)
	{

		auto& selectedBlocks = m_selectedBlockMap[nSelectionIndex];
		auto& selectedBlockMap = selectedBlocks.m_blocks;
		auto& highLightTexture = m_highLightTextures[nSelectionIndex];
		if(selectedBlockMap.size() == 0)
			return;

		if((int)m_selectBlockInstData.size() < m_maxSelectBlockPerBatch * 4)
			m_selectBlockInstData.resize(m_maxSelectBlockPerBatch * 4);

		RenderDevicePtr pDevice = CGlobals::GetRenderDevice();
		float blockSize = BlockConfig::g_blockSize;

		if(!highLightTexture)
		{
			return;
		}
		else if(!highLightTexture->IsLoaded())
		{
			highLightTexture->LoadAsset();
			return;
		}

		BuildSelectionBlockBuffer();

		const float fBlockSize = BlockConfig::g_blockSize;
		Vector3 renderOfs  = CGlobals::GetScene()->GetRenderOrigin();
		float verticalOffset = GetVerticalOffset();

		int32_t renderBlockOfs_x = (int32_t)(renderOfs.x/fBlockSize);
		int32_t renderBlockOfs_y = (int32_t)(renderOfs.y/fBlockSize);
		int32_t renderBlockOfs_z = (int32_t)(renderOfs.z/fBlockSize);

		Vector3 renderBlockOfs_remain;
		renderBlockOfs_remain.x = renderOfs.x - renderBlockOfs_x * fBlockSize;
		renderBlockOfs_remain.y = renderOfs.y - renderBlockOfs_y * fBlockSize;
		renderBlockOfs_remain.z = renderOfs.z - renderBlockOfs_z * fBlockSize;

		// scale the block a little to avoid z fighting. 
		fScaledBlockSize = BlockConfig::g_blockSize*fScaledBlockSize;

		int32_t count = (int32)selectedBlockMap.size();
		int32_t curInstCount = 0;
		int32_t instFloatCount = 0;

		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->BeginEffect(TECH_BLOCK);
		CEffectFile* pEffect = pEffectManager->GetCurrentEffectFile();

		pDevice->SetStreamSource(0, 0,0,0);
		pDevice->SetIndices(0);
		

		// culling mode 
		if(selectedBlocks.m_bOnlyRenderClickableArea)
		{
			pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		}
		else
		{
			pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
		}

		if ( pEffect == 0)
		{
#ifdef USE_DIRECTX_RENDERER
			// fixed function pipeline
			pDevice->SetTexture(0,highLightTexture->GetTexture());

			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);
			pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
			pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

			pDevice->SetFVF(mesh_vertex_plain::FVF);

			uint32_t time = CGlobals::GetSceneState()->GetGlobalTime();
			uint8 lightScaler = (bColorAnimate) ?  (uint8)(abs(sin(time*0.0015f)) * 0.4f * 255) : 0xff;
			DWORD lightIntensity = COLOR_ARGB(255, lightScaler, lightScaler, lightScaler);
			pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CONSTANT);
			pDevice->SetTextureStageState( 0, D3DTSS_CONSTANT, lightIntensity);

			Matrix4 vWorldMatrix(Matrix4::IDENTITY);
			vWorldMatrix._41 = - renderBlockOfs_remain.x - (fScaledBlockSize - fBlockSize)*0.5f;
			vWorldMatrix._42 = - renderBlockOfs_remain.y - (fScaledBlockSize - fBlockSize)*0.5f;
			vWorldMatrix._43 = - renderBlockOfs_remain.z - (fScaledBlockSize - fBlockSize)*0.5f;

			pDevice->SetTransform(D3DTS_WORLD, vWorldMatrix.GetPointer());
			

			int nMaxFaceCount = (m_maxSelectBlockPerBatch-1)*6;

			std::map<int64_t, Uint16x3>::iterator itCur, itEnd = selectedBlockMap.end();
			
			for(itCur = selectedBlockMap.begin(); itCur!=itEnd; itCur++)
			{
				Uint16x3& BlockPos = (*itCur).second;
				uint16_t x, y, z;
				x = BlockPos.x;
				y = BlockPos.y;
				z = BlockPos.z;

				if(curInstCount >= nMaxFaceCount)
				{
					RenderDevice::DrawIndexedPrimitiveUP(pDevice, RenderDevice::DRAW_PERF_TRIANGLES_TERRAIN, D3DPT_TRIANGLELIST,0,curInstCount * 4,curInstCount * 2, &(m_select_block_indices[0]), D3DFMT_INDEX16, &(m_select_block_vertices[0]), sizeof(SelectBlockVertex));
					curInstCount = 0;
					instFloatCount = 0;
				}
				Vector3 vOffset((x  - renderBlockOfs_x) * fBlockSize, (y  - renderBlockOfs_y) * fBlockSize + verticalOffset, (z  - renderBlockOfs_z) * fBlockSize);
				int nCount = 0;
				if(selectedBlocks.m_bOnlyRenderClickableArea)
					nCount = FillSelectBlockInvert(selectedBlockMap, x,y,z, &(m_select_block_vertices[instFloatCount*4]), vOffset, fScaledBlockSize);
				else
					nCount = FillSelectBlockVertice(&selectedBlockMap, x,y,z, &(m_select_block_vertices[instFloatCount*4]), vOffset, fScaledBlockSize);

				instFloatCount+=nCount;
				curInstCount+=nCount;
			}

			if(curInstCount > 0)
			{
				RenderDevice::DrawIndexedPrimitiveUP(pDevice, RenderDevice::DRAW_PERF_TRIANGLES_TERRAIN, D3DPT_TRIANGLELIST,0,curInstCount * 4,curInstCount * 2, &(m_select_block_indices[0]), D3DFMT_INDEX16, &(m_select_block_vertices[0]), sizeof(SelectBlockVertex));
			}

			pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			pDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
			pDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);

			pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT);
#endif
		}
		else if(pEffect != 0 && pEffect->begin(false))
		{
			pDevice->SetVertexDeclaration(GetSelectBlockVertexLayout());

			if(pEffect->BeginPass(g_selectBlockPass))
			{
				IScene* pScene = CGlobals::GetEffectManager()->GetScene();
				CBaseCamera* pCamera = pScene->GetCurrentCamera();

				Matrix4 matViewProj;
				if (pCamera)
				{
					const Matrix4& pView = (CGlobals::GetEffectManager()->GetViewTransform());
					const Matrix4& pProj = (CGlobals::GetEffectManager()->GetProjTransform());
					matViewProj = (pView) * (pProj);
				}
							
				pDevice->SetTexture(0,highLightTexture->GetTexture());
					
				pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
				pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_ONE);
				pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

				uint32_t time = CGlobals::GetSceneState()->GetGlobalTime();
				float lightIntensity = 1.f;
				if(bColorAnimate)
					lightIntensity = abs(sin(time*0.0015f)) * 0.4f;
				
				Vector4 vLightParams(lightIntensity, 0,0,0);
				pEffect->setParameter(CEffectFile::k_ConstVector0, (const void*)(&vLightParams));

				Matrix4 vWorldMatrix(Matrix4::IDENTITY);
				vWorldMatrix._41 = - renderBlockOfs_remain.x - (fScaledBlockSize - fBlockSize)*0.5f;
				vWorldMatrix._42 = - renderBlockOfs_remain.y - (fScaledBlockSize - fBlockSize)*0.5f;
				vWorldMatrix._43 = - renderBlockOfs_remain.z - (fScaledBlockSize - fBlockSize)*0.5f;

				vWorldMatrix = vWorldMatrix * matViewProj;

				pEffect->setMatrix(CEffectFile::k_worldViewProjMatrix, &vWorldMatrix);

				pEffect->CommitChanges();

				int nMaxFaceCount = (m_maxSelectBlockPerBatch-1)*6;

				std::map<int64_t, Uint16x3>::iterator itCur, itEnd = selectedBlockMap.end();
				for(itCur = selectedBlockMap.begin(); itCur!=itEnd; itCur++)
				{
					Uint16x3& BlockPos = (*itCur).second;
					uint16_t x, y, z;
					x = BlockPos.x;
					y = BlockPos.y;
					z = BlockPos.z;

					if(curInstCount >= nMaxFaceCount)
					{
						RenderDevice::DrawIndexedPrimitiveUP(pDevice, D3DPT_TRIANGLELIST, D3DPT_TRIANGLELIST, 0, curInstCount * 4, curInstCount * 2, &(m_select_block_indices[0]), D3DFMT_INDEX16, &(m_select_block_vertices[0]), sizeof(SelectBlockVertex));
						curInstCount = 0;
						instFloatCount = 0;
					}

					Vector3 vOffset((x  - renderBlockOfs_x) * fBlockSize, (y  - renderBlockOfs_y) * fBlockSize + verticalOffset, (z  - renderBlockOfs_z) * fBlockSize);
					int nCount = 0;
					if(selectedBlocks.m_bOnlyRenderClickableArea)
						nCount = FillSelectBlockInvert(selectedBlockMap, x,y,z, &(m_select_block_vertices[instFloatCount*4]), vOffset, fScaledBlockSize);
					else
						nCount = FillSelectBlockVertice(&selectedBlockMap, x,y,z, &(m_select_block_vertices[instFloatCount*4]), vOffset, fScaledBlockSize);
					instFloatCount+=nCount;
					curInstCount+=nCount;
				}

				if(curInstCount > 0)
				{
					RenderDevice::DrawIndexedPrimitiveUP(pDevice, D3DPT_TRIANGLELIST, D3DPT_TRIANGLELIST, 0, curInstCount * 4, curInstCount * 2, &(m_select_block_indices[0]), D3DFMT_INDEX16, &(m_select_block_vertices[0]), sizeof(SelectBlockVertex));
				}

				pEffect->EndPass();
				pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
				pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				pDevice->SetRenderState(D3DRS_DEPTHBIAS, 0);
			}
			pEffect->end();
		}
		pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);

	}

	void BlockWorldClient::RenderDamagedBlock()
	{
		if(m_damageDegree <= 0)
			return;
#ifdef USE_DIRECTX_RENDERER
		RenderDevicePtr pDevice = CGlobals::GetRenderDevice();
		float blockSize = BlockConfig::g_blockSize;

		if(!m_damangeTexture)
		{
			return;
		}
		else if(!m_damangeTexture->IsLoaded())
		{
			m_damangeTexture->LoadAsset();
			return;
		}

		//-----------------------------------------
		BuildSelectionBlockBuffer();

		const float fBlockSize = BlockConfig::g_blockSize;
		Vector3 renderOfs  = CGlobals::GetScene()->GetRenderOrigin();
		float verticalOffset = GetVerticalOffset();

		int32_t renderBlockOfs_x = (int32_t)(renderOfs.x/fBlockSize);
		int32_t renderBlockOfs_y = (int32_t)(renderOfs.y/fBlockSize);
		int32_t renderBlockOfs_z = (int32_t)(renderOfs.z/fBlockSize);

		Vector3 renderBlockOfs_remain;
		renderBlockOfs_remain.x = renderOfs.x - renderBlockOfs_x * fBlockSize;
		renderBlockOfs_remain.y = renderOfs.y - renderBlockOfs_y * fBlockSize;
		renderBlockOfs_remain.z = renderOfs.z - renderBlockOfs_z * fBlockSize;

		// scale the block a little to avoid z fighting. 
		float fScaledBlockSize = BlockConfig::g_blockSize*1.02f;

		//-----------------------------------------
		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->BeginEffect(TECH_BLOCK);
		CEffectFile* pEffect = pEffectManager->GetCurrentEffectFile();

		if ( pEffect == 0)
		{
			// fixed function pipeline
			pDevice->SetTexture(0,m_damangeTexture->GetTexture());

			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

			pDevice->SetFVF(mesh_vertex_plain::FVF);

			Matrix4 vWorldMatrix(Matrix4::IDENTITY);
			vWorldMatrix._41 = - renderBlockOfs_remain.x - (fScaledBlockSize - fBlockSize)*0.5f;
			vWorldMatrix._42 = - renderBlockOfs_remain.y - (fScaledBlockSize - fBlockSize)*0.5f;
			vWorldMatrix._43 = - renderBlockOfs_remain.z - (fScaledBlockSize - fBlockSize)*0.5f;

			pDevice->SetTransform(D3DTS_WORLD, vWorldMatrix.GetPointer());

			int curInstCount = 0;
			int instFloatCount = 0;
			{
				Vector3 vOffset((m_damagedBlockId.x  - renderBlockOfs_x) * fBlockSize, (m_damagedBlockId.y  - renderBlockOfs_y) * fBlockSize + verticalOffset, (m_damagedBlockId.z  - renderBlockOfs_z) * fBlockSize);
				FillSelectBlockVertice(NULL, 0,0,0, &(m_select_block_vertices[instFloatCount*24]), vOffset, fScaledBlockSize, ((int)(max(m_damageDegree-0.1f,0.f)*8))/8.f, 1/8.f);
				instFloatCount++;
				curInstCount++;
			}
			RenderDevice::DrawIndexedPrimitiveUP(pDevice, RenderDevice::DRAW_PERF_TRIANGLES_TERRAIN, D3DPT_TRIANGLELIST,0,curInstCount * 24,curInstCount * 12, &(m_select_block_indices[0]), D3DFMT_INDEX16, &(m_select_block_vertices[0]), sizeof(SelectBlockVertex));

			pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			pDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		}
		else if(pEffect != 0 && pEffect->begin(false))
		{
			pDevice->SetVertexDeclaration(GetSelectBlockVertexLayout());

			if(pEffect->BeginPass(2))
			{
				IScene* pScene = CGlobals::GetEffectManager()->GetScene();
				CBaseCamera* pCamera = pScene->GetCurrentCamera();
				Matrix4 matViewProj;
				if (pCamera)
				{
					const Matrix4& pView = (CGlobals::GetEffectManager()->GetViewTransform());
					const Matrix4& pProj = (CGlobals::GetEffectManager()->GetProjTransform());
					matViewProj = (pView) * (pProj);
				}

				pDevice->SetTexture(0,m_damangeTexture->GetTexture());

				pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

				Matrix4 vWorldMatrix(Matrix4::IDENTITY);
				vWorldMatrix._41 = - renderBlockOfs_remain.x - (fScaledBlockSize - fBlockSize)*0.5f;
				vWorldMatrix._42 = - renderBlockOfs_remain.y - (fScaledBlockSize - fBlockSize)*0.5f;
				vWorldMatrix._43 = - renderBlockOfs_remain.z - (fScaledBlockSize - fBlockSize)*0.5f;

				vWorldMatrix = vWorldMatrix*matViewProj;

				pEffect->setMatrix(CEffectFile::k_worldViewProjMatrix, &vWorldMatrix);

				pEffect->CommitChanges();

				int curInstCount = 0;
				int instFloatCount = 0;
				{
					Vector3 vOffset((m_damagedBlockId.x  - renderBlockOfs_x) * fBlockSize, (m_damagedBlockId.y  - renderBlockOfs_y) * fBlockSize + verticalOffset, (m_damagedBlockId.z  - renderBlockOfs_z) * fBlockSize);
					// animate the UV's y offset according to m_damageDegree
					FillSelectBlockVertice(NULL, 0,0,0, &(m_select_block_vertices[instFloatCount*24]), vOffset, fScaledBlockSize, ((int)(max(m_damageDegree-0.1f,0.f)*8))/8.f, 1/8.f);
					instFloatCount++;
					curInstCount++;
				}
				RenderDevice::DrawIndexedPrimitiveUP(pDevice, D3DPT_TRIANGLELIST, D3DPT_TRIANGLELIST,0,curInstCount * 24,curInstCount * 12, &(m_select_block_indices[0]), D3DFMT_INDEX16, &(m_select_block_vertices[0]), sizeof(SelectBlockVertex));
				
				pEffect->EndPass();
				pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
				pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			}
			pEffect->end();
		}
#endif
	}


	int BlockWorldClient::FillSelectBlockInvert(std::map<int64_t, Uint16x3>& selectedBlockMap,  uint16_t x,uint16_t y,uint16_t z, SelectBlockVertex* blockModel, const Vector3& vOffset, const float blockSize, float fUV_Y_Offset /*= 0.f*/, float fUV_Y_Size/*=1.f*/ )
	{
		int nCount = 0;
		
		if(GetBlockTemplateIdByIdx(x,y+1,z)!=0)
		{
			//top face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y+blockSize;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z+blockSize;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x+blockSize;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z+blockSize;
			blockModel[2].texcoord = Vector2(1,fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x+blockSize;
			blockModel[3].position.y = vOffset.y+blockSize;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1,fUV_Y_Offset + fUV_Y_Size);

			blockModel+=4;
			nCount++;
		}
		if(GetBlockTemplateIdByIdx(x,y,z-1)!=0)
		{
			//front face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x+blockSize;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord =Vector2(1.f,fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x+blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel+=4;
			nCount++;
		}
		if(GetBlockTemplateIdByIdx(x,y-1,z)!=0)
		{
			//bottom face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z+blockSize;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x+blockSize;
			blockModel[2].position.y = vOffset.y;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x+blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z+blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel+=4;
			nCount++;
		}
		if(GetBlockTemplateIdByIdx(x-1,y,z)!=0)
		{
			//left face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z+blockSize;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z+blockSize;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel+=4;
			nCount++;
		}
		if(GetBlockTemplateIdByIdx(x+1,y,z)!=0)
		{
			//right
			blockModel[0].position.x = vOffset.x+blockSize;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x+blockSize;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x+blockSize;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z+blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x+blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z+blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel+=4;
			nCount++;
		}
		if(GetBlockTemplateIdByIdx(x,y,z+1)!=0)
		{
			//back face
			blockModel[0].position.x = vOffset.x+blockSize;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z+blockSize;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x+blockSize;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z+blockSize;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z+blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z+blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel+=4;
			nCount++;
		}
		if(nCount == 0)
		{
			nCount = FillSelectBlockVertice(&selectedBlockMap, x,y,z, blockModel, vOffset, blockSize, fUV_Y_Offset, fUV_Y_Size);
		}
		return nCount;
	}

	int BlockWorldClient::FillSelectBlockVertice( std::map<int64_t, Uint16x3>* pSelectedBlockMap, uint16_t x,uint16_t y,uint16_t z, SelectBlockVertex* blockModel, const Vector3& vOffset, const float blockSize, float fUV_Y_Offset, float fUV_Y_Size)
	{
		int nCount = 0;
		if(!pSelectedBlockMap || pSelectedBlockMap->find(GetBlockSparseIndex(x,y+1,z)) == pSelectedBlockMap->end())
		{
			//top face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y+blockSize;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z+blockSize;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x+blockSize;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z+blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x+blockSize;
			blockModel[3].position.y = vOffset.y+blockSize;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel+=4;
			nCount++;
		}
		if(!pSelectedBlockMap || pSelectedBlockMap->find(GetBlockSparseIndex(x,y,z-1)) == pSelectedBlockMap->end())
		{
			//front face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x+blockSize;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x+blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel+=4;
			nCount++;
		}
		if(!pSelectedBlockMap || pSelectedBlockMap->find(GetBlockSparseIndex(x,y-1,z)) == pSelectedBlockMap->end())
		{
			//bottom face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z+blockSize;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x+blockSize;
			blockModel[2].position.y = vOffset.y;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x+blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z+blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel+=4;
			nCount++;
		}
		if(!pSelectedBlockMap || pSelectedBlockMap->find(GetBlockSparseIndex(x-1,y,z)) == pSelectedBlockMap->end())
		{
			//left face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z+blockSize;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z+blockSize;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel+=4;
			nCount++;
		}
		
		if(!pSelectedBlockMap || pSelectedBlockMap->find(GetBlockSparseIndex(x+1,y,z)) == pSelectedBlockMap->end())
		{
			//right
			blockModel[0].position.x = vOffset.x+blockSize;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x+blockSize;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x+blockSize;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z+blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x+blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z+blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel+=4;
			nCount++;
		}
		

		if(!pSelectedBlockMap || pSelectedBlockMap->find(GetBlockSparseIndex(x,y,z+1)) == pSelectedBlockMap->end())
		{
			//back face
			blockModel[0].position.x = vOffset.x+blockSize;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z+blockSize;
			blockModel[0].texcoord = Vector2(0,fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x+blockSize;
			blockModel[1].position.y = vOffset.y+blockSize;
			blockModel[1].position.z = vOffset.z+blockSize;
			blockModel[1].texcoord = Vector2(0,fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x;
			blockModel[2].position.y = vOffset.y+blockSize;
			blockModel[2].position.z = vOffset.z+blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z+blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel+=4;
			nCount++;
		}
		
		return nCount;
	}

	void BlockWorldClient::BuildSelectionBlockBuffer()
	{
		if(m_select_block_vertices.empty())
		{
			float blockSize = BlockConfig::g_blockSize;
			SelectBlockVertex blockModel[24];

			Vector3 vOffset(0,0,0);
			FillSelectBlockVertice(NULL, 0,0,0, blockModel, vOffset, BlockConfig::g_blockSize);

			m_select_block_vertices.resize(24 * m_maxSelectBlockPerBatch);

			SelectBlockVertex* pVertices = &(m_select_block_vertices[0]);
			for(int i=0;i<m_maxSelectBlockPerBatch;i++)
			{
				memcpy(pVertices,blockModel,sizeof(SelectBlockVertex)*24);
				pVertices += 24;
			}
		}
		if(m_select_block_indices.empty())
		{
			m_select_block_indices.resize(36 * m_maxSelectBlockPerBatch);

			uint16_t* pIndices = &(m_select_block_indices[0]);
			
			for(int i = 0; i< m_maxSelectBlockPerBatch; i++)
			{
				uint16_t indexOfs = 24 * i;
				//top face
				*pIndices = indexOfs + 0;
				pIndices++;
				*pIndices = indexOfs + 1;
				pIndices++;
				*pIndices = indexOfs + 3;
				pIndices++;

				*pIndices = indexOfs + 1;
				pIndices++;
				*pIndices = indexOfs + 2;
				pIndices++;
				*pIndices = indexOfs + 3;
				pIndices++;

				//front face
				*pIndices = indexOfs + 4;
				pIndices++;
				*pIndices = indexOfs + 5;
				pIndices++;
				*pIndices = indexOfs + 7;
				pIndices++;

				*pIndices = indexOfs + 5;
				pIndices++;
				*pIndices = indexOfs + 6;
				pIndices++;
				*pIndices = indexOfs + 7;
				pIndices++;

				//bottom face
				*pIndices = indexOfs + 8;
				pIndices++;
				*pIndices = indexOfs + 9;
				pIndices++;
				*pIndices = indexOfs + 11;
				pIndices++;

				*pIndices = indexOfs + 9;
				pIndices++;
				*pIndices = indexOfs + 10;
				pIndices++;
				*pIndices = indexOfs + 11;
				pIndices++;

				//left face
				*pIndices = indexOfs + 12;
				pIndices++;
				*pIndices = indexOfs + 13;
				pIndices++;
				*pIndices = indexOfs + 15;
				pIndices++;

				*pIndices = indexOfs + 13;
				pIndices++;
				*pIndices = indexOfs + 14;
				pIndices++;
				*pIndices = indexOfs + 15;
				pIndices++;

				//right face
				*pIndices = indexOfs + 16;
				pIndices++;
				*pIndices = indexOfs + 17;
				pIndices++;
				*pIndices = indexOfs + 19;
				pIndices++;

				*pIndices = indexOfs + 17;
				pIndices++;
				*pIndices = indexOfs + 18;
				pIndices++;
				*pIndices = indexOfs + 19;
				pIndices++;

				//back face
				*pIndices = indexOfs + 20;
				pIndices++;
				*pIndices = indexOfs + 21;
				pIndices++;
				*pIndices = indexOfs + 23;
				pIndices++;

				*pIndices = indexOfs + 21;
				pIndices++;
				*pIndices = indexOfs + 22;
				pIndices++;
				*pIndices = indexOfs + 23;
				pIndices++;
			}
		}
	}
 
	void BlockWorldClient::AddRenderTask(BlockRenderTask* pRenderTask)
	{
		GetRenderQueueByPass(pRenderTask->GetTemplate()->GetRenderPass())->push_back(pRenderTask);
	}

	IndexBufferDevicePtr_type BlockWorldClient::GetIndexBuffer()
	{
		if(!m_sharedIndexBuffer.IsValid())
		{
			// at most 6 indices per face (2 triangles)
			const int face_count = BlockConfig::g_maxFaceCountPerBatch;
			int32_t bufferSize = sizeof(uint16_t) * 6 * face_count;
			// D3DFMT_INDEX16 boundary check
			PE_ASSERT((face_count * 4) <= 0xffff);
			m_sharedIndexBuffer.CreateBuffer(bufferSize, D3DFMT_INDEX16, 0);

			uint16_t* pIndices;
			if (m_sharedIndexBuffer.Lock((void**)&pIndices, 0, 0))
			{
				for (int i = 0; i < face_count; i++)
				{
					uint16_t indexOfs = 4 * i;
					*pIndices = indexOfs + 0;
					pIndices++;
					*pIndices = indexOfs + 1;
					pIndices++;
					*pIndices = indexOfs + 3;
					pIndices++;

					*pIndices = indexOfs + 1;
					pIndices++;
					*pIndices = indexOfs + 2;
					pIndices++;
					*pIndices = indexOfs + 3;
					pIndices++;
				}
				m_sharedIndexBuffer.Unlock();
			}
		}
		return m_sharedIndexBuffer.GetDevicePointer();
	}
	VertexDeclarationPtr BlockWorldClient::GetVertexLayout()
	{
		return CGlobals::GetEffectManager()->GetVertexDeclaration(EffectManager::S0_POS_NORM_TEX0_COLOR0_COLOR1);
	}

	VertexDeclarationPtr BlockWorldClient::GetSelectBlockVertexLayout()
	{
		return CGlobals::GetEffectManager()->GetVertexDeclaration(EffectManager::S0_POS_TEX0);
	}
	void BlockWorldClient::DeleteDeviceObjects()
	{
		m_sharedIndexBuffer.ReleaseBuffer();
		for(uint32_t i=0;i<m_activeChunks.size();i++)
			m_activeChunks[i]->DeleteDeviceObjects();
		m_pMultiFrameRenderer->DeleteDeviceObjects();
		m_alphaTestRenderTasks.clear();
		m_solidRenderTasks.clear();
		m_alphaBlendRenderTasks.clear();
		m_reflectedWaterRenderTasks.clear();
	}

	void BlockWorldClient::RendererRecreated()
	{
		m_sharedIndexBuffer.RendererRecreated();
		for (uint32_t i = 0; i < m_activeChunks.size(); i++)
			m_activeChunks[i]->RendererRecreated();
		m_pMultiFrameRenderer->RendererRecreated();
		m_alphaTestRenderTasks.clear();
		m_solidRenderTasks.clear();
		m_alphaBlendRenderTasks.clear();
		m_reflectedWaterRenderTasks.clear();
		if (IsInBlockWorld())
			UpdateAllActiveChunks();
	}

	void BlockWorldClient::SetDamagedBlock(uint16_t x,uint16_t y,uint16_t z)
	{
		m_damagedBlockId.x = x;
		m_damagedBlockId.y = y;
		m_damagedBlockId.z = z;
	}

	void BlockWorldClient::SetDamagedBlockDegree(float degree)
	{
		m_damageDegree = degree;
	}

	void BlockWorldClient::SetSelectionTexture( const char* textureName )
	{
		if(textureName)
		{
			char c = textureName[0];
			int nIndex = 0;
			if(textureName[1] == ':' && c>='0' && c<='9')
			{
				nIndex = c-'0';
				textureName+=2;
			}
			if(nIndex<BLOCK_GROUP_ID_MAX)
			{
				if(m_highLightTextures[nIndex])
				{
					if(m_highLightTextures[nIndex]->GetKey() == textureName)
					{
						return;
					}
				}
				m_highLightTextures[nIndex] = CGlobals::GetAssetManager()->LoadTexture("", textureName, TextureEntity::StaticTexture);
			}
		}
	}

	std::string BlockWorldClient::GetSelectionTexture()
	{
		if(m_highLightTextures[0])
		{
			return (m_highLightTextures[0]->GetKey());
		}
		return "";
	}

	void BlockWorldClient::SetDamageTexture( const char* textureName )
	{
		if(m_damangeTexture)
		{
			if(m_damangeTexture->GetKey() == textureName)
			{
				return;
			}
		}
		m_damangeTexture = CGlobals::GetAssetManager()->LoadTexture("", textureName, TextureEntity::StaticTexture);
	}

	std::string BlockWorldClient::GetDamageTexture()
	{
		if(m_damangeTexture)
		{
			return (m_damangeTexture->GetKey());
		}
		return "";
	}

	bool BlockWorldClient::IsPointUnderWater( const Vector3& vPos )
	{
		if(IsInBlockWorld())
		{
			// first check block world and then check real world
			uint16_t block_id = GetBlockTemplateId(vPos.x,vPos.y,vPos.z);
			if(block_id > 0)
			{
				BlockTemplate* pBlockTemplate = GetBlockTemplate(block_id);
				if(pBlockTemplate != 0 && pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_liquid))
				{
					return true;
				}
			}
#ifdef USE_DIRECTX_RENDERER
			return CGlobals::GetOceanManager()->IsPointUnderWater(vPos);
#else
			return false;
#endif
		}
		else
		{
#ifdef USE_DIRECTX_RENDERER
			return CGlobals::GetOceanManager()->IsPointUnderWater(vPos);
#else
			return false;
#endif
		}
	}

	float BlockWorldClient::GetWaterLevel( float x, float y, float z, int nRayLength)
	{

		float fWaterLevel = -1000.f;
#ifdef USE_DIRECTX_RENDERER
		if(m_isInWorld)
		{
			Uint16x3 blockIdx;
			BlockCommon::ConvertToBlockIndex(x,y,z,blockIdx.x,blockIdx.y,blockIdx.z);

			if(blockIdx.y<BlockConfig::g_regionBlockDimY)
			{
				for (int i=0;i<nRayLength; i++)
				{
					uint16_t block_id = GetBlockTemplateIdByIdx(blockIdx.x, blockIdx.y-i, blockIdx.z);
					if(block_id>0)
					{
						BlockTemplate* pBlockTemplate = GetBlockTemplate(block_id);
						if(pBlockTemplate != 0 && (pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_liquid) && !pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_solid)))
						{
							Vector3 vPos = BlockCommon::ConvertToRealPosition(blockIdx.x, blockIdx.y-i, blockIdx.z, 4);
							fWaterLevel = vPos.y - BlockConfig::g_blockSize *0.2f;
						}
						break;
					}
				}
			}

			if(CGlobals::GetOceanManager()->OceanEnabled())
			{
				float fLevel = CGlobals::GetOceanManager()->GetWaterLevel();
				if(fLevel > fWaterLevel)
					fWaterLevel = fLevel;
			}
		}
#endif
		return fWaterLevel;
	}

	void BlockWorldClient::SetBlockRenderMethod( BlockRenderMethod method )
	{
#ifdef USE_DIRECTX_RENDERER
		if(m_dwBlockRenderMethod != method)
		{
			if(method == BLOCK_RENDER_FANCY_SHADER)
			{
				if(CGlobals::GetSettings().GetMultiSampleType()!=0)
				{
					OUTPUT_LOG("BLOCK_RENDER_FANCY_SHADER can not be set with AA\n");
					return;
				}
				// check for device compatibilities
				if( CanUseAdvancedShading() )
				{
					// just ensure that game effect set is not too low. 
					if(CGlobals::GetSettings().GetGameEffectSet() >= 1024)
					{
						CGlobals::GetSettings().LoadGameEffectSet(0);
					}
					// disable glow effect since we are using our own. 
					CGlobals::GetScene()->EnableFullScreenGlow(false);
				}
				else
				{
					OUTPUT_LOG("warning: your device is too low to use deferred shading. \n");
					return;
				}

				CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
				if (pShadowMap !=NULL)
				{
					if(GetUseSunlightShadowMap())
						CGlobals::GetScene()->SetShadow(true);
					else
						CGlobals::GetScene()->SetShadow(false);
				}
				// always disable instancing
				CGlobals::GetScene()->EnableInstancing(false);

				// replacing shader to support multiple render target. 
				if(m_terrain_fancy)
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_TERRAIN, m_terrain_fancy.get());
				}
				if(m_normal_mesh_effect_fancy)
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL, m_normal_mesh_effect_fancy.get());
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_CHARACTER, m_normal_mesh_effect_fancy.get());
				}
				if (m_bmax_model_effect_fancy)
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_BMAX_MODEL, m_bmax_model_effect_fancy.get());
				}
			}
			else
			{
				// reset effect mapping to default values. 
				CGlobals::GetSettings().LoadGameEffectSet(CGlobals::GetSettings().GetGameEffectSet());
			}

			if(m_dwBlockRenderMethod == BLOCK_RENDER_FANCY_SHADER)
			{
				// if switching from fancy shader to plain shader
				// remove all surfaces
				SAFE_RELEASE(m_render_target_block_info_surface);
				SAFE_RELEASE(m_render_target_depth_tex_surface);
				SAFE_RELEASE(m_render_target_normal_surface);
				SAFE_RELEASE(m_pDepthStencilSurface);
				m_render_target_color_hdr.reset();
				m_render_target_color.reset();
				m_render_target_block_info.reset();
				m_render_target_depth_tex.reset();
				m_render_target_normal.reset();
				CGlobals::GetRenderDevice()->SetRenderTarget(1,NULL);
				CGlobals::GetRenderDevice()->SetRenderTarget(2,NULL);
				CGlobals::GetRenderDevice()->SetRenderTarget(3,NULL);
			}
			m_dwBlockRenderMethod = method;

			ClearBlockRenderCache();
		}
#elif defined(USE_OPENGL_RENDERER)
		CGlobals::GetSettings().LoadGameEffectSet(0);
#endif
	}

	void BlockWorldClient::ClearBlockRenderCache()
	{
		if (IsInBlockWorld())
		{
			for (int x = 0; x < m_activeChunkDim; x++)
			{
				for (int z = 0; z < m_activeChunkDim; z++)
				{
					for (int y = 0; y < m_activeChunkDim; y++)
					{
						RenderableChunk& chunk = GetActiveChunk(x, y, z);

						if (!chunk.IsEmptyChunk())
						{
							chunk.SetChunkDirty(true);
						}
					}
				}
			}
		}
	}

	void BlockWorldClient::SetBlockLightColor( const LinearColor& color )
	{
		m_vBlockLightColor = color;
	}

	LinearColor BlockWorldClient::GetBlockLightColor()
	{
		return m_vBlockLightColor;
	}

	bool BlockWorldClient::CompareRenderOrder( BlockRenderTask* v0,BlockRenderTask* v1 )
	{
		return (v0->GetRenderOrder() < v1->GetRenderOrder());
	}

	void BlockWorldClient::InitDeviceObjects()
	{

	}

	void BlockWorldClient::RestoreDeviceObjects()
	{

	}

	void BlockWorldClient::InvalidateDeviceObjects()
	{
#ifdef USE_DIRECTX_RENDERER
		SAFE_RELEASE(m_render_target_block_info_surface);
		SAFE_RELEASE(m_render_target_depth_tex_surface);
		SAFE_RELEASE(m_render_target_normal_surface);
		SAFE_RELEASE(m_pDepthStencilSurface);

		m_render_target_color_hdr.reset();
		m_render_target_color.reset();
		m_render_target_block_info.reset();
		m_render_target_depth_tex.reset();
		m_render_target_normal.reset();
#endif
	}


	void BlockWorldClient::RestoreNormalShader()
	{
		// restore effect to old one. 
		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->EndEffect();
		auto* pEffect = pEffectManager->GetEffectByName("simple_mesh_normal");
		if (pEffect)
		{
			pEffect->LoadAsset();
			pEffectManager->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL, pEffect);
			pEffectManager->MapHandleToEffect(TECH_CHARACTER, pEffect);
		}
		pEffect = pEffectManager->GetEffectByName("BMaxModel");
		if (pEffect)
		{
			pEffect->LoadAsset();
			pEffectManager->MapHandleToEffect(TECH_BMAX_MODEL, pEffect);
		}
		CGlobals::GetSceneState()->SetDeferredShading(true);
	}

	void BlockWorldClient::DoPostRenderingProcessing(BlockRenderPass nRenderPass)
	{
#ifdef USE_DIRECTX_RENDERER
		if(IsInBlockWorld() && GetBlockRenderMethod() ==  BLOCK_RENDER_FANCY_SHADER)
		{
			if (nRenderPass == BlockRenderPass_Opaque)
			{
				if (!m_sPostProcessorCallbackScript.empty())
					CGlobals::GetNPLRuntime()->GetMainRuntimeState()->DoString(m_sPostProcessorCallbackScript.c_str(), (int)m_sPostProcessorCallbackScript.size());
				RenderDynamicBlocks();
				RestoreNormalShader();
			}
			else if (nRenderPass == BlockRenderPass_AlphaBlended)
			{
				if (!m_sPostProcessorAlphaCallbackScript.empty())
					CGlobals::GetNPLRuntime()->GetMainRuntimeState()->DoString(m_sPostProcessorAlphaCallbackScript.c_str(), (int)m_sPostProcessorAlphaCallbackScript.size());
			}
		}
#endif
	}

	bool BlockWorldClient::PrepareAllRenderTargets( bool bSetRenderTarget /*= true*/ )
	{
#ifdef USE_DIRECTX_RENDERER
		if(GetBlockRenderMethod() ==  BLOCK_RENDER_FANCY_SHADER)
		{
			RenderDevicePtr pDevice = CGlobals::GetRenderDevice();

			if(m_block_effect_fancy == 0)
			{
				m_block_effect_fancy = CGlobals::GetAssetManager()->LoadEffectFile("blockFancy", "script/apps/Aries/Creator/Game/Shaders/mrt_blocks.fxo");
				m_block_effect_fancy->LoadAsset();
				if(!m_block_effect_fancy->IsValid())
				{
					// try a lower shader other than fancy 
					return false;
				}
			}
			else if(!m_block_effect_fancy->IsValid())
			{
				// try a lower shader other than fancy 
				return false;
			}

			if (!IsInBlockWorld())
			{
				RestoreNormalShader();
				return false;
			}

			if(m_render_target_block_info == 0)
			{
				m_render_target_block_info = CGlobals::GetAssetManager()->LoadTexture("_BlockInfoRT", "_BlockInfoRT", TextureEntity::RenderTarget);
				m_render_target_block_info->LoadAsset();
				SAFE_RELEASE(m_render_target_block_info_surface);
				m_render_target_block_info->GetTexture()->GetSurfaceLevel(0, &m_render_target_block_info_surface);
			}

			if(m_render_target_depth_tex == 0)
			{
				m_render_target_depth_tex = CGlobals::GetAssetManager()->LoadTexture("_DepthTexRT_R32F", "_DepthTexRT_R32F", TextureEntity::RenderTarget);
				m_render_target_depth_tex->LoadAsset();
				SAFE_RELEASE(m_render_target_depth_tex_surface);
				m_render_target_depth_tex->GetTexture()->GetSurfaceLevel(0, &m_render_target_depth_tex_surface);
			}
			if(m_render_target_normal == 0)
			{
				m_render_target_normal = CGlobals::GetAssetManager()->LoadTexture("_NormalRT", "_NormalRT", TextureEntity::RenderTarget);
				m_render_target_normal->LoadAsset();
				SAFE_RELEASE(m_render_target_normal_surface);
				m_render_target_normal->GetTexture()->GetSurfaceLevel(0, &m_render_target_normal_surface);
			}
			
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_BLOCK_FANCY, m_block_effect_fancy.get());
			if(m_normal_mesh_effect_fancy == 0)
			{
				m_normal_mesh_effect_fancy = CGlobals::GetAssetManager()->LoadEffectFile("mesh_normal_fancy", "script/apps/Aries/Creator/Game/Shaders/mrt_mesh_normal.fxo");
				m_normal_mesh_effect_fancy->LoadAsset();
				if(!m_normal_mesh_effect_fancy->IsValid())
				{
					// try a lower shader other than fancy 
					return false;
				}
			}
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL, m_normal_mesh_effect_fancy.get());
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_CHARACTER, m_normal_mesh_effect_fancy.get());

			if (m_bmax_model_effect_fancy == 0)
			{
				m_bmax_model_effect_fancy = CGlobals::GetAssetManager()->LoadEffectFile("bmax_model_effect_fancy", "script/apps/Aries/Creator/Game/Shaders/mrt_bmax_model.fxo");
				m_bmax_model_effect_fancy->LoadAsset();
				if (!m_bmax_model_effect_fancy->IsValid())
				{
					// try a lower shader other than fancy 
					return false;
				}
			}
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_BMAX_MODEL, m_bmax_model_effect_fancy.get());

			if(m_terrain_fancy == 0)
			{
				m_terrain_fancy = CGlobals::GetAssetManager()->LoadEffectFile("terrain_normal_fancy", "script/apps/Aries/Creator/Game/Shaders/mrt_terrain_normal.fxo");
				m_terrain_fancy->LoadAsset();
				if(!m_terrain_fancy->IsValid())
				{
					// try a lower shader other than fancy 
					return false;
				}
			}
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_TERRAIN, m_terrain_fancy.get());

			if (bSetRenderTarget)
			{
				if (FAILED(pDevice->SetRenderTarget(1, m_render_target_depth_tex_surface)) ||
					FAILED(pDevice->SetRenderTarget(2, m_render_target_block_info_surface)) ||
					FAILED(pDevice->SetRenderTarget(3, m_render_target_normal_surface)))
				{
					return false;
				}

				CGlobals::GetSceneState()->SetDeferredShading(true);

				// clear all other render targets to zero.
				{
					static int nLastFrameCount = 0;
					if (nLastFrameCount != CGlobals::GetSceneState()->GetRenderFrameCount())
					{
						nLastFrameCount = CGlobals::GetSceneState()->GetRenderFrameCount();
						LPDIRECT3DSURFACE9 pOldRenderTarget = NULL;
						pOldRenderTarget = CGlobals::GetDirectXEngine().GetRenderTarget();
						pDevice->SetRenderTarget(3, NULL);
						pDevice->SetRenderTarget(0, m_render_target_normal_surface);
						pDevice->Clear(0L, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0L);
						pDevice->SetRenderTarget(0, pOldRenderTarget);
						pDevice->SetRenderTarget(3, m_render_target_normal_surface);
					}
					CGlobals::GetViewportManager()->GetActiveViewPort()->ApplyViewport();
				}
			}
		}
#endif
		return  true;
	}

	bool BlockWorldClient::DrawMultiFrameBlockWorld()
	{
		if (m_pMultiFrameRenderer)
			return m_pMultiFrameRenderer->Draw();
		return false;
	}
	
	void BlockWorldClient::SetPostProcessingScript(const char* sCallbackScript)
	{
		m_sPostProcessorCallbackScript = sCallbackScript;
	}

	void BlockWorldClient::SetPostProcessingAlphaScript(const char* sCallbackScript)
	{
		m_sPostProcessorAlphaCallbackScript = sCallbackScript;
	}

	bool BlockWorldClient::CanUseAdvancedShading()
	{
#ifdef USE_DIRECTX_RENDERER
		if( CGlobals::GetDirectXEngine().GetVertexShaderVersion()>=3 &&
			CGlobals::GetDirectXEngine().GetPixelShaderVersion()>=3 &&
			CGlobals::GetDirectXEngine().m_d3dCaps.NumSimultaneousRTs >=3)
		{
			return true;
		}
#endif
		return false;
	}

	void BlockWorldClient::SetUseSunlightShadowMap( bool bEnable )
	{
		m_bUseSunlightShadowMap = bEnable;
#ifdef USE_DIRECTX_RENDERER
		if(GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER)
		{
			CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
			if (pShadowMap !=NULL)
			{
				CGlobals::GetScene()->SetShadow(m_bUseSunlightShadowMap);
			}
		}
#endif
	}

	bool BlockWorldClient::HasSunlightShadowMap()
	{
		return GetUseSunlightShadowMap() && CGlobals::GetScene()->IsShadowMapEnabled();
	}

	bool BlockWorldClient::GetUseSunlightShadowMap()
	{
		return m_bUseSunlightShadowMap;
	}

	void BlockWorldClient::SetUseWaterReflection( bool bEnable )
	{
		m_bUseWaterReflection = bEnable;
	}

	bool BlockWorldClient::GetUseWaterReflection()
	{
		return m_bUseWaterReflection;
	}

	void BlockWorldClient::PrepareShadowCasters( CShadowMap* pShadowMap )
	{
#ifdef USE_DIRECTX_RENDERER
		if(GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER && GetUseSunlightShadowMap())
		{
			// TODO: add all shadow caster chunks within shadow radius. 
			CShapeAABB aabb;
			Vector3 camWorldPos = CGlobals::GetSceneState()->vEye;
			Vector3 renderOrig = CGlobals::GetScene()->GetRenderOrigin();
			Vector3 vCenter = camWorldPos - renderOrig;
			float fRadius = min(CGlobals::GetScene()->GetShadowRadius(), 50.f);
			Vector3 vExtent(fRadius, fRadius, fRadius);
			aabb.SetCenterExtents(vCenter, vExtent);
			pShadowMap->AddShadowCasterPoint(aabb);
		}
#endif
	}
	
	void BlockWorldClient::RenderDynamicBlocks()
	{
		// no need to lock 
		
		int nCount = (int)m_selectedBlockMap.size();
		for(int i=0; i<nCount; ++i)
		{
			auto& select_group = m_selectedBlockMap[i];
			if(select_group.GetBlocks().size() > 0)
			{
				if(select_group.m_bWireFrame)
					RenderWireFrameBlock(i, select_group.m_fScaling, &select_group.m_color);
				else
					RenderSelectionBlock(i, select_group.m_fScaling, &select_group.m_color, select_group.m_bEnableBling);
			}
		}
		
		if(m_damageDegree > 0)
			RenderDamagedBlock();
	}

	BlockWorldClient* BlockWorldClient::GetInstance()
	{
		return g_pInstance;
	}

	void BlockWorldClient::GetMaxBlockHeightWatchingSky(uint16_t blockX_ws, uint16_t blockZ_ws, ChunkMaxHeight* pResult)
	{
		CBlockWorld::GetMaxBlockHeightWatchingSky(blockX_ws, blockZ_ws, pResult);
#ifdef PARAENGINE_CLIENT
		ParaTerrain::CGlobalTerrain *pTerrain = CGlobals::GetGlobalTerrain();
		uint16_t mask = 0;
		if (pTerrain->TerrainRenderingEnabled())
		{
			for (int i = 0; i < 5; i++)
			{
				float x, z;
				if (i == 0)
				{
					x = blockX_ws * BlockConfig::g_blockSize;
					z = blockZ_ws * BlockConfig::g_blockSize;
				}
				else if (i == 1)
				{
					x = (blockX_ws + 1) * BlockConfig::g_blockSize;
					z = blockZ_ws * BlockConfig::g_blockSize;
				}
				else if (i == 2)
				{
					x = (blockX_ws - 1) * BlockConfig::g_blockSize;
					z = blockZ_ws * BlockConfig::g_blockSize;
				}
				else if (i == 3)
				{
					x = blockX_ws * BlockConfig::g_blockSize;
					z = (blockZ_ws + 1) * BlockConfig::g_blockSize;
				}
				else
				{
					x = blockX_ws * BlockConfig::g_blockSize;
					z = (blockZ_ws - 1) * BlockConfig::g_blockSize;
				}

				float elevation = pTerrain->GetElevation(x, z);
				if (elevation != -FLOAT_POS_INFINITY)
				{
					elevation -= GetVerticalOffset();
					int32 terrainBlockHeight = (int32)(elevation / BlockConfig::g_blockSize);
					if (terrainBlockHeight < 0)
						terrainBlockHeight = 0;
					if (terrainBlockHeight > BlockConfig::g_regionBlockDimY)
						terrainBlockHeight = BlockConfig::g_regionBlockDimY;

					if (terrainBlockHeight > pResult[i].GetMaxHeight())
					{
						pResult[i].SetHeight((int16)terrainBlockHeight);
						mask |= (1 << i);
					}
				}
			}
		}
		pResult[5].SetHeight(mask);
#endif
	}

	void BlockWorldClient::UpdateVisibleChunks(bool bIsShadowPass /*= false*/)
	{
		m_visibleChunks.clear();
		// bool is_perspective = CGlobals::GetScene()->GetCurrentCamera()->IsPerspectiveView();

		Vector3 camWorldPos = CGlobals::GetSceneState()->vEye;
		Vector3 renderOrig = CGlobals::GetScene()->GetRenderOrigin();
		int nRenderFrameCount = CGlobals::GetSceneState()->GetRenderFrameCount();

		CCameraFrustum*  frustum;
		static CCameraFrustum caster_frustum;

		Vector3 camMin;
		Vector3 camMax;

		if (bIsShadowPass)
		{
#ifdef USE_DIRECTX_RENDERER
			frustum = &caster_frustum;
			Vector3 vCenter = camWorldPos - renderOrig;
			float fRadius = min(CGlobals::GetScene()->GetShadowRadius(), 50.f);

			frustum->UpdateFrustum(CGlobals::GetEffectManager()->GetShadowMap()->GetViewProjMatrix());
			camMin = Vector3(vCenter.x - fRadius, vCenter.y - fRadius, vCenter.z - fRadius);
			camMax = Vector3(vCenter.x + fRadius, vCenter.y + fRadius, vCenter.z + fRadius);
#else	
			return;
#endif
		}
		else
		{
			frustum = CGlobals::GetScene()->GetCurrentCamera()->GetFrustum();

			// calculate aabb for frustum
			Vector3* frusCorner = frustum->vecFrustum;
			camMin = frusCorner[0];
			camMax = frusCorner[0];

			for (int i = 1; i<8; i++)
			{
				Vector3& v = frusCorner[i];

				if (camMin.x > v.x)
					camMin.x = v.x;
				if (camMin.y > v.y)
					camMin.y = v.y;
				if (camMin.z > v.z)
					camMin.z = v.z;

				if (camMax.x < v.x)
					camMax.x = v.x;
				if (camMax.y < v.y)
					camMax.y = v.y;
				if (camMax.z < v.z)
					camMax.z = v.z;
			}
		}
		camMax += renderOrig;
		camMin += renderOrig;

		Uint16x3 camBlockPos;
		BlockCommon::ConvertToBlockIndex(camWorldPos.x, camWorldPos.y, camWorldPos.z, camBlockPos.x, camBlockPos.y, camBlockPos.z);
		SetEyeBlockId(camBlockPos);
		
		Uint16x3 startIdx, endIdx;
		BlockCommon::ConvertToBlockIndex(camMin.x, camMin.y, camMin.z, startIdx.x, startIdx.y, startIdx.z);
		startIdx.x /= 16; startIdx.y /= 16; startIdx.z /= 16;
		startIdx.x = startIdx.x > 0 ? startIdx.x - 1 : 0;
		if (m_minActiveChunkId_ws.x > startIdx.x)
			startIdx.x = m_minActiveChunkId_ws.x;
		startIdx.y = startIdx.y > 0 ? (startIdx.y <= 15 ? startIdx.y - 1 : 15) : 0;
		startIdx.z = startIdx.z > 0 ? startIdx.z - 1 : 0;
		if (m_minActiveChunkId_ws.z > startIdx.z)
			startIdx.z = m_minActiveChunkId_ws.z;
		BlockCommon::ConvertToBlockIndex(camMax.x, camMax.y, camMax.z, endIdx.x, endIdx.y, endIdx.z);
		endIdx.x /= 16; endIdx.y /= 16; endIdx.z /= 16;
		if ((m_minActiveChunkId_ws.x + m_activeChunkDim) < endIdx.x)
			endIdx.x = m_minActiveChunkId_ws.x + m_activeChunkDim;
		endIdx.y = endIdx.y < 15 ? endIdx.y + 1 : 15;
		if ((m_minActiveChunkId_ws.z + m_activeChunkDim) < endIdx.z)
			endIdx.z = m_minActiveChunkId_ws.z + m_activeChunkDim;

		float fRenderDist = GetRenderDist() * BlockConfig::g_blockSize;
		CShapeSphere sEyeSphere(camWorldPos, fRenderDist);

		// cull and update each chunk
		// we will add using a spiral rectangle path from the center.
		int32 nIndex = 0;
		int32 dx = 0;
		int32 dz = 0;

		int32 chunkX = GetEyeChunkId().x;
		int32 chunkY = GetEyeChunkId().y;
		int32 chunkZ = GetEyeChunkId().z;
		int32 chunkViewRadius = (int32)(GetRenderDist() / 16);
		int32 chunkViewSize = chunkViewRadius * 2;
		
		Vector3 vChunkSize(BlockConfig::g_chunkSize, BlockConfig::g_chunkSize, BlockConfig::g_chunkSize);

		// OUTPUT_LOG("---------------cx %d, cz %d\n", chunkX, chunkZ);

		for (uint16 y = startIdx.y; y <= endIdx.y; y++)
		{
			uint16 x = (uint16)(chunkX);
			uint16 z = (uint16)(chunkZ);
			RenderableChunk& chunk = GetActiveChunk(x, y, z);
			if (x >= startIdx.x && x <= endIdx.x && z >= startIdx.z && z <= endIdx.z && chunk.IsIntersect(sEyeSphere))
			{
				Vector3 vMin = BlockCommon::ConvertToRealPosition(x * 16, y * 16, z * 16, 7);
				vMin -= renderOrig;
				Vector3 vMax = vMin + vChunkSize;

				CShapeAABB box;
				box.SetMinMax(vMin, vMax);
				if (frustum->TestBox(&box) && chunk.IsNearbyChunksLoaded())
				{
					AddToVisibleChunk(chunk, 0, nRenderFrameCount);
				}
			}
		}

		for (int32 length = 1; length <= chunkViewSize; ++length)
		{
			for (int32 k = 1; k <= 2; ++k)
			{
				int32* dir = &(BlockCommon::g_xzDirectionsConst[(nIndex % 4)][0]);
				nIndex++;
            
				for (int32 i = 0; i < length; ++i)
				{
					dx = dx + dir[0];
					dz = dz + dir[1];

					//OUTPUT_LOG("%d, %d\n", dx, dz);
					for (uint16 y = startIdx.y; y <= endIdx.y; y++)
					{
						uint16 x = (uint16)(chunkX + dx);
						uint16 z = (uint16)(chunkZ + dz);
						RenderableChunk& chunk = GetActiveChunk(x, y, z);
						if (x >= startIdx.x && x <= endIdx.x && z >= startIdx.z && z <= endIdx.z && chunk.IsIntersect(sEyeSphere))
						{
							Vector3 vMin = BlockCommon::ConvertToRealPosition(x * 16, y * 16, z * 16, 7);
							vMin -= renderOrig;
							Vector3 vMax = vMin + vChunkSize;

							CShapeAABB box;
							box.SetMinMax(vMin, vMax);
							if (frustum->TestBox(&box) && chunk.IsNearbyChunksLoaded())
							{
								AddToVisibleChunk(chunk, length, nRenderFrameCount);
							}
						}
					}
				}
			}
		}

		nIndex = (nIndex % 4);

		for (int32 i = 1; i <= chunkViewSize; ++i)
		{
			dx = dx + BlockCommon::g_xzDirectionsConst[nIndex][0];
			dz = dz + BlockCommon::g_xzDirectionsConst[nIndex][1];
			
			for (uint16_t y = startIdx.y; y <= endIdx.y; y++)
			{
				uint16 x = (uint16)(chunkX + dx);
				uint16 z = (uint16)(chunkZ + dz);
				RenderableChunk& chunk = GetActiveChunk(x, y, z);
				if (x >= startIdx.x && x <= endIdx.x && z >= startIdx.z && z <= endIdx.z && chunk.IsIntersect(sEyeSphere))
				{
					Vector3 vMin = BlockCommon::ConvertToRealPosition(x * 16, y * 16, z * 16, 7);
					vMin -= renderOrig;
					Vector3 vMax = vMin + vChunkSize;

					CShapeAABB box;
					box.SetMinMax(vMin, vMax);
					if (frustum->TestBox(&box) && chunk.IsNearbyChunksLoaded())
					{
						AddToVisibleChunk(chunk, chunkViewSize, nRenderFrameCount);
					}
				}
			}
		}
		
		if (!bIsShadowPass)
			m_isVisibleChunkDirty = false;
	}


	void BlockWorldClient::AddToVisibleChunk(RenderableChunk &chunk, int nViewDist, int nRenderFrameCount)
	{
		chunk.SetChunkViewDistance((int16)nViewDist);
		chunk.SetViewIndex((int16)m_visibleChunks.size());
		chunk.SetRenderFrameCount(nRenderFrameCount);
		m_visibleChunks.push_back(&chunk);
	}

	void BlockWorldClient::ClearVisibleChunksToByteLimit(bool bIsShadowPass)
	{
		int nMaxChunkIndex = (int)m_visibleChunks.size();

		if (!bIsShadowPass)
		{
			int nCurrentVertexBufferSize = 0;
			for (int i = 0; i < nMaxChunkIndex; i++)
			{
				RenderableChunk* pRenderChunk = m_visibleChunks[i];
				int nBufferSize = pRenderChunk->GetLastBufferBytes();
				nCurrentVertexBufferSize += nBufferSize;
				if (nCurrentVertexBufferSize > GetMaxVisibleVertexBufferBytes())
				{
					nMaxChunkIndex = i + 1;
					m_visibleChunks.resize(nMaxChunkIndex);
					break;
				}
			}
		}
	}

	int BlockWorldClient::ClearActiveChunksToMemLimit(bool bIsShadowPass)
	{
		int nMaxChunkIndex = (int)m_visibleChunks.size();

		int nUsedBytes = (int)(RenderableChunk::GetVertexBufferPool()->GetTotalBufferBytes());
		
		int nBytesToRemove = nUsedBytes - GetVertexBufferSizeLimit();
		if (nBytesToRemove > 0)
		{
			if (bIsShadowPass)
				return 0;
			static std::set<RenderableChunk*> m_safeVisibleChunks;
			static std::deque<RenderableChunk*> m_removableChunks;
			m_safeVisibleChunks.clear();
			m_removableChunks.clear();
			int nCurrentRenderFrame = (m_visibleChunks.size() > 0) ? m_visibleChunks[0]->GetRenderFrameCount() : 0;

			if (GetAlwaysInVertexBufferChunkRadius() > 0)
			{
				// add all always in cache chunks to safe chunks. 
				int32 chunkX = GetEyeChunkId().x;
				int32 chunkY = GetEyeChunkId().y;
				int32 chunkZ = GetEyeChunkId().z;
				int32 nCacheRadius = GetAlwaysInVertexBufferChunkRadius();

				for (int x = chunkX - nCacheRadius; x <= (chunkX + nCacheRadius); x++)
				{
					for (int y = chunkY - nCacheRadius; y <= (chunkY + nCacheRadius); y++)
					{
						for (int z = chunkZ - nCacheRadius; z <= (chunkZ + nCacheRadius); z++)
						{
							if (x >= 0 && y >= 0 && z >= 0)
							{
								RenderableChunk& chunk = GetActiveChunk((uint16)x, (uint16)y, (uint16)z);
								m_safeVisibleChunks.insert(&chunk);
							}
						}
					}
				}
			}
			int nCurrentVertexBufferSize = 0;
			for (int i = 0; i < nMaxChunkIndex; i++)
			{
				RenderableChunk* pRenderChunk = m_visibleChunks[i];
				int nBufferSize = pRenderChunk->GetLastBufferBytes();
				nCurrentVertexBufferSize += nBufferSize;
				m_safeVisibleChunks.insert(pRenderChunk);
				if (nCurrentVertexBufferSize > GetVertexBufferSizeLimit())
				{
					nMaxChunkIndex = i + 1;
					// if memory is tight, remove chunks that is out of memory range. 
					ChunkVertexBuilderManager::GetInstance().RemovePendingChunks(&m_safeVisibleChunks);
					break;
				}
			}

			for (int z = 0; z < m_activeChunkDim; z++)
			{
				int16_t curChunkWZ = m_minActiveChunkId_ws.z + z;
				for (int x = 0; x < m_activeChunkDim; x++)
				{
					int16_t curChunkWX = m_minActiveChunkId_ws.x + x;
					{
						for (int y = 0; y < m_activeChunkDimY; y++)
						{
							int16_t curChunkWY = m_minActiveChunkId_ws.y + y;

							RenderableChunk* pChunk = &GetActiveChunk(curChunkWX, curChunkWY, curChunkWZ);
							int nBufferSize = pChunk->GetVertexBufferBytes();
							if (nBufferSize > 0 && m_safeVisibleChunks.find(pChunk) == m_safeVisibleChunks.end())
							{
								if (nCurrentVertexBufferSize < GetVertexBufferSizeLimit())
								{
									// visible chunks has not exceed memory limit, we will put unused chunks to a sorted queue according to render frame count. 
									if ( m_removableChunks.empty() 
										|| m_removableChunks.back()->GetRenderFrameCount() > pChunk->GetRenderFrameCount()
										|| (nCurrentRenderFrame == pChunk->GetRenderFrameCount() && m_removableChunks.back()->GetChunkViewDistance() < pChunk->GetChunkViewDistance()) ) 
										m_removableChunks.push_back(pChunk);
									else
										m_removableChunks.push_front(pChunk);
								}
								else
								{
									// visible chunks already consumes too much memory, we will remove everything else
									if (nBytesToRemove > nBufferSize)
									{
										nBytesToRemove -= nBufferSize;
										pChunk->ClearChunkData();
									}
								}
							}
						}
					}
				}
			}

			// m_removableChunks is sorted from newest chunk to oldest chunk.
			// we will remove oldest chunks first. 
			while (!m_removableChunks.empty())
			{
				RenderableChunk* pChunk = m_removableChunks.back();
				int nBufferSize = pChunk->GetVertexBufferBytes();
				if (nBytesToRemove > nBufferSize)
				{
					nBytesToRemove -= nBufferSize;
					pChunk->ClearChunkData();
					m_removableChunks.pop_back();
				}
				else
					break;
			}
			m_removableChunks.clear();
		}
		return nMaxChunkIndex;
	}

	void BlockWorldClient::CheckRebuildVisibleChunks(bool bAsyncMode, bool bIsShadowPass)
	{
		int nMaxChunkIndex = ClearActiveChunksToMemLimit(bIsShadowPass);

		m_tempDirtyChunks.clear();
		for (int i = 0; i < nMaxChunkIndex; i++)
		{
			RenderableChunk* pRenderChunk = m_visibleChunks[i];
			BlockChunk* pChunk = pRenderChunk->GetChunk();
			if (pChunk/* && pChunk->IsLightingInitialized()*/)
			{
				if (pRenderChunk->ShouldRebuildRenderBuffer(this, true, false))
				{
					// new buffer first
					int nLastDelayTick = pRenderChunk->GetDelayedRebuildTick();
					int nViewDist = pRenderChunk->GetChunkViewDistance();
					if (nViewDist <= m_nNearCameraChunkDist)
						pRenderChunk->SetDelayedRebuildTick(nLastDelayTick + 3);
					else
						pRenderChunk->SetDelayedRebuildTick(nLastDelayTick + 1);
					m_tempDirtyChunks.push_back(pRenderChunk);
				}
				else if (pRenderChunk->ShouldRebuildRenderBuffer(this, false, true))
				{
					// check update buffer. 
					int nLastDelayTick = pRenderChunk->GetDelayedRebuildTick();
					int nViewDist = pRenderChunk->GetChunkViewDistance();
					if (nViewDist <= m_nNearCameraChunkDist)
						pRenderChunk->SetDelayedRebuildTick(nLastDelayTick + 2);
					else
						pRenderChunk->SetDelayedRebuildTick(nLastDelayTick + 1);
					m_tempDirtyChunks.push_back(pRenderChunk);
				}
			}
		}

		std::stable_sort(m_tempDirtyChunks.begin(), m_tempDirtyChunks.end(), [](RenderableChunk* a, RenderableChunk* b) {
			return (a->GetDelayedRebuildTick() > b->GetDelayedRebuildTick());
		});

		int nMaxBufferRebuildPerTick = GetMaxBufferRebuildPerTick();
		for (RenderableChunk * pRenderChunk : m_tempDirtyChunks)
		{
			if (pRenderChunk->RebuildRenderBuffer(this, bAsyncMode))
			{
#ifdef PRINT_CHUNK_LOG
				if (pRenderChunk->IsDirty())
					OUTPUT_LOG("rebuild New chunk: pos(%d %d %d) ViewDist:%d  DelayCount:%d\n", pRenderChunk->GetChunkPosWs().x, pRenderChunk->GetChunkPosWs().y, pRenderChunk->GetChunkPosWs().z, pRenderChunk->GetChunkViewDistance(), pRenderChunk->GetDelayedRebuildTick());
				else
					OUTPUT_LOG("rebuild chunk: pos(%d %d %d) ViewDist:%d  DelayCount:%d\n", pRenderChunk->GetChunkPosWs().x, pRenderChunk->GetChunkPosWs().y, pRenderChunk->GetChunkPosWs().z, pRenderChunk->GetChunkViewDistance(), pRenderChunk->GetDelayedRebuildTick());
#endif
				m_nBufferRebuildCountThisTick++;
				if (pRenderChunk->GetChunkViewDistance() > GetNearCameraChunkDist()){
					// for far away blocks, we will only rebuild 1 per tick, instead of 3. just in case the view distance is set really high. 
					nMaxBufferRebuildPerTick = GetMaxBufferRebuildPerTick_FarChunk();
				}
				if (bAsyncMode && m_nBufferRebuildCountThisTick >= nMaxBufferRebuildPerTick)
					break;
			}
			else
				break;
		}
		m_tempDirtyChunks.clear();
	}

	std::vector<BlockRenderTask*>* BlockWorldClient::GetRenderQueueByPass(BlockRenderPass nRenderPass)
	{
		if (nRenderPass == BlockRenderPass_Opaque)
			return &m_solidRenderTasks;
		else if (nRenderPass == BlockRenderPass_AlphaTest)
			return &m_alphaTestRenderTasks;
		else if (nRenderPass == BlockRenderPass_ReflectedWater)
			return &m_reflectedWaterRenderTasks;
		else
			return &m_alphaBlendRenderTasks;
	}
	
	bool BlockWorldClient::IsMovieOutputMode() const
	{
		return m_bMovieOutputMode;
	}

	void BlockWorldClient::EnableMovieOutputMode(bool val)
	{
#ifdef WIN32
		if (m_bMovieOutputMode != val)
		{
			static int s_lastMaxCacheRegionCount = m_maxCacheRegionCount;
			static int s_nMaxBufferRebuildPerTick = m_nMaxBufferRebuildPerTick;
			static int s_nNearCameraChunkDist = m_nNearCameraChunkDist;
			static int s_nMaxBufferRebuildPerTick_FarChunk = m_nMaxBufferRebuildPerTick_FarChunk;

			m_bMovieOutputMode = val;
			if (m_bMovieOutputMode)
			{
				s_lastMaxCacheRegionCount = m_maxCacheRegionCount;
				s_nMaxBufferRebuildPerTick = m_nMaxBufferRebuildPerTick;
				s_nNearCameraChunkDist = m_nNearCameraChunkDist;
				s_nMaxBufferRebuildPerTick_FarChunk = m_nMaxBufferRebuildPerTick_FarChunk;
				m_maxCacheRegionCount = 64;
				m_nMaxBufferRebuildPerTick = 100;
				m_nNearCameraChunkDist = 20;
				m_nMaxBufferRebuildPerTick_FarChunk = 100;

				ChunkVertexBuilderManager::GetInstance().m_nMaxPendingChunks = 100;
				ChunkVertexBuilderManager::GetInstance().m_nMaxUploadingChunks = 100;
				ChunkVertexBuilderManager::GetInstance().m_nMaxChunksToUploadPerTick = 100;

				OUTPUT_LOG("movie output mode is on\n");
			}
			else
			{
				m_maxCacheRegionCount = s_lastMaxCacheRegionCount;
				m_nMaxBufferRebuildPerTick = s_nMaxBufferRebuildPerTick;
				m_nNearCameraChunkDist = s_nNearCameraChunkDist;
				m_nMaxBufferRebuildPerTick_FarChunk = s_nMaxBufferRebuildPerTick_FarChunk;

				ChunkVertexBuilderManager::GetInstance().m_nMaxPendingChunks = 4;
				ChunkVertexBuilderManager::GetInstance().m_nMaxUploadingChunks = 4;
				ChunkVertexBuilderManager::GetInstance().m_nMaxChunksToUploadPerTick = 8;

				OUTPUT_LOG("movie output mode is off\n");
			}
		}
#endif
	}

	void BlockWorldClient::UpdateActiveChunk()
	{
		CBlockWorld::UpdateActiveChunk();
	}
	
	int BlockWorldClient::GetVertexBufferSizeLimit() const
	{
		return m_nVertexBufferSizeLimit;
	}

	void BlockWorldClient::SetVertexBufferSizeLimit(int val)
	{
		m_nVertexBufferSizeLimit = val;
	}

	IAttributeFields* BlockWorldClient::GetChildAttributeObject(const std::string& sName)
	{
		if (sName == "ChunkVertexBuilderManager")
		{
			return &(ChunkVertexBuilderManager::GetInstance());
		}
		else if (sName == "LightGrid")
		{
			return &(GetLightGrid());
		}
		else if (sName == "CMultiFrameBlockWorldRenderer")
		{
			return m_pMultiFrameRenderer;
		}
		else
			return CBlockWorld::GetChildAttributeObject(sName);
	}

	int BlockWorldClient::GetChildAttributeColumnCount()
	{
		return 2;
	}

	IAttributeFields* BlockWorldClient::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
	{
		if (nColumnIndex == 1)
		{
			if (nRowIndex == 0)
				return &(GetLightGrid());
			else if (nRowIndex == 1)
				return &(ChunkVertexBuilderManager::GetInstance());
			else if (nRowIndex == 2)
				return m_pMultiFrameRenderer;
		}
		return CBlockWorld::GetChildAttributeObject(nRowIndex, nColumnIndex);
	}

	int BlockWorldClient::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
	{
		if (nColumnIndex == 1)
		{
			return 3;
		}
		return CBlockWorld::GetChildAttributeObjectCount(nColumnIndex);
	}

	int BlockWorldClient::GetAlwaysInVertexBufferChunkRadius() const
	{
		return m_nAlwaysInVertexBufferChunkRadius;
	}

	void BlockWorldClient::SetAlwaysInVertexBufferChunkRadius(int val)
	{
		m_nAlwaysInVertexBufferChunkRadius = val;
	}

	int BlockWorldClient::GetMaxVisibleVertexBufferBytes() const
	{
		return m_nMaxVisibleVertexBufferBytes;
	}

	void BlockWorldClient::SetMaxVisibleVertexBufferBytes(int val)
	{
		m_nMaxVisibleVertexBufferBytes = val;
	}

	bool BlockWorldClient::IsAsyncChunkMode() const
	{
		return m_bAsyncChunkMode;
	}

	void BlockWorldClient::SetAsyncChunkMode(bool val)
	{
		if (m_bAsyncChunkMode != val)
		{
			m_bAsyncChunkMode = val;
			if (!m_bAsyncChunkMode)
			{

			}
		}
	}

	int BlockWorldClient::GetMaxBufferRebuildPerTick() const
	{
		return m_nMaxBufferRebuildPerTick;
	}

	void BlockWorldClient::SetMaxBufferRebuildPerTick(int val)
	{
		m_nMaxBufferRebuildPerTick = val;
	}

	int BlockWorldClient::GetNearCameraChunkDist() const
	{
		return m_nNearCameraChunkDist;
	}

	void BlockWorldClient::SetNearCameraChunkDist(int val)
	{
		m_nNearCameraChunkDist = val;
	}

	int BlockWorldClient::GetMaxBufferRebuildPerTick_FarChunk() const
	{
		return m_nMaxBufferRebuildPerTick_FarChunk;
	}

	void BlockWorldClient::SetMaxBufferRebuildPerTick_FarChunk(int val)
	{
		m_nMaxBufferRebuildPerTick_FarChunk = val;
		if (m_nMaxBufferRebuildPerTick_FarChunk > GetMaxBufferRebuildPerTick())
			SetMaxBufferRebuildPerTick(m_nMaxBufferRebuildPerTick_FarChunk);
	}

	bool BlockWorldClient::DrawMultiFrameBlockWorldOnSky()
	{
		return m_pMultiFrameRenderer->DrawToSkybox();
	}


	void BlockWorldClient::RenderDeferredLights()
	{
#ifdef USE_DIRECTX_RENDERER
		SceneState* sceneState = CGlobals::GetSceneState();
		if (!sceneState->IsDeferredShading() || sceneState->listDeferredLightObjects.empty())
			return;

		// sort by light type
		std::sort(sceneState->listDeferredLightObjects.begin(), sceneState->listDeferredLightObjects.end(), [](CLightObject* a, CLightObject* b){
			return (a->GetLightType() < b->GetLightType());
		});

		// setup shader here
		for (int i = 0; i < 3; i++)
		{
			if (m_lightgeometry_effects[i] == 0)
			{
				if (i == 0)
					m_lightgeometry_effects[i] = CGlobals::GetAssetManager()->LoadEffectFile("blockFancy", "script/apps/Aries/Creator/Game/Shaders/DeferredShadingPointLighting.fxo");
				else if (i == 2)
					m_lightgeometry_effects[i] = CGlobals::GetAssetManager()->LoadEffectFile("blockFancy", "script/apps/Aries/Creator/Game/Shaders/DeferredShadingSpotLighting.fxo");
				else 
					m_lightgeometry_effects[i] = CGlobals::GetAssetManager()->LoadEffectFile("blockFancy", "script/apps/Aries/Creator/Game/Shaders/DeferredShadingDirectionalLighting.fxo");
				
				m_lightgeometry_effects[i]->LoadAsset();
			}
			if (!m_lightgeometry_effects[i] || ! m_lightgeometry_effects[i]->IsValid())
				return;
		}

		// TODO: setup render target here
		{


		}

		// sort by type and render light geometry
		int nLastType = -1;

		CGlobals::GetEffectManager()->EndEffect();
		CEffectFile* pEffectFile = NULL;
		for (CLightObject* lightObject : sceneState->listDeferredLightObjects)
		{
			if (lightObject->GetLightType() != nLastType)
			{
				if (pEffectFile) 
				{
					pEffectFile->end();
				}
				else 
				{
					// first time, we will switch to declaration
					auto pDecl = CGlobals::GetEffectManager()->GetVertexDeclaration(EffectManager::S0_POS);
					if (pDecl)
						CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
				}
				pEffectFile = m_lightgeometry_effects[lightObject->GetLightType() - 1].get();
				pEffectFile->begin();
			}
			lightObject->RenderDeferredLightMesh(sceneState);
		}
		if (pEffectFile) {
			pEffectFile->end();
		}
#endif
	}

	int BlockWorldClient::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		// install parent fields if there are any. Please replace CBlockWorld with your parent class name.
		CBlockWorld::InstallFields(pClass, bOverride);

		PE_ASSERT(pClass != NULL);

		pClass->AddField("BlockLightColor", FieldType_Vector3, (void*)SetBlockLightColor_s, (void*)GetBlockLightColor_s, NULL, NULL, bOverride);

		pClass->AddField("PostProcessingScript", FieldType_String, (void*)SetPostProcessingScript_s, NULL, NULL, NULL, bOverride);
		pClass->AddField("PostProcessingAlphaScript", FieldType_String, (void*)SetPostProcessingAlphaScript_s, NULL, NULL, NULL, bOverride);
		pClass->AddField("HasSunlightShadowMap", FieldType_Bool, (void*)0, (void*)HasSunlightShadowMap_s, NULL, NULL, bOverride);
		pClass->AddField("UseSunlightShadowMap", FieldType_Bool, (void*)SetUseSunlightShadowMap_s, (void*)GetUseSunlightShadowMap_s, NULL, NULL, bOverride);
		pClass->AddField("UseWaterReflection", FieldType_Bool, (void*)SetUseWaterReflection_s, (void*)GetUseWaterReflection_s, NULL, NULL, bOverride);
		pClass->AddField("CanUseAdvancedShading", FieldType_Bool, NULL, (void*)CanUseAdvancedShading_s, NULL, NULL, bOverride);
		pClass->AddField("MovieOutputMode", FieldType_Bool, (void*)EnableMovieOutputMode_s, (void*)IsMovieOutputMode_s, NULL, NULL, bOverride);
		pClass->AddField("AsyncChunkMode", FieldType_Bool, (void*)SetAsyncChunkMode_s, (void*)IsAsyncChunkMode_s, NULL, NULL, bOverride);
		pClass->AddField("VertexBufferSizeLimit", FieldType_Int, (void*)SetVertexBufferSizeLimit_s, (void*)GetVertexBufferSizeLimit_s, NULL, NULL, bOverride);
		pClass->AddField("AlwaysInVertexBufferChunkRadius", FieldType_Int, (void*)SetAlwaysInVertexBufferChunkRadius_s, (void*)GetAlwaysInVertexBufferChunkRadius_s, NULL, NULL, bOverride);
		pClass->AddField("MaxVisibleVertexBufferBytes", FieldType_Int, (void*)SetMaxVisibleVertexBufferBytes_s, (void*)GetMaxVisibleVertexBufferBytes_s, NULL, NULL, bOverride);

		pClass->AddField("NearCameraChunkDist", FieldType_Int, (void*)SetNearCameraChunkDist_s, (void*)GetNearCameraChunkDist_s, NULL, NULL, bOverride);
		pClass->AddField("MaxBufferRebuildPerTick", FieldType_Int, (void*)SetMaxBufferRebuildPerTick_s, (void*)GetMaxBufferRebuildPerTick_s, NULL, NULL, bOverride);
		pClass->AddField("MaxBufferRebuildPerTick_FarChunk", FieldType_Int, (void*)SetMaxBufferRebuildPerTick_FarChunk_s, (void*)GetMaxBufferRebuildPerTick_FarChunk_s, NULL, NULL, bOverride);
		
		return S_OK;
	}


}
