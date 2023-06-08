//-----------------------------------------------------------------------------
// Class:	BufferPicking
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2015.8.13
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ViewportManager.h"
#include "2dengine/GUIRoot.h"
#include "BaseCamera.h"
#include "EffectManager.h"
#include "SceneObject.h"
#include "RenderTarget.h"
#include "PaintEngine/Painter.h"
#include "BufferPicking.h"

#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#endif

using namespace ParaEngine;

ParaEngine::CBufferPicking::CBufferPicking(const AssetKey& key)
	: AssetEntity(key), m_bResultDirty(0), m_currentPickIndex(0), m_nViewport(-1)
{
	m_pickRect.setRect(0, 0, -1, -1);
}

ParaEngine::CBufferPicking::~CBufferPicking()
{

}

void ParaEngine::CBufferPicking::ClearPickingResult()
{
	SetResultDirty(true);
	m_pickingResult.clear();
}

HRESULT ParaEngine::CBufferPicking::InitDeviceObjects()
{
	ClearPickingResult();
	return S_OK;
}

HRESULT ParaEngine::CBufferPicking::DeleteDeviceObjects()
{
	ClearPickingResult();
	return S_OK;
}

DWORD ParaEngine::CBufferPicking::GetPickingID(int nIndex /*= 0*/)
{
	CheckDoPick();
	if (nIndex < 0)
		nIndex = GetPickIndex();
	return GetPickingCount() > nIndex ? m_pickingResult[nIndex] : 0;
}

void ParaEngine::CBufferPicking::FetchPickingResult(DWORD ** ppResult)
{
	CheckDoPick();
	if (ppResult)
	{
		*ppResult = &m_pickingResult[0];
	}
}

int ParaEngine::CBufferPicking::GetPickingCount()
{
	CheckDoPick();
	return (int)m_pickingResult.size();
}

int ParaEngine::CBufferPicking::GetPickIndex() const
{
	return m_currentPickIndex;
}

void ParaEngine::CBufferPicking::SetPickIndex(int val)
{
	m_currentPickIndex = val;
}

void ParaEngine::CBufferPicking::SetPickLeftTop(const Vector2& vPos)
{
	QRect oldRect = m_pickRect;
	int nLastWidth = m_pickRect.width();
	int nLastHeight = m_pickRect.width();
	m_pickRect.setTopLeft(QPoint((int)vPos.x, (int)vPos.y));
	m_pickRect.setWidth(nLastWidth);
	m_pickRect.setHeight(nLastHeight);
	if (oldRect != m_pickRect)
		SetResultDirty(true);
}

void ParaEngine::CBufferPicking::SetPickWidthHeight(const Vector2& vPos)
{
	QRect oldRect = m_pickRect;
	m_pickRect.setWidth((int)vPos.x);
	m_pickRect.setHeight((int)vPos.y);
	if (oldRect != m_pickRect)
		SetResultDirty(true);
}

const Vector2& ParaEngine::CBufferPicking::GetPickLeftTop()
{
	m_cache_left_top = Vector2((float)m_pickRect.x(), (float)m_pickRect.y());
	return m_cache_left_top;
}

const Vector2& ParaEngine::CBufferPicking::GetPickWidthHeight()
{
	m_cache_width_height = Vector2((float)m_pickRect.width(), (float)m_pickRect.height());
	return m_cache_width_height;
}

bool ParaEngine::CBufferPicking::IsResultDirty() const
{
	return m_bResultDirty;
}

void ParaEngine::CBufferPicking::SetResultDirty(bool val)
{
	m_bResultDirty = val;
}

void ParaEngine::CBufferPicking::CheckDoPick(bool bForceUpdate /*= false*/)
{
	if (IsResultDirty() || bForceUpdate)
		Pick(m_pickRect, m_nViewport);
}

int ParaEngine::CBufferPicking::GetViewport() const
{
	return m_nViewport;
}

