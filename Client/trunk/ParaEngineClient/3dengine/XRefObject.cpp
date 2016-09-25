//-----------------------------------------------------------------------------
// Class:	XRefObject
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007.5.25
// Revised: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "ParaXAnimInstance.h"
#include "CustomCharModelInstance.h"
#include "SceneState.h"
#include "ParaXModel/ParaXStaticModel.h"
#include "MeshObject.h"
#include "XRefObject.h"

using namespace ParaEngine;


//////////////////////////////////////////////////////////////////////////
//
// XReference objects
//
//////////////////////////////////////////////////////////////////////////

ParaEngine::XRefObject::XRefObject( CMeshObject* pParent, AssetEntity* pModel, const Vector3& vOrigin, const Matrix4& mxLocalTransform )
:m_pParent(pParent), m_pModelEntity(pModel), m_vOrigin(vOrigin), m_mxLocalTransform(mxLocalTransform), m_nTechniqueHandle(-1)
{
	SetParamsFromAsset();
}

ParaEngine::XRefObject::XRefObject()
:m_pParent(NULL), m_vOrigin(0,0,0)
{
}

ParaEngine::XRefObject::~XRefObject( void )
{

}

bool ParaEngine::XRefObject::SetParamsFromAsset()
{
	return true;
}


HRESULT ParaEngine::XRefObject::Draw( SceneState * sceneState )
{
	return S_OK;
}

HRESULT ParaEngine::XRefObject::DrawInner( SceneState * sceneState, const Matrix4* pMxWorld/*=0*/, float fCameraToObjectDist/*=0.f*/ )
{
	return S_OK;
}

Matrix4* ParaEngine::XRefObject::GetRenderMatrix( Matrix4& mxWorld , const Matrix4* pMxParent, int nRenderNumber)
{
	// get the parent's rotation and scaling matrix, here it is some trick to reuse the parent node's code. we actually get its world matrix and then remove the translation part.
	Matrix4 mat;
	if(pMxParent==0)
		m_pParent->GetRenderMatrix(mat, nRenderNumber);
	else
		mat = (*pMxParent);

	Vector3 vPos = m_pParent->GetRenderOffset();// get render offset
	mat._41 -= vPos.x;
	mat._42 -= vPos.y;
	mat._43 -= vPos.z;

	Vector3 vOffset;
	ParaVec3TransformCoord(&vOffset, &m_vOrigin, &mat);

	// apply the local transform, apply the parent's rotation and scaling matrix, and then offset origin
	mxWorld = m_mxLocalTransform;

	mxWorld *= mat;
	mxWorld._41 += vPos.x+vOffset.x;
	mxWorld._42 += vPos.y+vOffset.y;
	mxWorld._43 += vPos.z+vOffset.z;
	return &mxWorld;
}

Matrix4* ParaEngine::XRefObject::GetMatrix( Matrix4& mxWorld, const Matrix4* pMatParent )
{
	Matrix4 mat;
	if(pMatParent)
		mat = (*pMatParent);
	else
		m_pParent->GetViewClippingObject()->GetWorldTransform(mat);
	mxWorld = m_mxLocalTransform;
	mxWorld._41 += m_vOrigin.x;
	mxWorld._42 += m_vOrigin.y;
	mxWorld._43 += m_vOrigin.z;
	mxWorld *= mat;
	return &mxWorld;
}

//////////////////////////////////////////////////////////////////////////
//
// XRefAnimInstance class
//
//////////////////////////////////////////////////////////////////////////

XRefAnimInstance::XRefAnimInstance(CMeshObject* pParent, AssetEntity* pModel, const Vector3& vOrigin, const Matrix4& mxLocalTransform)
	:XRefObject(pParent, pModel, vOrigin, mxLocalTransform)
{
}

XRefAnimInstance::~XRefAnimInstance()
{
	DeleteAnimInstance();
}

