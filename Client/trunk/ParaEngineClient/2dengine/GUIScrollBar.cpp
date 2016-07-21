//----------------------------------------------------------------------
// Class:	CGUIContainer
// Authors:	LiXizhi, LiuWeili, 
// Date:	2005.8.1
// Revised: 2005.8.1
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "GUIResource.h"
#include "ParaWorldAsset.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "Type.h"
#include "GUIRoot.h"
#include "util/StringHelper.h"
#include "ic/ICConfigManager.h"
#include "GUIScrollBar.h"

using namespace ParaEngine;

// Delay and repeat period when clicking on the scroll bar arrows
#define SCROLLBAR_ARROWCLICK_DELAY  330
#define SCROLLBAR_ARROWCLICK_REPEAT 33

const IType* CGUIScrollBar::m_type = NULL;


//////////////////////////////////////////////////////////////////////////
// CGUIScrollbar
//////////////////////////////////////////////////////////////////////////
CGUIScrollBar::CGUIScrollBar() :CGUIBase()
{
	if (!m_type){
		m_type = IType::GetType("guiscrollbar");
	}
	//	m_objType =(m_objType&(~CGUIBase::COMMON_CONTROL_MASK))| CGUIBase::Type_GUIScrollBar;
	m_parent = NULL;

	m_bShowThumb = true;

	m_nDelta = 1;
	m_nPosition = 0;
	m_nPageSize = 1;
	m_nStart = 0;
	m_nEnd = 1;
	m_Arrow = CLEAR;
	m_dArrowTS = 0;
	m_bCanHasFocus = false;
	m_Linked = NULL;
	m_bFixedThumb = true;
	m_nThumbSize = 20;
	m_nOldPosition = -1;
}
void CGUIScrollBar::Clone(IObject* pobj)const
{
	PERF_BEGIN("Scrollbar clone");
	if (pobj == NULL) {
		return;
	}
	CGUIScrollBar* pScrollbar = (CGUIScrollBar*)pobj;
	CGUIBase::Clone(pScrollbar);
	pScrollbar->m_bShowThumb = m_bShowThumb;
	pScrollbar->m_nPosition = m_nPosition;
	pScrollbar->m_nPageSize = m_nPageSize;
	pScrollbar->m_nStart = m_nStart;
	pScrollbar->m_nEnd = m_nEnd;
	pScrollbar->m_Arrow = CLEAR;
	pScrollbar->m_dArrowTS = 0;
	pScrollbar->m_ScrollType = m_ScrollType;
	pScrollbar->m_nDelta = m_nDelta;
	pScrollbar->m_bFixedThumb = m_bFixedThumb;
	pScrollbar->m_nThumbSize = m_nThumbSize;
	PERF_END("Scrollbar clone");
}

IObject* CGUIScrollBar::Clone()const
{
	CGUIScrollBar* pScrollbar = new CGUIScrollBar();
	Clone(pScrollbar);
	return pScrollbar;
}


//--------------------------------------------------------------------------------------
CGUIScrollBar::~CGUIScrollBar()
{
}

