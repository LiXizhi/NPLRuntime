//-----------------------------------------------------------------------------
// Class: CLightObject
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneState.h"
#include "SceneObject.h"
#include "LightParam.h"
#include "LightManager.h"
#include "LightObject.h"

using namespace ParaEngine;

/**@def whether to automatically adjust light parameters by range. */
#define AUTO_LIGHT_PARAMS_BY_RANGE

CLightObject::CLightObject(void)
	:m_bDeleteLightParams(true), m_pLightParams(NULL), m_fLightAmbientBrightness(0.3f)
{
	SetMyType(_LocalLight);
	m_mxLocalTransform = Matrix4::IDENTITY;
	SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL_UNLIT);
}

CLightObject::~CLightObject(void)
{
	if(m_bDeleteLightParams)
		SAFE_DELETE(m_pLightParams);
}

void CLightObject::Cleanup()
{
		
}

std::string CLightObject::ToString(DWORD nMethod)
{
	#ifndef MAX_LINE
	#define MAX_LINE	500
	#endif
	/** -- sample script generated:
	player = ParaScene.CreateLightObject("", 1,1,1, "sLightParams", "matrix...");
	sceneLoader:AddChild(player);
	*/
	string sScript;
	char line[MAX_LINE+1];
	
	Matrix4 mat;
	GetLocalTransform(&mat);
	Vector3 vPos = GetObjectCenter();
	const char* sLightParams="";
	if(m_pLightParams!=0)
	{
		sLightParams = m_pLightParams->ToString();
	}
	if(SUCCEEDED(snprintf(line, MAX_LINE, "player = ParaScene.CreateLightObject(\"\", %f,%f,%f, \"%s\",\"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\");\n", 
		vPos.x, vPos.y, vPos.z, sLightParams, 
		mat._11, mat._12, mat._13, mat._21, mat._22, mat._23,mat._31, mat._32, mat._33,mat._41, mat._42, mat._43)))
	{
		sScript.append(line);
		sScript.append("ParaScene.Attach(player);\n");
	}
	else
	{
		OUTPUT_LOG("error writing light objects.\r\n");
	}
	return sScript;
}

CLightParam* CLightObject::GetLightParams()
{
	// get position in the render coordinate sytem
	Vector3 vPos = GetRenderOffset();
	// render at the center
	vPos.y+=GetHeight()/2;

	if(m_pLightParams!=0)
	{
		m_pLightParams->Position = vPos;
	}
	return m_pLightParams;
}

void CLightObject::SetLocalTransform(const Matrix4& mXForm)
{
	m_mxLocalTransform = mXForm;
}
void CLightObject::SetLocalTransform(float fScale, float fRotX, float fRotY, float fRotZ)
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
void CLightObject::SetLocalTransform(float fScale, const Quaternion& quat)
{
	Matrix4 mx;
	ParaMatrixScaling(&m_mxLocalTransform, fScale, fScale, fScale);
	quat.ToRotationMatrix(mx, Vector3::ZERO);
	m_mxLocalTransform = m_mxLocalTransform*mx; 
}
void CLightObject::GetLocalTransform(Matrix4* localTransform)
{
	*localTransform = m_mxLocalTransform;
}

void CLightObject::Rotate(float x, float y, float z)
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

void CLightObject::SetScaling(float s)
{
	Matrix4 mat;
	GetLocalTransform(&mat);
	Matrix4 mat1;
	ParaMatrixScaling(&mat1, s, s, s);
	mat = mat1*mat;
	SetLocalTransform(mat);
}

void CLightObject::Reset()
{
	SetLocalTransform(*CGlobals::GetIdentityMatrix());
}

HRESULT CLightObject::InitObject(CLightParam* pLight, MeshEntity* ppMesh, const Vector3& vCenter,  const Matrix4& mat,  bool bCopyParams)
{
	// set position
	SetObjectCenter(vCenter);

	if(m_bDeleteLightParams)
		SAFE_DELETE(m_pLightParams);

	if(pLight==NULL)
	{
		m_pLightParams = new CLightParam();
		m_pLightParams->MakeWhitePointLight();
		m_bDeleteLightParams = true;
	}
	else
	{
		if(bCopyParams){
			m_pLightParams = new CLightParam();
			*m_pLightParams = *pLight;
			m_bDeleteLightParams = true;
		}
		else{
			m_pLightParams = pLight;
			m_bDeleteLightParams = false;
		}
	}

	// set the radius, this is for view clipping object calculation and scene attachment . 
	// TODO: we should over write the view clipping object to calculate the light region's bounding box. 
	SetRadius(m_pLightParams->Range);
	return S_OK;
}

void CLightObject::SetLightType(int nType)
{
	if(m_pLightParams!=0)
	{
		m_pLightParams->Type = (D3DLIGHTTYPE)nType;
	}
}

int CLightObject::GetLightType()
{
	return (int)((m_pLightParams!=0) ? m_pLightParams->Type: D3DLIGHT_POINT);
}

void CLightObject::SetDiffuse(const LinearColor& color)
{
	if(m_pLightParams!=0)
	{
		m_pLightParams->Diffuse = color;
	}
}

const LinearColor& CLightObject::GetDiffuse()
{
	static const LinearColor g_default = { 1, 1, 1, 1 };
	return (m_pLightParams!=0) ? m_pLightParams->Diffuse: g_default;
}

