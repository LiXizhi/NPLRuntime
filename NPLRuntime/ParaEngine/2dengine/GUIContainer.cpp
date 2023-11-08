//----------------------------------------------------------------------
// Class:	CGUIContainer
// Authors:	Liu Weili, LiXizhi
// Date:	2005.8.1
// Revised: 2005.8.1
//
// desc: 
// There are some controls that can have children. All these controls should inherited from CGUIContainer.
// CGUIContainer provide basic function and properties for a container should have. 
// Such as get/lost focus and mouse in/out controls. 
// CGUIContainer objects provide a logical boundary for contained controls. 
// The container control can capture the TAB key press and move focus to the next 
// control in the collection.
// Note: The container control also receives focus; But in most cases it does not do anything and passes
// the focus events to its children which has focus. 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include <queue>
#include <algorithm>
#include "IParaEngineApp.h"
#include "GUIRoot.h"
#include "GUIResource.h"
#include "ParaWorldAsset.h"
#include "GUIEvent.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "InfoCenter/ICConfigManager.h"
#include "util/StringHelper.h"
#include "Type.h"
#include "GUIButton.h"
#include "GUIEdit.h"
#include "GUIScrollBar.h"
#include "RenderTarget.h"
#include "PaintEngine/Painter.h"
#include "SceneObject.h"
#include "GUIContainer.h"
#include "memdebug.h"

using namespace ParaEngine;

/** default container border size*/
#define DEFAULT_CONTAINER_BORDER_SIZE	0
/** default container  margin size*/
#define DEFAULT_CONTAINER_MARGIN_SIZE	0
/** The maximum level of the GUI hierarchy. Root is level 1. Root.m_children is level 2 and so on.*/
#define MAX_GUI_HIERARCHY_LEVEL 20

const IType* CGUIContainer::m_type = NULL;

CGUIContainer::CGUIContainer() :CGUIBase(),
m_bScrollable(false),
m_pKeyFocus(NULL),
m_pMouseFocus(NULL),
m_ScrollType(0),
m_nBorder(DEFAULT_CONTAINER_BORDER_SIZE),
m_nMargin(DEFAULT_CONTAINER_MARGIN_SIZE),
m_nSBWidth(16),
m_bFastRender(true),
m_VScroll(NULL),
m_HScroll(NULL),
m_nPopupStyle(Popup_None),
m_bIsTop(false),
m_bBatching(false),
m_bEnableNonClientTest(false),
m_bNeedCalClientRect(false)
{
	// Fixed.2010.10.27: the container will not capture mouse, so it will leak messages to 3D
	m_bCanCaptureMouse = false;

	if (!m_type){
		m_type = IType::GetType("guicontainer");
	}

	ZeroMemory(&m_clientRect, sizeof(m_clientRect));
}

CGUIContainer::~CGUIContainer()
{
	SAFE_RELEASE(m_VScroll);
	SAFE_RELEASE(m_HScroll);
	if (m_bIsTop){
		SetTopLevel(false);
	}
	if (m_renderTarget)
	{
		// set dead immediately to be destroyed in the next frame move. 
		((CRenderTarget*)m_renderTarget.get())->SetLifeTime(0);
	}
}
/**
 * This is not a complete clone. The m_children, m_pKeyFocus, m_pMouseFocus, m_oldRect are not cloned.
 **/
void CGUIContainer::Clone(IObject* pobj)const
{
	if (pobj == NULL) {
		return;
	}
	CGUIContainer* pContainer = (CGUIContainer*)pobj;
	CGUIBase::Clone(pContainer);
	pContainer->m_bScrollable = m_bScrollable;
	pContainer->m_children.clear();
	pContainer->m_ScrollType = m_ScrollType;
	pContainer->m_nBorder = m_nBorder;
	pContainer->m_nMargin = m_nMargin;
	pContainer->m_nSBWidth = m_nSBWidth;
	pContainer->m_bNeedUpdate = m_bNeedUpdate;
	pContainer->m_bFastRender = m_bFastRender;
	ZeroMemory(&pContainer->m_clientRect, sizeof(m_clientRect));

	if (m_VScroll){
		pContainer->m_VScroll = (CGUIScrollBar*)m_VScroll->Clone();
		pContainer->m_VScroll->SetLinkedObject(pContainer);
		pContainer->m_VScroll->m_parent = pContainer;
		pContainer->m_VScroll->m_sIdentifer = m_VScroll->m_sIdentifer;
	}
	if (m_HScroll){
		pContainer->m_HScroll = (CGUIScrollBar*)m_HScroll->Clone();
		pContainer->m_HScroll->SetLinkedObject(pContainer);
		pContainer->m_HScroll->m_parent = pContainer;
		pContainer->m_HScroll->m_sIdentifer = m_HScroll->m_sIdentifer;
	}
    
}

IObject* CGUIContainer::Clone()const
{
	CGUIContainer* pContainter = new CGUIContainer();
	Clone(pContainter);
	return pContainter;
}

CGUIScrollBar*	CGUIContainer::GetScrollBar(int nVerticalHorizontal)
{
	return nVerticalHorizontal == 0 ? m_VScroll : m_HScroll;
}

