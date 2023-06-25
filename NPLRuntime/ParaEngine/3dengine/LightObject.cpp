//-----------------------------------------------------------------------------
// Class: CLightObject
// Authors:	LiXizhi, devilwalk
// Emails:	LiXizhi@yeah.net
// Date:	2006.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneState.h"
#include "SceneObject.h"
#include "LightParam.h"
#include "BlockEngine/BlockWorldClient.h"
#include "LightManager.h"
#include "LightObject.h"
#include "LightGeoUtil.h"

using namespace ParaEngine;

/**@def whether to automatically adjust light parameters by range. */
#define AUTO_LIGHT_PARAMS_BY_RANGE

CLightObject::CLightObject(void)
	:m_bDeleteLightParams(true), m_pLightParams(NULL), m_bAutoSetAttenuation(true), m_bRotationDirty(false)
{

	m_pLightParams = new CLightParam();

	m_pLightParams->MakeRedPointLight();
	//m_pLightParams->MakeRedSpotLight();
	//m_pLightParams->MakeRedDirectionalLight();

	m_mxLocalTransform = Matrix4::IDENTITY;
	SetMyType(_LocalLight);
	SetShadowCaster(false);
}

CLightObject::~CLightObject(void)
{
	if (m_bDeleteLightParams)
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
	string sScript;
	char line[MAX_LINE + 1];

	Matrix4 mat;
	GetLocalTransform(&mat);
	Vector3 vPos = GetObjectCenter();
	const char* sLightParams = "";
	if (m_pLightParams != 0)
	{
		sLightParams = m_pLightParams->ToString();
	}
	if (SUCCEEDED(snprintf(line, MAX_LINE, "player = ParaScene.CreateLightObject(\"\", %f,%f,%f, \"%s\",\"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\");\n",
		vPos.x, vPos.y, vPos.z, sLightParams,
		mat._11, mat._12, mat._13, mat._21, mat._22, mat._23, mat._31, mat._32, mat._33, mat._41, mat._42, mat._43)))
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
	// get position in the render coordinate system
	Vector3 vPos = GetRenderOffset();
	// render at the center
	vPos.y += GetHeight() / 2;

	if (m_pLightParams != 0)
	{
		m_pLightParams->Position = vPos;
	}
	return m_pLightParams;
}

void CLightObject::SetLocalTransform(const Matrix4& mXForm)
{
	m_mxLocalTransform = mXForm;
	m_bRotationDirty = true;
}

void CLightObject::SetLocalTransform(float fScale, float fRotX, float fRotY, float fRotZ)
{
	Matrix4 mx;
	ParaMatrixScaling(&m_mxLocalTransform, fScale, fScale, fScale);
	ParaMatrixRotationZ(&mx, fRotZ);
	m_mxLocalTransform = m_mxLocalTransform * mx;
	ParaMatrixRotationX(&mx, fRotX);
	m_mxLocalTransform = m_mxLocalTransform * mx;
	ParaMatrixRotationY(&mx, fRotY);
	m_mxLocalTransform = m_mxLocalTransform * mx;
	m_bRotationDirty = true;
}

void CLightObject::SetLocalTransform(float fScale, const Quaternion& quat)
{
	Matrix4 mx;
	ParaMatrixScaling(&m_mxLocalTransform, fScale, fScale, fScale);
	quat.ToRotationMatrix(mx, Vector3::ZERO);
	m_mxLocalTransform = m_mxLocalTransform * mx;
	m_bRotationDirty = true;
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
	if (x != 0.f)
		mat = (*ParaMatrixRotationX(&mat1, x))*mat;
	if (y != 0.f)
		mat = (*ParaMatrixRotationY(&mat1, y))*mat;
	if (z != 0.f)
		mat = (*ParaMatrixRotationZ(&mat1, z))*mat;

	SetLocalTransform(mat);
}

void CLightObject::SetScaling(float s)
{
	Matrix4 mat;
	GetLocalTransform(&mat);
	Matrix4 mat1;
	ParaMatrixScaling(&mat1, s, s, s);
	mat = mat1 * mat;
	SetLocalTransform(mat);
}

void CLightObject::Reset()
{
	SetLocalTransform(*CGlobals::GetIdentityMatrix());
}

