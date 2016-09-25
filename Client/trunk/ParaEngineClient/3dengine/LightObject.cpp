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
#include "StaticMesh.h"
#include "LightObject.h"

using namespace ParaEngine;

/**@def whether to automatically adjust light parameters by range. */
#define AUTO_LIGHT_PARAMS_BY_RANGE

/** this is the mesh path for the mesh asset CGlobals::GetAssetManager()->GetMesh("_default_light_mesh");*/
const char g_default_light_mesh_path[] = "model/pops/lightball/lightball.x";

CLightObject::CLightObject(void)
:m_bDeleteLightParams(true), m_pLightParams(NULL)
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
	// set mesh
	if(ppMesh == NULL)
	{
		ppMesh = CGlobals::GetAssetManager()->GetMesh("_default_light_mesh");
		if(ppMesh==0 || !ppMesh->IsValid())
		{
			ppMesh = CGlobals::GetAssetManager()->LoadMesh("_default_light_mesh", g_default_light_mesh_path);
		}
	}
	m_ppMesh = ppMesh;
	
	if(ppMesh!=0)
	{
		// use a special shader without light. 
		SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL_UNLIT);
	}
	
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

HRESULT CLightObject::Draw( SceneState * sceneState)
{
	if(CGlobals::GetScene()->IsShowLocalLightMesh())
	{
		if(m_ppMesh==NULL || ! (m_ppMesh->IsValid()))
			return E_FAIL;

		RenderDevicePtr pd3dDevice = sceneState->GetRenderDevice();

		// world translation
		Vector3 vPos = GetRenderOffset();
		// render at the center
		vPos.y+=GetHeight()/2;

		Matrix4 mxWorld = m_mxLocalTransform;
		mxWorld._41 += vPos.x;
		mxWorld._42 += vPos.y;
		mxWorld._43 += vPos.z;

		CParaXStaticMesh* pMesh = m_ppMesh->GetMesh();
		if(pMesh == NULL)
			return E_FAIL;

		//CGlobals::GetEffectManager()->applyObjectLocalLighting(this);

		CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if ( pEffectFile == 0)
		{
			//////////////////////////////////////////////////////////////////////////
			// fixed programming pipeline
			CGlobals::GetWorldMatrixStack().push(mxWorld);

			// render by default as non-transparent.
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			CGlobals::GetEffectManager()->SetCullingMode(true);

			// Draw with mesh materials
			pMesh->Render(sceneState, pd3dDevice, true, true,sceneState->fAlphaFactor);
			CGlobals::GetWorldMatrixStack().pop();
		}
		else
		{
			//////////////////////////////////////////////////////////////////////////
			// draw using effect file
			CGlobals::GetWorldMatrixStack().push(mxWorld);
			pMesh->Render(sceneState, pEffectFile, true, true,sceneState->fAlphaFactor);
			CGlobals::GetWorldMatrixStack().pop();
		}
	}
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

void CLightObject::SetDiffuse(const D3DCOLORVALUE& color)
{
	if(m_pLightParams!=0)
	{
		m_pLightParams->Diffuse = color;
	}
}

const D3DCOLORVALUE& CLightObject::GetDiffuse()
{
	static const D3DCOLORVALUE g_default = {1,1,1,1};
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

int CLightObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CSphereObject::InstallFields(pClass, bOverride);

	pClass->AddField("LightType",FieldType_Int, SetLightType_s, GetLightType_s, NULL, NULL, bOverride);
	pClass->AddField("Range",FieldType_Float, SetRange_s, GetRange_s, CAttributeField::GetSimpleSchemaOfFloat(1.f,30.f), NULL, bOverride);
	pClass->AddField("Diffuse",FieldType_Vector3, SetDiffuse_s, GetDiffuse_s, CAttributeField::GetSimpleSchemaOfRGB(), NULL, bOverride);
	pClass->AddField("Attenuation0",FieldType_Float, SetAttenuation0_s, GetAttenuation0_s, NULL, NULL, bOverride);
	pClass->AddField("Attenuation1",FieldType_Float, SetAttenuation1_s, GetAttenuation1_s, NULL, NULL, bOverride);
	pClass->AddField("Attenuation2",FieldType_Float, SetAttenuation2_s, GetAttenuation2_s, NULL, NULL, bOverride);

	return S_OK;
}

AssetEntity* ParaEngine::CLightObject::GetPrimaryAsset()
{
	return (AssetEntity*)(m_ppMesh.get());
}
