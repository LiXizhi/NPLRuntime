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
#include "ShadowMap.h"
#include "SunLight.h"
#include "BaseCamera.h"
#include "ParaWorldAsset.h"
#include "terrain/GlobalTerrain.h"
#include "WorldInfo.h"
#include "BlockCommon.h"
#include "BlockDirection.h"
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

#include "ParaScriptingCommon.h"

#include "ParaScriptingMisc.h"

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
	int32_t BlockWorldClient::g_materialPass = 4; 
	int32_t BlockWorldClient::g_materialFancyPass = 6;
#ifdef PARAENGINE_MOBILE
	int32_t BlockWorldClient::g_transparentBlockPass = 3;
	int32_t BlockWorldClient::g_waterBlockPass = 2;
	int32_t BlockWorldClient::g_bumpyBlockPass = 1;
#else
	int32_t BlockWorldClient::g_transparentBlockPass = 3;
	int32_t BlockWorldClient::g_waterBlockPass = 4;
	int32_t BlockWorldClient::g_bumpyBlockPass = 5;
#endif

	int32_t BlockWorldClient::g_selectBlockPass = 1;

	inline int64_t GetBlockSparseIndex(uint16_t bx, uint16_t by, uint16_t bz)
	{
		return (((uint64)bx) << 32) + (((uint64)by) << 16) + bz;
	}

	void FromBlockSparseIndex(int64_t index, uint16_t& bx, uint16_t& by, uint16_t& bz)
	{
		bx = (uint16_t)(index >> 32);
		by = (uint16_t)(((DWORD)index) >> 16);
		bz = (uint16_t)(((DWORD)index) & 0xffff);
	}

	//////////////////////////////////////////////////////////////////////////
	//BlockWorldClient
	//////////////////////////////////////////////////////////////////////////
	BlockWorldClient::BlockWorldClient()
		: m_maxSelectBlockPerBatch(500), m_isUnderLiquid(false), m_vBlockLightColor(DEFAULT_BLOCK_LIGHT_COLOR),
		m_nBufferRebuildCountThisTick(0), m_bUsePointTextureFiltering(true),
		m_nVertexBufferSizeLimit(100 * 1024 * 1024),
		m_nMaxVisibleVertexBufferBytes(100 * 1024 * 1024),
		m_nAlwaysInVertexBufferChunkRadius(2),
		m_pMultiFrameRenderer(NULL),
		m_bAutoBoundColorRT0(false),
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
		Cleanup();
		SAFE_DELETE(m_pMultiFrameRenderer);
		for (int i = 0; i < (int)(m_activeChunks.size()); ++i)
		{
			SAFE_DELETE(m_activeChunks[i]);
		}
		m_activeChunks.clear();
		ChunkVertexBuilderManager::GetInstance().Cleanup();

		RenderableChunk::StaticRelease();
	}

	void BlockWorldClient::EnterWorld(const string& sWorldDir, float x, float y, float z)
	{
		if (m_isInWorld)
			return;
		if (CGlobals::GetSettings().IsLowEndRenderDevice())
		{
			m_nVertexBufferSizeLimit = (std::min)(m_nVertexBufferSizeLimit, 64 * 1024 * 1024);
			m_nMaxVisibleVertexBufferBytes = (std::min)(m_nMaxVisibleVertexBufferBytes, 32 * 1024 * 1024);
			m_nMaxBufferRebuildPerTick = (std::min)(m_nMaxBufferRebuildPerTick, 2);
			m_nMaxBufferRebuildPerTick_FarChunk = 1;
			m_bUseSunlightShadowMap = false;
			m_bUseWaterReflection = false;
		}
		ChunkVertexBuilderManager::GetInstance().StartChunkBuildThread(this);
		Scoped_WriteLock<BlockReadWriteLock> lock_(GetReadWriteLock());
		CBlockWorld::EnterWorld(sWorldDir, x, y, z);
	}

	void BlockWorldClient::Cleanup()
	{
		DeleteDeviceObjects();

		if (auto* pDevice = CGlobals::GetRenderDevice())
		{
			pDevice->SetRenderTarget(3, nullptr);
			pDevice->SetRenderTarget(2, nullptr);
			pDevice->SetRenderTarget(1, nullptr);
			if (m_bAutoBoundColorRT0)
			{
				pDevice->SetRenderTarget(0, nullptr);
				m_bAutoBoundColorRT0 = false;
			}
		}

		if (CGlobals::GetSceneState())
		{
			CGlobals::GetSceneState()->SetDeferredShading(false);
		}

		if (m_dwBlockRenderMethod == BLOCK_RENDER_FANCY_SHADER)
		{
			m_dwBlockRenderMethod = BLOCK_RENDER_FAST_SHADER;
			if (CGlobals::GetApp() && CGlobals::GetApp()->GetAppState() != PEAppState_Exiting)
			{
				CGlobals::GetSettings().LoadGameEffectSet(CGlobals::GetSettings().GetGameEffectSet());
			}
			else if (auto* pEffectMgr = CGlobals::GetEffectManager())
			{
				pEffectMgr->EndEffect();
				pEffectMgr->MapHandleToEffect(TECH_TERRAIN, nullptr);
				pEffectMgr->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL, nullptr);
				pEffectMgr->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_INSTANCED, nullptr);
				pEffectMgr->MapHandleToEffect(TECH_CHARACTER, nullptr);
				pEffectMgr->MapHandleToEffect(TECH_CHARACTER_SKINNED, nullptr);
				pEffectMgr->MapHandleToEffect(TECH_BMAX_MODEL, nullptr);
				pEffectMgr->MapHandleToEffect(TECH_BMAX_SKINNED, nullptr);
			}
		}

		for (int i = 0; i < BLOCK_GROUP_ID_MAX; ++i)
		{
			m_highLightTextures[i].reset();
		}

		m_render_target_color_hdr.reset();
		m_render_target_color.reset();
		m_render_target_scene_color.reset();
		m_render_target_block_info.reset();
		m_render_target_depth_tex.reset();
		m_render_target_normal.reset();
		m_damageTexture.reset();

		m_block_effect_fancy.reset();
		m_normal_mesh_effect_fancy.reset();
		m_bmax_model_effect_fancy.reset();
		m_character_skinned_fancy.reset();
		m_bmax_skinned_fancy.reset();
		m_mesh_instanced_fancy.reset();
		m_terrain_fancy.reset();
		m_effect_light_point.reset();
		m_effect_light_spot.reset();
		m_effect_light_directional.reset();
		m_effect_deferred_lighting.reset();
		for (int i = 0; i < 3; ++i)
		{
			m_lightgeometry_effects[i].reset();
		}
		m_softwareOccluder.Reset();
		m_bHasLastMainCameraState = false;
	}

	void BlockWorldClient::LeaveWorld()
	{
		if (!m_isInWorld)
			return;
		ChunkVertexBuilderManager::GetInstance().Cleanup();
		Scoped_WriteLock<BlockReadWriteLock> lock_(GetReadWriteLock());
		Cleanup();
		CBlockWorld::LeaveWorld();
	}

	void BlockWorldClient::DeleteAllBlocks()
	{
		Scoped_WriteLock<BlockReadWriteLock> lock_(GetReadWriteLock());
		for (std::map<int, BlockRegion*>::iterator iter = m_regionCache.begin(); iter != m_regionCache.end(); iter++)
		{
			iter->second->DeleteAllBlocks();
		}

		for (uint32_t i = 0; i < m_activeChunks.size(); i++)
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

		bool bCameraChanged = !m_bHasLastMainCameraState;
		if (!bIsShadowPass && m_bHasLastMainCameraState)
		{
			if ((camWorldPos - m_lastMainEyePos).squaredLength() > 1e-6f)
			{
				bCameraChanged = true;
			}
			else if (auto* pEffMgr = CGlobals::GetEffectManager())
			{
				const Matrix4 curVP = pEffMgr->GetViewTransform() * pEffMgr->GetProjTransform();
				if (std::memcmp(&curVP, &m_lastMainViewProj, sizeof(Matrix4)) != 0)
					bCameraChanged = true;
			}
		}

		//perform culling and refresh render buffers if dirty.
		m_nBufferRebuildCountThisTick = 0;
		if (bIsShadowPass || m_isVisibleChunkDirty || bCameraChanged || (GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER && GetUseSunlightShadowMap()))
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
			if (m_nBufferRebuildCountThisTick > 0)
			{
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
		if (!IsGroupByChunkBeforeTexture())
		{
			if (m_solidRenderTasks.size() > 0)
				sort(m_solidRenderTasks.begin(), m_solidRenderTasks.end(), CompareRenderOrder);
		}

		if (m_alphaTestRenderTasks.size() > 0)
			sort(m_alphaTestRenderTasks.begin(), m_alphaTestRenderTasks.end(), CompareRenderOrder);

		if (m_alphaBlendRenderTasks.size() > 0)
			sort(m_alphaBlendRenderTasks.begin(), m_alphaBlendRenderTasks.end(), CompareRenderOrder);

		if (m_reflectedWaterRenderTasks.size() > 0)
			sort(m_reflectedWaterRenderTasks.begin(), m_reflectedWaterRenderTasks.end(), CompareRenderOrder);
	}

	void BlockWorldClient::RenderShadowMap()
	{
		// only render shadow map for fancy shader mode.
		if ((GetBlockRenderMethod() != BLOCK_RENDER_FANCY_SHADER) ||
			!GetUseSunlightShadowMap())
			return;
		// prepare shadow pass
		PreRender(true);
		// make sure all shaders are replaced.
		PrepareAllRenderTargets(false);
		CGlobals::GetEffectManager()->SetAllEffectsTechnique(EffectManager::EFFECT_GEN_SHADOWMAP);

		if (m_alphaTestRenderTasks.size() == 0 && m_solidRenderTasks.size() == 0 && m_alphaBlendRenderTasks.size() == 0)
			return;

		if (m_block_effect_fancy == 0)
			return;
		m_block_effect_fancy->LoadAsset();

		const float fBlockSize = BlockConfig::g_blockSize;
		Vector3 renderOfs = CGlobals::GetScene()->GetRenderOrigin();
		float verticalOffset = GetVerticalOffset();

		Uint16x3 renderBlockOfs;
		renderBlockOfs.x = (uint16_t)(renderOfs.x / fBlockSize);
		renderBlockOfs.z = (uint16_t)(renderOfs.z / fBlockSize);

		Vector3 renderBlockOfs_remain;
		renderBlockOfs_remain.x = renderOfs.x - renderBlockOfs.x * fBlockSize;
		renderBlockOfs_remain.z = renderOfs.z - renderBlockOfs.z * fBlockSize;

		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		auto pRenderDevice = CGlobals::GetRenderDevice();
		if (!pRenderDevice)
			return;

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
		if (pEffect != 0 && pEffect->begin(false))
		{
			VertexDeclarationPtr pVertexLayout = GetVertexLayout();
			pRenderDevice->SetVertexDeclaration(pVertexLayout);

			// turn off alpha blending to enable early-Z on modern graphic cards.
			pRenderDevice->SetRenderState(ERenderState::SRCBLEND, RSV_BLEND_ONE);
			pRenderDevice->SetRenderState(ERenderState::DESTBLEND, RSV_BLEND_ZERO);
			pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);

			//////////////////////////////////////////////////////////////////////////
			// set the wave time parameter
			double time = CGlobals::GetGameTime();
			time = (int)(time * 1000) % 1000000;
			pEffect->setParameter(CEffectFile::k_ConstVector1, Vector4((float)time, 0.f, 0.f, 0.f).ptr());

			IndexBufferDevicePtr_type pIndexBuffer = GetIndexBuffer();
			pRenderDevice->SetIndices(pIndexBuffer);

			for (int nRenderPass = 0; nRenderPass <= BlockRenderPass_AlphaBlended; nRenderPass++)
			{
				std::vector<BlockRenderTask*>* pCurRenderQueue = GetRenderQueueByPass((BlockRenderPass)nRenderPass);

				VertexBufferDevicePtr_type pCurVB = nullptr;
				uint16_t curTemplateId = 0;
				int32_t curPass = -1;
				ERSVCULL culling = RSV_CULL_CCW;
				DeviceTexturePtr_type pCurTex0 = nullptr;
				Uint16x3 lastMinPos(0xffff, 0xffff, 0xffff);
				bool bHasLastMinPos = false;

				if (pCurRenderQueue->size() > 0)
				{
					for (uint32_t i = 0; i < pCurRenderQueue->size(); i++)
					{
						BlockRenderTask* pRenderTask = (*pCurRenderQueue)[i];
						BlockTemplate* pTempate = pRenderTask->GetTemplate();
						if (!pTempate || !pTempate->IsShadowCaster())
							continue;

						VertexBufferDevicePtr_type pVB = pRenderTask->GetVertexBuffer();

						if (pVB != pCurVB)
						{
							pRenderDevice->SetStreamSource(0, pVB, 0, sizeof(BlockVertexCompressed));
							pCurVB = pVB;
						}
						int32_t passId = 0;
						if (curTemplateId != pRenderTask->GetTemplateId())
						{
							curTemplateId = pRenderTask->GetTemplateId();

							if (curPass != passId)
							{
								if (curPass > -1)
									pEffect->EndPass();

								if (pEffect->BeginPass(passId))
								{
									curPass = passId;
									bHasLastMinPos = false;
								}
								else
								{
									curPass = -1;
									continue;
								}
							}

							TextureEntity* pTexEntity = pTempate->GetTexture0(pRenderTask->GetUserData());
							if (pTexEntity && pTexEntity->GetTexture() != pCurTex0)
							{
								pCurTex0 = pTexEntity->GetTexture();
								pRenderDevice->SetTexture(0, pCurTex0);
							}

							// culling mode
							if (pTempate->GetBlockModel().IsDisableFaceCulling())
							{
								if (culling != RSV_CULL_NONE)
								{
									pRenderDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
									culling = RSV_CULL_NONE;
								}
							}
							else if (nRenderPass != BlockRenderPass_Opaque && m_isUnderLiquid && pTempate->IsMatchAttribute(BlockTemplate::batt_liquid))
							{
								if (culling != RSV_CULL_CW)
								{
									pRenderDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CW);
									culling = RSV_CULL_CW;
								}
							}
							else
							{
								if (culling != RSV_CULL_CCW)
								{
									pRenderDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
									culling = RSV_CULL_CCW;
								}
							}
						}

						Uint16x3& vMinPos = pRenderTask->GetMinBlockPos();
						if (!bHasLastMinPos || vMinPos.x != lastMinPos.x || vMinPos.y != lastMinPos.y || vMinPos.z != lastMinPos.z)
						{
							lastMinPos = vMinPos;
							bHasLastMinPos = true;

							Matrix4 vWorldMatrix(Matrix4::IDENTITY);
							vWorldMatrix._11 = vWorldMatrix._22 = vWorldMatrix._33 = fBlockSize;
							vWorldMatrix._41 = (vMinPos.x - renderBlockOfs.x) * fBlockSize - renderBlockOfs_remain.x;
							vWorldMatrix._42 = vMinPos.y * fBlockSize - renderOfs.y + verticalOffset;
							vWorldMatrix._43 = (vMinPos.z - renderBlockOfs.z) * fBlockSize - renderBlockOfs_remain.z;

							vWorldMatrix = vWorldMatrix * matViewProj;

							pEffect->setMatrix(CEffectFile::k_worldViewProjMatrix, &vWorldMatrix);
						}

						pEffect->CommitChanges();

						pRenderDevice->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST, 0, pRenderTask->GetVertexOfs(),
							pRenderTask->GetVertexCount(), pRenderTask->GetIndexOfs(), pRenderTask->GetPrimitiveCount());
					}
				}
				if (curPass > -1)
					pEffect->EndPass();

				pRenderDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
			}
			// turn blending on again
			pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
			pRenderDevice->SetRenderState(ERenderState::SRCBLEND, RSV_BLEND_SRCALPHA);
			pRenderDevice->SetRenderState(ERenderState::DESTBLEND, RSV_BLEND_INVSRCALPHA);
			pEffect->end();
		}
	}

	void BlockWorldClient::Render(BlockRenderPass nRenderPass, std::vector<BlockRenderTask*>* pCurRenderQueue, int nRenderMethod)
	{
		if (!m_isInWorld)
			return;

		BlockRenderMethod dwRenderMethod = (nRenderMethod < 0) ? GetBlockRenderMethod() : (BlockRenderMethod)nRenderMethod;

		// no need to lock
		if (pCurRenderQueue == 0)
			pCurRenderQueue = GetRenderQueueByPass(nRenderPass);

		auto pDevice = CGlobals::GetRenderDevice();

		if (pCurRenderQueue->size() > 0)
		{
			const float fBlockSize = BlockConfig::g_blockSize;
			Vector3 renderOfs = CGlobals::GetScene()->GetRenderOrigin();
			float verticalOffset = GetVerticalOffset();

			Uint16x3 renderBlockOfs;
			renderBlockOfs.x = (uint16_t)(renderOfs.x / fBlockSize);
			renderBlockOfs.z = (uint16_t)(renderOfs.z / fBlockSize);

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

			if (nRenderPass == BlockRenderPass_AlphaBlended || nRenderPass == BlockRenderPass_ReflectedWater)
			{
				pDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
				pDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
				pDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
			else
			{
				if (nRenderPass == BlockRenderPass_AlphaTest)
					CGlobals::GetRenderDevice()->SetSamplerState(0, ESamplerStateType::MIPFILTER, D3DTEXF_NONE);

				// turn off alpha blending to enable early-Z on modern graphic cards.
				pDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
				pDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ZERO);
				pDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
			}

			if (pEffect == 0)
			{
				// All ParaRHI backends render through programmable shaders (TECH_BLOCK).
			}
			else if (pEffect != 0 && pEffect->begin(false))
			{
				VertexDeclarationPtr pVertexLayout = GetVertexLayout();
				CGlobals::GetRenderDevice()->SetVertexDeclaration(pVertexLayout);

				// set the wave time parameter
				double time = CGlobals::GetGameTime();
				// in case it loses accuracy, we will subtract integer number of 2*PI from time
				// time -= ((int)(time / (2*MATH_PI)))*(2*MATH_PI);
				time = (int)(time * 1000) % 1000000;
				Vector4 v4((float)time, 0.f, 0.f, 0.f);
				pEffect->setParameter(CEffectFile::k_ConstVector1, v4.ptr());

				if (dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
				{
				}
				else
				{
					Vector3 vDir = -CGlobals::GetScene()->GetSunLight().GetSunDirection();
					Vector4 vDir_(vDir.x, vDir.y, vDir.z, 1.0f);
					pEffect->setParameter(CEffectFile::k_sunVector, &vDir_);
				}

				IndexBufferDevicePtr_type pIndexBuffer = GetIndexBuffer();
				CGlobals::GetRenderDevice()->SetIndices(pIndexBuffer);

				VertexBufferDevicePtr_type pCurVB = 0;
				uint16_t curTemplateId = 0;
				int32 curMaterialId = -1;
				int32 lastMaterialId = -1;
				int32_t curPass = -1;
				ERSVCULL culling = RSV_CULL_CCW;
				DeviceTexturePtr_type pCurTex0 = 0;
				DeviceTexturePtr_type pCurTex1 = 0;
				DeviceTexturePtr_type pCurTex2 = 0;

				EffectManager* pEffectManager = CGlobals::GetEffectManager();
				IScene* pScene = pEffectManager->GetScene();
				CBaseCamera* pCamera = pScene->GetCurrentCamera();
				Matrix4 matViewProj;
				if (pCamera)
				{
					const Matrix4& pView = (pEffectManager->GetViewTransform());
					const Matrix4& pProj = (pEffectManager->GetProjTransform());
					matViewProj = (pView) * (pProj);
				}

				/** block light params and sun intensity*/
				Vector4 vLightParams(m_vBlockLightColor.r, m_vBlockLightColor.g, m_vBlockLightColor.b, m_sunIntensity);
				pEffect->setParameter(CEffectFile::k_ConstVector0, (const void*)(&vLightParams));
				Uint16x3 lastMinPos(0xffff, 0xffff, 0xffff);
				bool bHasLastMinPos = false;
				const bool bUsingShadowMap = pEffectManager->IsUsingShadowMap();
				const Matrix4* pTexViewProj = bUsingShadowMap ? pEffectManager->GetTexViewProjMatrix() : nullptr;

				for (uint32_t i = 0; i < pCurRenderQueue->size(); i++)
				{
					BlockRenderTask* pRenderTask = (*pCurRenderQueue)[i];
					VertexBufferDevicePtr_type pVB = pRenderTask->GetVertexBuffer();
					if (pVB != pCurVB)
					{
						pDevice->SetStreamSource(0, pVB, 0, sizeof(BlockVertexCompressed));
						pCurVB = pVB;
					}
					int32_t passId;
					BlockTemplate* pTemplate = pRenderTask->GetTemplate();
					if (curTemplateId != pRenderTask->GetTemplateId() || curMaterialId != pRenderTask->GetMaterialId())
					{
						curTemplateId = pRenderTask->GetTemplateId();
						curMaterialId = pRenderTask->GetMaterialId();
						if (curMaterialId > 0) {
							if (dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
							{
								passId = g_materialFancyPass;
							}
							else
							{
								passId = g_materialPass;
							}
						}
						else if (pTemplate->IsMatchAttribute(BlockTemplate::batt_twoTexture))
							passId = g_twoTexPass;
						else if (nRenderPass == BlockRenderPass_AlphaTest)
						{
							passId = g_transparentBlockPass;
						}
						else if (nRenderPass == BlockRenderPass_AlphaBlended || nRenderPass == BlockRenderPass_ReflectedWater)
						{
							passId = g_TexPass;
							if (pTemplate->GetCategoryID() == 8 && dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
							{
								passId = g_waterBlockPass;
							}
						}
						else if (pTemplate->GetNormalMap() && dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
						{
							passId = g_bumpyBlockPass;
						}
						else
							passId = g_TexPass;

						if (curPass != passId)
						{
							if (curPass > -1)
								pEffect->EndPass();

							if (pEffect->BeginPass(passId))
							{
								curPass = passId;
								bHasLastMinPos = false;

								pCurTex0 = 0;
								pCurTex1 = 0;
								pCurTex2 = 0;
								lastMaterialId = -1;
								// pEffect->setMatrix(CEffectFile::k_viewProjMatrix, &matViewProj);
								// pDevice->SetVertexShaderConstantF(0,&matViewProj._11,4);
							}
							else
							{
								curPass = -1;
								continue;
							}
						}

						// use linear filtering and wrap addressing for block material diffuse texture
						if (lastMaterialId != curMaterialId)
						{
							if (lastMaterialId <= 0 && curMaterialId > 0)
							{
								pEffectManager->SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
								pEffectManager->SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
								pEffectManager->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP, true);
								pEffectManager->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP, true);
							}
							else if (lastMaterialId > 0 && curMaterialId <= 0)
							{
								pEffectManager->SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
								pEffectManager->SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
							}
							lastMaterialId = curMaterialId;
						}
						
						if (curMaterialId > 0)
						{
							TextureEntity* defaultTex = CGlobals::GetAssetManager()->GetDefaultTexture(0);
							CBlockMaterial* material = CGlobals::GetBlockMaterialManager()->GetBlockMaterialByID(curMaterialId);
							CParameterBlock* paramBlock = material ? material->GetParamBlock() : nullptr;
							if (paramBlock)
							{
								CParameter* materialUV = paramBlock->GetParameter("MaterialUV");
								if (materialUV) pEffect->setParameter(CEffectFile::k_material_uv, materialUV->GetRawData(), materialUV->GetRawDataLength());
								CParameter* baseColor = paramBlock->GetParameter("BaseColor");
								if (baseColor) pEffect->setParameter(CEffectFile::k_material_base_color, baseColor->GetRawData(), baseColor->GetRawDataLength());
								CParameter* metallic = paramBlock->GetParameter("Metallic");
								if (metallic) pEffect->setParameter(CEffectFile::k_material_metallic, metallic->GetRawData(), metallic->GetRawDataLength());
								CParameter* specular = paramBlock->GetParameter("Specular");
								if (specular) pEffect->setParameter(CEffectFile::k_material_specular, specular->GetRawData(), specular->GetRawDataLength());
								CParameter* roughness = paramBlock->GetParameter("Roughness");
								if (roughness) pEffect->setParameter(CEffectFile::k_material_roughness, roughness->GetRawData(), roughness->GetRawDataLength());
								CParameter* emissiveColor = paramBlock->GetParameter("EmissiveColor");
								if (emissiveColor) pEffect->setParameter(CEffectFile::k_material_emissive_color, emissiveColor->GetRawData(), emissiveColor->GetRawDataLength());
								CParameter* opacity = paramBlock->GetParameter("Opacity");
								if (opacity) pEffect->setParameter(CEffectFile::k_material_opacity, opacity->GetRawData(), opacity->GetRawDataLength());

								auto getMaterialTexPath = [&](const char* fullKey, const char* relKey) -> const std::string& {
									static const std::string s_empty;
									CParameter* pFull = paramBlock->GetParameter(fullKey);
									if (pFull && !pFull->GetValueAsConstString().empty())
										return pFull->GetValueAsConstString();
									CParameter* pRel = paramBlock->GetParameter(relKey);
									if (pRel && !pRel->GetValueAsConstString().empty())
										return pRel->GetValueAsConstString();
									return s_empty;
								};

								bool bHasDiffuseTex = false;
								const std::string& sDiffuseFile = getMaterialTexPath("DiffuseFullPath", "Diffuse");
								if (!sDiffuseFile.empty())
								{
									auto tex = CGlobals::GetAssetManager()->GetTexture(sDiffuseFile);
									if(tex == NULL)
										tex = CGlobals::GetAssetManager()->LoadTexture(sDiffuseFile, sDiffuseFile);
									if (tex)
									{
										auto curTex = tex->GetTexture();
										if (curTex != nullptr)
										{
											bHasDiffuseTex = true;
											if (pCurTex0 != curTex)
											{
												pCurTex0 = curTex;
												pEffect->setTexture(0, tex);
											}
										}
									}
								}
								if(!bHasDiffuseTex && defaultTex)
								{
									// always use a white texture instead of setting a conditional boolean in shader, since 99% cases, there is a diffuse texture. 
									auto curTex = defaultTex->GetTexture();
									if (curTex != nullptr && pCurTex0 != curTex)
									{
										pCurTex0 = curTex;
										pEffect->setTexture(0, defaultTex);
									}
								}

								bool bHasNormalTex = false;
								const std::string& sNormalFile = getMaterialTexPath("NormalFullPath", "Normal");
								if (!sNormalFile.empty())
								{
									auto tex = CGlobals::GetAssetManager()->GetTexture(sNormalFile);
									if (tex == NULL)
										tex = CGlobals::GetAssetManager()->LoadTexture(sNormalFile, sNormalFile);
									if (tex)
									{
										auto curTex = tex->GetTexture();
										if (curTex != nullptr)
										{
											bHasNormalTex = true;
											if (dwRenderMethod == BLOCK_RENDER_FANCY_SHADER && pCurTex2 != curTex)
											{
												pCurTex2 = curTex;
												pEffect->setTexture(2, tex);
											}
										}
									}
								}
								if (!bHasNormalTex)
								{
									float zeroMetallic = 0.0f;
									pEffect->setParameter(CEffectFile::k_material_metallic, &zeroMetallic, sizeof(float));
									if (dwRenderMethod == BLOCK_RENDER_FANCY_SHADER && defaultTex)
									{
										auto curTex = defaultTex->GetTexture();
										if (curTex != nullptr && pCurTex2 != curTex)
										{
											pCurTex2 = curTex;
											pEffect->setTexture(2, defaultTex);
										}
									}
								}

								bool bHasEmissiveTex = false;
								const std::string& sEmissiveFile = getMaterialTexPath("EmissiveFullPath", "Emissive");
								if (!sEmissiveFile.empty())
								{
									auto tex = CGlobals::GetAssetManager()->GetTexture(sEmissiveFile);
									if (tex == NULL)
										tex = CGlobals::GetAssetManager()->LoadTexture(sEmissiveFile, sEmissiveFile);
									if (tex)
									{
										auto curTex = tex->GetTexture();
										if (curTex != nullptr)
										{
											bHasEmissiveTex = true;
											if (pCurTex1 != curTex)
											{
												pCurTex1 = curTex;
												pEffect->setTexture(1, tex);
											}
										}
									}
								}
								if (!bHasEmissiveTex)
								{
									LinearColor zeroEmissive(0.f, 0.f, 0.f, 0.f);
									pEffect->setParameter(CEffectFile::k_material_emissive_color, &zeroEmissive, sizeof(LinearColor));
									if (defaultTex)
									{
										auto curTex = defaultTex->GetTexture();
										if (curTex != nullptr && pCurTex1 != curTex)
										{
											pCurTex1 = curTex;
											pEffect->setTexture(1, defaultTex);
										}
									}
								}
							}
							else if (defaultTex)
							{
								auto curTex = defaultTex->GetTexture();
								if (curTex != nullptr && pCurTex0 != curTex)
								{
									pCurTex0 = curTex;
									pEffect->setTexture(0, defaultTex);
								}
								float zeroMetallic = 0.0f;
								pEffect->setParameter(CEffectFile::k_material_metallic, &zeroMetallic, sizeof(float));
								LinearColor zeroEmissive(0.f, 0.f, 0.f, 0.f);
								pEffect->setParameter(CEffectFile::k_material_emissive_color, &zeroEmissive, sizeof(LinearColor));
							}
						}
	
						// culling mode
						if (pTemplate->GetBlockModel().IsDisableFaceCulling())
						{
							if (culling != RSV_CULL_NONE)
							{
								pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
								culling = RSV_CULL_NONE;
							}
						}
						else if (nRenderPass != BlockRenderPass_Opaque && m_isUnderLiquid && pRenderTask->GetTemplate()->IsMatchAttribute(BlockTemplate::batt_liquid))
						{
							if (culling != RSV_CULL_CW)
							{
								pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CW);
								culling = RSV_CULL_CW;
							}
						}
						else
						{
							if (culling != RSV_CULL_CCW)
							{
								pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
								culling = RSV_CULL_CCW;
							}
						}
					}

					if (curMaterialId <= 0)
					{
						// use block's internal material
						TextureEntity* pTexEntity = pTemplate->GetTexture0(pRenderTask->GetUserData());
						if (pTexEntity)
						{
							auto curTex = pTexEntity->GetTexture();
							if (curTex != nullptr && curTex != pCurTex0)
							{
								pCurTex0 = curTex;
								pEffect->setTexture(0, pTexEntity);
							}
						}

						pTexEntity = pTemplate->GetTexture1();
						if (pTexEntity)
						{
							auto curTex = pTexEntity->GetTexture();
							if (curTex != nullptr && curTex != pCurTex1)
							{
								pCurTex1 = curTex;
								pEffect->setTexture(1, pTexEntity);
							}
						}

						pTexEntity = pTemplate->GetNormalMap();
						if (pTexEntity)
						{
							auto curTex = pTexEntity->GetTexture();
							if (curTex != nullptr && curTex != pCurTex2)
							{
								pCurTex2 = curTex;
								if (dwRenderMethod == BLOCK_RENDER_FANCY_SHADER) {
									pEffect->setTexture(2, pTexEntity);
								}
							}
						}
					}

					Uint16x3& vMinPos = pRenderTask->GetMinBlockPos();
					if (!bHasLastMinPos || vMinPos.x != lastMinPos.x || vMinPos.y != lastMinPos.y || vMinPos.z != lastMinPos.z)
					{
						lastMinPos = vMinPos;
						bHasLastMinPos = true;

						Matrix4 vWorldMatrix(Matrix4::IDENTITY);
						vWorldMatrix._11 = vWorldMatrix._22 = vWorldMatrix._33 = fBlockSize;
						vWorldMatrix._41 = (vMinPos.x - renderBlockOfs.x) * fBlockSize - renderBlockOfs_remain.x;
						vWorldMatrix._42 = vMinPos.y * fBlockSize - renderOfs.y + verticalOffset;
						vWorldMatrix._43 = (vMinPos.z - renderBlockOfs.z) * fBlockSize - renderBlockOfs_remain.z;

						Matrix4 mWorldViewProj = vWorldMatrix * matViewProj;
						pEffect->setMatrix(CEffectFile::k_worldViewProjMatrix, &mWorldViewProj);
						if (pEffect->isMatrixUsed(CEffectFile::k_worldViewMatrix))
						{
							Matrix4 mWorldView;
							ParaMatrixMultiply(&mWorldView, &vWorldMatrix, &matViewProj);
							pEffect->setMatrix(CEffectFile::k_worldViewMatrix, &mWorldView);
						}
						if (pTexViewProj && pEffect->isMatrixUsed(CEffectFile::k_TexWorldViewProjMatrix))
						{
							Matrix4 mTex;
							ParaMatrixMultiply(&mTex, &vWorldMatrix, pTexViewProj);
							pEffect->setMatrix(CEffectFile::k_TexWorldViewProjMatrix, &mTex);
						}
						Vector4 vWorldPos(
							renderOfs.x + vWorldMatrix._41,
							renderOfs.y + vWorldMatrix._42,
							renderOfs.z + vWorldMatrix._43,
							1.f);
						pEffect->setParameter(CEffectFile::k_worldPos, &vWorldPos);
					}

					pEffect->CommitChanges();

					pDevice->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST, 0, pRenderTask->GetVertexOfs(),
						pRenderTask->GetVertexCount(), pRenderTask->GetIndexOfs(), pRenderTask->GetPrimitiveCount());
				}

				if (curPass > -1)
					pEffect->EndPass();

				if (lastMaterialId > 0)
				{
					pEffectManager->SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
					pEffectManager->SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
				}

				pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);

				if (dwRenderMethod == BLOCK_RENDER_FANCY_SHADER)
				{
					// disable render target again
				}

				pEffect->end();
				if (pCurVB != 0)
				{
					// unbind buffer if any, this fixed a crash bug with OpenGL multi-frame renderer
					// where the same stream source is used both in multi-frame renderer and main renderer.
					CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
				}
			}

			if (nRenderPass != BlockRenderPass_AlphaBlended)
			{
				if (nRenderPass == BlockRenderPass_AlphaTest)
					CGlobals::GetRenderDevice()->SetSamplerState(0, ESamplerStateType::MIPFILTER, D3DTEXF_LINEAR);
				// turn blending on again
				pDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
				pDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
				pDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
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

		if (selectedBlockMap.size() == 0)
			return;

		SceneState* sceneState = CGlobals::GetSceneState();
		IBatchedElementDraw* pBatchedElementDraw = sceneState->GetBatchedElementDrawer();
		if (pBatchedElementDraw == 0)
			return;

		Vector3 pVecBounds[8];
		int nNumVertices;

		const float fBlockSize = BlockConfig::g_blockSize;
		Vector3 renderOfs = CGlobals::GetScene()->GetRenderOrigin();
		float verticalOffset = GetVerticalOffset();

		int32_t renderBlockOfs_x = (int32_t)(renderOfs.x / fBlockSize);
		int32_t renderBlockOfs_y = (int32_t)(renderOfs.y / fBlockSize);
		int32_t renderBlockOfs_z = (int32_t)(renderOfs.z / fBlockSize);

		Vector3 renderBlockOfs_remain;
		renderBlockOfs_remain.x = renderOfs.x - renderBlockOfs_x * fBlockSize;
		renderBlockOfs_remain.y = renderOfs.y - renderBlockOfs_y * fBlockSize;
		renderBlockOfs_remain.z = renderOfs.z - renderBlockOfs_z * fBlockSize;

		// scale the block a little to avoid z fighting.
		float fScaledBlockSize = BlockConfig::g_blockSize * fScaling;

		if (pBatchedElementDraw)
		{
			PARAVECTOR4 color(0.2f, 0.2f, 0.2f, 0.7f);

			if (pLineColor)
			{
				color = *((PARAVECTOR4*)pLineColor);
			}
			const float fLineWidth = -2.f;

			auto itEnd = selectedBlockMap.end();
			for (auto itCur = selectedBlockMap.begin(); itCur != itEnd; itCur++)
			{
				uint16_t x, y, z;
				FromBlockSparseIndex((*itCur).first, x, y, z);

				BlockTemplate* pBlockTemplate = GetBlockTemplate(x, y, z);
				if (pBlockTemplate == 0)
					pBlockTemplate = GetBlockTemplate(1);
				if (pBlockTemplate)
				{
					int nLenCount = 12;
					CShapeAABB aabb;
					pBlockTemplate->GetAABB(this, x, y, z, &aabb);

					Vector3 vOffset((x - renderBlockOfs_x) * fBlockSize, (y - renderBlockOfs_y) * fBlockSize + verticalOffset, (z - renderBlockOfs_z) * fBlockSize);
					vOffset -= renderBlockOfs_remain;

					aabb.GetCenter() += vOffset;
					aabb.GetExtents() *= fScaling;

					BlockModel::GetBoundingBoxVertices(aabb, pVecBounds, &nNumVertices);
					{
						static const int pIndexBuffer[] = {
							0, 1, 2, 3, 0,	// bottom
							4, 5, 6, 7, 4,	// top
							5, 1, 6, 2, 3, 7, // sides
						};

						Vector3 pVertexList[16];
						for (int i = 0; i < 16; i++)
						{
							pVertexList[i] = pVecBounds[pIndexBuffer[i]];
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

	void BlockWorldClient::RenderSelectionBlock(int nSelectionIndex, float fScaledBlockSize, LinearColor* pColor, BOOL bColorAnimate)
	{
		auto& selectedBlocks = m_selectedBlockMap[nSelectionIndex];
		auto& selectedBlockMap = selectedBlocks.m_blocks;
		auto& highLightTexture = m_highLightTextures[nSelectionIndex];
		if (selectedBlockMap.size() == 0)
			return;

		if ((int)m_selectBlockInstData.size() < m_maxSelectBlockPerBatch * 4)
			m_selectBlockInstData.resize(m_maxSelectBlockPerBatch * 4);

		RenderDevicePtr pDevice = CGlobals::GetRenderDevice();
		float blockSize = BlockConfig::g_blockSize;

		if (!highLightTexture)
		{
			return;
		}
		else if (!highLightTexture->IsLoaded())
		{
			highLightTexture->LoadAsset();
			return;
		}

		BuildSelectionBlockBuffer();

		const float fBlockSize = BlockConfig::g_blockSize;
		Vector3 renderOfs = CGlobals::GetScene()->GetRenderOrigin();
		float verticalOffset = GetVerticalOffset();

		int32_t renderBlockOfs_x = (int32_t)(renderOfs.x / fBlockSize);
		int32_t renderBlockOfs_y = (int32_t)(renderOfs.y / fBlockSize);
		int32_t renderBlockOfs_z = (int32_t)(renderOfs.z / fBlockSize);

		Vector3 renderBlockOfs_remain;
		renderBlockOfs_remain.x = renderOfs.x - renderBlockOfs_x * fBlockSize;
		renderBlockOfs_remain.y = renderOfs.y - renderBlockOfs_y * fBlockSize;
		renderBlockOfs_remain.z = renderOfs.z - renderBlockOfs_z * fBlockSize;

		// scale the block a little to avoid z fighting.
		fScaledBlockSize = BlockConfig::g_blockSize * fScaledBlockSize;

		int32_t count = (int32)selectedBlockMap.size();
		int32_t curInstCount = 0;
		int32_t instFloatCount = 0;

		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->BeginEffect(TECH_BLOCK);
		CEffectFile* pEffect = pEffectManager->GetCurrentEffectFile();

		CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
		CGlobals::GetRenderDevice()->SetIndices(0);

		// culling mode
		if (selectedBlocks.m_bOnlyRenderClickableArea)
		{
			pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
		}
		else
		{
			pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
		}

		if (pEffect == 0)
		{
			// All ParaRHI backends render through programmable shaders (TECH_BLOCK).
		}
		else if (pEffect != 0 && pEffect->begin(false))
		{
			CGlobals::GetRenderDevice()->SetVertexDeclaration(GetSelectBlockVertexLayout());

			if (pEffect->BeginPass(g_selectBlockPass))
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

				CGlobals::GetRenderDevice()->SetTexture(0, highLightTexture->GetTexture());

				pDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
				pDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
				pDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
				pDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);

				uint32_t time = CGlobals::GetSceneState()->GetGlobalTime();
				float lightIntensity = 1.f;
				if (bColorAnimate)
					lightIntensity = abs(sin(time * 0.0015f)) * 0.4f;

				Vector4 vLightParams(lightIntensity, 0, 0, 0);
				pEffect->setParameter(CEffectFile::k_ConstVector0, (const void*)(&vLightParams));

				Matrix4 vWorldMatrix(Matrix4::IDENTITY);
				vWorldMatrix._41 = -renderBlockOfs_remain.x - (fScaledBlockSize - fBlockSize) * 0.5f;
				vWorldMatrix._42 = -renderBlockOfs_remain.y - (fScaledBlockSize - fBlockSize) * 0.5f;
				vWorldMatrix._43 = -renderBlockOfs_remain.z - (fScaledBlockSize - fBlockSize) * 0.5f;

				vWorldMatrix = vWorldMatrix * matViewProj;

				pEffect->setMatrix(CEffectFile::k_worldViewProjMatrix, &vWorldMatrix);

				pEffect->CommitChanges();

				int nMaxFaceCount = (m_maxSelectBlockPerBatch - 1) * 6;

				int eyeX = GetEyeBlockId().x;
				int eyeZ = GetEyeBlockId().z;
				int nRenderDist = GetRenderDist();

				auto itEnd = selectedBlockMap.end();
				for (auto itCur = selectedBlockMap.begin(); itCur != itEnd; itCur++)
				{
					uint16_t nNearbyValue = (*itCur).second;
					if (nNearbyValue == 0 && !selectedBlocks.m_bOnlyRenderClickableArea)
						continue;

					uint16_t x, y, z;
					FromBlockSparseIndex((*itCur).first, x, y, z);

					if (abs(eyeX - x) >= nRenderDist || abs(eyeZ - z) >= nRenderDist)
						continue;

					if (curInstCount >= nMaxFaceCount)
					{

						CGlobals::GetRenderDevice()->DrawIndexedPrimitiveUP(EPrimitiveType::TRIANGLELIST, 0, curInstCount * 4, curInstCount * 2, &(m_select_block_indices[0]), PixelFormat::INDEX16, &(m_select_block_vertices[0]), sizeof(SelectBlockVertex));
						curInstCount = 0;
						instFloatCount = 0;
					}

					int nCount = 0;
					Vector3 vOffset((x - renderBlockOfs_x)* fBlockSize, (y - renderBlockOfs_y)* fBlockSize + verticalOffset, (z - renderBlockOfs_z)* fBlockSize);

					if (selectedBlocks.m_bOnlyRenderClickableArea)
						nCount = FillSelectBlockInvert(nNearbyValue, x, y, z, &(m_select_block_vertices[instFloatCount * 4]), vOffset, fScaledBlockSize);
					else
						nCount = FillSelectBlockVertice(nNearbyValue, x, y, z, &(m_select_block_vertices[instFloatCount * 4]), vOffset, fScaledBlockSize);

					instFloatCount += nCount;
					curInstCount += nCount;
				}

				if (curInstCount > 0)
				{
					CGlobals::GetRenderDevice()->DrawIndexedPrimitiveUP(EPrimitiveType::TRIANGLELIST, 0, curInstCount * 4, curInstCount * 2, &(m_select_block_indices[0]), PixelFormat::INDEX16, &(m_select_block_vertices[0]), sizeof(SelectBlockVertex));
				}

				pEffect->EndPass();
				pDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
				pDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
				pDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
				pDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
				pDevice->SetRenderState(ERenderState::DEPTHBIAS, 0);
			}
			pEffect->end();
		}
		pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
	}

	void BlockWorldClient::RenderDamagedBlock()
	{
		if (m_damageDegree <= 0)
			return;
		RenderDevicePtr pDevice = CGlobals::GetRenderDevice();
		if (!pDevice)
			return;
		float blockSize = BlockConfig::g_blockSize;

		if (!m_damageTexture)
		{
			return;
		}
		else if (!m_damageTexture->IsLoaded())
		{
			m_damageTexture->LoadAsset();
			return;
		}

		//-----------------------------------------
		BuildSelectionBlockBuffer();

		const float fBlockSize = BlockConfig::g_blockSize;
		Vector3 renderOfs = CGlobals::GetScene()->GetRenderOrigin();
		float verticalOffset = GetVerticalOffset();

		int32_t renderBlockOfs_x = (int32_t)(renderOfs.x / fBlockSize);
		int32_t renderBlockOfs_y = (int32_t)(renderOfs.y / fBlockSize);
		int32_t renderBlockOfs_z = (int32_t)(renderOfs.z / fBlockSize);

		Vector3 renderBlockOfs_remain;
		renderBlockOfs_remain.x = renderOfs.x - renderBlockOfs_x * fBlockSize;
		renderBlockOfs_remain.y = renderOfs.y - renderBlockOfs_y * fBlockSize;
		renderBlockOfs_remain.z = renderOfs.z - renderBlockOfs_z * fBlockSize;

		// scale the block a little to avoid z fighting.
		float fScaledBlockSize = BlockConfig::g_blockSize * 1.02f;

		//-----------------------------------------
		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->BeginEffect(TECH_BLOCK);
		CEffectFile* pEffect = pEffectManager->GetCurrentEffectFile();

		if (pEffect != 0 && pEffect->begin(false))
		{
			pDevice->SetVertexDeclaration(GetSelectBlockVertexLayout());

			if (pEffect->BeginPass(2))
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

				pDevice->SetTexture(0, m_damageTexture->GetTexture());

				pDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
				pDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);

				Matrix4 vWorldMatrix(Matrix4::IDENTITY);
				vWorldMatrix._41 = -renderBlockOfs_remain.x - (fScaledBlockSize - fBlockSize) * 0.5f;
				vWorldMatrix._42 = -renderBlockOfs_remain.y - (fScaledBlockSize - fBlockSize) * 0.5f;
				vWorldMatrix._43 = -renderBlockOfs_remain.z - (fScaledBlockSize - fBlockSize) * 0.5f;

				vWorldMatrix = vWorldMatrix * matViewProj;

				pEffect->setMatrix(CEffectFile::k_worldViewProjMatrix, &vWorldMatrix);

				pEffect->CommitChanges();

				int curInstCount = 0;
				int instFloatCount = 0;
				{
					Vector3 vOffset((m_damagedBlockId.x - renderBlockOfs_x) * fBlockSize, (m_damagedBlockId.y - renderBlockOfs_y) * fBlockSize + verticalOffset, (m_damagedBlockId.z - renderBlockOfs_z) * fBlockSize);
					// animate the UV's y offset according to m_damageDegree
					FillSelectBlockVertice(0xff, 0, 0, 0, &(m_select_block_vertices[instFloatCount * 24]), vOffset, fScaledBlockSize, ((int)(max(m_damageDegree - 0.1f, 0.f) * 8)) / 8.f, 1 / 8.f);
					instFloatCount++;
					curInstCount++;
				}
				pDevice->DrawIndexedPrimitiveUP(EPrimitiveType::TRIANGLELIST, 0, curInstCount * 24, curInstCount * 12, &(m_select_block_indices[0]), PixelFormat::INDEX16, &(m_select_block_vertices[0]), sizeof(SelectBlockVertex));

				pEffect->EndPass();
				pDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
				pDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			}
			pEffect->end();
		}
	}

	int BlockWorldClient::FillSelectBlockInvert(uint16_t nNearbyValue, uint16_t x, uint16_t y, uint16_t z, SelectBlockVertex* blockModel, const Vector3& vOffset, const float blockSize, float fUV_Y_Offset /*= 0.f*/, float fUV_Y_Size/*=1.f*/)
	{
		int nCount = 0;

		if (GetBlockTemplateIdByIdx(x, y + 1, z) != 0)
		{
			//top face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y + blockSize;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z + blockSize;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x + blockSize;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z + blockSize;
			blockModel[2].texcoord = Vector2(1, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x + blockSize;
			blockModel[3].position.y = vOffset.y + blockSize;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1, fUV_Y_Offset + fUV_Y_Size);

			blockModel += 4;
			nCount++;
		}
		if (GetBlockTemplateIdByIdx(x, y, z - 1) != 0)
		{
			//front face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x + blockSize;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x + blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel += 4;
			nCount++;
		}
		if (GetBlockTemplateIdByIdx(x, y - 1, z) != 0)
		{
			//bottom face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z + blockSize;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x + blockSize;
			blockModel[2].position.y = vOffset.y;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x + blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z + blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel += 4;
			nCount++;
		}
		if (GetBlockTemplateIdByIdx(x - 1, y, z) != 0)
		{
			//left face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z + blockSize;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z + blockSize;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel += 4;
			nCount++;
		}
		if (GetBlockTemplateIdByIdx(x + 1, y, z) != 0)
		{
			//right
			blockModel[0].position.x = vOffset.x + blockSize;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x + blockSize;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x + blockSize;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z + blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x + blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z + blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel += 4;
			nCount++;
		}
		if (GetBlockTemplateIdByIdx(x, y, z + 1) != 0)
		{
			//back face
			blockModel[0].position.x = vOffset.x + blockSize;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z + blockSize;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x + blockSize;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z + blockSize;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z + blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z + blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel += 4;
			nCount++;
		}
		if (nCount == 0)
		{
			nCount = FillSelectBlockVertice(nNearbyValue, x, y, z, blockModel, vOffset, blockSize, fUV_Y_Offset, fUV_Y_Size);
		}
		return nCount;
	}

	int BlockWorldClient::FillSelectBlockVertice(uint16_t nNearbyValue, uint16_t x, uint16_t y, uint16_t z, SelectBlockVertex* blockModel, const Vector3& vOffset, const float blockSize, float fUV_Y_Offset, float fUV_Y_Size)
	{
		int nCount = 0;
		uint16_t value = nNearbyValue;
		if (value & 0x1)
		{
			//top face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y + blockSize;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z + blockSize;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x + blockSize;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z + blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x + blockSize;
			blockModel[3].position.y = vOffset.y + blockSize;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel += 4;
			nCount++;
		}
		if (value & (0x1 << 1))
		{
			//front face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x + blockSize;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x + blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel += 4;
			nCount++;
		}
		if (value & (0x1 << 2))
		{
			//bottom face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z + blockSize;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x + blockSize;
			blockModel[2].position.y = vOffset.y;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x + blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z + blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel += 4;
			nCount++;
		}
		if (value & (0x1 << 3))
		{
			//left face
			blockModel[0].position.x = vOffset.x;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z + blockSize;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z + blockSize;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel += 4;
			nCount++;
		}

		if (value & (0x1 << 4))
		{
			//right
			blockModel[0].position.x = vOffset.x + blockSize;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x + blockSize;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x + blockSize;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z + blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x + blockSize;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z + blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);

			blockModel += 4;
			nCount++;
		}

		if (value & (0x1 << 5))
		{
			//back face
			blockModel[0].position.x = vOffset.x + blockSize;
			blockModel[0].position.y = vOffset.y;
			blockModel[0].position.z = vOffset.z + blockSize;
			blockModel[0].texcoord = Vector2(0, fUV_Y_Offset + fUV_Y_Size);

			blockModel[1].position.x = vOffset.x + blockSize;
			blockModel[1].position.y = vOffset.y + blockSize;
			blockModel[1].position.z = vOffset.z + blockSize;
			blockModel[1].texcoord = Vector2(0, fUV_Y_Offset);

			blockModel[2].position.x = vOffset.x;
			blockModel[2].position.y = vOffset.y + blockSize;
			blockModel[2].position.z = vOffset.z + blockSize;
			blockModel[2].texcoord = Vector2(1.f, fUV_Y_Offset);

			blockModel[3].position.x = vOffset.x;
			blockModel[3].position.y = vOffset.y;
			blockModel[3].position.z = vOffset.z + blockSize;
			blockModel[3].texcoord = Vector2(1.f, fUV_Y_Offset + fUV_Y_Size);
			blockModel += 4;
			nCount++;
		}

		return nCount;
	}

	void BlockWorldClient::BuildSelectionBlockBuffer()
	{
		if (m_select_block_vertices.empty())
		{
			float blockSize = BlockConfig::g_blockSize;
			SelectBlockVertex blockModel[24];

			Vector3 vOffset(0, 0, 0);
			FillSelectBlockVertice(0xff, 0, 0, 0, blockModel, vOffset, BlockConfig::g_blockSize);

			m_select_block_vertices.resize(24 * m_maxSelectBlockPerBatch);

			SelectBlockVertex* pVertices = &(m_select_block_vertices[0]);
			for (int i = 0; i < m_maxSelectBlockPerBatch; i++)
			{
				memcpy(pVertices, blockModel, sizeof(SelectBlockVertex) * 24);
				pVertices += 24;
			}
		}
		if (m_select_block_indices.empty())
		{
			m_select_block_indices.resize(36 * m_maxSelectBlockPerBatch);

			uint16_t* pIndices = &(m_select_block_indices[0]);

			for (int i = 0; i < m_maxSelectBlockPerBatch; i++)
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
		if (!m_sharedIndexBuffer.IsValid())
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
		m_lightSphereVB.ReleaseBuffer();
		m_lightSphereIB.ReleaseBuffer();
		for (uint32_t i = 0; i < m_activeChunks.size(); i++)
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
		m_lightSphereVB.RendererRecreated();
		m_lightSphereIB.RendererRecreated();
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

	void BlockWorldClient::SetDamagedBlock(uint16_t x, uint16_t y, uint16_t z)
	{
		m_damagedBlockId.x = x;
		m_damagedBlockId.y = y;
		m_damagedBlockId.z = z;
	}

	void BlockWorldClient::SetDamagedBlockDegree(float degree)
	{
		m_damageDegree = degree;
	}

	void BlockWorldClient::SetSelectionTexture(const char* textureName)
	{
		if (textureName)
		{
			char c = textureName[0];
			int nIndex = 0;
			if (textureName[1] == ':' && c >= '0' && c <= '9')
			{
				nIndex = c - '0';
				textureName += 2;
			}
			if (nIndex < BLOCK_GROUP_ID_MAX)
			{
				if (m_highLightTextures[nIndex])
				{
					if (m_highLightTextures[nIndex]->GetKey() == textureName)
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
		if (m_highLightTextures[0])
		{
			return (m_highLightTextures[0]->GetKey());
		}
		return "";
	}

	void BlockWorldClient::SetDamageTexture(const char* textureName)
	{
		if (m_damageTexture)
		{
			if (m_damageTexture->GetKey() == textureName)
			{
				return;
			}
		}
		m_damageTexture = CGlobals::GetAssetManager()->LoadTexture("", textureName, TextureEntity::StaticTexture);
	}

	std::string BlockWorldClient::GetDamageTexture()
	{
		if (m_damageTexture)
		{
			return (m_damageTexture->GetKey());
		}
		return "";
	}

	bool BlockWorldClient::IsPointUnderWater(const Vector3& vPos)
	{
		if (IsInBlockWorld())
		{
			// first check block world and then check real world
			uint16_t block_id = GetBlockTemplateId(vPos.x, vPos.y, vPos.z);
			if (block_id > 0)
			{
				BlockTemplate* pBlockTemplate = GetBlockTemplate(block_id);
				if (pBlockTemplate != 0 && pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_liquid))
				{
					return true;
				}
			}
		}
		return CGlobals::GetOceanManager() ? CGlobals::GetOceanManager()->IsPointUnderWater(vPos) : false;
	}

	float BlockWorldClient::GetWaterLevel(float x, float y, float z, int nRayLength)
	{
		float fWaterLevel = -1000.f;
		if (m_isInWorld)
		{
			Uint16x3 blockIdx;
			BlockCommon::ConvertToBlockIndex(x, y, z, blockIdx.x, blockIdx.y, blockIdx.z);

			if (blockIdx.y < BlockConfig::g_regionBlockDimY)
			{
				for (int i = 0; i < nRayLength; i++)
				{
					uint16_t block_id = GetBlockTemplateIdByIdx(blockIdx.x, blockIdx.y - i, blockIdx.z);
					if (block_id > 0)
					{
						BlockTemplate* pBlockTemplate = GetBlockTemplate(block_id);
						if (pBlockTemplate != 0 && (pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_liquid) && !pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_solid)))
						{
							Vector3 vPos = BlockCommon::ConvertToRealPosition(blockIdx.x, blockIdx.y - i, blockIdx.z, 4);
							fWaterLevel = vPos.y - BlockConfig::g_blockSize * 0.2f;
						}
						break;
					}
				}
			}

			if (CGlobals::GetOceanManager() && CGlobals::GetOceanManager()->OceanEnabled())
			{
				float fLevel = CGlobals::GetOceanManager()->GetWaterLevel();
				if (fLevel > fWaterLevel)
					fWaterLevel = fLevel;
			}
		}
		return fWaterLevel;
	}

	void BlockWorldClient::SetBlockRenderMethod(BlockRenderMethod method)
	{
		if (m_dwBlockRenderMethod != method)
		{
			if (method == BLOCK_RENDER_FANCY_SHADER)
			{
				if (CGlobals::GetSettings().GetMultiSampleType() != 0)
				{
					OUTPUT_LOG("BLOCK_RENDER_FANCY_SHADER can not be set with AA\n");
					return;
				}
				// check for device capabilities via ParaRHI
				if (CanUseAdvancedShading())
				{
					if (CGlobals::GetSettings().GetGameEffectSet() >= 1024)
					{
						CGlobals::GetSettings().LoadGameEffectSet(0);
					}
					CGlobals::GetScene()->EnableFullScreenGlow(false);
				}
				else
				{
					OUTPUT_LOG("warning: your device is too low to use deferred shading. \n");
					return;
				}

				CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
				if (pShadowMap != NULL)
				{
					CGlobals::GetScene()->SetShadow(GetUseSunlightShadowMap());
				}
				auto* pRenderDevice = CGlobals::GetRenderDevice();
				const bool bCanInstanceFancy = pRenderDevice
					&& pRenderDevice->GetCaps().supportsInstancing
					&& !CGlobals::GetSettings().IsLowEndRenderDevice();
				CGlobals::GetScene()->EnableInstancing(bCanInstanceFancy);

				// replacing shader to support multiple render target.
				if (m_terrain_fancy)
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_TERRAIN, m_terrain_fancy.get());
				}
				if (m_normal_mesh_effect_fancy)
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL, m_normal_mesh_effect_fancy.get());
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_CHARACTER, m_normal_mesh_effect_fancy.get());
				}
				if (m_mesh_instanced_fancy && bCanInstanceFancy)
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_INSTANCED, m_mesh_instanced_fancy.get());
				}
				if (m_bmax_model_effect_fancy)
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_BMAX_MODEL, m_bmax_model_effect_fancy.get());
				}
				if (!CGlobals::GetSettings().IsLowEndRenderDevice() && pRenderDevice && pRenderDevice->GetCaps().GetMaxGpuSkinningBones() > 0)
				{
					if (m_character_skinned_fancy)
						CGlobals::GetEffectManager()->MapHandleToEffect(TECH_CHARACTER_SKINNED, m_character_skinned_fancy.get());
					if (m_bmax_skinned_fancy)
						CGlobals::GetEffectManager()->MapHandleToEffect(TECH_BMAX_SKINNED, m_bmax_skinned_fancy.get());
				}
			}
			else
			{
				// reset effect mapping to default values.
				CGlobals::GetSettings().LoadGameEffectSet(CGlobals::GetSettings().GetGameEffectSet());
			}

			if (m_dwBlockRenderMethod == BLOCK_RENDER_FANCY_SHADER)
			{
				auto* pDevice = CGlobals::GetRenderDevice();
				if (pDevice)
				{
					pDevice->SetRenderTarget(3, nullptr);
					pDevice->SetRenderTarget(2, nullptr);
					pDevice->SetRenderTarget(1, nullptr);
					if (m_bAutoBoundColorRT0)
					{
						pDevice->SetRenderTarget(0, nullptr);
						m_bAutoBoundColorRT0 = false;
					}
				}
				m_render_target_color_hdr.reset();
				m_render_target_color.reset();
				m_render_target_scene_color.reset();
				m_render_target_block_info.reset();
				m_render_target_depth_tex.reset();
				m_render_target_normal.reset();
			}
			m_dwBlockRenderMethod = method;

			ClearBlockRenderCache();
		}
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

	void BlockWorldClient::SetBlockLightColor(const LinearColor& color)
	{
		m_vBlockLightColor = color;
	}

	LinearColor BlockWorldClient::GetBlockLightColor()
	{
		return m_vBlockLightColor;
	}

	bool BlockWorldClient::CompareRenderOrder(BlockRenderTask* v0, BlockRenderTask* v1)
	{
		return (v0->GetRenderOrder() < v1->GetRenderOrder());
	}

	bool BlockWorldClient::GetUsePointTextureFiltering()
	{
		return m_bUsePointTextureFiltering;
	}

	void BlockWorldClient::SetUsePointTextureFiltering(bool bUse)
	{
		m_bUsePointTextureFiltering = bUse;
	}

	void BlockWorldClient::InitDeviceObjects()
	{
		if (IsInBlockWorld())
			UpdateAllActiveChunks();
	}

	void BlockWorldClient::RestoreDeviceObjects()
	{
	}

	void BlockWorldClient::InvalidateDeviceObjects()
	{
		m_bAutoBoundColorRT0 = false;
		m_render_target_color_hdr.reset();
		m_render_target_color.reset();
		m_render_target_scene_color.reset();
		m_render_target_block_info.reset();
		m_render_target_depth_tex.reset();
		m_render_target_normal.reset();
	}

	void BlockWorldClient::FinishAutoBoundSceneColorRT()
	{
		if (!m_bAutoBoundColorRT0)
			return;
		auto* pDevice = CGlobals::GetRenderDevice();
		if (pDevice)
		{
			pDevice->SetRenderTarget(3, nullptr);
			pDevice->SetRenderTarget(2, nullptr);
			pDevice->SetRenderTarget(1, nullptr);
			IRHIRenderTarget* sceneColorRT = m_render_target_scene_color ? m_render_target_scene_color->GetRHIRenderTarget() : nullptr;
			if (sceneColorRT && pDevice->GetCurrentRenderTargetBinding().targets[0] == sceneColorRT)
			{
				pDevice->BlitRenderTarget(sceneColorRT, nullptr, nullptr, nullptr, false);
				pDevice->SetRenderTarget(0, nullptr);
				if (CGlobals::GetViewportManager() && CGlobals::GetViewportManager()->GetActiveViewPort())
					CGlobals::GetViewportManager()->GetActiveViewPort()->ApplyViewport();
			}
		}
		m_bAutoBoundColorRT0 = false;
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
		auto* pDevice = CGlobals::GetRenderDevice();
		if (!CGlobals::GetSettings().IsLowEndRenderDevice())
		{
			if (pDevice && pDevice->GetCaps().supportsInstancing)
			{
				pEffect = pEffectManager->GetEffectByName("simple_mesh_normal_instanced");
				if (pEffect)
				{
					pEffect->LoadAsset();
					pEffectManager->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_INSTANCED, pEffect);
				}
			}
			if (!pDevice || pDevice->GetCaps().GetMaxGpuSkinningBones() > 0)
			{
				pEffect = pEffectManager->GetEffectByName("character_skinned");
				if (pEffect)
				{
					pEffect->LoadAsset();
					pEffectManager->MapHandleToEffect(TECH_CHARACTER_SKINNED, pEffect);
				}
				pEffect = pEffectManager->GetEffectByName("bmax_model_skinned");
				if (pEffect)
				{
					pEffect->LoadAsset();
					pEffectManager->MapHandleToEffect(TECH_BMAX_SKINNED, pEffect);
				}
			}
		}
		CGlobals::GetSceneState()->SetDeferredShading(true);
	}

	void BlockWorldClient::DoPostRenderingProcessing(BlockRenderPass nRenderPass)
	{
		if (IsInBlockWorld() && GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER)
		{
			if (nRenderPass == BlockRenderPass_Opaque)
			{
				// Unbind G-buffer MRT slots 3..1 BEFORE running the composite Lua script, which samples them as SRVs.
				if (auto* pDevice = CGlobals::GetRenderDevice())
				{
					pDevice->SetRenderTarget(3, nullptr);
					pDevice->SetRenderTarget(2, nullptr);
					pDevice->SetRenderTarget(1, nullptr);
				}

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
		if (nRenderPass == BlockRenderPass_AlphaBlended)
		{
			FinishAutoBoundSceneColorRT();
		}
	}

	bool BlockWorldClient::PrepareAllRenderTargets(bool bSetRenderTarget /*= true*/)
	{
		if (GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER)
		{
			RenderDevicePtr pDevice = CGlobals::GetRenderDevice();
			if (!pDevice || !CanUseAdvancedShading())
				return false;

			if (m_block_effect_fancy == 0)
			{
				m_block_effect_fancy = CGlobals::GetAssetManager()->LoadEffectFile("blockFancy", ":IDR_FX_MRT_BLOCKS");
				m_block_effect_fancy->LoadAsset();
				if (!m_block_effect_fancy->IsValid())
				{
					return false;
				}
			}
			else if (!m_block_effect_fancy->IsValid())
			{
				return false;
			}

			if (!IsInBlockWorld())
			{
				RestoreNormalShader();
				return false;
			}

			if (m_render_target_block_info == 0)
			{
				m_render_target_block_info = CGlobals::GetAssetManager()->LoadTexture("_BlockInfoRT", "_BlockInfoRT", TextureEntity::RenderTarget);
				m_render_target_block_info->LoadAsset();
			}

			if (m_render_target_depth_tex == 0)
			{
				m_render_target_depth_tex = CGlobals::GetAssetManager()->LoadTexture("_DepthTexRT_R32F", "_DepthTexRT_R32F", TextureEntity::RenderTarget);
				m_render_target_depth_tex->LoadAsset();
			}
			if (m_render_target_normal == 0)
			{
				m_render_target_normal = CGlobals::GetAssetManager()->LoadTexture("_NormalRT", "_NormalRT", TextureEntity::RenderTarget);
				m_render_target_normal->LoadAsset();
			}

			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_BLOCK_FANCY, m_block_effect_fancy.get());

			if (m_normal_mesh_effect_fancy == 0)
			{
				m_normal_mesh_effect_fancy = CGlobals::GetAssetManager()->LoadEffectFile("mesh_normal_fancy", ":IDR_FX_MRT_MESH_NORMAL");
				m_normal_mesh_effect_fancy->LoadAsset();
				if (!m_normal_mesh_effect_fancy->IsValid())
				{
					return false;
				}
			}
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL, m_normal_mesh_effect_fancy.get());
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_CHARACTER, m_normal_mesh_effect_fancy.get());

			if (pDevice->GetCaps().supportsInstancing && !CGlobals::GetSettings().IsLowEndRenderDevice())
			{
				if (m_mesh_instanced_fancy == 0)
				{
					m_mesh_instanced_fancy = CGlobals::GetAssetManager()->LoadEffectFile("mesh_instanced_fancy", ":IDR_FX_MRT_MESH_NORMAL_INSTANCED");
					m_mesh_instanced_fancy->LoadAsset();
				}
				if (m_mesh_instanced_fancy && m_mesh_instanced_fancy->IsValid())
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_INSTANCED, m_mesh_instanced_fancy.get());
				}
			}

			if (m_effect_light_point == 0)
			{
				m_effect_light_point = CGlobals::GetAssetManager()->LoadEffectFile("m_effect_light_point", ":IDR_FX_DEFERRED_POINT_LIGHTING");
				m_effect_light_point->LoadAsset();
			}
			if (m_effect_light_spot == 0)
			{
				m_effect_light_spot = CGlobals::GetAssetManager()->LoadEffectFile("m_effect_light_spot", ":IDR_FX_DEFERRED_SPOT_LIGHTING");
				m_effect_light_spot->LoadAsset();
			}
			if (m_effect_light_directional == 0)
			{
				m_effect_light_directional = CGlobals::GetAssetManager()->LoadEffectFile("m_effect_light_directional", ":IDR_FX_DEFERRED_DIRECTIONAL_LIGHTING");
				m_effect_light_directional->LoadAsset();
			}
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_LIGHT_POINT, m_effect_light_point.get());
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_LIGHT_SPOT, m_effect_light_spot.get());
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_LIGHT_DIRECTIONAL, m_effect_light_directional.get());

			if (m_bmax_model_effect_fancy == 0)
			{
				m_bmax_model_effect_fancy = CGlobals::GetAssetManager()->LoadEffectFile("bmax_model_effect_fancy", ":IDR_FX_MRT_BMAX_MODEL");
				m_bmax_model_effect_fancy->LoadAsset();
				if (!m_bmax_model_effect_fancy->IsValid())
				{
					return false;
				}
			}
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_BMAX_MODEL, m_bmax_model_effect_fancy.get());

			if (!CGlobals::GetSettings().IsLowEndRenderDevice() && pDevice->GetCaps().GetMaxGpuSkinningBones() > 0)
			{
				if (m_character_skinned_fancy == 0)
				{
					m_character_skinned_fancy = CGlobals::GetAssetManager()->LoadEffectFile("character_skinned_fancy", ":IDR_FX_MRT_CHARACTER_SKINNED");
					m_character_skinned_fancy->LoadAsset();
				}
				if (m_character_skinned_fancy && m_character_skinned_fancy->IsValid())
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_CHARACTER_SKINNED, m_character_skinned_fancy.get());
				}

				if (m_bmax_skinned_fancy == 0)
				{
					m_bmax_skinned_fancy = CGlobals::GetAssetManager()->LoadEffectFile("bmax_skinned_fancy", ":IDR_FX_MRT_BMAX_MODEL_SKINNED");
					m_bmax_skinned_fancy->LoadAsset();
				}
				if (m_bmax_skinned_fancy && m_bmax_skinned_fancy->IsValid())
				{
					CGlobals::GetEffectManager()->MapHandleToEffect(TECH_BMAX_SKINNED, m_bmax_skinned_fancy.get());
				}
			}

			if (m_terrain_fancy == 0)
			{
				m_terrain_fancy = CGlobals::GetAssetManager()->LoadEffectFile("terrain_normal_fancy", ":IDR_FX_MRT_TERRAIN_NORMAL");
				m_terrain_fancy->LoadAsset();
				if (!m_terrain_fancy->IsValid())
				{
					return false;
				}
			}
			CGlobals::GetEffectManager()->MapHandleToEffect(TECH_TERRAIN, m_terrain_fancy.get());

			if (bSetRenderTarget)
			{
				IRHIRenderTarget* depthRT = m_render_target_depth_tex ? m_render_target_depth_tex->GetRHIRenderTarget() : nullptr;
				IRHIRenderTarget* blockInfoRT = m_render_target_block_info ? m_render_target_block_info->GetRHIRenderTarget() : nullptr;
				IRHIRenderTarget* normalRT = m_render_target_normal ? m_render_target_normal->GetRHIRenderTarget() : nullptr;
				if (!depthRT || !blockInfoRT || !normalRT)
					return false;

				static int nLastFrameCount = 0;
				const int curFrameCount = CGlobals::GetSceneState()->GetRenderFrameCount();
				const bool bIsNewFrame = (nLastFrameCount != curFrameCount);
				if (bIsNewFrame)
				{
					nLastFrameCount = curFrameCount;
					IRHIRenderTarget* gbufferRTs[3] = { depthRT, blockInfoRT, normalRT };
					for (IRHIRenderTarget* gbRT : gbufferRTs)
					{
						pDevice->PushRenderTarget(gbRT);
						pDevice->SetClearColor(Color4f(0.f, 0.f, 0.f, 0.f));
						pDevice->Clear(true, false, false);
						pDevice->PopRenderTarget();
					}
				}

				IRHIRenderTarget* rt0 = pDevice->GetCurrentRenderTargetBinding().targets[0];
				const ERHIBackend backend = pDevice->GetBackendType();
				const bool supportsSwapchainMRT = (backend == ERHIBackend::D3D11 || backend == ERHIBackend::D3D12 || backend == ERHIBackend::Vulkan);
				if (rt0 == nullptr && !supportsSwapchainMRT)
				{
					// OpenGL/Metal/WebGPU default FBO 0 cannot attach offscreen MRT color attachments:
					// render into a dedicated screen-sized RT (distinct from _ColorRT used by FancyPostProcessing.lua).
					if (m_render_target_scene_color == 0)
					{
						m_render_target_scene_color = CGlobals::GetAssetManager()->LoadTexture("_DeferredSceneColorRT", "_DeferredSceneColorRT", TextureEntity::RenderTarget);
					}
					m_render_target_scene_color->LoadAsset();
					rt0 = m_render_target_scene_color->GetRHIRenderTarget();
					if (!rt0)
						return false;
					if (bIsNewFrame)
					{
						const bool copiedBackbuffer = pDevice->BlitRenderTarget(nullptr, rt0, nullptr, nullptr, false);
						pDevice->PushRenderTarget(rt0);
						if (!copiedBackbuffer)
						{
							LinearColor bg = CGlobals::GetScene() ? CGlobals::GetScene()->GetFogColor() : LinearColor(0.f, 0.f, 0.f, 0.f);
							pDevice->SetClearColor(Color4f(bg.r, bg.g, bg.b, bg.a));
						}
						pDevice->SetClearDepth(1.0f);
						pDevice->SetClearStencil(0);
						pDevice->Clear(!copiedBackbuffer, true, true);
						pDevice->PopRenderTarget();
					}
					m_bAutoBoundColorRT0 = true;
				}

				IRHIRenderTarget* mrtTargets[4] = { rt0, depthRT, blockInfoRT, normalRT };
				IRHIRenderTarget* depthStencilRT = (rt0 && rt0->GetDepthTexture()) ? rt0 : pDevice->GetCurrentRenderTargetBinding().depthStencilOverride;
				if (!pDevice->SetRenderTargets(4, mrtTargets, depthStencilRT))
					return false;

				CGlobals::GetSceneState()->SetDeferredShading(true);
				if (CGlobals::GetViewportManager() && CGlobals::GetViewportManager()->GetActiveViewPort())
					CGlobals::GetViewportManager()->GetActiveViewPort()->ApplyViewport();
			}
		}
		return true;
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
		auto* pDevice = CGlobals::GetRenderDevice();
		if (!pDevice || pDevice->GetBackendType() == ERHIBackend::Null)
			return false;
		const RHIDeviceCaps& caps = pDevice->GetCaps();
		if (caps.IsLowEndDevice())
			return false;
		return caps.GetShaderModelMajor() >= 3 && caps.maxSimultaneousRTs >= 4 && caps.supportsFloatRenderTargets;
	}

	void BlockWorldClient::SetUseSunlightShadowMap(bool bEnable)
	{
		if (bEnable && CGlobals::GetSettings().IsLowEndRenderDevice())
			bEnable = false;
		m_bUseSunlightShadowMap = bEnable;
		if (GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER)
		{
			CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
			if (pShadowMap != NULL)
			{
				CGlobals::GetScene()->SetShadow(m_bUseSunlightShadowMap);
			}
		}
	}

	bool BlockWorldClient::HasSunlightShadowMap()
	{
		return GetUseSunlightShadowMap() && CGlobals::GetScene()->IsShadowMapEnabled();
	}

	bool BlockWorldClient::GetUseSunlightShadowMap()
	{
		if (CGlobals::GetSettings().IsLowEndRenderDevice())
			return false;
		return m_bUseSunlightShadowMap;
	}

	void BlockWorldClient::SetUseWaterReflection(bool bEnable)
	{
		if (bEnable && CGlobals::GetSettings().IsLowEndRenderDevice())
			bEnable = false;
		m_bUseWaterReflection = bEnable;
	}

	bool BlockWorldClient::GetUseWaterReflection()
	{
		if (CGlobals::GetSettings().IsLowEndRenderDevice())
			return false;
		return m_bUseWaterReflection;
	}

	void BlockWorldClient::PrepareShadowCasters(CShadowMap* pShadowMap)
	{
		if (pShadowMap && GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER && GetUseSunlightShadowMap())
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
	}

	void BlockWorldClient::RenderDynamicBlocks()
	{
		// no need to lock

		int nCount = (int)m_selectedBlockMap.size();
		for (int i = 0; i < nCount; ++i)
		{
			auto& select_group = m_selectedBlockMap[i];
			if (select_group.GetBlocks().size() > 0)
			{
				if (select_group.m_bWireFrame)
					RenderWireFrameBlock(i, select_group.m_fScaling, &select_group.m_color);
				else
					RenderSelectionBlock(i, select_group.m_fScaling, &select_group.m_color, select_group.m_bEnableBling);
			}
		}

		if (m_damageDegree > 0)
			RenderDamagedBlock();
	}

	BlockWorldClient* BlockWorldClient::GetInstance()
	{
		return g_pInstance;
	}

	void BlockWorldClient::GetMaxBlockHeightWatchingSky(uint16_t blockX_ws, uint16_t blockZ_ws, ChunkMaxHeight* pResult)
	{
		CBlockWorld::GetMaxBlockHeightWatchingSky(blockX_ws, blockZ_ws, pResult);
#ifndef NPLRUNTIME_SERVER
		ParaTerrain::CGlobalTerrain* pTerrain = CGlobals::GetGlobalTerrain();
		uint16_t mask = 0;
		if (pTerrain && pTerrain->TerrainRenderingEnabled())
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
		if (m_activeChunkDim <= 0 || m_activeChunks.empty())
			return;

		Vector3 camWorldPos = CGlobals::GetSceneState()->vEye;
		Vector3 renderOrig = CGlobals::GetScene()->GetRenderOrigin();
		int nRenderFrameCount = CGlobals::GetSceneState()->GetRenderFrameCount();

		CCameraFrustum* frustum = nullptr;
		static CCameraFrustum caster_frustum;

		Vector3 camMin;
		Vector3 camMax;

		if (bIsShadowPass)
		{
			CShadowMap* pShadowMap = CGlobals::GetEffectManager() ? CGlobals::GetEffectManager()->GetShadowMap() : nullptr;
			if (!pShadowMap)
				return;
			frustum = &caster_frustum;
			Vector3 vCenter = camWorldPos - renderOrig;
			float fRadius = (std::min)(CGlobals::GetScene()->GetShadowRadius(), 50.f);

			frustum->UpdateFrustum(pShadowMap->GetViewProjMatrix());
			camMin = Vector3(vCenter.x - fRadius, vCenter.y - fRadius, vCenter.z - fRadius);
			camMax = Vector3(vCenter.x + fRadius, vCenter.y + fRadius, vCenter.z + fRadius);
		}
		else
		{
			auto* pCam = CGlobals::GetScene()->GetCurrentCamera();
			if (!pCam)
				return;
			frustum = pCam->GetFrustum();

			Vector3* frusCorner = frustum->vecFrustum;
			camMin = frusCorner[0];
			camMax = frusCorner[0];

			for (int i = 1; i < 8; i++)
			{
				Vector3& v = frusCorner[i];
				if (camMin.x > v.x) camMin.x = v.x;
				if (camMin.y > v.y) camMin.y = v.y;
				if (camMin.z > v.z) camMin.z = v.z;
				if (camMax.x < v.x) camMax.x = v.x;
				if (camMax.y < v.y) camMax.y = v.y;
				if (camMax.z < v.z) camMax.z = v.z;
			}
		}
		camMax += renderOrig;
		camMin += renderOrig;

		Uint16x3 camBlockPos;
		BlockCommon::ConvertToBlockIndex(camWorldPos.x, camWorldPos.y, camWorldPos.z, camBlockPos.x, camBlockPos.y, camBlockPos.z);
		SetEyeBlockId(camBlockPos);

		const int32 minActiveX = (std::max<int32>)(0, static_cast<int32>(m_minActiveChunkId_ws.x));
		const int32 maxActiveX = static_cast<int32>(m_minActiveChunkId_ws.x) + m_activeChunkDim - 1;
		const int32 minActiveZ = (std::max<int32>)(0, static_cast<int32>(m_minActiveChunkId_ws.z));
		const int32 maxActiveZ = static_cast<int32>(m_minActiveChunkId_ws.z) + m_activeChunkDim - 1;
		if (minActiveX > maxActiveX || minActiveZ > maxActiveZ)
			return;

		Uint16x3 rawStart, rawEnd;
		BlockCommon::ConvertToBlockIndex(camMin.x, camMin.y, camMin.z, rawStart.x, rawStart.y, rawStart.z);
		BlockCommon::ConvertToBlockIndex(camMax.x, camMax.y, camMax.z, rawEnd.x, rawEnd.y, rawEnd.z);

		const int32 startIdxX = (std::max<int32>)(minActiveX, static_cast<int32>(rawStart.x / 16) - 1);
		const int32 startIdxY = (std::max<int32>)(0, (std::min<int32>)(15, static_cast<int32>(rawStart.y / 16) - 1));
		const int32 startIdxZ = (std::max<int32>)(minActiveZ, static_cast<int32>(rawStart.z / 16) - 1);

		const int32 endIdxX = (std::min<int32>)(maxActiveX, static_cast<int32>(rawEnd.x / 16) + 1);
		const int32 endIdxY = (std::min<int32>)(15, static_cast<int32>(rawEnd.y / 16) + 1);
		const int32 endIdxZ = (std::min<int32>)(maxActiveZ, static_cast<int32>(rawEnd.z / 16) + 1);

		const float fRenderDist = GetRenderDist() * BlockConfig::g_blockSize;
		CShapeSphere sEyeSphere(camWorldPos, fRenderDist);
		const Vector3 vChunkSize(BlockConfig::g_chunkSize, BlockConfig::g_chunkSize, BlockConfig::g_chunkSize);

		const int32 chunkX = GetEyeChunkId().x;
		const int32 chunkY = GetEyeChunkId().y;
		const int32 chunkZ = GetEyeChunkId().z;

		if (!bIsShadowPass)
		{
			Matrix4 matViewProj = Matrix4::IDENTITY;
			if (auto* pEffMgr = CGlobals::GetEffectManager())
			{
				matViewProj = pEffMgr->GetViewTransform() * pEffMgr->GetProjTransform();
			}
			m_lastMainEyePos = camWorldPos;
			m_lastMainViewProj = matViewProj;
			m_bHasLastMainCameraState = true;

			bool clipZeroToOne = true;
			if (auto* pDev = CGlobals::GetRenderDevice())
				clipZeroToOne = pDev->GetCaps().clipSpaceZeroToOne;
			m_softwareOccluder.BeginFrame(matViewProj, camWorldPos, renderOrig, clipZeroToOne);

			if (startIdxX > endIdxX || startIdxZ > endIdxZ)
			{
				m_isVisibleChunkDirty = false;
				return;
			}

			const double eyeRelY = static_cast<double>(camWorldPos.y) - CBlockWorld::GetVerticalOffset();
			const int32 rawSeedY = (eyeRelY < 0.0) ? -1 : ((chunkY >= 16) ? 16 : chunkY);
			const int32 bfsMinY = (rawSeedY < 0) ? -1 : 0;
			const int32 bfsMaxY = (rawSeedY >= 16) ? 16 : 15;

			const int32 seedX = (std::max)(minActiveX, (std::min)(maxActiveX, chunkX));
			const int32 seedY = rawSeedY;
			const int32 seedZ = (std::max)(minActiveZ, (std::min)(maxActiveZ, chunkZ));
			const bool bSeedClampedXZ = (seedX != chunkX || seedZ != chunkZ);

			const int32 chunkViewRadius = (std::max<int32>)(static_cast<int32>(GetRenderDist() / 16), 1);
			const int32 chunkViewSize = chunkViewRadius * 2;
			const int32 viewMinX = (std::max)(minActiveX, chunkX - chunkViewSize);
			const int32 viewMaxX = (std::min)(maxActiveX, chunkX + chunkViewSize);
			const int32 viewMinZ = (std::max)(minActiveZ, chunkZ - chunkViewSize);
			const int32 viewMaxZ = (std::min)(maxActiveZ, chunkZ + chunkViewSize);

			const int32 travMinX = (std::max)(viewMinX, (std::min)(startIdxX, seedX));
			const int32 travMaxX = (std::min)(viewMaxX, (std::max)(endIdxX, seedX));
			const int32 travMinZ = (std::max)(viewMinZ, (std::min)(startIdxZ, seedZ));
			const int32 travMaxZ = (std::min)(viewMaxZ, (std::max)(endIdxZ, seedZ));
			const int32 travMinY = (std::min)(startIdxY, seedY);
			const int32 travMaxY = (std::max)(endIdxY, seedY);

			const size_t dim = static_cast<size_t>(m_activeChunkDim);
			const size_t dimXZ = dim * dim;
			const size_t totalBfsSlots = dimXZ * 18u; // cy in [-1..16]
			if (m_bfsVisitedGen.size() < totalBfsSlots)
			{
				m_bfsVisitedGen.assign(totalBfsSlots, 0u);
				m_bfsEntryFaces.assign(totalBfsSlots, 0u);
			}
			++m_bfsGeneration;
			if (m_bfsGeneration == 0u)
			{
				std::fill(m_bfsVisitedGen.begin(), m_bfsVisitedGen.end(), 0u);
				m_bfsGeneration = 1u;
			}
			const uint32_t curGen = m_bfsGeneration;

			auto calcBfsIdx = [&](int32 x, int32 y, int32 z) -> size_t {
				const size_t lx = static_cast<size_t>(x - static_cast<int32>(m_minActiveChunkId_ws.x));
				const size_t lz = static_cast<size_t>(z - static_cast<int32>(m_minActiveChunkId_ws.z));
				return static_cast<size_t>(y + 1) * dimXZ + lx * dim + lz;
			};

			m_bfsQueue.clear();
			if (m_bfsQueue.capacity() < 4096)
				m_bfsQueue.reserve(4096);

			const size_t seedSlot = calcBfsIdx(seedX, seedY, seedZ);
			m_bfsVisitedGen[seedSlot] = curGen;
			m_bfsEntryFaces[seedSlot] = 0x3Fu;
			m_bfsQueue.push_back({ static_cast<int16_t>(seedX), static_cast<int16_t>(seedY), static_cast<int16_t>(seedZ) });

			struct StepDir
			{
				int8_t dx, dy, dz;
				uint8_t exitFace;
				uint8_t neighborEntryFace;
			};
			// Matches BlockChunk::UpdateVisibilityGraph boundary face convention:
			// cx==15 -> left (+X), cx==0 -> right (-X), cy==15 -> top (+Y), cy==0 -> bottom (-Y), cz==15 -> front (+Z), cz==0 -> back (-Z)
			static const StepDir kStepDirs[6] = {
				{ +1,  0,  0, static_cast<uint8_t>(BlockDirection::left),   static_cast<uint8_t>(BlockDirection::right)  },
				{ -1,  0,  0, static_cast<uint8_t>(BlockDirection::right),  static_cast<uint8_t>(BlockDirection::left)   },
				{  0, +1,  0, static_cast<uint8_t>(BlockDirection::top),    static_cast<uint8_t>(BlockDirection::bottom) },
				{  0, -1,  0, static_cast<uint8_t>(BlockDirection::bottom), static_cast<uint8_t>(BlockDirection::top)    },
				{  0,  0, +1, static_cast<uint8_t>(BlockDirection::front),  static_cast<uint8_t>(BlockDirection::back)   },
				{  0,  0, -1, static_cast<uint8_t>(BlockDirection::back),   static_cast<uint8_t>(BlockDirection::front)  },
			};

			size_t qHead = 0;
			while (qHead < m_bfsQueue.size())
			{
				const BfsChunkNode cur = m_bfsQueue[qHead++];
				const int32 cx = cur.x;
				const int32 cy = cur.y;
				const int32 cz = cur.z;
				const size_t curSlot = calcBfsIdx(cx, cy, cz);
				const uint8_t entryMask = m_bfsEntryFaces[curSlot];
				const int32 manhattanDist = std::abs(cx - seedX) + std::abs(cy - seedY) + std::abs(cz - seedZ);

				uint16_t visGraph = BlockChunk::kVisGraphAllConnected;

				if (cy >= 0 && cy <= 15)
				{
					Vector3 vMin = BlockCommon::ConvertToRealPosition(static_cast<uint16>(cx * 16), static_cast<uint16>(cy * 16), static_cast<uint16>(cz * 16), 7);
					vMin -= renderOrig;
					Vector3 vMax = vMin + vChunkSize;

					CShapeAABB box;
					box.SetMinMax(vMin, vMax);
					const bool inFrustum = frustum->TestBox(&box) != 0;
					if (!inFrustum && manhattanDist > 1 && !bSeedClampedXZ)
						continue;

					if (inFrustum && manhattanDist >= 2 && m_softwareOccluder.IsRenderAABBOccluded(box))
						continue;

					RenderableChunk& chunk = GetActiveChunk(static_cast<uint16>(cx), static_cast<uint16>(cy), static_cast<uint16>(cz));
					if (!chunk.IsEmptyChunk())
					{
						chunk.RefreshVisibilityCache(this);
						const bool bChunkDirty = chunk.IsDirty();
						if (!bChunkDirty)
						{
							visGraph = chunk.GetVisibilityGraph();
						}

						const int32 viewDist = (std::max)((std::max)(std::abs(cx - chunkX), std::abs(cz - chunkZ)), std::abs(cy - chunkY));
						if (inFrustum
							&& cx >= startIdxX && cx <= endIdxX
							&& cy >= startIdxY && cy <= endIdxY
							&& cz >= startIdxZ && cz <= endIdxZ
							&& chunk.IsIntersect(sEyeSphere)
							&& chunk.IsNearbyChunksLoaded())
						{
							if (bChunkDirty || chunk.GetTotalFaceCount() > 0 || chunk.ShouldRebuildRenderBuffer(this, false, true))
							{
								AddToVisibleChunk(chunk, viewDist, nRenderFrameCount);
							}
							if (!bChunkDirty && viewDist <= 8)
							{
								Vector3 occMin, occMax;
								if (chunk.GetOpaqueOccluderBoxWS(occMin, occMax))
								{
									m_softwareOccluder.RasterizeOccluderWorldAABB(occMin, occMax);
								}
							}
						}
					}
				}

				for (int d = 0; d < 6; ++d)
				{
					const StepDir& step = kStepDirs[d];
					if (step.dx != 0 && (cx - seedX) * step.dx < 0) continue;
					if (step.dy != 0 && (cy - seedY) * step.dy < 0) continue;
					if (step.dz != 0 && (cz - seedZ) * step.dz < 0) continue;

					const int32 nx = cx + step.dx;
					const int32 ny = cy + step.dy;
					const int32 nz = cz + step.dz;
					if (nx < travMinX || nx > travMaxX || ny < bfsMinY || ny > bfsMaxY || nz < travMinZ || nz > travMaxZ)
						continue;
					if (ny >= 0 && ny <= 15 && (ny < travMinY || ny > travMaxY))
						continue;

					if (manhattanDist > 0 && visGraph != BlockChunk::kVisGraphAllConnected)
					{
						if (visGraph == BlockChunk::kVisGraphFullyOpaque)
							continue;
						bool bConnected = false;
						for (int ef = 0; ef < 6; ++ef)
						{
							if ((entryMask & (1u << ef)) != 0 && BlockChunk::CanTraverseVisGraph(visGraph, ef, step.exitFace))
							{
								bConnected = true;
								break;
							}
						}
						if (!bConnected)
							continue;
					}

					const size_t nSlot = calcBfsIdx(nx, ny, nz);
					const uint8_t entryBit = static_cast<uint8_t>(1u << step.neighborEntryFace);
					if (m_bfsVisitedGen[nSlot] != curGen)
					{
						m_bfsVisitedGen[nSlot] = curGen;
						m_bfsEntryFaces[nSlot] = entryBit;
						m_bfsQueue.push_back({ static_cast<int16_t>(nx), static_cast<int16_t>(ny), static_cast<int16_t>(nz) });
					}
					else
					{
						m_bfsEntryFaces[nSlot] |= entryBit;
					}
				}
			}

			m_softwareOccluder.FinalizeHierarchy();
			m_isVisibleChunkDirty = false;
			return;
		}

		if (startIdxX > endIdxX || startIdxZ > endIdxZ)
			return;

		// Shadow pass: radial spiral walk bounded tightly to the shadow caster AABB, skipping empty / 0-face interior chunks.
		int32 chunkViewRadius = (std::max<int32>)(static_cast<int32>(GetRenderDist() / 16), 1);
		int32 maxShadowExtent = (std::max)(
			(std::max)(std::abs(startIdxX - chunkX), std::abs(endIdxX - chunkX)),
			(std::max)(std::abs(startIdxZ - chunkZ), std::abs(endIdxZ - chunkZ)));
		int32 chunkViewSize = (std::min)(chunkViewRadius * 2, maxShadowExtent);

		auto tryAddShadowChunk = [&](int32 cx, int32 cz, int32 dist) {
			if (cx < startIdxX || cx > endIdxX || cz < startIdxZ || cz > endIdxZ)
				return;
			for (int32 cy = startIdxY; cy <= endIdxY; ++cy)
			{
				RenderableChunk& chunk = GetActiveChunk(static_cast<uint16>(cx), static_cast<uint16>(cy), static_cast<uint16>(cz));
				if (!chunk.IsIntersect(sEyeSphere))
					continue;
				if (!chunk.IsDirty() && chunk.GetTotalFaceCount() == 0 && !chunk.ShouldRebuildRenderBuffer(this, false, true))
					continue;

				Vector3 vMin = BlockCommon::ConvertToRealPosition(static_cast<uint16>(cx * 16), static_cast<uint16>(cy * 16), static_cast<uint16>(cz * 16), 7);
				vMin -= renderOrig;
				Vector3 vMax = vMin + vChunkSize;

				CShapeAABB box;
				box.SetMinMax(vMin, vMax);
				if (frustum->TestBox(&box) && chunk.IsNearbyChunksLoaded())
				{
					AddToVisibleChunk(chunk, dist, nRenderFrameCount);
				}
			}
		};

		tryAddShadowChunk(chunkX, chunkZ, 0);

		int32 nIndex = 0;
		int32 dx = 0;
		int32 dz = 0;
		for (int32 length = 1; length <= chunkViewSize; ++length)
		{
			for (int32 k = 1; k <= 2; ++k)
			{
				int32* dir = &(BlockCommon::g_xzDirectionsConst[(nIndex % 4)][0]);
				nIndex++;
				for (int32 i = 0; i < length; ++i)
				{
					dx += dir[0];
					dz += dir[1];
					tryAddShadowChunk(chunkX + dx, chunkZ + dz, length);
				}
			}
		}

		nIndex = (nIndex % 4);
		for (int32 i = 1; i <= chunkViewSize; ++i)
		{
			dx += BlockCommon::g_xzDirectionsConst[nIndex][0];
			dz += BlockCommon::g_xzDirectionsConst[nIndex][1];
			tryAddShadowChunk(chunkX + dx, chunkZ + dz, chunkViewSize);
		}
	}

	void BlockWorldClient::AddToVisibleChunk(RenderableChunk& chunk, int nViewDist, int nRenderFrameCount)
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
									if (m_removableChunks.empty() || m_removableChunks.back()->GetRenderFrameCount() > pChunk->GetRenderFrameCount() || (nCurrentRenderFrame == pChunk->GetRenderFrameCount() && m_removableChunks.back()->GetChunkViewDistance() < pChunk->GetChunkViewDistance()))
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
			if (pChunk /* && pChunk->IsLightingInitialized()*/)
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
		for (RenderableChunk* pRenderChunk : m_tempDirtyChunks)
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
				if (pRenderChunk->GetChunkViewDistance() > GetNearCameraChunkDist())
				{
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

	IAttributeFields* BlockWorldClient::GetChildAttributeObject(const char * sName)
	{
		std::string strName = sName;
		if (strName == "ChunkVertexBuilderManager")
		{
			return &(ChunkVertexBuilderManager::GetInstance());
		}
		else if (strName == "LightGrid")
		{
			return &(GetLightGrid());
		}
		else if (strName == "CMultiFrameBlockWorldRenderer")
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
#ifdef EMSCRIPTEN_SINGLE_THREAD
		return false;
#else
		return m_bAsyncChunkMode;
#endif
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

	namespace
	{
		void BuildUnitSphereMesh(int slices, int stacks, std::vector<Vector3>& outVerts, std::vector<uint16_t>& outIndices)
		{
			outVerts.clear();
			outIndices.clear();
			outVerts.reserve(static_cast<size_t>((stacks + 1) * (slices + 1)));
			outIndices.reserve(static_cast<size_t>(stacks * slices * 6));
			const float kPi = 3.14159265358979323846f;
			for (int i = 0; i <= stacks; ++i)
			{
				const float phi = kPi * static_cast<float>(i) / static_cast<float>(stacks);
				const float sinPhi = sinf(phi);
				const float cosPhi = cosf(phi);
				for (int j = 0; j <= slices; ++j)
				{
					const float theta = 2.0f * kPi * static_cast<float>(j) / static_cast<float>(slices);
					outVerts.emplace_back(sinPhi * cosf(theta), cosPhi, sinPhi * sinf(theta));
				}
			}
			const int ringStride = slices + 1;
			for (int i = 0; i < stacks; ++i)
			{
				for (int j = 0; j < slices; ++j)
				{
					const uint16_t i0 = static_cast<uint16_t>(i * ringStride + j);
					const uint16_t i1 = static_cast<uint16_t>((i + 1) * ringStride + j);
					const uint16_t i2 = static_cast<uint16_t>((i + 1) * ringStride + j + 1);
					const uint16_t i3 = static_cast<uint16_t>(i * ringStride + j + 1);
					// Emit CW outward-facing triangles in left-handed D3D coordinates (matching D3DXCreateSphere),
					// required by DeferredShadingPointLighting/SpotLighting two-pass stencil light volumes.
					outIndices.push_back(i0);
					outIndices.push_back(i2);
					outIndices.push_back(i1);
					outIndices.push_back(i0);
					outIndices.push_back(i3);
					outIndices.push_back(i2);
				}
			}
		}
	}

	void BlockWorldClient::RenderDeferredLightsMesh()
	{
		// Obsolete legacy debug stub; deferred local lights are rendered in RenderDeferredLighting().
	}

	void BlockWorldClient::RenderDeferredLighting()
	{
		SceneState* sceneState = CGlobals::GetSceneState();
		if (!sceneState || !sceneState->IsDeferredShading() || sceneState->listDeferredLightObjects.empty())
			return;

		auto* pDevice = sceneState->GetRenderDevice();
		if (!pDevice || !m_render_target_block_info || !m_render_target_depth_tex || !m_render_target_normal)
			return;

		IRHIRenderTarget* blockInfoRT = m_render_target_block_info->GetRHIRenderTarget();
		IRHIRenderTarget* depthRT = m_render_target_depth_tex->GetRHIRenderTarget();
		IRHIRenderTarget* normalRT = m_render_target_normal->GetRHIRenderTarget();
		if (!blockInfoRT || !depthRT || !normalRT)
			return;

		CGlobals::GetEffectManager()->EndEffect();

		// Sort deferred lights by light type to minimize shader/declaration switches.
		std::sort(sceneState->listDeferredLightObjects.begin(), sceneState->listDeferredLightObjects.end(), [](CLightObject* a, CLightObject* b) {
			return (a->GetLightType() < b->GetLightType());
		});

		if (!m_effect_light_point)
		{
			m_effect_light_point = CGlobals::GetAssetManager()->LoadEffectFile("m_effect_light_point", ":IDR_FX_DEFERRED_POINT_LIGHTING");
			if (m_effect_light_point)
				m_effect_light_point->LoadAsset();
		}
		if (!m_effect_light_spot)
		{
			m_effect_light_spot = CGlobals::GetAssetManager()->LoadEffectFile("m_effect_light_spot", ":IDR_FX_DEFERRED_SPOT_LIGHTING");
			if (m_effect_light_spot)
				m_effect_light_spot->LoadAsset();
		}
		if (!m_effect_light_directional)
		{
			m_effect_light_directional = CGlobals::GetAssetManager()->LoadEffectFile("m_effect_light_directional", ":IDR_FX_DEFERRED_DIRECTIONAL_LIGHTING");
			if (m_effect_light_directional)
				m_effect_light_directional->LoadAsset();
		}

		const auto prevBinding = pDevice->GetCurrentRenderTargetBinding();
		IRHIRenderTarget* rt0 = prevBinding.targets[0];
		IRHIRenderTarget* depthStencilRT = (rt0 && rt0->GetDepthTexture()) ? rt0 : prevBinding.depthStencilOverride;

		pDevice->SetTexture(0, nullptr);
		pDevice->SetTexture(1, nullptr);
		pDevice->SetTexture(2, nullptr);

		// Bind COLOR0 = scene color RT0 and COLOR1 = _BlockInfoRT while detaching G-buffer depth/normal RTs (slots 2 & 3) so they can be sampled as SRVs.
		IRHIRenderTarget* lightMrtTargets[2] = { rt0, blockInfoRT };
		if (!pDevice->SetRenderTargets(2, lightMrtTargets, depthStencilRT))
			return;

		if (CGlobals::GetViewportManager() && CGlobals::GetViewportManager()->GetActiveViewPort())
			CGlobals::GetViewportManager()->GetActiveViewPort()->ApplyViewport();

		pDevice->SetRenderState(ERenderState::ZENABLE, TRUE);
		pDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pDevice->SetRenderState(ERenderState::SEPARATEALPHABLENDENABLE, FALSE);
		pDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
		pDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);

		static std::vector<Vector3> s_unitSphereVerts;
		static std::vector<uint16_t> s_sphereIndices;
		if (s_unitSphereVerts.empty())
		{
			BuildUnitSphereMesh(28, 18, s_unitSphereVerts, s_sphereIndices);
			// Circumscribe slightly so polygonal facets fully enclose the true attenuation sphere.
			for (Vector3& v : s_unitSphereVerts)
				v *= 1.025f;
		}
		const uint32_t numSphereVerts = static_cast<uint32_t>(s_unitSphereVerts.size());
		const uint32_t spherePrimCount = static_cast<uint32_t>(s_sphereIndices.size() / 3);

		if (!m_lightSphereVB.IsValid() || !m_lightSphereIB.IsValid())
		{
			m_lightSphereVB.ReleaseBuffer();
			m_lightSphereIB.ReleaseBuffer();
			const uint32 vbBytes = numSphereVerts * sizeof(Vector3);
			const uint32 ibBytes = static_cast<uint32>(s_sphereIndices.size() * sizeof(uint16_t));
			if (m_lightSphereVB.CreateBuffer(vbBytes, 0, D3DUSAGE_WRITEONLY) &&
				m_lightSphereIB.CreateBuffer(ibBytes, D3DFMT_INDEX16, D3DUSAGE_WRITEONLY))
			{
				void* pVBData = nullptr;
				if (m_lightSphereVB.Lock(&pVBData, 0, vbBytes))
				{
					std::memcpy(pVBData, s_unitSphereVerts.data(), vbBytes);
					m_lightSphereVB.Unlock();
				}
				void* pIBData = nullptr;
				if (m_lightSphereIB.Lock(&pIBData, 0, ibBytes))
				{
					std::memcpy(pIBData, s_sphereIndices.data(), ibBytes);
					m_lightSphereIB.Unlock();
				}
				m_lightSphereVertCount = numSphereVerts;
				m_lightSpherePrimCount = spherePrimCount;
			}
		}
		const bool bUseGpuSphereBuffers = m_lightSphereVB.IsValid() && m_lightSphereIB.IsValid();

		CCameraFrustum* pFrustum = nullptr;
		if (CGlobals::GetScene() && CGlobals::GetScene()->GetCurrentCamera())
			pFrustum = CGlobals::GetScene()->GetCurrentCamera()->GetFrustum();

		Vector2 vpScale(1.0f, 1.0f);
		Vector2 vpOffset(0.0f, 0.0f);
		Vector2 screenSize(1.0f, 1.0f);
		if (CGlobals::GetViewportManager())
		{
			screenSize = Vector2(
				static_cast<float>((std::max)(1, CGlobals::GetViewportManager()->GetWidth())),
				static_cast<float>((std::max)(1, CGlobals::GetViewportManager()->GetHeight())));
			if (CGlobals::GetViewportManager()->GetActiveViewPort())
				CGlobals::GetViewportManager()->GetActiveViewPort()->GetViewportTransform(&vpScale, &vpOffset);
		}

		const RHIDeviceCaps& caps = pDevice->GetCaps();
		Vector2 pointSpotScreenParam = screenSize;
		Vector2 pointSpotVpScale = vpScale;
		if (!caps.needsHalfPixelOffset)
		{
			// MainVS computes: output.texCoord = (t + 0.5 / screenParam) * viewportScale.
			// On modern APIs (pixel centers already at +0.5), eliminate the D3D9 half-texel bias
			// (and fold any non-zero viewportOffset into 0.5 / screenParam * viewportScale).
			pointSpotScreenParam.x = (vpOffset.x > 1e-6f && vpScale.x > 1e-6f) ? (0.5f * vpScale.x / vpOffset.x) : 1e30f;
			if (!caps.topLeftScreenOrigin)
			{
				// OpenGL FBOs have bottom-left origin while SetViewport maps top-left (Y, Height) to RTHeight - Y - Height:
				// v_gl = (1 - o_y) - t_y * s_y == (t_y + 0.5 / screenParam.y) * (-s_y).
				const float invTop = 1.0f - vpOffset.y;
				pointSpotVpScale.y = -vpScale.y;
				pointSpotScreenParam.y = (invTop > 1e-6f && vpScale.y > 1e-6f) ? (-0.5f * vpScale.y / invTop) : 1e30f;
			}
			else
			{
				pointSpotScreenParam.y = (vpOffset.y > 1e-6f && vpScale.y > 1e-6f) ? (0.5f * vpScale.y / vpOffset.y) : 1e30f;
			}
		}

		CEffectFile* pEffectFile = nullptr;
		VertexDeclarationPtr pDecl = nullptr;
		int nLastType = -1;

		for (CLightObject* lightObject : sceneState->listDeferredLightObjects)
		{
			if (!lightObject)
				continue;
			auto* light_param = lightObject->GetLightParams();
			if (!light_param)
				continue;

			const auto light_type = light_param->Type;
			const auto light_range = light_param->Range;

			Matrix4 mxWorld;
			lightObject->GetRenderMatrix(mxWorld);
			if (light_type == ELightType::Point || light_type == ELightType::Spot)
			{
				if (light_range <= 0.001f)
					continue;
				const Vector3 renderCenter(mxWorld._41, mxWorld._42, mxWorld._43);
				const float cullRadius = light_range * 1.04f;
				CShapeAABB lightBox;
				lightBox.SetMinMax(
					renderCenter - Vector3(cullRadius, cullRadius, cullRadius),
					renderCenter + Vector3(cullRadius, cullRadius, cullRadius));
				if (pFrustum && !pFrustum->TestBox(&lightBox))
					continue;
				if (m_softwareOccluder.IsRenderAABBOccluded(lightBox))
					continue;

				Matrix4 mxScaling;
				mxScaling.makeScale(Vector3(light_range, light_range, light_range));
				mxWorld = mxScaling * mxWorld;
			}

			const auto light_diffuse = light_param->Diffuse;
			const auto light_specular = light_param->Specular;
			const auto light_ambient = light_param->Ambient;
			const auto light_position = light_param->Position;
			const auto light_direction = lightObject->GetDirection();
			const auto light_falloff = light_param->Falloff;
			const auto light_attenuation0 = light_param->Attenuation0;
			const auto light_attenuation1 = light_param->Attenuation1;
			const auto light_attenuation2 = light_param->Attenuation2;
			const auto light_theta = light_param->Theta;
			const auto light_phi = light_param->Phi;

			if (static_cast<int>(light_type) != nLastType)
			{
				nLastType = static_cast<int>(light_type);
				if (pEffectFile)
				{
					pEffectFile->end(true);
					pEffectFile = nullptr;
				}

				if (light_type == ELightType::Point || light_type == ELightType::Spot)
				{
					pEffectFile = (light_type == ELightType::Point) ? m_effect_light_point.get() : m_effect_light_spot.get();
					pDecl = CGlobals::GetEffectManager()->GetVertexDeclaration(EffectManager::S0_POS);
				}
				else if (light_type == ELightType::Directional)
				{
					pEffectFile = m_effect_light_directional.get();
					pDecl = CGlobals::GetEffectManager()->GetVertexDeclaration(EffectManager::S0_POS_TEX0);
				}

				if (!pEffectFile)
					continue;

				pEffectFile->LoadAsset();
				if (!pEffectFile->IsValid())
				{
					pEffectFile = nullptr;
					continue;
				}

				if (pDecl)
					pDevice->SetVertexDeclaration(pDecl);
				if ((light_type == ELightType::Point || light_type == ELightType::Spot) && bUseGpuSphereBuffers)
				{
					pDevice->SetStreamSource(0, m_lightSphereVB.GetDevicePointer(), 0, sizeof(Vector3));
					pDevice->SetIndices(m_lightSphereIB.GetDevicePointer());
				}

				if (!pEffectFile->begin(false, 0, true))
				{
					pEffectFile = nullptr;
					continue;
				}

				auto* params = pEffectFile->GetParamBlock();
				params->SetParamByStringValue("ViewAspect", "floatViewAspect");
				params->SetParamByStringValue("TanHalfFOV", "floatTanHalfFOV");
				params->SetParameter("viewportOffset", vpOffset);
				if (light_type == ELightType::Point || light_type == ELightType::Spot)
				{
					params->SetParameter("screenParam", pointSpotScreenParam);
					params->SetParameter("viewportScale", pointSpotVpScale);
				}
				else
				{
					params->SetParameter("screenParam", screenSize);
					params->SetParameter("viewportScale", vpScale);
				}

				pEffectFile->SetTextureByName("tex0", nullptr);
				pEffectFile->SetTextureByName("tex1", m_render_target_depth_tex.get());
				pEffectFile->SetTextureByName("tex2", m_render_target_normal.get());
				pDevice->SetTexture(0, nullptr);
				pEffectFile->setTexture(1, m_render_target_depth_tex.get());
				pEffectFile->setTexture(2, m_render_target_normal.get());
			}

			if (!pEffectFile)
				continue;

			auto* params = pEffectFile->GetParamBlock();

			CGlobals::GetWorldMatrixStack().push(mxWorld);
			params->SetParamByStringValue("matWorld", "mat4World");
			CGlobals::GetWorldMatrixStack().pop();

			params->SetParamByStringValue("matView", "mat4View");
			params->SetParamByStringValue("matProj", "mat4Projection");

			params->SetParameter("light_diffuse", Vector4(light_diffuse.r, light_diffuse.g, light_diffuse.b, light_diffuse.a));
			params->SetParameter("light_specular", Vector4(light_specular.r, light_specular.g, light_specular.b, light_specular.a));
			params->SetParameter("light_ambient", Vector4(light_ambient.r, light_ambient.g, light_ambient.b, light_ambient.a));

			params->SetParameter("light_position", Vector3(light_position.x, light_position.y, light_position.z));
			params->SetParameter("light_direction", Vector3(light_direction.x, light_direction.y, light_direction.z));

			params->SetParameter("light_range", light_range);
			params->SetParameter("light_falloff", light_falloff);

			params->SetParameter("light_attenuation0", light_attenuation0);
			params->SetParameter("light_attenuation1", light_attenuation1);
			params->SetParameter("light_attenuation2", light_attenuation2);

			params->SetParameter("light_theta", light_theta);
			params->SetParameter("light_phi", light_phi);

			pEffectFile->applyCameraMatrices();
			params->ApplyToEffect(pEffectFile);

			pDevice->SetRenderState(ERenderState::STENCILWRITEMASK, 0xFFFFFFFF);
			pDevice->SetClearStencil(0);
			pDevice->Clear(false, false, true);

			switch (light_type)
			{
			case ELightType::Point:
			case ELightType::Spot:
				for (int pass = 0; pass < 2; ++pass)
				{
					if (pEffectFile->BeginPass(pass, true))
					{
						if (pass == 1)
						{
							pEffectFile->setTexture(1, m_render_target_depth_tex.get());
							pEffectFile->setTexture(2, m_render_target_normal.get());
						}
						if (bUseGpuSphereBuffers)
						{
							pDevice->DrawIndexedPrimitive(EPrimitiveType::TRIANGLELIST, 0, 0, m_lightSphereVertCount, 0, m_lightSpherePrimCount);
						}
						else
						{
							pDevice->DrawIndexedPrimitiveUP(EPrimitiveType::TRIANGLELIST, 0, numSphereVerts, spherePrimCount, s_sphereIndices.data(), PixelFormat::INDEX16, s_unitSphereVerts.data(), sizeof(Vector3));
						}
						pEffectFile->EndPass(true);
					}
				}
				break;
			case ELightType::Directional:
				if (pEffectFile->BeginPass(0, true))
				{
					pEffectFile->setTexture(1, m_render_target_depth_tex.get());
					pEffectFile->setTexture(2, m_render_target_normal.get());
					ParaScripting::CParaEngine::DrawQuad();
					pEffectFile->EndPass(true);
				}
				break;
			default:
				break;
			}
		}

		if (pEffectFile)
		{
			pEffectFile->end(true);
		}

		pDevice->SetTexture(1, nullptr);
		pDevice->SetTexture(2, nullptr);

		pDevice->SetRenderTargets(rt0 ? 1u : 0u, rt0 ? &rt0 : nullptr, depthStencilRT);
		if (CGlobals::GetViewportManager() && CGlobals::GetViewportManager()->GetActiveViewPort())
			CGlobals::GetViewportManager()->GetActiveViewPort()->ApplyViewport();

		pDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		pDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
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
		pClass->AddField("UsePointTextureFiltering", FieldType_Bool, (void*)SetUsePointTextureFiltering_s, (void*)GetUsePointTextureFiltering_s, NULL, NULL, bOverride);
		return S_OK;
	}

} // namespace ParaEngine
