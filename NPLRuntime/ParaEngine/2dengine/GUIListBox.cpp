//----------------------------------------------------------------------
// Class:	CGUIListBox
// Authors:	Liu Weili, LiXizhi
// Date:	2005.12.10
// Revised: 2005.12.10
//
// desc: 
// The CGUIListBox control enables you to display a list of items to the user that the user can select by clicking. 
// A CGUIListBox control can provide single or multiple selections using the SetMultiSelect method. 
// In addition to display and selection functionality, the ListBox also provides features that enable you to 
// efficiently add items to the CGUIListBox and to find text within the items of the list. Use Size() to get the number of 
// list items in the CGUIListBox. When the control's GetMultiSelect()==true, use GetSelected to check if the item at given 
// index is selected. When the control's GetMultiSelect()==false, use GetSelected to get the selected item. 
// The GetItem method provide access to the list items that are used by the CGUIListBox. 
// This class provides fast back-end-adding and sequential and random access of the list items. 
// But deleting and randomly-inserting items are slower.
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "GUIRoot.h"
#include "GUIContainer.h"
#include "GUIResource.h"
#include "ParaEngine.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "util/StringHelper.h"
#include "PaintEngine/Painter.h"
#include "ParaWorldAsset.h"
#include "GUIScrollBar.h"
#include "InfoCenter/ICConfigManager.h"
#include "GUIListBox.h"

using namespace ParaEngine;


const IType* CGUIListBox::m_type = NULL;
CGUIListBox::CGUIListBox() :CGUIContainer()
{
	if (!m_type){
		m_type = IType::GetType("guilistbox");
	}
	//	m_objType=CGUIBase::Type_GUIListBox|CGUIBase::GUIContainer;
	m_bMultiSelect = false;
	m_nSelected = -1;
	m_bAllowAdvObject = false;
	m_bNeedUpdate = true;
	m_deltaY = m_deltaX = 0;
	m_SelBkColor = COLOR_ARGB(255, 40, 50, 92);
	m_ItemHeight = 0;
	m_nBorder = 2;
	m_nMargin = 6;
	m_bCanHasFocus = true;
	m_bDrag = false;
	m_nSelStart = 0;
	m_bFastRender = true;
	m_bWordBreak = false;
	m_bScrollable = true;
}

CGUIListBox::~CGUIListBox()
{
	RemoveAll();
}

IObject* CGUIListBox::Clone()const
{
	CGUIListBox* pListbox = new CGUIListBox();
	Clone(pListbox);
	return pListbox;
}

void CGUIListBox::Clone(IObject *pobj)const
{
	PE_ASSERT(pobj != NULL);
	if (pobj == NULL) {
		return;
	}
	CGUIListBox* pListbox = (CGUIListBox*)pobj;
	CGUIContainer::Clone(pListbox);
	pListbox->m_bMultiSelect = m_bMultiSelect;
	pListbox->m_nSelected = m_nSelected;
	pListbox->m_ItemHeight = m_ItemHeight;
	pListbox->m_deltaX = m_deltaX;
	pListbox->m_deltaY = m_deltaY;
	pListbox->m_bItemCanDrag = m_bItemCanDrag;
	pListbox->m_bAllowAdvObject = m_bAllowAdvObject;
	pListbox->m_bDrag = m_bDrag;
	pListbox->m_bWordBreak = m_bWordBreak;
	pListbox->m_SelBkColor = m_SelBkColor;
	pListbox->m_nSelStart = m_nSelStart;
	pListbox->m_items.assign(m_items.begin(), m_items.end());
	list<GUIListItem>::iterator iter = m_items.begin(), iterend = m_items.end();
	for (int a = 0; iter != iterend; iter++, a++) {
		pListbox->m_itemmap[a] = &(*iter);
	}

}

