//-----------------------------------------------------------------------------
// Class: CMeshObject
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "MeshObject.h"
#include "SceneState.h"
#include "SceneObject.h"
#include "AutoCamera.h"
#include "MeshEntity.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockWorldClient.h"
#include "LightManager.h"
// vegetation shader constant
#include "shaders/simple_mesh_normal_vegetation.h"
#include "ParaXAnimInstance.h"
#include "ParaEngineSettings.h"
#include "XRefObject.h"
#include "ViewCullingObject.h"
#include "util/StringHelper.h"

#include "memdebug.h"
using namespace ParaEngine;


/** minimum height of vegetation models in meters. this is something used for vegetation shader. */
#define MIN_VEGETATION_HEIGHT	1.f

// use the fast version printf
#define My_snprinf StringHelper::fast_snprintf

/**
* a global function for linearly animating construction percentage.
* @param fCurrentPercentage: current percentage
* @param fDeltaTime: time delta in seconds.
* @param fObjectHeight: the vertical height of the object in object space. i.e.height without the local and world transformation being applied. 
* @param fSpeed: unit per seconds. 
* @return: the new percentage after delta time. return value is clamped to [0,1]
*/
float AnimateCtorPercentage_linear(float fCurrentPercentage, float fDeltaTime, float fObjectHeight=1.0f)
{
	float fSpeed = CGlobals::GetSettings().GetCtorSpeed();
	float fPercentage = fCurrentPercentage;
	fPercentage += (float)fDeltaTime*fSpeed/fObjectHeight;
	if(fPercentage>1.0f)
		fPercentage = 1.0f;
	if(fPercentage<0.f)
		fPercentage = 0.f;
	return fPercentage;
}

CMeshObject::CMeshObject(void)
:m_fCtorPercentage(0.f),m_bIsTransparent(false),m_bForceNoLight(false),m_XRefSearched(false), 
m_nAnimID(0), m_nAnimFrame(0), m_bUseGlobalTime(false), m_bViewClippingUpdated(false), m_pViewClippingObject(NULL), m_bIsPersistent(true), m_bDisableFaceCulling(false)
{
	m_mxLocalTransform = Matrix4::IDENTITY;
}

CMeshObject::~CMeshObject(void)
{
	SAFE_DELETE(m_pViewClippingObject);
	m_ReplaceableTextures.clear();
	CleanupXRefInstances();
}

void CMeshObject::CleanupXRefInstances()
{
	int nTotal = (int)m_XRefObjects.size();
	for (int i=0;i<nTotal;++i)
	{
		XRefObject* pEntity = m_XRefObjects[i];
		SAFE_DELETE(pEntity);
	}
	m_XRefObjects.clear();

}
float CMeshObject::GetCtorPercentage()
{
	return m_fCtorPercentage;
}

void CMeshObject::SetCtorPercentage(float fPercentage)
{
	m_fCtorPercentage = fPercentage;
}

//-----------------------------------------------------------------------------
// Name: InitObject
/// Desc: It saves device references, but do not initialize those devices.
///		 it only does initialization not concerning device object. Object dependent
///       initialization is done with InitDeviceObjects(). if bUseMaterials is set to
///       false, ppTexture and Material are ignored, otherwise they can be used to 
///		 specify textures for the first subset of the mesh.
//-----------------------------------------------------------------------------
HRESULT CMeshObject::InitObject(MeshEntity* ppMesh, TextureEntity* ppTexture, Vector3 vPos)
{
	m_XRefSearched = false;
	m_ppMesh = ppMesh;
	m_ppTexture = ppTexture;
	m_ReplaceableTextures.clear();
	m_vPos = vPos;

	if(GetPrimaryTechniqueHandle()<0)
	{
		// in case the asset is loaded successfully, we shall set the primary asset. 
		SetParamsFromAsset();
	}
	return S_OK;
}


bool CMeshObject::SetParamsFromAsset()
{
	// in case the asset is loaded successfully, we shall set the primary asset. 
	if(m_ppMesh.get()!=0 && m_ppMesh->GetPrimaryTechniqueHandle() > 0)
	{
		const string& filename = m_ppMesh->GetKey();
		int nSize = (int)filename.size();
		for (int i=0;i<5;++i)
		{
			// Index of the character '_'
			int nMarkIndex = nSize-4-i*2;
			if(nMarkIndex>=0 && filename[nMarkIndex] == '_')
			{
				char symbol=filename[nMarkIndex+1];
				switch (symbol)
				{
				case 'b':
					{
						// if model file name end with "_b", it will be treated as billboarded.
						SetBillboarded(true);
						break;
					}
				case 'r':
					{
						SetShadowCaster(false);
						SetShadowReceiver(true);
						break;
					}
				case 't':
					{
						SetTransparent(true);
						break;
					}
				case 'p':
					{
						SetBillboarded(true);
						SetTransparent(true);
						ForceNoLight(true);
						break;
					}
				case 'd':
					{
						ForceNoLight(true);
						break;
					}
				case 'e':
					{
						SetShadowCaster(false);
						break;
					}
				case 'v':
					{
						SetVegetation(true);
						break;
					}
				}
				
			}
			else
				break;
		}

		if(!IsShadowReceiver())
		{
			if(!IsNoLight())
			{
				SetPrimaryTechniqueHandle(m_ppMesh->GetPrimaryTechniqueHandle());
			}
			else
			{
				// use a special shader without light. 
				SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL_UNLIT);
			}
		}
		else
			SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL_SHADOW);

		// stop the construction shader for other effects.
		if(GetPrimaryTechniqueHandle() != TECH_SIMPLE_MESH_NORMAL)
			SetCtorPercentage(1.f);

		if(IsVegetation() && GetPrimaryTechniqueHandle() == TECH_SIMPLE_MESH_NORMAL)
			SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL_VEGETATION);

		//////////////////////////////////////////////////////////////////////////
		// Newly added: 2007.5.30
		//
		// this allows the base static mesh to share replaceable textures with any of its xref animated meshes.
		// if the xref mesh does not contains replaceable textures, this function takes no effect. 
		//////////////////////////////////////////////////////////////////////////
		
		m_XRefSearched = false;
		int nCount = (int)m_ReplaceableTextures.size();
		if(nCount > 0)
		{
			SearchXRefObject();
			int nTotal = (int)m_XRefObjects.size();
			if(nTotal > 0)
			{
				for (int ReplaceableTextureID=0; ReplaceableTextureID < nCount; ++ ReplaceableTextureID)
				{
					if(m_ReplaceableTextures[ReplaceableTextureID].get() != 0)
					{
						for (int i=0;i<nTotal;++i)
						{
							XRefObject* pEntity = m_XRefObjects[i];
							pEntity->SetReplaceableTexture(ReplaceableTextureID, m_ReplaceableTextures[ReplaceableTextureID].get());
						}
					}
				}
			}
		}
		if(m_nAnimID != 0)
			SetAnimation(m_nAnimID);
		if(m_nAnimFrame != 0)
			SetAnimFrame(m_nAnimFrame);
		if(m_bUseGlobalTime)
			SetUseGlobalTime(m_bUseGlobalTime);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Name: InitDeviceObjects
