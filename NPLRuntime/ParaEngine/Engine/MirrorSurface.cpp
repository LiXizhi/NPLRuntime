//-----------------------------------------------------------------------------
// Class:	CMirrorSurface
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.7.2
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "EffectManager.h"
#include "ParaWorldAsset.h"
#include "SortedFaceGroups.h"
#include "AutoCamera.h"

#include "MirrorSurface.h"
#include "memdebug.h"

using namespace ParaEngine;

/** @def reflection map width in pixels */
#define MAX_REFLECTION_MAP_WIDTH	512
/** @def reflection map height in pixels */
#define MAX_REFLECTION_MAP_HEIGHT	512

/** view angle overdraw for reflection map. 1.0 means the same as the current camera view angle. */
#define REFLECTION_MAP_OVERDRAW		1.0f

CMirrorSurface::CMirrorSurface(void)
	: m_bEnable(false)
	, m_bInitialized(false)
	, m_vPos(0, 0, 0)
	, m_reflectionPlane(0, 1, 0, 0)
	, m_reflectionMapOverdraw(REFLECTION_MAP_OVERDRAW)
	, m_reflectionTextureWidth(MAX_REFLECTION_MAP_WIDTH)
	, m_reflectionTextureHeight(MAX_REFLECTION_MAP_HEIGHT)
	, m_rhiReflectionRT(nullptr)
	, m_reflectViewMatrix(false)
	, m_bDisableFogInReflection(false)
{
}

CMirrorSurface::~CMirrorSurface(void)
{
	Cleanup();
}

DeviceTexturePtr_type CMirrorSurface::GetReflectionTexture()
{
	return m_rhiReflectionRT ? m_rhiReflectionRT->GetColorTexture(0) : nullptr;
}

void CMirrorSurface::Cleanup()
{
	InvalidateDeviceObjects();
}

void CMirrorSurface::SetPlane(const Plane& plane)
{
	m_reflectionPlane = plane;
}

void CMirrorSurface::InitDeviceObjects()
{
}

void CMirrorSurface::RestoreDeviceObjects()
{
	InvalidateDeviceObjects();

	auto pRenderDevice = CGlobals::GetRenderDevice();
	if (!pRenderDevice)
		return;

	int deviceWidth = (int)pRenderDevice->GetBackBufferWidth();
	int deviceHeight = (int)pRenderDevice->GetBackBufferHeight();
	if (deviceWidth <= 0)
		deviceWidth = m_reflectionTextureWidth;
	if (deviceHeight <= 0)
		deviceHeight = m_reflectionTextureHeight;

	int nWidth = (std::min)(deviceWidth, m_reflectionTextureWidth);
	int nHeight = (std::min)(deviceHeight, m_reflectionTextureHeight);
	if (nWidth <= 0 || nHeight <= 0)
		return;

	m_rhiReflectionRT = pRenderDevice->CreateRenderTarget((uint32_t)nWidth, (uint32_t)nHeight, PixelFormat::X8R8G8B8, PixelFormat::D24S8);
	if (!m_rhiReflectionRT)
	{
		OUTPUT_LOG("error: CMirrorSurface failed to create reflection render target (%dx%d)\n", nWidth, nHeight);
		return;
	}
	m_bInitialized = true;
}

void CMirrorSurface::InvalidateDeviceObjects()
{
	m_bInitialized = false;
	ReleaseRHIResource(m_rhiReflectionRT);
}

void CMirrorSurface::DeleteDeviceObjects()
{
	InvalidateDeviceObjects();
}

void CMirrorSurface::SetPosition(Vector3 vPos, bool bUseCloset)
{
	float fEyeY = CGlobals::GetSceneState()->vEye.y;
	if (vPos.y < fEyeY)
	{
		if (IsEnabled() && bUseCloset)
		{
			if (m_vPos.y < fEyeY && m_vPos.y > vPos.y)
			{
				// use old position
			}
			else
				m_vPos = vPos;
		}
		else
			m_vPos = vPos;

		SetEnabled(true);
	}
}

void CMirrorSurface::SetMatrices(bool bPostPushMatrices, bool bPrePopMatrices)
{
	// pop matrices
	if (bPrePopMatrices)
	{
		CGlobals::GetViewMatrixStack().pop();
	}

	Matrix4 viewMatrix;

	// Reflect the view matrix if asked
	if (m_reflectViewMatrix)
	{
		CAutoCamera* pCamera = ((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()));
		Vector3 vUp(0, -1, 0);
		DVector3 vEye = pCamera->GetEyePosition();
		vEye.y = m_vPos.y * 2 - vEye.y;
		DVector3 vLookAt = pCamera->GetLookAtPosition();
		vLookAt.y = m_vPos.y * 2 - vLookAt.y;

		pCamera->ComputeViewMatrix(&viewMatrix, &vEye, &vLookAt, &vUp);
	}
	else
	{
		viewMatrix = CGlobals::GetViewMatrixStack().SafeGetTop();
	}

	// push matrices
	if (bPostPushMatrices)
	{
		CGlobals::GetViewMatrixStack().push(viewMatrix);
	}

	CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true, true, true);
}