void CGUIContainer::StaticInit()
{
	CObjectManager *pOm = &CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIBase")) {
		CGUIBase::StaticInit();
	}
	//load the default CGUIBase object and copy all its value to the new button
	CGUIContainer *pContainer = new CGUIContainer();
	pContainer->m_bFastRender = true;
	pContainer->m_bScrollable = false;
	pContainer->m_nMargin = DEFAULT_CONTAINER_MARGIN_SIZE;
	pContainer->m_nSBWidth = 16;
	pContainer->m_nBorder = DEFAULT_CONTAINER_BORDER_SIZE;
	pOm->CloneObject("default_CGUIBase", pContainer);
	pContainer->m_bCanCaptureMouse = false;
	// 	pContainer->m_objType=CGUIBase::GUIContainer;
	CEventBinding* pBinding = pContainer->m_event->GetEventBindingObj();
	//pBinding->DisableKeyboard();
	pBinding->EnableKeyboard();
	pBinding->EnableMouse();
	//in button, i define that a click on the button will get the button key focus
	//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_CTRL_CAPTUREMOUSE);
	//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_CTRL_RELEASEMOUSE);

	pBinding->DisableEvent(EM_MOUSE_DBCLICK);
	pBinding->DisableEvent(EM_MOUSE_CLICK);
	//	pBinding->EnableEvent(EM_KEY);
	//	pBinding->EnableEvent(EM_KEY_PAGE_DOWN);
	//	pBinding->EnableEvent(EM_KEY_PAGE_UP);
	//	pContainer->m_event->SetEventBinding(pBinding);


	if (!pOm->IsExist("default_CGUIScrollbar")) {
		CGUIScrollBar::StaticInit();
	}

	pContainer->m_VScroll = (CGUIScrollBar*)pOm->CloneObject("default_CGUIScrollbar");
	pContainer->m_VScroll->SetName("vscrollbar");
	pContainer->m_HScroll = (CGUIScrollBar*)pOm->CloneObject("default_CGUIScrollbar");
	pContainer->m_HScroll->SetName("hscrollbar");

	// TODO: 2006.8.LXZ: when GUI object is reference counted, the default container should reference the default scroll bar, so that when 
	// the style of default scroll bar is changed, so does the container scroll bar. Right now, we clone the scroll bar. 
	//pContainer->m_VScroll = (CGUIScrollBar*)pOm->GetObject("default_CGUIScrollbar");
	//pContainer->m_HScroll = (CGUIScrollBar*)pContainer->m_VScroll;

	//TODO: we could add default texture or text here, load from config
	using namespace ParaInfoCenter;
	CICConfigManager *cm = CGlobals::GetICConfigManager();
	string value0, value1;
	int event0, event1, a;
	DWORD b;
	HRESULT hr;
	hr = cm->GetSize("GUI_container_control_mapping", &b);
	if (hr == E_INVALIDARG || hr == E_ACCESSDENIED) {
		//error
	}
	else{
		for (a = 0; a < (int)b; a += 2) {
			hr = cm->GetTextValue("GUI_container_control_mapping", value0, a);
			if (FAILED(hr)) {
				break;
			}
			hr = cm->GetTextValue("GUI_container_control_mapping", value1, a + 1);
			if (FAILED(hr)) {
				break;
			}
			event0 = CEventBinding::StringToEventValue(value0);
			event1 = CEventBinding::StringToEventValue(value1);
			pBinding->MapEvent(event0, event1);
		}
	}
	int tempint;;
	if (cm->GetIntValue("GUI_container_control_fastrender", &tempint) == S_OK) {
		if (tempint == 0) {
			pContainer->m_bFastRender = false;
		}
		else
			pContainer->m_bFastRender = true;
	}
	if (cm->GetIntValue("GUI_container_control_canhasfocus", &tempint) == S_OK) {
		if (tempint == 0) {
			pContainer->m_bCanHasFocus = false;
		}
		else
			pContainer->m_bCanHasFocus = true;
	}
	if (cm->GetIntValue("GUI_container_control_visible", &tempint) == S_OK) {
		if (tempint == 0) {
			pContainer->m_bIsVisible = false;
		}
		else
			pContainer->m_bIsVisible = true;
	}
	if (cm->GetIntValue("GUI_container_control_enable", &tempint) == S_OK) {
		if (tempint == 0) {
			pContainer->m_bIsEnabled = false;
		}
		else
			pContainer->m_bIsEnabled = true;
	}
	if (cm->GetIntValue("GUI_container_control_candrag", &tempint) == S_OK) {
		if (tempint == 0) {
			pContainer->SetCandrag(false);
		}
		else
			pContainer->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_container_control_scrollable", &tempint) == S_OK) {
		if (tempint == 0) {
			pContainer->m_bScrollable = false;
		}
		else
			pContainer->m_bScrollable = true;
	}
	if (cm->GetIntValue("GUI_container_control_lifetime", &tempint) == S_OK) {
		pContainer->m_nLifeTimeCountDown = tempint;
	}
	if (cm->GetIntValue("GUI_container_control_scrollbarwidth", &tempint) == S_OK) {
		pContainer->m_nSBWidth = tempint;
	}
	if (cm->GetIntValue("GUI_container_control_margin", &tempint) == S_OK) {
		pContainer->m_nMargin = tempint;
	}
	if (cm->GetIntValue("GUI_container_control_borderwidth", &tempint) == S_OK) {
		pContainer->m_nBorder = tempint;
	}



	//Set the default texture and font of the default button
	pContainer->m_objResource->SetActiveLayer();
	pContainer->m_objResource->SetCurrentState();
	pContainer->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
	RECT *prect = NULL;
	RECT rect;
	SpriteFontEntity* pFont;
	TextureEntity* pTexture = NULL;
	pFont = CGlobals::GetAssetManager()->GetFont("sys");
	GUIFontElement font_;
	GUIFontElement* pFontElement = &font_;
	GUITextureElement tex_;
	GUITextureElement* pElement = &tex_;
	pFontElement->SetElement(pFont, COLOR_ARGB(255, 0, 0, 0), DT_LEFT | DT_TOP | DT_WORDBREAK);
	pContainer->m_objResource->AddElement(pFontElement, "text");
	string background, filename;
	bool bLoadDefault = true;
	if (cm->GetTextValue("GUI_container_control_background", background) == S_OK) {
		prect = ParaEngine::StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 13, 124, 241, 265);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pContainer->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pContainer->m_objResource->AddElement(pElement, "background");
	pContainer->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pContainer->m_objResource->AddElement(pElement, "background");
	pContainer->m_objResource->SetCurrentState(GUIResourceState_Highlight);
	pContainer->m_objResource->AddElement(pElement, "background");
	pContainer->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pContainer->m_objResource->AddElement(pElement, "background");
	pContainer->m_objResource->SetCurrentState();
	pContainer->m_objResource->SetActiveLayer();

	pOm->SetObject("default_CGUIContainer", pContainer);
	SAFE_RELEASE(pContainer);
}
string CGUIContainer::ToScript(int option)
{
	string script = CGUIBase::ToScript(option);
	//add "__this:SetScrollable(true);"like script
	script += "__this.scrollable=";
	if (m_bScrollable) script += "true";
	else script += "false";
	script += ";\n";
	return script;
}

void CGUIContainer::DestroyChildren()
{
	// delete children
	GUIBase_List_Type::iterator itCurCP, itEndCP = m_children.end();

	for (itCurCP = m_children.begin(); itCurCP != itEndCP; ++itCurCP)
	{
		CGUIBase* temp = (*itCurCP);
		if (((CGUIType*)(*itCurCP)->GetType())->IsContainer()) {
			((CGUIContainer*)(*itCurCP))->DestroyChildren();
		}
		CGUIRoot::DeleteGUIObject(*itCurCP);
	}
	m_children.clear();

	// reset client area
	ZeroMemory(&m_clientRect, sizeof(m_clientRect));
	// remove and reset scroll bar
	SetScrollType(0);
	ResetScrollbar();

	m_bNeedUpdate = true;
	m_bNeedCalClientRect = true;
	SetDirty(true);
}

CGUIBase* CGUIContainer::GetChildByID(int nChildID)
{
	GUIBase_List_Type::iterator itCurCP, itEndCP = m_children.end();

	for (itCurCP = m_children.begin(); itCurCP != itEndCP; ++itCurCP)
	{
		if ((*itCurCP)->GetID() == nChildID)
			return (*itCurCP);
	}
	return NULL;
}

CGUIBase* CGUIContainer::GetChildByName(const string& name)
{
	GUIBase_List_Type::iterator itCurCP, itEndCP = m_children.end();

	for (itCurCP = m_children.begin(); itCurCP != itEndCP; ++itCurCP)
	{
		if ((*itCurCP)->GetName() == name)
			return (*itCurCP);
	}
	return NULL;
}