void CGUIScrollBar::StaticInit()
{
	CObjectManager *pOm = &CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIBase")) {
		CGUIBase::StaticInit();
	}
	//load the default CGUIBase object and copy all its value to the new button

	CGUIScrollBar *pScrollbar = new CGUIScrollBar();
	pOm->CloneObject("default_CGUIBase", pScrollbar);
	//	pScrollbar->m_objType=CGUIBase::Type_GUIScrollBar;
	pScrollbar->m_bShowThumb = true;
	pScrollbar->m_nPosition = 0;
	pScrollbar->m_nPageSize = 1;
	pScrollbar->m_nStart = 0;
	pScrollbar->m_nEnd = 1;
	pScrollbar->m_Arrow = CLEAR;
	pScrollbar->m_dArrowTS = 0;
	pScrollbar->m_ScrollType = 0;
	pScrollbar->m_nDelta = 1;
	CEventBinding* pBinding = pScrollbar->m_event->GetEventBindingObj();
	pBinding->EnableKeyboard();
	pBinding->EnableMouse();

	//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_CTRL_CAPTUREMOUSE);
	//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_SB_ACTIONBEGIN);
	//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_CTRL_RELEASEMOUSE);
	//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_SB_ACTIONEND);
	//	pBinding->MapEvent(EM_KEY_PAGE_DOWN,EM_SB_PAGEDOWN);
	//	pBinding->MapEvent(EM_KEY_PAGE_UP,EM_SB_PAGEUP);
	pBinding->DisableEvent(EM_MOUSE_DBCLICK);
	pBinding->DisableEvent(EM_MOUSE_CLICK);


	pBinding->DisableEvent(EM_MOUSE_DBCLICK);

	using namespace ParaInfoCenter;
	CICConfigManager *cm = CGlobals::GetICConfigManager();
	string value0, value1;
	int event0, event1, a;
	DWORD b;
	HRESULT hr;
	hr = cm->GetSize("GUI_scrollbar_control_mapping", &b);
	if (hr == E_INVALIDARG || hr == E_ACCESSDENIED) {
		//error
	}
	else{
		for (a = 0; a<(int)b; a += 2) {
			hr = cm->GetTextValue("GUI_scrollbar_control_mapping", value0, a);
			if (FAILED(hr)) {
				break;
			}
			hr = cm->GetTextValue("GUI_scrollbar_control_mapping", value1, a + 1);
			if (FAILED(hr)) {
				break;
			}
			event0 = CEventBinding::StringToEventValue(value0);
			event1 = CEventBinding::StringToEventValue(value1);
			pBinding->MapEvent(event0, event1);
		}
	}
	int tempint;
	if (cm->GetIntValue("GUI_scrollbar_control_canhasfocus", &tempint) == S_OK) {
		if (tempint == 0) {
			pScrollbar->m_bCanHasFocus = false;
		}
		else
			pScrollbar->m_bCanHasFocus = true;
	}
	if (cm->GetIntValue("GUI_scrollbar_control_visible", &tempint) == S_OK) {
		if (tempint == 0) {
			pScrollbar->m_bIsVisible = false;
		}
		else
			pScrollbar->m_bIsVisible = true;
	}
	if (cm->GetIntValue("GUI_scrollbar_control_enable", &tempint) == S_OK) {
		if (tempint == 0) {
			pScrollbar->m_bIsEnabled = false;
		}
		else
			pScrollbar->m_bIsEnabled = true;
	}
	if (cm->GetIntValue("GUI_scrollbar_control_candrag", &tempint) == S_OK) {
		if (tempint == 0) {
			pScrollbar->SetCandrag(false);
		}
		else
			pScrollbar->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_scrollbar_control_lifetime", &tempint) == S_OK) {
		pScrollbar->m_nLifeTimeCountDown = tempint;
	}

	//Set the default texture and font of the default scrollbar

	pScrollbar->m_objResource->SetActiveLayer();
	pScrollbar->m_objResource->SetCurrentState();
	pScrollbar->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
	RECT *prect = NULL;
	RECT rect;
	TextureEntity* pTexture = NULL;
	SpriteFontEntity* pFont = CGlobals::GetAssetManager()->GetFont("sys");
	GUIFontElement font_;
	GUIFontElement* pFontElement = &font_;
	GUITextureElement tex_;
	GUITextureElement* pElement = &tex_;
	string background, filename;
	bool bLoadDefault = true;
	if (cm->GetTextValue("GUI_scrollbar_control_track", background) == S_OK) {
		prect = ParaEngine::StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 243, 144, 265, 155);
	}
	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pScrollbar->m_objResource->AddElement(pElement, "track");

	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 240, 240, 240));
	pScrollbar->m_objResource->AddElement(pElement, "track");

	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 180, 180, 180));
	pScrollbar->m_objResource->AddElement(pElement, "track");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_scrollbar_control_upleft", background) == S_OK) {
		prect = ParaEngine::StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 243, 124, 265, 144);
	}
	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pScrollbar->m_objResource->AddElement(pElement, "up_left");

	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 240, 240, 240));
	pScrollbar->m_objResource->AddElement(pElement, "up_left");

	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 180, 180, 180));
	pScrollbar->m_objResource->AddElement(pElement, "up_left");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_scrollbar_control_downright", background) == S_OK) {
		prect = ParaEngine::StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 243, 155, 265, 176);
	}
	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pScrollbar->m_objResource->AddElement(pElement, "down_right");

	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 240, 240, 240));
	pScrollbar->m_objResource->AddElement(pElement, "down_right");

	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 180, 180, 180));
	pScrollbar->m_objResource->AddElement(pElement, "down_right");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_scrollbar_control_thumb", background) == S_OK) {
		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 266, 123, 286, 167);
	}
	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pScrollbar->m_objResource->AddElement(pElement, "thumb");

	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 240, 240, 240));
	pScrollbar->m_objResource->AddElement(pElement, "thumb");

	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 180, 180, 180));
	pScrollbar->m_objResource->AddElement(pElement, "thumb");

	pScrollbar->m_objResource->SetCurrentState(GUIResourceState_Highlight);
	pScrollbar->m_objResource->CloneState("normal");

	pScrollbar->m_objResource->SetCurrentState();
	pScrollbar->m_objResource->SetActiveLayer();

	pOm->SetObject("default_CGUIScrollbar", pScrollbar);
	SAFE_RELEASE(pScrollbar);
	//the pBase don't need to be released because it is obtained using GetObject()
}

