//-----------------------------------------------------------------------------
// Class:	MultiFrameBlockWorldRenderer
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: 
// Date:	2015.11.25
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "RenderTarget.h"
#include "CanvasCamera.h"
#include "ViewportManager.h"
#include "EffectManager.h"
#include "BlockWorldClient.h"
#include "TextureEntity.h"
#include "2dengine/GUIRoot.h"
#include "PaintEngine/Painter.h"
#include "BlockRegion.h"
#include "BlockLightGridBase.h"

#include "MultiFrameBlockWorldRenderer.h"


using namespace ParaEngine;

ParaEngine::CMultiFrameBlockWorldRenderer::CMultiFrameBlockWorldRenderer(BlockWorldClient* pWorld)
	:m_pWorld(pWorld), m_bIsDirty(false), m_bIsFinished(false), m_bEnabled(false), m_nMaxChunksToDrawPerTick(16), m_bDrawToSkybox(true), m_nProgress(0),
	m_chunkViewSize(0), m_nRenderDistance(2048), m_pCamera(NULL), m_bUseMyCamera(false), m_bDebugImage(false), m_nCurRenderPass(0), m_bCanResumeProgress(false)
{
	if (GetIdentifier().empty())
		SetIdentifier("MultiFrameBlockWorldRT");
}

ParaEngine::CMultiFrameBlockWorldRenderer::~CMultiFrameBlockWorldRenderer()
{
	for (int i = 0; i < (int)(m_activeChunks.size()); ++i)
	{
		SAFE_DELETE(m_activeChunks[i]);
	}
	m_activeChunks.clear();

	SAFE_DELETE(m_pCamera);
}

int ParaEngine::CMultiFrameBlockWorldRenderer::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);

	pClass->AddField("Enabled", FieldType_Bool, (void*)SetEnabled_s, (void*)IsEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("Dirty", FieldType_Bool, (void*)SetDirty_s, (void*)IsDirty_s, NULL, NULL, bOverride);
	pClass->AddField("IsFinished", FieldType_Bool, (void*)0, (void*)IsFinished_s, NULL, NULL, bOverride);
	pClass->AddField("DebugImage", FieldType_Bool, (void*)SetDebugImage_s, (void*)IsDebugImage_s, NULL, NULL, bOverride);
	pClass->AddField("UseMyCamera", FieldType_Bool, (void*)SetUseMyCamera_s, (void*)IsUseMyCamera_s, NULL, NULL, bOverride);
	pClass->AddField("MaxChunksToDrawPerTick", FieldType_Int, (void*)SetMaxChunksToDrawPerTick_s, (void*)GetMaxChunksToDrawPerTick_s, NULL, NULL, bOverride);
	pClass->AddField("RenderDistance", FieldType_Int, (void*)SetRenderDistance_s, (void*)GetRenderDistance_s, NULL, NULL, bOverride);

	return S_OK;
}

bool ParaEngine::CMultiFrameBlockWorldRenderer::Draw(int nMaxChunks)
{
	if (!m_bEnabled || !m_pWorld->IsInBlockWorld())
		return false;

	if (CGlobals::GetScene()->GetSceneState()->m_bCameraMoved)
	{
		SetDirty(true);
	}
	if (IsFinished())
		return true;

	CRenderTarget* pRenderTarget = CreateGetRenderTarget();
	if (pRenderTarget)
	{
		if (!CGlobals::GetViewportManager()->GetActiveViewPort()->GetRenderTarget())
		{
			ParaViewport viewport;
			CGlobals::GetViewportManager()->GetCurrentViewport(viewport);
			//  shall use the same buffer size as the current selected viewport
			pRenderTarget->SetRenderTargetSize(viewport.Width, viewport.Height);

			if (pRenderTarget->GetPrimaryAsset())
			{
				if (pRenderTarget->Begin())
				{
					if (nMaxChunks <= 0)
						nMaxChunks = GetMaxChunksToDrawPerTick();
					DrawInternal(nMaxChunks);

					pRenderTarget->End();
					return true;
				}
			}
		}
	}
	CGlobals::GetEffectManager()->EndEffect();

	return m_bIsFinished;
}