void CGUIContainer::AddChild(CGUIBase* pChild, int nIndex)
{
	if (pChild == 0)
		return;
	SetDirty(true);
	if (nIndex < 0)
	{
		// append by z-order, from small to larger (ascending order)
		bool bAttanched = false;
		GUIBase_List_Type::reverse_iterator itCurCP, itEndCP = m_children.rend();
		for (itCurCP = m_children.rbegin(); itCurCP != itEndCP; ++itCurCP)
		{
			if ((*itCurCP)->GetZOrder() <= pChild->GetZOrder())
			{
				m_children.insert(itCurCP.base(), pChild);
				bAttanched = true;
				break;
			}
		}
		if (!bAttanched)
		{
			m_children.push_front(pChild);
		}

		//m_children.push_back(pChild);
	}
	else
	{
		GUIBase_List_Type::iterator itCurCP, itEndCP = m_children.end();
		int i = 0;
		for (itCurCP = m_children.begin(); itCurCP != itEndCP; ++itCurCP, ++i)
		{
			if (i == nIndex)
			{
				m_children.insert(itCurCP, pChild);
				break;
			}
		}
	}
}

void CGUIContainer::ResetScrollbar()
{
	if (m_HScroll)
		m_HScroll->SetTrackPos(0);
	if (m_VScroll)
		m_VScroll->SetTrackPos(0);
}
/**
We want to set the m_pKeyFocus property of all its parents.
For example, the control that gets focus is A, its parent is B, root is R
The control that lost focus is D, its parent is C. The current
focus is B.m_pKeyFocus=NULL, R.m_pKeyFocus=C, C.m_pKeyFocus=D
See the follow hierarchy tree.
R
/ \
B   C
/     \
A       D
Case 1:
Our desirable result is B.m_pKeyFocus=A, R.m_pKeyFocus=B, C.m_pKeyFocus=NULL
We are calling B.SetKeyFocus(A).
The following is the steps:
1. We first follows the B.m_parent link of A upwards until we comes to a control
whose m_pKeyFocus!=NULL. We call it the Common Parent(CP). In the above case
it is R.
2. Then we follow the CP.m_pKeyFocus link downwards until reach D.
Follow the D.m_parent upwards and sets all the m_pKeyFocus
in that link to NULL and trigger the "OnFocusOut" event of each
controls in that link until we get to CP.
3. We follows the A.m_parent link upwards and sets the the m_pKeyFocus in that
link and trigger "OnFocusIn" event in reverse sequence for each node until we get to CP.

Case 2:
On another case, when we want to set R.m_pKeyFocus=C, C.m_pKeyFocus=NULL;
1. We know the C's parent's m_pKeyFocus=C. We set the Common Parent(CP) to C;
2. Do the same as case 1;
3. The same as case 1 but only do it for C;
*/
void CGUIContainer::SetKeyFocus(CGUIBase* control)
{
	CGUIContainer* temp = this, *cp;
	CGUIContainer* route[MAX_GUI_HIERARCHY_LEVEL];
	int routecount = 0;
	if (!control) {
		cp = this;
	}
	else if (this->m_pKeyFocus == control){//if it's the same control clean all this descendant focus
		if (((CGUIType*)control->GetType())->IsContainer()) {
			route[routecount++] = (CGUIContainer*)control;
			route[routecount] = temp;
			cp = (CGUIContainer*)control;
		}
		else
			return;
	}
	else{
		//step 1
		//route[0]=control,route[1]=this....route[routecount]=cp;
		route[routecount++] = (CGUIContainer*)control;
		while (temp->m_pKeyFocus == NULL&&temp->m_parent != NULL) {
			route[routecount++] = temp;
			temp = (CGUIContainer*)temp->m_parent;
		}
		route[routecount] = temp;
		cp = temp;
	}

	//step 2, do if cp->m_pKeyFocus!=NULL
	if (cp->m_pKeyFocus != NULL) {
		temp = (CGUIContainer*)cp->m_pKeyFocus;
		while (((CGUIType*)temp->GetType())->IsContainer() && temp->m_pKeyFocus != NULL) {
			temp = (CGUIContainer*)temp->m_pKeyFocus;
		}
		while (temp != cp) {
			temp->OnFocusOut();

			temp->m_bHasFocus = false;
			temp->ClearEvent(CGUIEvent::KEYBOARD);
			if (((CGUIType*)temp->GetType())->IsContainer()) {
				temp->m_pKeyFocus = NULL;
			}
			temp = temp->m_parent;
		}
		temp->m_pKeyFocus = NULL;
	}
	//step 3
	temp = this;
	while (routecount > 0) {
		route[routecount]->m_pKeyFocus = route[routecount - 1];
		route[routecount - 1]->OnFocusIn();
		route[routecount - 1]->m_bHasFocus = true;
		routecount--;
	}
	if (control) {
		if (((CGUIType*)control->GetType())->IsContainer()) {
			((CGUIContainer*)control)->m_pKeyFocus = NULL;
		}
		control->m_bHasFocus = true;
	}
}

//the process of SetMouseIn is the same as SetKeyFocus, except that it activates 
//OnMouseEnter and OnMouseLeave event and set m_pMouseFocus

void CGUIContainer::SetMouseFocus(CGUIBase* control)
{
	m_pMouseFocus = control;
	return;

	CGUIMouseVirtual *pMouse = CGUIRoot::GetInstance()->m_pMouse;
	CGUIContainer* temp = this, *cp;
	CGUIContainer* route[MAX_GUI_HIERARCHY_LEVEL];
	STRUCT_DRAG_AND_DROP *pdrag = &IObjectDrag::DraggingObject;
	int routecount = 0;
	if (!control) {
		cp = this;
	}
	else if (this->m_pMouseFocus == control){//if it's the same control clean all this descendant focus
		if (((CGUIType*)control->GetType())->IsContainer()) {
			route[routecount++] = (CGUIContainer*)control;
			route[routecount] = temp;
			cp = (CGUIContainer*)control;
		}
		else
			return;
	}
	else{
		//step 1
		//route[0]=control,route[1]=this....route[routecount]=cp;
		route[routecount++] = (CGUIContainer*)control;
		while (temp->m_pMouseFocus == NULL&&temp->m_parent != NULL) {
			route[routecount++] = temp;
			temp = (CGUIContainer*)temp->m_parent;
		}
		route[routecount] = temp;
		cp = temp;
	}

	//step 2, do if temp->m_pMouseFocus!=NULL
	if (cp->m_pMouseFocus != NULL) {
		temp = (CGUIContainer*)cp->m_pMouseFocus;
		while (((CGUIType*)temp->GetType())->IsContainer() && temp->m_pMouseFocus != NULL) {
			temp = (CGUIContainer*)temp->m_pMouseFocus;
		}
		while (temp != cp)
		{
			if (pdrag->pSource != NULL) {
				temp->OnMouseLeave();
			}
			bool bIsParent = false;
			if (((CGUIType*)temp->GetType())->IsContainer())
			{
				if (control)
				{
					bIsParent = control->HasParent(temp);
				}
				temp->m_pMouseFocus = NULL;
			}
			if (!bIsParent)
				temp->OnMouseLeave();
			CGUIRoot::GetInstance()->m_tooltip->DeactivateTip(temp);
			CGUIRoot::GetInstance()->m_pMouse->ReleaseCapture(temp);
			temp->m_bMouseOver = false;
			temp->ClearEvent(CGUIEvent::MOUSE);

			temp = temp->m_parent;
		}
		temp->m_pMouseFocus = NULL;
	}
	//step 3
	temp = this;
	while (routecount > 0) {
		route[routecount]->m_pMouseFocus = route[routecount - 1];
		route[routecount - 1]->OnMouseEnter();
		route[routecount - 1]->m_bMouseOver = true;
		if (route[routecount - 1]->m_event)
		{
			if ((pMouse->m_dims2.buttons[0]) == EKeyState::PRESS) {
				//this may be wrong
				route[routecount - 1]->m_event->m_mouse.LastLDown.init();
			}
			if ((pMouse->m_dims2.buttons[1]) == EKeyState::PRESS) {
				//this may be wrong
				route[routecount - 1]->m_event->m_mouse.LastRDown.init();
			}
			if ((pMouse->m_dims2.buttons[2]) == EKeyState::PRESS) {
				//this may be wrong
				route[routecount - 1]->m_event->m_mouse.LastMDown.init();
			}
			if (pdrag->pSource != NULL) {
				route[routecount - 1]->OnMouseEnter();
			}

		}
		routecount--;
	}
	if (control) {
		if (((CGUIType*)control->GetType())->IsContainer()) {
			((CGUIContainer*)control)->m_pMouseFocus = NULL;
		}
		control->m_bMouseOver = true;
		CGUIRoot::GetInstance()->m_tooltip->ActivateTip(control);
	}

}

