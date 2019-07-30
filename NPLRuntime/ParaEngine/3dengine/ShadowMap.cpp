//-----------------------------------------------------------------------------
// Class:	CShadowMap
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.4
// Note: partially based on nvidia sdk sample:PracticalPSM 2005
/* Rwv
*/
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#endif
#if defined(USE_DIRECTX_RENDERER) || defined(USE_OPENGL_RENDERER)

#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#endif
#include <float.h>
#include "ShapeFrustum.h"

#include "ShapeAABB.h"
#include "ShapeCone.h"
#include "terrain/GlobalTerrain.h"
#include "EffectManager.h"
#include "RayCollider.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "SunLight.h"
#include "effect_file.h"
#include "ParaWorldAsset.h"
#include "BlockEngine/BlockWorldClient.h"
#include "ViewportManager.h"
#include "ShadowMap.h"



using namespace ParaEngine;

/** @def define this if one wants only one bounding box as the terrain's shadow receiver. Otherwise multiple boxes are used.*/
//#define SINGLE_TERRAIN_BOUNDINGBOX

/** NVidia demo recommends 1536 at year 2005*/
const int TEXDEPTH_HEIGHT_20 = 2048;
const int TEXDEPTH_WIDTH_20 = 2048;
const int TEXDEPTH_SIZE_11 = 1024;
const float W_EPSILON = 0.001f;

#define DW_AS_FLT(DW) (*(FLOAT*)&(DW))
#define FLT_AS_DW(F) (*(DWORD*)&(F))
#define FLT_SIGN(F) ((FLT_AS_DW(F) & 0x80000000L))
#define ALMOST_ZERO(F) ((FLT_AS_DW(F) & 0x7f800000L)==0)
#define IS_SPECIAL(F)  ((FLT_AS_DW(F) & 0x7f800000L)==0x7f800000L)
namespace ParaEngine
{
	int CShadowMap::g_nShadowMapTexelSizeLevel = 0;
}

CShadowMap::CShadowMap(void)
{
#ifdef USE_DIRECTX_RENDERER
	m_pBackBuffer = NULL;
	m_pZBuffer = NULL;
	m_pSMColorSurface = NULL;
	m_pSMZSurface = NULL;
	m_pSMColorSurfaceBlurredHorizontal = NULL;
	m_pSMColorSurfaceBlurredVertical = NULL;
#else
	mOldFrameBufferObject = 0;
	mSMDepthStencilBufferObject = 0;
	mSMFrameBufferObject = 0;
#endif
	m_lightDir = Vector3(0.f, 1.f, 0.f);
	m_bUnitCubeClip = true;
	m_bSlideBack = true;
	m_bitDepth = 24;
#ifdef _DEBUG
	m_bDisplayShadowMap = true;
#else
	m_bDisplayShadowMap = false;
#endif
	m_bBlurSMColorTexture = true; // default value is true
	m_iDepthBias = 4;
	m_fLSPSM_NoptWeight = 1.f;
	m_fBiasSlope = 1.0f;
	m_fSlideBack = 0.f;
	m_fMinInfinityZ = 1.5f;
	m_fTSM_Delta = 0.52f;

	m_shadowTexWidth = (g_nShadowMapTexelSizeLevel>0) ? TEXDEPTH_WIDTH_20 : TEXDEPTH_SIZE_11;
	m_shadowTexHeight = (g_nShadowMapTexelSizeLevel>0) ? TEXDEPTH_HEIGHT_20 : TEXDEPTH_SIZE_11;
	
	//m_iShadowType = (int)SHADOWTYPE_LSPSM; // by LXZ, stable in 2007.2 : there are still special cases that does not work well, such as when camera is looking along the light direction.
	m_iShadowType = (int)SHADOWTYPE_ORTHO; // by LXZ, stable in 2007.2 
	//m_iShadowType = (int)SHADOWTYPE_TSM;  // by LXZ, stable.
	//m_iShadowType = (int)SHADOWTYPE_PSM;  // working

	m_World = Matrix4::IDENTITY;
	m_View = Matrix4::IDENTITY;
	m_Projection = Matrix4::IDENTITY;
}

CShadowMap::~CShadowMap(void)
{
}
#ifdef USE_DIRECTX_RENDERER
HRESULT CShadowMap::CheckResourceFormatSupport(D3DFORMAT fmt, D3DRESOURCETYPE resType, DWORD dwUsage)
{
	//HRESULT hr = S_OK;
	//IDirect3D9* tempD3D = NULL;
	//auto pRenderDevice = CGlobals::GetRenderDevice();
	//
	//pRenderDevice->GetDirect3D(&tempD3D);
	//const D3DCAPS9& devCaps = CGlobals::GetDirectXEngine().m_d3dCaps;
	//
	//D3DDISPLAYMODE displayMode;
	//tempD3D->GetAdapterDisplayMode(devCaps.AdapterOrdinal, &displayMode);

	//hr = tempD3D->CheckDeviceFormat(devCaps.AdapterOrdinal, devCaps.DeviceType, displayMode.Format, dwUsage, resType, fmt);

	//tempD3D->Release(), tempD3D = NULL;

	return S_OK;
}
#endif
int CShadowMap::GetShadowMapTexelSize()
{
	return m_shadowTexWidth;
}

void CShadowMap::SetShadowMapTexelSizeLevel(int nLevel)
{
	int shadowTexWidth = (nLevel>0) ? TEXDEPTH_WIDTH_20 : TEXDEPTH_SIZE_11;
	int shadowTexHeight = (nLevel>0) ? TEXDEPTH_HEIGHT_20 : TEXDEPTH_SIZE_11;
	SetShadowMapTexelSize(shadowTexWidth, shadowTexHeight);
}

void CShadowMap::SetShadowMapTexelSize(int nWidth, int nHeight)
{
	if(m_shadowTexWidth!=nWidth || m_shadowTexHeight!=nHeight)
	{
		m_shadowTexWidth = nWidth;
		m_shadowTexHeight = nHeight;
		if(m_pSMColorTexture!=0
#ifdef USE_DIRECTX_RENDERER
			|| m_pSMZSurface!=0
#endif
			)
		{
			InvalidateDeviceObjects();
			RestoreDeviceObjects();
		}
	}
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//  The device exists, but was just lost or reset, and is now being restored.  
//  Resources in D3DPOOL_DEFAULT and any other device state that persists during 
//  rendering should be set here.  Render states, matrices, textures, etc., that 
//  don't change during rendering can be set once here to avoid redundant state 
//  setting during Render(). 
//-----------------------------------------------------------------------------
HRESULT CShadowMap::RestoreDeviceObjects()
{
	HRESULT hr=S_OK;
	return S_OK;
}

bool CShadowMap::PrepareAllSurfaces()
{
#ifdef USE_DIRECTX_RENDERER
	if (m_pSMColorSurface)
#else
	if (mSMFrameBufferObject)
#endif
		return true;
#ifdef USE_DIRECTX_RENDERER
	auto pRenderDevice = CGlobals::GetRenderDevice();
	

	//  hardware shadow maps are enabled by creating a texture with a depth format (D16, D24X8, D24S8),
	//  with usage DEPTHSTENCIL set.
	//  set this texture as the depth/stencil buffer when rendering the shadow map, and as a texture
	//  when performing the shadow comparison.

	D3DFORMAT zFormat = D3DFMT_D24S8;
	m_bitDepth = 24;

	if(FAILED(CheckResourceFormatSupport(zFormat, D3DRTYPE_TEXTURE, D3DUSAGE_DEPTHSTENCIL)))
	{
		OUTPUT_LOG("warning: Device/driver does not support hardware shadow maps, using R32F.\r\n");
		m_bSupportsHWShadowMaps = false;
	}
	else
		m_bSupportsHWShadowMaps = true;

#define USE_F32_SHADOWMAP
#ifdef USE_F32_SHADOWMAP
	m_bSupportsHWShadowMaps = false;
#endif

	const D3DCAPS9& deviceCaps = CGlobals::GetDirectXEngine().m_d3dCaps;


	D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;
#else
	m_bSupportsHWShadowMaps = false;
#endif
	TextureEntity::TextureInfo tex_info;
	tex_info.m_width = m_shadowTexWidth;
	tex_info.m_height = m_shadowTexHeight;

	if (m_bSupportsHWShadowMaps)
	{
#ifdef USE_DIRECTX_RENDERER
		if (m_pSMColorTexture == 0)
		{
			m_pSMColorTexture = CGlobals::GetAssetManager()->LoadTexture("_SMColorTexture_R32F", "_SMColorTexture_R32F", TextureEntity::RenderTarget);
			m_pSMColorTexture->SetTextureInfo(tex_info);
			m_pSMColorTexture->LoadAsset();
			if(!m_pSMColorTexture->IsValid())
				return false;
		}
		SAFE_RELEASE(m_pSMColorSurface);
		m_pSMColorTexture->GetTexture()->GetSurfaceLevel(0, &m_pSMColorSurface);

		if(m_pSMZTexture == 0)
		{
			m_pSMZTexture = CGlobals::GetAssetManager()->LoadTexture("_SMZTexture", "_SMZTexture", TextureEntity::DEPTHSTENCIL);
			m_pSMZTexture->SetTextureInfo(tex_info);
			m_pSMZTexture->LoadAsset();
			if(!m_pSMZTexture->IsValid())
				return false;
		}
		SAFE_RELEASE(m_pSMZSurface);
		m_pSMZTexture->GetTexture()->GetSurfaceLevel(0, &m_pSMZSurface);
#endif
	}
	else
	{
		//  use R32F & shaders instead of depth textures
		if(m_pSMColorTexture == 0)
		{
			m_pSMColorTexture = CGlobals::GetAssetManager()->LoadTexture("_SMColorTexture_R32F", "_SMColorTexture_R32F", TextureEntity::RenderTarget);
			m_pSMColorTexture->SetTextureInfo(tex_info);
			m_pSMColorTexture->LoadAsset();
			if(!m_pSMColorTexture->IsValid())
				return false;
		}
#ifdef USE_DIRECTX_RENDERER
		SAFE_RELEASE(m_pSMColorSurface);
		m_pSMColorTexture->GetTexture()->GetSurfaceLevel(0, &m_pSMColorSurface);

		if(FAILED(GETD3D(CGlobals::GetRenderDevice())->CreateDepthStencilSurface(m_shadowTexWidth, m_shadowTexHeight, zFormat,
			D3DMULTISAMPLE_NONE, 0, FALSE, &m_pSMZSurface, NULL)))
			return false;

		if(!m_pSMColorSurface )
			return false;
#else
		glDeleteFramebuffers(1, &mSMFrameBufferObject);
		glDeleteRenderbuffers(1, &mSMDepthStencilBufferObject);
		glGenFramebuffers(1, &mSMFrameBufferObject);
		glBindFramebuffer(GL_FRAMEBUFFER, mSMFrameBufferObject);

		auto pTex = m_pSMColorTexture->GetTexture();
		pTex->bind(pTex);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTex->getName(pTex), 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glGenRenderbuffers(1, &mSMDepthStencilBufferObject);
		glBindRenderbuffer(GL_RENDERBUFFER, mSMDepthStencilBufferObject);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, tex_info.GetWidth(), tex_info.GetHeight());
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mSMDepthStencilBufferObject);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		PE_CHECK_GL_ERROR_DEBUG();
		
#endif
	}