bool XRefAnimInstance::SetParamsFromAsset()
{
	// in case the asset is loaded successfully, we shall set the primary asset. 
	if(m_pModelEntity.get() && (m_pModelEntity->GetType() == AssetEntity::parax))
	{
		if(((ParaXEntity*)m_pModelEntity.get())->GetPrimaryTechniqueHandle() > 0)
		{
			/** load the default animation */
			if(m_pAI)
			{
				/** load the default animation */
				float fSpeed=0.f;
				m_pAI->LoadDefaultStandAnim(&fSpeed);
			}
			SetPrimaryTechniqueHandle(((ParaXEntity*)m_pModelEntity.get())->GetPrimaryTechniqueHandle());
			return true;
		}
	}
	else
	{
		return false;
	}
	return false;
}


void XRefAnimInstance::DeleteAnimInstance()
{
	if(m_pAI)    
	{
		if(m_pModelEntity->GetType() == AssetEntity::parax)
		{
			// manually clear this resource
			m_pAI.reset();
		}
	}
}

/// the size and speed of the animation and the biped object is not synchronized.
/// you need to manually get the current size and speed from the animation instance
/// so that the biped and its animation instance could synchronize in action.
CAnimInstanceBase* XRefAnimInstance::GetAnimInstance()
{
	if(!m_pAI && m_pModelEntity)
	{
		if(m_pModelEntity->GetType() == AssetEntity::parax)
		{
			m_pAI = ((ParaXEntity*)m_pModelEntity.get())->CreateAnimInstance();
			if(m_pAI)
			{
				/** load the default animation */
				float fSpeed=0.f;
				m_pAI->LoadDefaultStandAnim(&fSpeed);
			}
		}
		else
		{
			OUTPUT_LOG("error: loading XRefAnimInstance");
		}
	}
	return m_pAI.get();
}

CParaXAnimInstance* XRefAnimInstance::GetParaXAnimInstance()
{
	CAnimInstanceBase* pAI = GetAnimInstance();
	if(pAI && m_pModelEntity)
	{
		if(m_pModelEntity->GetType() == AssetEntity::parax)
		{
			return (CParaXAnimInstance*)pAI;
		}
		else
		{
			OUTPUT_LOG("error: loading XRefAnimInstance");
		}
	}
	return NULL;
}

CharModelInstance* XRefAnimInstance::GetCharModelInstance()
{
	CParaXAnimInstance* pAI =  GetParaXAnimInstance();
	if(pAI)
	{
		CharModelInstance * pChar =  pAI->GetCharModel();
		if(pChar)
		{
			return pChar;
		}
	}
	return NULL;
}

HRESULT XRefAnimInstance::Draw( SceneState * sceneState )
{
	if(GetPrimaryTechniqueHandle()<0)
	{
		// in case the asset is loaded successfully, we shall set the primary asset. 
		SetParamsFromAsset();

		// try loading the asset if it has not been done before. 
		m_pModelEntity->LoadAsset();
		return E_FAIL;
	}

	//////////////////////////////////////////////////////////////////////////
	// we animate in the draw, so we do not need to call animate in a pre-process
	Animate(sceneState->dTimeDelta, sceneState->GetRenderFrameCount());

	//////////////////////////////////////////////////////////////////////////
	// we will draw the animated model 

	// for models with particle systems
	sceneState->SetCurrentSceneObject(m_pParent);

	// call Draw() of biped animation instance
	CAnimInstanceBase* pAI =  GetAnimInstance();
	if(pAI)
	{
#ifdef USE_DIRECTX_RENDERER
		// use the lighting of the parent.
		CGlobals::GetEffectManager()->applyObjectLocalLighting(m_pParent);
#endif
		// draw the model
		Matrix4 mxWorld;
		pAI->Draw(sceneState, GetRenderMatrix(mxWorld, NULL, sceneState->GetRenderFrameCount()));
	}
	return S_OK;
}