void CGUIContainer::SetScrollType(int etype)
{
	m_ScrollType = etype;
	m_bNeedUpdate = true;
	SetDirty(true);
}

void CGUIContainer::BringToFront(CGUIBase* obj)
{
	// if(m_children.size() > 1 && (*(m_children.rbegin())) != obj)
	{
		GUIBase_List_Type::iterator iter, iterend = m_children.end();
		for (iter = m_children.begin(); iter != iterend; iter++){
			if ((*iter) == obj){
				m_children.erase(iter);
				m_children.insert(m_children.end(), obj);
				SortChildrenByZOrder();
				break;
			}
		}
	}
}

void CGUIContainer::SendToBack(CGUIBase* obj)
{
	// if(m_children.size() > 1 && (*(m_children.begin())) != obj)
	{
		GUIBase_List_Type::iterator iter, iterend = m_children.end();
		for (iter = m_children.begin(); iter != iterend; iter++){
			if ((*iter) == obj){
				m_children.erase(iter);
				m_children.insert(m_children.begin(), obj);
				SortChildrenByZOrder();
				break;
			}
		}
	}
}

void CGUIContainer::SetTopLevel(bool value)
{
	if (!m_bIsVisible || m_bCandrag){
		return;
	}
	if (!m_parent || m_parent->GetType()->GetTypeValue() == Type_GUIRoot){
		if (m_bIsTop){
			if (!value){
				m_bIsTop = false;
				CGlobals::GetGUI()->RemoveTopLevelControl(this);
			}

		}
		else{
			if (value){
				m_bIsTop = true;
				CGlobals::GetGUI()->PushTopLevelControl(this);
			}
		}
	}
}

void CGUIContainer::SetPopUp(int popup)
{
	if (!m_parent){
		OUTPUT_LOG("Must Attach the object to another object or Root before setting popup property\n");
		return;
	}
	if (popup != Popup_None&&m_nPopupStyle == Popup_None){
		m_bCandrag = false;
		Focus();
	}
	m_nPopupStyle = popup;
}

bool CGUIContainer::OnFocusOut()
{
	switch (m_nPopupStyle){
	case Popup_Autodelete:
		CGUIRoot::PostDeleteGUIObject(this);
		break;
	case Popup_None:
		break;
	case Popup_Autohide:
		m_bIsVisible = false;
		break;
	}
	return CGUIBase::OnFocusOut();
}
/**
 * First, send the message to the scrollbar.
 * If scrollbar does not handle the message, let the container handle it.
 * Container only handle EM_MOUSE_WHEEL, other events are sent further to CGUIBase::MsgProc
 **/
bool CGUIContainer::MsgProc(MSG *event)
{
	//one call to this function will trigger at most one event
	if (!m_bIsEnabled || m_event == 0)
		return false;
	bool bHandled = false;
	CGUIMouseVirtual *pMouse = CGUIRoot::GetInstance()->m_pMouse;
	CGUIKeyboardVirtual *pKeyboard = CGUIRoot::GetInstance()->m_pKeyboard;
	if (m_bScrollable&&m_bIsVisible) {
		CGUIPosition pos;
		if (m_ScrollType&VSCROLL) {
			if (m_event->IsMapTo(event->message, EM_MOUSE)) {
				m_VScroll->GetAbsolutePosition(&pos, m_VScroll->GetPosition());

				if (pos.rect.left <= event->pt.x && pos.rect.top <= event->pt.y &&
					pos.rect.right >= event->pt.x && pos.rect.bottom >= event->pt.y&&
					!(m_event->IsMapTo(event->message, EM_MOUSE_DRAGEND) || m_event->IsMapTo(event->message, EM_MOUSE_DRAGOVER)))
				{
					/// object should at least has a life count greater than 1, and isEnabled
					if ((m_VScroll->GetLifeTime() == -1) || ((m_VScroll->GetLifeTime() > 1) && m_VScroll->m_bIsVisible)) {

						bHandled |= m_VScroll->MsgProc(event);
					}
				}
			}
		}
		if (m_ScrollType&HSCROLL) {
			if (m_event->IsMapTo(event->message, EM_MOUSE)) {
				m_HScroll->GetAbsolutePosition(&pos, m_HScroll->GetPosition());

				if (pos.rect.left <= event->pt.x && pos.rect.top <= event->pt.y &&
					pos.rect.right >= event->pt.x && pos.rect.bottom >= event->pt.y&&
					!(m_event->IsMapTo(event->message, EM_MOUSE_DRAGEND) || m_event->IsMapTo(event->message, EM_MOUSE_DRAGOVER)))
				{
					/// object should at least has a life count greater than 1, and isEnabled
					if ((m_HScroll->GetLifeTime() == -1) || ((m_HScroll->GetLifeTime() > 1) && m_HScroll->m_bIsVisible)) {
						bHandled |= m_HScroll->MsgProc(event);
					}
				}
			}
		}
		if (bHandled) {
			m_event->ResetState();
			return true;
		}
	}
	if (event != NULL&&!m_event->InterpretMessage(event)) {
		return false;
	}
	POINT pt;
	pt.x = m_event->m_mouse.x;
	pt.y = m_event->m_mouse.y;
	int nEvent = m_event->GetTriggerEvent();
	if (m_bIsVisible){
		if (nEvent == EM_NONE) {
			return false;
		}
		if (m_event->IsMapTo(nEvent, EM_MOUSE_WHEEL)) {
			if (m_bScrollable && (m_ScrollType&VSCROLL)) {
				MSG msg = m_event->GenerateMessage();
				bHandled |= m_VScroll->MsgProc(&msg);
			}
			// bHandled=true; let container receiver handle the message
		}
		else if (m_event->IsMapTo(nEvent, EM_SB_SCROLL)){
			GUIBase_List_Type::iterator iter;
			OffsetRect(&m_clientRect, (int)event->lParam, (int)event->wParam);
			//scrolls all its children, updating their position and drawing rectangles
			if (event->wParam != 0) {
				for (iter = m_children.begin(); iter != m_children.end(); iter++) {
					(*iter)->SetLocationI((*iter)->GetPosition()->rect.left, (*iter)->GetPosition()->rect.top + event->wParam);
				}
			}
			if (event->lParam != 0) {
				for (iter = m_children.begin(); iter != m_children.end(); iter++) {
					(*iter)->SetLocationI((*iter)->GetPosition()->rect.left + event->lParam, (*iter)->GetPosition()->rect.top);
				}
			}
		}
		else if (m_event->IsMapTo(nEvent, EM_KEY))
		{
			if (IS_KEYDOWN(event->lParam))
			{
				bHandled = OnKeyDown();
			}
		}
	}
	if (!bHandled)
	{
		MSG msg = m_event->GenerateMessage();
		return CGUIBase::MsgProc(&msg);
	}
	else
		return true;

}

