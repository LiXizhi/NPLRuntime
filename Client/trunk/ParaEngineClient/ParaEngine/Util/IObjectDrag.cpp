//-----------------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.11
// Note:
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "2dengine/GUIBase.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIDirectInput.h"
#include "MiscEntity.h"
#include "IObjectDrag.h"

ParaEngine::STRUCT_DRAG_AND_DROP ParaEngine::IObjectDrag::DraggingObject; 

using namespace ParaEngine;
using namespace std;

bool IObjectDrag::CancelDrag(STRUCT_DRAG_AND_DROP* pdrag)
{
	//////////////////////////////////////////////////////////////////////////
	/// restore dragging control to source control 
	bool bDraggingCanceled = false;
	if(pdrag && (pdrag->pDragging != pdrag->pSource || pdrag->pSource != NULL))
	{

		// TODO: assume that pdrag->pSource and pdrag->pDragging are always a CGUIBase object. A runtime check is needed in future. 
		//CGUIBase* receiver = (CGUIBase*) pdrag->pSource;

		//if(receiver->GetReceiveDrag())
		//{
		//	// restore location.
		//	CGUIBase* dragCtrl = (CGUIBase*) pdrag->pDragging;
		//	dragCtrl->SetLocation(pdrag->startX+pdrag->nRelativeX,pdrag->startY+pdrag->nRelativeY);
		//	// here we just assume mouse
		//	receiver->OnDragEnd(EM_MOUSE_LEFT, pdrag->startX,pdrag->startY);
		//	bDraggingCanceled = true;
		//}
		bDraggingCanceled = true;

		/////////////////////////////////////////
		// restore to old position
		CGUIRoot *root=CGUIRoot::GetInstance();
		if(pdrag->pDragging)
		{
			if(pdrag->GetOldPosition())
				pdrag->pDragging->SetPositionI(*(pdrag->GetOldPosition()));
			// TODO: assume that pdrag->pSource and pdrag->pDragging are always a CGUIBase object. A runtime check is needed in future. 
			root->AttachGUIElement((CGUIBase*) pdrag->pSource,(CGUIBase*)pdrag->pDragging);
			if (pdrag->pCleanUp!=NULL) {
				pdrag->pCleanUp();
			}
			pdrag->init();
#ifdef USE_DIRECTX_RENDERER
			root->m_pMouse->Update();
#endif
		}
	}
	return bDraggingCanceled;	
}

void STRUCT_DRAG_AND_DROP::init()
{
	pSource=NULL;pDragging=NULL;pDestination=NULL;nEvent=0;nRelativeY=0;nRelativeX=0;
	m_bIsCandicateOnly = false;
	startX=0;startY=0;
	pCleanUp=NULL;
	SAFE_DELETE(m_pOldPosition);
}

void STRUCT_DRAG_AND_DROP::SetOldPosition(const CGUIPosition& vPos)
{
	if(m_pOldPosition==0)
		m_pOldPosition = new CGUIPosition();
	*m_pOldPosition = vPos;
}

CGUIPosition* STRUCT_DRAG_AND_DROP::GetOldPosition()
{
	return m_pOldPosition;
}


void STRUCT_DRAG_AND_DROP::CleanUpReceivers()
{
	m_receiverNames.clear();
}

void STRUCT_DRAG_AND_DROP::AddReceiver(const char* sName)
{
	if(!HasReceiver(sName))
		m_receiverNames.push_back(sName);
}

bool STRUCT_DRAG_AND_DROP::HasReceiver( const char* sName )
{
	for (int i=0;i<(int)m_receiverNames.size();++i)
	{
		if(m_receiverNames[i] == sName)
			return true;
	}
	return false;
}


void ParaEngine::STRUCT_DRAG_AND_DROP::SetDraggingCandidate( IObjectDrag* pDragging_ )
{
	if(pDragging==NULL)
	{
		m_bIsCandicateOnly = true;
		pDragging = pDragging_;
	}
}

void ParaEngine::STRUCT_DRAG_AND_DROP::UnsetDraggingCandidate()
{
	if(m_bIsCandicateOnly)
	{
		m_bIsCandicateOnly = false;
		init();
	}
}
