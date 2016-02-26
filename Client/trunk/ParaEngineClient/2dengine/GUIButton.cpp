//----------------------------------------------------------------------
// Class:	CGUIButton
// Authors:	LiXizhi, Liu Weili
// Date:	2005.8.3
// Revised: 2005.8.3
//
// desc: 
// The button control provides drag-and-drop feature
//
// By default, the button control cannot have focus and cannot be dragged.
//----------------------------------------------------------------------
#include "ParaEngine.h"
#include "GUIButton.h"
#include "GUIContainer.h"
#include "GUIResource.h"
#include "AISimulator.h"
#include "ParaWorldAsset.h"
#include "ic/ICConfigManager.h"
#include "GUIRoot.h"
#include "GUIDirectInput.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "util/StringHelper.h"
#include "memdebug.h"

using namespace ParaEngine;

/** @def if this is defined. highlighting will be disabled automatically when a button is clicked.*/
#define AUTO_DISABLE_HIGHLIGHTING

/** max button animation time in milliseconds. */
const int MaxMilliSeconds = 200;

/** the inflate percentage for button style 1 and 2. */
const float Level1_InflatePercentage = 0.05f;

/** the rotation speed for button style 3 and 4.*/
const float Level1_RotateSpeed = 0.5f;

const IType* CGUIButton::m_type=NULL;




CGUIButton::CGUIButton():
	m_text_scale(1.f), m_nPadding(0), m_nAnimationStyle(0), m_nAnimationData(0),m_bIsDefaultButton(false), m_bPressed(false)
{
	if (!m_type){
		m_type=IType::GetType("guibutton");
	}
}

CGUIButton::~CGUIButton()
{}

void CGUIButton::Reset()
{
	CGUIBase::Reset();
	m_bPressed=false;
}

void CGUIButton::Clone(IObject* pobj)const
{
	PE_ASSERT(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}
	
	CGUIButton* pButton=(CGUIButton*)pobj;
	CGUIBase::Clone(pButton);
	pButton->m_bPressed=m_bPressed;
	pButton->m_nPadding = m_nPadding;
	pButton->m_nAnimationStyle = m_nAnimationStyle;
	pButton->m_szText=m_szText;
}

IObject* CGUIButton::Clone()const
{
	CGUIButton *pButton=new CGUIButton();
	Clone(pButton);
	return pButton;
}

int CGUIButton::GetAnimationStyle()
{
	return m_nAnimationStyle;
}
void CGUIButton::SetAnimationStyle(int nStyle)
{
	if(m_nAnimationStyle!=nStyle)
	{
		m_nAnimationStyle = nStyle;
		m_nAnimationData = 0;
		
		if(m_nAnimationStyle>=1 && m_nAnimationStyle<=5)
		{
			// gradually enlarge 5%, 10%, ... when getting focus
		}
		else if(m_nAnimationStyle>=11 && m_nAnimationStyle<=15)
		{
			// same as animation style 1-5, except that highlighting is disabled. 
			GUITextureElement *pElement = NULL;
			m_objResource->SetCurrentState(GUIResourceState_Normal);
			pElement = m_objResource->GetTextureElement("background");
			if(pElement)
				pElement->TextureColor=0xffffffff;
		}
		else if(m_nAnimationStyle>=21 && m_nAnimationStyle<=25)
		{
			// same as animation style 1-5, except that highlighting is disabled and when highlighted the animation will not stop. 
			GUITextureElement *pElement = NULL;
			m_objResource->SetCurrentState(GUIResourceState_Normal);
			pElement = m_objResource->GetTextureElement("background");
			if(pElement)
				pElement->TextureColor=0xffffffff;
		}

	}
}

void CGUIButton::StaticInit()
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIBase")) {
		CGUIBase::StaticInit();
	}
	//load the default CGUIBase object and copy all its value to the new button
	CGUIButton *pButton=new CGUIButton();
	pOm->CloneObject("default_CGUIBase",pButton);