void CGUIScrollBar::SetScrollbarWidth(int width)
{
	PE_ASSERT(width>0);
	if (m_ScrollType == HSCROLL){
		SetHeightI(width);
	}
	else if (m_ScrollType == VSCROLL){
		SetWidthI(width);
	}
}

int CGUIScrollBar::GetScrollbarWidth()const
{
	if (m_ScrollType == HSCROLL){
		return GetHeight();
	}
	else if (m_ScrollType == VSCROLL){
		return GetWidth();
	}
	return 0;
}
//--------------------------------------------------------------------------------------
void CGUIScrollBar::UpdateRects()
{
	/*
	m_objResource->DrawingRects[0]: track rectangle;
	m_objResource->DrawingRects[1]: up/left button rectangle;
	m_objResource->DrawingRects[2]: down/right button rectangle;
	m_objResource->DrawingRects[3]: thumb button rectangle;
	*/

	CGUIBase::UpdateRects();

	// Make the buttons square
	CGUIPosition tempPos;
	GetAbsolutePosition(&tempPos, &m_position);
	if (m_position.IsRelativeTo3DObject())
	{
		SetDepth(tempPos.GetDepth());
	}

	if (tempPos.GetWidth()>tempPos.GetHeight()) {
		m_ScrollType = HSCROLL;
	}
	else
	{
		m_ScrollType = VSCROLL;
	}
	RECT tempRect;
	if (m_ScrollType == HSCROLL) {
		SetRect(&tempRect, tempPos.rect.left, tempPos.rect.top,
			tempPos.rect.left + tempPos.GetHeight(), tempPos.rect.bottom);
		m_objResource->SetDrawingRects(&tempRect, 1);
		SetRect(&tempRect, tempPos.rect.right - tempPos.GetHeight(), tempPos.rect.top,
			tempPos.rect.right, tempPos.rect.bottom);
		m_objResource->SetDrawingRects(&tempRect, 2);
		SetRect(&tempRect, m_objResource->GetDrawingRects(1).right, m_objResource->GetDrawingRects(1).top,
			m_objResource->GetDrawingRects(2).left, m_objResource->GetDrawingRects(2).bottom);
		m_objResource->SetDrawingRects(&tempRect, 0);
		tempRect = m_objResource->GetDrawingRects(3);
		tempRect.top = m_objResource->GetDrawingRects(1).top;
		tempRect.bottom = m_objResource->GetDrawingRects(1).bottom;
		m_objResource->SetDrawingRects(&tempRect, 3);

	}
	else{
		SetRect(&tempRect, tempPos.rect.left, tempPos.rect.top,
			tempPos.rect.right, tempPos.rect.top + tempPos.GetWidth());
		m_objResource->SetDrawingRects(&tempRect, 1);
		SetRect(&tempRect, tempPos.rect.left, tempPos.rect.bottom - tempPos.GetWidth(),
			tempPos.rect.right, tempPos.rect.bottom);
		m_objResource->SetDrawingRects(&tempRect, 2);
		SetRect(&tempRect, m_objResource->GetDrawingRects(1).left, m_objResource->GetDrawingRects(1).bottom,
			m_objResource->GetDrawingRects(2).right, m_objResource->GetDrawingRects(2).top);
		m_objResource->SetDrawingRects(&tempRect, 0);
		tempRect = m_objResource->GetDrawingRects(3);
		tempRect.left = m_objResource->GetDrawingRects(1).left;
		tempRect.right = m_objResource->GetDrawingRects(1).right;
		m_objResource->SetDrawingRects(&tempRect, 3);
	}

	UpdateThumbRect();
	m_bNeedUpdate = false;
}