void ParaEngine::CBufferPicking::SetViewport(int val)
{
	m_nViewport = val;
}

DWORD ParaEngine::CBufferPicking::Pick(int nX, int nY, int nViewportId /*= -1*/)
{
	QRect rect(nX, nY, 1, 1);
	return (Pick(rect, nViewportId) > 0) ? GetPickingID(0) : 0;
}

int ParaEngine::CBufferPicking::Pick(const QRect& region_, int nViewportId /*= -1*/)
{

	auto pRenderDevice =CGlobals::GetRenderDevice();

	ClearPickingResult();
	SetResultDirty(false);
	QRect region = region_;
	if (!region.isValid())
		return 0;
	if (BeginBuffer())
	{
		// just in case viewport is scaled.
		float fScalingX = (float)CGlobals::GetGUI()->GetUIScalingX();
		float fScalingY = (float)CGlobals::GetGUI()->GetUIScalingY();

		if (GetIdentifier() == "overlay")
		{
			// offset by viewport of 3d scene
			CViewport* pViewport = CGlobals::GetViewportManager()->CreateGetViewPort(1);
			region.setX(region.x() + pViewport->GetLeft());
			region.setY(region.y() + pViewport->GetTop());
		}

		if (fScalingX != 1.f || fScalingY != 1.f)
		{
			int nWidth = Math::Max(1, (int)(region.width() * fScalingX + 0.5f));
			int nHeight = Math::Max(1, (int)(region.height() * fScalingY + 0.5f));
			region.setX((int)(region.x()*fScalingX));
			region.setY((int)(region.y()*fScalingY));
			region.setWidth(nWidth);
			region.setHeight(nHeight);
		}
		int nWidth = region.width();
		int nHeight = region.height();

		m_pickingResult.resize(nWidth*nHeight);

		// this detects a bug when viewport changes dynamically causing frame buffer to be incomplete
		/*if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) {
			OUTPUT_LOG("error: frame buffer not complete because GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS!\n");
		}*/

		if (!CGlobals::GetRenderDevice()->ReadPixels(region.left(), region.top(), nWidth, nHeight, (void*)(&(m_pickingResult[0]))))
			m_pickingResult.clear();
#ifdef USE_OPENGL_RENDERER
		for (auto & result : m_pickingResult)
		{
			unsigned char * byte_ptr = reinterpret_cast<unsigned char*>(&result);
			unsigned char red = *byte_ptr++;
			unsigned char green = *byte_ptr++;
			unsigned char blue = *byte_ptr++;
			unsigned char alpha = *byte_ptr;
			result = (alpha << 24) | (red << 16) | (green << 8) | (blue);
		}
#endif
		EndBuffer();
	}
	return m_pickingResult.size();
}

void ParaEngine::BufferPickingManager::SetResultDirty()
{
	// TODO:
}

BufferPickingManager& ParaEngine::BufferPickingManager::GetInstance()
{
	static BufferPickingManager s_instance;
	return s_instance;
}

IAttributeFields * ParaEngine::CBufferPicking::GetChildAttributeObject(const char * sName)
{
	if (std::string(sName) == "rendertarget")
	{
		return GetChildAttributeObject(0, 0);
	}
	return NULL;
}

int ParaEngine::CBufferPicking::GetChildAttributeObjectCount(int nColumnIndex)
{
	return 1;
}

int ParaEngine::CBufferPicking::GetChildAttributeColumnCount()
{
	return 1;
}

IAttributeFields * ParaEngine::CBufferPicking::GetChildAttributeObject(int nRowIndex, int nColumnIndex)
{
	if (nRowIndex == 0 && nColumnIndex == 0)
	{
		if (GetIdentifier() == "backbuffer")
			return NULL;
		else
		{
			return CreateGetRenderTarget();
		}
	}
	return NULL;
}