HRESULT CLightObject::InitObject(CLightParam* pLight, MeshEntity* ppMesh, const Vector3& vCenter, const Matrix4& mat, bool bCopyParams)
{
	// set position
	SetObjectCenter(vCenter);

	if (m_bDeleteLightParams)
		SAFE_DELETE(m_pLightParams);

	if (pLight == NULL)
	{
		m_pLightParams = new CLightParam();
		m_pLightParams->MakeRedPointLight();
		m_bDeleteLightParams = true;
	}
	else
	{
		if (bCopyParams) {
			m_pLightParams = new CLightParam();
			*m_pLightParams = *pLight;
			m_bDeleteLightParams = true;
		}
		else {
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
	if (m_pLightParams != 0)
	{
		m_pLightParams->Type = (ELightType)nType;
	}
}

int CLightObject::GetLightType()
{
	return (int)((m_pLightParams != 0) ? m_pLightParams->Type : ELightType::Point);
}

void CLightObject::SetDiffuse(const LinearColor& color)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Diffuse = color;
	}
}

const LinearColor& CLightObject::GetDiffuse()
{
	static const LinearColor g_default = { 1, 1, 1, 1 };
	return (m_pLightParams != 0) ? m_pLightParams->Diffuse : g_default;
}

void CLightObject::SetSpecular(const LinearColor& color)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Specular = color;
	}
}

const LinearColor& CLightObject::GetSpecular()
{
	static const LinearColor g_default = { 1, 1, 1, 1 };
	return (m_pLightParams != 0) ? m_pLightParams->Specular : g_default;
}

void CLightObject::SetAmbient(const LinearColor& color)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Ambient = color;
	}
}

const LinearColor& CLightObject::GetAmbient()
{
	static const LinearColor g_default = { 1, 1, 1, 1 };
	return (m_pLightParams != 0) ? m_pLightParams->Ambient : g_default;
}

void CLightObject::SetDirection(const Vector3& dir)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Direction = dir;
	}
}

const Vector3& CLightObject::GetDirection()
{
	static const Vector3 g_default = { 1, 1, 1 };

	if (IsRotationDirty()) {
		m_pLightParams->RecalculateDirection(&m_mxLocalTransform);
		SetRotationDirty(false);
	}

	return (m_pLightParams != 0) ? m_pLightParams->Direction : g_default;
}

void CLightObject::SetYaw(float yaw)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Yaw = yaw;
		m_bRotationDirty = true;
	}
}

float CLightObject::GetYaw()
{
	return (m_pLightParams != 0) ? m_pLightParams->Yaw : 0.f;
}

void CLightObject::SetPitch(float pitch)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Pitch = pitch;
		m_bRotationDirty = true;
	}
}

float CLightObject::GetPitch()
{
	return (m_pLightParams != 0) ? m_pLightParams->Pitch : 0.f;
}

void CLightObject::SetRoll(float roll)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Roll = roll;
		m_bRotationDirty = true;
	}
}

float CLightObject::GetRoll()
{
	return (m_pLightParams != 0) ? m_pLightParams->Roll : 0.f;
}

void CLightObject::SetRange(float range)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Range = range;
	}

	AutoSetAttenation();
}

float CLightObject::GetRange()
{
	return (m_pLightParams != 0) ? m_pLightParams->Range : 0.f;
}

void CLightObject::SetFalloff(float falloff)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Falloff = falloff;
	}
}

float CLightObject::GetFalloff()
{
	return (m_pLightParams != 0) ? m_pLightParams->Falloff : 0.f;
}

void CLightObject::SetAttenuation0(float Attenuation0)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Attenuation0 = Attenuation0;
	}
}

float CLightObject::GetAttenuation0()
{
	return (m_pLightParams != 0) ? m_pLightParams->Attenuation0 : 0.f;
}

void CLightObject::SetAttenuation1(float Attenuation1)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Attenuation1 = Attenuation1;
	}
}

float CLightObject::GetAttenuation1()
{
	return (m_pLightParams != 0) ? m_pLightParams->Attenuation1 : 0.f;
}

void CLightObject::SetAttenuation2(float Attenuation2)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Attenuation2 = Attenuation2;
	}
}

float CLightObject::GetAttenuation2()
{
	return (m_pLightParams != 0) ? m_pLightParams->Attenuation2 : 0.f;
}

void CLightObject::SetTheta(float theta)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Theta = theta;
	}
}

float CLightObject::GetTheta()
{
	return (m_pLightParams != 0) ? m_pLightParams->Theta : 0.f;
}

void CLightObject::SetPhi(float phi)
{
	if (m_pLightParams != 0)
	{
		m_pLightParams->Phi = phi;
	}
}

float CLightObject::GetPhi()
{
	return (m_pLightParams != 0) ? m_pLightParams->Phi : 0.f;
}