//--------------------------------------------------------------------------------------
// Compute the dimension of the scroll thumb
void CGUIScrollBar::UpdateThumbRect()
{
	RECT tempRect = m_objResource->GetDrawingRects(3);
	if (m_ScrollType == HSCROLL) {

		if (m_nEnd - m_nStart > m_nPageSize)
		{
			int nThumbWidth = m_bFixedThumb ? m_nThumbSize : Math::Max(RectWidth(m_objResource->GetDrawingRects(0)) * m_nPageSize / (m_nEnd - m_nStart), SCROLLBAR_MINTHUMBSIZE);
			int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
			tempRect.left = m_objResource->GetDrawingRects(0).left + (m_nPosition - m_nStart) * (RectWidth(m_objResource->GetDrawingRects(0)) - nThumbWidth)
				/ nMaxPosition;
			tempRect.right = tempRect.left + nThumbWidth;
			m_bShowThumb = true;

		}
		else
		{
			// No content to scroll
			tempRect.right = tempRect.left;
			m_bShowThumb = false;
		}
		m_objResource->SetDrawingRects(&tempRect, 3);
	}
	else{
		if (m_nEnd - m_nStart > m_nPageSize)
		{
			int nThumbHeight = m_bFixedThumb ? m_nThumbSize : Math::Max(RectHeight(m_objResource->GetDrawingRects(0)) * m_nPageSize / (m_nEnd - m_nStart), SCROLLBAR_MINTHUMBSIZE);
			int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
			tempRect.top = m_objResource->GetDrawingRects(0).top + (m_nPosition - m_nStart) * (RectHeight(m_objResource->GetDrawingRects(0)) - nThumbHeight)
				/ nMaxPosition;
			tempRect.bottom = tempRect.top + nThumbHeight;
			m_bShowThumb = true;

		}
		else
		{
			// No content to scroll
			tempRect.bottom = tempRect.top;
			m_bShowThumb = false;
		}

		m_objResource->SetDrawingRects(&tempRect, 3);
		if (!m_bFixedThumb && m_bShowThumb)
		{
			UpdateThumbRectNineElement();
		}
	}

}



//--------------------------------------------------------------------------------------
// Scroll() scrolls by nDelta items.  A positive value scrolls down, while a negative
// value scrolls up.
void CGUIScrollBar::Scroll(int nDelta)
{
	int tempPos = m_nPosition;
	// Perform scroll
	m_nPosition += nDelta;

	// Cap position
	Cap();

	// Update thumb position
	UpdateThumbRect();


	if (m_Linked) {
		if (m_ScrollType == HSCROLL) {
			m_Linked->UpdateScroll(tempPos - m_nPosition, 0);
		}
		if (m_ScrollType == VSCROLL) {
			m_Linked->UpdateScroll(0, tempPos - m_nPosition);
		}
	}
	OnChange();
}


//--------------------------------------------------------------------------------------
void CGUIScrollBar::ShowItem(int nIndex)
{
	// Cap the index

	if (nIndex < 0)
		nIndex = 0;

	if (nIndex >= m_nEnd)
		nIndex = m_nEnd - 1;

	// Adjust position

	if (m_nPosition > nIndex)
		m_nPosition = nIndex;
	else
		if (m_nPosition + m_nPageSize <= nIndex)
			m_nPosition = nIndex - m_nPageSize + 1;

	UpdateThumbRect();
}

