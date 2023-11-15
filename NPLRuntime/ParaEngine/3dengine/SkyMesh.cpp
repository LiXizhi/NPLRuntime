//-----------------------------------------------------------------------------
// Class:	CSkyMesh
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.4.14
// 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "MeshObject.h"
#include "SceneObject.h"
#include "AutoCamera.h"
#include "SkyMesh.h"
#include "SunLight.h"
#include "memdebug.h"
#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#endif
using namespace ParaEngine;

namespace ParaEngine
{
	float	CSkyMesh::m_fFogBlendAngleFrom = -0.2f;
	float	CSkyMesh::m_fFogBlendAngleTo = 0.6f;

	const char DEFAULT_SUNSET_TEXTURE[] = "Texture/common/Sunset.dds";
	const char DEFAULT_CLOUD_TEXTURE[] = "Texture/common/cloud.dds";
}
CSkyMesh::CSkyMesh(void)
	:m_fSimsky_cloudThickness(0.5f), m_vSimsky_sunSize(500.f, 12.f), m_vSimsky_moonSize(500.f, 100.f), m_vSimsky_fogColor(0.9f, 0.8f, 0.8f)
{
	// set object type
	SetMyType(_Sky);

	m_typeSky = Sky_StaticMesh;

	SetBoundingBox(1, 1, 1, 0);

	//m_fFogBlendAngleFrom = MATH_PI/20;
	//m_fFogBlendAngleTo = MATH_PI/10;

	m_skycolorFactor = LinearColor(1, 1, 1, 1);
	SetPrimaryTechniqueHandle(TECH_SKY_MESH);

	m_bSimsky_BufferInited = false;
	m_bSimsky_AutoDayTime = true;
#ifdef PARAENGINE_MOBILE
	CreateSkySimulated(8, 16, 16.f, 64.f);
#else
	CreateSkySimulated(16, 32, 16.f, 64.f);
#endif
}

CSkyMesh::~CSkyMesh(void)
{
	m_simsky_vb.ReleaseBuffer();
	m_simsky_ib.ReleaseBuffer();
}

//-----------------------------------------------------------------------------
// Name: Draw()
/// Desc: Render the Sky Box/plane/dome. Always render SkyBox first before render
/// any other object in the scene. So in the scene rendering sky box are rendered 
/// first. This is because, sky box rendering will turn off Z-buffer temporarily. 
/// Note the box is not infinitely large.(smaller than the camera culling), it 
/// just gives the fault illusion of sky in the infinite distance,provided that 
/// vCenter is the camera position. 
//-----------------------------------------------------------------------------
HRESULT CSkyMesh::Draw(SceneState * sceneState)
{
	RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();
	const Matrix4& viewMatrix = CGlobals::GetViewMatrixStack().SafeGetTop();
	Matrix4 InverseViewMatrix;
	InverseViewMatrix = viewMatrix.inverse();
	Vector3 vCenterPos;
	ParaVec3TransformCoord(&vCenterPos, &Vector3::ZERO, &InverseViewMatrix);
	vCenterPos += sceneState->GetScene()->GetRenderOrigin();
	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	CSunLight& sunlight = sceneState->GetScene()->GetSunLight();

	// Render the Skybox/dome/none
	{
		//-- Center view matrix for sky box
		// and set the sun light position if the eye has moved
		if ((Vector3)m_vPos != vCenterPos)
		{
			m_vPos = vCenterPos;
			Vector3 newpos(vCenterPos);
			newpos.y += SUN_HEIGHT;
			sunlight.SetPosition(newpos);
		}
		/**
		* Render the sky box
		*/
		Vector3 vPos = GetPosition();

		switch (m_typeSky)
		{
		case Sky_StaticMesh:
		{
			DrawStaticMeshSky(pEffectManager, sceneState, pRenderDevice, sunlight, vPos);
			break;
		}
		case Sky_Simulated:
		{
			DrawSimulatedSky(pEffectManager, sceneState, sunlight, pRenderDevice, vPos);
			break;
		}
		default:
			break;
		}

		/** draw children */
		if (! GetChildren().empty() )
		{
			if (pEffectManager->BeginEffect(TECH_SKY_MESH, &(sceneState->m_pCurrentEffect)))
			{
				CEffectFile* pEffectFile = sceneState->m_pCurrentEffect;
				if (pEffectFile)
				{
					auto vPos = GetPosition();
					pEffectManager->EnableZWrite(false);
					// all sub mesh model are assumed to be centered at 0,0,0 and with height +/-0.5
					const float fObjectHalfHeight = 0.5f;
					float fFogHeightFrom = tanf(m_fFogBlendAngleFrom) * fObjectHalfHeight;
					float fFogHeightTo = tanf(m_fFogBlendAngleTo) * fObjectHalfHeight;

					Vector4 fogParam(fFogHeightFrom, fFogHeightTo - fFogHeightFrom, 1.0f, 0.1f);
					LinearColor fogColor = sceneState->GetScene()->GetFogColor();
					pEffectFile->applyFogParameters((m_fFogBlendAngleFrom < m_fFogBlendAngleTo) && sceneState->GetScene()->IsFogEnabled(), &fogParam, &fogColor);
					pEffectFile->setParameter(CEffectFile::k_ConstVector0, &GetSkyColorFactor());

					// enable texture wrapping for sub module
					CGlobals::GetRenderDevice()->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
					CGlobals::GetRenderDevice()->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);

					for (auto& child : GetChildren())
					{
						child->SetPosition(vPos);
						if (sceneState->dTimeDelta > 0)
							child->Animate(sceneState->dTimeDelta);
						child->Draw(sceneState);
					}
				}
			}
		}

		//-- Restore the render states
		pEffectManager->EnableZWrite(true);
	}
	return S_OK;
}