void CGUIListBox::StaticInit()
{
	CObjectManager *pOm = &CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIContainer")) {
		CGUIContainer::StaticInit();
	}
	//load the default CGUIBase object and copy all its value to the new button
	CGUIListBox *pListbox = new CGUIListBox();
	pOm->CloneObject("default_CGUIContainer", pListbox);
	//pListbox->m_objType=CGUIBase::Type_GUIListBox|CGUIBase::GUIContainer;
	pListbox->m_bMultiSelect = false;
	pListbox->m_nSelected = -1;
	pListbox->m_bAllowAdvObject = false;
	pListbox->m_bNeedUpdate = true;
	pListbox->m_deltaX = 0;
	pListbox->m_deltaY = 0;
	pListbox->m_SelBkColor = COLOR_ARGB(255, 40, 50, 92);
	pListbox->m_ItemHeight = 0;
	pListbox->m_nBorder = 2;
	pListbox->m_nMargin = 6;
	pListbox->m_bCanHasFocus = true;
	pListbox->m_bDrag = false;
	pListbox->m_nSelStart = 0;
	pListbox->m_bFastRender = true;
	pListbox->m_bWordBreak = false;
	pListbox->m_bScrollable = true;

	CEventBinding* pBinding = (CEventBinding*)pListbox->m_event->GetEventBindingObj();
	pBinding->EnableKeyboard();
	pBinding->EnableMouse();

	//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_CTRL_CAPTUREMOUSE);
	//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_LB_ACTIONBEGIN);
	//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_CTRL_RELEASEMOUSE);
	//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_LB_ACTIONEND);
	//	pBinding->MapEvent(EM_MOUSE_LEFTCLICK,EM_LB_ACTIONEND);
	//	pBinding->MapEvent(EM_MOUSE_LEFTCLICK,EM_CTRL_RELEASEMOUSE);
	//	pBinding->MapEvent(EM_MOUSE_LEFTDBCLICK,EM_CTRL_CHANGE);
	//	pBinding->MapEvent(EM_KEY_RETURN,EM_CTRL_CHANGE);
	pBinding->MapEvent(EM_KEY_HOME, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_END, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_PAGE_DOWN, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_PAGE_UP, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_LEFT, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_RIGHT, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_DOWN, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_UP, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_RETURN, EM_CTRL_HOLDKEY);
	pBinding->EnableEvent(EM_MOUSE_DBCLICK);
	pBinding->EnableEvent(EM_MOUSE_CLICK);

	using namespace ParaInfoCenter;
	CICConfigManager *cm = CGlobals::GetICConfigManager();

	string value0, value1;
	int event0, event1, a;
	DWORD b;
	HRESULT hr;
	hr = cm->GetSize("GUI_listbox_control_mapping", &b);
	if (hr == E_INVALIDARG || hr == E_ACCESSDENIED) {
		//error
	}
	else{
		for (a = 0; a < (int)b; a += 2) {
			hr = cm->GetTextValue("GUI_listbox_control_mapping", value0, a);
			if (FAILED(hr)) {
				break;
			}
			hr = cm->GetTextValue("GUI_listbox_control_mapping", value1, a + 1);
			if (FAILED(hr)) {
				break;
			}
			event0 = CEventBinding::StringToEventValue(value0);
			event1 = CEventBinding::StringToEventValue(value1);
			pBinding->MapEvent(event0, event1);
		}
	}
	int tempint;
	if (cm->GetIntValue("GUI_listbox_control_visible", &tempint) == S_OK) {
		if (tempint == 0) {
			pListbox->m_bIsVisible = false;
		}
		else
			pListbox->m_bIsVisible = true;
	}
	if (cm->GetIntValue("GUI_listbox_control_enable", &tempint) == S_OK) {
		if (tempint == 0) {
			pListbox->m_bIsEnabled = false;
		}
		else
			pListbox->m_bIsEnabled = true;
	}
	if (cm->GetIntValue("GUI_listbox_control_canhasfocus", &tempint) == S_OK) {
		if (tempint == 0) {
			pListbox->m_bCanHasFocus = false;
		}
		else
			pListbox->m_bCanHasFocus = true;
	}
	if (cm->GetIntValue("GUI_listbox_control_candrag", &tempint) == S_OK) {
		if (tempint == 0) {
			pListbox->SetCandrag(false);
		}
		else
			pListbox->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_listbox_control_lifetime", &tempint) == S_OK) {
		pListbox->m_nLifeTimeCountDown = tempint;
	}
	if (cm->GetIntValue("GUI_listbox_control_multiselect", &tempint) == S_OK) {
		if (tempint == 0) {
			pListbox->m_bMultiSelect = false;
		}
		else
			pListbox->m_bMultiSelect = true;
	}
	if (cm->GetIntValue("GUI_listbox_control_fastrender", &tempint) == S_OK) {
		if (tempint == 0) {
			pListbox->m_bFastRender = false;
		}
		else
			pListbox->m_bFastRender = true;
	}
	if (cm->GetIntValue("GUI_listbox_control_wordbreak", &tempint) == S_OK) {
		if (tempint == 0) {
			pListbox->m_bWordBreak = false;
		}
		else
			pListbox->m_bWordBreak = true;
	}
	if (cm->GetIntValue("GUI_listbox_control_margin", &tempint) == S_OK) {
		pListbox->m_nMargin = tempint;
	}
	if (cm->GetIntValue("GUI_listbox_control_borderwidth", &tempint) == S_OK) {
		pListbox->m_nBorder = tempint;
	}
	if (cm->GetIntValue("GUI_listbox_control_itemheight", &tempint) == S_OK) {
		pListbox->m_ItemHeight = tempint;
	}
	if (cm->GetIntValue("GUI_listbox_control_scrollbarwidth", &tempint) == S_OK) {
		pListbox->m_nSBWidth = tempint;
	}
	if (cm->GetIntValue("GUI_listbox_control_scrollable", &tempint) == S_OK) {
		if (tempint == 0) {
			pListbox->m_bScrollable = false;
		}
		else
			pListbox->m_bScrollable = true;
	}


	//Set the default texture and font of the default scrollbar

	pListbox->m_objResource->SetActiveLayer();
	pListbox->m_objResource->SetCurrentState();
	pListbox->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);

	RECT *prect = NULL;
	RECT rect;
	SpriteFontEntity* pFont;
	TextureEntity* pTexture = NULL;
	pFont = CGlobals::GetAssetManager()->GetFont("sys");
	GUIFontElement font_;
	GUIFontElement* pFontElement = &font_;
	GUITextureElement tex_;
	GUITextureElement* pElement = &tex_;

	pListbox->m_objResource->Clear();	//clean all resources;

	pFontElement->SetElement(pFont, COLOR_ARGB(255, 16, 16, 16), DT_LEFT | DT_TOP);
	pListbox->m_objResource->AddElement(pFontElement, "text");
	pFontElement->SetElement(pFont, COLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_TOP);
	pListbox->m_objResource->AddElement(pFontElement, "selected_text");
	string background, filename;
	bool bLoadDefault = true;
	if (cm->GetTextValue("GUI_listbox_control_background", background) == S_OK) {
		prect = ParaEngine::StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL) {
			bLoadDefault = false;
		}
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 13, 123, 241, 160);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));

	pListbox->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pListbox->m_objResource->AddElement(pElement, "background");
	pListbox->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pListbox->m_objResource->AddElement(pElement, "background");
	pListbox->m_objResource->SetCurrentState(GUIResourceState_Highlight);
	pListbox->m_objResource->AddElement(pElement, "background");
	pListbox->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pListbox->m_objResource->AddElement(pElement, "background");
	pListbox->m_objResource->SetCurrentState();

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_listbox_control_selection", background) == S_OK) {
		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 17, 269, 241, 287);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));

	pListbox->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pListbox->m_objResource->AddElement(pElement, "selection");
	pListbox->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pListbox->m_objResource->AddElement(pElement, "selection");
	pListbox->m_objResource->SetCurrentState(GUIResourceState_Highlight);
	pListbox->m_objResource->AddElement(pElement, "selection");
	pListbox->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pListbox->m_objResource->AddElement(pElement, "selection");
	pListbox->m_objResource->SetCurrentState();


	pOm->SetObject("default_CGUIListBox", pListbox);
	SAFE_RELEASE(pListbox);

}
void CGUIListBox::SetWordBreak(bool wordbreak)
{
	if (m_bWordBreak != wordbreak) {
		GUIFontElement* pFontElement;
		DWORD dwFlag;
		m_bWordBreak = wordbreak;
		if (m_bWordBreak) {
			dwFlag = DT_LEFT | DT_TOP | DT_VCENTER | DT_WORDBREAK;
		}
		else
			dwFlag = DT_LEFT | DT_TOP | DT_VCENTER;
		pFontElement = m_objResource->GetFontElement(0);//text
		pFontElement->SetElement(pFontElement->GetFont(), COLOR_ARGB(255, 16, 16, 16), dwFlag);
		pFontElement = m_objResource->GetFontElement(1);//selected text
		pFontElement->SetElement(pFontElement->GetFont(), COLOR_ARGB(255, 255, 255, 255), dwFlag);
	}
}