int ParaEngine::CLightObject::GetPrimaryTechniqueHandle()
{
	switch (m_pLightParams->Type) {
	case ELightType::Point:
		return TECH_LIGHT_POINT;
	case ELightType::Spot:
		return TECH_LIGHT_SPOT;
	case ELightType::Directional:
		return TECH_LIGHT_DIRECTIONAL;
	}
	return TECH_LIGHT_POINT;
}

int ParaEngine::CLightObject::PrepareRender(CBaseCamera* pCamera, SceneState * sceneState)
{
	if (sceneState->GetScene()->PrepareRenderObject(this, pCamera, *sceneState))
	{
#ifdef USE_DIRECTX_RENDERER
		if (IsDeferredLightOnly())
		{
			sceneState->AddToDeferredLightPool(this);
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
			if (pModel->GetAttachmentMatrix(pOut, nAttachmentID, m_CurrentAnim, AnimIndex(), 0.f, m_CurrentAnim, AnimIndex(), 0.f))
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

void ParaEngine::CLightObject::RenderDeferredLightMesh(SceneState * sceneState)
{
	if (sceneState->IsShadowPass())
		return;

#ifdef USE_DIRECTX_RENDERER
	sceneState->SetCurrentSceneObject(this);
	SetFrameNumber(sceneState->m_nRenderCount);

	// get world transform matrix
	Matrix4 mxWorld;
	GetRenderMatrix(mxWorld);
	mxWorld = m_mxLocalTransform * mxWorld;
	CGlobals::GetWorldMatrixStack().push(mxWorld);

	struct LightVertex
	{
	public:
		LightVertex() :normal(1, 0, 0), color(0xffffffff) {};
		Vector3 position;  //4byte
		Vector3 normal;  //4byte
		DWORD color;	//4byte;
	};
	std::vector<LightVertex> m_Vertices;
	m_Vertices.resize(8);
	m_Vertices[0].position = Vector3(0, 1, 0);
	m_Vertices[1].position = Vector3(0, 1, 1);
	m_Vertices[2].position = Vector3(1, 1, 1);
	m_Vertices[3].position = Vector3(1, 1, 0);
	uint16 m_indices[6] = { 0,1,2,   0,2,3 };

	LightVertex* vb_vertices = NULL;
	LightVertex *ov = NULL;
	LightVertex *m_origVertices = &(m_Vertices[0]);

	DynamicVertexBufferEntity* pBufEntity = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_XYZ_NORM_DIF);

	auto pDevice = sceneState->GetRenderDevice();
	pDevice->SetStreamSource(0, pBufEntity->GetBuffer(), 0, sizeof(LightVertex));

	int nNumLockedVertice;
	int nNumFinishedVertice = 0;
	int indexCount = (int)m_Vertices.size();
	do
	{
		if ((nNumLockedVertice = pBufEntity->Lock(indexCount - nNumFinishedVertice, (void**)(&vb_vertices))) > 0)
		{
			int nLockedNum = nNumLockedVertice / 3;

			int nIndexOffset = nNumFinishedVertice;
			for (int i = 0; i < nLockedNum; ++i)
			{
				int nVB = 3 * i;
				for (int k = 0; k < 3; ++k, ++nVB)
				{
					int a = m_indices[nIndexOffset + nVB];
					LightVertex& out_vertex = vb_vertices[nVB];
					// weighted vertex
					ov = m_origVertices + a;
					out_vertex.position = ov->position;
					out_vertex.normal = ov->normal;
					out_vertex.color = 0xffffffff;
				}
			}
			pBufEntity->Unlock();

			if (pBufEntity->IsMemoryBuffer())
				//RenderDevice::DrawPrimitiveUP(pDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, D3DPT_TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);
				CGlobals::GetRenderDevice()->DrawPrimitiveUP(EPrimitiveType::TRIANGLELIST, nLockedNum, pBufEntity->GetBaseVertexPointer(), pBufEntity->m_nUnitSize);

			else
				//RenderDevice::DrawPrimitive(pDevice, RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER, D3DPT_TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);
				CGlobals::GetRenderDevice()->DrawPrimitive(EPrimitiveType::TRIANGLELIST, pBufEntity->GetBaseVertex(), nLockedNum);
		}
		if ((indexCount - nNumFinishedVertice) > nNumLockedVertice)
		{
			nNumFinishedVertice += nNumLockedVertice;
		}
		else
			break;

	} while (1);


	CGlobals::GetWorldMatrixStack().pop();
#endif
}


void ParaEngine::CLightObject::AutoSetAttenation()
{
	/*
	 * att = 1 / (a0 + a1*r + a2*r*r)
	 *
	 * if att at the maximum range edge is 0.1, then
	 *    a0 + a1*r + a2*r*r = 1 / att = 10
	 *
	 * we prefer a0,a1,a2 acting in range [0, 1],
	 * so we use a greedy way to calculate the a0, a1, a2
	 */
	if (m_bAutoSetAttenuation) {
		float edge_att = 0.1f;
		float one_over_att = 1 / edge_att;
		float range = m_pLightParams->Range;

		SetAttenuation0(1.0f);
		one_over_att = one_over_att - 1;

		if (range > one_over_att) {
			SetAttenuation1(one_over_att / range);
			one_over_att = 0;
		}
		else {
			SetAttenuation1(1.0f);
			one_over_att = one_over_att - range;
		}

		SetAttenuation2(one_over_att / (range * range));
		one_over_att = 0;
	}
}

bool ParaEngine::CLightObject::IsDeferredLightOnly() const
{
	return m_bDeferredLightOnly;
}

void ParaEngine::CLightObject::SetDeferredLightOnly(bool val)
{
	m_bDeferredLightOnly = val;
}


bool ParaEngine::CLightObject::IsRotationDirty() const
{
	return m_bRotationDirty;
}

void ParaEngine::CLightObject::SetRotationDirty(bool val)
{
	m_bRotationDirty = val;
}

int CLightObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CSphereObject::InstallFields(pClass, bOverride);

	pClass->AddField("LightType", FieldType_Int, (void*)SetLightType_s, (void*)GetLightType_s, NULL, NULL, bOverride);

	pClass->AddField("Diffuse", FieldType_Vector3, (void*)SetDiffuse_s, (void*)GetDiffuse_s, CAttributeField::GetSimpleSchemaOfRGB(), NULL, bOverride);
	pClass->AddField("Specular", FieldType_Vector3, (void*)SetSpecular_s, (void*)GetSpecular_s, CAttributeField::GetSimpleSchemaOfRGB(), NULL, bOverride);
	pClass->AddField("Ambient", FieldType_Vector3, (void*)SetAmbient_s, (void*)GetAmbient_s, CAttributeField::GetSimpleSchemaOfRGB(), NULL, bOverride);

	pClass->AddField("Position", FieldType_DVector3, (void*)SetPosition_s, (void*)GetPosition_s, NULL, NULL, bOverride);
	pClass->AddField("Direction", FieldType_Vector3, (void*)SetDirection_s, (void*)GetDirection_s, NULL, NULL, bOverride);
	pClass->AddField("Yaw", FieldType_Float, (void*)SetYaw_s, (void*)GetYaw_s, NULL, NULL, bOverride);
	pClass->AddField("Pitch", FieldType_Float, (void*)SetPitch_s, (void*)GetPitch_s, NULL, NULL, bOverride);
	pClass->AddField("Roll", FieldType_Float, (void*)SetRoll_s, (void*)GetRoll_s, NULL, NULL, bOverride);

	pClass->AddField("Range", FieldType_Float, (void*)SetRange_s, (void*)GetRange_s, NULL, NULL, bOverride);
	pClass->AddField("Falloff", FieldType_Float, (void*)SetFalloff_s, (void*)GetFalloff_s, NULL, NULL, bOverride);

	pClass->AddField("Attenuation0", FieldType_Float, (void*)SetAttenuation0_s, (void*)GetAttenuation0_s, NULL, NULL, bOverride);
	pClass->AddField("Attenuation1", FieldType_Float, (void*)SetAttenuation1_s, (void*)GetAttenuation1_s, NULL, NULL, bOverride);
	pClass->AddField("Attenuation2", FieldType_Float, (void*)SetAttenuation2_s, (void*)GetAttenuation2_s, NULL, NULL, bOverride);

	pClass->AddField("Theta", FieldType_Float, (void*)SetTheta_s, (void*)GetTheta_s, NULL, NULL, bOverride);
	pClass->AddField("Phi", FieldType_Float, (void*)SetPhi_s, (void*)GetPhi_s, NULL, NULL, bOverride);

	pClass->AddField("IsDeferredLightOnly", FieldType_Bool, (void*)SetDeferredLightOnly_s, (void*)IsDeferredLightOnly_s, NULL, NULL, bOverride);
	return S_OK;
}
