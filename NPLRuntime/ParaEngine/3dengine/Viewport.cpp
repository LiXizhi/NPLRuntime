//-----------------------------------------------------------------------------
// Class:	Viewport
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8.8
// Desc: managing all view ports
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "2dengine/GUIRoot.h"
#include "ParaWorldAsset.h"
#include "MoviePlatform.h"
#include "ViewportManager.h"
#include "NPLRuntime.h"
#include "RenderTarget.h"
#include "Viewport.h"

#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#endif

using namespace ParaEngine;

CViewport::CViewport(CViewportManager* pViewportManager)
	:m_position(), m_pScene(NULL), m_pGUIRoot(NULL), m_pViewportManager(pViewportManager), m_fScalingX(1.f), m_fScalingY(1.f), m_fAspectRatio(1.f), m_bIsModifed(true), m_nZOrder(0), m_bIsEnabled(true), m_nEyeMode(STEREO_EYE_NORMAL), m_nPipelineOrder(-1), m_bDisableDeltaTime(false)
{
	memset(&m_rect, 0, sizeof(m_rect));
}

CViewport::~CViewport(void)
{
	m_pRenderTarget.reset();
}

void ParaEngine::CViewport::SetCamera(CAutoCamera* val) 
{ 
	m_pCamera = val;
}

bool ParaEngine::CViewport::IsUseSceneCamera()
{
	return !m_pCamera;
}

void ParaEngine::CViewport::SetUseSceneCamera(bool bUseSceneCamera)
{
	if (bUseSceneCamera)
	{
		m_pCamera.reset();
	}
	else
	{
		if (!m_pCamera)
		{
			m_pCamera = new CAutoCamera();
			// update camera from current camera settings
			CBaseCamera* pFromCamera = CGlobals::GetScene()->GetCurrentCamera();
			m_pCamera->CopyCameraParamsFrom(pFromCamera);
			m_pCamera->UpdateProjParams();
			if (!m_pScene)
			{
				m_pScene = CGlobals::GetScene();
			}
		}
	}
}

CAutoCamera* ParaEngine::CViewport::GetCamera()
{
	if (!m_pCamera)
	{
		if (GetScene())
		{
			return ((CAutoCamera*)GetScene()->GetCurrentCamera());
		}
	}
	return m_pCamera.get();
}