// 	pButton->m_objType=CGUIBase::Type_GUIButton;
	pButton->m_szText.clear();
	pButton->m_bPressed=false;

	CEventBinding* pBinding=pButton->m_event->GetEventBindingObj();
	pBinding->DisableKeyboard();
	pBinding->EnableMouse();
	//in button, i define that a click on the button will get the button key focus
//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_CTRL_CAPTUREMOUSE);
//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_CTRL_RELEASEMOUSE);
//	pBinding->MapEvent(EM_MOUSE_LEFTCLICK,EM_BTN_CLICK);
//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_BTN_DOWN);
//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_BTN_UP);
	//pBinding->DisableEvent(EM_MOUSE_WHEEL); // LXZ 2007.9.19: this means that scrollable event will leak to its parent container.
	pBinding->DisableEvent(EM_MOUSE_DBCLICK); // this means that ondoubleclick is not available to button
	

	using namespace ParaInfoCenter;
	CICConfigManager *cm=CGlobals::GetICConfigManager();
	string value0,value1;
	int event0,event1,a;
	DWORD b;
	HRESULT hr;
	hr=cm->GetSize("GUI_button_control_mapping",&b);
	if (hr==E_INVALIDARG||hr==E_ACCESSDENIED) {
		//error
	}else{
		for (a=0;a<(int)b;a+=2) {
			hr=cm->GetTextValue("GUI_button_control_mapping",value0,a);
			if (FAILED(hr)) {
				break;
			}
			hr=cm->GetTextValue("GUI_button_control_mapping",value1,a+1);
			if (FAILED(hr)) {
				break;
			}
			event0=CEventBinding::StringToEventValue(value0);
			event1=CEventBinding::StringToEventValue(value1);
			pBinding->MapEvent(event0,event1);
		}
	}
	int tempint;
	if (cm->GetIntValue("GUI_button_control_visible",&tempint)==S_OK) {
		if (tempint==0) {
			pButton->m_bIsVisible=false;
		}else
			pButton->m_bIsVisible=true;
	}
	if (cm->GetIntValue("GUI_button_control_enable",&tempint)==S_OK) {
		if (tempint==0) {
			pButton->m_bIsEnabled=false;
		}else
			pButton->m_bIsEnabled=true;
	}
	if (cm->GetIntValue("GUI_button_control_canhasfocus",&tempint)==S_OK) {
		if (tempint==0) {
			pButton->m_bCanHasFocus=false;
		}else
			pButton->m_bCanHasFocus=true;
	}
	if (cm->GetIntValue("GUI_button_control_candrag",&tempint)==S_OK) {
		if (tempint==0) {
			pButton->SetCandrag(false);
		}else
			pButton->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_button_control_lifetime",&tempint)==S_OK) {
		pButton->m_nLifeTimeCountDown=tempint;
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
	if (cm->GetTextValue("GUI_button_control_background",background)==S_OK) {
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
	pButton->m_objResource->SetActiveLayer();
	pButton->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);

	pButton->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,230,230,230));
	pButton->m_objResource->AddElement(pElement,"background");

	pButton->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,160,160,160));
	pButton->m_objResource->AddElement(pElement,"background");

	pButton->m_objResource->SetCurrentState(GUIResourceState_Highlight);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	pButton->m_objResource->AddElement(pElement, "background");

	pButton->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pElement->SetElement(pTexture,prect,COLOR_ARGB(128,255,255,255));
	pButton->m_objResource->AddElement(pElement,"background");

	pFontElement->SetElement(pFont,COLOR_ARGB(255,0,0,0),DT_CENTER|DT_VCENTER);
	pButton->m_objResource->SetCurrentState(GUIResourceState_Normal);
	pButton->m_objResource->AddElement(pFontElement,"text");
	pButton->m_objResource->SetCurrentState(GUIResourceState_Pressed);
	pButton->m_objResource->AddElement(pFontElement,"text");
	pButton->m_objResource->SetCurrentState(GUIResourceState_Highlight);
	pButton->m_objResource->AddElement(pFontElement,"text");
	pButton->m_objResource->SetCurrentState(GUIResourceState_Disabled);
	pButton->m_objResource->AddElement(pFontElement,"text");

	pButton->m_objResource->SetCurrentState();
	pButton->m_objResource->SetActiveLayer();
	
	pOm->SetObject("default_CGUIButton",pButton);
	SAFE_RELEASE(pButton);
}

