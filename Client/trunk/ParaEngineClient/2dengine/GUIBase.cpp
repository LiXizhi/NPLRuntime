//----------------------------------------------------------------------
// Class:	CGUIBase
// Authors:	LiXizhi, Liu Weili
// Date:	2005.8.1
// Revised:	2006.10.27 by Li,Xizhi
//
// desc: 
// Base class for all GUI elements. It defines the basic behavior of the GUI object.
// We support activation of the remote or local script within a CGUIBase class. 
// It also contains a resource object to identifying its appearance and behavior.
// Manipulating such resource object is the role of the CGUIResource class and 
// we will not talk about it here. 
//
// @change 2006.10.27 by LiXizhi: drag and drop operation is revised. so that if the destination
// can not receive drag, it will restore to the source, instead of the parent of the destination. 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ic/ICConfigManager.h"
#include "IParaEngineApp.h"
#include "NPLHelper.h"
#include "NPLRuntime.h"
#include <list>
#include <queue>
#include "GUIHighlight.h"
#include "GUIDirectInput.h"
#include "GUIRoot.h"
#include "GUIContainer.h"
#include "GUIResource.h"
#include "GUIScript.h"
#include "PaintEngine/Painter.h"
#include "MiscEntity.h"
#include "util/StringHelper.h"
#include "EventBinding.h"
#include "EventsCenter.h"
#include "ObjectManager.h"
#include "ParaWorldAsset.h"
#include "RenderTarget.h"
#include "EffectManager.h"
#include "ViewportManager.h"
#include "GUIIME.h"
#include "memdebug.h"
using namespace ParaEngine;

namespace ParaEngine
{
	/** globals for generating id*/
	static int g_nNextID = 0;

	/** generate a unique id for ui object*/
	int GenerateUIObj_ID(){
		return ++g_nNextID;
	}
}


CGUIBase::CGUIBase(void)
	:m_bIsVisible(true),m_nZOrder(0),m_bIsEnabled(true), m_bUseTextShadow(false), m_bAlwaysMouseOver(false), 
	m_nLifeTimeCountDown(-1), // permanent
	m_objResource(NULL),
	m_parent(NULL),
	m_pForcedPaintDevice(NULL),
	m_bMouseOver(false),
	m_bCanCaptureMouse(true),
	m_bHasFocus(false),
	m_bCanHasFocus(false),
	m_bReceiveDrag(false),
	m_bNeedUpdate(true),
	m_bUsePointTextureFiltering(false),
	m_bInputMethodEnabled(false),
	m_bOwnerDraw(false), m_bDirty(true), m_bAutoClearBackground(true),
	m_eHighlight(HighlightNone),
	m_event(NULL),
	m_fRotation(0.f),
	m_vRotOriginOffset(0.f,0.f),
	m_vScaling(1.f, 1.f),
	m_vTranslation(0.f,0.f),
	m_nSpacing(0),
	m_nID(-1),
	m_dwColorMask(0xFFFFFFFF),
	m_fForceZDepth(-1.f),
	m_bClickThrough(false),
	m_bIsUVWrappingEnabled(false),
	m_nHotkey(0),m_nCursorHotSpotX(-1),m_nCursorHotSpotY(-1),
	m_textShadowQuality(0),m_textShadowColor(0),
	m_textOffsetX(0),m_textOffsetY(0), m_touchTranslateAttFlag(0xff)
{
	// m_bIsDummy = false;
	// m_objType = GUIBase;
	// SetCandrag(false);
}

CGUIBase::~CGUIBase(void)
{
	OnDestroy();
	SAFE_RELEASE(m_objResource);
	SAFE_RELEASE(m_event);

	// cancel dragging
	STRUCT_DRAG_AND_DROP *pdrag=&IObjectDrag::DraggingObject;
	if(pdrag->pDragging==this)
	{
		if (pdrag->pCleanUp!=NULL) {
			pdrag->pCleanUp();
		}
		pdrag->init();
	}

	CGUIRoot *root = CGUIRoot::GetInstance();
	{
		// erase from name map
		std::map<string, CGUIBase*>::iterator iter = root->m_namemap.find(GetName());
		if (iter != root->m_namemap.end() && iter->second == this) {
			root->m_namemap.erase(iter);
		}
	}
	{
		// erase from id map
		std::map<int, CGUIBase*>::iterator iter = root->m_idmap.find(GetID());
		if (iter != root->m_idmap.end() && iter->second == this) {
			root->m_idmap.erase(iter);
		}
		else{
			// OUTPUT_LOG("warning: UI object with id %d and name %s is not found in id map upon deletion. \n", GetID(), GetName().c_str());
		}
	}
	root->m_scripts.DeleteScript(this);
}

void ParaEngine::CGUIBase::DeleteThis()
{
	CGUIRoot::GetInstance()->DeleteGUIObject(this);
}

int ParaEngine::CGUIBase::GetChildAttributeColumnCount()
{
	return 1;
}

IAttributeFields* ParaEngine::CGUIBase::GetChildAttributeObject(const char * sName)
{
	return GetChildByName(sName);
}

IAttributeFields* ParaEngine::CGUIBase::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	auto pChildren = GetChildren();
	if (pChildren && nRowIndex < (int)pChildren->size()){
		return (*pChildren)[nRowIndex];
	}
	return NULL;
}

int ParaEngine::CGUIBase::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	auto pChildren = GetChildren();
	return pChildren ? (int)pChildren->size() : 0;
}

int ParaEngine::CGUIBase::GetIndex()
{
	CGUIContainer* temp = GetParent();
	if (temp)
	{
		return temp->GetChildIndex(this);
	}
	return -1;
}

bool CGUIBase::DoAutoSize()
{
	return true;
}

int CGUIBase::Release()
{
	
	//TODO:  if(-- reference count <=0 )
	delete this;
	return 0;
	

	//return IObject::Release();
}

bool CGUIBase::Equals(const IObject *obj)const
{
	return this==obj?true:false;
}

void CGUIBase::Clone(IObject* pobj)const
{
	PE_ASSERT(pobj!=NULL);
	CGUIBase *pBase=(CGUIBase*)pobj;
	if (pBase==NULL) {
		return;
	}
	pBase->m_bIsVisible = m_bIsVisible;
	pBase->m_nZOrder = m_nZOrder;
	pBase->m_bIsEnabled = m_bIsEnabled;
	pBase->m_touchTranslateAttFlag = m_touchTranslateAttFlag;
	//pBase->m_bIsDummy = m_bIsDummy;
	pBase->m_nLifeTimeCountDown = m_nLifeTimeCountDown;	
// 	pBase->m_objType = m_objType;
	pBase->m_position=m_position;
	pBase->m_parent=NULL;
	pBase->m_bMouseOver=m_bMouseOver;
	pBase->m_bHasFocus=m_bHasFocus;
	pBase->m_bCanHasFocus=m_bCanHasFocus;
	pBase->m_bCandrag=m_bCandrag;
	pBase->m_bReceiveDrag=m_bReceiveDrag;
	pBase->m_bNeedUpdate=m_bNeedUpdate;
	pBase->m_bClickThrough = m_bClickThrough;
	pBase->m_nHotkey=m_nHotkey;
	//pBase->m_textOffsetX=m_textOffsetX;
	//pBase->m_textOffsetY=m_textOffsetY;
	SAFE_RELEASE(pBase->m_event);
	//pBase->m_event=(CGUIEvent*)m_event->Clone();
	pBase->m_event=new CGUIEvent();
	pBase->m_event->SetBinding(pBase);
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	pBase->m_event->m_eventbinding=m_event->m_eventbinding;
	SAFE_RELEASE(pBase->m_objResource);
	pBase->m_objResource=(CGUIResource*)m_objResource->Clone();
	pBase->m_objResource->SetCurrentState();
}

IObject* CGUIBase::Clone()const
{
	CGUIBase *pObj=new CGUIBase();
	Clone(pObj);
	return pObj;
}

void CGUIBase::StaticInit()
{
	//here we initialize the default properties for CGUIBase, and store the object in ObjectManager
	//When the descendant class of CGUIBase statically initialize itself, it obtain the default CGUIBase object 
	//from the ObjectManager and adds its own default properties and store in the ObjectManager again.
	//When the engine instantiates a GUI object, it clones an object form the ObjectManager
	//It will not get the default CGUIBase from ObjectManager directly.
	//Further improvement may be loading default values from configuration file.
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	CGUIBase *pBase=new CGUIBase();
	pBase->m_bIsVisible = true;
	pBase->m_bIsEnabled = true;
	//pBase->m_bIsDummy = false;
	pBase->m_nLifeTimeCountDown = -1;	// permanent
// 	pBase->m_objType = GUIBase;
	pBase->m_position=CGUIPosition();
	pBase->m_parent=NULL;
	pBase->m_bMouseOver=false;
	pBase->m_bHasFocus=false;
	pBase->m_bCanHasFocus=false;
	pBase->SetCandrag(false);
	pBase->m_bReceiveDrag=false;
	pBase->m_bNeedUpdate=true;
	pBase->m_nHotkey=0;
	pBase->m_event=new CGUIEvent();
	pBase->m_objResource = new CGUIResource();
	pBase->m_objResource->SetActiveLayer();
	
	//this eventbinding object will set to the m_event of the pBase
	if (!pOm->IsExist("default_CEventBinding")) {
		CEventBinding::StaticInit();
	}
	CEventBinding* pBinding=(CEventBinding*)pOm->CloneObject("default_CEventBinding");
	using namespace ParaInfoCenter;
	CICConfigManager *cm=CGlobals::GetICConfigManager();
	string value0,value1;
	int event0,event1,a;
	DWORD b;
	HRESULT hr;
	hr=cm->GetSize("GUI_basic_control_mapping",&b);
	if (hr==E_INVALIDARG||hr==E_ACCESSDENIED) {
		//error
	}else{
		for (a=0;a<(int)b;a+=2) {
			hr=cm->GetTextValue("GUI_basic_control_mapping",value0,a);
			if (hr!=S_OK) {
				break;
			}
			hr=cm->GetTextValue("GUI_basic_control_mapping",value1,a+1);
			if (hr!=S_OK) {
				break;
			}
			event0=CEventBinding::StringToEventValue(value0);
			event1=CEventBinding::StringToEventValue(value1);
			pBinding->MapEvent(event0,event1);
		}
	}
	hr=cm->GetSize("GUI_basic_control_script",&b);
	if (hr==E_INVALIDARG||hr==E_ACCESSDENIED) {
	}else{
		for (a=0;a<(int)b;a+=2) {
			hr=cm->GetTextValue("GUI_basic_control_script",value0,a);
			if (hr!=S_OK) {
				break;
			}
			hr= cm->GetTextValue("GUI_basic_control_script",value1,a+1);
			if (hr!=S_OK) {
				break;
			}
			event0=CEventBinding::StringToEventValue(value0);
			SimpleScript script;
			StringHelper::DevideString(value1,script.szFile,script.szCode,';');
			pBinding->MapEventToScript(event0,&script);
		}
	}
	int tempint;
	if (cm->GetIntValue("GUI_basic_control_visible",&tempint)==S_OK) {
		if (tempint==0) {
			pBase->m_bIsVisible=false;
		}else
			pBase->m_bIsVisible=true;
	}
	if (cm->GetIntValue("GUI_basic_control_enable",&tempint)==S_OK) {
		if (tempint==0) {
			pBase->m_bIsEnabled=false;
		}else
			pBase->m_bIsEnabled=true;
	}
	if (cm->GetIntValue("GUI_basic_control_canhasfocus",&tempint)==S_OK) {
		if (tempint==0) {
			pBase->m_bCanHasFocus=false;
		}else
			pBase->m_bCanHasFocus=true;
	}
	if (cm->GetIntValue("GUI_basic_control_receivedrag",&tempint)==S_OK) {
		if (tempint==0) {
			pBase->m_bReceiveDrag=false;
		}else
			pBase->m_bReceiveDrag=true;
	}
	if (cm->GetIntValue("GUI_basic_control_candrag",&tempint)==S_OK) {
		if (tempint==0) {
			pBase->SetCandrag(false);
		}else
			pBase->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_basic_control_lifetime",&tempint)==S_OK) {
		pBase->m_nLifeTimeCountDown=tempint;
	}
	pBase->m_event->m_eventbinding=EventBinding_cow_type(pBinding);
	pOm->SetObject("default_CGUIBase",pBase);
	SAFE_RELEASE(pBase);
}


