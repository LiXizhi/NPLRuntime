//-----------------------------------------------------------------------------
// Class:	OverlayObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2015.8.10
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BaseCamera.h"
#include "SceneState.h"
#include "2dengine/GUIButton.h"
#include "OverlayObject.h"

using namespace ParaEngine;

ParaEngine::COverlayObject::COverlayObject()
	:m_bIsTileObject(false)
{
	SetAttribute(OBJ_SKIP_RENDER, true);
}

ParaEngine::COverlayObject::~COverlayObject()
{
}

bool ParaEngine::COverlayObject::IsTileObject()
{
	return m_bIsTileObject;
}

void ParaEngine::COverlayObject::SetTileObject(bool bIsTileObject)
{
	if (m_bIsTileObject != bIsTileObject)
	{
		m_bIsTileObject = bIsTileObject;
		if (GetTileContainer())
		{
			UpdateTileContainer();
		}
	}
}

int ParaEngine::COverlayObject::PrepareRender(CBaseCamera* pCamera, SceneState * sceneState)
{
	IViewClippingObject* pViewClippingObject = GetViewClippingObject();
	if (pViewClippingObject->TestCollision(pCamera))
	{
		float fObjectToCameraDist = pViewClippingObject->GetObjectToPointDistance(&(sceneState->vEye));
		sceneState->listHeadonDisplayObject.push_back(PostRenderObjectWeakPtr(this, fObjectToCameraDist));
	}
	return 0;
}

CGUIBase* ParaEngine::COverlayObject::GetHeadOnUIObject(int nIndex /*= 0*/)
{
	if (!m_pGUIObject)
	{
		m_pGUIObject = new CGUIButton();
		m_pGUIObject->InitObject("", "_lt", 0,0,0,0);
		m_pGUIObject->SetOwnerDraw(true);
		SetHeadOn3DFacing(0.f, 0);
		// tricky: since headon display will scale by 1/100 for GUI coordinate, however, we wants to work in real coordinate by default. 
		// if one wants to draw GUI objects on overlay, scale by 1/100 explicitly. 
		SetHeadOnMaxUIScaling(100.f);
		SetHeadOnMinUIScaling(100.f);
		SetHeadOnOffest(Vector3::ZERO);
	}
	return m_pGUIObject.get();
}

int ParaEngine::COverlayObject::GetChildAttributeColumnCount()
{
	return 2;
}

IAttributeFields* ParaEngine::COverlayObject::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 1)
		return GetHeadOnUIObject(0);
	else
		return CTileObject::GetChildAttributeObject(nRowIndex, nColumnIndex);
}

IAttributeFields* ParaEngine::COverlayObject::GetChildAttributeObject(const std::string& sName)
{
	if (sName == "gui")
		return GetHeadOnUIObject(0);
	else
		return CTileObject::GetChildAttributeObject(sName);
}

int ParaEngine::COverlayObject::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 1)
		return 1;
	else
		return CTileObject::GetChildAttributeObjectCount(nColumnIndex);
}

