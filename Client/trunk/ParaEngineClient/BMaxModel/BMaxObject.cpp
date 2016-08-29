//-----------------------------------------------------------------------------
// Class:	
// Authors:	Leio, LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2015.5.19
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BMaxObject.h"
#include "ParaWorldAsset.h"
#include "SceneState.h"
#include "EffectManager.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockModel.h"
#include "ParaXModel/ParaXModel.h"
#include "ShapeOBB.h"
#include "ShapeAABB.h"
#include "IScene.h"
#include <vector>
#include "ParaXModel/ParaXModelExporter.h"
namespace ParaEngine
{
	BMaxObject::BMaxObject()
		:m_fScale(1.0f), m_fLastBlockLight(0.f), m_dwLastBlockHash(0)
	{
			
	}

	BMaxObject::~BMaxObject()
	{
		
	}

	float BMaxObject::GetScaling()
	{
		return m_fScale;
	}


	void BMaxObject::SetScaling(float fScale)
	{
		if (m_fScale != fScale)
		{
			m_fScale = fScale;
			SetGeometryDirty(true);
		}
	}

	void BMaxObject::SetAssetFileName(const std::string& sFilename)
	{
		auto pNewModel = CGlobals::GetAssetManager()->LoadParaX("", sFilename);
		if (m_pAnimatedMesh != pNewModel)
		{
			m_pAnimatedMesh = pNewModel;
			m_CurrentAnim.MakeInvalid();
			SetGeometryDirty(true);
		}
	}

	Matrix4* BMaxObject::GetAttachmentMatrix(Matrix4& matOut, int nAttachmentID /*= 0*/, int nRenderNumber /*= 0*/)
	{
		if (m_pAnimatedMesh && m_pAnimatedMesh->IsLoaded())
		{
			CParaXModel* pModel = m_pAnimatedMesh->GetModel();
			if (pModel)
			{
				Matrix4* pOut = &matOut;
				if (pModel->GetAttachmentMatrix(pOut, nAttachmentID, m_CurrentAnim, AnimIndex(), 0.f))
				{
					Matrix4 matScale;
					float fScaling = GetScaling();
					if (fabs(fScaling - 1.0f) > FLT_TOLERANCE)
					{
						ParaMatrixScaling(&matScale, fScaling, fScaling, fScaling);
						(*pOut) = (*pOut)*matScale;
					}
					return pOut;
				}
			}
		}
		return NULL;
	}

	AssetEntity* BMaxObject::GetPrimaryAsset()
	{
		return m_pAnimatedMesh.get();
	}

	void BMaxObject::UpdateGeometry()
	{
		SetGeometryDirty(false);
		if (m_pAnimatedMesh && m_pAnimatedMesh->IsLoaded())
		{
			Vector3 vMin, vMax;
			if (m_pAnimatedMesh->GetBoundingBox(&vMin, &vMax))
			{
				Matrix4 mat;
				GetLocalTransform(&mat);
				CShapeOBB obb(CShapeBox(vMin, vMax), mat);
				CShapeBox minmaxBox;
				minmaxBox.Extend(obb);
				SetAABB(&minmaxBox.GetMin(), &minmaxBox.GetMax());
			}
		}
	}