// Desc: Initializes the class and readies it for animation
//-----------------------------------------------------------------------------
HRESULT CMeshObject::InitDeviceObjects()
{
    HRESULT hr = S_OK;
	return hr;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Reinitialize necessary objects
//-----------------------------------------------------------------------------
HRESULT CMeshObject::RestoreDeviceObjects()
{
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Free D3D objects so that the device can be reset.
//-----------------------------------------------------------------------------
HRESULT CMeshObject::InvalidateDeviceObjects()
{
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Performs cleanup tasks for CBipedObject
//-----------------------------------------------------------------------------
void CMeshObject::Cleanup()
{
    // TODO: ...
}

IViewClippingObject* CMeshObject::GetViewClippingObject()
{
	if(m_ppMesh && m_ppMesh->IsValid())
	{
		if(m_bViewClippingUpdated)
		{
			return m_pViewClippingObject;
		}
		else
		{
			// if mesh is 0 size, this possibly means that it is not ready yet. 
			if((m_ppMesh->GetAABBMax().x - m_ppMesh->GetAABBMin().x) <= 0.f )
				return this;
			if(m_pViewClippingObject == 0)
			{
				m_pViewClippingObject = new CViewCullingObject();
			}
			m_pViewClippingObject->SetAABB(&(m_ppMesh->GetAABBMin()), &(m_ppMesh->GetAABBMax()));
			m_pViewClippingObject->SetTransform(&m_mxLocalTransform, m_vPos, &m_fYaw);
			m_bViewClippingUpdated = true;
			return m_pViewClippingObject;
		}
	}
	return this;
}

void CMeshObject::SetLocalTransform(const Matrix4& mXForm)
{
	m_mxLocalTransform = mXForm;
	m_bViewClippingUpdated = false;
}
void CMeshObject::SetLocalTransform(float fScale, float fRotX, float fRotY, float fRotZ)
{
	Matrix4 mx;
	ParaMatrixScaling(&m_mxLocalTransform, fScale, fScale, fScale);
	ParaMatrixRotationZ(&mx, fRotZ);
	m_mxLocalTransform = m_mxLocalTransform*mx; 
	ParaMatrixRotationX(&mx, fRotX);
	m_mxLocalTransform = m_mxLocalTransform*mx; 
	ParaMatrixRotationY(&mx, fRotY);
	m_mxLocalTransform = m_mxLocalTransform*mx;
}
void CMeshObject::SetLocalTransform(float fScale, const Quaternion& quat)
{
	Matrix4 mx;
	ParaMatrixScaling(&m_mxLocalTransform, fScale, fScale, fScale);
	quat.ToRotationMatrix(mx, Vector3::ZERO);
	m_mxLocalTransform = m_mxLocalTransform*mx; 
	m_bViewClippingUpdated = false;
}
void CMeshObject::GetLocalTransform(Matrix4* localTransform)
{
	*localTransform = m_mxLocalTransform;
	m_bViewClippingUpdated = false;
}

void CMeshObject::GetLocalTransform(float* fScale, Quaternion* quat)
{
	Vector3 vScale(1,1,1);
	Vector3 vTrans(0,0,0);
	// assume uniform scale
	*fScale = vScale.x; 
	ParaMatrixDecompose(&vScale, quat, &vTrans, &m_mxLocalTransform);
	m_bViewClippingUpdated = false;
}

void CMeshObject::SetRotation(const Quaternion& quat)
{
	Vector3 vScale(1,1,1);
	Vector3 vTrans(0,0,0);
	Quaternion oldQuat;
	if(ParaMatrixDecompose(&vScale, &oldQuat, &vTrans, &m_mxLocalTransform))
	{
		// assume uniform scale by using vScale.x
		ParaMatrixAffineTransformation(&m_mxLocalTransform, vScale.x, NULL, &quat, &vTrans);
	}
	m_bViewClippingUpdated = false;
}

void CMeshObject::GetRotation(Quaternion* quat)
{
	Vector3 vScale(1,1,1);
	Vector3 vTrans(0,0,0);
	ParaMatrixDecompose(&vScale, quat, &vTrans, &m_mxLocalTransform);
	m_bViewClippingUpdated = false;
}

void CMeshObject::Rotate(float x, float y, float z)
{
	Matrix4 mat;
	GetLocalTransform(&mat);
	Matrix4 mat1;
	if(x!=0.f)
		mat = (*ParaMatrixRotationX(&mat1, x))*mat;
	if(y!=0.f)
		mat = (*ParaMatrixRotationY(&mat1, y))*mat;
	if(z!=0.f)
		mat = (*ParaMatrixRotationZ(&mat1, z))*mat;

	SetLocalTransform(mat);
}

void CMeshObject::SetScaling(float s)
{
	Matrix4 mat;
	GetLocalTransform(&mat);
	float fScalingX,fScalingY,fScalingZ;
	Math::GetMatrixScaling(mat, &fScalingX,&fScalingY,&fScalingZ);
	Matrix4 mat1;
	ParaMatrixScaling(&mat1, s/fScalingX,s/fScalingY,s/fScalingZ);
	mat = mat1*mat;
	SetLocalTransform(mat);
}

float CMeshObject::GetScaling()
{
	Matrix4 mat;
	GetLocalTransform(&mat);
	float fScalingX,fScalingY,fScalingZ;
	Math::GetMatrixScaling(mat, &fScalingX,&fScalingY,&fScalingZ);

	return max(max(fScalingX,fScalingY), fScalingZ);
}

void CMeshObject::Reset()
{
	SetLocalTransform(*CGlobals::GetIdentityMatrix());
}


Matrix4* CMeshObject::GetRenderMatrix( Matrix4& mxWorld, int nRenderNumber)
{
	// render offset
	Vector3 vPos = GetRenderOffset();

	// get world transform matrix
	Matrix4 mx;
	float fFacing = GetFacing();

	if(IsBillboarded())
	{
		// TODO: how about in the reflection pass?
		Vector3 vDir = m_vPos - CGlobals::GetScene()->GetCurrentCamera()->GetEyePosition();
		if( vDir.x > 0.0f )
			fFacing += -atanf(vDir.z/vDir.x)+MATH_PI/2;
		else
			fFacing += -atanf(vDir.z/vDir.x)-MATH_PI/2;
	}
	if(fFacing != 0.f)
	{
		ParaMatrixRotationY(&mxWorld, fFacing);/** set facing and rotate local matrix round y axis*/
		mxWorld = m_mxLocalTransform*mxWorld;
	}
	else
	{
		mxWorld = m_mxLocalTransform;
	}

	// world translation
	mxWorld._41 += vPos.x;
	mxWorld._42 += vPos.y;
	mxWorld._43 += vPos.z;
	return &mxWorld;
}

HRESULT CMeshObject::DrawInner( SceneState * sceneState, const Matrix4* pMxWorld, float fCameraToObjectDist, CParameterBlock* materialParams)
{
	if(m_ppMesh==0 || ! (m_ppMesh->IsValid()))
		return E_FAIL;

	if(GetPrimaryTechniqueHandle()<0)
	{
		// in case the asset is loaded successfully, we shall set the primary asset. 
		SetParamsFromAsset();

		// try loading the asset if it has not been done before. 
		m_ppMesh->LoadAsset();
		return E_FAIL;
	}

	if(!CGlobals::GetEffectManager()->IsCurrentEffectValid())
	{
		return E_FAIL;
	}

	int nIndex = m_ppMesh->GetLodIndex(fCameraToObjectDist);
	CParaXStaticModelRawPtr pMesh = m_ppMesh->GetMesh(nIndex);

	if(pMesh == NULL)
		return E_FAIL;

	RenderDevicePtr pd3dDevice = sceneState->m_pd3dDevice;

	//////////////////////////////////////////////////////////////////////////
	// set replaceable textures if any
	int nNumReplaceableTextures=pMesh->GetNumReplaceableTextures();
	for (int i=0;i<nNumReplaceableTextures;++i)
	{
		TextureEntity* pTexOld = pMesh->GetReplaceableTexture(i);
		if(pTexOld) // this ensures that the replaceable texture at index i is used in the model.
		{
			TextureEntity* pTexNew = GetReplaceableTexture(i);
			if(pTexOld!=pTexNew)
			{
				pMesh->SetReplaceableTexture(i, pTexNew);
			}
		}
	}

	/// set whether to use the material(texture) in the mesh file
	pMesh->m_bUseMaterials = true;
	bool bIsRendered = false;
	CGlobals::GetEffectManager()->applyObjectLocalLighting(this);

	if(GetSelectGroupIndex() >= 0 && !sceneState->IsShadowPass())
	{
		if((sceneState->m_nCurRenderGroup & RENDER_SELECTION) == 0)
		{
			// we will not draw selected objects during normal render pass, instead we render it during selection render pass. 
			// selection render pass is usually called after opache meshes are rendered. 
			return S_OK;
		}

		// for selection render effect
		CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if(pEffectFile!=0)
		{
			int nSelectionEffectStyle = GetSelectionEffect();
			if(nSelectionEffectStyle == RenderSelectionStyle_border)
			{
#ifdef USE_DIRECTX_RENDERER
				// Let us render the border using the same technique as rendering text shadow.
				// i.e. render 4 times shifting 2 pixels around the border. 
				CGlobals::GetEffectManager()->BeginEffect(TECH_SIMPLE_MESH_NORMAL_BORDER, &(sceneState->m_pCurrentEffect));
				pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();

				pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

				if(pEffectFile!=0)
				{
					if(pMxWorld!=0)
						CGlobals::GetWorldMatrixStack().push(*pMxWorld);

					// the border width in meters. 
					const float border_width = 0.03f;
				
					for(int x=-1; x<=1; x+=2)
					{
						for(int y=-1; y<=1; y+=2)
						{
							Vector3 vOffsets(x*border_width, y*border_width, 0.f);
							pEffectFile->GetDXEffect()->SetRawValue("g_offsets", &vOffsets, 0, sizeof(Vector3));
							pMesh->Render(sceneState, pEffectFile, true, true,sceneState->fAlphaFactor);
						}
					}
				
					if(pMxWorld!=0)
						CGlobals::GetWorldMatrixStack().pop();
				}

				pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
#endif
				// change back to primary technique
				CGlobals::GetEffectManager()->BeginEffect(GetPrimaryTechniqueHandle(), &(sceneState->m_pCurrentEffect));

				if (CheckAttribute(MESH_USE_LIGHT)  && !(sceneState->IsShadowPass()))
				{
					BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
					if (pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
					{
						// Note: do this if one wants point light
						CGlobals::GetEffectManager()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
						CGlobals::GetEffectManager()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
					}
				}
			}
			else if(nSelectionEffectStyle == RenderSelectionStyle_unlit)
			{
#ifdef USE_DIRECTX_RENDERER
				CGlobals::GetEffectManager()->BeginEffect(TECH_SIMPLE_MESH_NORMAL_UNLIT, &(sceneState->m_pCurrentEffect));
				pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
				if(pEffectFile!=0)
				{
					Vector3 vColorAdd(0.2f, 0.2f, 0.2f);
					pEffectFile->GetDXEffect()->SetRawValue("g_color_add", &vColorAdd, 0, sizeof(Vector3));

					if(pMxWorld!=0)
						CGlobals::GetWorldMatrixStack().push(*pMxWorld);
				
					if (CheckAttribute(MESH_USE_LIGHT) && !(sceneState->IsShadowPass()))
					{
						BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
						if (pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
						{
							// Note: do this if one wants point light
							CGlobals::GetEffectManager()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
							CGlobals::GetEffectManager()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
						}
					}

					pMesh->Render(sceneState, pEffectFile, true, true,sceneState->fAlphaFactor);
					
					if(pMxWorld!=0)
						CGlobals::GetWorldMatrixStack().pop();
					bIsRendered = true;
				}
#endif
				// change back to primary technique
				CGlobals::GetEffectManager()->BeginEffect(GetPrimaryTechniqueHandle(), &(sceneState->m_pCurrentEffect));
			}
		}
		else
		{
			// TODO: find a way to render a single color with offsets with fixed function. 

			// change back to primary technique
			CGlobals::GetEffectManager()->BeginEffect(GetPrimaryTechniqueHandle(), &(sceneState->m_pCurrentEffect));
		}

	}

	if(!bIsRendered)
	{
		bool bRevertCulling = false;
		if(IsFaceCullingDisabled() && ! (CGlobals::GetEffectManager()->IsD3DCullingDisabled()))
		{
			CGlobals::GetEffectManager()->DisableD3DCulling(true);
			bRevertCulling = true;
		}

		CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if ( pEffectFile == 0)
		{
#ifdef USE_DIRECTX_RENDERER
			//////////////////////////////////////////////////////////////////////////
			// fixed programming pipeline
			if(pMxWorld!=0)
				CGlobals::GetWorldMatrixStack().push(*pMxWorld);

			// render by default as non-transparent.
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			CGlobals::GetEffectManager()->SetCullingMode(true);

			if(GetPrimaryTechniqueHandle() == TECH_SIMPLE_MESH_NORMAL_CTOR)
			{
				// animate the construction process, physics will be applied only after the construction is complete.
				float fPercentage = GetCtorPercentage();
				float fObjectHeight = m_ppMesh->GetAABBMax().y;
				Vector4 fogParam(fPercentage*fObjectHeight, CGlobals::GetSettings().GetCtorHeight(), 1.0f, 0.1f);
				// animate for the next frame
				fPercentage = AnimateCtorPercentage_linear(fPercentage, (float)sceneState->dTimeDelta, fObjectHeight);
				SetCtorPercentage(fPercentage);
			}

			//if(m_bUseMaterials)
			//{// Draw with assets materials
			//	if(m_ppTexture)
			//		pd3dDevice->SetTexture( 0, (m_ppTexture->GetTexture()) );
			//	pd3dDevice->SetMaterial( & m_Material );
			//	pMesh->GetLocalMesh()->DrawSubset( 0 );
			//}
			//else
			{// Draw with mesh materials
				pMesh->Render(sceneState, pd3dDevice, true, true,sceneState->fAlphaFactor);
			}

			if(pMxWorld!=0)
				CGlobals::GetWorldMatrixStack().pop();
#endif
		}
		else
		{
			bool bHasSetShaderOpacity = false;
			if(GetPrimaryTechniqueHandle() == TECH_SIMPLE_MESH_NORMAL_CTOR)
			{
				// animate the construction process, physics will be applied only after the construction is complete
				float fPercentage = GetCtorPercentage();
				float fObjectHeight = m_ppMesh->GetAABBMax().y;
				if(fObjectHeight > 0.01f)
				{
					Vector4 fogParam(fPercentage*fObjectHeight, CGlobals::GetSettings().GetCtorHeight(), 1.0f, 0.1f);
					LinearColor vColor = CGlobals::GetSettings().GetCtorColor();
					pEffectFile->applyFogParameters(true, &fogParam, &vColor);

					// animate for the next frame
					fPercentage = AnimateCtorPercentage_linear(fPercentage, (float)sceneState->dTimeDelta, fObjectHeight);
					SetCtorPercentage(fPercentage);
				}
				else
				{
					SetCtorPercentage(1.f);
				}
			}
			else if(GetPrimaryTechniqueHandle() == TECH_SIMPLE_MESH_NORMAL_VEGETATION)
			{
				//////////////////////////////////////////////////////////////////////////
				// set the wave time parameter
				double time = CGlobals::GetGameTime(); 
				// in case it loses accuracy, we will subtract integer number of 2*PI from time
				time -= ((int)(time / (2*MATH_PI)))*(2*MATH_PI);

				//////////////////////////////////////////////////////////////////////////
				// set object height
				float fObjectHeight = m_ppMesh->GetAABBMax().y;

				if(fObjectHeight<MIN_VEGETATION_HEIGHT)
					fObjectHeight = MIN_VEGETATION_HEIGHT; 
				//////////////////////////////////////////////////////////////////////////
				// world origin adjustment for the wave length 
				Vector3 vOrigin = CGlobals::GetScene()->GetRenderOrigin();
				float worldX = ((float)((int)(vOrigin.x/VEGETATION_WAVE_LENGTH)))*((float)VEGETATION_WAVE_LENGTH);
				float worldZ = ((float)((int)(vOrigin.z/VEGETATION_WAVE_LENGTH)))*((float)VEGETATION_WAVE_LENGTH);
				vOrigin.x = vOrigin.x - worldX;
				vOrigin.z = vOrigin.z - worldZ;
				Vector4 vTmp((float)time, fObjectHeight, vOrigin.x, vOrigin.z);
				pEffectFile->setParameter(CEffectFile::k_ConstVector0, (const float*)&vTmp);
			}
			else
			{
				// apply per mesh asset effect parameters
				if(materialParams!=0)
				{
					materialParams->ApplyToEffect(pEffectFile);
				}
				else 
				{
					if(m_ppMesh->GetParamBlock())
					{
						m_ppMesh->GetParamBlock()->ApplyToEffect(pEffectFile);
					}
					// apply per mesh object effect parameters
					if(GetEffectParamBlock()!=0)
					{
						GetEffectParamBlock()->ApplyToEffect(pEffectFile);
					}
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// draw using effect file
			if(pMxWorld!=0)
				CGlobals::GetWorldMatrixStack().push(*pMxWorld);
			pMesh->Render(sceneState, pEffectFile, true, true,sceneState->fAlphaFactor, materialParams);

			if(materialParams!=0)
			{
#ifdef USE_DIRECTX_RENDERER
				CParameter* pParams = materialParams->GetParameter("g_opacity");
				if(pParams && ((float)(*pParams)) < 1.f )
				{
					float fOpacity = 1.f;
					pEffectFile->GetDXEffect()->SetRawValue(pParams->GetName().c_str(), &fOpacity, 0, pParams->GetRawDataLength());
				}
#endif
			}

			if(pMxWorld!=0)
				CGlobals::GetWorldMatrixStack().pop();
		}

		if(bRevertCulling)
		{
			CGlobals::GetEffectManager()->DisableD3DCulling(false);
		}
	}

	//
	// for static and animated x-reference objects
	// 
	int nTotal = GetXRefInstanceCount();
	for (int i=0;i<nTotal;++i)
	{
		XRefObject* pEntity = GetXRefInstanceByIndex(i);
		if(pEntity)
		{
			pEntity->DrawInner(sceneState, pMxWorld, fCameraToObjectDist);
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Draw()
/// Desc: Render this instance using the current settings.
//-----------------------------------------------------------------------------

HRESULT CMeshObject::Draw( SceneState * sceneState)
{
	if(m_ppMesh==0 || ! (m_ppMesh->IsValid()))
		return E_FAIL;

	sceneState->SetCurrentSceneObject(this);
	
	SetFrameNumber(sceneState->m_nRenderCount);

	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	// apply block space lighting for object whose size is comparable to a single block size
	if ((CheckAttribute(MESH_USE_LIGHT) || sceneState->IsDeferredShading()) && !(sceneState->IsShadowPass()))
	{
		BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
		if(pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
		{
			uint8_t brightness[2];
			Uint16x3 blockId_ws(0,0,0);
			Vector3 vPos = GetPosition();
			BlockCommon::ConvertToBlockIndex(vPos.x, vPos.y+0.1f, vPos.z, blockId_ws.x, blockId_ws.y, blockId_ws.z);
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
			
			// Note: do this if one wants point light
			pEffectManager->SetSamplerState( 0, D3DSAMP_MINFILTER,  D3DTEXF_POINT);
			pEffectManager->SetSamplerState( 0, D3DSAMP_MAGFILTER,  D3DTEXF_POINT);
		}
	}
	else
	{
		pEffectManager->SetSamplerState(0, D3DSAMP_MINFILTER, pEffectManager->GetDefaultSamplerState(0, D3DSAMP_MINFILTER));
		pEffectManager->SetSamplerState(0, D3DSAMP_MAGFILTER, pEffectManager->GetDefaultSamplerState(0, D3DSAMP_MAGFILTER));
	}

	// get world transform matrix
	Matrix4 mxWorld;
	GetRenderMatrix(mxWorld);

	DrawInner(sceneState, &mxWorld, sceneState->GetCameraToCurObjectDistance());

	sceneState->EnableLocalMaterial(false);

    return S_OK;
}


void ParaEngine::CMeshObject::ForceNoLight( bool bNoLight )
{
	m_bForceNoLight = bNoLight;
}

bool ParaEngine::CMeshObject::IsNoLight()
{
	return m_bForceNoLight;	
}

void ParaEngine::CMeshObject::SearchXRefObject()
{
	if(!m_XRefSearched)
	{
		m_XRefSearched = true;
		CleanupXRefInstances();

		if(m_ppMesh && m_ppMesh->GetMesh())
		{
			int nXRefCount = m_ppMesh->GetMesh()->GetXRefObjectsCount();
			if(nXRefCount>0)
			{
				m_XRefObjects.reserve(nXRefCount);
				for (int i = 0;i<nXRefCount;++i)
				{
					const ParaXRefObject* pXRef = m_ppMesh->GetMesh()->GetXRefObjectsByIndex(i);
					if(pXRef)
					{
						if(pXRef->m_data.type == ParaXRefObject::XREF_ANIMATED)
						{
							// this is an paraX animation object
							ParaXEntity* pAsset = CGlobals::GetAssetManager()->LoadParaX("", pXRef->m_filename);
							XRefObject* pXRefInst = new XRefAnimInstance(this, pAsset, pXRef->m_data.origin, pXRef->m_data.localTransform);
							// tricky: we append to back for all animated objects.
							m_XRefObjects.push_back(pXRefInst);
						}
						else if(pXRef->m_data.type == ParaXRefObject::XREF_STATIC)
						{
							// this is an paraX static mesh object
							MeshEntity* pAsset = CGlobals::GetAssetManager()->LoadMesh("", pXRef->m_filename);
							XRefObject* pXRefInst = new XRefMeshObject(this, pAsset, pXRef->m_data.origin, pXRef->m_data.localTransform);
							// tricky: we insert at front for all mesh objects.
							m_XRefObjects.insert(m_XRefObjects.begin(),  pXRefInst);
						}
					}
				}
			}
		}
	}
}

Matrix4* ParaEngine::CMeshObject::GetAttachmentMatrix( Matrix4& pOut, int nAttachmentID/*=0*/, int nRenderNumber )
{
	SearchXRefObject();
	int nTotal = (int)m_XRefObjects.size();
	for (int i=0;i<nTotal;++i)
	{
		XRefObject* pEntity = m_XRefObjects[i];
		if(pEntity->GetAttachmentMatrix(pOut, nAttachmentID, nRenderNumber))
		{
			Matrix4 attMat;
			pOut *= *(pEntity->GetMatrix(attMat, CGlobals::GetIdentityMatrix()));
			return &pOut;
		}
	}

	return NULL;
}

void ParaEngine::CMeshObject::SetAnimation( int nAnimID )
{
	m_nAnimID = nAnimID;
	SearchXRefObject();
	int nTotal = (int)m_XRefObjects.size();
	for (int i=0;i<nTotal;++i)
	{
		XRefObject* pEntity = m_XRefObjects[i];
		pEntity->SetAnimation(nAnimID);
	}

}

int ParaEngine::CMeshObject::GetAnimation()
{
	SearchXRefObject();
	int nTotal = (int)m_XRefObjects.size();
	for (int i=0;i<nTotal;++i)
	{
		XRefObject* pEntity = m_XRefObjects[i];
		return pEntity->GetAnimation();
	}
	return m_nAnimID;
}

void ParaEngine::CMeshObject::SetAnimFrame(int nFrame)
{
	m_nAnimFrame = nFrame;
	SearchXRefObject();
	int nTotal = (int)m_XRefObjects.size();
	for (int i=0;i<nTotal;++i)
	{
		XRefObject* pEntity = m_XRefObjects[i];
		pEntity->SetAnimFrame(nFrame);
	}
}

int ParaEngine::CMeshObject::GetAnimFrame()
{
	SearchXRefObject();
	int nTotal = (int)m_XRefObjects.size();
	for (int i=0;i<nTotal;++i)
	{
		XRefObject* pEntity = m_XRefObjects[i];
		return pEntity->GetAnimFrame();
	}
	return m_nAnimFrame;
}

void ParaEngine::CMeshObject::SetUseGlobalTime(bool bUseGlobalTime)
{
	m_bUseGlobalTime = bUseGlobalTime;
	SearchXRefObject();
	int nTotal = (int)m_XRefObjects.size();
	for (int i=0;i<nTotal;++i)
	{
		XRefObject* pEntity = m_XRefObjects[i];
		pEntity->SetUseGlobalTime(bUseGlobalTime);
	}
}

bool ParaEngine::CMeshObject::IsUseGlobalTime()
{
	return m_bUseGlobalTime;
}


int ParaEngine::CMeshObject::GetXRefInstanceCount()
{
	SearchXRefObject();
	return (int) m_XRefObjects.size();
}

XRefObject* ParaEngine::CMeshObject::GetXRefInstanceByIndex( int nIndex )
{
	return (nIndex>=0 && nIndex< GetXRefInstanceCount()) ?  (m_XRefObjects[nIndex]):NULL; 
}

int ParaEngine::CMeshObject::GetXRefScriptCount()
{
	int nScriptCount = 0;
	if(m_ppMesh && m_ppMesh->GetMesh())
	{
		int nXRefCount = m_ppMesh->GetMesh()->GetXRefObjectsCount();
		if(nXRefCount>0)
		{
			for (int i = 0;i<nXRefCount;++i)
			{
				const ParaXRefObject* pXRef = m_ppMesh->GetMesh()->GetXRefObjectsByIndex(i);
				if(pXRef && pXRef->m_data.type == ParaXRefObject::XREF_SCRIPT)
				{
					++nScriptCount; 
				}
			}
		}
	}
	return nScriptCount;
}

const char* ParaEngine::CMeshObject::GetXRefScript( int nIndex )
{
	int nScriptCount = 0;
	if(m_ppMesh && m_ppMesh->GetMesh())
	{
		int nXRefCount = m_ppMesh->GetMesh()->GetXRefObjectsCount();
		if(nXRefCount>0)
		{
			for (int i = 0;i<nXRefCount;++i)
			{
				const ParaXRefObject* pXRef = m_ppMesh->GetMesh()->GetXRefObjectsByIndex(i);
				if(pXRef && pXRef->m_data.type == ParaXRefObject::XREF_SCRIPT)
				{
					if(nIndex == nScriptCount)
					{
						return pXRef->m_filename.c_str();
					}
					++nScriptCount; 
				}
			}
		}
	}
	return NULL;
}

Vector3 ParaEngine::CMeshObject::GetXRefScriptPosition(int nIndex)
{
	int nScriptCount = 0;
	if(m_ppMesh && m_ppMesh->GetMesh())
	{
		int nXRefCount = m_ppMesh->GetMesh()->GetXRefObjectsCount();
		if(nXRefCount>0)
		{
			for (int i = 0;i<nXRefCount;++i)
			{
				const ParaXRefObject* pXRef = m_ppMesh->GetMesh()->GetXRefObjectsByIndex(i);
				if(pXRef && pXRef->m_data.type == ParaXRefObject::XREF_SCRIPT)
				{
					if(nIndex == nScriptCount)
					{
						// get the parent's rotation and scaling matrix, here it is some trick to reuse the parent node's code. we actually get its world matrix and then remove the translation part.
						Matrix4 mat;
						GetRenderMatrix(mat);
						mat = pXRef->m_data.localTransform*mat;

						Vector3 vOut;
						ParaVec3TransformCoord(&vOut, &Vector3::ZERO, &mat);

						return (vOut+CGlobals::GetScene()->GetRenderOrigin()); // this gives the position in world space
					}
					++nScriptCount; 
				}
			}
		}
	}
	return Vector3(0,0,0);
}

Vector3 ParaEngine::CMeshObject::GetXRefScriptScaling(int nIndex)
{
	int nScriptCount = 0;
	if(m_ppMesh && m_ppMesh->GetMesh())
	{
		int nXRefCount = m_ppMesh->GetMesh()->GetXRefObjectsCount();
		if(nXRefCount>0)
		{
			for (int i = 0;i<nXRefCount;++i)
			{
				const ParaXRefObject* pXRef = m_ppMesh->GetMesh()->GetXRefObjectsByIndex(i);
				if(pXRef && pXRef->m_data.type == ParaXRefObject::XREF_SCRIPT)
				{
					if(nIndex == nScriptCount)
					{
						// get the parent's rotation and scaling matrix, here it is some trick to reuse the parent node's code. we actually get its world matrix and then remove the translation part.
						Matrix4 mat;
						GetRenderMatrix(mat);
						Vector3 vPos = GetRenderOffset();// get render offset
						mat._41 -= vPos.x;
						mat._42 -= vPos.y;
						mat._43 -= vPos.z;

						// apply the local transform, apply the parent's rotation and scaling matrix, and then offset origin
						mat = pXRef->m_data.localTransform* mat;
						
						Vector3 vOut1, vOut0;
						ParaVec3TransformCoord(&vOut0, &Vector3::ZERO, &mat);
						ParaVec3TransformCoord(&vOut1, &Vector3::UNIT_SCALE, &mat); 

						return (vOut1-vOut0);
					}
					++nScriptCount; 
				}
			}
		}
	}

	return Vector3(1.f,1.f,1.f);
}

float ParaEngine::CMeshObject::GetXRefScriptFacing(int nIndex)
{
	int nScriptCount = 0;
	if(m_ppMesh && m_ppMesh->GetMesh())
	{
		int nXRefCount = m_ppMesh->GetMesh()->GetXRefObjectsCount();
		if(nXRefCount>0)
		{
			for (int i = 0;i<nXRefCount;++i)
			{
				const ParaXRefObject* pXRef = m_ppMesh->GetMesh()->GetXRefObjectsByIndex(i);
				if(pXRef && pXRef->m_data.type == ParaXRefObject::XREF_SCRIPT)
				{
					if(nIndex == nScriptCount)
					{
						// get the parent's rotation and scaling matrix, here it is some trick to reuse the parent node's code. we actually get its world matrix and then remove the translation part.
						Matrix4 mat;
						GetRenderMatrix(mat);
						Vector3 vPos = GetRenderOffset();// get render offset
						mat._41 -= vPos.x;
						mat._42 -= vPos.y;
						mat._43 -= vPos.z;

						// apply the local transform, apply the parent's rotation and scaling matrix, and then offset origin
						mat = pXRef->m_data.localTransform* mat;
						
						Vector3 vOut1, vOut0;
						ParaVec3TransformCoord(&vOut0, &Vector3::ZERO, &mat);
						ParaVec3TransformCoord(&vOut1, &Vector3::UNIT_X, &mat); // suppose it is positive x axis where facing is 0.f
						float fFacing=0.f;
						if(!Math::ComputeFacingTarget(vOut1,vOut0, fFacing))
							fFacing = 0.f;
						return fFacing;
					}
					++nScriptCount; 
				}
			}
		}
	}
	return 0.f;
}

void ParaEngine::CMeshObject::SetPosition(const DVector3& v)
{
	if(m_vPos != v)
	{
		m_vPos = v;
		m_bViewClippingUpdated = false;
	}
}

const char* CMeshObject::GetXRefScriptLocalMatrix( int nIndex )
{
	static char sMatrix[256];
	int nScriptCount = 0;
	if(m_ppMesh && m_ppMesh->GetMesh())
	{
		int nXRefCount = m_ppMesh->GetMesh()->GetXRefObjectsCount();
		if(nXRefCount>0)
		{
			for (int i = 0;i<nXRefCount;++i)
			{
				const ParaXRefObject* pXRef = m_ppMesh->GetMesh()->GetXRefObjectsByIndex(i);
				if(pXRef && pXRef->m_data.type == ParaXRefObject::XREF_SCRIPT)
				{
					if(nIndex == nScriptCount)
					{
						// get the parent's rotation and scaling matrix, here it is some trick to reuse the parent node's code. we actually get its world matrix and then remove the translation part.
						Matrix4 mat;
						GetRenderMatrix(mat);
						Vector3 vPos = GetRenderOffset();// get render offset
						mat._41 -= vPos.x;
						mat._42 -= vPos.y;
						mat._43 -= vPos.z;

						// apply the local transform, apply the parent's rotation and scaling matrix, and then offset origin
						mat = pXRef->m_data.localTransform* mat;

						// scaling and rotation matrix is returned.
						My_snprinf(sMatrix, 256, "%f,%f,%f,%f,%f,%f,%f,%f,%f,0,0,0", mat._11, mat._12,mat._13,mat._21, mat._22,mat._23,mat._31, mat._32,mat._33);
						return sMatrix;
					}
					++nScriptCount; 
				}
			}
		}
	}
	return NULL;
}

int CMeshObject::GetNumReplaceableTextures()
{
	if(m_ppMesh==0 || ! (m_ppMesh->IsValid()))
		return 0;
	CParaXStaticModelRawPtr pMesh = m_ppMesh->GetMesh();
	if(pMesh == NULL)
		return 0;

	return pMesh->GetNumReplaceableTextures();
}

TextureEntity* CMeshObject::GetDefaultReplaceableTexture( int ReplaceableTextureID )
{
	if(m_ppMesh==0 || ! (m_ppMesh->IsValid()))
		return 0;
	CParaXStaticModelRawPtr pMesh = m_ppMesh->GetMesh();
	if(pMesh == NULL)
		return 0;

	return pMesh->GetDefaultReplaceableTexture(ReplaceableTextureID);
}

TextureEntity* CMeshObject::GetReplaceableTexture( int ReplaceableTextureID )
{
	if(m_ppMesh==0 || ! (m_ppMesh->IsValid()))
		return 0;
	CParaXStaticModelRawPtr pMesh = m_ppMesh->GetMesh();
	if(pMesh == NULL)
		return 0;
	if(pMesh->GetReplaceableTexture(ReplaceableTextureID))
	{
		if(ReplaceableTextureID>=0 && (int)m_ReplaceableTextures.size()>ReplaceableTextureID)
			return	m_ReplaceableTextures[ReplaceableTextureID].get();
		else
		{
			// load default one if no one has been set before
			TextureEntity* pNew = GetDefaultReplaceableTexture(ReplaceableTextureID);
			SetReplaceableTexture(ReplaceableTextureID, pNew);
			return pNew;
		}
	}
	else
		return 0;
}

bool CMeshObject::SetReplaceableTexture( int ReplaceableTextureID, TextureEntity* pTextureEntity )
{
	if(ReplaceableTextureID>32 || ReplaceableTextureID<0)
		return false;
	if((int)m_ReplaceableTextures.size()<=ReplaceableTextureID)
		m_ReplaceableTextures.resize(ReplaceableTextureID+1);
	PE_ASSERT((int)m_ReplaceableTextures.size()>ReplaceableTextureID);

	m_ReplaceableTextures[ReplaceableTextureID] = pTextureEntity;
	return true;
}

void CMeshObject::Animate( double dTimeDelta , int nRenderNumber)
{
	//
	// for static and animated x-reference objects
	// 
	int nTotal = GetXRefInstanceCount();
	for (int i=0;i<nTotal;++i)
	{
		XRefObject* pEntity = GetXRefInstanceByIndex(i);
		if(pEntity)
		{
			pEntity->Animate(dTimeDelta, nRenderNumber);
		}
	}
}


bool ParaEngine::CMeshObject::ViewTouch()
{
	if(m_ppMesh==0 || ! (m_ppMesh->IsValid()))
		return false;
	if(GetPrimaryTechniqueHandle()<0)
	{
		// in case the asset is loaded successfully, we shall set the primary asset. 
		SetParamsFromAsset();

		// try loading the asset if it has not been done before. 
		m_ppMesh->LoadAsset();
		return false;
	}
	return true;
}

bool ParaEngine::CMeshObject::IsPersistent()
{
	return m_bIsPersistent;
}

void ParaEngine::CMeshObject::SetPersistent( bool bPersistent )
{
	m_bIsPersistent = bPersistent;
}

void ParaEngine::CMeshObject::SetFaceCullingDisabled( bool bDisableFaceCulling )
{
	m_bDisableFaceCulling = bDisableFaceCulling;
}

bool ParaEngine::CMeshObject::IsFaceCullingDisabled()
{
	return m_bDisableFaceCulling;
}

int ParaEngine::CMeshObject::InstallFields( CAttributeClass* pClass, bool bOverride )
{
	CTileObject::InstallFields(pClass, bOverride);

	pClass->AddField("FaceCullingDisabled", FieldType_Bool, (void*)SetFaceCullingDisabled_s, (void*)IsFaceCullingDisabled_s, NULL, "", bOverride);

	return S_OK;
}

bool ParaEngine::CMeshObject::HasAlphaBlendedObjects()
{
	return (GetPrimaryTechniqueHandle() == TECH_SIMPLE_MESH_NORMAL_CTOR);
}