void CGUIListBox::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm = &CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIListBox")) {
		CGUIListBox::StaticInit();
	}
	pOm->CloneObject("default_CGUIListBox", this);
	CGUIBase::InitObject(strObjectName, alignment, x, y, width, height);
	//initialize scrollbars
	string str;

	str = GetName() + ".vscrollbar";
	m_VScroll->InitObject(str.c_str(), "_rt", -m_nSBWidth - m_nBorder, m_nBorder, m_nSBWidth, RectHeight(m_objResource->GetDrawingRects(0)));
	m_VScroll->SetParent(this);
	m_VScroll->SetLinkedObject(this);
	m_VScroll->UpdateRects();

	// 	str=GetName()+".hscrollbar";
	// 	m_HScroll->InitObject(str.c_str(),"_lb",m_nBorder,-m_nSBWidth-m_nBorder,RectWidth(m_objResource->GetDrawingRects(0)),m_nSBWidth);
	// 	m_HScroll->m_parent=this;
	// 	m_HScroll->SetLinkedObject(this);
	// 	m_HScroll->UpdateRects();
}

bool CGUIListBox::GetSelected(int index)
{
	if (index < 0 || index >= (int)m_items.size()) {
		return false;
	}

	return m_itemmap[index]->selected;
}

bool CGUIListBox::MsgProc(MSG *event)
{
	//one call to this function will trigger at most one event
	if (!m_bIsEnabled)
		return false;
	bool bHandled = false;
	CGUIMouseVirtual *pMouse = CGUIRoot::GetInstance()->m_pMouse;
	CGUIKeyboardVirtual *pKeyboard = CGUIRoot::GetInstance()->m_pKeyboard;
	if (event != NULL&&!m_event->InterpretMessage(event)) {
		return false;
	}
	POINT pt;
	pt.x = m_event->m_mouse.x;
	pt.y = m_event->m_mouse.y;
	int nEvent = m_event->GetTriggerEvent();
	MSG newMsg;
	int nClicked;
	CEventBinding::InitMsg(&newMsg, event->time, event->message, pt);
	if (m_bIsVisible&&!m_bCandrag){
		if (nEvent == EM_NONE) {
			return false;
		}
		if (m_bScrollable) {
			CGUIPosition pos;
			if (m_ScrollType&VSCROLL) {
				m_VScroll->GetAbsolutePosition(&pos, m_VScroll->GetPosition());

				if (pos.rect.left <= m_event->m_mouse.x && pos.rect.top <= m_event->m_mouse.y &&
					pos.rect.right >= m_event->m_mouse.x && pos.rect.bottom >= m_event->m_mouse.y&&
					!(m_event->IsMapTo(nEvent, EM_MOUSE_DRAGEND) || m_event->IsMapTo(nEvent, EM_MOUSE_DRAGOVER)))
				{
					/// object should at least has a life count greater than 1, and isEnabled
					if ((m_VScroll->GetLifeTime() == -1) || ((m_VScroll->GetLifeTime() > 1) && m_VScroll->GetVisible())) {
						//m_VScroll->AddEvent(event);

						bHandled |= m_VScroll->MsgProc(event);
					}
				}
			}
			// 			if (m_ScrollType&HSCROLL) {
			// 				m_HScroll->GetAbsolutePosition(&pos, m_HScroll->GetPosition());
			// 
			// 				if(pos.rect.left<=m_event->m_mouse.x && pos.rect.top<=m_event->m_mouse.y &&
			// 					pos.rect.right>=m_event->m_mouse.x && pos.rect.bottom>=m_event->m_mouse.y&&
			// 					!(m_event->IsMapTo(nEvent,EM_MOUSE_DRAGEND)||m_event->IsMapTo(nEvent,EM_MOUSE_DRAGOVER)))
			// 				{
			// 					/// object should at least has a life count greater than 1, and isEnabled
			// 					if((m_HScroll->GetLifeTime() == -1) ||(m_HScroll->GetLifeTime() > 1)&&m_HScroll->m_bIsVisible) {
			// 						//m_HScroll->AddEvent(event);
			// 						bHandled|=m_HScroll->MsgProc(event);
			// 						//if (bHandled) {
			// 						//	m_pMouseEvent.m_mouse.init();
			// 						//}
			// 					}
			// 				}
			// 			}
			if (bHandled) {
				m_event->ResetState();
				return true;
			}
		}
		if (m_event->IsMapTo(nEvent, EM_MOUSE_WHEEL)) {
			if (m_bScrollable && (m_ScrollType&VSCROLL)) {
				//				newMsg.message=EM_MOUSE_WHEEL;
				//				newMsg.lParam=event->lParam;
				bHandled |= m_VScroll->MsgProc(event);
			}
			bHandled = true;
		}
		RECT rcText = m_objResource->GetDrawingRects(3);
		if (m_bDrag && (m_event->IsMapTo(nEvent, EM_MOUSE_MOVE) || m_event->IsMapTo(nEvent, EM_CTRL_FRAMEMOVE)) && !ControlPressed&&!ShiftPressed)
		{
			// Compute the index of the item below cursor
			int nItem;
			if (m_ItemHeight)
				nItem = m_VScroll->GetTrackPos() + (pt.y - rcText.top) / m_ItemHeight;
			else
				nItem = -1;

			// Only proceed if the cursor is on top of an item.

			if (nItem >= (int)m_VScroll->GetTrackPos() &&
				nItem < (int)m_items.size() &&
				nItem < m_VScroll->GetTrackPos() + m_VScroll->GetPageSize())
			{
				m_nSelected = nItem;
			}
			else
				if (nItem < (int)m_VScroll->GetTrackPos())
				{
					// User drags the mouse above window top
					m_VScroll->Scroll(-1);
					m_nSelected = m_VScroll->GetTrackPos();
				}
				else
					if (nItem >= m_VScroll->GetTrackPos() + m_VScroll->GetPageSize())
					{
						// User drags the mouse below window bottom
						m_VScroll->Scroll(1);
						m_nSelected = Math::Min((int)m_items.size(), m_VScroll->GetTrackPos() + m_VScroll->GetPageSize()) - 1;
					}

		}
		if (m_event->IsMapTo(nEvent, EM_CTRL_CHANGE)) {
			newMsg.message = EM_CTRL_RELEASEMOUSE;
			CGUIBase::MsgProc(&newMsg);
			m_bDrag = false;
			bHandled = true;
			if (m_event->IsMapTo(nEvent, EM_MOUSE)) {
				if (m_items.size() > 0 && PtInRect(rcText, pt))
				{
					// Compute the index of the clicked item

					int nClicked;
					if (m_ItemHeight)
						nClicked = m_VScroll->GetTrackPos() + (pt.y - rcText.top) / m_ItemHeight;
					else
						nClicked = -1;

					// Only proceed if the click falls on top of an item.

					if (nClicked >= m_VScroll->GetTrackPos() &&
						nClicked < (int)m_items.size() &&
						nClicked < m_VScroll->GetTrackPos() + m_VScroll->GetPageSize())
					{
						bool isEnter = false;
						if (m_bMultiSelect) {
							GUIListItem *pSelItem = m_itemmap.find(nClicked)->second;
							if (pSelItem->selected == true) {
								isEnter = true;
							}
						}
						else if (m_nSelected == nClicked) {
							isEnter = true;
						}

						if (isEnter) {
							// If this is a double click, fire off an event and exit
							// since the first click would have taken care of the selection
							// updating.
							OnDoubleClick(nEvent, pt.x, pt.y);
						}
						else {
							newMsg.message = EM_CTRL_SELECT;
							newMsg.lParam = (DWORD)nClicked;
							CGUIBase::MsgProc(&newMsg);
						}
					}

				}

			}
			else if (m_event->IsMapTo(nEvent, EM_KEY)) {
				bool isEnter = false;
				if (m_bMultiSelect) {
					for (int i = 0; i < Size(); i++) {
						if (GetSelected(i)) {
							isEnter = true;
							break;
						}
					}
				}
				else if (m_nSelected != -1) {
					isEnter = true;
				}
				if (isEnter) {
					OnDoubleClick(nEvent, pt.x, pt.y);
				}
			}
		}
		else if (m_event->IsMapTo(nEvent, EM_CTRL_SELECT)) {
			SelectItem((int)event->lParam);
			OnSelect();
			bHandled = true;
		}
		if (m_event->IsMapTo(nEvent, EM_LB_ACTIONBEGIN)){
			// Check for clicks in the text area
			if (m_items.size() > 0 && PtInRect(rcText, pt))
			{
				// Compute the index of the clicked item

				int nClicked;
				if (m_ItemHeight)
					nClicked = m_VScroll->GetTrackPos() + (pt.y - rcText.top) / m_ItemHeight;
				else
					nClicked = -1;

				// Only proceed if the click falls on top of an item.

				if (nClicked >= m_VScroll->GetTrackPos() &&
					nClicked < (int)m_items.size() &&
					nClicked < m_VScroll->GetTrackPos() + m_VScroll->GetPageSize())
				{
					newMsg.message = EM_CTRL_CAPTUREMOUSE;
					CGUIBase::MsgProc(&newMsg);
					m_bDrag = true;

					m_nSelected = nClicked;
					if (!ShiftPressed)
						m_nSelStart = m_nSelected;

					// If this is a multi-selection listbox, update per-item
					// selection data.

					if (m_bMultiSelect)
					{
						// Determine behavior based on the state of Shift and Ctrl

						GUIListItem *pSelItem = m_itemmap.find(m_nSelected)->second;
						if (ControlPressed&&!ShiftPressed)
						{
							// Control click. Reverse the selection of this item.

							pSelItem->selected = !pSelItem->selected;
						}
						else
							if (ShiftPressed&&!ControlPressed)
							{
								// Shift click. Set the selection for all items
								// from last selected item to the current item.
								// Clear everything else.

								int nBegin = Math::Min(m_nSelStart, m_nSelected);
								int nEnd = Math::Max(m_nSelStart, m_nSelected);

								for (int i = 0; i < nBegin; ++i)
								{
									GUIListItem *pItem = m_itemmap.find(i)->second;
									pItem->selected = false;
								}

								for (int i = nEnd + 1; i < (int)m_items.size(); ++i)
								{
									GUIListItem *pItem = m_itemmap.find(i)->second;
									pItem->selected = false;
								}

								for (int i = nBegin; i <= nEnd; ++i)
								{
									GUIListItem *pItem = m_itemmap.find(i)->second;
									pItem->selected = true;
								}
							}
							else
								if (ControlPressed&&ShiftPressed)
								{
									// Control-Shift-click.

									// The behavior is:
									//   Set all items from m_nSelStart to m_nSelected to
									//     the same state as m_nSelStart, not including m_nSelected.
									//   Set m_nSelected to selected.

									int nBegin = Math::Min(m_nSelStart, m_nSelected);
									int nEnd = Math::Max(m_nSelStart, m_nSelected);

									// The two ends do not need to be set here.

									bool bLastSelected = m_itemmap.find(m_nSelStart)->second->selected;
									for (int i = nBegin + 1; i < nEnd; ++i)
									{
										GUIListItem *pItem = m_itemmap.find(i)->second;
										pItem->selected = bLastSelected;
									}

									pSelItem->selected = bLastSelected;

									// Restore m_nSelected to the previous value
									// This matches the Windows behavior

									m_nSelected = m_nSelStart;
								}
								else
								{
									// Simple click.  Clear all items and select the clicked
									// item.


									for (int i = 0; i < (int)m_items.size(); ++i)
									{
										GUIListItem *pItem = m_itemmap.find(i)->second;
										pItem->selected = false;
									}

									pSelItem->selected = true;
								}
					}  // End of multi-selection case


				}

				bHandled = true;
			}
		}
		else if (m_bDrag&&m_event->IsMapTo(nEvent, EM_LB_ACTIONEND)) {
			m_bDrag = false;

			if (m_nSelected != -1 && m_nSelStart >= 0)
			{
				// Set all items between m_nSelStart and m_nSelected to
				// the same state as m_nSelStart
				int nEnd = Math::Max(m_nSelStart, m_nSelected);

				for (int n = Math::Min(m_nSelStart, m_nSelected) + 1; n < nEnd; ++n)
					m_itemmap[n]->selected = m_itemmap[m_nSelStart]->selected;
				m_itemmap[m_nSelected]->selected = m_itemmap[m_nSelStart]->selected;

				// If m_nSelStart and m_nSelected are not the same,
				// the user has dragged the mouse to make a selection.
				// Notify the application of this.
				if (m_nSelStart != m_nSelected)
				{
					// TODO:
				}
				OnSelect();
			}
			bHandled = true;
			newMsg.message = EM_CTRL_RELEASEMOUSE;
			CGUIBase::MsgProc(&newMsg);
		}
		if (m_event->IsMapTo(nEvent, EM_KEY)) {
			if ((event->lParam & 0x80) != 0) {
				if (m_event->IsMapTo(nEvent, EM_KEY_UP)) {
					nClicked = m_nSelected - 1;
					SelectItem(nClicked);
					OnSelect();
					bHandled = true;
				}
				else if (m_event->IsMapTo(nEvent, EM_KEY_DOWN)) {
					nClicked = m_nSelected + 1;
					SelectItem(nClicked);
					OnSelect();
					bHandled = true;
				}
				else if (m_event->IsMapTo(nEvent, EM_KEY_PAGE_DOWN)) {
					nClicked = m_nSelected + m_VScroll->GetPageSize() - 1;
					SelectItem(nClicked);
					OnSelect();
					bHandled = true;
				}
				else if (m_event->IsMapTo(nEvent, EM_KEY_PAGE_UP)) {
					nClicked = m_nSelected - (m_VScroll->GetPageSize() - 1);
					SelectItem(nClicked);
					OnSelect();
					bHandled = true;
				}
				else if (m_event->IsMapTo(nEvent, EM_KEY_HOME)) {
					nClicked = 0;
					SelectItem(nClicked);
					OnSelect();
					bHandled = true;
				}
				else if (m_event->IsMapTo(nEvent, EM_KEY_END)) {
					nClicked = (int)m_items.size() - 1;
					SelectItem(nClicked);
					OnSelect();
					bHandled = true;
				}

			}
		}
	}
	if (!bHandled) {
		MSG msg = m_event->GenerateMessage();
		return CGUIContainer::MsgProc(&msg);
	}
	else
		return true;

}