HRESULT XRefAnimInstance::DrawInner( SceneState * sceneState, const Matrix4* pMxWorld/*=0*/, float fCameraToObjectDist/*=0.f*/ )
{
	if(m_pModelEntity.get() == 0)
		return E_FAIL;
	if(GetPrimaryTechniqueHandle()<0)
	{
		// in case the asset is loaded successfully, we shall set the primary asset. 
		SetParamsFromAsset();

		// try loading the asset if it has not been done before. 
		m_pModelEntity->LoadAsset();
		return E_FAIL;
	}

	if(!CGlobals::GetEffectManager()->IsCurrentEffectValid())
	{
		return E_FAIL;
	}

	//////////////////////////////////////////////////////////////////////////
	// we animate in the draw, so we do not need to call animate in a pre-process
	Animate(sceneState->dTimeDelta, sceneState->GetRenderFrameCount());


	//////////////////////////////////////////////////////////////////////////
	// we will draw the animated model 
	// call Draw() of biped animation instance
	CAnimInstanceBase* pAI =  GetAnimInstance();
	if(pAI)
	{
		// use the lighting of the parent.
		CGlobals::GetEffectManager()->applyObjectLocalLighting(m_pParent);

		// draw the model
		Matrix4 mxWorld;
		pAI->Draw(sceneState, GetRenderMatrix(mxWorld, pMxWorld, sceneState->GetRenderFrameCount()));
	}

	return S_OK;
}

void XRefAnimInstance::Animate( double dTimeDelta , int nRenderNumber)
{
	if(dTimeDelta<0.000001)
		return;
	/** For animation instance. */
	CAnimInstanceBase* pAI =  GetAnimInstance();
	if(pAI)
	{
		pAI->Animate(dTimeDelta, nRenderNumber);
	}
}

bool ParaEngine::XRefAnimInstance::SetReplaceableTexture( int ReplaceableTextureID, TextureEntity* pTextureEntity )
{
	CharModelInstance* pChar =  GetCharModelInstance();
	if(pChar)
	{
		return pChar->SetReplaceableTexture(ReplaceableTextureID, pTextureEntity);
	}
	return false;
}

Matrix4* ParaEngine::XRefAnimInstance::GetAttachmentMatrix( Matrix4& pOut, int nAttachmentID/*=0*/ , int nRenderNumber)
{
	CParaXAnimInstance* pAnimInst = GetParaXAnimInstance();
	if(pAnimInst)
	{
		return pAnimInst->GetAttachmentMatrix(&pOut, nAttachmentID, nRenderNumber);
	}
	return NULL;
}

void ParaEngine::XRefAnimInstance::SetAnimation( int nAnimID )
{
	CAnimInstanceBase * pAnim = GetAnimInstance();
	if(pAnim)
	{
		float fSpeed;
		pAnim->LoadAnimation(nAnimID, &fSpeed);
	}
}

int ParaEngine::XRefAnimInstance::GetAnimation()
{
	CAnimInstanceBase * pAnim = GetAnimInstance();
	if(pAnim)
	{
		return pAnim->GetCurrentAnimation();
	}
	return 0;
}

void ParaEngine::XRefAnimInstance::SetAnimFrame(int nFrame)
{
	CAnimInstanceBase * pAnim = GetAnimInstance();
	if(pAnim)
	{
		pAnim->SetAnimFrame(nFrame);
	}
}

int ParaEngine::XRefAnimInstance::GetAnimFrame()
{
	CAnimInstanceBase * pAnim = GetAnimInstance();
	if(pAnim)
	{
		return pAnim->GetAnimFrame();
	}
	return 0;

}

void ParaEngine::XRefAnimInstance::SetUseGlobalTime(bool bUseGlobalTime)
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	if(pAI)
	{
		pAI->SetUseGlobalTime(bUseGlobalTime);
	}
}

bool ParaEngine::XRefAnimInstance::IsUseGlobalTime()
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	if(pAI)
	{
		return pAI->IsUseGlobalTime();
	}
	return false;
}

/////////////////////////////////////////////////////////////////
//
// call the draw method
//
/////////////////////////////////////////////////////////////////
ParaEngine::XRefMeshObject::XRefMeshObject( CMeshObject* pParent, AssetEntity* pModel, const Vector3& vOrigin, const Matrix4& mxLocalTransform )
	:XRefObject(pParent, pModel, vOrigin, mxLocalTransform)
{

}