//-----------------------------------------------------------------------
void CSkyMesh::CreateSkyVertexColorSimple()
{

}

void CSkyMesh::CreateSkyStaticMesh(const string& strObjectName, AssetEntity& MeshAsset,
	FLOAT fScaleX, FLOAT fScaleY, FLOAT fScaleZ, FLOAT fHeightOffset/* = 0*/)
{
	m_sIdentifer = strObjectName;

	fScaleX = min(10.f, fScaleX);
	fScaleY = min(10.f, fScaleY);
	fScaleZ = min(10.f, fScaleZ);
	SetBoundingBox(fScaleX, fScaleY, fScaleZ, 0);

	SetSkyMesh(MeshAsset, fHeightOffset);
}

void ParaEngine::CSkyMesh::SetSkyMesh(AssetEntity& MeshAsset, float fHeightOffset/* = 0*/)
{
	float fScaleX, fScaleY, fScaleZ, fFacing;
	GetBoundingBox(&fScaleX, &fScaleY, &fScaleZ, &fFacing);

	// create the static mesh
	m_typeSky = Sky_StaticMesh;
	MeshEntity* pMesh = NULL;
	if (MeshAsset.IsValid() && (MeshAsset.GetType() == AssetEntity::mesh))
		pMesh = (MeshEntity*)(&MeshAsset);

	// create the object
	if (pMesh)
	{
		CMeshObject* pObj = new CMeshObject();
		pObj->InitObject(pMesh, NULL, Vector3(0, 0, 0));
		pObj->SetBoundingBox(1, 1, 1, 0);

		m_pStaticMesh = pObj;
		if (m_pStaticMesh->GetPrimaryTechniqueHandle() == TECH_SIMPLE_MESH_NORMAL)
			m_pStaticMesh->SetPrimaryTechniqueHandle(TECH_SKY_MESH);

		/* The following code is needed in case one want to scale it to unit.
		IViewClippingObject * pViewObj =  pObj->GetViewClippingObject();
		float x,y,z,facing;
		pViewObj->GetBoundingBox(&x,&y,&z,&facing);
		*/
		// scale the sky box/dome to the user defined value
		Matrix4	mx;
		// TODO: 10.0f is arbitrary value. there is no need to really render the sky box at the distance.
		ParaMatrixScaling(&m_pStaticMesh->m_mxLocalTransform, fScaleX, fScaleY, fScaleZ);
		mx.makeTrans(0, fHeightOffset, 0);
		ParaMatrixMultiply(&m_pStaticMesh->m_mxLocalTransform, &m_pStaticMesh->m_mxLocalTransform, &mx);
	}
}

void ParaEngine::CSkyMesh::SetSkyMeshFile(const char* filename)
{
	if (m_pStaticMesh && m_pStaticMesh->GetPrimaryAsset() && (m_pStaticMesh->GetPrimaryAsset()->GetKey() == filename))
		return;
	if (filename && filename[0] != '\0')
	{
		MeshEntity* pMesh = CGlobals::GetAssetManager()->LoadMesh("", filename);
		if (pMesh)
		{
			// disable replaceable texture, if the sky mesh file is changed
			SetSkyMeshTexture("");
			SetSkyMesh(*pMesh);
		}
	}
	else
	{
		m_pStaticMesh.reset();
	}
}

const char* ParaEngine::CSkyMesh::GetSkyMeshFile()
{
	static string g_str;

	if (m_pStaticMesh && m_pStaticMesh->GetPrimaryAsset())
		g_str = m_pStaticMesh->GetPrimaryAsset()->GetKey();
	else
		g_str.clear();
	return g_str.c_str();
}

void ParaEngine::CSkyMesh::EnableSimulatedSky(bool bEnable)
{
	if (bEnable)
	{
		SetPrimaryTechniqueHandle(TECH_SKY_DOME);
		m_typeSky = Sky_Simulated;
	}
	else
	{
		SetPrimaryTechniqueHandle(TECH_SKY_MESH);
		m_typeSky = Sky_StaticMesh;
	}
}

