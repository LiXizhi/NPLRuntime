//----------------------------------------------------------------------
// Class:	CGUICanvas
// Authors:	Liu Weili, LiXizhi
// Date:	2006.7.24
// Revised: 2006.7.24
// desc: 
// The canvas control provides a canvas to let the user draw anything on it. It supports rotate, zoom methods.
//----------------------------------------------------------------------

#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "GUICanvas.h"
#include "GUIContainer.h"
#include "GUIResource.h"
#include "AISimulator.h"
#include "Globals.h"
#include "ParaWorldAsset.h"
#include "ic/ICConfigManager.h"
#include "GUIRoot.h"
#include "GUIDirectInput.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "SceneObject.h"
#include "util/StringHelper.h"

#include "memdebug.h"

using namespace ParaEngine;

const IType* CGUICanvas::m_type=NULL;


CGUICanvas::CGUICanvas():CGUIBase()
{
	if (!m_type){
		m_type=IType::GetType("guicanvas");
	}
	m_bCanHasFocus=false;
	m_bCandrag=false;
	m_nHotkey = 0;
	m_eState=CanvasNone;
	m_nCanvasIndex=-1;
	m_fPanSpeed=1.0;
	m_fRotateSpeed=1.0;
}

CGUICanvas::~CGUICanvas()
{}
void CGUICanvas::Clone(IObject* pobj)const
{
	PE_ASSERT(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}

	CGUICanvas* pCanvas=(CGUICanvas*)pobj;
	CGUIBase::Clone(pCanvas);
	pCanvas->m_nCanvasIndex=m_nCanvasIndex;
	pCanvas->m_eState=CanvasNone;
	pCanvas->m_fPanSpeed=m_fPanSpeed;
	pCanvas->m_fRotateSpeed=m_fRotateSpeed;
}

IObject* CGUICanvas::Clone()const
{
	CGUICanvas *pCanvas=new CGUICanvas();
	Clone(pCanvas);
	return pCanvas;
}

void CGUICanvas::StaticInit()
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIBase")) {
		CGUIBase::StaticInit();
	}
	//load the default CGUIBase object and copy all its value to the new button
	CGUICanvas *pCanvas=new CGUICanvas();
	pOm->CloneObject("default_CGUIBase",pCanvas);

	CEventBinding* pBinding=pCanvas->m_event->GetEventBindingObj();
	pBinding->DisableKeyboard();
	pBinding->DisableEvent(EM_MOUSE_DRAGBEGIN);
	pBinding->DisableEvent(EM_MOUSE_CLICK);
	pBinding->DisableEvent(EM_MOUSE_DBCLICK);
	//pBinding->DisableMouse();

	using namespace ParaInfoCenter;
	CICConfigManager *cm=CGlobals::GetICConfigManager();
	string value0,value1;
	int event0,event1,a;
	DWORD b;
	HRESULT hr;
	hr=cm->GetSize("GUI_canvas_control_mapping",&b);
	if (hr==E_INVALIDARG||hr==E_ACCESSDENIED) {
		//error
	}else{
		for (a=0;a<(int)b;a+=2) {
			hr=cm->GetTextValue("GUI_canvas_control_mapping",value0,a);
			if (FAILED(hr)) {
				break;
			}
			hr=cm->GetTextValue("GUI_canvas_control_mapping",value1,a+1);
			if (FAILED(hr)) {
				break;
			}
			event0=CEventBinding::StringToEventValue(value0);
			event1=CEventBinding::StringToEventValue(value1);
			pBinding->MapEvent(event0,event1);
		}
	}
	int tempint;
	if (cm->GetIntValue("GUI_canvas_control_visible",&tempint)==S_OK) {
		if (tempint==0) {
			pCanvas->m_bIsVisible=false;
		}else
			pCanvas->m_bIsVisible=true;
	}
	if (cm->GetIntValue("GUI_canvas_control_enable",&tempint)==S_OK) {
		if (tempint==0) {
			pCanvas->m_bIsEnabled=false;
		}else
			pCanvas->m_bIsEnabled=true;
	}
	if (cm->GetIntValue("GUI_canvas_control_canhasfocus",&tempint)==S_OK) {
		if (tempint==0) {
			pCanvas->m_bCanHasFocus=false;
		}else
			pCanvas->m_bCanHasFocus=true;
	}
	if (cm->GetIntValue("GUI_canvas_control_candrag",&tempint)==S_OK) {
		if (tempint==0) {
			pCanvas->SetCandrag(false);
		}else
			pCanvas->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_canvas_control_lifetime",&tempint)==S_OK) {
		pCanvas->m_nLifeTimeCountDown=tempint;
	}
	double tempfloat;
	if (cm->GetDoubleValue("GUI_canvas_control_rotatespeed",&tempfloat)==S_OK) {
		pCanvas->m_fRotateSpeed=tempfloat;
	}
	if (cm->GetDoubleValue("GUI_canvas_control_rotatespeed",&tempfloat)==S_OK) {
		pCanvas->m_fPanSpeed=tempfloat;
	}

	RECT *prect=NULL;
	RECT rect;
	SpriteFontEntity* pFont;
	TextureEntity* pTexture=NULL;
	pFont=CGlobals::GetAssetManager()->GetFont("sys");
	GUIFontElement font_;
	GUIFontElement* pFontElement=&font_;
	GUITextureElement tex_;
	GUITextureElement* pElement=&tex_;
	string background,filename;
	bool bLoadDefault=true;
	if (cm->GetTextValue("GUI_canvas_control_background",background)==S_OK) {
		prect=ParaEngine::StringHelper::GetImageAndRect(background,filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
		if (pTexture!=NULL) 
			bLoadDefault=false;
	} 
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture=CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds",TextureEntity::StaticTexture);
		prect=&rect;
		SetRect( prect, 136, 0, 272, 54 );
	}
	pCanvas->m_objResource->SetActiveLayer();
	pCanvas->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);

	pCanvas->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,200,200,200));
	pCanvas->m_objResource->AddElement(pElement,"background");
	
	pOm->SetObject("default_CGUICanvas",pCanvas);
	SAFE_RELEASE(pCanvas);
}