ParaEngine::XRefMeshObject::XRefMeshObject()
	:XRefObject()
{

}

ParaEngine::XRefMeshObject::~XRefMeshObject( void )
{

}

bool XRefMeshObject::SetParamsFromAsset()
{
	// in case the asset is loaded successfully, we shall set the primary asset. 
	if(m_pModelEntity.get()!=0 &&  ((MeshEntity*)m_pModelEntity.get())->GetPrimaryTechniqueHandle() > 0)
	{
		SetPrimaryTechniqueHandle(((MeshEntity*)m_pModelEntity.get())->GetPrimaryTechniqueHandle());
		return true;
	}
	return false;
}

HRESULT ParaEngine::XRefMeshObject::DrawInner( SceneState * sceneState, const Matrix4* pMxWorld/*=0*/, float fCameraToObjectDist/*=0.f*/ )
{

	if(m_pModelEntity.get() == 0)
		return E_FAIL;
	if(GetPrimaryTechniqueHandle()<0)
	{
		// in case the asset is loaded successfully, we shall set the primary asset. 
		SetParamsFromAsset();

		// try loading the asset if it has not been done before. 
		m_pModelEntity->LoadAsset();
		return E_FAIL;
	}

	if(!CGlobals::GetEffectManager()->IsCurrentEffectValid())
	{
		return E_FAIL;
	}

	int nIndex = ((MeshEntity*)m_pModelEntity.get())->GetLodIndex(fCameraToObjectDist);
	CParaXStaticModelRawPtr pMesh = ((MeshEntity*)m_pModelEntity.get())->GetMesh(nIndex);

	if(pMesh == NULL)
		return E_FAIL;

	// get world transform matrix
	Matrix4 mxWorld;
	GetRenderMatrix(mxWorld, pMxWorld, sceneState->GetRenderFrameCount());

	RenderDevicePtr pd3dDevice = sceneState->m_pd3dDevice;

	// use the lighting of the parent.
	CGlobals::GetEffectManager()->applyObjectLocalLighting(m_pParent);

	/// set whether to use the material(texture) in the mesh file
	pMesh->m_bUseMaterials = true;

	CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
	if ( pEffectFile == 0)
	{
#ifdef USE_DIRECTX_RENDERER
		//////////////////////////////////////////////////////////////////////////
		// fixed programming pipeline
		CGlobals::GetWorldMatrixStack().push(mxWorld);

		// render by default as non-transparent.
		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		CGlobals::GetEffectManager()->EnableZWrite(true);
		CGlobals::GetEffectManager()->SetCullingMode(true);

		// Draw with mesh materials
		pMesh->Render(sceneState, pd3dDevice, true, true,sceneState->fAlphaFactor);
		CGlobals::GetWorldMatrixStack().pop();
#endif
	}
	else
	{
		// apply per mesh asset effect parameters
		if(((MeshEntity*)m_pModelEntity.get())->GetParamBlock())
		{
			((MeshEntity*)m_pModelEntity.get())->GetParamBlock()->ApplyToEffect(pEffectFile);
		}

		//////////////////////////////////////////////////////////////////////////
		// draw using effect file
		CGlobals::GetWorldMatrixStack().push(mxWorld);
		pMesh->Render(sceneState, pEffectFile, true, true,sceneState->fAlphaFactor);
		CGlobals::GetWorldMatrixStack().pop();
	}
	return S_OK;
}

HRESULT ParaEngine::XRefMeshObject::Draw( SceneState * sceneState )
{
	//////////////////////////////////////////////////////////////////////////
	// we will draw the static mesh model 
	sceneState->SetCurrentSceneObject(m_pParent);
	
	DrawInner(sceneState, NULL, sceneState->GetCameraToCurObjectDistance());
	return S_OK;
}

bool ParaEngine::XRefMeshObject::SetReplaceableTexture( int ReplaceableTextureID, TextureEntity* pTextureEntity )
{
	// TODO: 
	return true;
}