string CGUIBase::ToScript(int option/*=0*/)
{
	string script="";//by default we have s"local __this,__parent,__res1,__res2;";

	//add a "button2=ParaUI.CreateUIObject("button","btn_cancel", "_rb",-140,-50, 100,30);" like script
	script+="__this=ParaUI.CreateUIObject(\"";
	switch(GetType()->GetTypeValue()) {
	case Type_GUIButton:
		script+="button";
		break;
	case Type_GUIEditBox:
		script+="editbox";
		break;
	case Type_GUIIMEEditBox:
		script+="imeeditbox";
		break;
	case Type_GUIScrollBar:
		script+="scrollbar";
		break;
	case Type_GUIText:
		script+="text";
		break;
	case Type_GUISlider:
		script+="slider";
		break;
	case Type_GUIListBox:
		script+="listbox";
		break;
	case Type_GUIContainer:
		script+="container";
		break;
	}
	script+="\",\"";
	script+=GetName();
	script+="\", \"";
	switch((int)m_position.Relative.To2D.alignType) {
	case CGUIPosition::bottom_right:
		script+="_rb";
		break;
	case CGUIPosition::bottom_left:
		script+="_lb";
		break;
	case CGUIPosition::center:
		script+="_ct";
		break;
	case CGUIPosition::center_top:
		script+="_ctt";
		break;
	case CGUIPosition::center_bottom:
		script+="_ctb";
		break;
	case CGUIPosition::center_left:
		script+="_ctl";
		break;
	case CGUIPosition::center_right:
		script+="_ctr";
		break;
	case CGUIPosition::upper_left:
		script+="_lt";
		break;
	case CGUIPosition::upper_right:
		script+="_rt";
		break;
	case CGUIPosition::middle_top:
		script+="_mt";
		break;
	case CGUIPosition::middle_right:
		script+="_mr";
		break;
	case CGUIPosition::middle_bottom:
		script+="_mb";
		break;
	case CGUIPosition::middle_left:
		script+="_ml";
		break;
	case CGUIPosition::fill:
		script+="_fi";
		break;
	}
	script+="\",";
	char temp[30];
	ParaEngine::StringHelper::fast_itoa(m_position.rect.left,temp,30);
	script+=temp;
    script+=",";
	ParaEngine::StringHelper::fast_itoa(m_position.rect.top,temp,30);
	script+=temp;
	script+=",";
	ParaEngine::StringHelper::fast_itoa(m_position.GetWidth(),temp,30);
	script+=temp;
	script+=",";
	ParaEngine::StringHelper::fast_itoa(m_position.GetHeight(),temp,30);
	script+=temp;
	script+=");\n";
	
	//Attach to root or parent
	if (m_parent) {
		if ((m_parent->GetType()->GetTypeValue()==Type_GUIRoot)||\
			(CGUIRoot::GetInstance()->m_bDesign&&m_parent->GetName()[0]=='_'&&m_parent->GetName()[1]=='_')) {
			//add "		__this:AttachToRoot();"like script;
			script+="__this:AttachToRoot();\n";
		}else{
			//add "__parent=ParaUI.GetUIObject("name");" like script
			script+="__parent=ParaUI.GetUIObject(\"";
			script+=m_parent->GetName();
			script+="\");";
			//add "__parent:AddChild(__this);"like script
			script+="__parent:AddChild(__this);\n";
		}
	}

	return script;
}

const std::string& ParaEngine::CGUIBase::GetIdentifier()
{
	return GetName();
}

void ParaEngine::CGUIBase::SetIdentifier(const std::string& sID)
{
	SetName(sID.c_str());
}

//----------------------------------------------------------------------------------
/// compute the absolute screen position for the object.
//----------------------------------------------------------------------------------
void CGUIBase::GetAbsolutePosition(CGUIPosition* pOut, const CGUIPosition* pIn)
{
	GUIState *pGUIState=&CGUIRoot::GetInstance()->GetGUIState();
	if(pIn->IsRelativeToScreen())
	{
		RECT parent = {0,0,static_cast<LONG>(pGUIState->nBkbufWidth), static_cast<LONG>(pGUIState->nBkbufHeight)};
		CGUIPosition::GetAbsPosition(&pOut->rect, &parent, pIn);
		pOut->Relative.To2D.alignType = CGUIPosition::upper_left;
	}
	else if(pIn->IsRelativeToParent())
	{
		CGUIPosition parentPos(0,0,pGUIState->nBkbufWidth,pGUIState->nBkbufHeight);
		if (m_parent) {
			m_parent->GetAbsolutePosition(&parentPos,&m_parent->m_position);
		}
		CGUIPosition::GetAbsPosition(&pOut->rect, &parentPos.rect, pIn);
		pOut->Relative.To2D.alignType = CGUIPosition::upper_left;
		pOut->SetDepth(parentPos.GetDepth());
	}
	else if(pIn->IsRelativeTo3DObject())
	{
		//calculate 3d coordinates of the 3d object
		Vector3 vIn,vOut;
		ParaViewport  viewport;
		vIn = *(Vector3*)(&(pIn->Relative.To3D.m_v3DPosition));
		
		CViewport* pViewport = CGlobals::GetViewportManager()->GetActiveViewPort();
		CViewport* pViewportScene = pViewport;

		// find the closet scene viewport whose index is bigger than the current viewport
		if (pViewport->GetScene() == 0)
		{
			auto pViewportManager = CGlobals::GetViewportManager();
			int nCount = pViewportManager->GetViewportCount();
			int nCurIndex = -1;
			for (int i = 0; i < nCount; ++i)
			{
				CViewport* pViewport2 = pViewportManager->CreateGetViewPort(i);
				if (pViewport2->GetScene())
				{
					pViewportScene = pViewport2;
					if (nCurIndex >= 0)
						break;
				}
				else if (pViewport == pViewport2)
				{
					nCurIndex = i;
				}
			}
		}
		viewport.X = pViewportScene->GetLeft() - pViewport->GetLeft();
		viewport.Y = pViewportScene->GetTop() - pViewport->GetTop();
		viewport.Width = pViewportScene->GetWidth();
		viewport.Height = pViewportScene->GetHeight();
		

		ParaVec3Project(&vOut, &vIn, &viewport, CGUIRoot::GetInstance()->Get3DViewProjMatrix(), NULL, NULL);
		
		int nWidth = pIn->rect.right - pIn->rect.left;
		int nHeight = pIn->rect.bottom - pIn->rect.top;
		pOut->rect.left=pIn->rect.left+Math::Round(vOut.x);
		pOut->rect.top=pIn->rect.top+Math::Round(vOut.y);

		pOut->rect.right = pOut->rect.left+nWidth;
		pOut->rect.bottom = pOut->rect.top+nHeight;

		float fScaleX = 1.f;
		float fScaleY = 1.f;
		CGUIRoot::GetInstance()->GetUIScale(&fScaleX, &fScaleY);
		if(fScaleX != 1.f)
		{
			int nWidth = pOut->rect.right - pOut->rect.left;
			pOut->rect.left = (int)((float)pOut->rect.left / fScaleX);
			pOut->rect.right = pOut->rect.left + nWidth;
		}

		if(fScaleY != 1.f)
		{
			int nHeight = pOut->rect.bottom - pOut->rect.top;
			pOut->rect.top = (int)((float)(pOut->rect.top) / fScaleY);
			pOut->rect.bottom = pOut->rect.top + nHeight;
		}

		pOut->Update3DDepth((GetZDepth() < 0.f) ? vOut.z: GetZDepth());
	}
}

void CGUIBase::AttachTo3D(const char* s3DObjectName)
{
	GetPosition()->SetRelatveTo3DObject(s3DObjectName);
	SetVisible(false);
}

void ParaEngine::CGUIBase::AttachTo3D(CBaseObject* p3DObject)
{
	GetPosition()->SetRelatveTo3DObject(p3DObject);
	SetVisible(false);
}

void CGUIBase::BringToFront()
{
	if (m_parent){
		m_parent->BringToFront(this);
	}
}

void CGUIBase::SendToBack()
{
	if (m_parent){
		m_parent->SendToBack(this);
	}
}

void CGUIBase::ClearAllEvent()
{
	if(m_event)
	{
		m_event->Initialize();
		m_event->SetBinding(this);
	}
}

void CGUIBase::ClearEvent(int etype)
{
	if (m_event && etype==CGUIEvent::KEYBOARD) {
		m_event->m_keyboard.Size=0;
		m_event->m_keyboard.HoldKey.clear();
	}
}
void CGUIBase::SetLocationI( int x, int y ) 
{ 
	m_position.SetXY(x,y);
	m_bNeedUpdate=true;
	SetDirty(true);
}

void CGUIBase::SetLocation(int x, int y )
{
	SetLocationI(x,y);
	UpdateParentRect();
}
void CGUIBase::SetSizeI(int width, int height )
{
	m_position.SetSize(width,height);
	m_bNeedUpdate=true; 
	SetDirty(true);
}
void CGUIBase::SetSize( int width, int height ) 
{ 
	SetSizeI(width,height);
	UpdateParentRect();
}

void CGUIBase::SetWidthI(int width )
{
	m_position.SetWidth(width);
	m_bNeedUpdate=true; 
	SetDirty(true);
}
void CGUIBase::SetWidth( int width ) 
{ 
	SetWidthI(width);
	UpdateParentRect();
}

void CGUIBase::SetHeightI(int height )
{
	m_position.SetHeight(height);
	m_bNeedUpdate=true; 
	SetDirty(true);
}

void CGUIBase::SetHeight( int height ) 
{ 
	SetHeightI(height);
	UpdateParentRect();
}
void CGUIBase::SetPositionI(int left,int top, int right, int bottom)
{
	SetRect(&m_position.rect,left,top,right,bottom);
	m_bNeedUpdate=true;
	SetDirty(true);
}

void CGUIBase::SetPositionI(const CGUIPosition& position)
{
	m_position = position;
}

void CGUIBase::SetPosition(int left,int top, int right, int bottom)
{
	SetPositionI(left,top,right,bottom);
	UpdateParentRect();
}

void CGUIBase::SetVisible(bool visible)
{
	m_bNeedUpdate=m_bNeedUpdate | (m_bIsVisible^visible);
	m_bIsVisible=visible;

	//if the control is on the edge of the parent, need to recalculate childrect
	if (m_bNeedUpdate)
	{
		if(m_parent)
		{
			m_parent->SetDirty(true);
			if (visible){
				m_parent->UpdateClientRect(m_position);
			}else
				m_parent->UpdateClientRect(m_position,true);
		}
	}
}

 
bool CGUIBase::OnDragBegin(int MouseState,int X, int Y)
{
	STRUCT_DRAG_AND_DROP *pdrag=&IObjectDrag::DraggingObject;
	if(pdrag->pDragging != this || pdrag->m_bIsCandicateOnly)
	{
		BeginDrag(MouseState, X, Y, (int)m_position.GetDepth());
		string code=GenerateOnMouseScript(MouseState,X,Y);
		code+="dragging_source=\"";
		code+=((CGUIBase*)IObjectDrag::DraggingObject.pSource)->GetName();
		code+="\";";
		code+="dragging_control=\"";
		code+=((CGUIBase*)IObjectDrag::DraggingObject.pDragging)->GetName();
		code+="\";";
		ActivateScript(code,EM_MOUSE_DRAGBEGIN);
		return true;
	}
	else
		return false;
}
void CGUIBase::BeginDrag(int nEvent, int x, int y, int z)
{
	CGUIRoot *root=CGUIRoot::GetInstance();
	STRUCT_DRAG_AND_DROP *pdrag=&IObjectDrag::DraggingObject;
	if(pdrag->pDragging!=this || pdrag->m_bIsCandicateOnly)
	{
		pdrag->m_bIsCandicateOnly = false;
		pdrag->nEvent=nEvent;
		// keep the parent, so that a drag can be restored.
		pdrag->pSource=m_parent;
		// keep the start position
		pdrag->startX = x;
		pdrag->startY = y;
		// this is object being dragged. 
		pdrag->pDragging=this;
		// save old position
		pdrag->SetOldPosition(m_position);
		// keep relative position.
		CGUIPosition vpos;
		GetAbsolutePosition(&vpos,&m_position);
		Reposition("_lt", 0,0,vpos.rect.right-vpos.rect.left, vpos.rect.bottom-vpos.rect.top);
		// detach this object, and let the GUIRoot to manage it as a special global singleton GUI object without parent
		root->DetachGUIElement(this);

		pdrag->nRelativeX=vpos.rect.left-x;
		pdrag->nRelativeY=vpos.rect.top-y;
		pdrag->nRelativeZ=(int)vpos.GetDepth()-z;

	}
}