void CGUICanvas::Reset()
{
	CGUIBase::Reset();
}
//if return false, the message can be handled onto other message handler.
bool CGUICanvas::MsgProc(MSG *event)
{
	//one call to this function will trigger at most one event
	if( !m_bIsEnabled )
		return false;
	bool bHandled=false;
	CGUIRoot *pRoot=CGUIRoot::GetInstance();
	CDirectMouse *pMouse=pRoot->m_pMouse;
	CDirectKeyboard *pKeyboard=pRoot->m_pKeyboard;
	MSG newMsg;
	if (event!=NULL&&!m_event->InterpretMessage(event)) {
		return false;
	}
	POINT pt;
	//static bool bLeftdown=false,bRightdown=false,bMiddledown=false;
	pt.x=m_event->m_mouse.x;
	pt.y=m_event->m_mouse.y;
	int nEvent=m_event->GetTriggerEvent();
	CEventBinding::InitMsg(&newMsg,event->time,nEvent,pt);
	if (m_bIsVisible){
		if (nEvent==EM_NONE) {
			return false;
		}
		if (m_event->IsMapTo(nEvent,EM_MOUSE_MOVE)) {
			int dx=pMouse->GetMouseXDeltaSteps(),dy=pMouse->GetMouseYDeltaSteps();
			switch(m_eState){
			case CanvasPan:
				Pan(dx,dy);
				break;
			case CanvasZoom:
				break;
			case CanvasRotate:
				Rotate(dx*0.01,dy*0.01);
			    break;
			}
		}else if (m_event->IsMapTo(nEvent,EM_CV_ROTATEBEGIN)) {
			newMsg.message=EM_CTRL_CAPTUREMOUSE;
			CGUIBase::MsgProc(&newMsg);
			switch(m_eState){
			case CanvasPan:
				newMsg.message=EM_CV_PANEND;
				MsgProc(&newMsg);
				//continue to CanvasNone case
			case CanvasNone:
				pMouse->SetLock(true);
				pMouse->ShowCursor(false);
				m_eState=CanvasRotate;
			    break;
			}
			bHandled=true;
		}else if (m_event->IsMapTo(nEvent,EM_CV_ROTATEEND)) {
			newMsg.message=EM_CTRL_RELEASEMOUSE;
			CGUIBase::MsgProc(&newMsg);
			pMouse->SetLock(false);
			pMouse->ShowCursor(true);
			m_eState=CanvasNone;
			bHandled=true;
		}else if (m_event->IsMapTo(nEvent,EM_CV_PANBEGIN)) {
			newMsg.message=EM_CTRL_CAPTUREMOUSE;
			CGUIBase::MsgProc(&newMsg);
			switch(m_eState){
			case CanvasRotate:
				newMsg.message=EM_CV_ROTATEEND;
				MsgProc(&newMsg);
				//continue to CanvasNone case
			case CanvasNone:
				pMouse->SetLock(true);
				pMouse->ShowCursor(false);
				m_eState=CanvasPan;
				break;
			}
			bHandled=true;
		}else if (m_event->IsMapTo(nEvent,EM_CV_PANEND)) {
			newMsg.message=EM_CTRL_RELEASEMOUSE;
			CGUIBase::MsgProc(&newMsg);
			pMouse->SetLock(false);
			pMouse->ShowCursor(true);
			m_eState=CanvasNone;
			bHandled=true;
		}else if (m_event->IsMapTo(nEvent,EM_MOUSE_WHEEL)) {
			int nZoomAmount = int(m_event->m_mouse.WheelDelta) / 120;
			Zoom(nZoomAmount>0?1+0.1*nZoomAmount:(1+0.1*nZoomAmount>0?(1+0.1*nZoomAmount):0.1));
           bHandled=true;
		}
	}

	if (!bHandled) {
		MSG msg = m_event->GenerateMessage();
		return CGUIBase::MsgProc(&msg);
	}
	return true;
}