HRESULT ParaEngine::CSkyMesh::InitDeviceObjects()
{
	if (IsSimulatedSkyEnabled())
	{
		m_bSimsky_BufferInited = true;

		//////////////////////////////////////////////////////////////////////////
		// create vertex buffer
		RenderDevicePtr pRenderDevice = CGlobals::GetRenderDevice();

		if (m_simsky_vb.CreateBuffer(m_nSimsky_vertexCount*sizeof(mesh_vertex_plain), 0, D3DUSAGE_WRITEONLY))
		{
			mesh_vertex_plain* verts = NULL;
			if (m_simsky_vb.Lock((void**)&verts))
			{
				float deltaH = 2 * MATH_PI / m_nSimsky_horizonSampleCount;
				float deltaV = MATH_PI / 2 / m_nSimsky_verticalSampleCount;

				// from bottom ring to top ring
				for (int i = 0; i < m_nSimsky_verticalSampleCount; i++)
				{
					float horizonR = (m_nSimsky_verticalSampleCount - i)* m_fSimsky_horizonRadius / m_nSimsky_verticalSampleCount;
					float verticalR = m_fSimsky_verticalRadius * (float)sinf(i * deltaV);

					for (int j = 0; j < m_nSimsky_horizonSampleCount; j++)
					{
						int index = m_nSimsky_horizonSampleCount * i + j;

						verts[index].p = Vector3((float)(horizonR * cosf(j * deltaH)),
							verticalR, (float)(horizonR * sinf(j * deltaH)));

						verts[index].uv = Vector2((verts[index].p.x + m_fSimsky_horizonRadius) / (2 * m_fSimsky_horizonRadius),
							(verts[index].p.z + m_fSimsky_horizonRadius) / (2 * m_fSimsky_horizonRadius));

						// OUTPUT_LOG("%d: %f %f %f:%f %f\n", i, verts[index].p.x, verts[index].p.y, verts[index].p.z, verts[index].uv.x, verts[index].uv.y);
					}
				}

				// add the top most point
				verts[m_nSimsky_horizonSampleCount * m_nSimsky_verticalSampleCount].p = Vector3(0, m_fSimsky_verticalRadius, 0);
				verts[m_nSimsky_horizonSampleCount * m_nSimsky_verticalSampleCount].uv = Vector2(0.5f, 0.5f);

				m_simsky_vb.Unlock();
			}
		}
		//////////////////////////////////////////////////////////////////////////
		// create index buffer
		if (m_simsky_ib.CreateIndexBuffer(sizeof(unsigned short)*m_nSimsky_indexCount, D3DFMT_INDEX16, D3DUSAGE_WRITEONLY))
		{
			unsigned short* indices = NULL;
			if (m_simsky_ib.Lock((void**)&indices))
			{
				int index = 0;
				int maxVerticalSample = m_nSimsky_verticalSampleCount - 1;

				// from bottom to top: triangle strips
				for (int i = 0; i < maxVerticalSample; i++)
				{
					for (int j = 0; j < m_nSimsky_horizonSampleCount; j++)
					{
						indices[index++] = i * m_nSimsky_horizonSampleCount + j;
						indices[index++] = (i + 1) * m_nSimsky_horizonSampleCount + j;
					}
					indices[index++] = i * m_nSimsky_horizonSampleCount;
					indices[index++] = (i + 1) * m_nSimsky_horizonSampleCount;
				}

				// the top most ones
				for (int i = 0; i < m_nSimsky_horizonSampleCount; i++)
				{
					indices[index++] = maxVerticalSample * m_nSimsky_horizonSampleCount + i;
					indices[index++] = m_nSimsky_vertexCount - 1;
				}
				indices[index++] = maxVerticalSample * m_nSimsky_horizonSampleCount;

				m_simsky_ib.Unlock();
			}
		}
	}
	return 0;
}

HRESULT ParaEngine::CSkyMesh::DeleteDeviceObjects()
{
	m_bSimsky_BufferInited = false;
	m_simsky_vb.ReleaseBuffer();
	m_simsky_ib.ReleaseBuffer();
	return 0;
}

void ParaEngine::CSkyMesh::UpdateSimSky(float gameTime)
{
	CSunLight& sunlight = CGlobals::GetScene()->GetSunLight();

}

const char* ParaEngine::CSkyMesh::GetSunGlowTexture()
{
	if (m_texSimsky_sunsetGlowTexture)
	{
		return m_texSimsky_sunsetGlowTexture->GetKey().c_str();
	}
	else
	{
		return DEFAULT_SUNSET_TEXTURE;
	}
}

void ParaEngine::CSkyMesh::SetSunGlowTexture(const char* sValue)
{
	if (sValue == NULL)
		return;
	if (m_texSimsky_sunsetGlowTexture)
	{
		if (m_texSimsky_sunsetGlowTexture->GetKey() == sValue)
		{
			return;
		}
	}
	m_texSimsky_sunsetGlowTexture = CGlobals::GetAssetManager()->LoadTexture("", sValue, TextureEntity::StaticTexture);
}

