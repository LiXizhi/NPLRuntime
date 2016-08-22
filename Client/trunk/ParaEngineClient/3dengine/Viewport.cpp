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

using namespace ParaEngine;

CViewport::CViewport(CViewportManager* pViewportManager)
	:m_position(), m_pScene(NULL), m_pCamera(NULL), m_pGUIRoot(NULL), m_pViewportManager(pViewportManager), m_fScalingX(1.f), m_fScalingY(1.f), m_fAspectRatio(1.f), m_bIsModifed(true), m_nZOrder(0), m_bIsEnabled(true), m_nEyeMode(STEREO_EYE_NORMAL), m_nPipelineOrder(-1), m_pRenderTarget(NULL)
{
	memset(&m_rect, 0, sizeof(m_rect));
}

CViewport::~CViewport(void)
{
	SAFE_DELETE(m_pRenderTarget);
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
	return m_pCamera;
}

void ParaEngine::CViewport::ApplyCamera(CAutoCamera* pCamera)
{
	pCamera->SetAspectRatio(GetAspectRatio());
	if (GetEyeMode() == STEREO_EYE_LEFT || GetEyeMode() == STEREO_EYE_RIGHT)
	{
		pCamera->EnableStereoVision(true);
		pCamera->SetStereoEyeShiftDistance(GetEyeMode() == STEREO_EYE_LEFT ? -GetStereoEyeSeparation() : GetStereoEyeSeparation());
		pCamera->UpdateViewMatrix();
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

void ParaEngine::CViewport::SetRenderTargetName(const std::string& val)
{
	if (m_sRenderTargetName != val)
	{
		m_sRenderTargetName = val;
		SAFE_DELETE(m_pRenderTarget);
		if (!m_sRenderTargetName.empty()){
			m_pRenderTarget = new CRenderTarget();
			m_pRenderTarget->SetCanvasTextureName(val);
		}
	}
}

HRESULT ParaEngine::CViewport::Render(double dTimeDelta, int nPipelineOrder)
{
	if (!IsEnabled() || (GetPipelineOrder() >= 0 && GetPipelineOrder() != nPipelineOrder))
		return S_OK;

	if (GetEyeMode() == STEREO_EYE_RIGHT)
	{
		dTimeDelta = 0.f;
	}

	if (nPipelineOrder == PIPELINE_3D_SCENE || nPipelineOrder == PIPELINE_POST_UI_3D_SCENE)
	{
		CSceneObject* pRootScene = GetScene();
		CAutoCamera* pCamera = GetCamera();
		if (pRootScene && pCamera)
		{
			if (m_pRenderTarget)
			{
				m_pRenderTarget->SetRenderTargetSize(Vector2((float)GetWidth(), (float)GetHeight()));
				m_pRenderTarget->GetPrimaryAsset(); // touch asset
			}
			ScopedPaintOnRenderTarget painter_(m_pRenderTarget);
			if (m_pRenderTarget && nPipelineOrder == PIPELINE_3D_SCENE)
			{
				m_pRenderTarget->Clear(pRootScene->GetFogColor());
			}

			SetActive();
			ApplyCamera(pCamera);
			ApplyViewport();

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
	pClass->AddField("left", FieldType_Int, (void*)SetLeft_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("top", FieldType_Int, (void*)SetTop_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("width", FieldType_Int, (void*)SetWidth_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("height", FieldType_Int, (void*)SetHeight_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("ApplyViewport", FieldType_void, (void*)ApplyViewport_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("RenderScript", FieldType_String, (void*)SetRenderScript_s, (void*)GetRenderScript_s, NULL, NULL, bOverride);
	pClass->AddField("RenderTargetName", FieldType_String, (void*)SetRenderTargetName_s, (void*)GetRenderTargetName_s, NULL, NULL, bOverride);
	pClass->AddField("PipelineOrder", FieldType_Int, (void*)SetPipelineOrder_s, (void*)GetPipelineOrder_s, NULL, NULL, bOverride);
	return S_OK;
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
		GetGUIRoot()->SetUIScale(1, 1, true);
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
	ParaViewport CurrentViewport;
	CGlobals::GetRenderDevice()->GetViewport(reinterpret_cast<D3DVIEWPORT9*>(&CurrentViewport));
	ParaViewport myViewport = CurrentViewport;
	myViewport.X = x;
	myViewport.Y = y;
	myViewport.Width = width;
	myViewport.Height = height;
	CGlobals::GetRenderDevice()->SetViewport(reinterpret_cast<const D3DVIEWPORT9*>(&myViewport));
	return myViewport;
}

const std::string& ParaEngine::CViewport::GetIdentifier()
{
	return m_sName;
}

void ParaEngine::CViewport::SetIdentifier(const std::string& sID)
{
	m_sName = sID;
}