HRESULT CGUICanvas::Render(GUIState* pGUIState ,float fElapsedTime)
{	
	if (!m_bIsVisible) {
		return S_OK;
	}
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();

	if (m_position.nPositionType==CGUIPosition::relative_to_3Dobject) 
	{
		if(pGUIState)
		{
			pGUIState->pd3dDevice->SetRenderState(D3DRS_ZENABLE,TRUE);
			pGUIState->pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		}
		UpdateRects();
	}

	RECT rcWindow = m_objResource->GetDrawingRects(0);

	return S_OK;
}

void CGUICanvas::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUICanvas")) {
		CGUICanvas::StaticInit();
	}
	pOm->CloneObject("default_CGUICanvas",this);
	CGUIBase::InitObject(strObjectName,alignment,x,y,width,height);
}

void CGUICanvas::UpdateRects()
{
	CGUIPosition tempPos;
	GetAbsolutePosition(&tempPos,&m_position);
	if(m_position.nPositionType == CGUIPosition::relative_to_3Dobject)
	{
		SetDepth(tempPos.GetDepth());
	}
	RECT rcInner = tempPos.rect;
	m_objResource->SetDrawingRects(&tempPos.rect,0);

	if(m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT)
	{
		const int offset = 1;
		RECT tempRect;
		// use the left top and right bottom texture element size to calculate the inner rect size. 
		GUITextureElement* pElement = m_objResource->GetTextureElement(1);
		if(pElement)
		{
			pElement->GetRect(&tempRect);
			rcInner.left = RectWidth(tempRect) + tempPos.rect.left;
			rcInner.top = RectHeight(tempRect) + tempPos.rect.top;
		}
		pElement = m_objResource->GetTextureElement(8);
		if(pElement)
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
		m_objResource->SetDrawingRects(&rcInner,offset+0);
		SetRect( &tempRect, tempPos.rect.left, tempPos.rect.top, rcInner.left, rcInner.top );
		m_objResource->SetDrawingRects(&tempRect,offset+1);
		SetRect( &tempRect, rcInner.left, tempPos.rect.top, rcInner.right, rcInner.top );
		m_objResource->SetDrawingRects(&tempRect,offset+2);
		SetRect( &tempRect, rcInner.right, tempPos.rect.top, tempPos.rect.right, rcInner.top );
		m_objResource->SetDrawingRects(&tempRect,offset+3);
		SetRect( &tempRect, tempPos.rect.left, rcInner.top, rcInner.left, rcInner.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+4);
		SetRect( &tempRect, rcInner.right, rcInner.top, tempPos.rect.right, rcInner.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+5);
		SetRect( &tempRect, tempPos.rect.left, rcInner.bottom, rcInner.left, tempPos.rect.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+6);
		SetRect( &tempRect, rcInner.left, rcInner.bottom, rcInner.right, tempPos.rect.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+7);
		SetRect( &tempRect, rcInner.right, rcInner.bottom, tempPos.rect.right, tempPos.rect.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+8);
	}

	m_bNeedUpdate=false;
}

void CGUICanvas::Zoom(double zoom)
{
}

void CGUICanvas::Rotate(double xrotate,double yrotate)
{
}

void CGUICanvas::Pan(double x, double y)
{
}

void CGUICanvas::SetCanvasIndex(int index)
{
	m_nCanvasIndex=index;
}
#endif