void CGUIListBox::SelectItem(int index)
{
	// If no item exists, do nothing.
	if (m_items.size() == 0)
		return;

	int nOldSelected = m_nSelected;

	// Adjust m_nSelected
	m_nSelected = index;

	// Perform capping
	if (m_nSelected < 0)
		m_nSelected = 0;
	if (m_nSelected >= (int)m_items.size())
		m_nSelected = (int)m_items.size() - 1;

	if (m_bMultiSelect) {
		// Determine behavior based on the state of Shift and Ctrl

		GUIListItem *pSelItem = m_itemmap.find(m_nSelected)->second;
		if (ControlPressed&&!ShiftPressed)
		{
			// Control click. Reverse the selection of this item.

			pSelItem->selected = !pSelItem->selected;
			m_nSelStart = m_nSelected;
		}
		else if ((ShiftPressed&&!ControlPressed))
		{
			// Shift click. Set the selection for all items
			// from last selected item to the current item.
			// Clear everything else.

			int nBegin = Math::Min(m_nSelStart, m_nSelected);
			int nEnd = Math::Max(m_nSelStart, m_nSelected);

			for (int i = 0; i < nBegin; ++i)
			{
				GUIListItem *pItem = m_itemmap.find(i)->second;
				pItem->selected = false;
			}

			for (int i = nEnd + 1; i < (int)m_items.size(); ++i)
			{
				GUIListItem *pItem = m_itemmap.find(i)->second;
				pItem->selected = false;
			}

			for (int i = nBegin; i <= nEnd; ++i)
			{
				GUIListItem *pItem = m_itemmap.find(i)->second;
				pItem->selected = true;
			}
		}
		else if (ControlPressed&&ShiftPressed)
		{
			// Control-Shift-click.

			// The behavior is:
			//   Set all items from m_nSelStart to m_nSelected to
			//     the same state as m_nSelStart, not including m_nSelected.
			//   Set m_nSelected to selected.

			int nBegin = Math::Min(m_nSelStart, m_nSelected);
			int nEnd = Math::Max(m_nSelStart, m_nSelected);

			// The two ends do not need to be set here.

			bool bLastSelected = m_itemmap.find(m_nSelStart)->second->selected;
			for (int i = nBegin + 1; i < nEnd; ++i)
			{
				GUIListItem *pItem = m_itemmap.find(i)->second;
				pItem->selected = bLastSelected;
			}

			pSelItem->selected = bLastSelected;

			// Restore m_nSelected to the previous value
			// This matches the Windows behavior

			m_nSelected = m_nSelStart;
		}
		else{
			// Simple click.  Clear all items and select the clicked
			// item.
			for (int i = 0; i < (int)m_items.size(); ++i)
			{
				GUIListItem *pItem = m_itemmap.find(i)->second;
				pItem->selected = false;
			}

			pSelItem->selected = true;
			// Update selection start
			m_nSelStart = m_nSelected;
		}
	}
	else{
		// Update selection start
		m_nSelStart = m_nSelected;
	}
	// Adjust scroll bar
	m_VScroll->ShowItem(m_nSelected);

}
void CGUIListBox::AddItem(const GUIListItem& listitem)
{
	m_items.push_back(listitem);
	m_itemmap[(int)(m_items.size() - 1)] = &m_items.back();
	m_bNeedUpdate = true;
	m_VScroll->SetTrackRange(0, (int)m_items.size());
	SetDirty(true);
}