void CGUIContainer::Begin(GUIState* pGUIState, float fElapsedTime)
{
	if (m_bNeedUpdate&&!m_bBatching) {
		UpdateRects();
	}
	if (m_bScrollable) {
		//draw scroll bars
		if (m_ScrollType&VSCROLL) {
			m_VScroll->DoRender(pGUIState, fElapsedTime);
		}
		if (m_ScrollType&HSCROLL) {
			m_HScroll->DoRender(pGUIState, fElapsedTime);
		}
	}
	CPainter* painter = GetPainter(pGUIState);

	if (!m_bFastRender)
	{
		RECT clipRect = GetClippingRect(pGUIState);
		// the old GUI system never restore sprite transform, and always assume absolute screen coordinates when drawing. 
		// hence, we will need to set identity here. 
		painter->SetSpriteTransform(&Matrix4::IDENTITY);
		painter->setClipRect(QRect(clipRect), ReplaceClip);
	}
	CGUIBase::Begin(pGUIState, fElapsedTime);

	if (GetUsePointTextureFiltering())
	{
#ifdef USE_DIRECTX_RENDERER
		CGlobals::GetEffectManager()->SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
		CGlobals::GetEffectManager()->SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
#endif
	}
}

void CGUIContainer::End(GUIState* pGUIState, float fElapsedTime)
{
	CPainter* painter = GetPainter(pGUIState);
	if (!m_bFastRender)
	{
		painter->setClipRect(QRect(), NoClip);
	}

	// no need to flush now, since D3DXSPRITE_SORT_TEXTURE is not specified, we will only merge texture when they are the same.
	// painter->Flush();

	CGUIBase::End(pGUIState, fElapsedTime);

	if (GetUsePointTextureFiltering())
	{
#ifdef USE_DIRECTX_RENDERER
		CGlobals::GetEffectManager()->SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR);
		CGlobals::GetEffectManager()->SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR);
#endif
	}
}


HRESULT CGUIContainer::Render(GUIState* pGUIState, float fElapsedTime)
{
	SetDirty(false);

	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();

	if (m_position.IsRelativeTo3DObject())
	{
		BeginDepthTest(pGUIState);
		UpdateRects();
	}

	RECT rcScreen;//rcScreen is the rect of this container
	//render background first
	rcScreen = m_objResource->GetDrawingRects(2);

	if (m_objResource->GetLayerType() == GUILAYER::ONE_ELEMENT)
	{
		DrawElement(m_objResource->GetTextureElement(0), &rcScreen, &rcScreen);
	}
	else if (m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT)
	{
		for (int e = 0; e < 9; ++e)
		{
			auto rc = m_objResource->GetDrawingRects(e + 3);
			DrawElement(m_objResource->GetTextureElement(e), &rc, &rcScreen, pGUIState);
		}
	}

	GUIBase_List_Type::iterator iter, iterend;
	iterend = this->GetChildren()->end();
	bool bRender = false;
	if (m_bBatching)
	{
		RECT rctext;
		SetRect(&rctext, 5, 5, 60, 30);
		static u16string sText;
		if (sText.empty())
			StringHelper::UTF8ToUTF16_Safe("Updating", sText);
		GetPainter(pGUIState)->DrawText(sText.c_str(), m_objResource->GetFontElement(0), &rctext, m_position.GetDepth());
	}
	else
	{
		for (iter = this->GetChildren()->begin(); iter != iterend; iter++)
		{
			CGUIBase* pObjChild = *iter;
			/// skip any node that is not visible
			if (!(pObjChild->m_bIsVisible))
				continue;
			RECT rcChild;
			bRender = false;

			if (!pObjChild->GetPosition()->IsRelativeTo3DObject())
			{
				if (pObjChild->m_bNeedUpdate)
					pObjChild->UpdateRects();
				rcChild = pObjChild->m_objResource->GetDrawingRects(0);
				if (GetFastRender() || RectIntersect(rcChild, rcScreen)){
					bRender = true;
				}
				else
					bRender = false;
			}
			if (bRender)
			{
				pObjChild->DoRender(pGUIState, fElapsedTime);
			}
		}
	}

	if (m_position.IsRelativeTo3DObject())
	{
		EndDepthTest(pGUIState);
	}
	OnFrameMove(fElapsedTime);

	return S_OK;
}

void CGUIContainer::SetScrollable(bool bScrollable)
{
	m_bScrollable = bScrollable;
	// if it is scrollable, we must not use fast rendering 
	if (m_bScrollable)
		SetFastRender(false);
}

bool CGUIContainer::IsScrollable()
{
	return m_bScrollable;
}

int CGUIContainer::GetScrollType()
{
	return m_ScrollType;
}

bool CGUIContainer::InvalidateRect(const RECT* lpRect)
{
	if (IsPainterWindow())
	{
		SetDirty(true);
	}
	else
	{
		CGUIBase::InvalidateRect(lpRect);

		if (lpRect == NULL)
		{
			m_bNeedCalClientRect = true;
			UpdateRects();
		}
		if (m_bScrollable) {
			if (m_ScrollType&HSCROLL){
				m_HScroll->Scroll(-1000);
			}
			if (m_ScrollType&VSCROLL){
				m_VScroll->Scroll(-1000);
			}
		}
	}
	return true;
}