bool CGUIBase::OnDragEnd(int MouseState,int X, int Y)
{
	CGUIRoot *root=CGUIRoot::GetInstance();
	STRUCT_DRAG_AND_DROP *pdrag=&IObjectDrag::DraggingObject;
	if (pdrag->pSource && m_event &&((m_event->IsMapTo(MouseState,EM_MOUSE_LEFT)&&m_event->IsMapTo(pdrag->nEvent,EM_MOUSE_LEFT))
		||(m_event->IsMapTo(MouseState,EM_MOUSE_RIGHT)&&m_event->IsMapTo(pdrag->nEvent,EM_MOUSE_RIGHT))
		||(m_event->IsMapTo(MouseState,EM_MOUSE_MIDDLE)&&m_event->IsMapTo(pdrag->nEvent,EM_MOUSE_MIDDLE)))) {
		string postcode="dragging_source=nil;dragging_control=nil;dragging_destination=nil;";
		string precode=GenerateOnMouseScript(MouseState,X,Y);

		if (pdrag->pDragging!=this) {
			precode+=("dragging_destination=\""+GetName()+"\";");
		}
		precode+=("dragging_control=\""+((CGUIBase*)pdrag->pDragging)->GetName()+"\";");
		precode+=("dragging_dragging_source=\""+((CGUIBase*)pdrag->pSource)->GetName()+"\";");
		ActivateScript(precode,postcode,EM_MOUSE_DRAGEND);
		EndDrag(MouseState, X, Y,(int) m_position.GetDepth());
	}
	return true;
}

void CGUIBase::EndDrag(int nEvent, int x, int y, int z)
{
	CGUIRoot *root=CGUIRoot::GetInstance();
	STRUCT_DRAG_AND_DROP *pdrag=&IObjectDrag::DraggingObject;
	CGUIPosition tempPos;
	//If this is the object who receives the drag, set the dragging object to be the child of this object if possible.
	if (pdrag->pDragging!=this) {
		if (((CGUIType*)this->GetType())->IsContainer()) {
			GetAbsolutePosition(&tempPos,&m_position);
			SetRect(&((CGUIBase*)pdrag->pDragging)->m_position.rect,((CGUIBase*)pdrag->pDragging)->m_position.rect.left-tempPos.rect.left,\
				((CGUIBase*)pdrag->pDragging)->m_position.rect.top-tempPos.rect.top,((CGUIBase*)pdrag->pDragging)->m_position.rect.right-tempPos.rect.left,\
				((CGUIBase*)pdrag->pDragging)->m_position.rect.bottom-tempPos.rect.top);
			root->AttachGUIElement(this,((CGUIBase*)pdrag->pDragging));
		}else if (this->m_parent) {
			m_parent->GetAbsolutePosition(&tempPos,&m_parent->m_position);
			SetRect(&((CGUIBase*)pdrag->pDragging)->m_position.rect,((CGUIBase*)pdrag->pDragging)->m_position.rect.left-tempPos.rect.left,\
				((CGUIBase*)pdrag->pDragging)->m_position.rect.top-tempPos.rect.top,((CGUIBase*)pdrag->pDragging)->m_position.rect.right-tempPos.rect.left,\
				((CGUIBase*)pdrag->pDragging)->m_position.rect.bottom-tempPos.rect.top);
			root->AttachGUIElement(this->m_parent,((CGUIBase*)pdrag->pDragging));
		}
		if (pdrag->pCleanUp!=NULL) {
			pdrag->pCleanUp();
		}
		pdrag->init();
#ifdef USE_DIRECTX_RENDERER
		root->m_pMouse->Update();
#endif

	}
}

bool CGUIBase::OnSize()
{
	if( !HasEvent(EM_WM_SIZE) )
		return false;
	return ActivateScript("", EM_WM_SIZE);
}

bool CGUIBase::OnDestroy()
{
	if( !HasEvent(EM_WM_DESTROY) )
		return false;

	// Fixed Xizhi: we do not call ActivateScript, because we do not wants to pass this pointer, since the this pointer is already not valid. 
	// return ActivateScript("", EM_WM_DESTROY);
	int etype = EM_WM_DESTROY;
	CGUIRoot *root=CGUIRoot::GetInstance();
	const SimpleScript *tempScript=GetEventScript(etype);
	if (tempScript) 
	{
		// OUTPUT_LOG1("CGUIBase::OnDestroy %s, scode: %s\n", GetName().c_str(),tempScript->szCode.c_str());
		root->m_scripts.AddScript(tempScript->szFile,etype,tempScript->szCode,NULL);
		return true;
	}
	return false;
}


bool CGUIBase::OnClick(int MouseState, int X, int Y)
{
	if( !HasEvent(EM_MOUSE_CLICK) )
		return false;
	// onclick event will have control id perpended from now on.
	string script="id=";
	char tmp[30];
	ParaEngine::StringHelper::fast_itoa(GetID(), tmp,30);
	script+=tmp;
	script+=";";
	script+=GenerateOnMouseScript(MouseState,X,Y);
	return ActivateScript(script,EM_MOUSE_CLICK);
}

CGUIBase* ParaEngine::CGUIBase::GetWindow()
{
	if (!HasEvent(EM_WM_ACTIVATE))
	{
		auto pParent = GetParent();
		return (pParent) ? pParent->GetWindow() : NULL;
	}
	else
		return this;
}

void ParaEngine::CGUIBase::MakeActivate(int nState)
{
	if (nState > 0)
	{
		CGUIRoot::GetInstance()->SetActiveWindow(GetWindow());
	}
}

bool ParaEngine::CGUIBase::OnActivate( int nState )
{
	if( !HasEvent(EM_WM_ACTIVATE) )
		return false;
	// onclick event will have control id perpended from now on.
	string script="id=";
	char tmp[30];
	ParaEngine::StringHelper::fast_itoa(GetID(), tmp, 30);
	script+=tmp;
	script+=";param1=";
	ParaEngine::StringHelper::fast_itoa(nState, tmp, 30);
	script+=tmp;
	script+=";";
	return ActivateScript(script,EM_WM_ACTIVATE);
}


bool CGUIBase::OnDoubleClick(int MouseState, int X, int Y)
{
	if( !HasEvent(EM_MOUSE_DBCLICK) )
		return false;
	string script="id=";
	char tmp[30];
	ParaEngine::StringHelper::fast_itoa(GetID(), tmp,30);
	script+=tmp;
	script+=";";
	script+=GenerateOnMouseScript(MouseState,X,Y);
	return ActivateScript(GenerateOnMouseScript(MouseState,X,Y),EM_MOUSE_DBCLICK);

}

bool CGUIBase::OnMouseDown(int MouseState, int X, int Y)
{
	if( !HasEvent(EM_MOUSE_DOWN))
		return false;
	return ActivateScript(GenerateOnMouseScript(MouseState,X,Y),EM_MOUSE_DOWN);
}

bool CGUIBase::OnDragOver(int MouseState,int X, int Y)
{
	if( !HasEvent(EM_MOUSE_DRAGOVER) )
		return false;
	return ActivateScript(GenerateOnMouseScript(MouseState,X,Y),EM_MOUSE_DRAGOVER);
}

bool CGUIBase::OnTouch(const TouchEvent& touch)
{
	if (!HasEvent(EM_TOUCH))
		return false;
	std::string sCode = touch.ToScriptCode();
	return ActivateScript(sCode, EM_TOUCH);
}

bool CGUIBase::OnMouseUp(int MouseState, int X, int Y)
{
	if( !HasEvent(EM_MOUSE_UP) )
		return false;
	return ActivateScript(GenerateOnMouseScript(MouseState,X,Y),EM_MOUSE_UP);
}

bool CGUIBase::OnMouseEnter()
{
	int nHotSpotX = -1;
	int nHotSpotY = -1;
	const std::string& sCursorFile = GetCursor(&nHotSpotX, &nHotSpotY);
	if(!sCursorFile.empty())
	{
#ifdef USE_DIRECTX_RENDERER
		CDirectMouse *pMouse=CGUIRoot::GetInstance()->m_pMouse;
		if(pMouse)
		{
			pMouse->SetCursorFromFile(sCursorFile.c_str(), nHotSpotX, nHotSpotY);
		}
#endif
	}
	// Only set mouse over to true, if all parent and this control is enabled. 
	// m_bMouseOver=true;
	{
		CGUIBase * pObj = this;
		bool bIsEnabled = true;
		while(pObj!=0 && bIsEnabled)
		{
			bIsEnabled = pObj->GetEnabled();
			pObj = pObj->GetParent();
		}
		m_bMouseOver=bIsEnabled;
	}
	if( !HasEvent(EM_MOUSE_ENTER) )
		return false;
	return ActivateScript("",EM_MOUSE_ENTER);
}


bool CGUIBase::OnMouseLeave()
{
	m_bMouseOver=false;
	if( !HasEvent(EM_MOUSE_LEAVE) )
		return false;
	return ActivateScript("",EM_MOUSE_LEAVE);
}


bool CGUIBase::OnMouseHover(int MouseState, int X, int Y)
{
	if( !HasEvent(EM_MOUSE_HOVER) )
		return false;
	return ActivateScript(GenerateOnMouseScript(MouseState,X,Y),EM_MOUSE_HOVER);
}


bool CGUIBase::OnMouseWheel(int Delta, int X, int Y)
{
	if( !HasEvent(EM_MOUSE_WHEEL) )
		return false;
	char ctemp[40];
	string code="mouse_wheel=";
	ParaEngine::StringHelper::fast_itoa(Delta,ctemp,40);
	code+=ctemp;code+=";";
	code+="mouse_x=";
	ParaEngine::StringHelper::fast_itoa(X,ctemp,40);
	code+=ctemp;code+=";";
	code+="mouse_y=";
	ParaEngine::StringHelper::fast_itoa(Y,ctemp,40);
	code+=ctemp;code+=";";
	// OUTPUT_LOG("OnMouseWheel: delta %d\n", Delta);
	return ActivateScript(code,EM_MOUSE_WHEEL);
}


bool CGUIBase::OnMouseMove(int MouseState, int X, int Y)
{
	if( !HasEvent(EM_MOUSE_MOVE) )
		return false;
	return ActivateScript(GenerateOnMouseScript(0/*MouseState UI Mouse move will generate absolute mouse position*/,X,Y),EM_MOUSE_MOVE);
}


ParaEngine::QPoint ParaEngine::CGUIBase::GetCompositionPoint()
{
	// we need to set default ime window position if our d3d based ime system is not enabled. 
	RECT rcWindow = m_objResource->GetDrawingRects(0);
	int x = rcWindow.left;
	int y = rcWindow.top;
	int width = rcWindow.right - rcWindow.left;
	int height = rcWindow.bottom - rcWindow.top;

	CGlobals::GetApp()->GameToClient(x, y);
	CGlobals::GetApp()->GameToClient(width, height);

	float fScaleX = 1.f;
	float fScaleY = 1.f;
	CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);

	if (fScaleX != 1.f)
	{
		x = (int)((float)x * fScaleX);
		width = (int)((float)width * fScaleX);
	}

	if (fScaleY != 1.f)
	{
		y = (int)((float)y * fScaleY);
		height = (int)((float)height * fScaleY);
	}
	return QPoint(x, y + height);
}

void ParaEngine::CGUIBase::SetCompositionPoint(const QPoint& point)
{
	if (CGUIRoot::GetInstance()->GetIMEFocus() == this)
	{
		QPoint pt = GetCompositionPoint();
		CGUIIME::SetCompositionPosition(pt.x(), pt.y());
	}
}

bool ParaEngine::CGUIBase::IsNonClientTestEnabled()
{
	return false;
}

void ParaEngine::CGUIBase::EnableNonClientTest(bool val)
{

}

bool CGUIBase::OnFocusIn()
{
	if (m_bInputMethodEnabled)
	{
		CGUIRoot::GetInstance()->SetIMEFocus(this);
		CGUIIME::OnFocusIn();

		bool bIMEEnabled = CGUIIME::IsEnableImeSystem();
		if (!CGlobals::GetApp()->IsWindowedMode() && !bIMEEnabled)
		{
			CGUIIME::EnableImeSystem(true);
		}
		if (!bIMEEnabled)
		{
			QPoint pt = GetCompositionPoint();
			CGUIBase::SetCompositionPoint(pt);
		}
	}

	if( !HasEvent(EM_CTRL_FOCUSIN) )
		return false;
	return ActivateScript("",EM_CTRL_FOCUSIN);
}