void CMirrorSurface::RenderReflectionTexture()
{
	if (!m_bEnable)
		return;
	if (CGlobals::GetSceneState()->vEye.y >= m_vPos.y)
	{
		if (!m_bInitialized)
		{
			InitDeviceObjects();
			RestoreDeviceObjects();
		}
		auto pRenderDevice = CGlobals::GetRenderDevice();
		if (!pRenderDevice || !m_rhiReflectionRT)
		{
			SetEnabled(false);
			return;
		}

		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		Plane clipPlane;
		Plane transformedClipPlane;
		float reflectionMapFieldOfView;

		ParaViewport oldViewport = pRenderDevice->GetViewport();
		ParaViewport reflViewport;
		reflViewport.X = 0;
		reflViewport.Y = 0;
		reflViewport.Width = m_rhiReflectionRT->GetWidth();
		reflViewport.Height = m_rhiReflectionRT->GetHeight();
		reflViewport.MinZ = 0.0f;
		reflViewport.MaxZ = 1.0f;

		//////////////////////////////////////////////////////////////////////////
		// Render to the reflection map
		pRenderDevice->PushRenderTarget(m_rhiReflectionRT);
		pRenderDevice->BeginRenderTarget(m_rhiReflectionRT);
		pRenderDevice->SetViewport(reflViewport);

		// Compute the field of view and use it
		CAutoCamera* pCamera = ((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()));
		reflectionMapFieldOfView = 2.0f * atan((tan(pCamera->GetFieldOfView() / 2.0f)) * m_reflectionMapOverdraw);
		ParaMatrixPerspectiveFovLH(&m_reflectionProjectionMatrix, reflectionMapFieldOfView, pCamera->GetAspectRatio(), pCamera->GetNearPlane(), pCamera->GetFarPlane());

		CGlobals::GetProjectionMatrixStack().push(m_reflectionProjectionMatrix);
		m_reflectViewMatrix = true;
		SetMatrices(true, false);
		if (m_bDisableFogInReflection)
			pEffectManager->EnableFog(false);

		pCamera->UpdateFrustum();

		// Set up the clip plane to draw the correct geometry
		clipPlane = m_reflectionPlane;
		Vector3 vRenderOrig = CGlobals::GetScene()->GetRenderOrigin();
		clipPlane.d -= (m_vPos.y - vRenderOrig.y);

		Matrix4 tempMatrix = CGlobals::GetViewMatrixStack().SafeGetTop();
		tempMatrix *= CGlobals::GetProjectionMatrixStack().SafeGetTop();
		tempMatrix = tempMatrix.inverse();
		tempMatrix = tempMatrix.transpose();
		transformedClipPlane = clipPlane.PlaneTransform(tempMatrix);

		pEffectManager->SetClipPlane(0, (const float*)&clipPlane, false);
		pEffectManager->SetClipPlane(0, (const float*)&transformedClipPlane, true);
		pEffectManager->EnableClipPlane(true);

		pRenderDevice->SetClearColor(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
		pRenderDevice->SetClearDepth(1.0f);
		pRenderDevice->Clear(true, true, false);

		//////////////////////////////////////////////////////////////////////////
		// Render the actual scene to reflection texture:
		//////////////////////////////////////////////////////////////////////////
		bool bUseReflection = pEffectManager->IsReflectionRenderingEnabled();
		if (bUseReflection)
			pEffectManager->EnableReflectionRendering(false);

		CGlobals::GetScene()->RenderSelection(RENDER_SKY_BOX);
		pEffectManager->EnableClipPlane(true);
		CGlobals::GetScene()->RenderSelection(RENDER_PLAYER);
		CGlobals::GetScene()->RenderSelection(RENDER_MESH_FRONT_TO_BACK);
		CGlobals::GetScene()->RenderSelection(RENDER_MESH_TRANSPARENT);

		if (!CGlobals::GetSceneState()->GetFaceGroups()->IsEmpty())
		{
			CGlobals::GetScene()->RenderSelection(RENDER_TRANSLUCENT_FACE_GROUPS);
			CGlobals::GetSceneState()->GetFaceGroups()->Clear();
		}

		//////////////////////////////////////////////////////////////////////////
		// Restore state for the normal pipeline
		pEffectManager->EnableClipPlane(false);
		if (m_bDisableFogInReflection)
			pEffectManager->EnableFog(CGlobals::GetScene()->IsFogEnabled());
		CGlobals::GetProjectionMatrixStack().pop();
		m_reflectViewMatrix = false;

		SetMatrices(false, true);
		pCamera->UpdateFrustum();

		pRenderDevice->EndRenderTarget();
		pRenderDevice->PopRenderTarget();
		pRenderDevice->SetViewport(oldViewport);

		if (bUseReflection)
			pEffectManager->EnableReflectionRendering(true);
	}

	// always disable the mirror when the reflection map is updated. call GetReflectionTexture() to enable it.
	SetEnabled(false);
}