const char* ParaEngine::CSkyMesh::GetCloudTexture()
{
	if (m_texSimsky_cloudTexture)
	{
		return m_texSimsky_cloudTexture->GetKey().c_str();
	}
	else
	{
		return DEFAULT_CLOUD_TEXTURE;
	}
}

void ParaEngine::CSkyMesh::SetCloudTexture(const char* sValue)
{
	if (sValue == NULL)
		return;
	if (m_texSimsky_cloudTexture)
	{
		if (m_texSimsky_cloudTexture->GetKey() == sValue)
		{
			return;
		}
	}
	m_texSimsky_cloudTexture = CGlobals::GetAssetManager()->LoadTexture("", sValue, TextureEntity::StaticTexture);
}

void ParaEngine::CSkyMesh::SetSkyMeshTexture(const char* filename, int nTexIndex)
{
	if (m_pStaticMesh)
	{
		m_pStaticMesh->SetReplaceableTexture(nTexIndex, CGlobals::GetAssetManager()->LoadTexture("", filename, TextureEntity::StaticTexture));
	}
}

const char* ParaEngine::CSkyMesh::GetSkyMeshTexture(int nTexIndex)
{
	if (m_pStaticMesh)
	{
		TextureEntity* pEntity = m_pStaticMesh->GetReplaceableTexture(nTexIndex);
		if (pEntity)
		{
			return pEntity->GetKey().c_str();
		}
	}
	return CGlobals::GetString(0).c_str();
}

HRESULT ParaEngine::CSkyMesh::RendererRecreated()
{
	m_bSimsky_BufferInited = false;
	m_simsky_ib.RendererRecreated();
	m_simsky_vb.RendererRecreated();
	return S_OK;
}