CRenderTarget* ParaEngine::CBufferPicking::CreateGetRenderTarget(bool bCreateIfNotExist /*= true*/)
{
	if (m_renderTarget)
		return (CRenderTarget*)(m_renderTarget.get());
	else if (bCreateIfNotExist)
	{
		CRenderTarget* pRenderTarget = static_cast<CRenderTarget*>(CGlobals::GetScene()->FindObjectByNameAndType(GetIdentifier(), "CRenderTarget"));
		if (pRenderTarget)
		{
			pRenderTarget->SetLifeTime(-1);
			m_renderTarget = pRenderTarget->GetWeakReference();
			return pRenderTarget;
		}
		else
		{
			// create one if not exist. 
			CRenderTarget* pRenderTarget = new CRenderTarget();
			pRenderTarget->SetLifeTime(-1);
			pRenderTarget->SetIdentifier(GetIdentifier());
			pRenderTarget->SetRenderTargetSize(Vector2(512, 512));
			CGlobals::GetScene()->AttachObject(pRenderTarget);
			pRenderTarget->SetDirty(false);
			pRenderTarget->SetVisibility(false);
			m_renderTarget = pRenderTarget->GetWeakReference();
			return pRenderTarget;
		}
	}
	return NULL;
}

bool ParaEngine::CBufferPicking::BeginBuffer()
{
	if (GetIdentifier() == "backbuffer")
		return true;
	else
	{
		CRenderTarget* pRenderTarget = CreateGetRenderTarget();
		if (pRenderTarget)
		{
			CViewport* pViewport = NULL;
			if (GetIdentifier() == "overlay") 
			{
				// overlay always use the same viewport of the default 3d scene. 
				pViewport = CGlobals::GetViewportManager()->CreateGetViewPort(1);

				// always use the full screen viewport for overlay
				auto pScreenViewport = CGlobals::GetViewportManager()->CreateGetViewPort(0);
				pRenderTarget->SetRenderTargetSize(pScreenViewport->GetWidth(), pScreenViewport->GetHeight());
			}
			else
			{
				// shall use the same buffer size as the current selected viewport
				ParaViewport viewport;
				CGlobals::GetViewportManager()->GetCurrentViewport(viewport);
				pRenderTarget->SetRenderTargetSize(viewport.Width, viewport.Height);
			}
			

			if (pRenderTarget->GetPrimaryAsset())
			{
				if (pRenderTarget->Begin())
				{
					pRenderTarget->Clear(LinearColor(0,0,0,0));

					CSceneObject* pScene = CGlobals::GetScene();
					CBaseCamera* pCamera = pScene->GetCurrentCamera();
					if (pViewport) {
						pViewport->SetActive();
						pViewport->ApplyCamera((CAutoCamera*)pCamera);
						pViewport->ApplyViewport();
					}
					CGlobals::GetWorldMatrixStack().push(Matrix4::IDENTITY);
					CGlobals::GetProjectionMatrixStack().push(*pCamera->GetProjMatrix());
					CGlobals::GetViewMatrixStack().push(*pCamera->GetViewMatrix());

					EffectManager* pEffectManager = CGlobals::GetEffectManager();
					pEffectManager->UpdateD3DPipelineTransform(true, true, true);

					DrawObjects();

					return true;
				}
			}
		}
	}
	
	return false;
}

void ParaEngine::CBufferPicking::EndBuffer()
{
	if (GetIdentifier() == "backbuffer")
		return;
	else
	{
		CRenderTarget* pRenderTarget = CreateGetRenderTarget();
		if (pRenderTarget)
		{
			pRenderTarget->End();

			// restore transformations
			CGlobals::GetWorldMatrixStack().pop();
			CGlobals::GetProjectionMatrixStack().pop();
			CGlobals::GetViewMatrixStack().pop();
			CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true, true, true);
		}
		return;
	}
}

