//----------------------------------------------------------------------
// Class:	CGUISlider
// Authors:	Liu Weili, LiXizhi
// Date:	2006.1.10
// Revised: 2006.1.10
//
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "GUISlider.h"
#include "GUIDirectInput.h"
#include "GUIBase.h"
#include "GUIRoot.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "ic/ICConfigManager.h"
#include "util/StringHelper.h"
#include "memdebug.h"

using namespace ParaEngine;


const IType* CGUISlider::m_type=NULL;
CGUISlider::CGUISlider():CGUIBase()
{
	if (!m_type){
		m_type=IType::GetType("guislider");
	}

	m_nMin = 0;
	m_nMax = 100;
	m_nValue = 50;
	m_bReceiveDrag=false;
	m_Linked=NULL;
	m_bPressed = false;
	m_nOldValue = -1;
}

void CGUISlider::Clone(IObject *pobj)const
{
	PE_ASSERT(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}
	CGUISlider* pSlider=(CGUISlider*)pobj;
	CGUIBase::Clone(pSlider);
	pSlider->m_nMax=m_nMax;
	pSlider->m_nMin=m_nMin;
	pSlider->m_nValue=m_nValue;
	pSlider->m_Linked=m_Linked;
	pSlider->m_bPressed=m_bPressed;

}

IObject* CGUISlider::Clone()const
{
	CGUISlider *pSlider=new CGUISlider();
	Clone(pSlider);
	return pSlider;
}

void CGUISlider::StaticInit()
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIContainer")) {
		CGUIContainer::StaticInit();
	}
	//load the default CGUIBase object and copy all its value to the new button
	CGUISlider *pSlider=new CGUISlider();
	pOm->CloneObject("default_CGUIBase",pSlider);
// 	pSlider->m_objType = CGUIBase::Type_GUISlider;
	pSlider->m_nValue=50;
	pSlider->m_nMin=0;
	pSlider->m_nMax=100;
	pSlider->SetCandrag(false);
	pSlider->m_bReceiveDrag=false;
	pSlider->m_bPressed=false;
	pSlider->m_bCanHasFocus=true;
	pSlider->m_Linked=NULL;
	CEventBinding* pBinding=pSlider->m_event->GetEventBindingObj();
	pBinding->EnableMouse();
	pBinding->EnableKeyboard();