bool CGUIButton::OnMouseLeave()
{
	m_bPressed=false;
	return CGUIBase::OnMouseLeave();
}

bool CGUIButton::OnMouseEnter()
{
	return CGUIBase::OnMouseEnter();
}

//if return false, the message can be handled onto other message handler.
bool CGUIButton::MsgProc(MSG *event)
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
		if (m_event->IsMapTo(nEvent,EM_BTN_DOWN)) {
			newMsg.message=EM_CTRL_CAPTUREMOUSE;
			CGUIBase::MsgProc(&newMsg);
			m_bPressed=true;
			bHandled=true;
		}else if (m_event->IsMapTo(nEvent,EM_BTN_UP)) {
			newMsg.message=EM_CTRL_RELEASEMOUSE;
			CGUIBase::MsgProc(&newMsg);
			m_bPressed=false;
			bHandled=true;
		}else if (m_event->IsMapTo(nEvent,EM_BTN_CLICK)) {
			newMsg.message=EM_CTRL_RELEASEMOUSE;
			CGUIBase::MsgProc(&newMsg);
			OnClick(nEvent,m_event->m_mouse.x,m_event->m_mouse.y);
#ifdef	AUTO_DISABLE_HIGHLIGHTING
			if(m_eHighlight>0)
				m_eHighlight = 0;
#endif
			m_bPressed=false;
			bHandled=true;
		}else if (m_event->IsMapTo(nEvent,EM_MOUSE_DRAGBEGIN)) {
			m_bPressed=false;
		}
	}

	if (!bHandled) {
		MSG msg = m_event->GenerateMessage();
		return CGUIBase::MsgProc(&msg);
	}
	return true;
}

string CGUIButton::ToScript(int option)
{
	string script=CGUIBase::ToScript(option);
	//add "button1:SetText("È·¶¨");"like script

	std::string buf;
	int length = GetTextA(buf);
	if(length>0){
		script+="__this.text=\"";
		script+=buf;
		script+="\";\n";
	}
	return script;
}