void CGUIListBox::AddTextItemA(const char* szText)
{
	if (szText == NULL) {
		return;
	}
	GUIListItem item;
	StringHelper::UTF8ToUTF16_Safe(szText, item.text);
	item.onclick = GUI_SCRIPT();
	item.selected = false;
	item.advObj = NULL;
	AddItem(item);
}

//the inserted item will be at the position given by parameter index
void CGUIListBox::InsertItem(int index, const GUIListItem* listitem)
{
	if (index<0 || index>(int)m_items.size()) {
		return;
	}
	list<GUIListItem>::iterator iter;
	int a = 0, b;
	for (iter = m_items.begin(); iter != m_items.end(); iter++, a++) {
		if (index == a) {
			break;
		}
	}
	iter = m_items.insert(iter, *listitem);
	for (b = a + 1; b < (int)m_items.size(); b++) {
		m_itemmap[b] = m_itemmap[b - 1];
	}
	m_itemmap[a] = &(*iter);
	m_VScroll->SetTrackRange(0, (int)m_items.size());
	m_bNeedUpdate = true;
	SetDirty(true);
}

void CGUIListBox::DeleteItem(int index)
{
	if (index < 0 || index >= (int)m_items.size()) {
		return;
	}
	if (index == m_nSelected){
		m_nSelected = -1;
	}
	list<GUIListItem>::iterator iter;
	int a = 0, b;
	for (iter = m_items.begin(); iter != m_items.end(); iter++, a++) {
		if (index == a) {
			m_items.erase(iter);
			break;
		}
	}
	if (m_nSelected >= (int)m_items.size()){
		m_nSelected = (int)m_items.size() - 1;
	}
	for (b = a; b < (int)m_items.size(); b++) {
		m_itemmap[b] = m_itemmap[b + 1];
	}
	m_itemmap.erase(b);
	m_VScroll->SetTrackRange(0, (int)m_items.size());
	m_bNeedUpdate = true;
	SetDirty(true);
}