//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_SL_ACTIONBEGIN);
//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_SL_ACTIONEND);
//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_CTRL_FOCUSIN);
	pBinding->MapEvent(EM_KEY_HOME,EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_END,EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_PAGE_DOWN,EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_PAGE_UP,EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_LEFT,EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_RIGHT,EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_DOWN,EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_UP,EM_CTRL_HOLDKEY);
	pBinding->DisableEvent(EM_MOUSE_DBCLICK);
	pBinding->DisableEvent(EM_MOUSE_CLICK);
	
	using namespace ParaInfoCenter;
	CICConfigManager *cm=CGlobals::GetICConfigManager();
	string value0,value1;
	int event0,event1,a;
	DWORD b;
	HRESULT hr;
	hr=cm->GetSize("GUI_slider_control_mapping",&b);
	if (hr==E_INVALIDARG||hr==E_ACCESSDENIED) {
		//error
	}else{
		for (a=0;a<(int)b;a+=2) {
			hr=cm->GetTextValue("GUI_slider_control_mapping",value0,a);
			if (hr!=S_OK) {
				break;
			}
			hr=cm->GetTextValue("GUI_slider_control_mapping",value1,a+1);
			if (hr!=S_OK) {
				break;
			}
			event0=CEventBinding::StringToEventValue(value0);
			event1=CEventBinding::StringToEventValue(value1);
			pBinding->MapEvent(event0,event1);
		}
	}
	int tempint;;
	if (cm->GetIntValue("GUI_slider_control_defaultvalue",&tempint)==S_OK) {
		pSlider->m_nValue=tempint;
	}
	if (cm->GetIntValue("GUI_slider_control_defaultmax",&tempint)==S_OK) {
		pSlider->m_nMax=tempint;
	}
	if (cm->GetIntValue("GUI_slider_control_defaultmin",&tempint)==S_OK) {
		pSlider->m_nMin=tempint;
	}
	if (cm->GetIntValue("GUI_slider_control_visible",&tempint)==S_OK) {
		if (tempint==0) {
			pSlider->m_bIsVisible=false;
		}else
			pSlider->m_bIsVisible=true;
	}
	if (cm->GetIntValue("GUI_slider_control_enable",&tempint)==S_OK) {
		if (tempint==0) {
			pSlider->m_bIsEnabled=false;
		}else
			pSlider->m_bIsEnabled=true;
	}
	if (cm->GetIntValue("GUI_slider_control_canhasfocus",&tempint)==S_OK) {
		if (tempint==0) {
			pSlider->m_bCanHasFocus=false;
		}else
			pSlider->m_bCanHasFocus=true;
	}
	if (cm->GetIntValue("GUI_slider_control_receivedrag",&tempint)==S_OK) {
		if (tempint==0) {
			pSlider->m_bReceiveDrag=false;
		}else
			pSlider->m_bReceiveDrag=true;
	}
	if (cm->GetIntValue("GUI_slider_control_candrag",&tempint)==S_OK) {
		if (tempint==0) {
			pSlider->SetCandrag(false);
		}else
			pSlider->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_slider_control_lifetime",&tempint)==S_OK) {
		pSlider->m_nLifeTimeCountDown=tempint;
	}
	RECT *prect=NULL;
	RECT rect;
	SpriteFontEntity* pFont;
	TextureEntity* pTexture=NULL;
	pFont=CGlobals::GetAssetManager()->GetFont("sys");
	GUITextureElement tex_;
	GUITextureElement* pElement=&tex_;
	pSlider->m_objResource->SetActiveLayer();
	pSlider->m_objResource->SetCurrentState();

	string background,filename;
	bool bLoadDefault=true;
	if (cm->GetTextValue("GUI_slider_control_background",background)==S_OK) {
		prect=StringHelper::GetImageAndRect(background,filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
		if (pTexture!=NULL) 
			bLoadDefault=false;
	} 
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture=CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds",TextureEntity::StaticTexture);
		prect=&rect;
		SetRect( prect, 1, 290, 280, 331 );
	}
	pSlider->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(220,255,255,255));
	pSlider->m_objResource->AddElement(pElement,"background");

	pSlider->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	pSlider->m_objResource->AddElement(pElement,"background");

	pSlider->m_objResource->SetCurrentState(GUIResourceState_Highlight);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	pSlider->m_objResource->AddElement(pElement,"background");

	pSlider->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(70,255,255,255));
	pSlider->m_objResource->AddElement(pElement,"background");
	
	bLoadDefault=true;
	if (cm->GetTextValue("GUI_slider_control_button",background)==S_OK) {
		prect=StringHelper::GetImageAndRect(background,filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
		if (pTexture!=NULL) 
			bLoadDefault=false;
	} 
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture=CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds",TextureEntity::StaticTexture);
		prect=&rect;
		SetRect( prect, 248, 55, 289, 96 );
	}
	pSlider->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	pSlider->m_objResource->AddElement(pElement,"button");

	pSlider->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	pSlider->m_objResource->AddElement(pElement,"button");

	pSlider->m_objResource->SetCurrentState(GUIResourceState_Highlight);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	pSlider->m_objResource->AddElement(pElement,"button");

	pSlider->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(70,255,255,255));
	pSlider->m_objResource->AddElement(pElement,"button");
	

	pSlider->m_objResource->SetCurrentState();
	pSlider->m_objResource->SetActiveLayer();
	pOm->SetObject("default_CGUISlider",pSlider);
	SAFE_RELEASE(pSlider);

}
//--------------------------------------------------------------------------------------
BOOL CGUISlider::ContainsPoint( const POINT &pt ) 
{ 
	return ( PtInRect( m_objResource->GetDrawingRects(0), pt ) || 
		PtInRect( m_objResource->GetDrawingRects(1), pt ) ); 
}