void ParaEngine::CBufferPicking::DrawObjects()
{
	CSceneObject * pScene = CGlobals::GetScene();
	int nLastRenderPipeline = pScene->GetSceneState()->GetCurrentRenderPipeline();
	// draw objects
	if (GetIdentifier() == "overlay")
	{
		// "overlay" is a special built-in buffer, it will draw using overlays in current scene, 
		// but with PIPELINE_COLOR_PICKING enabled.
		pScene->GetSceneState()->SetCurrentRenderPipeline(PIPELINE_COLOR_PICKING);
		// Note: This will lead to potential crash if drawing non-overlay without week references.
		auto layout = CGlobals::GetViewportManager()->GetLayout();
		if (layout == VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE || layout == VIEW_LAYOUT_STEREO_OMNI_SINGLE_EYE_1 || layout == VIEW_LAYOUT_STEREO_OMNI)
		{
			// this fixed a bug, if current scene states is not the main scene, such as in ods layout, the last rendered scene is the bottom view. 
			CGlobals::GetScene()->PrepareRender(pScene->GetCurrentCamera(), pScene->GetSceneState());
		}
		CGlobals::GetScene()->RenderHeadOnDisplay(1);
	}
	else if (GetIdentifier() == "backbuffer")
	{
		// internal buffer, do not draw anything
	}
	else
	{
		// for all other picking buffer, just call the render target's owner draw method. 
		CRenderTarget* pRenderTarget = CreateGetRenderTarget();
		if (pRenderTarget && pRenderTarget->IsDirty())
		{
			pScene->GetSceneState()->SetCurrentRenderPipeline(PIPELINE_COLOR_PICKING);
			CPainter painter;
			painter.SetUse3DTransform(true);
			if(painter.begin(pRenderTarget))
			{
				// use GUI shader and enable 3d world transform by enable 3d text. 
				RenderDevicePtr pD3dDevice = CGlobals::GetRenderDevice();
				pD3dDevice->SetRenderState(ERenderState::ZENABLE, FALSE);
				pD3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
				
				painter.SetSpriteUseWorldMatrix(true);
				pRenderTarget->DoPaint(&painter);
				painter.SetSpriteUseWorldMatrix(false);

				pD3dDevice->SetRenderState(ERenderState::ZENABLE, TRUE);
				pD3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
				painter.end();
			}
			pRenderTarget->SetDirty(false);
		}
	}
	pScene->GetSceneState()->SetCurrentRenderPipeline(nLastRenderPipeline);
}

int ParaEngine::CBufferPicking::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	AssetEntity::InstallFields(pClass, bOverride);

	pClass->AddField("PickingCount", FieldType_Int, (void*)0, (void*)GetPickingCount_s, NULL, "", bOverride);
	pClass->AddField("PickingID", FieldType_DWORD, (void*)0, (void*)GetPickingID_s, NULL, "", bOverride);
	pClass->AddField("FetchPickingResult", FieldType_void_pointer, (void*)0, (void*)FetchPickingResult_s, NULL, "", bOverride);
	pClass->AddField("ClearPickingResult", FieldType_void, (void*)ClearPickingResult_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("CheckDoPick", FieldType_void, (void*)CheckDoPick_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("PickLeftTop", FieldType_Vector2, (void*)SetPickLeftTop_s, (void*)GetPickLeftTop_s, NULL, "", bOverride);
	pClass->AddField("PickWidthHeight", FieldType_Vector2, (void*)SetPickWidthHeight_s, (void*)GetPickWidthHeight_s, NULL, "", bOverride);
	pClass->AddField("ResultDirty", FieldType_Bool, (void*)SetResultDirty_s, (void*)IsResultDirty_s, NULL, "", bOverride);
	pClass->AddField("PickIndex", FieldType_Int, (void*)SetPickIndex_s, (void*)GetPickIndex_s, NULL, "", bOverride);
	pClass->AddField("Viewport", FieldType_Int, (void*)SetViewport_s, (void*)GetViewport_s, NULL, "", bOverride);
	return S_OK;
}