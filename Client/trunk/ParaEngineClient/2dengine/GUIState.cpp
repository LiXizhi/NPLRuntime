//----------------------------------------------------------------------
// Class:	CGUIState
// Authors:	LiXizhi
// Date:	2005.8.3
//----------------------------------------------------------------------
#include "ParaEngine.h"
#include "PaintEngine/Painter.h"
#include "GUIBase.h"
#include "GUIState.h"

using namespace ParaEngine;


ParaEngine::GUIState::GUIState()
	: painter(NULL), bIsBatchRender(false), nBkbufWidth(0), nBkbufHeight(0), m_nDepthEnableCount(0)
{

}

void ParaEngine::GUIState::AddToPostRenderingList(IObject* obj)
{
	listPostRenderingObjects.push_back(obj->GetWeakReference());
}

void ParaEngine::GUIState::CleanupGUIState()
{
	listPostRenderingObjects.clear();
	bIsBatchRender = false;
	painter = NULL;
	m_nDepthEnableCount = 0;
}

void ParaEngine::GUIState::BeginDepthTest(CGUIBase* pObject)
{
	if (m_nDepthEnableCount == 0)
	{
		if (pObject)
			pObject->GetPainter(this)->Flush();
		pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}
	m_nDepthEnableCount++;
}

void ParaEngine::GUIState::EndDepthTest(CGUIBase* pObject)
{
	if (m_nDepthEnableCount>0)
		m_nDepthEnableCount--;
	if (m_nDepthEnableCount == 0)
	{
		if (pObject)
			pObject->GetPainter(this)->Flush();
		pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}
}