bool CGUIBase::OnFocusOut()
{
	if (m_bInputMethodEnabled)
	{
		CGUIIME::OnFocusOut();
		if (CGUIRoot::GetInstance()->GetIMEFocus() == this)
		{
			CGUIRoot::GetInstance()->SetIMEFocus(NULL);
		}
	}

	if( !HasEvent(EM_CTRL_FOCUSOUT) )
		return false;
	return ActivateScript("",EM_CTRL_FOCUSOUT);
}


bool ParaEngine::CGUIBase::IsInputMethodEnabled() const
{
	return m_bInputMethodEnabled;
}

void ParaEngine::CGUIBase::SetInputMethodEnabled(bool val)
{
	if (m_bInputMethodEnabled != val)
	{
		m_bInputMethodEnabled = val;
		if (HasFocus()) 
		{
			if (!m_bInputMethodEnabled)
			{
				CGUIIME::OnFocusOut();
				if (CGUIRoot::GetInstance()->GetIMEFocus() == this)
				{
					CGUIRoot::GetInstance()->SetIMEFocus(NULL);
				}
			}
			else
			{
				CGUIRoot::GetInstance()->SetIMEFocus(this);
				CGUIIME::OnFocusIn();

				bool bIMEEnabled = CGUIIME::IsEnableImeSystem();
				if (!CGlobals::GetApp()->IsWindowedMode() && !bIMEEnabled)
				{
					CGUIIME::EnableImeSystem(true);
				}
				if (!bIMEEnabled)
				{
					QPoint pt = GetCompositionPoint();
					CGUIBase::SetCompositionPoint(pt);
				}
			}
		}
	}
}

bool CGUIBase::OnModify()
{
	if( !HasEvent(EM_CTRL_MODIFY) )
		return false;
	return ActivateScript(GenerateOnKeyboardScript(CGUIEvent::IGNORE_UP_EVENTS),EM_CTRL_MODIFY);
}

bool CGUIBase::OnFrameMove(float fDeltaTime)
{
	if(fDeltaTime>0.001f)
	{
		int nTime = GetLifeTime();
		if (nTime<0) 
		{
			// permanent object
		}
		else if(nTime>0)
		{
			SetLifeTime(max(0, nTime-(int)(fDeltaTime*1000)));
		}
		else if (GetLifeTime()==0) 
		{
			CGUIRoot::GetInstance()->PostDeleteGUIObject(this);
			return true;
		}

		if( !HasEvent(EM_CTRL_FRAMEMOVE) )
			return false;
		char buf[256];
		snprintf(buf, 255, "deltatime=%f;", fDeltaTime);
		return ActivateScript(buf,EM_CTRL_FRAMEMOVE);
	}
	return true;
}

bool CGUIBase::OnChange(const char* code)
{
	if( !HasEvent(EM_CTRL_CHANGE) )
		return false;
	// mod by LiXizhi, 2006.10.31. Ensure that there is only one OnChange event in the event pool.
	if(CGlobals::GetGUI()->m_scripts.GetScript(EM_CTRL_CHANGE, this)==NULL)
	{
		if(code)
			return ActivateScript(code,EM_CTRL_CHANGE);
		else
			return ActivateScript("",EM_CTRL_CHANGE);
	}
	else
		return true;
}

bool CGUIBase::OnSelect()
{
	if( !HasEvent(EM_CTRL_SELECT) )
		return false;
	return ActivateScript("",EM_CTRL_SELECT);
}

bool CGUIBase::OnKeyDown()
{
	if( !HasEvent(EM_CTRL_KEYDOWN) )
		return false;
	char buf[256];
	snprintf(buf, 255, "virtual_key=%d;", m_event->m_keyboard.nAlterKey);
	return ActivateScript(buf,EM_CTRL_KEYDOWN);
}


bool CGUIBase::OnKeyUp()
{
	if( !HasEvent(EM_CTRL_KEYUP) )
		return false;
	char buf[256];
	snprintf(buf, 255, "virtual_key=%d;", m_event->m_keyboard.nAlterKey);
	return ActivateScript(buf, EM_CTRL_KEYUP);
}

void CGUIBase::Focus()
{
	if (CanHaveFocus()) {
		if (m_parent!=NULL) {
			m_parent->SetKeyFocus(this);	
		}else{
			CGUIRoot::GetInstance()->SetKeyFocus(this);
		}
	}else{
		if (m_parent!=NULL) {
			m_parent->SetKeyFocus(NULL);	
		}else{
			CGUIRoot::GetInstance()->SetKeyFocus(NULL);
		}
	}
}

void CGUIBase::LostFocus()
{
	m_bHasFocus = false;
	CGUIContainer * pParent = m_parent;
	while(pParent)
	{
		pParent->SetKeyFocus(NULL);	
		pParent->SetMouseFocus(NULL);	
		pParent = pParent->GetParent();
	}
	CGUIRoot::GetInstance()->SetKeyFocus(NULL);
	CGUIRoot::GetInstance()->SetMouseFocus(NULL);
}

bool ParaEngine::CGUIBase::HasFocus() const
{
	return m_bHasFocus;
}

void CGUIBase::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	Reposition(alignment, x,y,width, height);

	SetName(strObjectName);
	if (!m_objResource) {
		m_objResource=new CGUIResource();
		m_objResource->SetActiveLayer();
	}
	m_nID = GenerateUIObj_ID();

	CGlobals::GetGUI()->AddToNameMap(GetName(), this);
	CGlobals::GetGUI()->AddToIDMap(GetID(), this);
}

void CGUIBase::Reposition( const char* alignment, int x,int y, int width, int height )
{
	m_position.Reposition(alignment, x, y, width, height);
}

bool CGUIBase::MsgProc(MSG *event)
{
	//one call to this function will trigger at most one event
	if( !m_bIsEnabled || m_event==0)
		return false;
	bool bHandled=false;
	CGUIRoot *pRoot=CGUIRoot::GetInstance();
	CDirectMouse *pMouse=pRoot->m_pMouse;
	CDirectKeyboard *pKeyboard=pRoot->m_pKeyboard;
	STRUCT_DRAG_AND_DROP *pdrag=&IObjectDrag::DraggingObject;
	MSG newMsg;
	if (event!=NULL&& !m_event->InterpretMessage(event)) {
		return false;
	}
	POINT pt;
	pt.x=m_event->m_mouse.x;
	pt.y=m_event->m_mouse.y;
	int nEvent=m_event->GetTriggerEvent();
	CEventBinding::InitMsg(&newMsg,event->time,nEvent,pt);
	if (m_bIsVisible){
		if (nEvent==EM_NONE) {
			return false;
		}

		if (m_event->IsMapTo(nEvent,EM_CTRL_CAPTUREMOUSE)) {
			if(CanCaptureMouse()){
				pKeyboard->SetCapture(this);
			}
			newMsg.message=EM_CTRL_FOCUSIN;
			CGUIBase::MsgProc(&newMsg);
			bHandled=true;
		}
		else if (m_event->IsMapTo(nEvent,EM_CTRL_RELEASEMOUSE)) {
			pKeyboard->ReleaseCapture(this);
			bHandled=true;
		}

		if (m_event->IsMapTo(nEvent,EM_MOUSE_MOVE)) {
			bHandled= OnMouseMove(nEvent,m_event->m_mouse.x,m_event->m_mouse.y) || GetCandrag() || HasEvent(EM_MOUSE_CLICK);
		}else if (m_event->IsMapTo(nEvent,EM_MOUSE_DOWN)) {
			bHandled=OnMouseDown(nEvent,m_event->m_mouse.x,m_event->m_mouse.y) || GetCandrag() || HasEvent(EM_MOUSE_CLICK) || HasEvent(EM_MOUSE_UP);
		}else if (m_event->IsMapTo(nEvent,EM_MOUSE_UP)) {
			bHandled=OnMouseUp(nEvent,m_event->m_mouse.x,m_event->m_mouse.y) || GetCandrag() || HasEvent(EM_MOUSE_CLICK);
		}else if (m_event->IsMapTo(nEvent,EM_MOUSE_CLICK)) {
			bHandled=OnClick(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);
		}else if (m_event->IsMapTo(nEvent,EM_MOUSE_DBCLICK)) {
			bHandled=OnDoubleClick(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);
		}else if (m_event->IsMapTo(nEvent,EM_CTRL_SELECT)) {
			bHandled=OnSelect();
		}else if (m_event->IsMapTo(nEvent,EM_CTRL_CHANGE)) {
			bHandled=OnChange();
		}else if (m_event->IsMapTo(nEvent,EM_CTRL_MODIFY)) {
			bHandled=OnModify();
		}else if (m_event->IsMapTo(nEvent,EM_MOUSE_WHEEL)) {
			// modified 2007.10.12 LXZ: so that the message can be leaked to its container during scrolling
			int nDelta = ((int32)(event->lParam)) / 120;
			if (nDelta == 0)
				nDelta = ((int32)(event->lParam)) > 0 ? 1 : -1;
			bHandled = OnMouseWheel(nDelta, m_event->m_mouse.x, m_event->m_mouse.y);
		}
		if (m_event->IsMapTo(nEvent,EM_CTRL_FOCUSIN)) {
			//by default, we map the Left click to focus in.
			//only left button down on other key focus object will give it focus
			if (CanHaveFocus()) {
				if (m_parent!=NULL) {
					m_parent->SetKeyFocus(this);	
				}else{
					pRoot->SetKeyFocus(this);
				}
			}else{
				if (m_parent!=NULL) {
					m_parent->SetKeyFocus(NULL);	
				}else{
					pRoot->SetKeyFocus(NULL);
				}
			}
			bHandled=true;
		}
		if (m_event->IsMapTo(nEvent,EM_MOUSE_DRAGBEGIN)) {
			// clean up so that the scripting interface can add or remove them on demand. 
			pdrag->CleanUpReceivers();
			OnDragBegin(nEvent,m_event->m_mouse.LastLDown.x,m_event->m_mouse.LastLDown.y);
			bHandled=true;
		}else if (m_event->IsMapTo(nEvent,EM_MOUSE_DRAGEND)) {
			// Both the object being dragged and the object who receives drags will receive the drag end event.
			if (pdrag->pDragging==this) 
			{ 
				//the dragging object should receive the event first
				OnDragEnd(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);

				//send the drag end message to the object the cursor is currently on if it can receive the drag.
				CGUIBase *pMouseTarget = pRoot->GetUIObject(m_event->m_mouse.x,m_event->m_mouse.y);
				if(pMouseTarget && pdrag->HasReceiver(pMouseTarget->GetName().c_str()))
				{
					pMouseTarget->OnDragEnd(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);
				}
				else if (pdrag->HasReceiver("root")) 
				{
					pRoot->OnDragEnd(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);
				}
				else
				{
					if(IObjectDrag::CancelDrag(pdrag))
					{
					}
				}
				// now release mouse capture
				pKeyboard->ReleaseCapture(this);
			}
			bHandled=true;
		}else if (m_event->IsMapTo(nEvent,EM_MOUSE_DRAGOVER)) {
			if (pdrag->pDragging!=this) {
				OnDragOver(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);
				bHandled=true;
			}else{
				//the dragging object should receive the event first
				OnDragOver(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);
				//send the dragend message to the object the cursor is currently on
				CGUIBase *pMouseTarget;
				if ((pMouseTarget=pRoot->GetUIObject(m_event->m_mouse.x,m_event->m_mouse.y))==NULL) {
					MSG msg = m_event->GenerateMessage();
					pRoot->MsgProc(&msg);
				}else{
					MSG msg = m_event->GenerateMessage();
					pMouseTarget->MsgProc(&msg);
				}
				bHandled=true;
			}
		}
	}

	//trigger all the key events
	if (m_event->IsMapTo(nEvent,EM_CTRL_UPDATEKEY)) {
		m_event->UpdateKey(CGUIEvent::REPEAT_PRESSED_KEYS);
		for (DWORD i=0;i<m_event->m_keyboard.Size;i++) {
			newMsg.message=(UINT)m_event->m_keyboard.KeyEvents[i].uAppData;
			newMsg.wParam=(WPARAM)m_event->m_keyboard.KeyEvents[i].dwOfs;
			newMsg.time = (DWORD)m_event->m_keyboard.KeyEvents[i].dwTimeStamp;
			newMsg.lParam = (LPARAM)m_event->m_keyboard.KeyEvents[i].dwData;
			newMsg.hwnd=(HWND)1;
			//update the key state according to the events
			CGUIEvent::KeyStates[newMsg.message]=(byte)(newMsg.lParam);
			//if the key is not processed, we will buffer it in a queue and sent it to the 3d scene
			if (!MsgProc(&newMsg)) {
				CGUIRoot::GetInstance()->m_events.push_back(newMsg);
			}
		}
		bHandled=true;
	}
	
	if (m_event->IsMapTo(nEvent,EM_CTRL_FRAMEMOVE)) 
	{
		//detect up event lost and resend the up event.
		/*if (!pMouse->IsButtonDown(CDirectMouse::LEFT_BUTTON)) {
			if (0!=(CGUIEvent::KeyStates[EM_MOUSE_LEFTDOWN]&0x80)) {
				CEventBinding::InitMsg(&newMsg,event->time,EM_MOUSE_LEFTUP,event->pt);
				MsgProc(&newMsg);
				CGUIEvent::KeyStates[EM_MOUSE_LEFTDOWN]=0;
			}
		}
		if (!pMouse->IsButtonDown(CDirectMouse::RIGHT_BUTTON)) {
			if (0!=(CGUIEvent::KeyStates[EM_MOUSE_RIGHTDOWN]&0x80)) {
				CEventBinding::InitMsg(&newMsg,event->time,EM_MOUSE_RIGHTUP,event->pt);
				MsgProc(&newMsg);
				CGUIEvent::KeyStates[EM_MOUSE_RIGHTDOWN]=0;
			}
		}
		if (!pMouse->IsButtonDown(CDirectMouse::MIDDLE_BUTTON)) {
			if (0!=(CGUIEvent::KeyStates[EM_MOUSE_MIDDLEDOWN]&0x80)) {
				CEventBinding::InitMsg(&newMsg,event->time,EM_MOUSE_MIDDLEUP,event->pt);
				MsgProc(&newMsg);
				CGUIEvent::KeyStates[EM_MOUSE_MIDDLEDOWN]=0;
			}
		}*/
		// by LXZ 2008.2.9. call frame move handler. 
		// OnFrameMove();
		bHandled=true;
	}
	return bHandled;
}