void ParaEngine::CSkyMesh::DrawStaticMeshSky(EffectManager* pEffectManager, SceneState * sceneState, RenderDevicePtr pRenderDevice, CSunLight &sunlight, Vector3 vPos)
{
	if (!m_pStaticMesh)
		return;
	if (!pEffectManager->BeginEffect(TECH_SKY_MESH, &(sceneState->m_pCurrentEffect)))
		return;

	sunlight.SetHasNightHours(false);

	// TODO: remove those line: for testing only
	//this->m_vPos.y = 0.0f;

	CEffectFile* pEffectFile = sceneState->m_pCurrentEffect;
	if (pEffectFile == 0)
	{
#ifdef USE_DIRECTX_RENDERER
		//////////////////////////////////////////////////////////////////////////
		// FF pipeline
		/**
		* Set default sun lighting
		*/
		GETD3D(CGlobals::GetRenderDevice())->SetLight(0, (const D3DLIGHT9*)sunlight.GetD3DLight());
		// turn off light
		//pRenderDevice->LightEnable(0, sceneState->GetScene()->IsLightEnabled() );
		//pRenderDevice->LightEnable(0, false);

		pEffectManager->EnableZWrite(false);

		const Matrix4& matProj = CGlobals::GetProjectionMatrixStack().SafeGetTop();

		// modify the projection matrix, so that the z is always 0.999 (very close to far plane 1.f);
		Matrix4 matProj_one_depth;
		matProj_one_depth = Matrix4::IDENTITY;
		matProj_one_depth._33 = 0.f;
		matProj_one_depth._43 = 0.999f;
		matProj_one_depth = matProj * matProj_one_depth;

		GETD3D(CGlobals::GetRenderDevice())->SetTransform(D3DTS_PROJECTION, matProj_one_depth.GetConstPointer());
		pRenderDevice->SetRenderState(ERenderState::FOGENABLE, FALSE);
		/** render the static mesh background */
		if (m_pStaticMesh)
		{
			m_pStaticMesh->SetPosition(DVector3(vPos));
			m_pStaticMesh->Draw(sceneState);
		}

		/**
		* prepare the scene for fog effects.
		* blend the fog color to the static background.
		* currently, I use triangular strip to build the four sides for the bounding box.
		* the vertex color at its base has alpha 1.0 which is the fog color; and color on its top
		* has alpha 0.0 which will blend with the static sky mesh background.
		* I then use triangular fans to build the bottom, the center vertex is moved downward for some distance
		* to avoid collision with the camera eye.
		* To make things real, I brightened the fog at the far end by some small factor say 1.17. So that reviewer
		* will be able to distinguish the silhouette of a distant object with the sky infinity. In case, a reviewer
		* is looking down from a mountain, the center point of the triangular fan has the exact color as the fog.
		* the front view the blending shape looks like this:\n
		* --------------------- \n
		* |                   | \n
		* ---------vPos-------- \n
		*   \               /   \n
		*        \    /         \n
		*          \/           \n
		*/
		/// TODO: for some reason,  DrawIndexedPrimitiveUP (the bottom triangle fans) will crash on VIA/S3G UniChrome Pro IGP, so I turned it off completely. 
		if (false && sceneState->GetScene()->IsFogEnabled())
		{
			/// with CCW winding
			static const short pIndexBufferSides[] = {
				// triangle strip for the four sides
				0, 4, 1, 5, 2, 6, 3, 7, 0, 4,
				// triangle fan for the bottom side
				8, 0, 1, 2, 3, 0,
			};

			LINEVERTEX pVertices[9];
			float fRadius = GetRadius() / 2;
			float fFogHeightFrom = sinf(m_fFogBlendAngleFrom) * fRadius;
			float fFogHeightTo = sinf(m_fFogBlendAngleTo) * fRadius;


			pVertices[8].p.x = 0;
			pVertices[8].p.y = -fRadius / 2;
			pVertices[8].p.z = 0;

			pVertices[0].p.x = fRadius;
			pVertices[0].p.y = fFogHeightFrom;
			pVertices[0].p.z = fRadius;

			pVertices[1].p.x = fRadius;
			pVertices[1].p.y = fFogHeightFrom;
			pVertices[1].p.z = -fRadius;

			pVertices[2].p.x = -fRadius;
			pVertices[2].p.y = fFogHeightFrom;
			pVertices[2].p.z = -fRadius;

			pVertices[3].p.x = -fRadius;
			pVertices[3].p.y = fFogHeightFrom;
			pVertices[3].p.z = fRadius;

			pVertices[4].p.x = fRadius;
			pVertices[4].p.y = fFogHeightTo;
			pVertices[4].p.z = fRadius;

			pVertices[5].p.x = fRadius;
			pVertices[5].p.y = fFogHeightTo;
			pVertices[5].p.z = -fRadius;

			pVertices[6].p.x = -fRadius;
			pVertices[6].p.y = fFogHeightTo;
			pVertices[6].p.z = -fRadius;

			pVertices[7].p.x = -fRadius;
			pVertices[7].p.y = fFogHeightTo;
			pVertices[7].p.z = fRadius;


			LinearColor color = sceneState->GetScene()->GetFogColor();
			// fan center color is the fog color
			pVertices[8].color = color;
			// brightening the color a little bit
			color *= 1.17f;
			for (int i = 0; i < 4; i++)
			{
				pVertices[i].color = color;
			}
			// the top is completely transparent
			color.a = 0;
			for (int i = 4; i < 8; i++)
			{
				pVertices[i].color = color;
			}
			/// blend the fog color with the back ground image
			//pEffectManager->SetCullingMode(true);
			pRenderDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
			pRenderDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pRenderDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);

			// use depth bias
			float depthBias = -0.00001f;
			pRenderDevice->SetRenderState(ERenderState::DEPTHBIAS, *(DWORD*)&depthBias);

			GETD3D(CGlobals::GetRenderDevice())->SetTexture(0, NULL);

			// set translation
			Matrix4 mxWorld;
			Vector3 vPos = GetRenderOffset();
			mxWorld.makeTrans(vPos.x, vPos.y, vPos.z);
			GETD3D(CGlobals::GetRenderDevice())->SetTransform(D3DTS_WORLD, mxWorld.GetConstPointer());

			GETD3D(CGlobals::GetRenderDevice())->SetVertexShader(NULL);
			GETD3D(CGlobals::GetRenderDevice())->SetPixelShader(NULL);
			GETD3D(CGlobals::GetRenderDevice())->SetFVF(LINEVERTEX::FVF);

			CGlobals::GetRenderDevice()->DrawIndexedPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 0,
				8, 8, pIndexBufferSides, PixelFormat::INDEX16, pVertices, sizeof(LINEVERTEX));

			pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);

			// this line crashes on VIA/S3G UniChrome Pro IGP
			//DirectXPerf::DrawIndexedPrimitiveUP(pRenderDevice, DirectXPerf::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLEFAN, 0, 
			//	5, 4, pIndexBufferSides+10, D3DFMT_INDEX16,pVertices, sizeof(LINEVERTEX));

			float fTemp = 0.0f;
			pRenderDevice->SetRenderState(ERenderState::DEPTHBIAS, *(DWORD*)&fTemp);
		}
		pRenderDevice->SetRenderState(ERenderState::FOGENABLE, sceneState->GetScene()->IsFogEnabled());
		GETD3D(CGlobals::GetRenderDevice())->SetTransform(D3DTS_PROJECTION, matProj.GetConstPointer());