HRESULT CGUIButton::Render(GUIState* pGUIState ,float fElapsedTime)
{	
	SetDirty(false);
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();
	if (!m_bIsEnabled) {
		m_objResource->SetCurrentState(GUIResourceState_Disabled);
	}
	else if( m_bPressed )
	{
		m_objResource->SetCurrentState(GUIResourceState_Pressed);

	}
	else if( m_bMouseOver )
	{
		m_objResource->SetCurrentState(GUIResourceState_Highlight);
	}

	if (m_position.IsRelativeTo3DObject()) 
	{
		BeginDepthTest(pGUIState);
		UpdateRects();
	}

	GUITextureElement* pElement = NULL;
	GUIFontElement* pFontElement = NULL;
	RECT rcWindow = m_objResource->GetDrawingRects(0);

	//////////////////////////////////////////////////////////////////////////
	//
	// render the background layer if any. This is very rare, only some very special buttons has this layer for special visual effect.
	//
	//////////////////////////////////////////////////////////////////////////

	if(m_objResource->HasLayer(CGUIResource::LAYER_BACKGROUND))
	{
		m_objResource->SetActiveLayer(CGUIResource::LAYER_BACKGROUND);
		m_objResource->SetCurrentState();
		if (!m_bIsEnabled) {
			m_objResource->SetCurrentState(GUIResourceState_Disabled);
		}
		else if( m_bPressed )
		{
			m_objResource->SetCurrentState(GUIResourceState_Pressed);
		}
		else if( m_bMouseOver )
		{
			m_objResource->SetCurrentState(GUIResourceState_Highlight);
		}

		// using the default background texture
		pElement = m_objResource->GetTextureElement(0);

		// background layer will use no animation style, so we will use the drawing rect of the default artwork layer.
		

		// render the layer
		if(m_objResource->GetLayerType() == GUILAYER::ONE_ELEMENT) 
		{
			DrawElement( pElement, &rcWindow,&rcWindow, pGUIState);
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
				pElement = m_objResource->GetTextureElement(e);
				if(pElement)
				{
					LinearColor tmpColor = pElement->TextureColor;
					pElement->TextureColor = ColorRes;
					RECT rect = m_objResource->GetDrawingRects(e + 2);
					DrawElement(pElement, &rect, &rcWindow, pGUIState);
					pElement->TextureColor = tmpColor;
				}
			}
			m_objResource->SetCurrentState(StateRes);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// render the artwork layer
	//
	//////////////////////////////////////////////////////////////////////////
	m_objResource->SetActiveLayer();
	
	// using the default background texture
	pElement = m_objResource->GetTextureElement(0);
	pFontElement= m_objResource->GetFontElement(0);

	rcWindow = m_objResource->GetDrawingRects(0);
	int nPadding = GetPadding();
	if(nPadding > 0)
	{
		InflateRect( &rcWindow, -nPadding, -nPadding);
	}

	int nBaseStyle = m_nAnimationStyle % 10; 
	int nSecondStyle = (m_nAnimationStyle-nBaseStyle)/10;
	
	bool bResetScaling = false;
	
	if(m_nAnimationStyle==0 || 
		!GetEnabled()/* for disabled buttons, always use animation style 0 */ )
	{
		// no animations
	}
	else if( nSecondStyle>=0 && nSecondStyle<=2)
	{
		// play scaling animations
		if(nBaseStyle>=1 && nBaseStyle<=9)
		{
			// enlarge by 5%
			float MaxInflatePercentage = Level1_InflatePercentage*nBaseStyle;
			int nInflatePixelX = 0;
			int nInflatePixelY = 0;
			float fMilliSeconds = 0.f;
			if(m_bMouseOver)
			{
				if(nSecondStyle==2)
				{
					// this value controls how fast the animation is.
					const float fSpeedFactor = 0.6f;
					//////////////////////////////////////////////////////////////////////////
					// animation does not stop during inflateMode for animation style 21-25
					// [0, MaxMilliSeconds] is normal inflation.
					// [MaxMilliSeconds, MaxMilliSeconds*1.5] is deflation.
					// [MaxMilliSeconds*1.5, MaxMilliSeconds*2] is inflation.
					// MaxMilliSeconds*2 will roll back to MaxMilliSeconds which begins another deflation.
					//////////////////////////////////////////////////////////////////////////
					m_nAnimationData = (int)(m_nAnimationData + (fSpeedFactor*fElapsedTime*1000));
					if(m_nAnimationData > 2*MaxMilliSeconds)
						m_nAnimationData = m_nAnimationData - MaxMilliSeconds;
					if(m_nAnimationData<=MaxMilliSeconds)
						fMilliSeconds = (float)m_nAnimationData;
					else
					{
						fMilliSeconds =  (float)(m_nAnimationData - MaxMilliSeconds);
						if(fMilliSeconds<MaxMilliSeconds/2)
							fMilliSeconds = MaxMilliSeconds - fMilliSeconds;
					}
				}
				else
				{
					fMilliSeconds = m_nAnimationData + (fElapsedTime*1000);
					m_nAnimationData = (int)fMilliSeconds;
					if(m_nAnimationData>MaxMilliSeconds)
						m_nAnimationData = MaxMilliSeconds;
				}
			}
			else
			{
				if(nSecondStyle==2)
				{
					if(m_nAnimationData>MaxMilliSeconds)
					{
						m_nAnimationData -= MaxMilliSeconds;
						if(m_nAnimationData < MaxMilliSeconds/2)
							m_nAnimationData = (MaxMilliSeconds - m_nAnimationData);
					}
				}
				fMilliSeconds =  m_nAnimationData - (fElapsedTime*1000);
				m_nAnimationData = (int)fMilliSeconds;
				if(m_nAnimationData<0)
					m_nAnimationData = 0;
			}
			if(fMilliSeconds>MaxMilliSeconds)
				fMilliSeconds = MaxMilliSeconds;
			if(fMilliSeconds<0)
				fMilliSeconds = 0;

			SetScaling(Vector2(1.f + (MaxInflatePercentage*fMilliSeconds/(float)MaxMilliSeconds), 1.f + (MaxInflatePercentage*fMilliSeconds/(float)MaxMilliSeconds)));
			bResetScaling = true;
		}
	}
	else if( nSecondStyle>=3 && nSecondStyle<=4)
	{
		// 3x is clock-wise rotation , 4x is counter-clock-wise rotation. 
		float fRotSpeed = Level1_RotateSpeed*nBaseStyle;
		if(m_nAnimationData>6283)
		{
			// 3.14*2*1000
			m_nAnimationData -= 6283;
		}
		m_nAnimationData = (int)(m_nAnimationData + (fRotSpeed*fElapsedTime*1000));
		SetRotation((3.5f-nSecondStyle)*2.f*m_nAnimationData/1000.f);
	}
	
	if(m_objResource->GetLayerType() == GUILAYER::ONE_ELEMENT) 
	{
		DrawElement( pElement, &rcWindow, &rcWindow, pGUIState);
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
			pElement = m_objResource->GetTextureElement(e);
			if(pElement)
			{
				LinearColor tmpColor = pElement->TextureColor;
				pElement->TextureColor = ColorRes;
				RECT rect = m_objResource->GetDrawingRects(e + 2);
				DrawElement(  pElement, &rect, &rcWindow, pGUIState);
				pElement->TextureColor = tmpColor;
			}
		}
		m_objResource->SetCurrentState(StateRes);
	}
		
	//////////////////////////////////////////////////////////////////////////
	//
	// render the button text
	//
	//////////////////////////////////////////////////////////////////////////
	
	RECT rcText = rcWindow;
	InflateRect( &rcText, -m_nSpacing, -m_nSpacing );
	rcText.top += m_textOffsetY;
	rcText.bottom += m_textOffsetY;

	rcText.left += m_textOffsetX;
	rcText.right += m_textOffsetX;
	DrawText( m_szText.c_str(), pFontElement, &rcText,&rcWindow, m_bUseTextShadow,-1,m_textShadowQuality,m_textShadowColor, pGUIState);
	
	if(bResetScaling)
		SetScaling(Vector2(1.f,1.f));

	if (m_position.IsRelativeTo3DObject()) 
	{
		EndDepthTest(pGUIState);
	}

	OnFrameMove(fElapsedTime);
	return S_OK;
}