void ParaEngine::CViewport::ApplyCamera(CAutoCamera* pCamera)
{
	pCamera->SetAspectRatio(GetAspectRatio());
	if (GetEyeMode() == STEREO_EYE_LEFT || GetEyeMode() == STEREO_EYE_RIGHT || GetEyeMode() == STEREO_EYE_ODS)
	{
		pCamera->EnableStereoVision(true);
		if (GetEyeMode() != STEREO_EYE_ODS) {
			pCamera->SetStereoEyeShiftDistance(GetEyeMode() == STEREO_EYE_LEFT ? -GetStereoEyeSeparation() : GetStereoEyeSeparation());
		}
		if (m_stereoODSparam.isODS) {
			pCamera->SetStereoEyeShiftDistance(m_stereoODSparam.eyeShiftDistance);
			DVector3 dEyePos = m_stereoODSparam.oldEyePos;
			DVector3 oldLookAtPos = m_stereoODSparam.oldLookAtPos;

			if (m_stereoODSparam.m_nOmniForceLookatDistance>0) {
				pCamera->SetForceOmniCameraObjectDistance(m_stereoODSparam.m_nOmniForceLookatDistance);
			}
			double fCameraObjectDist = pCamera->GetCameraObjectDistance();
			if (m_stereoODSparam.m_bOmniAlwaysUseUpFrontCamera) {
				pCamera->SetForceOmniCameraPitch(0);
				//始终水平，将oldLookAtPos旋转到水平方向上
				{
					Vector3 sightDir = oldLookAtPos - dEyePos;
					sightDir.normalise();

					Vector3 newSightDir = sightDir;
					newSightDir.y = 0;
					newSightDir.normalise();
					DVector3 newLookAt = DVector3(newSightDir * fCameraObjectDist) + dEyePos;

					oldLookAtPos = newLookAt;
				}				
			}
			pCamera->UpdateViewMatrix();			
			m_stereoODSparam.needRecoverCamera = true;

			Vector3 up = pCamera->GetWorldUp();
			if (m_stereoODSparam.m_bOmniAlwaysUseUpFrontCamera) {
				up = Vector3(0, 1, 0);
			}
			up.normalise();
			Vector3 right = pCamera->GetWorldRight();
			right.normalise();

			Matrix3 mRotPitch;
			Matrix3 mRotYaw;
			Matrix3 mRotRoll;

			//if (abs(m_stereoODSparam.moreRotX) > 0.00001f)//pitch 
			{
				
				Quaternion q_pitch(Radian(m_stereoODSparam.moreRotX), right);
				q_pitch.ToRotationMatrix(mRotPitch);

				Vector3 sightDir = oldLookAtPos - dEyePos;
				sightDir.normalise();

				Vector3 newSightDir = Vector3(mRotPitch * sightDir);
				DVector3 newLookAt = DVector3(newSightDir * fCameraObjectDist) + dEyePos;
				
				//pCamera->SetViewParams(dEyePos, newLookAt, &up);
				oldLookAtPos = newLookAt;
			}

			/*auto m_mCameraWorld = pCamera->GetViewMatrix()->inverse();
			up = Vector3(m_mCameraWorld._21, m_mCameraWorld._22, m_mCameraWorld._23);*/
			
			//if(abs(m_stereoODSparam.moreRotY)>0.00001f) //yaw
			{
				Quaternion q_yaw(Radian(m_stereoODSparam.moreRotY), up);
				q_yaw.ToRotationMatrix(mRotYaw);

				Vector3 sightDir = oldLookAtPos - dEyePos;
				sightDir.normalise(); 
				Vector3 newSightDir = Vector3(mRotYaw * sightDir);
				DVector3 newLookAt = DVector3(newSightDir * fCameraObjectDist) + dEyePos;

				//pCamera->SetViewParams(dEyePos, newLookAt, &up);
				oldLookAtPos = newLookAt;
			}
			
			//if (abs(m_stereoODSparam.moreRotZ) > 0.00001f) //roll
			{
				if (abs(m_stereoODSparam.moreRotZ - (-1.57))<0.0001) {
					int i = 0;
				}
				Quaternion q_roll(Radian(m_stereoODSparam.moreRotZ), up);
				float oldYaw = pCamera->GetCameraRotY();
				up = Vector3(mRotPitch * up);
				up.normalise();
				
				q_roll.ToRotationMatrix(mRotRoll);
				up = Vector3(mRotRoll * up);
				up.normalise();
			}
			pCamera->SetViewParams(dEyePos, oldLookAtPos, &up);
			pCamera->SetFieldOfView(m_stereoODSparam.fov, m_stereoODSparam.fov_h);
			
		}
		else {
			pCamera->UpdateViewMatrix();
		}
	}
	else
	{
		pCamera->EnableStereoVision(false);
		pCamera->SetStereoEyeShiftDistance(0.f);
	}
}

const std::string& ParaEngine::CViewport::GetRenderScript() const
{
	return m_sRenderScript;
}

void ParaEngine::CViewport::SetRenderScript(const std::string&& val)
{
	m_sRenderScript = val;
}

int ParaEngine::CViewport::GetPipelineOrder() const
{
	return m_nPipelineOrder;
}

void ParaEngine::CViewport::SetPipelineOrder(int val)
{
	m_nPipelineOrder = val;
}

const std::string& ParaEngine::CViewport::GetRenderTargetName() const
{
	return m_sRenderTargetName;
}

CRenderTarget* ParaEngine::CViewport::GetRenderTarget()
{
	return m_pRenderTarget.get();
}

void ParaEngine::CViewport::SetRenderTarget(CRenderTarget* target)
{
	m_pRenderTarget = target;
}

void ParaEngine::CViewport::SetRenderTargetName(const std::string& val)
{
	if (m_sRenderTargetName != val)
	{
		m_sRenderTargetName = val;
		//SAFE_DELETE(m_pRenderTarget);
		m_pRenderTarget.reset();
		if (!m_sRenderTargetName.empty()){

			CRenderTarget* pRenderTarget = static_cast<CRenderTarget*>(CGlobals::GetScene()->FindObjectByNameAndType(m_sRenderTargetName, "CRenderTarget"));
			if (pRenderTarget)
			{
				pRenderTarget->SetLifeTime(-1);
				m_pRenderTarget = pRenderTarget;
			}
			else
			{
				// create one if not exist. 
				CRenderTarget* pRenderTarget = new CRenderTarget();
				pRenderTarget->SetIdentifier(m_sRenderTargetName);
				CGlobals::GetScene()->AttachObject(pRenderTarget);
				pRenderTarget->SetDirty(false);
				pRenderTarget->SetVisibility(false);
				m_pRenderTarget = pRenderTarget;
			}
			m_pRenderTarget->SetCanvasTextureName(val);
		}
	}
}