#endif
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// programmable pipeline

		/** render the static mesh background*/
		if (m_pStaticMesh)
		{
			// just for some interesting effects.
			float fMaxY = m_pStaticMesh->m_ppMesh->GetAABBMax().y;
			float fMinY = m_pStaticMesh->m_ppMesh->GetAABBMin().y;

			// this ensure that sky box is 4 meters so that it stays in z buffer. 
			const float SKY_BOX_WIDTH = 4.f;
			float fScaling = SKY_BOX_WIDTH / (((fMaxY - fMinY) > 0.1f) ? (fMaxY - fMinY) : SKY_BOX_WIDTH);
			if (fScaling != 1.f)
			{
				// if sky mesh size is smaller than or equal to camera near plane. we need to scale up the mesh a little 
				// to prevent it being culled by camera near plane. 
				Matrix4 mat, mat1;
				ParaMatrixScaling(&mat1, fScaling, fScaling, fScaling);
				ParaMatrixMultiply(&mat, &mat1, &CGlobals::GetWorldMatrixStack().SafeGetTop());
				CGlobals::GetWorldMatrixStack().push(mat);

				//fMaxY *= fScaling;
				//fMinY *= fScaling;
			}
			float fObjectHeight = fMaxY - fMinY;
			float fObjectHalfHeight = fObjectHeight / 2.f;

			// disable z buffer
			pEffectManager->EnableZWrite(false);


			float fFogHeightFrom = tanf(m_fFogBlendAngleFrom) * fObjectHalfHeight;
			float fFogHeightTo = tanf(m_fFogBlendAngleTo) * fObjectHalfHeight;

			Vector4 fogParam((fMaxY + fMinY) / 2.f + fFogHeightFrom, fFogHeightTo - fFogHeightFrom, 1.0f, 0.1f);
			LinearColor fogColor = sceneState->GetScene()->GetFogColor();
			pEffectFile->applyFogParameters((m_fFogBlendAngleFrom < m_fFogBlendAngleTo) && sceneState->GetScene()->IsFogEnabled(), &fogParam, &fogColor);

			pEffectFile->setParameter(CEffectFile::k_ConstVector0, &GetSkyColorFactor());
			m_pStaticMesh->SetPosition(DVector3(vPos));
			DrawStaticMeshSkyInner(sceneState);


			if (fScaling != 1.f)
			{
				CGlobals::GetWorldMatrixStack().pop();
			}
		}
	}
}

