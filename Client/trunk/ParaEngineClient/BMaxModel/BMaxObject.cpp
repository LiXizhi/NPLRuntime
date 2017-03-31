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
#include "PhysicsWorld.h"
#include "ShapeOBB.h"
#include "ShapeAABB.h"
#include "IScene.h"
#include <vector>
#include "ParaXModel/ParaXModelExporter.h"
namespace ParaEngine
{
	BMaxObject::BMaxObject()
		:m_fScale(1.0f), m_fLastBlockLight(0.f), m_dwLastBlockHash(0), 
		m_nPhysicsGroup(0), m_dwPhysicsMethod(PHYSICS_FORCE_NO_PHYSICS)
	{
		SetAttribute(OBJ_VOLUMN_FREESPACE);
	}

	BMaxObject::~BMaxObject()
	{
		UnloadPhysics();
	}

	float BMaxObject::GetScaling()
	{
		return m_fScale;
	}

	Matrix4* BMaxObject::GetRenderMatrix(Matrix4& mxWorld, int nRenderNumber /*= 0*/)
	{
		mxWorld.identity();

		// order of rotation: roll * pitch * yaw , where roll is applied first. 
		bool bIsIdentity = true;

		float fScaling = GetScaling();
		if (fScaling != 1.f)
		{
			Matrix4 matScale;
			ParaMatrixScaling((Matrix4*)&matScale, fScaling, fScaling, fScaling);
			mxWorld = (bIsIdentity) ? matScale : matScale.Multiply4x3(mxWorld);
			bIsIdentity = false;
		}

		float fYaw = GetYaw();
		if (fYaw != 0.f)
		{
			Matrix4 matYaw;
			ParaMatrixRotationY((Matrix4*)&matYaw, fYaw);
			mxWorld = (bIsIdentity) ? matYaw : matYaw.Multiply4x3(mxWorld);
			bIsIdentity = false;
		}

		if (GetPitch() != 0.f)
		{
			Matrix4 matPitch;
			ParaMatrixRotationX(&matPitch, GetPitch());
			mxWorld = (bIsIdentity) ? matPitch : matPitch.Multiply4x3(mxWorld);
			bIsIdentity = false;
		}

		if (GetRoll() != 0.f)
		{
			Matrix4 matRoll;
			ParaMatrixRotationZ(&matRoll, GetRoll());
			mxWorld = (bIsIdentity) ? matRoll : matRoll.Multiply4x3(mxWorld);
			bIsIdentity = false;
		}

		// world translation
		Vector3 vPos = GetRenderOffset();
		mxWorld._41 += vPos.x;
		mxWorld._42 += vPos.y;
		mxWorld._43 += vPos.z;

		return &mxWorld;
	}

	bool BMaxObject::CanHasPhysics()
	{
		return IsPhysicsEnabled();
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
			UnloadPhysics();
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
				if (GetScaling()!= 1.0){
					minmaxBox.SetMinMax(minmaxBox.GetMin() * GetScaling(), minmaxBox.GetMax() * GetScaling());
				}
				SetAABB(&minmaxBox.GetMin(), &minmaxBox.GetMax());
			}

			UnloadPhysics();
			if (m_dwPhysicsMethod == 0)
				m_dwPhysicsMethod = PHYSICS_LAZY_LOAD;
			else if (IsPhysicsEnabled() && ((m_dwPhysicsMethod&PHYSICS_ALWAYS_LOAD)>0))
			{
				LoadPhysics();
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
		GetRenderMatrix(mxWorld);


		RenderDevicePtr pd3dDevice = sceneState->m_pd3dDevice;
		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->applyObjectLocalLighting(this);

		CEffectFile* pEffectFile = pEffectManager->GetCurrentEffectFile();
		CGlobals::GetWorldMatrixStack().push(mxWorld);

		ApplyBlockLighting(sceneState);

		
		CApplyObjectLevelParamBlock p(GetEffectParamBlock());

		if (pEffectFile == 0)
		{
			// TODO: Fixed Function. 
		}
		else
		{
			bool bUsePointTextureFilter = false;

			// apply block space lighting for object whose size is comparable to a single block size
			if (CheckAttribute(MESH_USE_LIGHT) && !(sceneState->IsShadowPass()))
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
					bUsePointTextureFilter = bUsePointTextureFilter || pBlockWorldClient->GetUsePointTextureFiltering();
				}
			}

			if (bUsePointTextureFilter)
			{
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
			pModel->draw(sceneState, p.GetParamsBlock()); 
		}

		CGlobals::GetWorldMatrixStack().pop();
		return S_OK;
	}

	void BMaxObject::LoadPhysics()
	{
		if (m_dwPhysicsMethod > 0 && IsPhysicsEnabled() && (GetStaticActorCount() == 0))
		{
			if (m_pAnimatedMesh && m_pAnimatedMesh->IsLoaded())
			{
				CParaXModel* ppMesh = m_pAnimatedMesh->GetModel();
				if (ppMesh == 0 || ppMesh->GetHeader().maxExtent.x <= 0.f)
				{
					EnablePhysics(false); // disable physics forever, if failed loading physics data
					return;
				}
				// get world transform matrix
				Matrix4 mxWorld;
				GetWorldTransform(mxWorld);
				IParaPhysicsActor* pActor = CGlobals::GetPhysicsWorld()->CreateStaticMesh(m_pAnimatedMesh.get(), mxWorld, m_nPhysicsGroup, &m_staticActors, this);
				if (m_staticActors.empty())
				{
					// disable physics forever, if no physics actors are loaded. 
					EnablePhysics(false);
				}
			}
		}
	}

	void BMaxObject::SetAlwaysLoadPhysics(bool bEnable)
	{
		if (bEnable)
		{
			m_dwPhysicsMethod |= PHYSICS_ALWAYS_LOAD;
		}
		else
		{
			m_dwPhysicsMethod &= (~PHYSICS_ALWAYS_LOAD);
		}
	}

	void BMaxObject::UnloadPhysics()
	{
		int nSize = (int)m_staticActors.size();
		if (nSize > 0)
		{
			for (int i = 0; i < nSize; ++i)
			{
				CGlobals::GetPhysicsWorld()->ReleaseActor(m_staticActors[i]);
			}
			m_staticActors.clear();
		}
	}

	void BMaxObject::SetPhysicsGroup(int nGroup)
	{
		PE_ASSERT(0 <= nGroup && nGroup < 32);
		if (m_nPhysicsGroup != nGroup)
		{
			m_nPhysicsGroup = nGroup;
			UnloadPhysics();
		}
	}

	int BMaxObject::GetPhysicsGroup()
	{
		return m_nPhysicsGroup;
	}

	void BMaxObject::EnablePhysics(bool bEnable)
	{
		if (!bEnable){
			UnloadPhysics();
			m_dwPhysicsMethod |= PHYSICS_FORCE_NO_PHYSICS;
		}
		else
		{
			m_dwPhysicsMethod &= (~PHYSICS_FORCE_NO_PHYSICS);
			if ((m_dwPhysicsMethod&PHYSICS_ALWAYS_LOAD)>0)
				LoadPhysics();
		}
	}

	bool BMaxObject::IsPhysicsEnabled()
	{
		return !((m_dwPhysicsMethod & PHYSICS_FORCE_NO_PHYSICS)>0);
	}

	int BMaxObject::GetStaticActorCount()
	{
		return (int)m_staticActors.size();
	}

	int BMaxObject::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		CTileObject::InstallFields(pClass, bOverride);

		return S_OK;
	}
}