void CGUIContainer::UpdateRects()
{
	CGUIPosition tempPos;
	GetAbsolutePosition(&tempPos, &m_position);
	if (m_position.IsRelativeTo3DObject())
	{
		SetDepth(tempPos.GetDepth());
	}

	RECT rc = m_objResource->GetDrawingRects(2);
	m_objResource->SetDrawingRects(&tempPos.rect, 2);
	bool bSizeChanged = !(rc.left == tempPos.rect.left && rc.top == tempPos.rect.top && rc.bottom == tempPos.rect.bottom && rc.right == tempPos.rect.right);

	m_objResource->SetDrawingRects(&tempPos.rect, 2);
	RECT rcInner = tempPos.rect;
	InflateRect(&rcInner, -m_nBorder, -m_nBorder);
	m_objResource->SetDrawingRects(&rcInner, 1);
	m_objResource->SetDrawingRects(&rcInner, 0);

	if (m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT)
	{
		const int offset = 3;
		RECT tempRect;
		// use the left top and right bottom texture element size to calculate the inner rect size. 
		GUITextureElement* pElement = m_objResource->GetTextureElement(1);
		if (pElement)
		{
			pElement->GetRect(&tempRect);
			rcInner.left = RectWidth(tempRect) + tempPos.rect.left;
			rcInner.top = RectHeight(tempRect) + tempPos.rect.top;
		}
		pElement = m_objResource->GetTextureElement(8);
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

	GUIBase_List_Type::iterator iter, iterend;
	// by LiXizhi, 2007.9.20: postpone UpdateClientRect if container is not scrollable, sicne client rect has nothing to do with rendering a fixed container. 
	if (m_bScrollable && m_bNeedCalClientRect){
		// SetRect(&m_clientRect,0,0,0,0);
		m_clientRect.right = 0;
		m_clientRect.bottom = 0;
		iterend = this->GetChildren()->end();
		for (iter = this->GetChildren()->begin(); iter != iterend; iter++)
		{
			CGUIBase* pObjChild = *iter;
			/// skip any node that is not visible
			if (!(pObjChild->GetVisible()))
				continue;
			UpdateClientRect(*pObjChild->GetPosition());
		}
		m_bNeedCalClientRect = false;
	}

	if (m_bScrollable) {
		UpdateScrollSize();
	}
	iterend = this->GetChildren()->end();
	for (iter = this->GetChildren()->begin(); iter != iterend; iter++)
	{
		CGUIBase* pObjChild = *iter;
		/// skip any node that is not visible
		if (!(pObjChild->m_bIsVisible))
			continue;
		pObjChild->UpdateRects();
	}
	if (m_ScrollType&HSCROLL){
		m_HScroll->UpdateRects();
	}
	if (m_ScrollType&VSCROLL){
		m_VScroll->UpdateRects();
	}

	m_bNeedUpdate = false;

	// call the event handler if size is changed.
	if (bSizeChanged)
		OnSize();
}

void CGUIContainer::UpdateScrollSize()
{
	if (GetType()->GetTypeValue() == Type_GUIRoot || (!m_bScrollable))
		return;
	RECT rc = m_objResource->GetDrawingRects(2);
	bool bSetV = false, bSetH = false;
	int width = RectWidth(rc) - m_nBorder * 2;
	int height = RectHeight(rc) - m_nBorder * 2;
	// modified by LiXizhi, 2006.10.29
	// replace m_clientRect.right with nClientWidth
	int nClientWidth = m_clientRect.right - m_clientRect.left;
	int nClientHeight = m_clientRect.bottom - m_clientRect.top;

	// add horizontal scroll bar, and decrease the client area
	if ((nClientWidth > width)){
		if ((GetScrollType()&HSCROLL) == 0){
			bSetH = true;
			SetScrollType(GetScrollType() | HSCROLL);
		}
	}
	else{
		if ((GetScrollType()&HSCROLL) != 0){
			bSetH = true;
			SetScrollType(GetScrollType() ^ HSCROLL);
		}
	}
	if ((GetScrollType()&HSCROLL) != 0){
		height -= (m_nSBWidth - m_nBorder);
	}

	// add vertical scroll bar, and decrease the client area
	if ((nClientHeight > height)){
		if ((GetScrollType()&VSCROLL) == 0){
			bSetV = true;
			SetScrollType(GetScrollType() | VSCROLL);
		}
	}
	else{
		if ((GetScrollType()&VSCROLL) != 0){
			bSetV = true;
			SetScrollType(GetScrollType() ^ VSCROLL);
		}
	}
	if ((GetScrollType()&VSCROLL) != 0){
		width -= (m_nSBWidth - m_nBorder);
	}


	/* why is this code here? deleted by LiXizhi. 2006.10.29
	if (bSetV&&(!bSetH)){
	bSetH=true;
	if (nClientWidth>width){
	SetScrollType(GetScrollType()|HSCROLL);
	height-=(m_nSBWidth-m_nBorder);
	}else{
	height=RectHeight(rc)-m_nBorder*2;
	}
	}*/

	m_HScroll->SetTrackRange(0, nClientWidth);
	m_VScroll->SetTrackRange(0, nClientHeight);
	if (bSetH){
		m_HScroll->SetPageSize(width);
		m_HScroll->SetSize(width, m_nSBWidth);
	}
	if (bSetV){
		m_VScroll->SetPageSize(height);
		m_VScroll->SetSize(m_nSBWidth, height);
	}

	// if scroll bar is available, the client area need to be adjusted (dwindled) accordingly. 
	RECT rc1 = m_objResource->GetDrawingRects(0);
	rc1.bottom = rc1.top + height;
	rc1.right = rc1.left + width;
	m_objResource->SetDrawingRects(&rc1, 0);
	m_bNeedUpdate = true;
	SetDirty(true);
}

//should only update reference coordinate
void CGUIContainer::UpdateClientRect(const CGUIPosition &pIn, bool unupdate/*=false*/)
{
	/** disable update client if it is not scrollable*/
	if (!m_bScrollable || GetType()->GetTypeValue() == Type_GUIRoot){
		return;
	}

	GUIState *pGUIState = &CGUIRoot::GetInstance()->GetGUIState();
	RECT rc = pIn.rect;

	if (pIn.IsRelativeToScreen())
	{
		RECT parent = { 0, 0, static_cast<LONG>(pGUIState->nBkbufWidth), static_cast<LONG>(pGUIState->nBkbufHeight) };
		CGUIPosition::GetAbsPosition(&rc, &parent, &pIn);
	}
	else if (pIn.IsRelativeToParent())
	{
		RECT parent = m_objResource->GetDrawingRects(2);
		CGUIPosition::GetAbsPosition(&rc, &parent, &pIn);
		OffsetRect(&rc, -parent.left, -parent.top);
	}

	if (unupdate && (rc.right == m_clientRect.right || rc.bottom == m_clientRect.bottom)){
		m_bNeedCalClientRect = true;
	}
	//update the client area
	if (m_clientRect.right < rc.right)
		m_clientRect.right = rc.right;
	if (m_clientRect.bottom < rc.bottom)
		m_clientRect.bottom = rc.bottom;
	m_bNeedUpdate = true;
	SetDirty(true);
}

void CGUIContainer::UpdateScroll(int nXDelta, int nYDelta)
{
	MSG newMsg;
	newMsg.message = EM_SB_SCROLL;
	newMsg.time = GetTickCount();
	newMsg.hwnd = CGlobals::GetAppHWND();
	newMsg.lParam = (DWORD)(nXDelta);
	newMsg.wParam = (DWORD)(nYDelta);
	MsgProc(&newMsg);
}
void CGUIContainer::NextKeyFocus()
{
	CGUIBase *keyfocus = GetKeyFocus();
	if (keyfocus == 0)
		return;

	if (!m_children.empty())
	{
		GUIBase_List_Type::iterator iter, iterEnd = m_children.end();
		for (iter = m_children.begin(); iter != iterEnd; iter++) {
			if ((*iter) == keyfocus) {
				iter++;
				break;
			}
		}
		// bug fixed: it will crash in old version. LXZ 2008.10.15
		for (; iter != iterEnd; iter++)
		{
			if ((*iter)->CanHaveFocus()) {
				SetKeyFocus((*iter));
				return;
			}
		}
		for (iter = m_children.begin(); iter != iterEnd; iter++) {
			if ((*iter)->CanHaveFocus()) {
				SetKeyFocus((*iter));
				return;
			}
		}
	}
}

bool ParaEngine::CGUIContainer::ActivateDefaultButton()
{
	CGUIButton* pDefaultButton = GetDefaultButton();
	if (pDefaultButton != 0)
	{
		// click the button
		pDefaultButton->OnClick(EM_MOUSE_LEFT, 0, 0);
		return true;
	}
	return false;
}

CGUIButton* ParaEngine::CGUIContainer::GetDefaultButton()
{
	GUIBase_List_Type::iterator iter;
	for (iter = m_children.begin(); iter != m_children.end(); iter++) {
		if (((*iter)->GetType()->GetTypeValue() == Type_GUIButton) && ((CGUIButton*)(*iter))->IsDefaultButton()) {
			return ((CGUIButton*)(*iter));
		}
	}
	return NULL;
}

bool CGUIContainer::ActivateNextEdit(CGUIEditBox* curCtrl)
{
	auto pNextCtrl = GetNextEdit(curCtrl);
	if (pNextCtrl)
	{
		SetKeyFocus(pNextCtrl);
		return true;
	}

	return false;
}

CGUIEditBox* CGUIContainer::GetNextEdit(CGUIEditBox* curCtrl)
{
	GUIBase_List_Type::iterator it;
    
	if (curCtrl)
	{
		it = std::find(m_children.begin(), m_children.end(), (CGUIBase*)curCtrl);
		auto nextIt = it;
		nextIt++;
		for (; nextIt != m_children.end(); nextIt++)
		{
			auto type = (*nextIt)->GetType()->GetTypeValue();
			if ((*nextIt)->GetVisible() && (type == Type_GUIEditBox || type == Type_GUIIMEEditBox))
			{
				break;
			}
		}
		
		if (nextIt == m_children.end())
		{
			bool bFind = false;
			for (nextIt = m_children.begin(); nextIt != it; nextIt++)
			{
				auto type = (*nextIt)->GetType()->GetTypeValue();
				if ((*nextIt)->GetVisible() && (type == Type_GUIEditBox || type == Type_GUIIMEEditBox))
				{
					bFind = true;
					break;
				}
			}

			if (!bFind)
			{
				nextIt = m_children.end();
			}
		}

		it = nextIt;
	}
	else
	{
		for (it = m_children.begin(); it != m_children.end(); it++)
		{
			auto type = (*it)->GetType()->GetTypeValue();
			if (type == Type_GUIEditBox || type == Type_GUIIMEEditBox)
			{
				break;
			}
		}
	}

	if (it != m_children.end())
	{
		return static_cast<CGUIEditBox*>(*it);
	}

	return nullptr;
}

void CGUIContainer::SetVisible(bool visible)
{
	if (!visible&&m_bIsTop){
		SetTopLevel(false);
	}
	CGUIBase::SetVisible(visible);
}

void CGUIContainer::SetCandrag(bool bCandrag)
{
	if (bCandrag&&m_bIsTop){
		SetTopLevel(false);
	}
	CGUIBase::SetCandrag(bCandrag);
}
void CGUIContainer::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm = &CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIContainer")) {
		CGUIContainer::StaticInit();
	}
	pOm->CloneObject("default_CGUIContainer", this);
	CGUIBase::InitObject(strObjectName, alignment, x, y, width, height);
	//initialize scrollbars
	string str;

	str = GetName() + ".vscrollbar";
	// Modified 2007.4.10 LiXizhi, alignment type is changed.
	//m_VScroll->InitObject(str.c_str(),"_rt",-m_nSBWidth-m_nBorder,m_nBorder,m_nSBWidth,RectHeight(m_objResource->GetDrawingRects(0)));
	m_VScroll->InitObject(str.c_str(), "_mr", m_nBorder, m_nBorder, m_nSBWidth, m_nBorder);
	m_VScroll->m_parent = this;
	m_VScroll->SetLinkedObject(this);
	// 	m_VScroll->UpdateRects();

	str = GetName() + ".hscrollbar";
	//m_HScroll->InitObject(str.c_str(),"_lb",m_nBorder,-m_nSBWidth-m_nBorder,RectWidth(m_objResource->GetDrawingRects(0)),m_nSBWidth);
	m_HScroll->InitObject(str.c_str(), "_mb", m_nBorder, m_nBorder, m_nBorder + m_nSBWidth, m_nSBWidth);
	m_HScroll->m_parent = this;
	m_HScroll->SetLinkedObject(this);
	// 	m_HScroll->UpdateRects();
	SetSize(width, height);
}