#ifdef USE_DIRECTX_RENDERER
	if (m_bBlurSMColorTexture)
	{
		TextureEntity::TextureInfo tex_info;
		tex_info.m_width = m_shadowTexWidth;
		tex_info.m_height = m_shadowTexHeight;

		if(m_pSMColorTextureBlurredHorizontal == 0)
		{
			m_pSMColorTextureBlurredHorizontal = CGlobals::GetAssetManager()->LoadTexture("_SMColorSurfaceBlurredHorizontal", "_SMColorSurfaceBlurredHorizontal", TextureEntity::RenderTarget);
			m_pSMColorTextureBlurredHorizontal->SetTextureInfo(tex_info);
			m_pSMColorTextureBlurredHorizontal->LoadAsset();
			if(!m_pSMColorTextureBlurredHorizontal->IsValid())
				return false;
		}
		SAFE_RELEASE(m_pSMColorSurfaceBlurredHorizontal);
		m_pSMColorTextureBlurredHorizontal->GetTexture()->GetSurfaceLevel(0, &m_pSMColorSurfaceBlurredHorizontal);


		if(m_pSMColorTextureBlurredVertical == 0)
		{
			m_pSMColorTextureBlurredVertical = CGlobals::GetAssetManager()->LoadTexture("_SMColorTextureBlurredVertical", "_SMColorTextureBlurredVertical", TextureEntity::RenderTarget);
			m_pSMColorTextureBlurredVertical->SetTextureInfo(tex_info);
			m_pSMColorTextureBlurredVertical->LoadAsset();
			if(!m_pSMColorTextureBlurredVertical->IsValid())
				return false;
		}
		SAFE_RELEASE(m_pSMColorSurfaceBlurredVertical);
		m_pSMColorTextureBlurredVertical->GetTexture()->GetSurfaceLevel(0, &m_pSMColorSurfaceBlurredVertical);
	}
#endif
	return true;
}
//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//  The device exists, but is about to be Reset(), so we should release some things. 
//  Resources in D3DPOOL_DEFAULT and any other device state that persists during 
//  rendering should be set here. Render states, matrices, textures, etc., that 
//  don't change during rendering can be set once here to avoid redundant state 
//  setting during Render(). 
//-----------------------------------------------------------------------------
HRESULT CShadowMap::InvalidateDeviceObjects()
{
#ifdef USE_DIRECTX_RENDERER
	SAFE_RELEASE(m_pSMColorSurface);
	SAFE_RELEASE(m_pSMZSurface);
	SAFE_RELEASE(m_pSMColorSurfaceBlurredHorizontal);
	SAFE_RELEASE(m_pSMColorSurfaceBlurredVertical);
#else
	glDeleteRenderbuffers(1, &mSMDepthStencilBufferObject);
	mSMDepthStencilBufferObject = 0;
	glDeleteFramebuffers(1, &mSMFrameBufferObject);
	mSMFrameBufferObject = 0;
	
#endif
	if (m_pSMColorTexture)
		m_pSMColorTexture->InvalidateDeviceObjects();
	if (m_pSMZTexture)
		m_pSMZTexture->InvalidateDeviceObjects();
	if (m_pSMColorTextureBlurredHorizontal)
		m_pSMColorTextureBlurredHorizontal->InvalidateDeviceObjects();
	if (m_pSMColorTextureBlurredVertical)
		m_pSMColorTextureBlurredVertical->InvalidateDeviceObjects();
	return S_OK;
}