	void BMaxObject::ApplyBlockLighting(SceneState * sceneState)
	{
		BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
		if (pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
		{
			uint8_t brightness[2];
			Uint16x3 blockId_ws(0, 0, 0);
			Vector3 vPos = GetPosition();
			BlockCommon::ConvertToBlockIndex(vPos.x, vPos.y + 0.1f, vPos.z, blockId_ws.x, blockId_ws.y, blockId_ws.z);
			float fLightness;

			pBlockWorldClient->GetBlockMeshBrightness(blockId_ws, brightness);
			// block light
			float fBlockLightness = Math::Max(pBlockWorldClient->GetLightBrightnessFloat(brightness[0]), 0.1f);
			sceneState->GetCurrentLightStrength().y = fBlockLightness;
			// sun light
			fLightness = Math::Max(pBlockWorldClient->GetLightBrightnessFloat(brightness[1]), 0.1f);
			sceneState->GetCurrentLightStrength().x = fLightness;
			fLightness *= pBlockWorldClient->GetSunIntensity();
			fLightness = Math::Max(fLightness, fBlockLightness);

			sceneState->GetLocalMaterial().Ambient = LinearColor(fLightness*0.7f, fLightness*0.7f, fLightness*0.7f, 1.f);
			sceneState->GetLocalMaterial().Diffuse = LinearColor(fLightness*0.35f, fLightness*0.35f, fLightness*0.35f, 1.f);

			sceneState->EnableLocalMaterial(true);
		}
	}

	void BMaxObject::GetLocalTransform(Matrix4* localTransform)
	{
		if (localTransform)
		{
			if (GetFacing() != 0)
			{
				ParaMatrixRotationY(localTransform, GetFacing());
			}
			else
				*localTransform = Matrix4::IDENTITY;
		}
	}

	HRESULT BMaxObject::Draw(SceneState * sceneState)
	{
		if (!m_pAnimatedMesh)
			return E_FAIL;
		if (GetPrimaryTechniqueHandle() < 0)
		{
			// try loading the asset if it has not been done before. 
			m_pAnimatedMesh->LoadAsset();
			if (m_pAnimatedMesh->IsLoaded())
			{
				SetPrimaryTechniqueHandle(m_pAnimatedMesh->GetPrimaryTechniqueHandle());
				UpdateGeometry();
			}
			return E_FAIL;
		}

		if (!CGlobals::GetEffectManager()->IsCurrentEffectValid())
		{
			return E_FAIL;
		}

		CParaXModel* pModel = m_pAnimatedMesh->GetModel(0);
		if (pModel == NULL)
			return E_FAIL;

		sceneState->SetCurrentSceneObject(this);
		SetFrameNumber(sceneState->m_nRenderCount);
		// get world transform matrix
		Matrix4 mxWorld;
		GetRenderWorldMatrix(&mxWorld);


		RenderDevicePtr pd3dDevice = sceneState->m_pd3dDevice;
		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->applyObjectLocalLighting(this);

		CEffectFile* pEffectFile = pEffectManager->GetCurrentEffectFile();
		CGlobals::GetWorldMatrixStack().push(mxWorld);

		ApplyBlockLighting(sceneState);

		
		if (pEffectFile == 0)
		{
			// TODO: Fixed Function. 
		}
		else
		{
			// apply block space lighting for object whose size is comparable to a single block size
			if (CheckAttribute(MESH_USE_LIGHT) || sceneState->IsDeferredShading())
			{
				BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
				if (pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
				{
					Vector3 vPos = GetPosition();
					vPos.y += 0.1f;
					Uint16x3 blockId_ws(0, 0, 0);
					BlockCommon::ConvertToBlockIndex(vPos.x, vPos.y, vPos.z, blockId_ws.x, blockId_ws.y, blockId_ws.z);
					DWORD dwPositionHash = blockId_ws.GetHashCode();
					uint8_t brightness[2];
					pBlockWorldClient->GetBlockMeshBrightness(blockId_ws, brightness, 2);
					// block light
					float fBlockLightness = Math::Max(pBlockWorldClient->GetLightBrightnessLinearFloat(brightness[0]), 0.1f);
					sceneState->GetCurrentLightStrength().y = fBlockLightness;
					// sun light
					float fSunLightness = Math::Max(pBlockWorldClient->GetLightBrightnessLinearFloat(brightness[1]), 0.1f);
					sceneState->GetCurrentLightStrength().x = fSunLightness;

					float fLightness = Math::Max(fBlockLightness, fSunLightness*pBlockWorldClient->GetSunIntensity());
					if (m_fLastBlockLight != fLightness)
					{
						float fMaxStep = (float)(sceneState->dTimeDelta*0.5f);
						if (dwPositionHash == m_dwLastBlockHash || m_dwLastBlockHash == 0)
							m_fLastBlockLight = fLightness;
						else
							Math::SmoothMoveFloat1(m_fLastBlockLight, fLightness, fMaxStep);

						fLightness = m_fLastBlockLight;
					}
					else
					{
						m_dwLastBlockHash = dwPositionHash;
					}
					sceneState->GetLocalMaterial().Ambient = (LinearColor(fLightness*0.7f, fLightness*0.7f, fLightness*0.7f, 1.f));
					sceneState->GetLocalMaterial().Diffuse = (LinearColor(fLightness*0.4f, fLightness*0.4f, fLightness*0.4f, 1.f));

					sceneState->EnableLocalMaterial(true);
				}
				// Note: do this if one wants point light
				pEffectManager->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				pEffectManager->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			}
			else
			{
				pEffectManager->SetSamplerState(0, D3DSAMP_MINFILTER, pEffectManager->GetDefaultSamplerState(0, D3DSAMP_MINFILTER));
				pEffectManager->SetSamplerState(0, D3DSAMP_MAGFILTER, pEffectManager->GetDefaultSamplerState(0, D3DSAMP_MAGFILTER));
			}

			// just a single standing animation is supported now and looped. 
			if (!m_CurrentAnim.IsValid())
				m_CurrentAnim = pModel->GetAnimIndexByID(0);
			if (m_CurrentAnim.IsValid())
			{
				int nAnimLength = std::max(1, m_CurrentAnim.nEndFrame - m_CurrentAnim.nStartFrame);
				int nToDoFrame = (m_CurrentAnim.nCurrentFrame + (int)(sceneState->dTimeDelta * 1000)) % nAnimLength;
				m_CurrentAnim.nCurrentFrame = nToDoFrame;
			}
			pModel->m_CurrentAnim = m_CurrentAnim;
			pModel->m_NextAnim.nIndex = 0;
			pModel->m_BlendingAnim.MakeInvalid();
			pModel->blendingFactor = 0;
			pModel->animate(sceneState, NULL);
			// force CParaXModel::BMAX_MODEL? 
			pModel->draw(sceneState, NULL); 
		}

		CGlobals::GetWorldMatrixStack().pop();
		return S_OK;
	}



	int BMaxObject::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		CTileObject::InstallFields(pClass, bOverride);

		return S_OK;
	}
}