void CGUIBase::UpdateRects()
{
	OnSize();
}

void CGUIBase::Reset()
{
	ClearAllEvent();
	// fixed Xizhi 2009.10.18: why delete all scripts if window loses focus.
	// CGUIRoot::Instance()->m_scripts.DeleteScript(this);
}


bool CGUIBase::HasEvent(int etype)
{
	if(m_event)
		return m_event->GetConstEventBindingObj()->HasEventScript(etype);
	else
		return false;
}
void CGUIBase::SetEventScript(int nEvent,const SimpleScript *script)
{
	if(m_event)
	{
		m_event->GetEventBindingObj()->MapEventToScript(nEvent,script);
		// well the speed of GetEventBindingObj() is almost like following code. 
		// for(int i=0;i<100;++i){OUTPUT_LOG("");}
	}
}

const SimpleScript* CGUIBase::GetEventScript(int nEvent)const
{
	if(m_event)
		return m_event->GetConstEventBindingObj()->GetEventScript(nEvent);
	else
		return NULL;
}

bool CGUIBase::ActivateScript(const string &code, int etype)
{
	CGUIRoot *root=CGUIRoot::GetInstance();
	const SimpleScript *tempScript=GetEventScript(etype);
	if (tempScript) {
		root->m_scripts.AddScript(tempScript->szFile,etype,code+tempScript->szCode,this);
		return true;
	}
	return false;
}


bool CGUIBase::ActivateScript(const string &precode,const string &postcode, int etype)
{
	CGUIRoot *root=CGUIRoot::GetInstance();
	const SimpleScript *tempScript=GetEventScript(etype);
	if (tempScript) {
		root->m_scripts.AddScript(tempScript->szFile,etype,precode+tempScript->szCode+postcode,this);
		return true;
	}
	return false;
}

void CGUIBase::SetName(const char *szName)
{
	if (szName==NULL) return;
	m_sIdentifer=szName;

	map<string,CGUIBase*>::iterator iter;
	CGUIRoot *root=CGUIRoot::GetInstance();
	if((iter=root->m_namemap.find(m_sIdentifer))!=root->m_namemap.end())
		root->m_namemap.erase(iter);
	root->m_namemap[m_sIdentifer]=this;
}

GUITextureElement* CGUIBase::GetTextureElement(const char *texturename)
{
	return m_objResource ? m_objResource->GetTextureElement(texturename) : NULL;
}

GUIFontElement* CGUIBase::GetFontElement(const char *fontname)
{
	return m_objResource ? m_objResource->GetFontElement(fontname) : NULL;
}

GUIFontElement* CGUIBase::GetFontElement(int nIndex)
{
	return m_objResource ? m_objResource->GetFontElement(nIndex) : NULL;
}

void CGUIBase::Begin(GUIState* pGUIState,float fElapsedTime )
{
	if (m_bNeedUpdate){
		UpdateRects();
	}
	CDirectMouse *pMouse=CGUIRoot::GetInstance()->m_pMouse;
	POINT pt;
	pt.x=pMouse->m_x;
	pt.y=pMouse->m_y;

	if(m_bAlwaysMouseOver)
	{
		if (m_objResource && ContainsPoint(m_objResource->GetDrawingRects(0), pt))
		{
			// fixed by LiXizhi 2008.7.19. Only mouse over if all parent and this control is enabled. 
			CGUIBase * pObj = this;
			bool bIsEnabled = true;
			while(pObj!=0 && bIsEnabled)
			{
				bIsEnabled = pObj->GetEnabled();
				pObj = pObj->GetParent();
			}
			m_bMouseOver=bIsEnabled;
		}else
		{
			m_bMouseOver=false;
		}
	}

	if( m_bIsVisible == false )
		return ;

	if (m_position.IsRelativeTo3DObject()) 
	{
		BeginDepthTest(pGUIState);
		UpdateRects();
	}
	if(IsUVWrappingEnabled())
	{
		if(pGUIState)
		{
			GetPainter(pGUIState)->Flush();
			pGUIState->pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP);
			pGUIState->pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );
		}
	}
}

void CGUIBase::End(GUIState* pGUIState,float fElapsedTime )
{
	if( m_bIsVisible == false )
		return ;

	if(m_eHighlight!=0)
	{
		CGUIHighlightManager* gm=&CSingleton<CGUIHighlightManager>::Instance();
		RECT rc = m_objResource->GetDrawingRects(0);
		gm->AddNewInstance(pGUIState,rc,fElapsedTime,m_eHighlight);
	}

	if (m_position.IsRelativeTo3DObject()) 
	{
		EndDepthTest(pGUIState);
	}

	if(IsUVWrappingEnabled())
	{
		// reset to default
		if(pGUIState)
		{
			GetPainter(pGUIState)->Flush();
			pGUIState->pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP);
			pGUIState->pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP);
		}
	}
}


BOOL CGUIBase::IsOnObject(int x,int y, int z)
{
	return IsPointInControl(x,y);
}

BOOL CGUIBase::IsPointInControl( int x, int y )
{
	static CGUIPosition pos;
	GetAbsolutePosition(&pos, GetPosition());
	return ( (pos.rect.left<=x && pos.rect.top<=y && pos.rect.right>=x && pos.rect.bottom>=y));
}

float	CGUIBase::GetRotation()
{
	return m_fRotation;
}
void	CGUIBase::SetRotation(float fRot)
{
	m_fRotation = fRot;
	SetDirty(true);
}
void CGUIBase::SetRotOriginOffset(const Vector2& in)
{
	m_vRotOriginOffset = in;
	SetDirty(true);
}

void CGUIBase::GetRotOriginOffset(Vector2* pOut)
{
	if(pOut)
		*pOut = m_vRotOriginOffset;
}
void ParaEngine::CGUIBase::SetScaling( const Vector2& in )
{
	m_vScaling = in;
	SetDirty(true);
}

void ParaEngine::CGUIBase::GetScaling( Vector2* pOut )
{
	if(pOut)
		*pOut = m_vScaling;
}

void ParaEngine::CGUIBase::SetTranslation( const Vector2& in )
{
	m_vTranslation = in;
	SetDirty(true);
}

void ParaEngine::CGUIBase::GetTranslation( Vector2* pOut )
{
	if(pOut)
		*pOut = m_vTranslation;
}

void ParaEngine::CGUIBase::SetColorMask( DWORD dwColor )
{
	m_dwColorMask = dwColor;
	SetDirty(true);
}

DWORD ParaEngine::CGUIBase::GetColorMask()
{
	return m_dwColorMask;
}

void ParaEngine::CGUIBase::UpdateParentRect()
{
	if (m_parent){
		m_parent->UpdateClientRect(m_position);
	}
}

// predefined texture elements
static GUITextureElement s_predefined_elements[ParaEngine::CGUIBase::TexElement_Last];

GUITextureElement* ParaEngine::CGUIBase::GetPredefinedTextureElement(EnumTexElementPredefined nID)
{
	GUITextureElement& element = s_predefined_elements[nID];
	if (element.name.empty())
	{
		element.name = "initialized";
		TextureEntity* pWhiteTex = NULL;
		if (nID == TexElement_White)
			pWhiteTex = CGlobals::GetAssetManager()->GetDefaultTexture(0);
		element.SetElement(pWhiteTex, 0);
	}
	return &element;
}

void ParaEngine::CGUIBase::ClearPredefinedTextureElement()
{
	for (int i = 0; i < TexElement_Last; ++i)
	{
		s_predefined_elements[i].SetElement(NULL);
	}
}

HRESULT ParaEngine::CGUIBase::DrawRect(Color color, RECT *prcDest, RECT* prcWindow /*= NULL*/, GUIState* pGUIState/* = NULL*/)
{
	if (prcDest != 0)
	{
		GUITextureElement* pElement = GetPredefinedTextureElement(TexElement_White);
		if (pElement)
		{
			auto oldColor = pElement->TextureColor;
			pElement->TextureColor = color;
			HRESULT hr = DrawElement(pElement, prcDest, prcWindow);
			pElement->TextureColor = oldColor;
			return hr;
		}
	}
	return E_FAIL;
}

HRESULT CGUIBase::DrawElement(GUITextureElement* pElement, RECT *prcDest, RECT* prcWindow, GUIState* pGUIState/* = NULL*/)
{
	if(prcDest != 0 && pElement!=0)
	{
		float fRot = GetRotation();
		
		Vector2 vTrans(0.f, 0.f);
		GetTranslation(&vTrans);
		BOOL UseTrans = (vTrans!=Vector2::ZERO);

		Vector2 vScaling(1.f, 1.f);
		GetScaling(&vScaling);
		BOOL UseScaling =(vScaling != Vector2::UNIT_SCALE);

		if(fRot == 0.f &&  !UseTrans && !UseScaling && (m_dwColorMask==0xFFFFFFFF))
			return GetPainter(pGUIState)->DrawSprite(pElement, prcDest, m_position.GetDepth());
		else
		{
			if(prcWindow==0)
				prcWindow = prcDest;
			Vector2 vOrigin((prcWindow->left+prcWindow->right)/2.0f, (prcWindow->top+prcWindow->bottom)/2.0f);
			Vector2 vOffset(0,0);
			GetRotOriginOffset(&vOffset);
			vOrigin += vOffset;
			return GetPainter(pGUIState)->DrawSprite(pElement, prcDest, vOrigin, fRot, UseScaling ? (&vScaling) : NULL, UseTrans ? (&vTrans) : NULL, (m_dwColorMask == 0xFFFFFFFF) ? NULL : &m_dwColorMask, m_position.GetDepth());
		}
	}
	return E_FAIL;
}