CGUIBase* CGUIContainer::GetObjectAtPoint(int x, int y)
{
	CGUIBase* pDest = NULL;
	CGUIBase* pObj;

	RECT rc = m_objResource->GetDrawingRects(0);
	if (x > rc.right || y > rc.bottom) {
		return this;
	}
	GUIBase_List_Type::reverse_iterator iter, iterEnd = GetChildren()->rend();
	for (iter = GetChildren()->rbegin(); iter != iterEnd; iter++)
	{
		pObj = *iter;
		if (pObj->m_bIsVisible && pObj->m_bIsEnabled /* added by LiXizhi: bug fixed when object is dragged over a disabled object*/
			&& pObj->IsPointInControl(x, y))
		{
			//set the target where the mouse event is sent to 
			pDest = pObj;
			break;
		}
	}

	if (pDest == NULL) {
		pDest = this;
	}
	return pDest;
}

CGUIBase* CGUIContainer::GetObjectAtPointRecursive(int x, int y)
{
	CGUIBase* pDest = NULL;
	CGUIBase* pObj;

	RECT rc = m_objResource->GetDrawingRects(0);
	if (x > rc.right || y > rc.bottom) {
		return this;
	}
	GUIBase_List_Type::reverse_iterator iter, iterEnd = GetChildren()->rend();
	for (iter = GetChildren()->rbegin(); iter != iterEnd; iter++)
	{
		pObj = *iter;
		if (pObj->m_bIsVisible && pObj->m_bIsEnabled /* added by LiXizhi: bug fixed when object is dragged over a disabled object*/
			&& pObj->IsPointInControl(x, y))
		{
			pDest = pObj;
			//set the target where the mouse event is sent to 
			if (((CGUIType*)pObj->GetType())->IsContainer())
			{
				pDest = ((CGUIContainer*)pObj)->GetObjectAtPointRecursive(x, y);
				if (pDest == 0)
					pDest = pObj;
			}
			if (!(pDest->IsClickThrough()))
			{
				break;
			}
			else
			{
				// do not return click through object. 
				pDest = NULL;
			}
		}
	}

	if (pDest == 0 && IsClickThrough())
	{
		pDest = this;
	}
	return pDest;
}