void CGUIListBox::RemoveAll()
{
	m_items.clear();
	m_itemmap.clear();
	m_VScroll->SetTrackRange(0, 1);
	m_nSelected = -1;
	m_nSelStart = -1;
	m_bNeedUpdate = true;
	SetDirty(true);
}
GUIListItem* CGUIListBox::GetItem(int index)
{
	if (index < 0 || index >= (int)m_items.size()) {
		return NULL;
	}
	return m_itemmap[index];
}

HRESULT CGUIListBox::Render(GUIState* pGUIState, float fElapsedTime)
{
	SetDirty(false);
	GUITextureElement* pElement, *pSelElement;
	GUIFontElement* pFontElement;
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();
	RECT rcScreen, rcWindow, rcText, rcSel;//rcScreen is the client rect of this container,rcText is the text rect of the container,rcWindow is the whole area
	pElement = m_objResource->GetTextureElement(0);
	rcScreen = m_objResource->GetDrawingRects(0);
	rcWindow = m_objResource->GetDrawingRects(2);
	rcText = m_objResource->GetDrawingRects(3);
	rcSel.left = rcScreen.left;
	rcSel.right = rcScreen.right;

	if (m_objResource->GetLayerType() == GUILAYER::ONE_ELEMENT)
	{
		DrawElement(pElement, &rcWindow, &rcWindow);
	}
	else if (m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT)
	{
		for (int e = 0; e < 9; ++e)
		{
			RECT rc = m_objResource->GetDrawingRects(e + 4);
			DrawElement(m_objResource->GetTextureElement(e + 2), &rc, &rcWindow);
		}
	}

	list<GUIListItem>::iterator iter, iterend;
	iterend = m_items.end();
	int counter = 0;
	RECT rcChild;
	pFontElement = m_objResource->GetFontElement(0);
	rcChild.left = rcText.left;
	rcChild.right = rcText.right;
	rcChild.top = rcText.top;
	rcChild.bottom = rcChild.top + m_ItemHeight;
	for (iter = m_items.begin(); iter != iterend; iter++, counter++)
	{
		if (counter < m_VScroll->GetTrackPos()) {
			continue;
		}
		if (rcChild.bottom >= rcText.bottom) {
			break;
		}
		GUIListItem* pItem = &(*iter);
		bool bSelectedStyle = false;

		if (!(m_bMultiSelect) && counter == m_nSelected)
			bSelectedStyle = true;
		else
			if (m_bMultiSelect)
			{
				if (m_bDrag &&
					((counter >= m_nSelStart && counter <= m_nSelected) ||
					(counter <= m_nSelStart && counter >= m_nSelected)))
					bSelectedStyle = m_itemmap[m_nSelStart]->selected;
				else
					if (pItem->selected)
						bSelectedStyle = true;
			}
		if (m_bAllowAdvObject) {
			pItem->advObj->DoRender(pGUIState, fElapsedTime);
		}
		else{

			if (bSelectedStyle) {
				rcSel.top = rcChild.top;
				rcSel.bottom = rcChild.bottom;
				pSelElement = m_objResource->GetTextureElement("selection");
				if (pSelElement) {
					GetPainter(pGUIState)->DrawSprite(pSelElement, &rcSel, m_position.GetDepth());
				}
				else
					GetPainter(pGUIState)->DrawRect(&rcSel, m_SelBkColor, m_position.GetDepth());
				pFontElement = m_objResource->GetFontElement(1);

				GetPainter(pGUIState)->DrawText(pItem->text.c_str(), pFontElement, &rcChild, m_position.GetDepth());
			}
			else{
				pFontElement = m_objResource->GetFontElement(0);
				GetPainter(pGUIState)->DrawText(pItem->text.c_str(), pFontElement, &rcChild, m_position.GetDepth());
			}

		}
		OffsetRect(&rcChild, 0, m_ItemHeight);
	}
	if (m_bScrollable) {

		//draw scroll bars
		if (m_ScrollType&VSCROLL) {
			m_VScroll->Render(pGUIState, fElapsedTime);
		}
		// 		if (m_ScrollType&HSCROLL) {
		// 			m_HScroll->Render(pGUIState);
		// 		}


	}

	return S_OK;
}