HRESULT CGUIBase::DrawText(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, RECT* prcWindow, bool bShadow, int nCount, int shadowQuality, Color shadowColor, GUIState* pGUIState/* = NULL*/)
{
	if(strText[0] == '\0')
		return S_OK;
	
	if (prcDest != 0 && pElement != 0 && pElement->FontColor.a > 0)
	{
		float fRot = GetRotation();
		Vector2 vTrans(0.f, 0.f);
		GetTranslation(&vTrans);
		BOOL UseTrans = (vTrans!=Vector2::ZERO);

		Vector2 vScaling(1.f, 1.f);
		GetScaling(&vScaling);
		if(GetTextScale() != 1.f)
		{
			float fTextScale = GetTextScale();
			vScaling *= fTextScale;
		}
		BOOL UseScaling =(vScaling != Vector2::UNIT_SCALE);

#ifdef USE_DIRECTX_RENDERER
		if(CGUIRoot::GetInstance()->GetUsePointTextureFiltering())
		{
			GetPainter(pGUIState)->Flush();
			CGlobals::GetEffectManager()->SetSamplerState( 0, D3DSAMP_MINFILTER,  D3DTEXF_LINEAR);
			CGlobals::GetEffectManager()->SetSamplerState( 0, D3DSAMP_MAGFILTER,  D3DTEXF_LINEAR);
		}
#endif

		HRESULT res = S_OK;
		if(fRot == 0.f &&  !UseTrans && !UseScaling && (m_dwColorMask==0xFFFFFFFF))
			res = GetPainter(pGUIState)->DrawText(strText, pElement, prcDest, m_position.GetDepth(), bShadow, nCount, shadowQuality, shadowColor);
		else
		{
			if(prcWindow==0)
				prcWindow = prcDest;
			Vector2 vOrigin((prcWindow->left+prcWindow->right)/2.0f, (prcWindow->top+prcWindow->bottom)/2.0f);
			Vector2 vOffset(0,0);
			GetRotOriginOffset(&vOffset);
			if (GetTextScale() == 1.f || vOffset == Vector2::ZERO)
			{
				vOrigin += vOffset;
				res = GetPainter(pGUIState)->DrawText(strText, pElement, prcDest, vOrigin, fRot, UseScaling ? (&vScaling) : NULL,
					UseTrans ? (&vTrans) : NULL, (m_dwColorMask == 0xFFFFFFFF) ? NULL : &m_dwColorMask, m_position.GetDepth(),
					bShadow, nCount, shadowQuality, shadowColor);
			}
			else
			{
				// both local scaling and global scaling should be applied 
				RECT rcScreen = *prcDest;
				Matrix4 matTransform;
				Vector2 vPos((float)rcScreen.left, (float)rcScreen.top);
				vScaling.x = vScaling.y = GetTextScale();
				{
					// apply local text scale
					vPos -= vOrigin;
					vPos.x *= vScaling.x;
					vPos.y *= vScaling.y;
					vPos += vOrigin;
				}
				{
					// apply global transform.
					GetScaling(&vScaling);
					vOrigin += vOffset;
					vPos -= vOrigin;
					ParaMatrixTransformation2D(&matTransform, NULL, 0.0, &vScaling, NULL, fRot, UseTrans ? (&vTrans) : NULL);
					vPos = vPos*matTransform;
					vPos += vOrigin;
					
					// out, scaling center, scaling rotation, scaling, rotation center, rotation, translation
					vScaling *= GetTextScale();
					ParaMatrixTransformation2D(&matTransform, NULL, 0.0, &vScaling, NULL, fRot, &vPos);
					matTransform._43 = m_position.GetDepth();
				}
				rcScreen.right -= rcScreen.left;
				rcScreen.left = 0;
				rcScreen.bottom -= rcScreen.top;
				rcScreen.top = 0;
				res = GetPainter(pGUIState)->DrawText(strText, pElement, &rcScreen, matTransform, (m_dwColorMask == 0xFFFFFFFF) ? NULL : &m_dwColorMask, m_position.GetDepth(),
					bShadow, nCount, shadowQuality, shadowColor);
			}
		}
		if(CGUIRoot::GetInstance()->GetUsePointTextureFiltering())
		{
#ifdef USE_DIRECTX_RENDERER
			GetPainter(pGUIState)->Flush();
			CGlobals::GetEffectManager()->SetSamplerState( 0, D3DSAMP_MINFILTER,  D3DTEXF_POINT);
			CGlobals::GetEffectManager()->SetSamplerState( 0, D3DSAMP_MAGFILTER,  D3DTEXF_POINT);
#endif
		}

		return res;
	}
	return E_FAIL;
}

void ParaEngine::CGUIBase::SetZOrder( int nOrder )
{
	if(m_parent!=0)
	{
		if(GetZOrder()!=nOrder)
		{
			m_nZOrder = nOrder;
			m_parent->SortChildrenByZOrder();
		}
	}
	else
	{
		m_nZOrder = nOrder;
	}
}

void ParaEngine::CGUIBase::SetCursor( const char* szCursorFile, int nHotSpotX /*= -1*/, int nHotSpotY /*= -1*/ )
{
	if(szCursorFile)
		m_sCursorFile = szCursorFile;
	else
		m_sCursorFile.clear();
	m_nCursorHotSpotX = nHotSpotX;
	m_nCursorHotSpotY = nHotSpotY;
}

const std::string& ParaEngine::CGUIBase::GetCursor( int* pnHotSpotX /*= 0*/, int* pnHotSpotY /*= 0*/ ) const
{
	if(m_sCursorFile.empty() && m_parent)
	{
		return m_parent->GetCursor(pnHotSpotX, pnHotSpotY);
	}
	if(pnHotSpotX)
	{
		*pnHotSpotX = m_nCursorHotSpotX;
	}
	if(pnHotSpotY)
	{
		*pnHotSpotY = m_nCursorHotSpotY;
	}
	return m_sCursorFile;
}

void ParaEngine::CGUIBase::ApplyAnimSelective(const Vector2* pvOrigin, const float* pfRadian, const Vector2* pvScaling, const Vector2* pvTranslation, const DWORD* pdwColor)
{
	if (pdwColor != 0)
		SetColorMask(*pdwColor);
	
	float fRadian;
	if (pfRadian == 0)
		fRadian = GetRotation();
	else
	{
		fRadian = *pfRadian;
		SetRotation(fRadian);
	}

	Vector2 vScaling;
	if (pvScaling == 0)
		GetScaling(&vScaling);
	else
	{
		vScaling = *pvScaling;
		SetScaling(vScaling);
	}
		

	Vector2 vTranslation;
	if (pvTranslation == 0)
		GetTranslation(&vTranslation);
	else
	{
		vTranslation = *pvTranslation;
		SetTranslation(vTranslation);
	}

	if (fRadian == 0.f && (vTranslation == Vector2(0.f, 0.f)) && (vScaling == Vector2(1.f, 1.f)))
	{
		if (pvOrigin == 0)
		{
			SetRotOriginOffset(Vector2(0.f, 0.f));
		}
		pvOrigin = NULL;
	}
	else
	{
		// keep relative position.
		CGUIPosition vpos;
		GetAbsolutePosition(&vpos, &m_position);
		RECT rcRect = vpos.rect;
		Vector2 vMyOrigin;
		vMyOrigin.x = (rcRect.left + rcRect.right) / 2.f;
		vMyOrigin.y = (rcRect.top + rcRect.bottom) / 2.f;

		Vector2 vOrigin;
		if (pvOrigin == 0)
		{
			GetRotOriginOffset(&vOrigin);
			vOrigin += vMyOrigin;
			pvOrigin = &vOrigin;
		}
		else
		{
			SetRotOriginOffset((*pvOrigin) - vMyOrigin);
		}
	}

	if (GetChildren() != 0)
	{
		GUIBase_List_Type::iterator itCurCP, itEndCP = GetChildren()->end();
		for (itCurCP = GetChildren()->begin(); itCurCP != itEndCP; ++itCurCP)
		{
			(*itCurCP)->ApplyAnimSelective(pvOrigin, pfRadian, pvScaling, pvTranslation, pdwColor);
		}
	}
}

void ParaEngine::CGUIBase::ApplyColorMask()
{
	DWORD dwColor;
	dwColor = GetColorMask();
	ApplyAnimSelective(NULL, NULL, NULL, NULL, &dwColor);
}

void ParaEngine::CGUIBase::ApplyAnim( const Vector2* pvOrigin, const float* pfRadian, const Vector2* pvScaling, const Vector2* pvTranslation, const DWORD* pdwColor)
{
	DWORD dwColor;
	if (pdwColor == 0)
	{
		dwColor = GetColorMask();
		pdwColor = &dwColor;
	}
	else
	{
		dwColor = *pdwColor;
		SetColorMask(dwColor);
	}
	float fRadian;
	if (pfRadian == 0)
	{
		fRadian = GetRotation();
		pfRadian = &fRadian;
	}
	else
	{
		fRadian = *pfRadian;
		SetRotation(fRadian);
	}

	Vector2 vScaling;
	if (pvScaling == 0)
	{
		GetScaling(&vScaling);
		pvScaling = &vScaling;
	}
	else
	{
		vScaling = *pvScaling;
		SetScaling(vScaling);
	}

	Vector2 vTranslation;
	if (pvTranslation == 0)
	{
		GetTranslation(&vTranslation);
		pvTranslation = &vTranslation;
	}
	else
	{
		vTranslation = *pvTranslation;
		SetTranslation(vTranslation);
	}

	Vector2 vOrigin;

	if (fRadian == 0.f && (vTranslation == Vector2::ZERO) && (vScaling == Vector2::UNIT_SCALE))
	{
		if (pvOrigin == 0)
		{
			SetRotOriginOffset(Vector2::ZERO);
		}
		pvOrigin = NULL;
	}
	else
	{
		// keep relative position.
		CGUIPosition vpos;
		GetAbsolutePosition(&vpos, &m_position);
		RECT rcRect = vpos.rect;
		Vector2 vMyOrigin;
		vMyOrigin.x = (rcRect.left + rcRect.right) / 2.f;
		vMyOrigin.y = (rcRect.top + rcRect.bottom) / 2.f;

		if (pvOrigin == 0)
		{
			GetRotOriginOffset(&vOrigin);
			vOrigin += vMyOrigin;
			pvOrigin = &vOrigin;
		}
		else
		{
			SetRotOriginOffset((*pvOrigin) - vMyOrigin);
		}
	}

	if(GetChildren() != 0)
	{
		GUIBase_List_Type::iterator itCurCP, itEndCP = GetChildren()->end();
		for( itCurCP = GetChildren()->begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			(*itCurCP)->ApplyAnim(pvOrigin, pfRadian, pvScaling, pvTranslation, pdwColor);
		}
	}
}

void ParaEngine::CGUIBase::SetTextScale(float fScale)
{

}

float ParaEngine::CGUIBase::GetTextScale()
{
	return 1.f;
}

int ParaEngine::CGUIBase::GetChildCount()
{
	return 0;
}

bool ParaEngine::CGUIBase::HasClickEvent()
{
	return HasEvent(EM_MOUSE_CLICK);
}

bool ParaEngine::CGUIBase::IsAncestorOf(CGUIBase * pChild)
{
	if (GetChildCount() > 0)
	{
		while (pChild != NULL)
		{
			if (pChild == this)
				return true;
			pChild = pChild->GetParent();
		}
	}
	return false;
}

bool ParaEngine::CGUIBase::HasParent(CGUIContainer* pParent)
{
	if (pParent && pParent->GetChildCount() > 0)
	{
		CGUIContainer* temp = GetParent();
		while (temp != NULL)
		{
			if (temp == pParent)
				return true;
			temp = temp->GetParent();
		}
	}
	return false;
}


bool ParaEngine::CGUIBase::GetVisibleRecursive()
{
	bool bVisible = GetVisible();
	CGUIContainer* temp = GetParent();
	while(bVisible==true && temp!=NULL)
	{
		bVisible = temp->GetVisible();
		temp = temp->GetParent();
	}
	return bVisible;
}



void ParaEngine::CGUIBase::BeginDepthTest(GUIState* pGUIState /*= NULL*/)
{
	if (pGUIState)
		pGUIState->BeginDepthTest(this);
}

void ParaEngine::CGUIBase::EndDepthTest(GUIState* pGUIState /*= NULL*/)
{
	if (pGUIState)
		pGUIState->EndDepthTest(this);
}

void ParaEngine::CGUIBase::SetDepth(float fDepth)
{
	m_position.Update3DDepth(fDepth);
}

float ParaEngine::CGUIBase::GetDepth()
{
	return m_position.GetDepth();
}

void ParaEngine::CGUIBase::SetZDepth( float fDepth )
{
	m_fForceZDepth = fDepth;
}