GUITextureElement* CGUIContainer::GetTextureElement(const char *texturename)
{
	if (strcmp(texturename, "vscrollbar.track") == 0) {
		return m_VScroll->m_objResource->GetTextureElement(0);
	}
	else
		if (strcmp(texturename, "vscrollbar.upbutton") == 0) {
			return m_VScroll->m_objResource->GetTextureElement(1);
		}
		else
			if (strcmp(texturename, "vscrollbar.downbutton") == 0) {
				return m_VScroll->m_objResource->GetTextureElement(2);
			}
			else
				if (strcmp(texturename, "vscrollbar.thumb") == 0) {
					return m_VScroll->m_objResource->GetTextureElement(3);
				}
				else
					if (strcmp(texturename, "hscrollbar.track") == 0) {
						return m_HScroll->m_objResource->GetTextureElement(0);
					}
					else
						if (strcmp(texturename, "hscrollbar.leftbutton") == 0) {
							return m_HScroll->m_objResource->GetTextureElement(1);
						}
						else
							if (strcmp(texturename, "hscrollbar.rightbutton") == 0) {
								return m_HScroll->m_objResource->GetTextureElement(2);
							}
							else
								if (strcmp(texturename, "hscrollbar.thumb") == 0) {
									return m_HScroll->m_objResource->GetTextureElement(3);
								}
								else
								{
									return m_objResource->GetTextureElement(texturename);
								}
}

void CGUIContainer::SetScrollbarWidth(int width)
{
	PE_ASSERT(width > 0);
	m_nSBWidth = width;
	m_HScroll->SetScrollbarWidth(m_nSBWidth);
	m_VScroll->SetScrollbarWidth(m_nSBWidth);
	m_VScroll->SetLocation(-m_nSBWidth - m_nBorder, m_nBorder);
	m_HScroll->SetLocation(m_nBorder, -m_nSBWidth - m_nBorder);
	m_bNeedUpdate = true;
	SetDirty(true);
}

struct ZOrderLessCompare
{
	inline bool operator()(const CGUIBase* _Left, const CGUIBase* _Right) const
	{
		return (_Left->GetZOrder()) < (_Right->GetZOrder());
	};
};

int ParaEngine::CGUIContainer::GetChildCount()
{
	return (int)m_children.size();
}

bool ParaEngine::CGUIContainer::SortChildrenByZOrder()
{
	std::stable_sort(m_children.begin(), m_children.end(), ZOrderLessCompare());
	return true;
}

int ParaEngine::CGUIContainer::GetZOrder() const
{
	return (!m_bIsTop) ? CGUIBase::GetZOrder() : CGUIBase::GetZOrder() + 1000;
}

void ParaEngine::CGUIContainer::SetDepth(float fDepth)
{
	CGUIBase::SetDepth(fDepth);

	GUIBase_List_Type::iterator itCurCP, itEndCP = m_children.end();

	for (itCurCP = m_children.begin(); itCurCP != itEndCP; ++itCurCP)
	{
		(*itCurCP)->SetDepth(fDepth);
	}
}

int ParaEngine::CGUIContainer::GetChildIndex(CGUIBase* pChild)
{
	int i = 0;
	for (auto pCurChild : m_children)
	{
		if (pCurChild == pChild)
			return i;
		i++;
	}
	return -1;
}

bool ParaEngine::CGUIContainer::IsNonClientTestEnabled()
{
	return m_bEnableNonClientTest;
}

void ParaEngine::CGUIContainer::EnableNonClientTest(bool val)
{
	m_bEnableNonClientTest = val;
}

CRenderTarget* ParaEngine::CGUIContainer::CreateGetRenderTarget(bool bCreateIfNotExist)
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
			pRenderTarget->SetRenderTargetSize(Vector2(128, 128));
			CGlobals::GetScene()->AttachObject(pRenderTarget);
			pRenderTarget->SetDirty(false);
			pRenderTarget->SetVisibility(false);
			m_renderTarget = pRenderTarget->GetWeakReference();
			return pRenderTarget;
		}
	}
	return NULL;
}


bool ParaEngine::CGUIContainer::CanPaintOnDevice(CPaintDevice* val)
{
	return CGUIBase::CanPaintOnDevice(val) || CreateGetRenderTarget(false) == val;
}

HRESULT ParaEngine::CGUIContainer::RestoreDeviceObjects()
{
	m_bNeedUpdate = true;
	m_bNeedCalClientRect = true;
	SetDirty(true);
	return 0;
}

ParaEngine::QPoint ParaEngine::CGUIContainer::GetCompositionPoint()
{
	if (m_compositionPoint.x() != 0 || m_compositionPoint.y() != 0)
	{
		int x = m_compositionPoint.x();
		int y = m_compositionPoint.y();
		CGlobals::GetApp()->GameToClient(x, y);
		float fScaleX = 1.f;
		float fScaleY = 1.f;
		CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);
		if (fScaleX != 1.f)
		{
			x = (int)((float)x * fScaleX);
		}

		if (fScaleY != 1.f)
		{
			y = (int)((float)y * fScaleY);
		}
		return QPoint(x, y);
	}
	else
		return CGUIBase::GetCompositionPoint();
}

void ParaEngine::CGUIContainer::SetCompositionPoint(const QPoint& point)
{
	m_compositionPoint = point;
	CGUIBase::SetCompositionPoint(point);
}

void ParaEngine::CGUIScrollBar::UpdateThumbRectNineElement()
{
	if (!m_bFixedThumb && m_bShowThumb)
	{
		RECT tempRect = m_objResource->GetDrawingRects(3);

		const int offset = 4;

		GUIResourceState StateRes = m_objResource->GetCurrentState();
		m_objResource->SetCurrentState(GUIResourceState_Normal);

		RECT bgRect = tempRect;
		bgRect.left = m_objResource->GetDrawingRects(1).left;
		bgRect.right = m_objResource->GetDrawingRects(1).right;
		RECT rcInner = bgRect;

		// use the left top and right bottom texture element size to calculate the inner rect size. 
		GUITextureElement* pElement = m_objResource->GetTextureElement(offset + 1);
		if (pElement)
		{
			pElement->GetRect(&tempRect);
			rcInner.left = RectWidth(tempRect) + bgRect.left;
			rcInner.top = RectHeight(tempRect) + bgRect.top;
		}
		pElement = m_objResource->GetTextureElement(offset + 8);
		if (pElement)
		{
			pElement->GetRect(&tempRect);
			rcInner.right = bgRect.right - RectWidth(tempRect);
			rcInner.bottom = bgRect.bottom - RectHeight(tempRect);
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
		SetRect(&tempRect, bgRect.left, bgRect.top, rcInner.left, rcInner.top);
		m_objResource->SetDrawingRects(&tempRect, offset + 1);
		SetRect(&tempRect, rcInner.left, bgRect.top, rcInner.right, rcInner.top);
		m_objResource->SetDrawingRects(&tempRect, offset + 2);
		SetRect(&tempRect, rcInner.right, bgRect.top, bgRect.right, rcInner.top);
		m_objResource->SetDrawingRects(&tempRect, offset + 3);
		SetRect(&tempRect, bgRect.left, rcInner.top, rcInner.left, rcInner.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 4);
		SetRect(&tempRect, rcInner.right, rcInner.top, bgRect.right, rcInner.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 5);
		SetRect(&tempRect, bgRect.left, rcInner.bottom, rcInner.left, bgRect.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 6);
		SetRect(&tempRect, rcInner.left, rcInner.bottom, rcInner.right, bgRect.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 7);
		SetRect(&tempRect, rcInner.right, rcInner.bottom, bgRect.right, bgRect.bottom);
		m_objResource->SetDrawingRects(&tempRect, offset + 8);

		m_objResource->SetCurrentState(StateRes);
	}
}