//-----------------------------------------------------------------------------
//  PracticalPSM::ComputeVirtualCameraParameters( )
//    computes the near & far clip planes for the virtual camera based on the
//    scene.
//
//    bounds the field of view for the virtual camera based on a swept-sphere/frustum
//    intersection.  if the swept sphere representing the extrusion of an object's bounding
//    sphere along the light direction intersects the view frustum, the object is added to
//    a list of interesting shadow casters.  the field of view is the minimum cone containing
//    all eligible bounding spheres.
//-----------------------------------------------------------------------------
bool CShadowMap::ComputeVirtualCameraParameters( )
{
	bool hit = false;

	//  frustum is in world space, so that bounding boxes are minimal size (xforming an AABB
	//  generally increases its volume).
	Matrix4 modelView;
	Matrix4 modelViewProjection;
	ParaMatrixMultiply(&modelView, &m_World, &m_View);
	ParaMatrixMultiply(&modelViewProjection, &modelView, &m_Projection);
	Vector3 sweepDir = -m_lightDir;

	m_ShadowCasterPoints.clear();
	m_ShadowReceiverPoints.clear();

	//////////////////////////////////////////////////////////////////////////
	// transform casters and receivers from world space to camera space. 
	// set hit=true if there are any casters or receivers.
	Vector3 vRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();
	CShapeAABB aabbWorld;
	CShapeAABB aabbCamera;
	{
		// casters
		SceneState::List_PostRenderObject_Type::const_iterator itCur, itEnd =  CGlobals::GetSceneState()->listShadowCasters.end();
		for( itCur = CGlobals::GetSceneState()->listShadowCasters.begin(); itCur != itEnd; ++ itCur)
		{
			ObjectType oType = (*itCur).m_pRenderObject->GetMyType();
			// TODO: only add specific type
			IViewClippingObject* pObj = (*itCur).m_pRenderObject->GetViewClippingObject();
			pObj->GetAABB(&aabbWorld);
			aabbWorld.SetCenter(aabbWorld.GetCenter()-vRenderOrigin);
			aabbWorld.Rotate(modelView, aabbCamera);
			m_ShadowCasterPoints.push_back(aabbCamera);
		}

		BlockWorldClient::GetInstance()->PrepareShadowCasters(this);
	}

	if(m_iShadowType != SHADOWTYPE_ORTHO)
	{
		hit = CGlobals::GetSceneState()->listShadowReceivers.size()>0;
		// receivers
		SceneState::List_PostRenderObject_Type::const_iterator itCur, itEnd =  CGlobals::GetSceneState()->listShadowReceivers.end();
		for( itCur = CGlobals::GetSceneState()->listShadowReceivers.begin(); itCur != itEnd; ++ itCur)
		{
			ObjectType oType = (*itCur).m_pRenderObject->GetMyType();
			// TODO: only add specific type
			IViewClippingObject* pObj = (*itCur).m_pRenderObject->GetViewClippingObject();
			pObj->GetAABB(&aabbWorld);
			aabbWorld.SetCenter(aabbWorld.GetCenter()-vRenderOrigin);
			aabbWorld.Rotate(modelView, aabbCamera);
			m_ShadowReceiverPoints.push_back(aabbCamera);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// Add the biggest shadow receiver, the global terrain 
	if(m_iShadowType == SHADOWTYPE_LSPSM || m_iShadowType == SHADOWTYPE_PSM)
	{
		// using many bounding box. 
		CShapeFrustum eyeFrustum( &m_Projection );
		CGlobals::GetGlobalTerrain()->GetBoundingBoxes(m_ShadowReceiverPoints, &modelView, eyeFrustum, 700,2);
		int nSize =(int)m_ShadowReceiverPoints.size();
		if(CGlobals::WillGenReport())
			CGlobals().GetReport()->SetValue("TerrainBBNum", (double)nSize);
	}
	else if(m_iShadowType == SHADOWTYPE_ORTHO)
	{
		// TODO: just use the shadow frustum as the receiver, a more accurate way is to use the terrain bounding box within shadow frustum.
		CShapeAABB aabb(CGlobals::GetScene()->GetCurrentCamera()->GetShadowFrustum()->vecFrustum, 8);
		aabb.Rotate(modelView, aabb);
		m_ShadowReceiverPoints.push_back(aabb);
	}
	else
	{
		// only one bounding box as the terrain's shadow receiver, for TSM and Orthogonal SM.
		CShapeBox terrainBox;
		CGlobals::GetGlobalTerrain()->GetBoundingBox(&terrainBox);
		if(terrainBox.IsValid())
		{
			hit = true;
			terrainBox.Translate(-vRenderOrigin);
			CShapeAABB aabb(terrainBox);
			aabb.Rotate(modelView, aabb);
			m_ShadowReceiverPoints.push_back(aabb);
		}
	}
	
	if((int)m_ShadowReceiverPoints.size() == 0)
		return false;
	//  these are the limits specified by the physical camera
	//  gamma is the "tilt angle" between the light and the view direction.
	m_fCosGamma = m_lightDir.x * m_View._13 +
		m_lightDir.y * m_View._23 +
		m_lightDir.z * m_View._33;

	CBaseCamera* pCamera = CGlobals::GetScene()->GetCurrentCamera();
	assert(pCamera!=0);
	float fNearPlane = pCamera->GetNearPlane();
	float fFarPlane = pCamera->GetShadowFrustum()->GetViewDepth();
	if (!hit)
	{
		m_zNear = fNearPlane;
		m_zFar = fFarPlane;
		m_fSlideBack = 0.f;
	}
	else
	{
		float min_z = 1e32f;
		float max_z = 0.f;

		//////////////////////////////////////////////////////////////////////////
		// compute z_near and z_far by all shadow receivers, so that they are tightly 
		// contained in the virtual camera's view frustum.
		int nSize = (int)m_ShadowReceiverPoints.size();
		for (int i=0;i < nSize; i++) 
		{
			min_z = min(min_z, m_ShadowReceiverPoints[i].GetMin(2));//GetMin().z
			max_z = max(max_z, m_ShadowReceiverPoints[i].GetMax(2));//GetMax().z
		}
		m_zNear = max(fNearPlane, min_z);  
		m_zFar = min( fFarPlane, max_z );
		m_fSlideBack = 0.f;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: BuildTSMProjectionMatrix
// Desc: Builds a trapezoidal shadow transformation matrix
//-----------------------------------------------------------------------------
bool CShadowMap::BuildTSMProjectionMatrix()
{
	//  this isn't strictly necessary for TSMs; however, my 'light space' matrix has a
	//  degeneracy when view==light, so this avoids the problem.
	if ( fabsf(m_fCosGamma) >= 0.999f )
	{
		return BuildOrthoShadowProjectionMatrix();
	}
	else
	{
		//  update list of shadow casters/receivers
		if(!ComputeVirtualCameraParameters())
			return false;

		//  get the near and the far plane (points) in eye space.
		Vector3 frustumPnts[8];

		CShapeFrustum eyeFrustum( &m_Projection );  // autocomputes all the extrema points

		for ( int i=0; i<4; i++ )
		{
			frustumPnts[i]   = eyeFrustum.vecFrustum[i+4];      // far plane
			frustumPnts[i+4] = eyeFrustum.vecFrustum[i];		// near plane
		}

		//   we need to transform the eye into the light's post-projective space.
		//   however, the sun is a directional light, so we first need to find an appropriate
		//   rotate/translate matrix, before constructing an ortho projection.
		//   this matrix is a variant of "light space" from LSPSMs, with the Y and Z axes permuted

		Vector3 leftVector, upVector, viewVector;
		const Vector3 eyeVector( 0.f, 0.f, -1.f );  //  eye is always -Z in eye space

		//  code copied straight from BuildLSPSMProjectionMatrix
		upVector = m_lightDir.TransformNormal(m_View);  // lightDir is defined in eye space, so xform it
		leftVector = upVector.crossProduct(eyeVector).normalisedCopy();
		viewVector = upVector.crossProduct(leftVector).normalisedCopy();

		Matrix4 lightSpaceBasis;  
		lightSpaceBasis._11 = leftVector.x; lightSpaceBasis._12 = viewVector.x; lightSpaceBasis._13 = -upVector.x; lightSpaceBasis._14 = 0.f;
		lightSpaceBasis._21 = leftVector.y; lightSpaceBasis._22 = viewVector.y; lightSpaceBasis._23 = -upVector.y; lightSpaceBasis._24 = 0.f;
		lightSpaceBasis._31 = leftVector.z; lightSpaceBasis._32 = viewVector.z; lightSpaceBasis._33 = -upVector.z; lightSpaceBasis._34 = 0.f;
		lightSpaceBasis._41 = 0.f;          lightSpaceBasis._42 = 0.f;          lightSpaceBasis._43 = 0.f;        lightSpaceBasis._44 = 1.f;

		//  rotate the view frustum into light space
		ParaVec3TransformCoordArray( frustumPnts, sizeof(Vector3), frustumPnts, sizeof(Vector3), &lightSpaceBasis, sizeof(frustumPnts)/sizeof(Vector3) );

		//  build an off-center ortho projection that translates and scales the eye frustum's 3D AABB to the unit cube
		CShapeAABB frustumBox( frustumPnts, sizeof(frustumPnts) / sizeof(Vector3) );

		//  also - transform the shadow caster bounding boxes into light projective space.  we want to translate along the Z axis so that
		//  all shadow casters are in front of the near plane.

		static vector<Vector3> g_shadowCasterPnts;
		int nSize = (int)m_ShadowCasterPoints.size();
		if((int)g_shadowCasterPnts.size()<8*nSize)
			g_shadowCasterPnts.resize(8*nSize);
		
		for ( int i=0; i<nSize; i++ )
		{
			for ( int j=0; j<8; j++ ) 
				g_shadowCasterPnts[i*8+j] = m_ShadowCasterPoints[i].Point(j);
		}

		ParaVec3TransformCoordArray( &(g_shadowCasterPnts[0]), sizeof(Vector3), &(g_shadowCasterPnts[0]), sizeof(Vector3), &lightSpaceBasis, (int)nSize*8 );
		CShapeAABB casterBox( &(g_shadowCasterPnts[0]), (int)nSize*8 );
		
		float min_z = min( casterBox.GetMin().z, frustumBox.GetMin().z );
		float max_z = max( casterBox.GetMax().z, frustumBox.GetMax().z );

		if ( min_z <= 0.f )
		{
			Matrix4 lightSpaceTranslate;
			lightSpaceTranslate.makeTrans(0.f, 0.f, -min_z + 1.f );
			max_z = -min_z + max_z + 1.f;
			min_z = 1.f;
			ParaMatrixMultiply ( &lightSpaceBasis, &lightSpaceBasis, &lightSpaceTranslate );
			ParaVec3TransformCoordArray( frustumPnts, sizeof(Vector3), frustumPnts, sizeof(Vector3), &lightSpaceTranslate, sizeof(frustumPnts)/sizeof(Vector3) );
			frustumBox = CShapeAABB( frustumPnts, sizeof(frustumPnts)/sizeof(Vector3) );
		}

		Matrix4 lightSpaceOrtho;
		ParaMatrixOrthoOffCenterLH( &lightSpaceOrtho, frustumBox.GetMin().x, frustumBox.GetMax().x, frustumBox.GetMin().y, frustumBox.GetMax().y, min_z, max_z );

		//  transform the view frustum by the new matrix
		ParaVec3TransformCoordArray( frustumPnts, sizeof(Vector3), frustumPnts, sizeof(Vector3), &lightSpaceOrtho, sizeof(frustumPnts)/sizeof(Vector3) );

		Vector2 centerPts[2];
		//  near plane
		centerPts[0].x = 0.25f * (frustumPnts[4].x + frustumPnts[5].x + frustumPnts[6].x + frustumPnts[7].x);
		centerPts[0].y = 0.25f * (frustumPnts[4].y + frustumPnts[5].y + frustumPnts[6].y + frustumPnts[7].y);
		//  far plane
		centerPts[1].x = 0.25f * (frustumPnts[0].x + frustumPnts[1].x + frustumPnts[2].x + frustumPnts[3].x);
		centerPts[1].y = 0.25f * (frustumPnts[0].y + frustumPnts[1].y + frustumPnts[2].y + frustumPnts[3].y);

		Vector2 centerOrig = (centerPts[0] + centerPts[1])*0.5f;

		Matrix4 trapezoid_space;

		Matrix4 xlate_center(           1.f,           0.f, 0.f, 0.f,
			0.f,           1.f, 0.f, 0.f,
			0.f,           0.f, 1.f, 0.f,
			-centerOrig.x, -centerOrig.y, 0.f, 1.f );

		float half_center_len = Vector2(centerPts[1] - centerOrig).squaredLength();
		float x_len = centerPts[1].x - centerOrig.x;
		float y_len = centerPts[1].y - centerOrig.y;

		float cos_theta = x_len / half_center_len;
		float sin_theta = y_len / half_center_len;

		Matrix4 rot_center( cos_theta, -sin_theta, 0.f, 0.f,
			sin_theta,  cos_theta, 0.f, 0.f,
			0.f,        0.f, 1.f, 0.f,
			0.f,        0.f, 0.f, 1.f );

		//  this matrix transforms the center line to y=0.
		//  since Top and Base are orthogonal to Center, we can skip computing the convex hull, and instead
		//  just find the view frustum X-axis extrema.  The most negative is Top, the most positive is Base
		//  Point Q (trapezoid projection point) will be a point on the y=0 line.
		ParaMatrixMultiply( &trapezoid_space, &xlate_center, &rot_center );
		ParaVec3TransformCoordArray( frustumPnts, sizeof(Vector3), frustumPnts, sizeof(Vector3), &trapezoid_space, sizeof(frustumPnts)/sizeof(Vector3) );

		CShapeAABB frustumAABB2D( frustumPnts, sizeof(frustumPnts)/sizeof(Vector3) );

		Vector3 frustumAABB2D_max = frustumAABB2D.GetMax();
		Vector3 frustumAABB2D_min = frustumAABB2D.GetMin();
		float x_scale = max( fabsf(frustumAABB2D_max.x), fabsf(frustumAABB2D_min.x) );
		float y_scale = max( fabsf(frustumAABB2D_max.y), fabsf(frustumAABB2D_min.y) );
		x_scale = 1.f/x_scale;
		y_scale = 1.f/y_scale;

		//  maximize the area occupied by the bounding box
		Matrix4 scale_center( x_scale, 0.f, 0.f, 0.f,
			0.f, y_scale, 0.f, 0.f,
			0.f,     0.f, 1.f, 0.f,
			0.f,     0.f, 0.f, 1.f );

		ParaMatrixMultiply( &trapezoid_space, &trapezoid_space, &scale_center );

		//  scale the frustum AABB up by these amounts (keep all values in the same space)
		frustumAABB2D_min.x *= x_scale;
		frustumAABB2D_max.x *= x_scale;
		frustumAABB2D_min.y *= y_scale;
		frustumAABB2D_max.y *= y_scale;

		//  compute eta.
		float lambda = frustumAABB2D_max.x - frustumAABB2D_min.x;
		float delta_proj = m_fTSM_Delta * lambda; //focusPt.x - frustumAABB2D_min.x;

		const float xi = -0.6f;  // 80% line

		float eta = (lambda*delta_proj*(1.f+xi)) / (lambda*(1.f-xi)-2.f*delta_proj);

		//  compute the projection point a distance eta from the top line.  this point is on the center line, y=0
		Vector2 projectionPtQ( frustumAABB2D_max.x + eta, 0.f );

		//  find the maximum slope from the projection point to any point in the frustum.  this will be the
		//  projection field-of-view
		float max_slope = -1e32f;
		float min_slope =  1e32f;

		for ( int i=0; i < sizeof(frustumPnts)/sizeof(Vector3); i++ )
		{
			Vector2 tmp( frustumPnts[i].x*x_scale, frustumPnts[i].y*y_scale );
			float x_dist = tmp.x - projectionPtQ.x;
			if ( !(ALMOST_ZERO(tmp.y) || ALMOST_ZERO(x_dist)))
			{
				max_slope = max(max_slope, tmp.y/x_dist);
				min_slope = min(min_slope, tmp.y/x_dist);
			}
		}

		float xn = eta;
		float xf = lambda + eta;

		Matrix4 ptQ_xlate(-1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			projectionPtQ.x, 0.f, 0.f, 1.f );
		ParaMatrixMultiply( &trapezoid_space, &trapezoid_space, &ptQ_xlate );

		//  this shear balances the "trapezoid" around the y=0 axis (no change to the projection pt position)
		//  since we are redistributing the trapezoid, this affects the projection field of view (shear_amt)
		float shear_amt = (max_slope + fabsf(min_slope))*0.5f - max_slope;
		max_slope = max_slope + shear_amt;

		Matrix4 trapezoid_shear( 1.f, shear_amt, 0.f, 0.f,
			0.f,       1.f, 0.f, 0.f,
			0.f,       0.f, 1.f, 0.f,
			0.f,       0.f, 0.f, 1.f );

		ParaMatrixMultiply( &trapezoid_space, &trapezoid_space, &trapezoid_shear );


		float z_aspect = (frustumBox.GetMax().z-frustumBox.GetMin().z) / (frustumAABB2D_max.y-frustumAABB2D_min.y);

		//  perform a 2DH projection to 'unsqueeze' the top line.
		Matrix4 trapezoid_projection(  xf/(xf-xn),          0.f, 0.f, 1.f,
			0.f, 1.f/max_slope, 0.f, 0.f,
			0.f,           0.f, 1.f/(z_aspect*max_slope), 0.f,
			-xn*xf/(xf-xn),           0.f, 0.f, 0.f );

		ParaMatrixMultiply( &trapezoid_space, &trapezoid_space, &trapezoid_projection );

		//  the x axis is compressed to [0..1] as a result of the projection, so expand it to [-1,1]
		Matrix4 biasedScaleX( 2.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			-1.f, 0.f, 0.f, 1.f );
		ParaMatrixMultiply( &trapezoid_space, &trapezoid_space, &biasedScaleX );

		ParaMatrixMultiply( &trapezoid_space, &lightSpaceOrtho, &trapezoid_space );
		ParaMatrixMultiply( &trapezoid_space, &lightSpaceBasis, &trapezoid_space );

		// now, focus on shadow receivers.
		if ( m_bUnitCubeClip )
		{
			Vector3* shadowReceiverPnts = NULL;
			int nCount = (int)m_ShadowReceiverPoints.size();
			int nByteCount = nCount*8;
			if(nByteCount>0)
			{
				static vector<Vector3> shadowReceiverPnts;
				if((int)shadowReceiverPnts.size()<nByteCount)
					shadowReceiverPnts.resize(nByteCount);
				for ( int i=0; i<nCount; i++ )
				{
					for ( int j=0; j<8; j++ ) 
						shadowReceiverPnts[i*8+j] = m_ShadowReceiverPoints[i].Point(j);
				}

				ParaVec3TransformCoordArray( &shadowReceiverPnts[0], sizeof(Vector3), &shadowReceiverPnts[0], sizeof(Vector3), &trapezoid_space, nByteCount );
				CShapeAABB rcvrBox( &shadowReceiverPnts[0], nByteCount );

				//  never shrink the box, only expand it.
				Vector3 rcvrBox_maxPt = rcvrBox.GetMax();
				Vector3 rcvrBox_minPt = rcvrBox.GetMin();
				rcvrBox_maxPt.x = min( 1.f, rcvrBox_maxPt.x );
				rcvrBox_minPt.x = max(-1.f, rcvrBox_minPt.x );
				rcvrBox_maxPt.y = min( 1.f, rcvrBox_maxPt.y );
				rcvrBox_minPt.y = max(-1.f, rcvrBox_minPt.y );
				float boxWidth  = rcvrBox_maxPt.x - rcvrBox_minPt.x;
				float boxHeight = rcvrBox_maxPt.y - rcvrBox_minPt.y;

				//  the receiver box is degenerate, this will generate specials (and there shouldn't be any shadows, anyway).
				if ( !(ALMOST_ZERO(boxWidth) || ALMOST_ZERO(boxHeight)) )
				{
					//  the divide by two's cancel out in the translation, but included for clarity
					float boxX = (rcvrBox_maxPt.x+rcvrBox_minPt.x) / 2.f;
					float boxY = (rcvrBox_maxPt.y+rcvrBox_minPt.y) / 2.f;
					Matrix4 trapezoidUnitCube( 2.f/boxWidth,                 0.f, 0.f, 0.f,
						0.f,       2.f/boxHeight, 0.f, 0.f,
						0.f,                 0.f, 1.f, 0.f,
						-2.f*boxX/boxWidth, -2.f*boxY/boxHeight, 0.f, 1.f );
					ParaMatrixMultiply( &trapezoid_space, &trapezoid_space, &trapezoidUnitCube );
				}
			}
			else
				return false;
		}

		ParaMatrixMultiply( &m_LightViewProj, &m_View, &trapezoid_space );

	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: BuildLiSPSMProjectionMatrix
// Desc: Builds a light-space perspective shadow map projection matrix
// Note: based on "Light Space Perspective Shadow Maps" by Michael Wimmer, Daniel Scherzer and Werner Purgathofer. revised version June 10, 2005
//-----------------------------------------------------------------------------
bool CShadowMap::BuildLSPSMProjectionMatrix()
{
	if ( fabsf(m_fCosGamma) >= 0.999f )  // degenerates to uniform shadow map
	{
		return BuildOrthoShadowProjectionMatrix();
	}
	else
	{
		CBaseCamera* pCamera = CGlobals::GetScene()->GetCurrentCamera();
		assert(pCamera!=0);
		float fNearPlane = pCamera->GetNearPlane();
		float fFarPlane = pCamera->GetShadowFrustum()->GetViewDepth();

		//  compute shadow casters & receivers
		if(!ComputeVirtualCameraParameters())
			return false;

		//////////////////////////////////////////////////////////////////////////
		// 1. Compute the convex body B that encloses all interesting light rays (i.e., the view frustum and all objects that can cast shadows into it).
		// The result of this calculation is a convex body B described by a number of points in view space.
		//////////////////////////////////////////////////////////////////////////
		std::vector<Vector3> bodyB; bodyB.reserve( m_ShadowCasterPoints.size()*8 + 8 );
		// add frustum extremities to a list of points (in view space) in convex body B
		CShapeFrustum eyeFrustum( &m_Projection );
		for ( int i=0; i<8; i++ ) 
			bodyB.push_back( eyeFrustum.vecFrustum[i] );
		
		
		// adding all the points for the shadow caster/receiver bounding boxes. Only casters are added, since receivers are already covered by the view frustum.
		std::vector<CShapeAABB>::iterator boxIt, boxItEnd = m_ShadowCasterPoints.end();
		for ( boxIt = m_ShadowCasterPoints.begin();boxIt!=boxItEnd;++boxIt)
		{
			const CShapeAABB& box = *boxIt;
			for ( int i=0; i<8; i++ ) 
				bodyB.push_back( box.Point(i) );
		}

		//////////////////////////////////////////////////////////////////////////
		// 2. Enclose this body B with an appropriate perspective frustum P(from View Space) that has a view vector parallel to the shadow map.
		// note:  since bodyB is defined in eye space, all axis vectors of frustum P should also be defined in eye/view space
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// 2.1 compute the direction of frustum P
		// leftVector, upVector, viewVector are the x, y, z axis of the frustum P in view space
		Vector3 leftVector, upVector, viewVector;

		// The y-axis(upVector) of frustum P is defined by the light vector (but pointing towards the light).
		// m_lightDir is originally defined in world space, so simple transform it to view space
		//  note: m_lightDir points away from the scene, so it is already the "negative" up direction;
		upVector = m_lightDir.TransformNormal(m_View);  

		// The x-axis(leftVector) is defined to be perpendicular to the plane containing the light vector and the observer view vector eyeVector.
		const Vector3 eyeVector( 0.f, 0.f, -1.f );  // eye vector in eye/view space is always -Z, LXZ: it seems to be (0,0,1) for a left hand coordinate system
		leftVector = (upVector*eyeVector).normalisedCopy();
		// The z-axis complements the other two axes in order to form an orthogonal coordinate system.
		viewVector = upVector.crossProduct(leftVector).normalisedCopy();

		Matrix4 lightSpaceBasis;  
		// the following transform is identical to  D3DXMatrixLookAtLH(&lightSpaceBasis, &Vector3(0,0,0), &viewVector, &upVector);
		lightSpaceBasis._11 = leftVector.x; lightSpaceBasis._12 = upVector.x; lightSpaceBasis._13 = viewVector.x; lightSpaceBasis._14 = 0.f;
		lightSpaceBasis._21 = leftVector.y; lightSpaceBasis._22 = upVector.y; lightSpaceBasis._23 = viewVector.y; lightSpaceBasis._24 = 0.f;
		lightSpaceBasis._31 = leftVector.z; lightSpaceBasis._32 = upVector.z; lightSpaceBasis._33 = viewVector.z; lightSpaceBasis._34 = 0.f;
		lightSpaceBasis._41 = 0.f;          lightSpaceBasis._42 = 0.f;        lightSpaceBasis._43 = 0.f;          lightSpaceBasis._44 = 1.f;

		//////////////////////////////////////////////////////////////////////////
		// 2.2 compute the near and far planes for the perspective frustum P
		// The near and far planes for the perspective frustum P are defined as planes parallel to the xy-coordinate plane of P, placed
		// at the minimum and maximum z-coordinate among the points of the body B in P.

		//////////////////////////////////////////////////////////////////////////
		//  transform all points in Body B from view space to a orthogonal coordinate system with the same direction of frustum P.
		ParaVec3TransformCoordArray( &bodyB[0], sizeof(Vector3), &bodyB[0], sizeof(Vector3), &lightSpaceBasis, (UINT)bodyB.size() );

		// OK: now that lightSpaceBox.minZ in the near plane, lightSpaceBox.maxZ in the far plane
		CShapeAABB lightSpaceBox( &bodyB[0], (int)bodyB.size() );

		// The x and y coordinates of the projection reference point lightSpaceOrigin
		// for P are chosen so that the resulting frustum is roughly symmetrical,
		//	by taking the x-coordinate from the transformed viewpoint and the y-coordinate as the middle of the minimum	and maximum y-coordinates of the body B.
		Vector3 lightSpaceOrigin;
		//  for some reason, the paper recommended using the x coordinate of the xformed viewpoint as
		//  the x-origin for lightspace, but that doesn't seem to make sense...  instead, we'll take
		//  the x-midpt of body B (like for the Y axis)
		lightSpaceBox.GetCenter(lightSpaceOrigin ); // in fact only lightSpaceOrigin.x and lightSpaceOrigin.y are used. 
		
		//////////////////////////////////////////////////////////////////////////
		// 2.3 Compute The remaining free parameter for P, the distance n of the projection reference point lightSpaceOrigin to the near plane,
		// the paper suggests an optimum n or Nopt = fNearPlane + sqrtf(fNearPlane*fFarPlane);
		float sinGamma = sqrtf( 1.f - m_fCosGamma*m_fCosGamma );

		//  use average of the "real" near/far distance and the optimized near/far distance to get a more pleasant result
		float Nopt0 = m_zNear + sqrtf(m_zNear*m_zFar);
		float Nopt1 = fNearPlane + sqrtf(fNearPlane*fFarPlane);
		m_fLSPSM_Nopt  = (Nopt0 + Nopt1) / (2.f*sinGamma);
		//  add a constant bias, to guarantee some minimum distance between the projection point and the near plane
		m_fLSPSM_Nopt += 0.1f;
		//  now use the weighting to scale between 0.1 and the computed Nopt
		float Nopt = 0.1f + m_fLSPSM_NoptWeight * (m_fLSPSM_Nopt - 0.1f);

		// OK: now that we have the z component of the lightSpaceOrigin
		lightSpaceOrigin.z = lightSpaceBox.GetMin().z - Nopt;

		// translate all points in BodyB by lightSpaceOrigin to match the new light space origin, and compute the fov and aspect ratio of frustum P
		float maxx=0.f, maxy=0.f, maxz=0.f;

		std::vector<Vector3>::iterator ptIt, ptItEnd = bodyB.end();
		for( ptIt = bodyB.begin(); ptIt!=ptItEnd;++ptIt)
		{
			// translation of each point is done here
			Vector3 tmp = (*ptIt) - lightSpaceOrigin;
			// assert(tmp.z > 0.f); // LXZ 2008.9.20, sometimes this will fail.
			// find the fov and aspect ratio of frustum P
			maxx = max(maxx, fabsf(tmp.x / tmp.z));
			maxy = max(maxy, fabsf(tmp.y / tmp.z));
			maxz = max(maxz, tmp.z);
		}

		// OK: we now have the fov and aspect ratio of frustum P
		float fovy = atanf(maxy);
		float fovx = atanf(maxx);

		Matrix4 lsTranslate, lsPerspective;  

		lsTranslate.makeTrans(-lightSpaceOrigin.x, -lightSpaceOrigin.y, -lightSpaceOrigin.z);
		ParaMatrixPerspectiveLH( &lsPerspective, 2.f*maxx*Nopt, 2.f*maxy*Nopt, Nopt, maxz );
		// build the final light space transform, so that lightSpaceBasis will then transform points from view space to the prospective space specified by frustum P.
		ParaMatrixMultiply( &lightSpaceBasis, &lightSpaceBasis, &lsTranslate ); // translate by origin, since we only rotated last time.
		ParaMatrixMultiply( &lightSpaceBasis, &lightSpaceBasis, &lsPerspective ); // apply the perspective transform.

		//  finally, rotate the entire post-projective cube P, so that the shadow map is looking down the Y-axis
		Matrix4 lsPermute, lsOrtho;

		lsPermute._11 = 1.f; lsPermute._12 = 0.f; lsPermute._13 = 0.f; lsPermute._14 = 0.f;
		lsPermute._21 = 0.f; lsPermute._22 = 0.f; lsPermute._23 =-1.f; lsPermute._24 = 0.f;
		lsPermute._31 = 0.f; lsPermute._32 = 1.f; lsPermute._33 = 0.f; lsPermute._34 = 0.f;
		lsPermute._41 = 0.f; lsPermute._42 = -0.5f; lsPermute._43 = 1.5f; lsPermute._44 = 1.f;
		ParaMatrixOrthoLH( &lsOrtho, 2.f, 1.f, 0.5f, 2.5f );
		ParaMatrixMultiply( &lsPermute, &lsPermute, &lsOrtho );
		// eventually, we have a lightSpaceBasis
		ParaMatrixMultiply( &lightSpaceBasis, &lightSpaceBasis, &lsPermute );

		//if ( false)
		if ( m_bUnitCubeClip )
		{
			//////////////////////////////////////////////////////////////////////////
			// the frustum lightSpaceBasis generated by the above step can be further clipped by 
			// including only the shadow receiver points, thus greatly increase the shadow map resolution.
			//////////////////////////////////////////////////////////////////////////
			int nCount = (int)m_ShadowReceiverPoints.size();
			if(nCount>0)
			{
				std::vector<Vector3> receiverPts;
				receiverPts.reserve(nCount * 8);
				std::vector<CShapeAABB>::iterator itRCCur, itRCEnd = m_ShadowReceiverPoints.end();
				for( itRCCur= m_ShadowReceiverPoints.begin(); itRCCur!= itRCEnd;++itRCCur)
				{
					for ( int i=0; i<8; i++ )
						receiverPts.push_back( itRCCur->Point(i) );
				}

				ParaVec3TransformCoordArray( &receiverPts[0], sizeof(Vector3), &receiverPts[0], sizeof(Vector3), &lightSpaceBasis, (UINT)receiverPts.size() );

				CShapeAABB receiverBox( &receiverPts[0], (int)receiverPts.size() );
				Vector3 receiverBox_maxPt = receiverBox.GetMax();
				Vector3 receiverBox_minPt = receiverBox.GetMin();

				receiverBox_maxPt.x = min( 1.f, receiverBox_maxPt.x );
				receiverBox_minPt.x = max(-1.f, receiverBox_minPt.x );
				receiverBox_maxPt.y = min( 1.f, receiverBox_maxPt.y );
				receiverBox_minPt.y = max(-1.f, receiverBox_minPt.y );
				float boxWidth = receiverBox_maxPt.x - receiverBox_minPt.x;
				float boxHeight = receiverBox_maxPt.y - receiverBox_minPt.y;

				if ( !ALMOST_ZERO(boxWidth) && !ALMOST_ZERO(boxHeight) )
				{
					float boxX = ( receiverBox_maxPt.x + receiverBox_minPt.x ) * 0.5f;
					float boxY = ( receiverBox_maxPt.y + receiverBox_minPt.y ) * 0.5f;

					Matrix4 clipMatrix( 2.f/boxWidth,  0.f, 0.f, 0.f,
						0.f, 2.f/boxHeight, 0.f, 0.f,
						0.f,           0.f, 1.f, 0.f,
						-2.f*boxX/boxWidth, -2.f*boxY/boxHeight, 0.f, 1.f );
					ParaMatrixMultiply( &lightSpaceBasis, &lightSpaceBasis, &clipMatrix );
				}
			}
		}

		ParaMatrixMultiply( &m_LightViewProj, &m_View, &lightSpaceBasis );
	}

	return true;
}
//-----------------------------------------------------------------------------
// Name: BuildPSMProjectionMatrix
// Desc: Builds a perpsective shadow map transformation matrix
//-----------------------------------------------------------------------------
bool CShadowMap::BuildPSMProjectionMatrix()
{
	Matrix4 lightView, lightProj, virtualCameraViewProj, virtualCameraView, virtualCameraProj;

	const Vector3 yAxis  ( 0.f, 1.f, 0.f);
	const Vector3 zAxis  ( 0.f, 0.f, 1.f);

	//  update the virtual scene camera's bounding parameters
	if(!ComputeVirtualCameraParameters())
		return false;

	CBaseCamera* pCamera = CGlobals::GetScene()->GetCurrentCamera();
	assert(pCamera!=0);
	float fNearPlane = pCamera->GetNearPlane();
	float fFarPlane = pCamera->GetShadowFrustum()->GetViewDepth();
	m_fAspect = pCamera->GetAspectRatio();

	//  compute a slide back, that force some distance between the infinity plane and the view-box
	const float Z_EPSILON=0.0001f;
	float infinity = m_zFar/(m_zFar-m_zNear);
	float fInfinityZ = m_fMinInfinityZ;
	if ( (infinity<=fInfinityZ) && m_bSlideBack)
	{
		float additionalSlide = fInfinityZ*(m_zFar-m_zNear) - m_zFar + Z_EPSILON;
		m_fSlideBack = additionalSlide;
		m_zFar += additionalSlide;
		m_zNear += additionalSlide;
	}

	if (m_bSlideBack)
	{
		//  clamp the view-cube to the slid back objects...
		const Vector3 eyePt(0.f, 0.f, 0.f);
		const Vector3 eyeDir(0.f, 0.f, 1.f);
		virtualCameraView.makeTrans(0.f, 0.f, m_fSlideBack);

		if ( m_bUnitCubeClip )
		{
			CShapeCone bc( &m_ShadowReceiverPoints[0],(int)m_ShadowReceiverPoints.size(), &virtualCameraView, &eyePt, &eyeDir );
			ParaMatrixPerspectiveLH( &virtualCameraProj, 2.f*tanf(bc.fovx)*m_zNear, 2.f*tanf(bc.fovy)*m_zNear, m_zNear, m_zFar );
		}
		else
		{
			const float viewHeight =  fFarPlane * 0.57735026919f;  // tan(0.5f*VIEW_ANGLE)*fFarPlane       
			float viewWidth  = viewHeight * m_fAspect;
			float halfFovy = atanf( viewHeight / (fFarPlane+m_fSlideBack) );
			float halfFovx = atanf( viewWidth  / (fFarPlane+m_fSlideBack) );

			ParaMatrixPerspectiveLH( &virtualCameraProj, 2.f*tanf(halfFovx)*m_zNear, 2.f*tanf(halfFovy)*m_zNear, m_zNear, m_zFar );
		}
		//        ParaMatrixPerspectiveFovLH( &virtualCameraProj, 2.f*halfFovy, halfFovx/halfFovy, m_zNear, m_zFar);
	}
	else
	{
		virtualCameraView = Matrix4::IDENTITY;
		ParaMatrixPerspectiveFovLH( &virtualCameraProj, 
#ifdef USE_DIRECTX_RENDERER
			D3DXToRadian(60.f)
#else
			60.f/180.f*3.1415926f
#endif
			, m_fAspect, m_zNear, m_zFar);
	}

	ParaMatrixMultiply(&virtualCameraViewProj, &m_View, &virtualCameraView);
	ParaMatrixMultiply(&virtualCameraViewProj, &virtualCameraViewProj, &virtualCameraProj);

	Matrix4 eyeToPostProjectiveVirtualCamera;
	ParaMatrixMultiply(&eyeToPostProjectiveVirtualCamera, &virtualCameraView, &virtualCameraProj);

	Vector3 eyeLightDir;  
	eyeLightDir = m_lightDir.TransformNormal(m_View);

	//  directional light becomes a point on infinity plane in post-projective space
	Vector4 lightDirW (eyeLightDir.x, eyeLightDir.y, eyeLightDir.z, 0.f);   
	Vector4   ppLight;

	ppLight = lightDirW * virtualCameraProj;

	m_bShadowTestInverted = (ppLight.w < 0.f); // the light is coming from behind the eye

	//  compute the projection matrix...
	//  if the light is >= 1000 units away from the unit box, use an ortho matrix (standard shadow mapping)
	if ( (fabsf(ppLight.w) <= W_EPSILON) )  // orthographic matrix; uniform shadow mapping
	{
		Vector3 ppLightDirection(ppLight.x, ppLight.y, ppLight.z);
		ParaVec3Normalize(&ppLightDirection, &ppLightDirection);

		CShapeAABB ppUnitBox; ppUnitBox.SetMinMax(Vector3(-1, -1, 0), Vector3(1, 1, 1));
		Vector3 cubeCenter = ppUnitBox.GetCenter();
		float t;

		ppUnitBox.Intersect( &t, &cubeCenter, &ppLightDirection );
		Vector3 lightPos = cubeCenter + 2.f*t*ppLightDirection;
		Vector3 axis = yAxis;

		//  if the yAxis and the view direction are aligned, choose a different up vector, to avoid singularity
		//  artifacts
		if ( fabsf(ppLightDirection.dotProduct(yAxis))>0.99f )
			axis = zAxis;

		ParaMatrixLookAtLH(&lightView, &lightPos, &cubeCenter, &axis);
		ppUnitBox.Rotate(lightView, ppUnitBox);
		Vector3 ppUnitBox_minPt = ppUnitBox.GetMin();
		Vector3 ppUnitBox_maxPt = ppUnitBox.GetMax();
		ParaMatrixOrthoOffCenterLH(&lightProj, ppUnitBox_minPt.x, ppUnitBox_maxPt.x, ppUnitBox_minPt.y, ppUnitBox_maxPt.y, ppUnitBox_minPt.z, ppUnitBox_maxPt.z);
		m_ppNear = ppUnitBox_minPt.z;
		m_ppFar  = ppUnitBox_maxPt.z;
		m_fSlideBack = 0.f;
	}
	else  // otherwise, use perspective shadow mapping
	{
		Vector3 ppLightPos;
		float wRecip = 1.0f / ppLight.w;
		ppLightPos.x = ppLight.x * wRecip;
		ppLightPos.y = ppLight.y * wRecip;
		ppLightPos.z = ppLight.z * wRecip;

		Matrix4 eyeToPostProjectiveLightView;

		const float ppCubeRadius = 1.5f;  // the post-projective view box is [-1,-1,0]..[1,1,1] in DirectX, so its radius is 1.5
		const Vector3 ppCubeCenter(0.f, 0.f, 0.5f);

		if (m_bShadowTestInverted)  // use the inverse projection matrix
		{
			CShapeCone viewCone;
			if (!m_bUnitCubeClip)
			{
				//  project the entire unit cube into the shadow map  
				std::vector<CShapeAABB> justOneBox;
				CShapeAABB unitCube;
				unitCube.SetMinMax(Vector3(-1.f, -1.f, 0.f), Vector3( 1.f, 1.f, 1.f ));
				justOneBox.push_back(unitCube);
				viewCone = CShapeCone(&justOneBox[0],(int)justOneBox.size(), &Matrix4::IDENTITY, &ppLightPos);               
			}
			else
			{
				//  clip the shadow map to just the used portions of the unit box.
				viewCone = CShapeCone(&m_ShadowReceiverPoints[0],(int)m_ShadowReceiverPoints.size(), &eyeToPostProjectiveVirtualCamera, &ppLightPos);
			}

			//  construct the inverse projection matrix -- clamp the fNear value for sanity (clamping at too low
			//  a value causes significant underflow in a 24-bit depth buffer)
			//  the multiplication is necessary since I'm not checking shadow casters
			viewCone.fNear = max(0.001f, viewCone.fNear*0.3f);
			m_ppNear = -viewCone.fNear;
			m_ppFar  = viewCone.fNear;
			lightView = viewCone.m_LookAt;
			ParaMatrixPerspectiveLH( &lightProj, 2.f*tanf(viewCone.fovx)*m_ppNear, 2.f*tanf(viewCone.fovy)*m_ppNear, m_ppNear, m_ppFar );
			//D3DXMatrixPerspectiveFovLH(&lightProj, 2.f*viewCone.fovy, viewCone.fovx/viewCone.fovy, m_ppNear, m_ppFar);
		}
		else  // regular projection matrix
		{
			float fFovy, fAspect, fFar, fNear;
			if (!m_bUnitCubeClip)
			{
				Vector3 lookAt = ppCubeCenter - ppLightPos;

				float distance = lookAt.length();
				lookAt = lookAt / distance;

				Vector3 axis = yAxis;
				//  if the yAxis and the view direction are aligned, choose a different up vector, to avoid singularity
				//  artifacts
				if ( fabsf(yAxis.dotProduct(lookAt))>0.99f )
					axis = zAxis;

				//  this code is super-cheese.  treats the unit-box as a sphere
				//  lots of problems, looks like hell, and requires that MinInfinityZ >= 2
				ParaMatrixLookAtLH(&lightView, &ppLightPos, &ppCubeCenter, &axis);
				fFovy = 2.f*atanf(ppCubeRadius/distance);
				fAspect = 1.f;
				fNear = max(0.001f, distance - 2.f*ppCubeRadius);
				fFar = distance + 2.f*ppCubeRadius;
				
				ParaMatrixMultiply(&eyeToPostProjectiveLightView, &eyeToPostProjectiveVirtualCamera, &lightView);
			}
			else
			{
				//  unit cube clipping
				//  fit a cone to the bounding geometries of all shadow receivers (incl. terrain) in the scene
				CShapeCone bc(&m_ShadowReceiverPoints[0],(int)m_ShadowReceiverPoints.size(), &eyeToPostProjectiveVirtualCamera, &ppLightPos);
				lightView = bc.m_LookAt;
				ParaMatrixMultiply(&eyeToPostProjectiveLightView, &eyeToPostProjectiveVirtualCamera, &lightView);
				float fDistance = (ppLightPos-ppCubeCenter).length();
				fFovy = 2.f * bc.fovy;
				fAspect = bc.fovx / bc.fovy;
				fFar = bc.fFar;
				//  hack alert!  adjust the near-plane value a little bit, to avoid clamping problems
				fNear = bc.fNear * 0.6f;
			}

			fNear = max(0.001f, fNear);
			m_ppNear = fNear;
			m_ppFar = fFar;
			ParaMatrixPerspectiveFovLH(&lightProj, fFovy, fAspect, m_ppNear, m_ppFar);
		}
	}

	//  build the composite matrix that transforms from world space into post-projective light space
	ParaMatrixMultiply(&m_LightViewProj, &lightView, &lightProj);
	ParaMatrixMultiply(&m_LightViewProj, &virtualCameraViewProj, &m_LightViewProj);
	return true;
}

bool CShadowMap::BuildOrthoShadowProjectionMatrix()
{
	//  update the list of shadow casters and receivers.
	if(!ComputeVirtualCameraParameters())
		return false;
	
	if(m_ShadowReceiverPoints.size()<=0 || m_ShadowCasterPoints.size()<=0)
	{
		m_LightViewProj = Matrix4::IDENTITY;
		return true;
	}

	CBaseCamera* pCamera = CGlobals::GetScene()->GetCurrentCamera();
	assert(pCamera!=0);
	float fNearPlane = pCamera->GetNearPlane();
// #define USE_SHADOW_RECEIVER_AABB
#ifdef USE_SHADOW_RECEIVER_AABB
	float fFarPlane = pCamera->GetShadowFrustum()->GetViewDepth();
	m_fAspect = pCamera->GetAspectRatio();

	Matrix4 lightView, lightProj;
	const Vector3 zAxis(0.f, 0.f, 1.f);
	const Vector3 yAxis(0.f, 1.f, 0.f);
	Vector3 eyeLightDir;

	eyeLightDir = m_lightDir.TransformNormal(m_View);

	float fHeight = D3DXToRadian(60.f);
	float fWidth = m_fAspect * fHeight;

	CShapeAABB frustumAABB;
	if ( m_bUnitCubeClip )
	{
		frustumAABB = CShapeAABB( &m_ShadowReceiverPoints[0], (int)m_ShadowReceiverPoints.size() );
	}
	else
	{
		frustumAABB.SetMinMax( Vector3(-fWidth*fFarPlane, -fHeight*fFarPlane, fNearPlane), Vector3( fWidth*fFarPlane,  fHeight*fFarPlane, fFarPlane));
	}

	//  light pt is "infinitely" far away from the view frustum.
	//  however, all that's really needed is to place it just outside of all shadow casters

	CShapeAABB casterAABB( &m_ShadowCasterPoints[0], (int)m_ShadowCasterPoints.size() );
	Vector3 frustumCenter; frustumAABB.GetCenter( frustumCenter );
	float t;
	casterAABB.ExtendByAABBList(&frustumAABB, 1); // 2007.2. add by LiXizhi, in case it never intersects.
	casterAABB.Intersect( &t, &frustumCenter, &eyeLightDir );
	
	Vector3 lightPt = frustumCenter + 2.f*t*eyeLightDir;
	Vector3 axis;

	if ( fabsf(eyeLightDir.dotProduct(yAxis))>0.99f )
		axis = zAxis;
	else
		axis = yAxis;

	ParaMatrixLookAtLH( &lightView, &lightPt, &frustumCenter, &axis );
	
	frustumAABB.Rotate(lightView, frustumAABB);
	casterAABB.Rotate(lightView, casterAABB);
	
	//  use a small fudge factor for the near plane, to avoid some minor clipping artifacts
	ParaMatrixOrthoOffCenterLH( &lightProj, frustumAABB.GetMin().x, frustumAABB.GetMax().x,
		frustumAABB.GetMin().y, frustumAABB.GetMax().y,
		casterAABB.GetMin().z, frustumAABB.GetMax().z );

	ParaMatrixMultiply( &lightView, &m_View, &lightView );
	ParaMatrixMultiply( &m_LightViewProj, &lightView, &lightProj );
#else
	// use the entire frustum
	
	//edit by devilwalk
	const Vector3 z_vec(0.f, 0.f, 1.f);
	const Vector3 y_vec(0.f, 1.f, 0.f);
	const Vector3 zero_vec(0, 0, 0);
	const Vector3 light_dir = -m_lightDir;
	// virtual light view matrix 
	Matrix4 fake_light_view;
	ParaMatrixLookAtLH(&fake_light_view, &zero_vec, &light_dir, fabsf(light_dir.dotProduct(y_vec)) > 0.99f ? &z_vec : &y_vec);
	// frustum to virtual light coordinate space 
	Vector3 frustum_point_in_fake_light[8];
	for (int i = 0; i < 8; ++i)
	{
		frustum_point_in_fake_light[i] = pCamera->GetShadowFrustum()->vecFrustum[i] * fake_light_view;
	}
	// compute new AABB
	Vector3 min_pt(FLT_MAX, FLT_MAX, FLT_MAX), max_pt(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int i = 0; i < 8; ++i)
	{
		min_pt.x = (std::min)(min_pt.x, frustum_point_in_fake_light[i].x);
		min_pt.y = (std::min)(min_pt.y, frustum_point_in_fake_light[i].y);
		min_pt.z = (std::min)(min_pt.z, frustum_point_in_fake_light[i].z);
		max_pt.x = (std::max)(max_pt.x, frustum_point_in_fake_light[i].x);
		max_pt.y = (std::max)(max_pt.y, frustum_point_in_fake_light[i].y);
		max_pt.z = (std::max)(max_pt.z, frustum_point_in_fake_light[i].z);
	}
	//snap
	Vector3 diagonal_vec = pCamera->GetShadowFrustum()->vecFrustum[0] - pCamera->GetShadowFrustum()->vecFrustum[6];
	float diagonal_len = diagonal_vec.length();
	diagonal_vec = diagonal_len;
	// The offset calculated will pad the ortho projection so that it is always the same size 
	// and big enough to cover the entire cascade interval.
	Vector3 vBoarderOffset = (diagonal_vec -
		(max_pt - min_pt))
		* 0.5f;
	vBoarderOffset.z = 0;
	max_pt += vBoarderOffset;
	min_pt -= vBoarderOffset;
	float fWorldUnitsPerTexel = diagonal_len / m_shadowTexWidth;
	min_pt.x /= fWorldUnitsPerTexel;
	min_pt.x = floorf(min_pt.x);
	min_pt.x *= fWorldUnitsPerTexel;
	min_pt.y /= fWorldUnitsPerTexel;
	min_pt.y = floorf(min_pt.y);
	min_pt.y *= fWorldUnitsPerTexel;
	max_pt.x /= fWorldUnitsPerTexel;
	max_pt.x = floorf(max_pt.x);
	max_pt.x *= fWorldUnitsPerTexel;
	max_pt.y /= fWorldUnitsPerTexel;
	max_pt.y = floorf(max_pt.y);
	max_pt.y *= fWorldUnitsPerTexel;
	// reconstruct project matrix
	Matrix4 fake_light_proj;
	ParaMatrixOrthoOffCenterLH(&fake_light_proj, min_pt.x, max_pt.x,
		min_pt.y, max_pt.y,
		min_pt.z - 100.0f, max_pt.z);
	ParaMatrixMultiply(&m_LightViewProj, &fake_light_view, &fake_light_proj);
#endif
	return true;
}

static ParaViewport oldViewport;

HRESULT CShadowMap::BeginShadowPass()
{
	m_lightDir = -CGlobals::GetScene()->GetSunLight().GetSunDirection();
	m_View = CGlobals::GetViewMatrixStack().SafeGetTop();
	m_World = CGlobals::GetWorldMatrixStack().SafeGetTop();
	m_Projection = CGlobals::GetProjectionMatrixStack().SafeGetTop();

	bool bRes = false;
	switch ( m_iShadowType )
	{
	case (int)SHADOWTYPE_PSM:
		bRes = BuildPSMProjectionMatrix();
		break;
	case (int)SHADOWTYPE_LSPSM:
		bRes = BuildLSPSMProjectionMatrix();
		break;
	case (int)SHADOWTYPE_TSM:
		bRes = BuildTSMProjectionMatrix();
		break;
	case (int)SHADOWTYPE_ORTHO:
		bRes = BuildOrthoShadowProjectionMatrix();
		break;
	}

	if(!bRes)
		return E_FAIL;

	if(!PrepareAllSurfaces())
		return E_FAIL;

	CGlobals::GetViewMatrixStack().push(*CGlobals::GetIdentityMatrix());
	CGlobals::GetProjectionMatrixStack().push(m_LightViewProj);
	
	//set special texture matrix for shadow mapping
	float fOffsetX = 0.5f + (0.5f / (float)(m_shadowTexWidth));
	float fOffsetY = 0.5f + (0.5f / (float)(m_shadowTexHeight));
	unsigned int range = 1;
	float fBias    = 0.0f;
#ifdef USE_DIRECTX_RENDERER
	Matrix4 texScaleBiasMat(0.5f, 0.0f, 0.0f, 0.0f,
							0.0f,    -0.5f,     0.0f,         0.0f,
							0.0f,     0.0f,     (float)range, 0.0f,
							fOffsetX, fOffsetY, 0.0f,         1.0f );
	
	ParaMatrixMultiply(&m_textureMatrix, &m_LightViewProj, &texScaleBiasMat);
//#ifdef USE_DIRECTX_RENDERER
	//  preserve old viewport and back buffer
	auto pd3dDevice =  CGlobals::GetRenderDevice();
	oldViewport = CGlobals::GetRenderDevice()->GetViewport();


	m_pBackBuffer = CGlobals::GetDirectXEngine().GetRenderTarget();
	if(FAILED(GETD3D(CGlobals::GetRenderDevice())->GetDepthStencilSurface(&m_pZBuffer)))
		return E_FAIL;
#else
	Matrix4 texScaleBiasMat(0.5f, 0.0f, 0.0f, 0.0f,
							0.0f,    0.5f,     0.0f,         0.0f,
							0.0f,     0.0f,     (float)range, 0.0f,
							0.5f, 0.5f, 0.0f, 1.0f);
	
	ParaMatrixMultiply(&m_textureMatrix, &m_LightViewProj, &texScaleBiasMat);
	auto pd3dDevice = CGlobals::GetRenderDevice();
	oldViewport = CGlobals::GetRenderDevice()->GetViewport();

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&mOldFrameBufferObject));
	
#endif

	// restore other textures
	CGlobals::GetRenderDevice()->SetTexture(1, NULL);
	CGlobals::GetRenderDevice()->SetTexture(2, NULL);
	CGlobals::GetRenderDevice()->SetTexture(3, NULL);

	//set render target to shadow map surfaces
#ifdef USE_DIRECTX_RENDERER
	if (FAILED(CGlobals::GetDirectXEngine().SetRenderTarget(0, m_pSMColorSurface)))
		return E_FAIL;

	GETD3D(CGlobals::GetRenderDevice())->SetRenderTarget(1,NULL);
	GETD3D(CGlobals::GetRenderDevice())->SetRenderTarget(2,NULL);
	GETD3D(CGlobals::GetRenderDevice())->SetRenderTarget(3,NULL);

	//set depth stencil
	if(FAILED(GETD3D(CGlobals::GetRenderDevice())->SetDepthStencilSurface(m_pSMZSurface)))
		return E_FAIL;
#else
	glBindFramebuffer(GL_FRAMEBUFFER, mSMFrameBufferObject);
	
	GETGL(pd3dDevice)->BeginRenderTarget(m_shadowTexWidth, m_shadowTexHeight);
#endif

	//  set new viewport for shadow map
	ParaViewport newViewport;
	//newViewport = CGlobals::GetViewportManager()->GetActiveViewPort()->GetTextureViewport(m_shadowTexWidth, m_shadowTexHeight);
	newViewport.X = 0;
	newViewport.Y = 0;
	newViewport.Width = m_shadowTexWidth;
	newViewport.Height = m_shadowTexHeight;
	newViewport.MinZ = 0.0f;
	newViewport.MaxZ = 1.0f;

	CGlobals::GetRenderDevice()->SetViewport(newViewport);

	float depthBias = float(m_iDepthBias) / 16777215.f;

	//  render color if it is going to be displayed (or necessary for shadow map) -- otherwise don't
	if (m_bSupportsHWShadowMaps && !(m_bDisplayShadowMap || m_bBlurSMColorTexture))
	{
		pd3dDevice->SetRenderState(ERenderState::COLORWRITEENABLE, 0);
		CGlobals::GetRenderDevice()->SetClearDepth(1.0f);
		CGlobals::GetRenderDevice()->SetClearStencil(0);
		CGlobals::GetRenderDevice()->Clear(false, true, true);
	}
	else
	{
		CGlobals::GetRenderDevice()->SetClearColor(Color4f(1,1,1,1));
		CGlobals::GetRenderDevice()->SetClearDepth(1.0f);
		CGlobals::GetRenderDevice()->Clear(true, true,false);
	}

	
	pd3dDevice->SetRenderState(ERenderState::ZFUNC, D3DCMP_LESSEQUAL);
	//depth bias
	if (m_bSupportsHWShadowMaps)
	{
		pd3dDevice->SetRenderState(ERenderState::DEPTHBIAS, *(DWORD*)&depthBias);
		pd3dDevice->SetRenderState(ERenderState::SLOPESCALEDEPTHBIAS, *(DWORD*)&m_fBiasSlope);
	}

	//GETD3D(CGlobals::GetRenderDevice())->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
	
	CGlobals::GetEffectManager()->SetCullingMode(false);
	CGlobals::GetEffectManager()->DisableD3DCulling(true);

	// set the technique
	// end the current effect, so that we may change its technique
	CGlobals::GetEffectManager()->SetAllEffectsTechnique(EffectManager::EFFECT_GEN_SHADOWMAP);
	
	return S_OK;
}

HRESULT CShadowMap::EndShadowPass()
{
	CGlobals::GetEffectManager()->DisableD3DCulling(false);
	CGlobals::GetEffectManager()->SetCullingMode(true);

	// restore technique to default: 
	// TODO: should restore to a previous technique set, which may not all be default.
	CGlobals::GetEffectManager()->SetAllEffectsTechnique(EffectManager::EFFECT_DEFAULT);

	CGlobals::GetViewMatrixStack().pop();
	CGlobals::GetProjectionMatrixStack().pop();

#ifdef USE_DIRECTX_RENDERER
	auto pd3dDevice = CGlobals::GetRenderDevice();
#else
	auto pd3dDevice = CGlobals::GetRenderDevice();
#endif
	float fTemp = 0.0f;
	pd3dDevice->SetRenderState(ERenderState::DEPTHBIAS, *(DWORD*)&fTemp);
	pd3dDevice->SetRenderState(ERenderState::SLOPESCALEDEPTHBIAS, *(DWORD*)&fTemp);

#ifdef USE_DIRECTX_RENDERER
	if (m_bBlurSMColorTexture &&
		BlockWorldClient::GetInstance()->GetBlockRenderMethod() != BLOCK_RENDER_FANCY_SHADER)
	{
		/* 
		1. Before rendering the depth shadow map, clear the color buffer with 1. During the
		render, draw 0 into the color buffer for every object except the landscape; for the
		landscape, draw 1 in color. After the whole shadow map renders, we¡¯ll have 1 where
		the landscape is nonshadowed and 0 where it¡¯s shadowed. 
		2. Blur the picture (the one in Figure 14-18) severely, using multiple passes with a simple
		blur pixel shader. For example, using a simple two-pass Gaussian blur gives good
		results. (You might want to adjust the blurring radius for distant objects.) After this
		step, we¡¯ll have a high-quality blurred texture.
		3. While rendering the scene with shadows, render the landscape with the blurred texture
		instead of the shadow map, and render all other objects with the depth part of
		the shadow map. 
		*/
		// we will carry out step 2: blur the picture in this section

		EffectManager* pEffectManager =  CGlobals::GetEffectManager();
		pEffectManager->BeginEffect(TECH_SHADOWMAP_BLUR);
		CEffectFile* pEffectFile = pEffectManager->GetCurrentEffectFile();
		if(pEffectFile != 0 && pEffectFile->begin(true, 0))
		{
			// full screen blur filter is completed in two passes.
			mesh_vertex_plain quadVertices[4] = {
				{Vector3(-1,-1,0), Vector2(0,1)},
				{Vector3(1,-1,0), Vector2(1,1)},
				{Vector3(-1,1,0), Vector2(0,0)},
				{Vector3(1,1,0), Vector2(1,0)},
			};
			// offset the texture coordinate by half texel in order to match texel to pixel. 
			float fhalfTexelWidth = 0.5f/m_shadowTexWidth;
			float fhalfTexelHeight = 0.5f/m_shadowTexWidth;

			for (int i=0;i<4;++i)
			{
				quadVertices[i].uv.x += fhalfTexelWidth;
				quadVertices[i].uv.y += fhalfTexelHeight;
			}
			pEffectFile->setParameter(CEffectFile::k_ConstVector0, Vector4(1.f/m_shadowTexWidth, 1.f/m_shadowTexHeight, 0.0f, 0.0f).ptr());

			GETD3D(CGlobals::GetRenderDevice())->SetRenderTarget(0, m_pSMColorSurfaceBlurredHorizontal );
			
			GETD3D(CGlobals::GetRenderDevice())->SetDepthStencilSurface( NULL );
			if(pEffectFile->BeginPass(0))
			{
				pEffectFile->setTexture(0, m_pSMColorTexture->GetTexture());
				pEffectFile->CommitChanges();

				HRESULT hr = CGlobals::GetRenderDevice()->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP,2,quadVertices,sizeof(mesh_vertex_plain));

				pEffectFile->EndPass();
			}
			// set texture 0 to NULL so same texture is never simultaneously a source and render target
			pEffectFile->setTexture( 0, (LPDIRECT3DTEXTURE9)NULL );
			
			GETD3D(CGlobals::GetRenderDevice())->SetRenderTarget( 0, m_pSMColorSurfaceBlurredVertical );
			GETD3D(CGlobals::GetRenderDevice())->SetDepthStencilSurface( NULL );
			if(pEffectFile->BeginPass(1))
			{
				pEffectFile->setTexture(0, m_pSMColorTextureBlurredHorizontal->GetTexture());
				pEffectFile->CommitChanges();

				HRESULT hr = CGlobals::GetRenderDevice()->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP,2,quadVertices,sizeof(mesh_vertex_plain));

				pEffectFile->EndPass();
			}
			// set texture 0 to NULL so same texture is never simultaneously a source and render target
			pEffectFile->setTexture( 0, (LPDIRECT3DTEXTURE9)NULL );
			pEffectFile->end();
		}
	}
#endif
	pd3dDevice->SetRenderState(ERenderState::ZFUNC, D3DCMP_LESSEQUAL);

	//set render target back to normal back buffer / depth buffer
#ifdef USE_DIRECTX_RENDERER
	if (FAILED(CGlobals::GetDirectXEngine().SetRenderTarget(0, m_pBackBuffer)))
		return E_FAIL;
	if(FAILED(GETD3D(CGlobals::GetRenderDevice())->SetDepthStencilSurface(m_pZBuffer)))
		return E_FAIL;
	SAFE_RELEASE(m_pZBuffer);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBufferObject);
	mOldFrameBufferObject = 0;
	
	GETGL(pd3dDevice)->EndRenderTarget();
#endif

	// restore old view port
	CGlobals::GetRenderDevice()->SetViewport(oldViewport);

	//re enable color writes
	if (m_bSupportsHWShadowMaps && !(m_bDisplayShadowMap || m_bBlurSMColorTexture))
		pd3dDevice->SetRenderState(ERenderState::COLORWRITEENABLE, 0xf);
	//GETD3D(CGlobals::GetRenderDevice())->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, COLOR_ARGB(0, 60, 60, 60), 1.0f, 0L);
	//pd3dDevice->SetRenderState(ERenderState::FILLMODE, D3DFILL_SOLID);
	//GETD3D(CGlobals::GetRenderDevice())->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	return S_OK;
}

bool CShadowMap::SaveShadowMapToFile(string file)
{
#ifdef USE_DIRECTX_RENDERER
	D3DXSaveTextureToFile(file.c_str(), D3DXIFF_JPG, m_pSMColorTexture->GetTexture(), NULL);
	return true;
#else
	return false;
#endif
}
const Matrix4* CShadowMap::GetTexViewProjMatrix()
{
	return &m_textureMatrix;
}

const Matrix4* CShadowMap::GetViewProjMatrix()
{
	return &m_LightViewProj;
}

HRESULT CShadowMap::SetShadowTexture(CEffectFile& pEffect, int nTextureIndex, int nUseBlur)
{
	if(BlockWorldClient::GetInstance()->GetBlockRenderMethod() != BLOCK_RENDER_FANCY_SHADER)
	{
		if(nUseBlur==0)
		{
			auto& pTexture = (m_bSupportsHWShadowMaps)? m_pSMZTexture : m_pSMColorTexture;
			if(pTexture && FAILED(pEffect.setTexture(nTextureIndex, pTexture->GetTexture())))
				return E_FAIL;
		}
		else
		{
			if(m_pSMColorTextureBlurredVertical && FAILED(pEffect.setTexture(nTextureIndex, m_pSMColorTextureBlurredVertical->GetTexture())))
				return E_FAIL;
		}
	}
	else
	{
		auto& pTexture = (m_bSupportsHWShadowMaps) ? m_pSMZTexture : m_pSMColorTexture;
		if (pTexture && FAILED(pEffect.setTexture(nTextureIndex, pTexture->GetTexture())))
			return E_FAIL;
	}

	
	pEffect.SetShadowMapSize(GetShadowMapTexelSize());
	/* we will do this directly in shader
	GETD3D(CGlobals::GetRenderDevice())->SetTextureStageState(nTextureIndex, D3DTSS_TEXCOORDINDEX, nTextureIndex);
	GETD3D(CGlobals::GetRenderDevice())->SetTextureStageState(nTextureIndex, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED);
	*/
	/* Shadow texture have the following format
	MinFilter = Linear;  
	MagFilter = Linear;
	MipFilter = None;
	AddressU  = BORDER;
	AddressV  = BORDER;
	BorderColor = 0xffffffff;
	*/
	return S_OK;
}

void CShadowMap::AddShadowCasterPoint( const CShapeAABB& aabb )
{
	m_ShadowCasterPoints.push_back(aabb);
}

void CShadowMap::UnsetShadowTexture(int nTextureIndex)
{
	// GETD3D(CGlobals::GetRenderDevice())->SetTextureStageState(nTextureIndex, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	// IMPORTANT: if the shader does not restore sampler states, the following must be called. Since shadow map will use the 0xffffffff as the border color
	/*
	GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( nTextureIndex, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP );
	GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( nTextureIndex, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );
	*/
}
#endif