float ParaEngine::CGUIBase::GetZDepth()
{
	return m_fForceZDepth;
}

void ParaEngine::CGUIBase::SetCanCaptureMouse( bool bCanCapture )
{
	m_bCanCaptureMouse = bCanCapture;
}

void ParaEngine::CGUIBase::SetClickThrough( bool bClickThrough )
{
	m_bClickThrough = bClickThrough;
}

bool ParaEngine::CGUIBase::IsUVWrappingEnabled()
{
	return m_bIsUVWrappingEnabled;
}

void ParaEngine::CGUIBase::EnableUVWrapping( bool bUVWrapping )
{
	m_bIsUVWrappingEnabled = bUVWrapping;
}

bool ParaEngine::CGUIBase::HasKeyFocus()
{
	return CGUIRoot::GetInstance()->GetUIKeyFocus() == this;
}

bool ParaEngine::CGUIBase::GetUsePointTextureFiltering()
{
	return m_bUsePointTextureFiltering;
}

void ParaEngine::CGUIBase::SetUsePointTextureFiltering(bool bUse)
{
	m_bUsePointTextureFiltering = bUse;
}

BOOL ParaEngine::CGUIBase::ContainsPoint(const RECT &pos, const POINT &pt) const
{
	return PtInRect(pos, pt);
}

BOOL ParaEngine::CGUIBase::ContainsPoint(const POINT &pt) const
{
	return PtInRect(m_position.rect, pt);
}

void ParaEngine::CGUIBase::InflateRect(RECT* lprc, int dx, int dy)
{
	if (lprc)
	{
		lprc->left -= dx;
		lprc->top -= dy;
		lprc->right += dx;
		lprc->bottom += dy;
	}
}

void ParaEngine::CGUIBase::OffsetRect(RECT* lprc, int dx, int dy)
{
	if (lprc)
	{
		lprc->left += dx;
		lprc->top += dy;
		lprc->right += dx;
		lprc->bottom += dy;
	}
}

BOOL ParaEngine::CGUIBase::UnionRect(RECT& dest, const RECT& src1, const RECT& src2)
{
	if (IsRectEmpty(src1))
	{
		if (IsRectEmpty(src2))
		{
			SetRectEmpty( dest );
			return FALSE;
		}
		else 
			dest = src2;
	}
	else
	{
		if (IsRectEmpty(src2)) dest = src1;
		else
		{
			dest.left   = Math::Min( src1.left,	 src2.left );
			dest.right  = Math::Max( src1.right, src2.right );
			dest.top    = Math::Min( src1.top,	 src2.top );
			dest.bottom = Math::Max( src1.bottom,src2.bottom );
		}
	}
	return TRUE;
}

void ParaEngine::CGUIBase::SetRectEmpty(RECT& rc)
{
	rc.left = rc.right = rc.top = rc.bottom = 0;
}

BOOL ParaEngine::CGUIBase::IsRectEmpty(const RECT& rect)
{
	return ((rect.left >= rect.right) || (rect.top >= rect.bottom));
}

BOOL ParaEngine::CGUIBase::PtInRect(const RECT& pos, const POINT &pt)
{
	return ((pos.left <= pt.x && pos.top <= pt.y && pos.right > pt.x && pos.bottom > pt.y));
}

BOOL ParaEngine::CGUIBase::SetRect(RECT* pRc, int xLeft, int yTop, int xRight, int yBottom)
{
	if (pRc)
	{
		pRc->left = xLeft;
		pRc->top = yTop;
		pRc->right = xRight;
		pRc->bottom = yBottom;
		return TRUE;
	}
	return FALSE;
}

bool ParaEngine::CGUIBase::RectIntersect(const RECT &src1, const RECT &src2)
{
	return !((src1.left >= src2.right) || (src2.left >= src1.right) ||
		(src1.top >= src2.bottom) || (src2.top >= src1.bottom));
}

bool ParaEngine::CGUIBase::IntersectRect(RECT* dest, const RECT &src1, const RECT &src2)
{
	if ((src1.left >= src2.right) || (src2.left >= src1.right) ||
		(src1.top >= src2.bottom) || (src2.top >= src1.bottom))
	 {
		 SetRectEmpty( *dest );
		 return false;
	 }
	 dest->left   = Math::Max( src1.left,		src2.left );
	 dest->right  = Math::Min( src1.right,	src2.right );
	 dest->top    = Math::Max( src1.top,		src2.top );
	 dest->bottom = Math::Min( src1.bottom,	src2.bottom );
	 return true;
}

const char16_t* ParaEngine::CGUIBase::GetText()
{
	static const std::u16string s_empty; 
	return s_empty.c_str();
}

int ParaEngine::CGUIBase::GetTextA(std::string& out)
{
	out.clear(); return 0;
}

bool ParaEngine::CGUIBase::IsScrollable()
{
	return false;
}

bool ParaEngine::CGUIBase::IsScrollableOrHasMouseWheelRecursive()
{
	bool bScrollable = IsScrollable() || HasEvent(EM_MOUSE_WHEEL);
	CGUIContainer* temp = GetParent();
	while (!bScrollable && temp != NULL)
	{
		bScrollable = temp->IsScrollable() || temp->HasEvent(EM_MOUSE_WHEEL);
		temp = temp->GetParent();
	}
	return bScrollable;
}

CPaintEngine * ParaEngine::CGUIBase::paintEngine() const
{
	OUTPUT_LOG("warning: this function should never be called. instead call render() \n");
	return 0;
}


void ParaEngine::CGUIBase::Draw(GUIState* pGUIState, float fElapsedTime)
{
	if (IsOwnerDraw() && OnOwnerDraw(pGUIState))
	{
		// skip render function is owner draw takes place. 
	}
	else
	{
		Render(pGUIState, fElapsedTime);
	}
}

HRESULT ParaEngine::CGUIBase::DoRender(GUIState* pGUIState, float fElapsedTime)
{
	if (CanPaintOnDevice(pGUIState->painter->device()))
	{
		Begin(pGUIState, fElapsedTime);

		Draw(pGUIState, fElapsedTime);

		End(pGUIState, fElapsedTime);
		return S_OK;
	}
	else if (IsSelfPaintEnabled())
	{
		CRenderTarget* pRenderTarget = CreateGetRenderTarget();
		if (pRenderTarget)
		{
			CGUIPosition windowPos;
			GetAbsolutePosition(&windowPos, &m_position);
			RECT rect = windowPos.rect;
			int nWidth = RectWidth(rect);
			int nHeight = RectHeight(rect);
			if (nWidth > 0 && nHeight > 0)
			{
				pRenderTarget->SetRenderTargetSize(Math::NextPowerOf2(nWidth), Math::NextPowerOf2(nHeight));
				if (pRenderTarget->IsDirty() || IsDirtyRecursive())
				{
					pGUIState->AddToPostRenderingList(this);
				}
				else
				{
					OnFrameMove(fElapsedTime);
				}
				// copy render target to back buffer. 
				{
					auto oldPen = pGUIState->painter->pen();
					pGUIState->painter->setPen(Color::White);
					QRect destRect(rect);
					pGUIState->painter->setWorldTransform(QTransform());
					pGUIState->painter->drawTexture(destRect, pRenderTarget->GetTexture(), QRect(0, 0, destRect.width(), destRect.height()));
					pGUIState->painter->setPen(oldPen);
				}
			}
		}
	}
	return E_FAIL;
}


void ParaEngine::CGUIBase::FillClippingRegion(GUIState* pGUIState)
{
	CPainter* painter = GetPainter(pGUIState);
	if (painter)
	{
		QRect rectClip(GetClippingRect(pGUIState));
		// clear with fully transparent color
		painter->setPen(Color(0, 0, 0, 0));
		auto oldCompositionMode = painter->compositionMode();
		painter->setCompositionMode(CPainter::CompositionMode_Source);
		painter->drawRect(rectClip);
		painter->setCompositionMode(oldCompositionMode);
	}
}


HRESULT ParaEngine::CGUIBase::DoSelfPaint(GUIState* pGUIState, float fElapsedTime)
{
	if (IsSelfPaintEnabled())
	{
		CRenderTarget* pRenderTarget = CreateGetRenderTarget();
		if (pRenderTarget)
		{
			if (pRenderTarget->IsDirty() || IsDirtyRecursive())
			{
				SetDirty(false);
				pRenderTarget->SetDirty(false);

				if (pRenderTarget->GetPrimaryAsset())
				{
					if (pRenderTarget->Begin())
					{
						// change the paint device. 
						auto pLastPainter = pGUIState->painter;
						{
							CPainter painter(pRenderTarget);
							pGUIState->painter = &painter;

							CGUIPosition windowPos;
							GetAbsolutePosition(&windowPos, &m_position);
							RECT rect = windowPos.rect;
							// offset viewport
							painter.setViewport(QRect(-rect.left, -rect.top, 0, 0));

							if (CanPaintOnDevice(painter.device()))
							{
								// fill clipping region with default color
								if (IsAutoClearBackground())
									FillClippingRegion(pGUIState);

								auto pDevice = CGlobals::GetRenderDevice();
								pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
								pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
								pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

								pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
								pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
								// do the actual rendering on the clipped area. 
								DoRender(pGUIState, fElapsedTime);
								pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
							}

							if (painter.GetPendingAssetCount() > 0)
							{
								SetDirty(true);
							}
						}
						pRenderTarget->End();
						pGUIState->painter = pLastPainter;
					}
				}
			}
		}
	}
	return S_OK;
}

bool ParaEngine::CGUIBase::OnOwnerDraw(GUIState* pGUIState)
{
	if (!HasEvent(EM_WM_OWNER_DRAW))
		return true;
	const SimpleScript *tempScript = GetEventScript(EM_WM_OWNER_DRAW);
	if (tempScript) {
		auto painter = GetPainter(pGUIState);

		CGUIPosition windowPos;
		GetAbsolutePosition(&windowPos, &m_position);
		RECT rect = windowPos.rect;
		QTransform oldTrans = painter->worldTransform();
		{
			// convert to local control's coordinate system. 
			painter->translate(QPoint(rect.left, rect.top));
			painter->SetDepth2D(windowPos.GetDepth());
			CGlobals::GetNPLRuntime()->GetMainRuntimeState()->DoString(tempScript->szCode.c_str(), (int)tempScript->szCode.size());
			painter->setWorldTransform(oldTrans);
		}
		return true;
	}
	return true;
}

CPainter* ParaEngine::CGUIBase::GetPainter(GUIState* pGUIState /*= NULL*/)
{
	if (pGUIState)
		return pGUIState->painter;
	else
	{
		return CGUIRoot::GetInstance()->GetGUIState().painter;
	}
}

void ParaEngine::CGUIBase::EnableSelfPaint(bool bEnable)
{
	SetForcedPaintDevice(bEnable ? this : NULL);
}

bool ParaEngine::CGUIBase::IsSelfPaintEnabled()
{
	return (m_pForcedPaintDevice == this);
}

bool ParaEngine::CGUIBase::IsAutoClearBackground() const
{
	return m_bAutoClearBackground;
}

void ParaEngine::CGUIBase::SetAutoClearBackground(bool val)
{
	m_bAutoClearBackground = val;
}

CRenderTarget* ParaEngine::CGUIBase::CreateGetRenderTarget(bool bCreateIfNotExist)
{
	return NULL;
}

bool ParaEngine::CGUIBase::IsDirty() const
{
	return m_bDirty;
}

bool ParaEngine::CGUIBase::IsDirtyRecursive()
{
	if (IsDirty())
		return true;
	else
	{
		auto pChildren = GetChildren();
		if (pChildren)
		{
			for (auto& child : *pChildren)
			{
				if (child->GetVisible() && child->IsDirtyRecursive())
					return true;
			}
		}
		return false;
	}
}

void ParaEngine::CGUIBase::SetDirty(bool val)
{
	if (m_bDirty != val)
	{
		m_bDirty = val;
	}
}

bool ParaEngine::CGUIBase::CanPaintOnDevice(CPaintDevice* val)
{
	return m_pForcedPaintDevice == 0 || m_pForcedPaintDevice == val;
}

CGUIContainer* ParaEngine::CGUIBase::GetPainterWindow()
{
	CGUIBase* w = this;
	CGUIContainer *p = w->GetParent();
	while (!w->IsPainterWindow() && p) {
		w = p;
		p = p->GetParent();
	}
	return w->IsPainterWindow() ? (CGUIContainer*)w : nullptr;
}