void CGUIButton::SetTextScale(float fScale)
{
	m_text_scale = fScale;
}

float CGUIButton::GetTextScale()
{
	return m_text_scale;
}

void CGUIButton::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIButton")) {
		CGUIButton::StaticInit();
	}
	pOm->CloneObject("default_CGUIButton",this);
	CGUIBase::InitObject(strObjectName,alignment,x,y,width,height);
}

void CGUIButton::SetText(const char16_t* wszText)
{
	PE_ASSERT( wszText != NULL );
	m_szText=wszText;
}

void CGUIButton::SetTextA(const char* szText)
{
	PE_ASSERT( szText != NULL );
	StringHelper::UTF8ToUTF16_Safe(szText, m_szText);
}

int CGUIButton::GetTextA(std::string& out)
{
	return StringHelper::UTF16ToUTF8(m_szText, out) ? (int) out.size() : 0;
}
void CGUIButton::UpdateRects()
{
	// LXZ 2007.11.10 bug corrected: when calling UpdateRects to an existing button when mouse is over it, the 9-tile rect rect is wrong. I corrected by setting to artworklayer and normal state before calling UpdateRects().
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
	m_objResource->SetDrawingRects(&tempPos.rect,1);
	
	int nLayerCount = 0;
	do 
	{
		if(nLayerCount == 1)
			m_objResource->SetActiveLayer(CGUIResource::LAYER_BACKGROUND);
		if(m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT)
		{
			const int offset = 2;
			RECT tempRect;
			RECT bgRect = tempPos.rect;
			int nPadding = GetPadding();
			if(nLayerCount == 0 && nPadding>0)
			{
				InflateRect( &bgRect, -nPadding, -nPadding);
			}
			// use the left top and right bottom texture element size to calculate the inner rect size. 
			GUITextureElement* pElement = m_objResource->GetTextureElement(1);
			if(pElement)
			{
				pElement->GetRect(&tempRect);
				rcInner.left = RectWidth(tempRect) + bgRect.left;
				rcInner.top = RectHeight(tempRect) + bgRect.top;
			}
			pElement = m_objResource->GetTextureElement(8);
			if(pElement)
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
			m_objResource->SetDrawingRects(&rcInner,offset+0);
			SetRect( &tempRect, bgRect.left, bgRect.top, rcInner.left, rcInner.top );
			m_objResource->SetDrawingRects(&tempRect,offset+1);
			SetRect( &tempRect, rcInner.left, bgRect.top, rcInner.right, rcInner.top );
			m_objResource->SetDrawingRects(&tempRect,offset+2);
			SetRect( &tempRect, rcInner.right, bgRect.top, bgRect.right, rcInner.top );
			m_objResource->SetDrawingRects(&tempRect,offset+3);
			SetRect( &tempRect, bgRect.left, rcInner.top, rcInner.left, rcInner.bottom );
			m_objResource->SetDrawingRects(&tempRect,offset+4);
			SetRect( &tempRect, rcInner.right, rcInner.top, bgRect.right, rcInner.bottom );
			m_objResource->SetDrawingRects(&tempRect,offset+5);
			SetRect( &tempRect, bgRect.left, rcInner.bottom, rcInner.left, bgRect.bottom );
			m_objResource->SetDrawingRects(&tempRect,offset+6);
			SetRect( &tempRect, rcInner.left, rcInner.bottom, rcInner.right, bgRect.bottom );
			m_objResource->SetDrawingRects(&tempRect,offset+7);
			SetRect( &tempRect, rcInner.right, rcInner.bottom, bgRect.right, bgRect.bottom );
			m_objResource->SetDrawingRects(&tempRect,offset+8);
		}
		if(nLayerCount == 1)
			m_objResource->SetActiveLayer(CGUIResource::LAYER_ARTWORK);
	} while((++nLayerCount)<2 && m_objResource->HasLayer(CGUIResource::LAYER_BACKGROUND));

	m_bNeedUpdate=false;
}