bool CGUIScrollBar::OnChange(const char* code)
{
	if (!HasEvent(EM_CTRL_CHANGE))
		return false;
	return ActivateScript("", EM_CTRL_CHANGE);
}

bool CGUIScrollBar::MsgProc(MSG *event)
{
	//one call to this function will trigger at most one event
	if (!m_bIsEnabled)
		return false;
	bool bHandled = false;
	static int ThumbOffsetY = 0;
	static int ThumbOffsetX = 0;
	static bool bDrag = false;
	CDirectMouse *pMouse = CGUIRoot::GetInstance()->m_pMouse;
	CDirectKeyboard *pKeyboard = CGUIRoot::GetInstance()->m_pKeyboard;
	STRUCT_DRAG_AND_DROP *pdrag = &IObjectDrag::DraggingObject;
	if (event != NULL&&!m_event->InterpretMessage(event)) {
		return false;
	}
	int nEvent = m_event->GetTriggerEvent();
	POINT pt;
	pt.x = m_event->m_mouse.x;
	pt.y = m_event->m_mouse.y;
	MSG newMsg;
	CEventBinding::InitMsg(&newMsg, event->time, nEvent, pt);
	if (m_bIsVisible){
		if (nEvent == EM_NONE) {
			return false;
		}
		if (m_event->IsMapTo(nEvent, EM_MOUSE_WHEEL)) {
			if (m_ScrollType&VSCROLL) {
				int nScrollAmount = int(event->lParam) / 120 * CGUIRoot::WheelScrollLines;
				Scroll(-nScrollAmount);
				bHandled = true;
			}
		}
		if (m_event->IsMapTo(nEvent, EM_SB_ACTIONEND)) {
			bDrag = false;
			bHandled = true;
			UpdateThumbRect();
			m_Arrow = CLEAR;
			newMsg.message = EM_CTRL_RELEASEMOUSE;
			CGUIBase::MsgProc(&newMsg);
			ReleaseMouseCapture();
		}
		if (m_event->IsMapTo(nEvent, EM_SB_PAGEDOWN)) {
			Scroll(m_nPageSize);
			bHandled = true;
		}
		else if (m_event->IsMapTo(nEvent, EM_SB_PAGEUP)) {
			Scroll(-m_nPageSize);
			bHandled = true;
		}
		else if (m_event->IsMapTo(nEvent, EM_SB_STEPDOWN)) {
			Scroll(m_nDelta);
			bHandled = true;
		}
		else if (m_event->IsMapTo(nEvent, EM_SB_STEPUP)) {
			Scroll(-m_nDelta);
			bHandled = true;
		}
		else if (m_event->IsMapTo(nEvent, EM_SB_SCROLL)) {
			Scroll((int)event->lParam);
			bHandled = true;
		}
		else if (m_event->IsMapTo(nEvent, EM_MOUSE_DRAGEND) && pdrag->pDragging != this) {
			//scrollbar does not accept dragging anything into it
			return false;
		}

		bool IsRepeat = false;
		DWORD dCurrTime = event->time;
		switch (m_Arrow)
		{
		case CLICKED_UP:
		case CLICKED_DOWN:
		case CLICKED:
			if (CGUIRoot::MouseDelay < dCurrTime - m_dArrowTS)
			{
				//m_Arrow = HELD;
				m_dArrowTS = dCurrTime;
				IsRepeat = true;
			}

			break;
		case HELD_UP:
		case HELD_DOWN:
		case HELD:
			if (CGUIRoot::MouseRepeat < dCurrTime - m_dArrowTS)
			{
				IsRepeat = true;
				m_dArrowTS = dCurrTime;
			}
			break;

		default:
			IsRepeat = true;
		}

		if (m_event->IsMapTo(nEvent, EM_CTRL_FRAMEMOVE)) {
			//update the thumb position
			if (bDrag)
			{
				int oldPos = m_nPosition;
				RECT tempRect = m_objResource->GetDrawingRects(3);
				if (m_ScrollType == HSCROLL) {

					tempRect.right += pt.x - ThumbOffsetX - tempRect.left;
					tempRect.left = pt.x - ThumbOffsetX;
					if (tempRect.left < m_objResource->GetDrawingRects(0).left)
						OffsetRect(&tempRect, m_objResource->GetDrawingRects(0).left - tempRect.left, 0);
					else
						if (tempRect.right > m_objResource->GetDrawingRects(0).right)
							OffsetRect(&tempRect, m_objResource->GetDrawingRects(0).right - tempRect.right, 0);

					// Compute first item index based on thumb position

					int nMaxFirstItem = m_nEnd - m_nStart - m_nPageSize;  // Largest possible index for first item
					int nMaxThumb = RectWidth(m_objResource->GetDrawingRects(0)) - RectWidth(tempRect);  // Largest possible thumb position from the top

					m_nPosition = m_nStart +
						(tempRect.left - m_objResource->GetDrawingRects(0).left +
						nMaxThumb / (nMaxFirstItem * 2)) * // Shift by half a row to avoid last row covered by only one pixel
						nMaxFirstItem / nMaxThumb;

				}
				else{
					tempRect.bottom += pt.y - ThumbOffsetY - tempRect.top;
					tempRect.top = pt.y - ThumbOffsetY;
					if (tempRect.top < m_objResource->GetDrawingRects(0).top)
						OffsetRect(&tempRect, 0, m_objResource->GetDrawingRects(0).top - tempRect.top);
					else
						if (tempRect.bottom > m_objResource->GetDrawingRects(0).bottom)
							OffsetRect(&tempRect, 0, m_objResource->GetDrawingRects(0).bottom - tempRect.bottom);

					// Compute first item index based on thumb position

					int nMaxFirstItem = m_nEnd - m_nStart - m_nPageSize;  // Largest possible index for first item
					int nMaxThumb = RectHeight(m_objResource->GetDrawingRects(0)) - RectHeight(tempRect);  // Largest possible thumb position from the top

					m_nPosition = m_nStart +
						(tempRect.top - m_objResource->GetDrawingRects(0).top +
						nMaxThumb / (nMaxFirstItem * 2)) * // Shift by half a row to avoid last row covered by only one pixel
						nMaxFirstItem / nMaxThumb;

				}
				m_objResource->SetDrawingRects(&tempRect, 3);
				UpdateThumbRectNineElement();

				if (m_Linked) {
					if (m_ScrollType == HSCROLL) {
						m_Linked->UpdateScroll(oldPos - m_nPosition, 0);
					}
					if (m_ScrollType == VSCROLL) {
						m_Linked->UpdateScroll(0, oldPos - m_nPosition);
					}
				}
				OnChange();
			}
			// Check for click on up button
			if (PtInRect(m_objResource->GetDrawingRects(1), pt))
			{
				if (IsRepeat && (m_Arrow == HELD_UP || m_Arrow == CLICKED_UP))
				{
					newMsg.message = EM_SB_STEPUP;
					MsgProc(&newMsg);
					if (m_Arrow == CLICKED_UP) m_Arrow = HELD_UP;
				}
				// Check for click on down button
			}
			else if (PtInRect(m_objResource->GetDrawingRects(2), pt)){
				if (IsRepeat && (m_Arrow == HELD_DOWN || m_Arrow == CLICKED_DOWN))
				{
					newMsg.message = EM_SB_STEPDOWN;
					MsgProc(&newMsg);
					if (m_Arrow == CLICKED_DOWN) m_Arrow = HELD_DOWN;
				}
				// Check for dragging thumb
			}
			else if (PtInRect(m_objResource->GetDrawingRects(3), pt)){
				//do nothing
				// Check for click on track
			}
			else if (IsRepeat && (m_Arrow == CLICKED || m_Arrow == HELD)) {
				if (m_ScrollType == HSCROLL) {
					if (m_objResource->GetDrawingRects(3).top <= pt.y &&
						m_objResource->GetDrawingRects(3).bottom > pt.y)
					{
						if (m_objResource->GetDrawingRects(3).left > pt.x)
						{
							newMsg.message = EM_SB_PAGEUP;
						}
						else if (m_objResource->GetDrawingRects(3).right <= pt.x)
						{
							newMsg.message = EM_SB_PAGEDOWN;
						}
						MsgProc(&newMsg);
						if (m_Arrow == CLICKED) m_Arrow = HELD;

					}
				}
				else{
					if (m_objResource->GetDrawingRects(3).left <= pt.x &&
						m_objResource->GetDrawingRects(3).right > pt.x)
					{
						if (m_objResource->GetDrawingRects(3).top > pt.y)
						{
							newMsg.message = EM_SB_PAGEUP;
						}
						else if (m_objResource->GetDrawingRects(3).bottom <= pt.y)
						{
							newMsg.message = EM_SB_PAGEDOWN;
						}
						MsgProc(&newMsg);
						if (m_Arrow == CLICKED) m_Arrow = HELD;
					}

				}
			}

		}
		if (m_event->IsMapTo(nEvent, EM_SB_ACTIONBEGIN) && m_Arrow == CLEAR){
			//((L_PRESSED(pMouse))&&(m_Arrow!=CLEAR))) {
			bHandled = true;
			newMsg.message = EM_CTRL_CAPTUREMOUSE;
			CGUIBase::MsgProc(&newMsg);
			//click on up button
			if (PtInRect(m_objResource->GetDrawingRects(1), pt))
			{
				newMsg.message = EM_SB_STEPUP;
				MsgProc(&newMsg);
				if (m_Arrow == CLEAR) m_Arrow = CLICKED_UP;
				m_dArrowTS = dCurrTime;
				// Check for click on down button
			}
			else if (PtInRect(m_objResource->GetDrawingRects(2), pt)){
				newMsg.message = EM_SB_STEPDOWN;
				MsgProc(&newMsg);
				if (m_Arrow == CLEAR) m_Arrow = CLICKED_DOWN;
				m_dArrowTS = dCurrTime;
				// Check for click on thumb
			}
			else if (PtInRect(m_objResource->GetDrawingRects(3), pt)){
				bDrag = true;
				if (m_ScrollType == HSCROLL)
					ThumbOffsetX = pt.x - m_objResource->GetDrawingRects(3).left;
				else
					ThumbOffsetY = pt.y - m_objResource->GetDrawingRects(3).top;
				// Check for click on track
				CaptureMouse();
			}
			else {
				if (m_ScrollType == HSCROLL) {

					if (m_objResource->GetDrawingRects(3).top <= pt.y &&
						m_objResource->GetDrawingRects(3).bottom > pt.y)
					{
						if (m_objResource->GetDrawingRects(3).left > pt.x)
						{
							newMsg.message = EM_SB_PAGEUP;
						}
						else
							if (m_objResource->GetDrawingRects(3).right <= pt.x)
							{
								newMsg.message = EM_SB_PAGEDOWN;
							}
						MsgProc(&newMsg);
						if (m_Arrow == CLEAR) m_Arrow = CLICKED;
						m_dArrowTS = dCurrTime;

					}
				}
				else{
					if (m_objResource->GetDrawingRects(3).left <= pt.x &&
						m_objResource->GetDrawingRects(3).right > pt.x)
					{
						if (m_objResource->GetDrawingRects(3).top > pt.y)
						{
							newMsg.message = EM_SB_PAGEUP;
						}
						else
							if (m_objResource->GetDrawingRects(3).bottom <= pt.y)
							{
								newMsg.message = EM_SB_PAGEDOWN;
							}
						MsgProc(&newMsg);
						if (m_Arrow == CLEAR) m_Arrow = CLICKED;

						m_dArrowTS = dCurrTime;
					}

				}
			}
		}

	}
	if (!bHandled)
	{
		MSG msg = m_event->GenerateMessage();
		return CGUIBase::MsgProc(&msg);
	}

	return bHandled;
}