void ParaEngine::CSkyMesh::DrawSimulatedSky(EffectManager* pEffectManager, SceneState * sceneState, CSunLight &sunlight, RenderDevicePtr pRenderDevice, Vector3 &vPos)
{
	if (!m_bSimsky_BufferInited)
	{
		InitDeviceObjects();
	}
	if (!m_simsky_vb.IsValid() || !m_simsky_ib.IsValid())
		return;
	if (!pEffectManager->BeginEffect(TECH_SKY_DOME, &(sceneState->m_pCurrentEffect)))
		return;

	sunlight.SetHasNightHours(true);

	CEffectFile* pEffectFile = sceneState->m_pCurrentEffect;
	if (pEffectFile != 0)
	{
		float fSunAngle = sunlight.GetCelestialAngle();
		float fSunAngleRadian = fSunAngle * Math::PI *2.0f;

		if (GetIsAutoDayTime())
		{
			// animate the parameters automatically according to current day time. 
			if (Math::Cos(fSunAngleRadian) < 0.f)
			{
				//switch to night shader
				m_bSimsky_isDaytime = false;
				m_vSimsky_lightSkyColor = Vector3(0.2f, 0.2f, 0.4f);
				m_vSimsky_darkSkyColor = Vector3(0.0f, 0.1f, 0.2f);
				m_vSimsky_cloudColor = CGlobals::GetScene()->GetFogColor(fSunAngle);
			}
			else
			{
				//switch to day shader
				m_bSimsky_isDaytime = true;
				m_vSimsky_lightSkyColor = Vector3(0.627f, 0.831f, 1.f);
				m_vSimsky_darkSkyColor = Vector3(0, 0.25f, 0.45f);
				m_vSimsky_cloudColor = CGlobals::GetScene()->GetFogColor(fSunAngle); //  Vector3(1.0f, 1.0f, 1.0f);
				// LXZ 2010.2.2: we use sky color for cloud color in simulated sky shader
				m_vSimsky_cloudColor.x = GetSkyColorFactor().r;
				m_vSimsky_cloudColor.y = GetSkyColorFactor().g;
				m_vSimsky_cloudColor.z = GetSkyColorFactor().b;
			}
		}
		else
		{
			// LXZ 2010.2.2: we use sky color for cloud color in simulated sky shader
			m_vSimsky_cloudColor.x = GetSkyColorFactor().r;
			m_vSimsky_cloudColor.y = GetSkyColorFactor().g;
			m_vSimsky_cloudColor.z = GetSkyColorFactor().b;
		}
		Vector3 vCloudColor = m_vSimsky_cloudColor;
		float fSunsetGlowWeight = 0.f;
		{
			LinearColor* sunColor = sunlight.CalcSunriseSunsetColors(fSunAngle);
			if (sunColor)
			{
				fSunsetGlowWeight = Math::Abs(sunColor->a);
			}
		}

		Vector3 vSunVector = sunlight.GetSunDirection();
		//update cloud position
		Vector2 cloudSpeed = m_vSimsky_cloudDirection;
		cloudSpeed.x *= m_vSimsky_cloudVelocity.x;
		cloudSpeed.y *= m_vSimsky_cloudVelocity.y;
		m_vSimsky_cloudOffset += cloudSpeed * (float)(sceneState->dTimeDelta);
		m_vSimsky_cloudOffset.x = m_vSimsky_cloudOffset.x - floor(m_vSimsky_cloudOffset.x);
		m_vSimsky_cloudOffset.y = m_vSimsky_cloudOffset.y - floor(m_vSimsky_cloudOffset.y);


		vPos = GetRenderOffset();
		Matrix4 mxWorld;
		mxWorld.makeTrans(vPos.x, vPos.y, vPos.z);
		CGlobals::GetWorldMatrixStack().push(mxWorld);

		/** infinite projection matrix
		http://www.terathon.com/gdc07_lengyel.pdf


		*/

		//////////////////////////////////////////////////////////////////////////
		// programmable pipeline
		pEffectManager->EnableZWrite(false);
		CGlobals::GetRenderDevice()->SetStreamSource(0, m_simsky_vb.GetDevicePointer(), 0, sizeof(mesh_vertex_plain));
		CGlobals::GetRenderDevice()->SetIndices(m_simsky_ib.GetDevicePointer());
		CGlobals::GetRenderDevice()->SetRenderState(ERenderState::COLORWRITEENABLE, 0x7);

		if (pEffectFile->begin(false))
		{
			if (m_texSimsky_sunsetGlowTexture == 0)
				m_texSimsky_sunsetGlowTexture = CGlobals::GetAssetManager()->LoadTexture("", DEFAULT_SUNSET_TEXTURE, TextureEntity::StaticTexture);
			if (m_texSimsky_cloudTexture == 0)
				m_texSimsky_cloudTexture = CGlobals::GetAssetManager()->LoadTexture("", DEFAULT_CLOUD_TEXTURE, TextureEntity::StaticTexture);
			int nRenderPass = m_bSimsky_isDaytime ? 0 : 1;

			if (pEffectFile->BeginPass(nRenderPass))
			{
				// manually apply camera matrices mostly for opengl, since secondary pass does not share uniform with first one. 
				pEffectFile->applyGlobalLightingData(sunlight);
				pEffectFile->applyCameraMatrices();
				pEffectFile->SetVector3("lightSkyColor", m_vSimsky_lightSkyColor);
				pEffectFile->SetVector3("darkSkyColor", m_vSimsky_darkSkyColor);
				pEffectFile->SetVector2("sunIntensity", m_vSimsky_sunIntensity);
				pEffectFile->SetVector3("sunColor", m_vSimsky_sunColor);
				pEffectFile->SetFloat("sunsetGlowWeight", fSunsetGlowWeight);
				pEffectFile->SetVector3("cloudColor", vCloudColor);
				// pEffectFile->SetVector3("sunDirection", vSunVector);
				pEffectFile->SetVector2("cloudOffset", m_vSimsky_cloudOffset);
				pEffectFile->SetFloat("cloudThickness", m_fSimsky_cloudThickness);// between 0,1
				pEffectFile->SetVector2("sunSize", m_vSimsky_sunSize);
				pEffectFile->SetVector2("moonSize", m_vSimsky_moonSize);
				//blend cloud
				pEffectFile->SetFloat("cloudBlendWeight", Math::Abs(Math::Sin(fSunAngleRadian*2.5f)));
				// sky vertical height
				pEffectFile->SetFloat("skyDomeHeight", m_fSimsky_verticalRadius);

				// textures
				pEffectFile->setTexture(0, m_texSimsky_sunsetGlowTexture.get());
				pEffectFile->setTexture(1, m_texSimsky_cloudTexture.get());

				// we don't want to render completely transparent parts
				pEffectFile->CommitChanges();
				CGlobals::GetRenderDevice()->DrawIndexedPrimitive( EPrimitiveType::TRIANGLESTRIP, 0, 0, m_nSimsky_vertexCount, 0, m_nSimsky_primitiveCount);
				pEffectFile->EndPass();
			}
			pEffectFile->end();
		}
		CGlobals::GetRenderDevice()->SetIndices(0);
		CGlobals::GetRenderDevice()->SetTexture(1, 0);
		CGlobals::GetRenderDevice()->SetRenderState(ERenderState::COLORWRITEENABLE, 0xF);
		CGlobals::GetWorldMatrixStack().pop();
	}
}

void ParaEngine::CSkyMesh::DrawStaticMeshSkyInner(SceneState * sceneState)
{
	if (m_pStaticMesh)
	{
		sceneState->SetCurrentSceneObject(m_pStaticMesh.get());
		SetFrameNumber(sceneState->m_nRenderCount);

		// get world transform matrix
		Matrix4 mxWorld;
		m_pStaticMesh->GetRenderMatrix(mxWorld);
		m_pStaticMesh->DrawInner(sceneState, &mxWorld, sceneState->GetCameraToCurObjectDistance());
	}
}