bool ParaEngine::CMultiFrameBlockWorldRenderer::IsEnabled() const
{
	return m_bEnabled;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::SetEnabled(bool val)
{
	m_bEnabled = val;
}

bool ParaEngine::CMultiFrameBlockWorldRenderer::IsUseMyCamera() const
{
	return m_bUseMyCamera;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::SetUseMyCamera(bool val)
{
	m_bUseMyCamera = val;
}

int ParaEngine::CMultiFrameBlockWorldRenderer::GetMaxChunksToDrawPerTick() const
{
	return m_nMaxChunksToDrawPerTick;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::SetMaxChunksToDrawPerTick(int val)
{
	m_nMaxChunksToDrawPerTick = val;
}


void ParaEngine::CMultiFrameBlockWorldRenderer::CheckCreateActiveChunks()
{
	if ((int)m_activeChunks.size() < m_nMaxChunksToDrawPerTick)
	{
		m_activeChunks.resize(m_nMaxChunksToDrawPerTick, NULL);
		for (int i = 0; i < (int)(m_activeChunks.size()); ++i)
		{
			if (m_activeChunks[i] == NULL)
			{
				m_activeChunks[i] = new RenderableChunk();
				// force using this shader format, since we will only render with fast shader. 
				m_activeChunks[i]->SetShaderID(BLOCK_RENDER_FAST_SHADER);
				// this is tricky, this will prevent clearing the chunk dirty mask when chunk buffer finished rebuilt. 
				m_activeChunks[i]->SetIsMainRenderer(false);
			}
		}
	}
}

bool ParaEngine::CMultiFrameBlockWorldRenderer::IsDrawToSkybox() const
{
	return m_bDrawToSkybox;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::SetDrawToSkybox(bool val)
{
	m_bDrawToSkybox = val;
}

bool ParaEngine::CMultiFrameBlockWorldRenderer::IsDirty() const
{
	return m_bIsDirty;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::SetDirty(bool val)
{
	if (m_bIsDirty != val)
	{
		m_bIsDirty = val;
		if (m_bIsDirty) {
			m_nProgress = 0;
			m_nCurRenderPass = 0;
			m_bIsFinished = false;

			m_tempRenderTasks.clear();
			m_alphaTestTasks.clear();
			m_alphaBlendTasks.clear();
			m_reflectedWaterTasks.clear();
		}
	}
}

bool ParaEngine::CMultiFrameBlockWorldRenderer::IsFinished() const
{
	return m_bIsFinished;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::SetFinished(bool val)
{
	m_bIsFinished = val;
}

int ParaEngine::CMultiFrameBlockWorldRenderer::GetRenderDistance() const
{
	return m_nRenderDistance;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::SetRenderDistance(int val)
{
	m_nRenderDistance = val;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::DeleteDeviceObjects()
{
	for (uint32_t i = 0; i < m_activeChunks.size(); i++)
		m_activeChunks[i]->DeleteDeviceObjects();
}

void ParaEngine::CMultiFrameBlockWorldRenderer::RendererRecreated()
{
	for (uint32_t i = 0; i < m_activeChunks.size(); i++)
		m_activeChunks[i]->RendererRecreated();
}

CBaseCamera* ParaEngine::CMultiFrameBlockWorldRenderer::GetCamera()
{
	if (m_pCamera == NULL)
		m_pCamera = new CBaseCamera();
	return m_pCamera;
}

CRenderTarget* ParaEngine::CMultiFrameBlockWorldRenderer::CreateGetRenderTarget(bool bCreateIfNotExist /*= true*/)
{
	if (m_renderTarget)
		return (CRenderTarget*)(m_renderTarget.get());
	else if (bCreateIfNotExist)
	{
		CRenderTarget* pRenderTarget = static_cast<CRenderTarget*>(CGlobals::GetScene()->FindObjectByNameAndType(GetIdentifier(), "CRenderTarget"));
		if (pRenderTarget)
		{
			pRenderTarget->SetLifeTime(-1);
			m_renderTarget = pRenderTarget->GetWeakReference();
			return pRenderTarget;
		}
		else
		{
			// create one if not exist. 
			CRenderTarget* pRenderTarget = new CRenderTarget();
			pRenderTarget->SetIdentifier(GetIdentifier());
			CGlobals::GetScene()->AttachObject(pRenderTarget);
			pRenderTarget->SetDirty(false);
			pRenderTarget->SetVisibility(false);
			m_renderTarget = pRenderTarget->GetWeakReference();
			return pRenderTarget;
		}
	}
	return NULL;
}

bool ParaEngine::CMultiFrameBlockWorldRenderer::DrawToSkybox()
{
	if (!IsDrawToSkybox() || !m_pWorld->IsInBlockWorld() || !IsEnabled() || GetRenderDistance() == 0)
		return false;

	CRenderTarget* pRenderTarget = CreateGetRenderTarget(false);
	if (!pRenderTarget)
		return false;

	auto painter = CGlobals::GetGUI()->GetPainter();
	painter->SetUse3DTransform(false);

	if (painter->begin(CGlobals::GetGUI()))
	{
		RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();
		pRenderDevice->SetRenderState(ERenderState::ZENABLE, TRUE);
		pRenderDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);

		painter->setCompositionMode(CPainter::CompositionMode_SourceBlend);
		painter->setPen(Color::White);
		if (IsDebugImage())
		{
			// only for debugging the render target
			painter->drawTexture(0, 0, 320, 240, pRenderTarget->GetTexture(), 0.f);
		}
		ParaViewport viewport;
		auto viewportManager = CGlobals::GetViewportManager();
		viewportManager->GetCurrentViewport(viewport);
		painter->drawTexture(QRectF((float)viewport.X / viewportManager->GetWidth(), (float)viewport.Y / viewportManager->GetHeight(),
			painter->FromUnitSpaceX((float)viewport.Width / viewportManager->GetWidth()), painter->FromUnitSpaceY((float)viewport.Height / viewportManager->GetHeight())), pRenderTarget->GetTexture(), QRectF(), 1.f);

		painter->Flush();
		pRenderDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		painter->end();
	}
	return true;
}


bool ParaEngine::CMultiFrameBlockWorldRenderer::IsDebugImage() const
{
	return m_bDebugImage;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::SetDebugImage(bool val)
{
	m_bDebugImage = val;
}

bool ParaEngine::CMultiFrameBlockWorldRenderer::GetChunkByProgress(int nProgress, Int16x3& outChunkPos, bool bIsResuming)
{
	if (nProgress == 1)
	{
		outChunkPos.x = GetEyeChunkId().x;
		outChunkPos.y = 0;
		outChunkPos.z = GetEyeChunkId().z;
		return true;
	}
	else
	{
		// we will add using a spiral rectangle path from the center.

		int32 chunkX = GetEyeChunkId().x;
		int32 chunkZ = GetEyeChunkId().z;
		Vector3 vChunkColumnSize(BlockConfig::g_chunkSize, BlockConfig::g_chunkSize * BlockConfig::g_regionChunkDimY, BlockConfig::g_chunkSize);
		if (!m_bCanResumeProgress)
			bIsResuming = false;
		if (!bIsResuming)
			ResetProgress();

		for (m_length = bIsResuming ? m_length : 1; m_length <= m_chunkViewSize; ++m_length)
		{
			for (m_k = bIsResuming ? m_k : 1; m_k <= 2; ++m_k)
			{
				int32* dir = &(BlockCommon::g_xzDirectionsConst[(m_nIndex % 4)][0]);

				for (m_i = bIsResuming ? m_i : 0; m_i < m_length; ++m_i)
				{
					if (!bIsResuming)
					{
						m_dx = m_dx + dir[0];
						m_dz = m_dz + dir[1];
					}

					uint16 x = (uint16)(chunkX + m_dx);
					uint16 z = (uint16)(chunkZ + m_dz);
					if (x >= m_startIdx.x && x <= m_endIdx.x && z >= m_startIdx.z && z <= m_endIdx.z)
					{
						Vector3 vMin = BlockCommon::ConvertToRealPosition(x * 16, 0, z * 16, 7);
						vMin -= m_renderOrig;
						Vector3 vMax = vMin + vChunkColumnSize;

						CShapeAABB box;
						box.SetMinMax(vMin, vMax);
						if (m_frustum.TestBoxInsideFarPlane(&box))
						{
							if (!bIsResuming)
								m_nProgressIndex++;
							if (m_nProgressIndex > nProgress)
							{
								outChunkPos.x = x;
								outChunkPos.y = 0;
								outChunkPos.z = z;
								m_bCanResumeProgress = true;
								return true;
							}
						}
					}
					bIsResuming = false;
				}
				++m_nIndex;
			}
		}
	}
	m_bCanResumeProgress = true;
	return false;
}


void ParaEngine::CMultiFrameBlockWorldRenderer::ResetProgress()
{
	m_nIndex = 0;
	m_dx = 0;
	m_dz = 0;
	m_length = 1;
	m_k = 1;
	m_i = 0;
	m_nProgressIndex = 2;
	m_bCanResumeProgress = false;
}

const Uint16x3& ParaEngine::CMultiFrameBlockWorldRenderer::GetEyeBlockId()
{
	return m_curCamBlockId;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::UpdateViewParams()
{
	Vector3 camWorldPos = CGlobals::GetSceneState()->vEye;
	m_renderOrig = CGlobals::GetScene()->GetRenderOrigin();

	{
		// update camera from current camera settings
		CBaseCamera* pFromCamera = CGlobals::GetScene()->GetCurrentCamera();
		CBaseCamera* pCamera = GetCamera();
		pCamera->CopyCameraParamsFrom(pFromCamera);
		pCamera->SetFarPlane(GetRenderDistance() * BlockConfig::g_blockSize);
		pCamera->UpdateProjParams();

		CGlobals::GetProjectionMatrixStack().push(*pCamera->GetProjMatrix());
		CGlobals::GetViewMatrixStack().push(*pCamera->GetViewMatrix());
		pCamera->UpdateFrustum();
		CGlobals::GetProjectionMatrixStack().pop();
		CGlobals::GetViewMatrixStack().pop();
	}

	Vector3 camMin;
	Vector3 camMax;
	m_frustum = *(GetCamera()->GetFrustum());
	CCameraFrustum* frustum = &m_frustum;

	// calculate aabb for frustum
	Vector3* frusCorner = frustum->vecFrustum;
	camMin = frusCorner[0];
	camMax = frusCorner[0];

	for (int i = 1; i < 8; i++)
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

	camMax += m_renderOrig;
	camMin += m_renderOrig;

	Uint16x3 camBlockPos;
	BlockCommon::ConvertToBlockIndex(camWorldPos.x, camWorldPos.y, camWorldPos.z, camBlockPos.x, camBlockPos.y, camBlockPos.z);
	SetEyeBlockId(camBlockPos);

	BlockCommon::ConvertToBlockIndex(camMin.x, camMin.y, camMin.z, m_startIdx.x, m_startIdx.y, m_startIdx.z);
	m_startIdx.x /= 16; m_startIdx.y /= 16; m_startIdx.z /= 16;
	m_startIdx.x = m_startIdx.x > 0 ? m_startIdx.x - 1 : 0;


	m_startIdx.y = m_startIdx.y > 0 ? (m_startIdx.y <= 15 ? m_startIdx.y - 1 : 15) : 0;
	m_startIdx.z = m_startIdx.z > 0 ? m_startIdx.z - 1 : 0;
	BlockCommon::ConvertToBlockIndex(camMax.x, camMax.y, camMax.z, m_endIdx.x, m_endIdx.y, m_endIdx.z);
	m_endIdx.x /= 16; m_endIdx.y /= 16; m_endIdx.z /= 16;
	m_endIdx.y = m_endIdx.y < 15 ? m_endIdx.y + 1 : 15;

	float fRenderDist = GetRenderDistance() * BlockConfig::g_blockSize;
	m_sEyeSphere = CShapeSphere(camWorldPos, fRenderDist);

	int32 chunkX = GetEyeChunkId().x;
	int32 chunkY = GetEyeChunkId().y;
	int32 chunkZ = GetEyeChunkId().z;
	int32 chunkViewRadius = (int32)(GetRenderDistance() / 16);
	m_chunkViewSize = chunkViewRadius * 2;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::SetEyeBlockId(const Uint16x3& eyePos)
{
	m_curCamBlockId = eyePos;
	m_curCamChunkId.x = m_curCamBlockId.x / 16;
	m_curCamChunkId.y = m_curCamBlockId.y / 16;
	m_curCamChunkId.z = m_curCamBlockId.z / 16;
}

const Uint16x3& ParaEngine::CMultiFrameBlockWorldRenderer::GetEyeChunkId()
{
	return m_curCamChunkId;
}

void ParaEngine::CMultiFrameBlockWorldRenderer::DrawRenderTasks(BlockRenderPass nCurRenderPass)
{
	sort(m_tempRenderTasks.begin(), m_tempRenderTasks.end(), BlockWorldClient::CompareRenderOrder);
	CGlobals::GetWorldMatrixStack().push(Matrix4::IDENTITY);
	CGlobals::GetProjectionMatrixStack().push(*GetCamera()->GetProjMatrix());
	CGlobals::GetViewMatrixStack().push(*GetCamera()->GetViewMatrix());

	CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true, true, true);

	// deferred shading are supported when generating multi-frame block world.
	// hence we need to force using BLOCK_RENDER_FAST_SHADER
	m_pWorld->Render(nCurRenderPass, &m_tempRenderTasks, BLOCK_RENDER_FAST_SHADER);

	CGlobals::GetWorldMatrixStack().pop();
	CGlobals::GetProjectionMatrixStack().pop();
	CGlobals::GetViewMatrixStack().pop();
	CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true, true, true);
	m_tempRenderTasks.clear();
}

RenderableChunk* ParaEngine::CMultiFrameBlockWorldRenderer::GetRenderableChunkByPosition(const Int16x3& chunkPos, int nBufferIndex, bool* pbFromMainBuffer)
{
	RenderableChunk* pChunk = m_pWorld->GetRenderableChunk(chunkPos);
	if (pChunk && !(pChunk->IsDirty()))
	{
		if (pbFromMainBuffer)
			*pbFromMainBuffer = true;
	}
	else
	{
		pChunk = NULL;
		int16_t regionX = chunkPos.x >> 5;
		int16_t regionZ = chunkPos.z >> 5;
		BlockRegion* pRegion = m_pWorld->GetRegion(regionX, regionZ);

		if (pRegion && !(pRegion->IsLocked()))
		{
			int16_t localChunkX = chunkPos.x & 0x1f;
			int16_t localChunkY = chunkPos.y & 0xf;
			int16_t localChunkZ = chunkPos.z & 0x1f;
			uint16_t chunkIndex = PackChunkIndex(localChunkX, localChunkY, localChunkZ);
			pChunk = m_activeChunks[nBufferIndex];
			pChunk->ReuseChunk(pRegion, chunkIndex);
		}

		if (pbFromMainBuffer)
			*pbFromMainBuffer = false;
	}

	return pChunk;
}

bool ParaEngine::CMultiFrameBlockWorldRenderer::DrawInternal(int nMaxChunks)
{
	CRenderTarget* pRenderTarget = CreateGetRenderTarget(false);
	if (!pRenderTarget || !m_pWorld->IsInBlockWorld())
		return false;
	if (!IsDirty() && IsFinished())
		return true;

	if (IsDirty())
	{
		SetDirty(false);
	}
	m_bIsFinished = false;

	if (m_nProgress == 0 && m_nCurRenderPass == BlockRenderPass_Opaque)
	{
		pRenderTarget->Clear(LinearColor(0, 0, 0, 0));
		UpdateViewParams();
		m_bCanResumeProgress = false;
		m_nProgress++;
		return true;
	}

	// since light is calculated asynchronously, we will wait until previous one is finished before we move on to the next. 
	if (m_pWorld->GetLightGrid().GetForcedChunkColumnCount() > 0)
		return false;

	CheckCreateActiveChunks();

	Int16x3 chunkPos;
	bool bFinished = false;

	if (m_nCurRenderPass == BlockRenderPass_Opaque)
	{
		int nLastProgress = m_nProgress;
		while (!(bFinished = !GetChunkByProgress(m_nProgress, chunkPos, m_bCanResumeProgress)))
		{
			// check if we also have lighting properly calculated. 
			int nLightingResult = m_pWorld->GetLightGrid().ForceAddChunkColumn(chunkPos.x, chunkPos.z);
			if (nLightingResult == 0)
			{
				m_nProgress++;
				m_tempRenderTasks.clear();
				for (int i = 0, j = 0; i < BlockConfig::g_regionChunkDimY && j < (int)m_activeChunks.size(); ++i)
				{
					chunkPos.y = (chunkPos.y & 0xfff0) + i;

					bool bIsFromMainBuffer = false;
					RenderableChunk* pChunk = GetRenderableChunkByPosition(chunkPos, j, &bIsFromMainBuffer);

					if (pChunk && pChunk->IsIntersect(m_sEyeSphere))
					{
						CShapeAABB box(pChunk->GetShapeAABB());
						box.GetCenter() -= m_renderOrig;
						if (m_frustum.TestBoxInsideFarPlane(&box))
						{
							if (!bIsFromMainBuffer)
								pChunk->RebuildRenderBuffer(m_pWorld, false);

							auto tasks = pChunk->GetRenderTasks();
							if (!tasks.empty())
							{
								for (BlockRenderTask* task : tasks)
								{
									auto dwRenderPass = task->GetTemplate()->GetRenderPass();
									if (dwRenderPass == BlockRenderPass_Opaque)
										m_tempRenderTasks.push_back(task);
									else if (dwRenderPass == BlockRenderPass_AlphaTest)
										m_alphaTestTasks.push_back(chunkPos);
									else if (dwRenderPass == BlockRenderPass_AlphaBlended)
										m_alphaBlendTasks.push_back(chunkPos);
									else if (dwRenderPass == BlockRenderPass_ReflectedWater)
										m_reflectedWaterTasks.push_back(chunkPos);
								}
								if (!bIsFromMainBuffer)
									j++;
							}
						}
					}
				}
				if (!m_tempRenderTasks.empty())
				{
					DrawRenderTasks(BlockRenderPass_Opaque);
					break;
				}
			}
			else if (nLightingResult == -1)
			{
				// nearby terrain not loaded, we will escape this. 
				m_nProgress++;
				// too many chunk columns processed, we will escape after 100 empty chunks anyway
				if ((m_nProgress - nLastProgress) > 100)
					break;
			}
			else
				break;
		}
		if (bFinished)
		{
			m_nCurRenderPass = BlockRenderPass_AlphaTest;
			bFinished = false;
			m_nProgress = 0;
		}
	}
	else
	{
		const BlockRenderPass postRenderPasses[] = { BlockRenderPass_AlphaTest, BlockRenderPass_AlphaBlended, BlockRenderPass_ReflectedWater };
		const int nMaxPass = sizeof(postRenderPasses) / sizeof(BlockRenderPass);
		for (int nPass = 0; nPass < nMaxPass; nPass++)
		{
			BlockRenderPass nCurRenderPass = postRenderPasses[nPass];
			if (nCurRenderPass == m_nCurRenderPass)
			{
				std::vector<Int16x3>& taskPositions = (m_nCurRenderPass == BlockRenderPass_AlphaTest) ? m_alphaTestTasks :
					((m_nCurRenderPass == BlockRenderPass_AlphaBlended) ? m_alphaBlendTasks : m_reflectedWaterTasks);

				m_tempRenderTasks.clear();
				for (int i = m_nProgress, j = 0; i < (int)taskPositions.size() && j < (int)m_activeChunks.size(); ++i, ++m_nProgress)
				{
					const Int16x3& chunkPos = taskPositions[i];
					bool bIsFromMainBuffer = false;
					RenderableChunk* pChunk = GetRenderableChunkByPosition(chunkPos, j, &bIsFromMainBuffer);

					if (pChunk)
					{
						if (!bIsFromMainBuffer)
							pChunk->RebuildRenderBuffer(m_pWorld, false);

						auto tasks = pChunk->GetRenderTasks();
						if (!tasks.empty())
						{
							for (BlockRenderTask* task : tasks)
							{
								auto dwRenderPass = task->GetTemplate()->GetRenderPass();
								if (dwRenderPass == nCurRenderPass)
									m_tempRenderTasks.push_back(task);
							}
							if (!bIsFromMainBuffer)
								j++;
						}
					}
				}
				if (!m_tempRenderTasks.empty())
				{
					DrawRenderTasks(nCurRenderPass);
				}
				bFinished = m_nProgress == (int)taskPositions.size();
				if (bFinished)
				{
					if ((nMaxPass - 1) == nPass)
					{
						// the last pass
					}
					else
					{
						// continue next render pass
						m_nCurRenderPass = postRenderPasses[nPass + 1];
						bFinished = false;
					}
					m_nProgress = 0;
				}
				break;
			}
		}
	}

	m_tempRenderTasks.clear();
	if (bFinished)
		SetFinished(true);
	return true;
}
