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
#include "BufferPicking.h"

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
	ClearPickingResult();
	SetResultDirty(false);
	QRect region = region_;
	if (!region.isValid())
		return 0;
	if (BeginBuffer())
	{
		// just in case viewport is scaled.
		D3DVIEWPORT9 viewport;
		CGlobals::GetRenderDevice()->GetViewport(&viewport);

		float fScalingX = (float)viewport.Width / CGlobals::GetGUI()->GetWidth();
		float fScalingY = (float)viewport.Height / CGlobals::GetGUI()->GetHeight();

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
		if (!RenderDevice::ReadPixels(region.left(), region.top(), nWidth, nHeight, (void*)(&(m_pickingResult[0]))))
			m_pickingResult.clear();

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

int ParaEngine::CBufferPicking::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	AssetEntity::InstallFields(pClass, bOverride);

	pClass->AddField("PickingCount", FieldType_Int, (void*)0, (void*)GetPickingCount_s, NULL, "", bOverride);
	pClass->AddField("PickingID", FieldType_DWORD, (void*)0, (void*)GetPickingID_s, NULL, "", bOverride);
	pClass->AddField("ClearPickingResult", FieldType_void, (void*)ClearPickingResult_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("CheckDoPick", FieldType_void, (void*)CheckDoPick_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("PickLeftTop", FieldType_Vector2, (void*)SetPickLeftTop_s, (void*)GetPickLeftTop_s, NULL, "", bOverride);
	pClass->AddField("PickWidthHeight", FieldType_Vector2, (void*)SetPickWidthHeight_s, (void*)GetPickWidthHeight_s, NULL, "", bOverride);
	pClass->AddField("ResultDirty", FieldType_Bool, (void*)SetResultDirty_s, (void*)IsResultDirty_s, NULL, "", bOverride);
	pClass->AddField("PickIndex", FieldType_Int, (void*)SetPickIndex_s, (void*)GetPickIndex_s, NULL, "", bOverride);
	pClass->AddField("Viewport", FieldType_Int, (void*)SetViewport_s, (void*)GetViewport_s, NULL, "", bOverride);
	return S_OK;
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
			ParaViewport viewport;
			CGlobals::GetViewportManager()->GetCurrentViewport(viewport);
			// TODO: shall use the same buffer size as the current selected viewport?
			pRenderTarget->SetRenderTargetSize(viewport.Width, viewport.Height);

			if (pRenderTarget->GetPrimaryAsset())
			{
				if (pRenderTarget->Begin())
				{
					pRenderTarget->Clear(LinearColor(0,0,0,0));

					CSceneObject* pScene = CGlobals::GetScene();
					CBaseCamera* pCamera = pScene->GetCurrentCamera();
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
		// draw overlays
		pScene->GetSceneState()->SetCurrentRenderPipeline(PIPELINE_COLOR_PICKING);
		// Note: This will lead to potential crash if drawing non-overlay without week references.
		CGlobals::GetScene()->RenderHeadOnDisplay(1);
	}
	else
	{
		// TODO: other objects?
	}
	pScene->GetSceneState()->SetCurrentRenderPipeline(nLastRenderPipeline);
}