HRESULT ParaEngine::CViewport::Render(double dTimeDelta, int nPipelineOrder)
{
	if (!IsEnabled() || (GetPipelineOrder() >= 0 && GetPipelineOrder() != nPipelineOrder))
		return S_OK;

	if (IsDeltaTimeDisabled())
	{
		dTimeDelta = 0.f;
	}

	if (nPipelineOrder == PIPELINE_3D_SCENE || nPipelineOrder == PIPELINE_POST_UI_3D_SCENE)
	{
		CSceneObject* pRootScene = GetScene();
		CAutoCamera* pCamera = GetCamera();
		if (pRootScene && pCamera)
		{
			UpdateRect();
			if (m_pRenderTarget)
			{
#ifdef USE_DIRECTX_RENDERER
				// for multiple render targets (/shader 2 of fancy block rendering) to work, the render target must be the same size of the back buffer. 
				m_pRenderTarget->SetRenderTargetSize(Vector2((float)(CGlobals::GetDirectXEngine().GetBackBufferWidth()), (float)(CGlobals::GetDirectXEngine().GetBackBufferHeight())));
#else
				m_pRenderTarget->SetRenderTargetSize(Vector2((float)GetWidth(), (float)GetHeight()));
				// m_pRenderTarget->SetRenderTargetSize(Vector2((float)m_pViewportManager->GetWidth(), (float)m_pViewportManager->GetHeight()));
				
#endif
				m_pRenderTarget->GetPrimaryAsset(); // touch asset
			}

			//ScopedPaintOnRenderTarget painter_(m_pRenderTarget.get());
			ScopedPaintOnRenderTarget painter_(m_pRenderTarget.get(), m_rect.left, m_rect.top, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
			
			if (m_pRenderTarget && nPipelineOrder == PIPELINE_3D_SCENE)
			{
				if (m_stereoODSparam.ods_group_size <= 0 || (m_stereoODSparam.ods_group_size > 0 && m_stereoODSparam.ods_group_idx == 0)) {
					m_pRenderTarget->Clear(pRootScene->GetFogColor());
				}
			}
			
			auto pLastCamera = pRootScene->GetCurrentCamera();
			pRootScene->SetCurrentCamera(pCamera);

			SetActive();
			ApplyCamera(pCamera);
			ApplyViewport();
			
			auto vEye = pCamera->GetEyePosition();
			Vector3 vEye_ = vEye;
			pRootScene->RegenerateRenderOrigin(vEye_);

			if (pRootScene->IsSceneEnabled())
			{
				//-- set up effects parameters
				// Light direction is same as camera front (reversed)
				pCamera->UpdateViewProjMatrix();
				
				// Draw next scene
				if (pRootScene->IsScenePaused())
					pRootScene->AdvanceScene(0);
				else
				{
					pRootScene->AdvanceScene(dTimeDelta, nPipelineOrder);
				}
			}
			else
			{
				// even main scene is not enabled, we may still need to render mini-scene graph
				/// Scene root: Draw next scene
				if (pRootScene->IsScenePaused())
					pRootScene->AdvanceScene(0, nPipelineOrder);
				else
				{
					pRootScene->AdvanceScene(dTimeDelta, nPipelineOrder);
				}
			}
			pRootScene->SetCurrentCamera(pLastCamera);
		}
	}
	else if (nPipelineOrder == PIPELINE_UI)
	{
		if (GetGUIRoot())
		{
			SetActive();
			// SetViewport(0, 0, m_pViewportManager->GetWidth(), m_pViewportManager->GetHeight());
			ApplyViewport();
			GetGUIRoot()->UpdateViewport(GetLeft(), GetTop(), GetWidth(), GetHeight());
			GetGUIRoot()->AdvanceGUI((float)dTimeDelta);
		}
	}

	if (!GetRenderScript().empty())
	{
		SetActive();
		ApplyViewport();
		CGlobals::GetNPLRuntime()->GetMainRuntimeState()->DoString(GetRenderScript().c_str(), (int)GetRenderScript().size());
	}
	return S_OK;
}

int ParaEngine::CViewport::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);
	PE_ASSERT(pClass != NULL);

	pClass->AddField("alignment", FieldType_String, (void*)SetAlignment_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("left", FieldType_Int, (void*)SetLeft_s, (void*)GetLeft_s, NULL, NULL, bOverride);
	pClass->AddField("ods_fov", FieldType_Float, (void*)SetODSFov_s, (void*)GetODSFov_s, NULL, NULL, bOverride);
	pClass->AddField("top", FieldType_Int, (void*)SetTop_s, (void*)GetTop_s, NULL, NULL, bOverride);
	pClass->AddField("width", FieldType_Int, (void*)SetWidth_s, (void*)GetWidth_s, NULL, NULL, bOverride);
	pClass->AddField("height", FieldType_Int, (void*)SetHeight_s, (void*)GetHeight_s, NULL, NULL, bOverride);
	pClass->AddField("ApplyViewport", FieldType_void, (void*)ApplyViewport_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("RenderScript", FieldType_String, (void*)SetRenderScript_s, (void*)GetRenderScript_s, NULL, NULL, bOverride);
	pClass->AddField("RenderTargetName", FieldType_String, (void*)SetRenderTargetName_s, (void*)GetRenderTargetName_s, NULL, NULL, bOverride);
	pClass->AddField("PipelineOrder", FieldType_Int, (void*)SetPipelineOrder_s, (void*)GetPipelineOrder_s, NULL, NULL, bOverride);
	pClass->AddField("UseSceneCamera", FieldType_Bool, (void*)SetUseSceneCamera_s, (void*)IsUseSceneCamera_s, NULL, NULL, bOverride);
	pClass->AddField("enabled", FieldType_Bool, (void*)SetIsEnabled_s, (void*)IsEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("zorder", FieldType_Int, (void*)SetZOrder_s, (void*)GetZOrder_s, NULL, NULL, bOverride);
	pClass->AddField("IsDeltaTimeDisabled", FieldType_Bool, (void*)DisableDeltaTime_s, (void*)IsDeltaTimeDisabled_s, NULL, NULL, bOverride);
	return S_OK;
}

IAttributeFields* ParaEngine::CViewport::GetChildAttributeObject(const char* sName)
{
	if (std::string(sName) == "camera")
	{
		return GetCamera();
	}
	return NULL;
}

int ParaEngine::CViewport::GetChildAttributeObjectCount(int nColumnIndex)
{
	return 1;
}

IAttributeFields* ParaEngine::CViewport::GetChildAttributeObject(int nRowIndex, int nColumnIndex)
{
	if(nRowIndex == 0 && nColumnIndex == 0)
	{
		return GetCamera();
	}
	return nullptr;
}

void ParaEngine::CViewport::OnParentSizeChanged(int nWidth, int nHeight)
{
	RECT parent = { 0, 0, nWidth, nHeight };
	m_position.CalculateAbsPosition(&m_rect, &parent);
	m_fScalingX = GetWidth()!=nWidth ? ((float)GetWidth() / (float)nWidth) : 1.f;
	m_fScalingY = GetHeight()!=nHeight ? ((float)GetHeight() / (float)nHeight) : 1.f;
	m_fAspectRatio = (float)GetWidth() / (float)GetHeight();
	m_bIsModifed = false;

	if (GetGUIRoot())
	{
		GetGUIRoot()->UpdateViewport(GetLeft(), GetTop(), GetWidth(), GetHeight(), true);
		// ensure minimum screen size, with largest UI scaling 
		GetGUIRoot()->SetUIScale(1, 1, true, true);
	}
}

float ParaEngine::CViewport::GetAspectRatio()
{
	return m_fAspectRatio;
}

bool ParaEngine::CViewport::IsPointOnViewport(int x, int y)
{
	return (x >= m_rect.left && y >= m_rect.top && x <= m_rect.right && y <= m_rect.bottom);
}

int ParaEngine::CViewport::GetWidth()
{
	return m_rect.right - m_rect.left;
}

int ParaEngine::CViewport::GetHeight()
{
	return m_rect.bottom - m_rect.top;
}

void ParaEngine::CViewport::GetPointOnViewport(int& x, int& y, int* pWidth, int* pHeight)
{
	x -= m_rect.left;
	y -= m_rect.top;
	
	if (pWidth)
		*pWidth = GetWidth();
	if (pHeight)
		*pHeight = GetHeight();
}

void ParaEngine::CViewport::SetPosition(const char* alignment, int left, int top, int width, int height)
{
	m_position.Reposition(alignment, left, top, width, height);
	SetModified();

}

void ParaEngine::CViewport::SetAlignment(const char* alignment)
{
	m_position.SetAlignment(alignment);
	SetModified();
}

void ParaEngine::CViewport::SetLeft(int left)
{
	m_position.SetLeft(left);
	SetModified();
}

void ParaEngine::CViewport::SetTop(int top)
{
	m_position.SetTop(top);
	SetModified();
}

void ParaEngine::CViewport::SetWidth(int width)
{
	m_position.SetWidth(width);
	SetModified();
}

void ParaEngine::CViewport::SetHeight(int height)
{
	m_position.SetHeight(height);
	SetModified();
}

ParaViewport ParaEngine::CViewport::ApplyViewport()
{
	UpdateRect();
	return SetViewport(m_rect.left, m_rect.top, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
}

void ParaEngine::CViewport::GetViewportTransform(Vector2* pvScale, Vector2* pvOffset /*= NULL*/)
{
	float fWidth = (float)m_pViewportManager->GetWidth();
	float fHeight = (float)m_pViewportManager->GetHeight();
	if (pvScale)
	{
		*pvScale = Vector2((m_rect.right - m_rect.left) / fWidth, (m_rect.bottom - m_rect.top) / fHeight);
	}
	if (pvOffset)
	{
		*pvOffset = Vector2(m_rect.left / fWidth, m_rect.top / fHeight);
	}
}

bool ParaEngine::CViewport::IsDeltaTimeDisabled()
{
	return m_bDisableDeltaTime;
}

void ParaEngine::CViewport::DisableDeltaTime(bool bDisabled)
{
	m_bDisableDeltaTime = bDisabled;
}

bool ParaEngine::CViewport::DrawQuad()
{
	float fWidth = (float)m_pViewportManager->GetWidth();
	float fHeight = (float)m_pViewportManager->GetHeight();

	ApplyViewport();
	
	float fX = m_rect.left / fWidth;
	float fY = m_rect.top / fHeight;
	float fRight = m_rect.right / fWidth;
	float fBottom = m_rect.bottom / fHeight;

	/*mesh_vertex_plain quadVertices[4] = {
		{ Vector3(fX*2-1, -fBottom*2+1, 0), Vector2(fX, fBottom) },
		{ Vector3(fRight * 2 - 1, -fBottom * 2 + 1, 0), Vector2(fRight, fBottom) },
		{ Vector3(fX*2 - 1, -fY*2+1, 0), Vector2(fX, fY) },
		{ Vector3(fRight * 2 - 1, -fY * 2 + 1, 0), Vector2(fRight, fY) },
	};*/
	mesh_vertex_plain quadVertices[4] = {
		{ Vector3(-1, -1, 0), Vector2(fX, fBottom) },
		{ Vector3(1, -1, 0), Vector2(fRight, fBottom) },
		{ Vector3(-1, 1, 0), Vector2(fX, fY) },
		{ Vector3(1, 1, 0), Vector2(fRight, fY) },
	};

	// this is done in shader code
	//
	// offset the texture coordinate by half texel in order to match texel to pixel. 
	// This takes me hours to figure out. :-(
	// float fhalfTexelWidth = 0.5f/m_glowtextureWidth;
	// float fhalfTexelHeight = 0.5f/m_glowtextureHeight;
	bool bSucceed = false;
#ifdef USE_DIRECTX_RENDERER
	bSucceed = SUCCEEDED(RenderDevice::DrawPrimitiveUP(CGlobals::GetRenderDevice(), RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, D3DPT_TRIANGLESTRIP, 2, quadVertices, sizeof(mesh_vertex_plain)));
#endif

	return bSucceed;
}

bool ParaEngine::CViewport::DrawQuad2()
{
	bool bSucceed = false;
#ifdef USE_DIRECTX_RENDERER
	D3DVIEWPORT9 old_vp;
	CGlobals::GetRenderDevice()->GetViewport(&old_vp);
	IDirect3DSurface9 * rt = nullptr;
	CGlobals::GetRenderDevice()->GetRenderTarget(0, &rt);
	D3DSURFACE_DESC desc;
	rt->GetDesc(&desc);
	SAFE_RELEASE(rt);
	D3DVIEWPORT9 vp = { 0 };
	vp.Height = desc.Height;
	vp.Width = desc.Width;
	vp.MaxZ = 1.0f;
	CGlobals::GetRenderDevice()->SetViewport(&vp);
	mesh_vertex_plain quadVertices[4] = {
		{ Vector3(-1, -1, 0), Vector2(0, 1) },
		{ Vector3(1, -1, 0), Vector2(1, 1) },
		{ Vector3(-1, 1, 0), Vector2(0, 0) },
		{ Vector3(1, 1, 0), Vector2(1, 0) },
	};

	bSucceed = SUCCEEDED(RenderDevice::DrawPrimitiveUP(CGlobals::GetRenderDevice(), RenderDevice::DRAW_PERF_TRIANGLES_UNKNOWN, D3DPT_TRIANGLESTRIP, 2, quadVertices, sizeof(mesh_vertex_plain)));
	CGlobals::GetRenderDevice()->SetViewport(&old_vp);
#endif      
	return bSucceed;
}

void ParaEngine::CViewport::SetModified()
{
	m_bIsModifed = true;
}


void ParaEngine::CViewport::UpdateRect()
{
	if (m_bIsModifed)
	{
		m_bIsModifed = false;
		OnParentSizeChanged(m_pViewportManager->GetWidth(), m_pViewportManager->GetHeight());
	}
}

ParaViewport ParaEngine::CViewport::GetTextureViewport(float fTexWidth, float fTexHeight)
{
	UpdateRect();
	ParaViewport newViewport;
	float fWidth = (float)m_pViewportManager->GetWidth();
	float fHeight = (float)m_pViewportManager->GetHeight();
	float fX = m_rect.left / fWidth;
	float fY = m_rect.top / fHeight;
	float fRight = m_rect.right / fWidth;
	float fBottom = m_rect.bottom / fHeight;

	newViewport.X = (int)(fX*fTexWidth);
	newViewport.Y = (int)(fY*fTexHeight);
	newViewport.Width = (int)(fRight*fTexHeight - newViewport.X);
	newViewport.Height = (int)(fBottom*fTexWidth - newViewport.Y);
	newViewport.MinZ = 0.0f;
	newViewport.MaxZ = 1.0f;
	return newViewport;
}

int ParaEngine::CViewport::GetZOrder() const
{
	return m_nZOrder;
}

void ParaEngine::CViewport::SetZOrder(int val)
{
	m_nZOrder = val;
}

bool ParaEngine::CViewport::IsEnabled() const
{
	return m_bIsEnabled;
}

void ParaEngine::CViewport::SetIsEnabled(bool val)
{
	m_bIsEnabled = val;
}

int ParaEngine::CViewport::GetLeft()
{
	return m_rect.left;
}

int ParaEngine::CViewport::GetTop()
{
	return m_rect.top;
}

ParaEngine::STEREO_EYE ParaEngine::CViewport::GetEyeMode() const
{
	return m_nEyeMode;
}

void ParaEngine::CViewport::SetEyeMode(ParaEngine::STEREO_EYE val)
{
	m_nEyeMode = val;
}

/** Camera yaw angle increment when recording Stereo video.*/
void ParaEngine::CViewport::SetStereoODSparam(ParaEngine::CViewport::StereoODSparam& param)
{ 
	m_stereoODSparam = param; 
}

ParaEngine::CViewport::StereoODSparam& ParaEngine::CViewport::GetStereoODSparam()
{
	return m_stereoODSparam; 
}

float ParaEngine::CViewport::GetStereoEyeSeparation()
{
	return CGlobals::GetMoviePlatform()->GetStereoEyeSeparation();
}

void ParaEngine::CViewport::SetActive()
{
	m_pViewportManager->SetActiveViewPort(this);
}


ParaViewport ParaEngine::CViewport::SetViewport(DWORD x, DWORD y, DWORD width, DWORD height)
{
	auto CurrentViewport = CGlobals::GetRenderDevice()->GetViewport();

	CurrentViewport.X = x;
	CurrentViewport.Y = y;
	CurrentViewport.Width = width;
	CurrentViewport.Height = height;

	if (!m_pRenderTarget)
	{
		// for back buffer
		if (CGlobals::GetApp()->IsRotateScreen())
		{
			RECT clipRect;
			RECT clipRect2 = m_rect;
			CurrentViewport.X = y;
			uint32_t viewWidth = CGlobals::GetViewportManager()->GetWidth();
			CurrentViewport.Y = viewWidth - (x + width);
			CurrentViewport.Width = height;
			CurrentViewport.Height = width;
		}
	}

	CGlobals::GetRenderDevice()->SetViewport(CurrentViewport);
	return CurrentViewport;
}

const std::string& ParaEngine::CViewport::GetIdentifier()
{
	return m_sName;
}

void ParaEngine::CViewport::SetIdentifier(const std::string& sID)
{
	m_sName = sID;
}