void CGUISlider::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUISlider")) {
		CGUISlider::StaticInit();
	}
	pOm->CloneObject("default_CGUISlider",this);
	CGUIBase::InitObject(strObjectName,alignment,x,y,width,height);

}


//--------------------------------------------------------------------------------------
void CGUISlider::UpdateRects()
{
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();

	CGUIPosition tempPos;
	GetAbsolutePosition(&tempPos,&m_position);
	if(m_position.IsRelativeTo3DObject())
	{
		SetDepth(tempPos.GetDepth());
	}
	RECT rcInner = tempPos.rect;
	m_objResource->SetDrawingRects(&tempPos.rect,0);

	// window rect at index 0
	RECT rcButton=m_objResource->GetDrawingRects(0);
	rcButton.right = rcButton.left + RectHeight( rcButton );
	OffsetRect( &rcButton, -RectWidth( rcButton )/2, 0 );

	// button rect at index 1
	m_nButtonX = RectWidth( rcButton )/2+(int) ( (m_nValue - m_nMin) * ((float)RectWidth( m_objResource->GetDrawingRects(0)) -RectWidth( rcButton )) / (m_nMax - m_nMin) );
	OffsetRect( &rcButton, m_nButtonX, 0 );
	m_objResource->SetDrawingRects(&rcButton,1);

	// 9 element bar at index 2-10

	int nLayerCount = 0;
	do 
	{
		if(nLayerCount == 1)
			m_objResource->SetActiveLayer(CGUIResource::LAYER_BACKGROUND);
		if(m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT)
		{
			const int offset = 2;
			RECT tempRect;
			// use the left top and right bottom texture element size to calculate the inner rect size. 
			GUITextureElement* pElement = m_objResource->GetTextureElement(offset+1);
			if(pElement)
			{
				pElement->GetRect(&tempRect);
				rcInner.left = RectWidth(tempRect) + tempPos.rect.left;
				rcInner.top = RectHeight(tempRect) + tempPos.rect.top;
			}
			pElement = m_objResource->GetTextureElement(offset+8);
			if(pElement)
			{
				pElement->GetRect(&tempRect);
				rcInner.right = tempPos.rect.right - RectWidth(tempRect);
				rcInner.bottom = tempPos.rect.bottom - RectHeight(tempRect);
			}

			// Element assignment:
			//   2+0 - text area
			//   2+1 - top left border
			//   2+2 - top border
			//   2+3 - top right border
			//   2+4 - left border
			//   2+5 - right border
			//   2+6 - lower left border
			//   2+7 - lower border
			//   2+8 - lower right border
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
		if(nLayerCount == 1)
			m_objResource->SetActiveLayer(CGUIResource::LAYER_ARTWORK);
	} while((++nLayerCount)<2 && m_objResource->HasLayer(CGUIResource::LAYER_BACKGROUND));

	m_bNeedUpdate=false;
}

int CGUISlider::ValueFromPos( int x )
{ 
	float fValuePerPixel = (float)(m_nMax - m_nMin) / (RectWidth( m_objResource->GetDrawingRects(0))-RectWidth( m_objResource->GetDrawingRects(1) )) ;
	return (int) (0.5f + m_nMin + fValuePerPixel * (x -  m_objResource->GetDrawingRects(0).left)) ; 
}

bool CGUISlider::MsgProc(MSG* event)
{
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
	pt.x=m_event->m_mouse.x;
	pt.y=m_event->m_mouse.y;
	int nEvent=m_event->GetTriggerEvent();
	CEventBinding::InitMsg(&newMsg,event->time,nEvent,pt);
	if (m_bIsVisible&&!m_bCandrag){
		if (nEvent==EM_NONE) {
			return false;
		}
		if( m_bPressed&&(m_event->IsMapTo(nEvent,EM_MOUSE_MOVE)||m_event->IsMapTo(nEvent,EM_CTRL_FRAMEMOVE)) )
		{
			SetValueInternal( ValueFromPos( m_objResource->GetDrawingRects(0).left + pt.x + m_nDragOffset ) );
		}
		if (m_event->IsMapTo(nEvent,EM_SL_ACTIONBEGIN)) {
			newMsg.message=EM_CTRL_CAPTUREMOUSE;
			CGUIBase::MsgProc(&newMsg);
			if( PtInRect( m_objResource->GetDrawingRects(1), pt ) )
			{
				// Pressed while inside the control
				m_bPressed = true;

				m_nDragX = pt.x;
				m_nDragOffset = m_nButtonX - m_nDragX-RectWidth( m_objResource->GetDrawingRects(1))/2;


//				if( !m_bHasFocus&&m_parent )
//					((CGUIContainer*)m_parent)->SetKeyFocus( this );
				bHandled=true;
			}else if( PtInRect( m_objResource->GetDrawingRects(0), pt ) )
			{
				m_nDragX = pt.x;
				m_nDragOffset = 0;               
				//m_bPressed = true;

//				if( !m_bHasFocus&&m_parent )
//					((CGUIContainer*)m_parent)->SetKeyFocus( this );

				if( pt.x > m_nButtonX + m_objResource->GetDrawingRects(0).left )
				{
					SetValueInternal( m_nValue + ( 10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10 ) );
//						SetValueInternal( m_nValue + 1 );
					bHandled=true;
				}

				if( pt.x < m_nButtonX + m_objResource->GetDrawingRects(0).left )
				{
					SetValueInternal( m_nValue - ( 10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10 ) );
//						SetValueInternal( m_nValue - 1 );
					bHandled=true;
				}
			}
			if(bHandled)
			{
				OnMouseDown(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);
			}
		}else if (m_event->IsMapTo(nEvent,EM_SL_ACTIONEND)) {
			newMsg.message=EM_CTRL_RELEASEMOUSE;
			CGUIBase::MsgProc(&newMsg);
			if( m_bPressed )
			{
				m_bPressed = false;
				bHandled=true;

				OnMouseUp(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);
			}
		}
		if (m_event->IsMapTo(nEvent,EM_KEY)) {
			if ((event->lParam&0x80)!=0) {
				if (m_event->IsMapTo(nEvent,EM_KEY_UP)) {
					SetValueInternal( m_nValue+1); 
					bHandled=true;
				}else if (m_event->IsMapTo(nEvent,EM_KEY_RIGHT)) {
					SetValueInternal( m_nValue+1); 
					bHandled=true;
				}else if (m_event->IsMapTo(nEvent,EM_KEY_LEFT)) {
					SetValueInternal( m_nValue-1); 
					bHandled=true;
				}else if (m_event->IsMapTo(nEvent,EM_KEY_DOWN)) {
					SetValueInternal( m_nValue-1); 
					bHandled=true;
				}else if (m_event->IsMapTo(nEvent,EM_KEY_PAGE_DOWN)) {
					SetValueInternal( m_nValue - ( 10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10 ) );
					bHandled=true;
				}else if (m_event->IsMapTo(nEvent,EM_KEY_PAGE_UP)) {
					SetValueInternal( m_nValue + ( 10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10 ) );
					bHandled=true;
				}else if (m_event->IsMapTo(nEvent,EM_KEY_HOME)) {
					SetValueInternal(m_nMin);
					bHandled=true;
				}else if (m_event->IsMapTo(nEvent,EM_KEY_END)) {
					SetValueInternal(m_nMax);
					bHandled=true;
				}
			}
		}
		if (m_event->IsMapTo(nEvent,EM_MOUSE_WHEEL)) {
			int nScrollAmount = int(m_event->m_mouse.WheelDelta) / 120 * CGUIRoot::WheelScrollLines;
			SetValueInternal( m_nValue-nScrollAmount );
			bHandled=true;
		}

	}
	if (!bHandled) {
		MSG msg = m_event->GenerateMessage();
		return CGUIBase::MsgProc(&msg);
	}
	return bHandled;
}
bool CGUISlider::OnChange(const char* code)
{
	if( !HasEvent(EM_CTRL_CHANGE) )
		return false;
	return ActivateScript("",EM_CTRL_CHANGE);
}


//--------------------------------------------------------------------------------------
void CGUISlider::SetRange( int nMin, int nMax ) 
{
	m_nMin = nMin;
	m_nMax = nMax;

	SetValueInternal( m_nValue );
}


//--------------------------------------------------------------------------------------
void CGUISlider::SetValueInternal( int nValue )
{
	// Clamp to range
	nValue = Math::Max( m_nMin, nValue );
	nValue = Math::Min( m_nMax, nValue );
	int oldvalue=m_nValue;
	if( nValue == m_nValue )
		return;

	m_nValue = nValue;
	UpdateRects();

	if (m_Linked) {
		m_Linked->UpdateScroll(oldvalue-m_nValue,0);
	}
	if (m_nOldValue!=nValue){
		OnChange();
	}
	m_nOldValue=nValue;
}

void CGUISlider::SetValue( int nValue )
{
	// Clamp to range
	nValue = Math::Max( m_nMin, nValue );
	nValue = Math::Min( m_nMax, nValue );
	int oldvalue=m_nValue;
	if( nValue == m_nValue )
		return;

	m_nValue = nValue;
	UpdateRects();

	if (m_Linked) {
		m_Linked->UpdateScroll(oldvalue-m_nValue,0);
	}
	m_nOldValue=nValue;
}

//--------------------------------------------------------------------------------------
HRESULT CGUISlider::Render( GUIState* pGUIState,float fElapsedTime)
{
	int nOffsetX = 0;
	int nOffsetY = 0;
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();

	if( m_bIsVisible == false )
	{
		return S_OK;
	}
	else if( m_bIsEnabled == false )
	{
		m_objResource->SetCurrentState(GUIResourceState_Disabled);
	}
	else if( m_bPressed )
	{
		m_objResource->SetCurrentState(GUIResourceState_Pressed);

		nOffsetX = 1;
		nOffsetY = 2;
	}
	else if( m_bMouseOver )
	{
		m_objResource->SetCurrentState(GUIResourceState_Highlight);

		nOffsetX = -1;
		nOffsetY = -2;
	}
	


	//need to adjust the bar in order to fit the dragging button inside the rect
	GUITextureElement* pElement = m_objResource->GetTextureElement(0);
	RECT rcBar=m_objResource->GetDrawingRects(0);
	rcBar.left+=((rcBar.bottom-rcBar.top)>>1);
	rcBar.right-=((rcBar.bottom-rcBar.top)>>1);

	//m_objResource->DrawSprite( pElement, &rcBar,m_position.GetDepth() );

	if(m_objResource->GetLayerType() == GUILAYER::ONE_ELEMENT) 
	{
		DrawElement( pElement, &rcBar, &rcBar);
	}	
	else if(m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT) 
	{
		GUIResourceState StateRes = m_objResource->GetCurrentState();
		// tricky: LiXizhi 2007.9.27: we will only use the normal state texture, but with the color of the other state's index 0 texture
		LinearColor ColorRes(0xffffffff);
		if(pElement != NULL)
			ColorRes = pElement->TextureColor;
		m_objResource->SetCurrentState(GUIResourceState_Normal);
		for( int e = 0; e < 9; ++e )
		{
			pElement = m_objResource->GetTextureElement(e+2);
			if(pElement)
			{
				LinearColor tmpColor = pElement->TextureColor;
				pElement->TextureColor = ColorRes;
				RECT rc = m_objResource->GetDrawingRects(e + 2);
				DrawElement(  pElement, &rc, &rcBar);
				pElement->TextureColor = tmpColor;
			}
		}
		m_objResource->SetCurrentState(StateRes);
	}

	pElement = m_objResource->GetTextureElement(1);

	RECT rc = m_objResource->GetDrawingRects(1);
	DrawElement(pElement,  &rc, NULL);
	return S_OK;
}