void CLightObject::SetRange(float range)
{
	if(m_pLightParams!=0)
	{
		m_pLightParams->Range = range;

#ifdef AUTO_LIGHT_PARAMS_BY_RANGE

		/** set the attenuation parameters accordingly. */
		float a0, a1,a2;
		if(range>1.f)
		{
			a0 = 1.f/range;
			a1 = 0.5f/range;
			a2 = 5/(range*range);
		}
		else
		{
			a0 = 1.f;
			a1 = 0.5f;
			a2 = 5.f;
		}
		
		SetAttenuation0(a0);
		SetAttenuation1(a1);
		SetAttenuation2(a2);
#endif
	}
}

float CLightObject::GetRange()
{
	return (m_pLightParams!=0) ? m_pLightParams->Range: 0.f;
}

void CLightObject::SetAttenuation0(float Attenuation0)
{
	if(m_pLightParams!=0)
	{
		m_pLightParams->Attenuation0 = Attenuation0;
	}
}

float CLightObject::GetAttenuation0()
{
	return (m_pLightParams!=0) ? m_pLightParams->Attenuation0: 0.f;
}

void CLightObject::SetAttenuation1(float Attenuation1)
{
	if(m_pLightParams!=0)
	{
		m_pLightParams->Attenuation1 = Attenuation1;
	}
}

float CLightObject::GetAttenuation1()
{
	return (m_pLightParams!=0) ? m_pLightParams->Attenuation1: 0.f;
}

void CLightObject::SetAttenuation2(float Attenuation2)
{
	if(m_pLightParams!=0)
	{
		m_pLightParams->Attenuation2 = Attenuation2;
	}
}

float CLightObject::GetAttenuation2()
{
	return (m_pLightParams!=0) ? m_pLightParams->Attenuation2: 0.f;
}

int ParaEngine::CLightObject::PrepareRender(CBaseCamera* pCamera, SceneState * sceneState)
{
	if (sceneState->GetScene()->PrepareRenderObject(this, pCamera, *sceneState))
	{
#ifdef USE_DIRECTX_RENDERER
		// add local light to global forward pipeline light manager
		if (IsDeferredLightOnly())
		{
			if (sceneState->IsDeferredShading())
			{
				// TODO: add to deferred shading pipeline for rendering in sceneState.
				// sceneState->AddTpDeferredLightPool
			}
		}
		else
		{
			CGlobals::GetLightManager()->RegisterLight(GetLightParams());
		}
#endif
	}
	return 0;
}

HRESULT CLightObject::Draw(SceneState * sceneState)
{
	// if(CGlobals::GetScene()->IsShowLocalLightMesh())
	if (IsDeferredLightOnly() && sceneState->IsDeferredShading())
	{
		// deferred shading
		RenderMesh(sceneState);
	}
	else if( !IsDeferredLightOnly() )
	{
		RenderMesh(sceneState);
	}
	return S_OK;
}

HRESULT ParaEngine::CLightObject::RenderMesh(SceneState * sceneState)
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

	if (pEffectFile == 0)
	{
		// TODO: Fixed Function. 
	}
	else
	{
		// apply block space lighting for object whose size is comparable to a single block size
		if (CheckAttribute(MESH_USE_LIGHT))
		{
			float fLightness = m_fLightAmbientBrightness;
			sceneState->GetLocalMaterial().Ambient = (LinearColor(fLightness*0.7f, fLightness*0.7f, fLightness*0.7f, 1.f));
			sceneState->GetLocalMaterial().Diffuse = (LinearColor(fLightness*0.4f, fLightness*0.4f, fLightness*0.4f, 1.f));
			sceneState->EnableLocalMaterial(true);
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

AssetEntity* ParaEngine::CLightObject::GetPrimaryAsset()
{
	return (m_pAnimatedMesh.get());
}

void ParaEngine::CLightObject::SetAssetFileName(const std::string& sFilename)
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

Matrix4* ParaEngine::CLightObject::GetAttachmentMatrix(Matrix4& matOut, int nAttachmentID /*= 0*/, int nRenderNumber /*= 0*/)
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

Matrix4* ParaEngine::CLightObject::GetRenderMatrix(Matrix4& out, int nRenderNumber /*= 0*/)
{
	// world translation
	Vector3 vPos = GetRenderOffset();
	// render at the center
	vPos.y += GetHeight() / 2;

	out = m_mxLocalTransform;
	out._41 += vPos.x;
	out._42 += vPos.y;
	out._43 += vPos.z;
	return &out;
}

bool ParaEngine::CLightObject::IsDeferredLightOnly() const
{
	return m_bDeferredLightOnly;
}

void ParaEngine::CLightObject::SetDeferredLightOnly(bool val)
{
	m_bDeferredLightOnly = val;
}


int CLightObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CSphereObject::InstallFields(pClass, bOverride);

	pClass->AddField("LightType", FieldType_Int, SetLightType_s, GetLightType_s, NULL, NULL, bOverride);
	pClass->AddField("Range", FieldType_Float, SetRange_s, GetRange_s, CAttributeField::GetSimpleSchemaOfFloat(1.f, 30.f), NULL, bOverride);
	pClass->AddField("Diffuse", FieldType_Vector3, SetDiffuse_s, GetDiffuse_s, CAttributeField::GetSimpleSchemaOfRGB(), NULL, bOverride);
	pClass->AddField("Attenuation0", FieldType_Float, SetAttenuation0_s, GetAttenuation0_s, NULL, NULL, bOverride);
	pClass->AddField("Attenuation1", FieldType_Float, SetAttenuation1_s, GetAttenuation1_s, NULL, NULL, bOverride);
	pClass->AddField("Attenuation2", FieldType_Float, SetAttenuation2_s, GetAttenuation2_s, NULL, NULL, bOverride);
	pClass->AddField("IsDeferredLightOnly", FieldType_Bool, SetDeferredLightOnly_s, IsDeferredLightOnly_s, NULL, NULL, bOverride);
	return S_OK;
}