RECT ParaEngine::CGUIBase::GetClippingRect(GUIState* pGUIState)
{
	RECT rcScreen, rcWindow;//rcScreen is the rect of this container, rcWindow is the available rect of drawing this container.
	CGUIContainer *parent = m_parent;
	rcWindow = rcScreen = m_objResource->GetDrawingRects(0);//the whole container
	while (parent != NULL) {
		// if (! parent->GetFastRender())
		{
			rcWindow = parent->m_objResource->GetDrawingRects(0);
			if (rcScreen.top > rcWindow.bottom || rcScreen.bottom<rcWindow.top || rcScreen.left>rcWindow.right || rcScreen.right < rcWindow.left) {
				SetRect(&rcScreen, 0, 0, 0, 0);
				break;
			}
			if (rcScreen.top < rcWindow.top) rcScreen.top = rcWindow.top + 1;
			if (rcScreen.left < rcWindow.left) rcScreen.left = rcWindow.left + 1;
			if (rcScreen.bottom > rcWindow.bottom) rcScreen.bottom = rcWindow.bottom - 1;
			if (rcScreen.right > rcWindow.right) rcScreen.right = rcWindow.right - 1;
		}
		parent = parent->m_parent;
	}
	if (rcScreen.right > (int)pGUIState->nBkbufWidth){
		rcScreen.right = pGUIState->nBkbufWidth;
	}
	if (rcScreen.top < 0){
		rcScreen.top = 0;
	}
	if (rcScreen.left < 0){
		rcScreen.left = 0;
	}
	if (rcScreen.bottom > (int)pGUIState->nBkbufHeight){
		rcScreen.bottom = pGUIState->nBkbufHeight;
	}
	return rcScreen;
}

bool ParaEngine::CGUIBase::InvalidateRect(const RECT* lpRect)
{
	CGUIContainer* painterParent = GetPainterWindow();
	if (painterParent && painterParent!=this)
	{
		painterParent->InvalidateRect(lpRect);
	}
	return true;
}

bool ParaEngine::CGUIBase::IsNeedUpdate(bool bRecursive /*= false*/)
{
	if (!bRecursive || m_bNeedUpdate)
		return m_bNeedUpdate;
	else
	{
		auto pChildren = GetChildren();
		if (pChildren)
		{
			for (auto& child : *pChildren)
			{
				if (child->GetVisible() && child->IsNeedUpdate(true))
					return true;
			}
		}
		return false;
	}
}

bool ParaEngine::CGUIBase::CanHaveFocus()
{
	return m_bCanHasFocus;
}

void ParaEngine::CGUIBase::SetCanHaveFocus(bool bCanHaveFocus)
{
	m_bCanHasFocus = bCanHaveFocus;
}

void ParaEngine::CGUIBase::CaptureMouse()
{
	CGUIRoot::GetInstance()->GetMouse()->SetCapture(this);
}

void ParaEngine::CGUIBase::ReleaseMouseCapture()
{
	CGUIRoot::GetInstance()->GetMouse()->ReleaseCapture(this);
}

bool ParaEngine::CGUIBase::IsMouseCaptured()
{
	return CGUIRoot::GetInstance()->GetMouse()->GetCapture() == this;
}


int ParaEngine::CGUIBase::OnHandleWinMsgChars(const std::wstring& sChars)
{
	SendInputMethodEvent(sChars);
	return 0;
}

void ParaEngine::CGUIBase::SendInputMethodEvent(const std::wstring& sText)
{
	if (!sText.empty())
	{
		u16string sTextWide;
		sTextWide.resize(sText.size());
		for (int i = 0; i < (int)sText.size(); ++i)
		{
			sTextWide[i] = sText[i];
		}
		string sTextUTF8;
		if (StringHelper::UTF16ToUTF8(sTextWide, sTextUTF8))
		{
			if (HasEvent(EM_WM_INPUT_METHOD))
			{
				string sOutput;
				NPL::NPLHelper::EncodeStringInQuotation(sOutput, 0, sTextUTF8);
				sOutput = string("msg=") + sOutput + ";";
				ActivateScript(sOutput, EM_WM_INPUT_METHOD);
			}
		}
	}
}

bool ParaEngine::CGUIBase::FetchIMEString()
{
#ifdef PARAENGINE_CLIENT
	wstring sText = CGUIIME::GetCurrentCompString();
	
	if (!sText.empty())
	{
		SendInputMethodEvent(sText);
		return true;
	}
#endif
	return false;
}

void* ParaEngine::CGUIBase::QueryObject(int nObjectType)
{
	if (nObjectType == ATTRIBUTE_CLASSID_CGUIBase)
		return this;
	else
		return NULL;
}


void ParaEngine::CGUIBase::SetX(int x)
{
	SetLocation(x, GetPosition()->rect.top);
}

int ParaEngine::CGUIBase::GetX()
{
	return GetPosition()->rect.left;
}

void ParaEngine::CGUIBase::SetY(int y)
{
	SetLocation(GetPosition()->rect.left, y);
}

int ParaEngine::CGUIBase::GetY()
{
	return GetPosition()->rect.top;
}


int ParaEngine::CGUIBase::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);
	PE_ASSERT(pClass != NULL);
	pClass->AddField("name", FieldType_String, (void*)SetName_s, (void*)GetName_s, NULL, NULL, bOverride);
	pClass->AddField("ID", FieldType_Int, (void*)SetID_s, (void*)GetID_s, NULL, NULL, bOverride);
	pClass->AddField("index", FieldType_Int, (void*)0, (void*)GetIndex_s, NULL, NULL, bOverride);
	pClass->AddField("Visible", FieldType_Bool, (void*)SetVisible_s, (void*)GetVisible_s, NULL, NULL, bOverride);
	pClass->AddField("VisibleRecursive", FieldType_Bool, (void*)SetVisible_s, (void*)GetVisibleRecursive_s, NULL, NULL, bOverride);
	pClass->AddField("x", FieldType_Int, (void*)SetX_s, (void*)GetX_s, NULL, NULL, bOverride);
	pClass->AddField("y", FieldType_Int, (void*)SetY_s, (void*)GetY_s, NULL, NULL, bOverride);
	pClass->AddField("Width", FieldType_Int, (void*)SetWidth_s, (void*)GetWidth_s, NULL, NULL, bOverride);
	pClass->AddField("Height", FieldType_Int, (void*)SetHeight_s, (void*)GetHeight_s, NULL, NULL, bOverride);

	pClass->AddField("ApplyAnim", FieldType_void, (void*)ApplyAnim_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("Scaling", FieldType_Vector2, (void*)SetScaling_s, (void*)GetScaling_s, NULL, NULL, bOverride);
	pClass->AddField("Translation", FieldType_Vector2, (void*)SetTranslation_s, (void*)GetTranslation_s, NULL, NULL, bOverride);
	pClass->AddField("Rotation", FieldType_Float, (void*)SetRotation_s, (void*)GetRotation_s, NULL, NULL, bOverride);
	pClass->AddField("RotOriginOffset", FieldType_Vector2, (void*)SetRotOriginOffset_s, (void*)GetRotOriginOffset_s, NULL, NULL, bOverride);
	pClass->AddField("ColorMask", FieldType_DWORD, (void*)SetColorMask_s, (void*)GetColorMask_s, NULL, NULL, bOverride);

	pClass->AddField("text", FieldType_String, (void*)SetText_s, (void*)GetText_s, NULL, NULL, bOverride);
	pClass->AddField("TextScale", FieldType_Float, (void*)SetTextScale_s, (void*)GetTextScale_s, NULL, NULL, bOverride);
	pClass->AddField("UseTextShadow", FieldType_Bool, (void*)SetUseTextShadow_s, (void*)GetUseTextShadow_s, NULL, NULL, bOverride);
	pClass->AddField("TextShadowQuality", FieldType_Int, (void*)SetTextShadowQuality_s, (void*)GetTextShadowQuality_s, NULL, NULL, bOverride);
	pClass->AddField("TextShadowColor", FieldType_DWORD, (void*)SetTextShadowColor_s, (void*)GetTextShadowColor_s, NULL, NULL, bOverride);
	pClass->AddField("TextOffsetX", FieldType_Int, (void*)SetTextOffsetX_s, (void*)GetTextOffsetX_s, NULL, NULL, bOverride);
	pClass->AddField("TextOffsetY", FieldType_Int, (void*)SetTextOffsetY_s, (void*)GetTextOffsetY_s, NULL, NULL, bOverride);
	pClass->AddField("AlwaysMouseOver", FieldType_Bool, (void*)SetAlwaysMouseOver_s, (void*)GetAlwaysMouseOver_s, NULL, NULL, bOverride);
	pClass->AddField("Enable", FieldType_Bool, (void*)SetEnabled_s, (void*)GetEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("Depth", FieldType_Float, (void*)SetDepth_s, (void*)GetDepth_s, NULL, NULL, bOverride);
	pClass->AddField("LifeTime", FieldType_Int, (void*)SetLifeTime_s, (void*)GetLifeTime_s, NULL, NULL, bOverride);
	pClass->AddField("Spacing", FieldType_Int, (void*)SetSpacing_s, (void*)GetSpacing_s, NULL, NULL, bOverride);
	pClass->AddField("Padding", FieldType_Int, (void*)SetPadding_s, (void*)GetPadding_s, NULL, NULL, bOverride);
	pClass->AddField("ZOrder", FieldType_Int, (void*)SetZOrder_s, (void*)GetZOrder_s, NULL, NULL, bOverride);
	pClass->AddField("Cursor", FieldType_String, (void*)SetCursor_s, (void*)GetCursor_s, NULL, NULL, bOverride);
	pClass->AddField("UsePointTextureFiltering", FieldType_Bool, (void*)SetUsePointTextureFiltering_s, (void*)GetUsePointTextureFiltering_s, NULL, NULL, bOverride);

	pClass->AddField("IsPressed", FieldType_Bool, NULL, (void*)IsPressed_s, NULL, NULL, bOverride);
	pClass->AddField("HighlightStyle", FieldType_Int, (void*)SetHighlightStyle_s, (void*)GetHighlightStyle_s, NULL, NULL, bOverride);
	pClass->AddField("AnimationStyle", FieldType_Int, (void*)SetAnimationStyle_s, (void*)GetAnimationStyle_s, NULL, NULL, bOverride);
	pClass->AddField("ClickThrough", FieldType_Bool, (void*)SetClickThrough_s, (void*)IsClickThrough_s, NULL, NULL, bOverride);
	pClass->AddField("UVWrappingEnabled", FieldType_Bool, (void*)EnableUVWrapping_s, (void*)IsUVWrappingEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("OwnerDraw", FieldType_Bool, (void*)SetOwnerDraw_s, (void*)IsOwnerDraw_s, NULL, NULL, bOverride);
	pClass->AddField("IsDirty", FieldType_Bool, (void*)SetDirty_s, (void*)IsDirty_s, NULL, NULL, bOverride);
	pClass->AddField("SelfPaint", FieldType_Bool, (void*)EnableSelfPaint_s, (void*)IsSelfPaintEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("HasKeyFocus", FieldType_Bool, NULL, (void*)HasKeyFocus_s, NULL, NULL, bOverride);
	pClass->AddField("CanHaveFocus", FieldType_Bool, (void*)SetCanHaveFocus_s, (void*)CanHaveFocus_s, NULL, NULL, bOverride);
	pClass->AddField("ApplyColorMask", FieldType_void, (void*)ApplyColorMask_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("MouseCaptured", FieldType_Bool, (void*)SetMouseCaptured_s, (void*)IsMouseCaptured_s, NULL, NULL, bOverride);
	pClass->AddField("AutoClearBackground", FieldType_Bool, (void*)SetAutoClearBackground_s, (void*)IsAutoClearBackground_s, NULL, NULL, bOverride);
	pClass->AddField("InputMethodEnabled", FieldType_Bool, (void*)SetInputMethodEnabled_s, (void*)IsInputMethodEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("CompositionPoint", FieldType_Vector2, (void*)SetCompositionPoint_s, (void*)GetCompositionPoint_s, NULL, NULL, bOverride);
	pClass->AddField("EnableClientTest", FieldType_Bool, (void*)EnableNonClientTest_s, (void*)IsNonClientTestEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("TouchTranslationAtt", FieldType_Int, (void*)SetTouchTranslationAttFlag_s, (void*)GetTouchTranslationAttFlag_s, NULL, NULL, bOverride);
	return S_OK;
}