// 16, 32, 16.f, 64.f
void CSkyMesh::CreateSkySimulated(int verticalSampleCount, int horizonSampleCount, float verticalRadius, float horizonRadius)
{
	m_vSimsky_position = Vector3(0, 0, 0);

	m_fSimsky_verticalRadius = verticalRadius;
	m_fSimsky_horizonRadius = horizonRadius;
	m_nSimsky_verticalSampleCount = verticalSampleCount;
	m_nSimsky_horizonSampleCount = horizonSampleCount;

	m_nSimsky_vertexCount = m_nSimsky_horizonSampleCount * m_nSimsky_verticalSampleCount + 1;
	m_nSimsky_indexCount = (m_nSimsky_verticalSampleCount - 1)*(m_nSimsky_horizonSampleCount * 2 + 2) + m_nSimsky_horizonSampleCount * 2 + 1;
	m_nSimsky_primitiveCount = m_nSimsky_indexCount - 2;

	m_vSimsky_cloudDirection = Vector2(0, 1.f);
	m_vSimsky_cloudVelocity = Vector2(0, 0.002f);
	m_vSimsky_cloudOffset = Vector2(0, 0);

	m_bSimsky_isDaytime = true;
	m_vSimsky_lightSkyColor = Vector3(0.627f, 0.831f, 1.f);
	m_vSimsky_darkSkyColor = Vector3(0, 0.25f, 0.45f);
	m_vSimsky_cloudColor = Vector3(1.0f, 1.0f, 1.0f);
	m_vSimsky_sunIntensity = Vector2(1, 0.3f);
	m_vSimsky_sunColor = Vector3(1, 1, 0.6f);
}

int CSkyMesh::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CTileObject::InstallFields(pClass, bOverride);

	pClass->AddField("SkyMeshFile", FieldType_String, (void*)SetSkyMeshFile_s, (void*)GetSkyMeshFile_s, NULL, NULL, bOverride);
	pClass->AddField("SkyMeshTexture", FieldType_String, (void*)SetSkyMeshTexture_s, (void*)GetSkyMeshTexture_s, NULL, NULL, bOverride);
	pClass->AddField("SkyColor", FieldType_Vector3, (void*)SetSkyColorFactor_s, (void*)GetSkyColorFactor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), "", bOverride);
	pClass->AddField("SkyFogAngleFrom", FieldType_Float, (void*)SetFogBlendAngleFrom_s, (void*)GetFogBlendAngleFrom_s, NULL, NULL, bOverride);
	pClass->AddField("SkyFogAngleTo", FieldType_Float, (void*)SetFogBlendAngleTo_s, (void*)GetFogBlendAngleTo_s, NULL, NULL, bOverride);

	// for simulated sky parameters.
	pClass->AddField("SimulatedSky", FieldType_Bool, (void*)EnableSimulatedSky_s, (void*)IsSimulatedSkyEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("IsAutoDayTime", FieldType_Bool, (void*)SetIsAutoDayTime_s, (void*)GetIsAutoDayTime_s, NULL, NULL, bOverride);
	pClass->AddField("SunGlowTexture", FieldType_String, (void*)SetSunGlowTexture_s, (void*)GetSunGlowTexture_s, NULL, NULL, bOverride);
	pClass->AddField("CloudTexture", FieldType_String, (void*)SetCloudTexture_s, (void*)GetCloudTexture_s, NULL, NULL, bOverride);
	pClass->AddField("SunColor", FieldType_Vector3, (void*)SetSunColor_s, (void*)GetSunColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), "", bOverride);
	pClass->AddField("LightSkyColor", FieldType_Vector3, (void*)SetLightSkyColor_s, (void*)GetLightSkyColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), "", bOverride);
	pClass->AddField("DarkSkyColor", FieldType_Vector3, (void*)SetDarkSkyColor_s, (void*)GetDarkSkyColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), "", bOverride);
	pClass->AddField("SunIntensity", FieldType_Vector2, (void*)SetSunIntensity_s, (void*)GetSunIntensity_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), "", bOverride);
	pClass->AddField("CloudColor", FieldType_Vector3, (void*)SetCloudColor_s, (void*)GetCloudColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), "", bOverride);
	pClass->AddField("CloudVelocity", FieldType_Vector2, (void*)SetCloudVelocity_s, (void*)GetCloudVelocity_s, NULL, NULL, bOverride);
	pClass->AddField("CloudOffset", FieldType_Vector2, (void*)SetCloudOffset_s, (void*)GetCloudOffset_s, NULL, NULL, bOverride);
	pClass->AddField("CloudThickness", FieldType_Float, (void*)SetCloudThickness_s, (void*)GetCloudThickness_s, NULL, NULL, bOverride);
	pClass->AddField("SunSize", FieldType_Vector2, (void*)SetSunSize_s, (void*)GetSunSize_s, NULL, NULL, bOverride);
	pClass->AddField("MoonSize", FieldType_Vector2, (void*)SetMoonSize_s, (void*)GetMoonSize_s, NULL, NULL, bOverride);
	pClass->AddField("FogColor", FieldType_Vector3, (void*)SetFogColor_s, (void*)GetFogColor_s, NULL, NULL, bOverride);
	return S_OK;
}

