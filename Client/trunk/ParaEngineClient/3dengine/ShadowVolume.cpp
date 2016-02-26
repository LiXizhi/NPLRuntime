//-----------------------------------------------------------------------------
// Class:	ShadowVolume
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AutoCamera.h"

#include "BaseObject.h"
#include "ShadowVolume.h"
#include "SceneState.h"
#include "memdebug.h"
using namespace ParaEngine;


float ShadowVolume::m_fMinShadowCastDistance = MIN_SHADOW_CAST_DISTANCE;

ShadowVolume::ShadowVolume(void)
{
	m_nVertexArraySize = INITIAL_SHADOWVOLUME_SIZE;
	m_pVertices = new Vector3[m_nVertexArraySize];
	Reset();
}

ShadowVolume::~ShadowVolume(void)
{
	if(m_pVertices)
		delete [] m_pVertices;
}

void ShadowVolume::BuildFromShadowCaster( ShadowCaster* pCaster, LightParams* pLight)
{
	pCaster->BuildShadowVolume(CGlobals::GetSceneState(), this, pLight);
}

void ShadowVolume::ReserveNewBlock(Vector3** pVertices, int nSize)
{
	if((m_nNumVertices+nSize)<m_nVertexArraySize)
	{
		/// we have enough space for the new block.
		*pVertices = m_pVertices+m_nNumVertices;
	}
	else if(m_pVertices!=NULL)
	{
		m_nVertexArraySize += INITIAL_SHADOWVOLUME_SIZE;
		Vector3* tmp = new Vector3[m_nVertexArraySize];
		memcpy(tmp, m_pVertices, sizeof(Vector3)*(m_nVertexArraySize-INITIAL_SHADOWVOLUME_SIZE));
		delete [] m_pVertices;
		m_pVertices = tmp;
		ReserveNewBlock(pVertices, nSize);
	}
}

void ShadowVolume::CommitBlock(int nSize)
{
	m_nNumVertices+=nSize;
}

HRESULT ShadowVolume::Render( SceneState * sceneState )
{
#ifdef USE_DIRECTX_RENDERER
	sceneState->m_pd3dDevice->SetFVF( D3DFVF_XYZ );
	
    return RenderDevice::DrawPrimitiveUP( sceneState->m_pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLELIST, m_nNumVertices/3, m_pVertices, sizeof(Vector3) );
#else
	return E_FAIL;
#endif
}
/// render to stencil with Z Pass
HRESULT ShadowVolume::RenderZPassShadow( SceneState * sceneState )
{
	return S_OK;
}
/// render to stencil with Z Fail
HRESULT ShadowVolume::RenderZFailShadow( SceneState * sceneState )
{
	return S_OK;
}

bool ShadowVolume::PointsInsideOcclusionPyramid(Vector3* pVecBounds, int nCount)
{
	bool bInside = true;
	for( int iPlane = 0; iPlane < 5; iPlane++ )
	{
		int iPoint;
		for( iPoint = 0; iPoint < nCount; iPoint++ )
		{
			if( occlusionPyramid[iPlane].PlaneDotCoord(pVecBounds[iPoint]) < 0)
			{
				// the point is inside the plane
				break;
			}
		}
		if( iPoint>= nCount)
		{
			bInside = false;
			break;
		}
	}
	return bInside;
}

void ShadowVolume::ReCalculateOcclusionPyramid(CAutoCamera* pCamera)
{
	Vector3 ur = pCamera->GetShadowFrustum()->vecFrustum[3];
	Vector3 lr = pCamera->GetShadowFrustum()->vecFrustum[1];
	Vector3 ll = pCamera->GetShadowFrustum()->vecFrustum[0];
	Vector3 ul = pCamera->GetShadowFrustum()->vecFrustum[2];
	 
	if(m_pLightParam->bIsDirectional)
	{
		Vector3 vLight = m_pLightParam->Direction;
		vLight *= m_pLightParam->Range;

		occlusionPyramid[0].redefine(ul, ur, lr); // botttom face
		occlusionPyramid[1].redefine((ur - vLight), ur, ul);
		occlusionPyramid[2].redefine((lr - vLight), lr, ur);
		occlusionPyramid[3].redefine((ll - vLight), ll, lr);
		occlusionPyramid[4].redefine((ul - vLight), ul, ll);

		if ((m_pLightParam->Direction).dotProduct(pCamera->GetWorldAhead())> 0) 
		{
			// light is behind us, flip all occlusion planes
			for (int i = 0; i < 5; ++i) {
				occlusionPyramid[i] = -occlusionPyramid[i];
			}
		}
	}
}

void ShadowVolume::UpdateProjectionInfo(const ParaViewport* pViewport,
		const Matrix4 *pProj,
		const Matrix4 *pView)
{
	m_viewport = *pViewport;
	m_matProjection = *pProj;
	m_matView = *pView;
}

void ShadowVolume::ProjectPoint(Vector3 *pOut, const Vector3 *pV, const Matrix4 *pWorld)
{
	ParaVec3Project(pOut, pV, &m_viewport, &m_matProjection, &m_matView, pWorld);
}