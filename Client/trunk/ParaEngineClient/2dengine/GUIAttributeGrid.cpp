//-----------------------------------------------------------------------------
// Class:	CGUIAttributeGrid
// Authors: Liu, Weili
// Date:	2006.3.21
// desc:
// This class represents a string inside a rectangular box in the screen coordinates
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "GUIResource.h"
#include "GUIAttributeGrid.h"
#include "GUIRoot.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "ic/ICConfigManager.h"
#include "memdebug.h"
using namespace ParaEngine;
using namespace std;
//CGUIAttributeGrid::CGUIAttributeGrid()
//{
//}
//CGUIAttributeGrid::~CGUIAttributeGrid()
//{
//}
//bool CGUIAttributeGrid::MsgProc(MSG *event)
//{
//	return false;
//}
//void CGUIAttributeGrid::Clone(IObject* pobj)const
//{
//	if (pobj==NULL) {
//		return;
//	}
//	CGUIAttributeGrid* pGrid=(CGUIAttributeGrid*)pobj;
//	CGUIBase::Clone(pGrid);
//	m_data.clear();
//}
//
//IObject *CGUIAttributeGrid::Clone()const 
//{
//	CGUIAttributeGrid *pGrid=new CGUIAttributeGrid();
//	Clone(pGrid);
//	return pGrid;
//}
//void CGUIAttributeGrid::StaticInit()
//{
//	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
//	if (!pOm->IsExist("default_CGUIBase")) {
//		CGUIBase::StaticInit();
//	}
//	//load the default CGUIBase object and copy all its value to the new button
//	CGUIAttributeGrid *pGrid=new CGUIAttributeGrid();
//	pOm->CloneObject("default_CGUIBase",pGrid);
//	pGrid->m_objType=CGUIBase::Type_GUIText;
//	pText->m_szText=L"";
//	pText->m_bAutoSize=true;
//	pText->m_bNeedCalRect=true;
//	CEventBinding* pBinding=pText->m_event->GetEventBindingObj();
//	pBinding->DisableKeyboard();
//	pBinding->DisableMouse();
//	//selectively enable some mouse events.
//	pBinding->EnableEvent(EM_MOUSE);
//	pBinding->EnableEvent(EM_MOUSE_MOVE);
//	pBinding->EnableEvent(EM_MOUSE_LEFTDOWN);
//	pBinding->EnableEvent(EM_MOUSE_LEFTUP);
//	pBinding->EnableEvent(EM_MOUSE_DRAGBEGIN);
//	pBinding->EnableEvent(EM_MOUSE_DRAGOVER);
//	pBinding->EnableEvent(EM_MOUSE_DRAGEND);
//	pBinding->EnableEvent(EM_MOUSE_LEFTDRAGBEGIN);
//	pBinding->EnableEvent(EM_MOUSE_LEFTDRAGEND);
//	pBinding->EnableEvent(EM_MOUSE_RIGHTDRAGBEGIN);
//	pBinding->EnableEvent(EM_MOUSE_RIGHTDRAGEND);
//	pBinding->EnableEvent(EM_MOUSE_MIDDLEDRAGBEGIN);
//	pBinding->EnableEvent(EM_MOUSE_MIDDLEDRAGEND);
//
//	using namespace ParaInfoCenter;
//	CICConfigManager *cm=CGlobals::GetICConfigManager();
//	int tempint;
//	if (cm->GetIntValue("GUI_text_control_visible",&tempint)==S_OK) {
//		if (tempint==0) {
//			pText->m_bIsVisible=false;
//		}else
//			pText->m_bIsVisible=true;
//	}
//	if (cm->GetIntValue("GUI_text_control_enable",&tempint)==S_OK) {
//		if (tempint==0) {
//			pText->m_bIsEnabled=false;
//		}else
//			pText->m_bIsEnabled=true;
//	}
//	if (cm->GetIntValue("GUI_text_control_canhasfocus",&tempint)==S_OK) {
//		if (tempint==0) {
//			pText->m_bCanHasFocus=false;
//		}else
//			pText->m_bCanHasFocus=true;
//	}
//	if (cm->GetIntValue("GUI_text_control_receivedrag",&tempint)==S_OK) {
//		if (tempint==0) {
//			pText->m_bReceiveDrag=false;
//		}else
//			pText->m_bReceiveDrag=true;
//	}
//	if (cm->GetIntValue("GUI_text_control_candrag",&tempint)==S_OK) {
//		if (tempint==0) {
//			pText->SetCandrag(false);
//		}else
//			pText->SetCandrag(true);
//	}
//	if (cm->GetIntValue("GUI_text_control_lifetime",&tempint)==S_OK) {
//		pText->m_nLifeTimeCountDown=tempint;
//	}
//
//	//initialize default texture and font
//	pText->m_objResource->SetActiveLayer();
//	pText->m_objResource->SetCurrentState();
//	pText->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
//	RECT *prect=NULL;
//	SpriteFontEntity* pFont;
//	TextureEntity* pTexture=NULL;
//	pFont=CGlobals::GetAssetManager()->GetFont("sys");
//GUIFontElement font_;
//GUIFontElement* pFontElement=&font_;
//GUITextureElement tex_;
//GUITextureElement* pElement=&tex_;
//	string background,filename;
//	bool bLoadDefault=true;
//	if (cm->GetTextValue("GUI_text_control_background",background)==S_OK) {
//		using namespace ParaScripting;
//		prect=ParaEngine::StringHelper::GetImageAndRect(background,filename);
//		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
//		if (pTexture!=NULL) 
//			bLoadDefault=false;
//	} 
//	if (bLoadDefault) {
//		//Set the default texture and font of the default button
//		pTexture=CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds",TextureEntity::StaticTexture);
//		prect=new RECT;
//		SetRect( prect, 0,0,0,0 );
//	}
//
//	pElement->SetElement(pTexture,prect,COLOR_ARGB(0,255,255,255));
//	pFontElement->SetElement(pFont,COLOR_ARGB(255,0,0,0),DT_LEFT|DT_TOP|DT_WORDBREAK);
//	pText->m_objResource->AddElement(pElement,L"background");
//	pText->m_objResource->AddElement(pFontElement,L"text");
//	SAFE_DELETE(prect);
//
//	pText->m_objResource->SetCurrentState();
//	pText->m_objResource->SetActiveLayer();
//	pOm->SetObject("default_CGUIText",pText);
//	SAFE_RELEASE(pText);
//}