HRESULT CGUIScrollBar::Render(GUIState* pGUIState, float fElapsedTime)
{
	PERF_BEGIN("GUI Scrollbar Render");
	SetDirty(false);
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();
	if (m_bIsVisible == false)
		return S_OK;
	else if (m_bIsEnabled == false || m_bShowThumb == false)
		m_objResource->SetCurrentState(GUIResourceState_Disabled);
	else if (m_bMouseOver | m_bHasFocus)
		m_objResource->SetCurrentState(GUIResourceState_Highlight);

	if (m_position.IsRelativeTo3DObject())
	{
		BeginDepthTest(pGUIState);
		UpdateRects();
	}

	RECT rcScreen;//rcScreen is the rect of this container
	//render background first
	if (GetParent())
	{
		rcScreen = GetParent()->m_objResource->GetDrawingRects(0);
	}
	else
	{
		rcScreen = m_objResource->GetDrawingRects(0);
	}

	if (m_ScrollType == HSCROLL)
	{
		// TODO: needs to rotate 90 degrees. it used to be -1: 
		// m_objResource->DrawSprite( pElement, &m_objResource->GetDrawingRects(0),-1,m_position.GetDepth() );
		// basic, Up Arrow, down Arrow, thumb button
		for (int i = 0; i<4; ++i)
		{
			RECT rc = m_objResource->GetDrawingRects(i);
			DrawElement(m_objResource->GetTextureElement(i), &rc, &rcScreen);
		}
	}
	else
	{
		// basic, Up Arrow, down Arrow, thumb button
		for (int i = 0; i<3; ++i)
		{
			RECT rc = m_objResource->GetDrawingRects(i);
			DrawElement(m_objResource->GetTextureElement(i), &rc, &rcScreen);
		}
		if (m_bFixedThumb)
		{
			RECT rc = m_objResource->GetDrawingRects(3);
			DrawElement(m_objResource->GetTextureElement(3), &rc, &rcScreen);
		}
		else
		{
			GUITextureElement* pElement = NULL;

			GUIResourceState StateRes = m_objResource->GetCurrentState();
			// tricky: LiXizhi 2007.9.27: we will only use the normal state texture, but with the color of the other state's index 0 texture
			LinearColor ColorRes(0xffffffff);
			if (pElement != NULL)
				ColorRes = pElement->TextureColor;
			m_objResource->SetCurrentState(GUIResourceState_Normal);

			int nOffset = 4;
			// using 9 element for the "thumb" button
			for (int e = 0; e < 9; ++e)
			{
				RECT rc = m_objResource->GetDrawingRects(e + nOffset);
				DrawElement(m_objResource->GetTextureElement(e + nOffset), &rc, &rcScreen);
			}
			m_objResource->SetCurrentState(StateRes);
		}
	}
	if (m_position.IsRelativeTo3DObject())
	{
		EndDepthTest(pGUIState);
	}
	PERF_END("GUI Scrollbar Render");

	return S_OK;
}