void ParaEngine::CGUIButton::SetDefaultButton( bool bDefault )
{
	// unset old default button. 
	if(bDefault && m_parent)
	{
		CGUIButton* pOldButton = m_parent->GetDefaultButton();
		if(pOldButton!=0 && pOldButton!=this)
		{
			pOldButton->SetDefaultButton(false);
		}
	}
	m_bIsDefaultButton = bDefault;
}

void ParaEngine::CGUIButton::SetPadding( int nPadding )
{
	m_nPadding = nPadding;
	m_bNeedUpdate = true; 
	SetDirty(true);
}

int ParaEngine::CGUIButton::GetPadding()
{
	return m_nPadding;
}

int ParaEngine::CGUIButton::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	CGUIBase::InstallFields(pClass, bOverride);
	PE_ASSERT(pClass != NULL);
	pClass->AddField("OnRenderScript", FieldType_String, (void*)SetOnRenderScript_s, (void*)GetOnRenderScript_s, NULL, NULL, bOverride);
	return S_OK;
}

const std::string& ParaEngine::CGUIButton::GetOnRenderScript()
{
	const SimpleScript* pScript = GetEventScript(EM_ONRENDER);
	if (pScript){
		return pScript->szCode;
	}
	else{
		return CGlobals::GetString(0);
	}
}

void ParaEngine::CGUIButton::SetOnRenderScript(const char* sScript)
{
	if (sScript == NULL || sScript[0] == '\0')
	{
		SetEventScript(EM_ONRENDER, NULL);
	}
	else
	{
		string temp(sScript);
		SimpleScript script;
		ParaEngine::StringHelper::DevideString(temp, script.szFile, script.szCode);
		SetEventScript(EM_ONRENDER, &script);
	}
}

HRESULT ParaEngine::CGUIButton::OnRenderExternal(GUIState* pGUIState, float fElapsedTime)
{
	return S_OK;
}