void CGUIListBox::UpdateRects()
{
	//decide whether to show the vertical scrollbar
	CGUIPosition tempPos;
	GetAbsolutePosition(&tempPos, &m_position);
	if (m_position.IsRelativeTo3DObject())
	{
		SetDepth(tempPos.GetDepth());
	}
	RECT rcText = tempPos.rect;
	RECT rcInner = tempPos.rect;

	if (m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT)
	{
		const int offset = 4;
		const int textureOffset = 2;
		RECT tempRect;
		// use the left top and right bottom texture element size to calculate the inner rect size. 
		GUITextureElement* pElement = m_objResource->GetTextureElement(textureOffset + 1);
		if (pElement)
		{
			pElement->GetRect(&tempRect);
			rcInner.left = RectWidth(tempRect) + tempPos.rect.left;
			rcInner.top = RectHeight(tempRect) + tempPos.rect.top;
		}
		pElement = m_objResource->GetTextureElement(textureOffset + 8);
		if (pElement)
		{
			pElement->GetRect(&tempRect);
			rcInner.right = tempPos.rect.right - RectWidth(tempRect);
			rcInner.bottom = tempPos.rect.bottom - RectHeight(tempRect);
		}

		// Element assignment:
		//   0 - text area
		//   1 - top left border
		//   2 - top border
		//   3 - top right border
		//   4 - left border
		//   5 - right border
		//   6 - lower left border
		//   7 - lower border
		//   8 - lower right border
		m_objResource->SetDrawingRects(&rcInner, offset + 0);
		SetRect(&tempRect, tempPos.rect.left, tempPos.rect.top, rcInner.left, rcInner.top);
		m_objResource->SetDrawingRects(&tempRect, offset + 1);
		SetRect(&tempRect, rcInner.left, tempPos.rect.top, rcInner.right, rcInner.top);
		m_objResource->SetDrawingRects(&tempRect, offset + 2);
		SetRect(&tempRect, rcInner.right, tempPos.rect.top, tempPos.rect.right, rcInner.top);
		m_objResource->SetDrawingRects(&tempRect, offset + 3);
		SetRect(&tempRect, tempPos.rect.left, rcInner.top, rcInner.left, rcInner.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 4);
		SetRect(&tempRect, rcInner.right, rcInner.top, tempPos.rect.right, rcInner.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 5);
		SetRect(&tempRect, tempPos.rect.left, rcInner.bottom, rcInner.left, tempPos.rect.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 6);
		SetRect(&tempRect, rcInner.left, rcInner.bottom, rcInner.right, tempPos.rect.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 7);
		SetRect(&tempRect, rcInner.right, rcInner.bottom, tempPos.rect.right, tempPos.rect.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 8);
	}

	GUIFontElement* pFontElement = m_objResource->GetFontElement(0);
	InflateRect(&rcText, 0, -m_nBorder);
	if (pFontElement)
	{
		int fontsize = pFontElement->GetFont()->m_nFontSize;

		if (m_ItemHeight < fontsize) {
			m_ItemHeight = fontsize + 4;
		}
	}
	else if (m_ItemHeight == 0) {
		m_ItemHeight = 16;
	}
	if ((int)m_items.size()*m_ItemHeight > RectHeight(rcText)) {
		m_ScrollType |= VSCROLL;
	}
	else
		m_ScrollType &= (~VSCROLL);
	m_objResource->SetDrawingRects(&tempPos.rect, 2);
	InflateRect(&tempPos.rect, -m_nBorder, -m_nBorder);
	m_objResource->SetDrawingRects(&tempPos.rect, 1);
	{
		if (m_bScrollable && (m_ScrollType&VSCROLL)) tempPos.rect.right = tempPos.rect.right - m_nSBWidth;
		if (m_bScrollable && (m_ScrollType&HSCROLL)) tempPos.rect.bottom = tempPos.rect.bottom - m_nSBWidth;
		{
			m_VScroll->SetSize(m_nSBWidth, RectHeight(tempPos.rect));
			m_VScroll->SetPageSize(RectHeight(rcText) / m_ItemHeight);
			m_VScroll->UpdateRects();
		}
		// 		{
		// 			m_HScroll->SetSize(RectWidth(tempPos.rect),m_nSBWidth);
		// 			m_HScroll->UpdateRects();
		// 		}	
	}
	m_objResource->SetDrawingRects(&tempPos.rect, 0);
	rcText = tempPos.rect;
	InflateRect(&rcText, -m_nMargin, 0);
	m_objResource->SetDrawingRects(&rcText, 3);


	//update rectangle of the advance objects.
	if (m_bAllowAdvObject) {
		list<GUIListItem>::iterator iter, iterend;
		iterend = m_items.end();
		int counter = 0;
		for (iter = m_items.begin(); iter != iterend; iter++, counter++) {
			if ((*iter).advObj != NULL) {
				RECT rcNew = m_objResource->GetDrawingRects(0);
				rcNew.right = rcNew.right - rcNew.left; rcNew.bottom = counter*m_ItemHeight - m_deltaY + m_ItemHeight;
				rcNew.left = 0; rcNew.top = counter*m_ItemHeight - m_deltaY;
				(*iter).advObj->SetPosition(rcNew.left, rcNew.top, rcNew.right, rcNew.bottom);
				(*iter).advObj->UpdateRects();
			}
		}
	}
	m_bNeedUpdate = false;
}

void CGUIListBox::UpdateScroll(int nXDelta, int nYDelta)
{
	if (nXDelta == 0 && nYDelta == 0) {
		return;
	}
	m_deltaY = m_ItemHeight*(m_VScroll->GetTrackPos() - m_VScroll->GetTrackStart());
	m_deltaX += nXDelta;
	m_bNeedUpdate = true;
	SetDirty(true);
}

void CGUIListBox::SetItemHeight(int height)
{
	if (m_bAllowAdvObject) {

	}
	m_ItemHeight = height;
	m_bNeedUpdate = true;
	SetDirty(true);
}

int CGUIListBox::GetTextA(int index, std::string& out)
{
	const char16_t* str = GetText(index);
	if (str)
		return StringHelper::UTF16ToUTF8(str, out) ? (int)out.size() : 0;
	else
		return 0;
}

const char16_t* CGUIListBox::GetText(int index)
{
	if (GetItem(index)) {
		return GetItem(index)->text.c_str();
	}
	return NULL;
}

const char16_t* ParaEngine::CGUIListBox::GetText()
{
	return GetText(GetSelected());
}

int ParaEngine::CGUIListBox::GetTextA(std::string& out)
{
	return GetTextA(GetSelected(), out);
}