//--------------------------------------------------------------------------------------
void CGUIScrollBar::SetTrackRange(int nStart, int nEnd)
{
	m_nStart = nStart; m_nEnd = nEnd;
	Cap();
	UpdateThumbRect();
}


//--------------------------------------------------------------------------------------
void CGUIScrollBar::Cap()  // Clips position at boundaries. Ensures it stays within legal range.
{
	if (m_nPosition < m_nStart ||
		(m_nEnd - m_nStart) <= m_nPageSize)
	{
		m_nPosition = m_nStart;
	}
	else
		if (m_nPosition + m_nPageSize > m_nEnd)
			m_nPosition = m_nEnd - m_nPageSize;

	//if( m_nPosition < m_nStart)
	//	m_nPosition = m_nStart;
	//else if(m_nPosition>m_nEnd)
	//	m_nPosition = m_nEnd;
}

void CGUIScrollBar::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm = &CSingleton<CObjectManager>::Instance();
	if (!m_event){
		if (!pOm->IsExist("default_CGUIScrollbar")) {
			CGUIScrollBar::StaticInit();
		}
		pOm->CloneObject("default_CGUIScrollbar", this);
	}
	CGUIBase::InitObject(strObjectName, alignment, x, y, width, height);
}

bool CGUIScrollBar::OnMouseLeave()
{
	m_Arrow = CLEAR;
	return CGUIBase::OnMouseLeave();
}

void ParaEngine::CGUIScrollBar::SetFixedThumb(bool bFixed)
{
	if (m_bFixedThumb^bFixed)
		m_bNeedUpdate = true; 
	m_bFixedThumb = bFixed;
	SetDirty(true);
}
