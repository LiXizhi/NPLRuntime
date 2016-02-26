//----------------------------------------------------------------------
// Class:	CGUIToolTip
// Authors:	Liu Weili, LiXizhi
// Date:	2005.12.13
// Revised: 2008.6.18 LiXizhi. Added mili-seconds delay
//  refactored by LiXizhi 2009.1.9, added attributes to config.lua
//
// desc: 
// 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "GUIRoot.h"
#include "GUIToolTip.h"
#include "GUIResource.h"
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "PaintEngine/Painter.h"
#include "util/StringHelper.h"
#include "GUIDirectInput.h"
#include "ic/ICConfigManager.h"
#include "memdebug.h"

/** @def display just above the cursor, positioned in the middle. */
//#define DISPLAY_ON_CURSOR_TOP




using namespace ParaEngine;
const IType* CGUIToolTip::m_type=NULL;
CGUIToolTip::CGUIToolTip():CGUIBase()
{
	Initialize();
}

CGUIToolTip::~CGUIToolTip()
{
	m_items.clear();
}
void CGUIToolTip::Initialize()
{
	m_ActiveTip=NULL;
	m_nAutoPopDelay=3000;
	m_nInitialDelay=0;
	m_nBehavior=0;
	m_nAutomaticDelay=0;
	m_nSpacing=8;
	m_nIndicator=0;
	m_items.clear();
	if (!m_type){
		m_type=IType::GetType("guitooltip");
	}
// 	m_objType=CGUIBase::GUIToolTip;
}
void CGUIToolTip::ActivateTip(CGUIBase* obj)
{
	if (obj==NULL||m_items.find(obj)==m_items.end()) {
		return;
	}
	if (m_ActiveTip) {
		if ((m_ActiveTip->m_behavior&HIGHTLIGHT)==0) {
			if(m_ActiveTip->m_obj!=obj)
				DeactivateTip(NULL);
			else
				return;
		}else
			return;
	}
	m_ActiveTip=&m_items[obj];
	m_ActiveTip->m_show=true;
	m_ActiveTip->m_rendered=false;
	m_ActiveTip->m_state=GUIToolTipItem::BEFORESHOW;
}

void CGUIToolTip::DeactivateTip(CGUIBase* obj)
{
	if (m_ActiveTip==NULL) {
		return;
	}
	if ((m_ActiveTip->m_behavior&HIGHTLIGHT)==0){
		if ((obj==NULL)||(obj!=NULL&&m_ActiveTip->m_obj==obj)) {
			m_ActiveTip->m_show=false;
			m_ActiveTip->m_timecount=0;
			m_ActiveTip->m_rendered=false;
			m_ActiveTip=NULL;
		}

	}else{
		if (m_ActiveTip->m_obj==obj) {
			m_ActiveTip=NULL;
			RemoveTip(obj);
		}
	}
}

const char16_t* CGUIToolTip::GetToolTip(CGUIBase* obj)
{
	if (obj==NULL||m_items.find(obj)==m_items.end()) {
		return NULL;
	}
	return m_items[obj].m_text.c_str();
}

int CGUIToolTip::GetToolTipA(CGUIBase* obj, std::string& out)
{
	if (obj==NULL) {
		return 0;
	}
	const char16_t* str = GetToolTip(obj);
	if (str)
		return StringHelper::UTF16ToUTF8(str, out) ? (int)out.size() : 0;
	else
		return 0;
}

void CGUIToolTip::SetToolTip(CGUIBase* obj,const char16_t* text)
{
	if (obj==NULL) {
		return;
	}
	//if text is empty, the tooltip of the object will be disabled
	u16string szText = text;
	if (szText.empty()) 
	{
		RemoveTip(obj);
		return;
	}
	m_items[obj].m_autopopdelay=m_nAutoPopDelay;
	m_items[obj].m_behavior=m_nBehavior;
	m_items[obj].m_initialdelay=m_nInitialDelay;
	m_items[obj].m_obj=obj;
	m_items[obj].m_text = szText;
	m_items[obj].m_timecount=0;
	m_items[obj].m_rendered=false;
	m_items[obj].m_show=false;
	RECT rcText;
	SetRect(&rcText,0,0,0,0);
	GUIFontElement *pFontElement=m_objResource->GetFontElement(0);
	obj->GetPainter()->CalcTextRect(text, pFontElement, &rcText, obj->GetPosition()->GetDepth());
	OffsetRect(&rcText,-rcText.left,-rcText.top);
	if ((pFontElement->dwTextFormat&DT_WORDBREAK)!=0) {
		rcText.right+=8;
		rcText.bottom+=6;
	}
	if ((pFontElement->dwTextFormat&DT_WORDBREAK)==0&&(pFontElement->dwTextFormat&DT_SINGLELINE)!=0) {
		rcText.right+=6;
		rcText.bottom+=6;
	}
	m_items[obj].m_rect=rcText;

	//if highlight
	if ((m_nBehavior&HIGHTLIGHT)!=0) {
		m_items[obj].m_initialdelay=0;
		m_items[obj].m_autopopdelay=-1;
		m_items[obj].m_show=true;
		m_items[obj].m_rendered=true;
		m_items[obj].m_behavior|=FLASHING;
	}
}

void CGUIToolTip::SetToolTipA(CGUIBase* obj,const char* szText)
{
	PE_ASSERT( szText != NULL );
	u16string out;

	StringHelper::UTF8ToUTF16_Safe(szText, out);
	SetToolTip(obj, out.c_str());
}

GUIToolTipItem* CGUIToolTip::GetActiveTip()
{
	return m_ActiveTip;
}

void CGUIToolTip::RemoveAll()
{
	m_items.clear();
}

void CGUIToolTip::RemoveTip(CGUIBase* obj)
{
	if (obj==NULL||m_items.find(obj)==m_items.end()) {
		return;
	}
	DeactivateTip(obj);
	m_items.erase(obj);
}

HRESULT CGUIToolTip::Render(GUIState* pGUIState ,float fElapsedTime)
{
	SetDirty(false);
	if (m_ActiveTip==NULL) {
		return S_OK;
	}
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();
	CheckTipState(fElapsedTime);
	bool bRender=false;
	switch(m_ActiveTip->m_state) {
	case (int)GUIToolTipItem::SHOWING:
		bRender=true;
		m_objResource->SetCurrentState(GUIResourceState_Normal);
		break;
	case (int)GUIToolTipItem::BLINKING:
		bRender=true;
		m_objResource->SetCurrentState(GUIResourceState_Highlight);
		break;
	}
	if (bRender&&m_ActiveTip) 
	{
		if (m_ActiveTip->m_obj->GetPosition()->IsRelativeTo3DObject()) 
		{
			BeginDepthTest(pGUIState);
			UpdateRects();
		}

		GUITextureElement* pElement = NULL;
		GUIFontElement* pFontElement = NULL;

		RECT rcWindow = m_objResource->GetDrawingRects(0);
		InflateRect( &rcWindow, m_nSpacing, m_nSpacing);
		
		for (int i=0;i<9;++i)
		{
			pElement = m_objResource->GetTextureElement(i);
			RECT rc = m_objResource->GetDrawingRects(i + 1);
			DrawElement(  pElement, &rc,&rcWindow);
		}
		RECT rc = m_objResource->GetDrawingRects(0);
		DrawText( m_ActiveTip->m_text.c_str(), m_objResource->GetFontElement(0), &rc,&rcWindow, false);
		
		if (m_ActiveTip->m_obj->GetPosition()->IsRelativeTo3DObject()) 
		{
			EndDepthTest(pGUIState);
		}

	}
	return S_OK;
}

void CGUIToolTip::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	using namespace ParaInfoCenter;
	CICConfigManager *cm=CGlobals::GetICConfigManager();

	CGUIBase::InitObject(strObjectName,alignment,x,y,width,height);
	TextureEntity* pTexture=NULL;
	GUITextureElement elem;
	GUITextureElement* pElement=&elem;

	SpriteFontEntity* pFont = NULL;
	GUIFontElement font_;
	GUIFontElement* pFontElement=&font_;
	string sFont;
	if (cm->GetTextValue("CGUIToolTip_font",sFont)==S_OK) 
	{
		string sFontname,sFontsize,sIsbold;
		int fontsize = 11;
		bool isbold = false;
		StringHelper::DevideString(sFont,sFontname,sFontsize);
		string temp = sFontsize;
		StringHelper::DevideString(temp,sFontsize,sIsbold);
		sscanf(sFontsize.c_str(), "%d", &fontsize);
		isbold = (sIsbold.find("bold")!=string::npos);

		string sFontKeyName=sFontname;
		sFontKeyName+=";";
		char tempSize[30];
		itoa(fontsize,tempSize,10);
		sFontKeyName+=tempSize;
		sFontKeyName+=";";
		sFontKeyName+= (isbold) ? "bold":"norm";
		
		if ((pFont=CGlobals::GetAssetManager()->GetFont(sFontKeyName.c_str()))==NULL) 
		{
			pFont=CGlobals::GetAssetManager()->LoadGDIFont(sFontKeyName,sFontname,fontsize,isbold);
		}
	}
	if(pFont == NULL)
	{
		pFont=CGlobals::GetAssetManager()->GetFont("sys");
	}

	m_objResource->SetLayerType(GUILAYER::NINE_ELEMENT);
	m_objResource->SetCurrentState(GUIResourceState_Normal);

	int nPadding=0;
	if (cm->GetIntValue("CGUIToolTip_padding",&nPadding)==S_OK) 
	{
		m_nSpacing = nPadding;
	}

	DWORD dwFontColor = COLOR_ARGB(255,0,0,0);
	string sFontcolor;
	if (cm->GetTextValue("CGUIToolTip_fontcolor",sFontcolor)==S_OK) 
	{
		int r=255,g=255,b=255,a=255;		
		int nCount = sscanf(sFontcolor.c_str(), "%d %d %d %d", &r,&g,&b,&a);
		dwFontColor = COLOR_ARGB(a,r,g,b);
	}
	
	//  please note: it must be a 9 tile image with image rect and inner rect explicitly specified. 
	string background;
	if (cm->GetTextValue("CGUIToolTip_background",background)!=S_OK) 
	{
		background = "Texture/UnCheckBox2.png;0 0 32 32:6 6 6 6";
	}
	string temp = background;
	int nInnerRectIndex = 0;
	for (int i=0;i<(int)(background.size()); ++i)
	{
		if(background[i] == ':')
		{
			nInnerRectIndex = i;
			break;
		}
	}
	if(nInnerRectIndex == 0)
	{
		OUTPUT_LOG("warning: CGUIToolTip_background: it must be a 9 tile image with image rect and inner rect explicitly specified \n");
	}
	else
	{
		// nice element BG
		int left, top, toRight, toBottom;
		if(sscanf(&(background[nInnerRectIndex+1]), "%d %d %d %d", &left, &top, &toRight, &toBottom)==4)
		{
			temp.resize(nInnerRectIndex);
			
			string filename;
			RECT rect;
			memset(&rect, 0, sizeof(RECT));
			StringHelper::GetImageAndRect(temp,filename, &rect);
			pTexture = CGlobals::GetAssetManager()->LoadTexture("",filename,TextureEntity::StaticTexture);

			if(rect.right == 0)
			{
				rect.right = 32;
				rect.bottom = 32;
			}
			RECT rcInner;
			rcInner.left = rect.left +left;
			rcInner.top = rect.top +top;
			rcInner.right = rect.right - toRight;
			rcInner.bottom = rect.bottom - toBottom;
			GUITextureElement TexElement;

			// use original (default) color
			Color color = 0xffffffff;

			int textureOffset = 0;
			

			for (int i=0;i<2;++i)
			{
				if(i==0)
					m_objResource->SetCurrentState(GUIResourceState_Normal);
				else if(i==1)
					m_objResource->SetCurrentState(GUIResourceState_Highlight);

				// 9 tile textures
				RECT rcTexture = rcInner;
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_objResource->SetElement(&TexElement, textureOffset+0);

				SetRect( &rcTexture, rect.left, rect.top, rcInner.left, rcInner.top );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_objResource->SetElement(&TexElement, textureOffset+1);

				SetRect( &rcTexture, rcInner.left, rect.top, rcInner.right, rcInner.top );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_objResource->SetElement(&TexElement, textureOffset+2);

				SetRect( &rcTexture, rcInner.right, rect.top, rect.right, rcInner.top );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_objResource->SetElement(&TexElement, textureOffset+3);

				SetRect( &rcTexture, rect.left, rcInner.top, rcInner.left, rcInner.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_objResource->SetElement(&TexElement, textureOffset+4);

				SetRect( &rcTexture,rcInner.right, rcInner.top, rect.right, rcInner.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_objResource->SetElement(&TexElement, textureOffset+5);

				SetRect( &rcTexture, rect.left, rcInner.bottom, rcInner.left, rect.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_objResource->SetElement(&TexElement, textureOffset+6);

				SetRect( &rcTexture, rcInner.left, rcInner.bottom, rcInner.right, rect.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_objResource->SetElement(&TexElement, textureOffset+7);

				SetRect( &rcTexture, rcInner.right, rcInner.bottom, rect.right, rect.bottom );
				TexElement.SetElement(pTexture,&rcTexture,color);
				m_objResource->SetElement(&TexElement, textureOffset+8);

				// font
				pFontElement->SetElement(pFont,dwFontColor,DT_CENTER|DT_VCENTER|DT_WORDBREAK);
				// m_objResource->SetElement(pFontElement, 0);
				m_objResource->AddElement(pFontElement,"text");
			}
		}
	}
	
	m_objResource->SetCurrentState();
	m_objResource->SetActiveLayer();
}

void CGUIToolTip::CheckTipState(float fTimeElapsed)
{
	if (m_ActiveTip==NULL || fTimeElapsed <=0.f) {
		return;
	}

	static double lastblinktime=0;
#ifdef WIN32
	static double blinktime=GetCaretBlinkTime() * 0.001f;
#else
	static double blinktime = 0.5;
#endif
	m_ActiveTip->m_timecount += (int)(fTimeElapsed*1000);

	switch(m_ActiveTip->m_state) {
	case (int)GUIToolTipItem::BEFORESHOW:
		if (m_ActiveTip->m_timecount>m_ActiveTip->m_initialdelay) {
			m_ActiveTip->m_state=GUIToolTipItem::SHOWING;
			m_ActiveTip->m_timecount=0;
			lastblinktime=(GetTickCount()/1000.f);
			UpdateRects();
		}
		
		break;
	case (int)GUIToolTipItem::SHOWING:
		if( (m_ActiveTip->m_behavior&FLASHING)!=0&&((GetTickCount()/1000.f) - lastblinktime) >= blinktime )
		{
			m_ActiveTip->m_state=GUIToolTipItem::BLINKING;
			lastblinktime = (GetTickCount()/1000.f);
		}
		if (m_ActiveTip->m_autopopdelay>0&&m_ActiveTip->m_timecount>m_ActiveTip->m_autopopdelay&&(m_ActiveTip->m_behavior&STATIC)==0) {
			m_ActiveTip->m_state=GUIToolTipItem::AFTERSHOW;
		}
		break;
	case (int)GUIToolTipItem::BLINKING:
		if( (m_ActiveTip->m_behavior&FLASHING)!=0&&((GetTickCount()/1000.f) - lastblinktime) >= blinktime )
		{
			m_ActiveTip->m_state=GUIToolTipItem::SHOWING;
			lastblinktime = (GetTickCount()/1000.f);
		}
		break;
	}
	
}

/**
m_objResource->m_objArtwork->DrawingRects[0]: text rectangle;
m_objResource->m_objArtwork->DrawingRects[1]: left-top corner rectangle;
m_objResource->m_objArtwork->DrawingRects[2]: top border rectangle;
m_objResource->m_objArtwork->DrawingRects[3]: right-top corner rectangle;
m_objResource->m_objArtwork->DrawingRects[4]: left border rectangle;
m_objResource->m_objArtwork->DrawingRects[5]: right border rectangle;
m_objResource->m_objArtwork->DrawingRects[6]: left-bottom corner rectangle;
m_objResource->m_objArtwork->DrawingRects[7]: bottom border rectangle;
m_objResource->m_objArtwork->DrawingRects[8]: right-bottom corner rectangle;
m_objResource->m_objArtwork->DrawingRects[9]: indicator rectangle;
m_objResource->m_objArtwork->DrawingRects[10]: indicator-extend line rectangle;
m_objResource->m_objArtwork->DrawingRects[11]: left indicator filling rectangle;
m_objResource->m_objArtwork->DrawingRects[12]: right indicator filling rectangle;
*/
void CGUIToolTip::UpdateRects()
{
	if (m_ActiveTip==NULL) {
		return;
	}

	int height,width;
	width=m_nSpacing*2+m_ActiveTip->m_rect.right-m_ActiveTip->m_rect.left;
	height=m_nSpacing+m_ActiveTip->m_rect.bottom-m_ActiveTip->m_rect.top+(m_nIndicator>0?m_nIndicator:m_nSpacing);
	POINT point;
	POINT ltpoint;// left top position of the tooltip
	if ((m_ActiveTip->m_behavior&HIGHTLIGHT)!=0) {
		CGUIPosition pos;
		m_ActiveTip->m_obj->GetAbsolutePosition(&pos,m_ActiveTip->m_obj->GetPosition());
		point.x=(pos.rect.left+pos.rect.right)/2;
		point.y=(pos.rect.bottom+pos.rect.top)/2;
	}else{
		point.x=CGUIRoot::GetInstance()->m_pMouse->m_x;
		point.y=CGUIRoot::GetInstance()->m_pMouse->m_y;
	}
	/* @changes LXZ 2006.8.28: */
#ifdef DISPLAY_ON_CURSOR_TOP
	// display just above the cursor, positioned in the middle.
	ltpoint.x=point.x-width/2;
	ltpoint.y=point.y-height-CGUIRoot::GetInstance()->m_pMouse->GetCursorSize()/2;
#else
	// display just below the cursor, positioned in the middle.
	ltpoint.x=point.x-width/2;
	ltpoint.y=point.y+CGUIRoot::GetInstance()->m_pMouse->GetCursorSize();
#endif

	
	if (ltpoint.y<0) {
		ltpoint.y=point.y;
	}
	if (ltpoint.x<0) {
		ltpoint.x=point.x;
	}
	if ((ltpoint.x+width)>CGUIRoot::GetInstance()->GetPosition()->rect.right) {
		ltpoint.x=CGUIRoot::GetInstance()->GetPosition()->rect.right-width;
	}
	if ((ltpoint.y+height)>CGUIRoot::GetInstance()->GetPosition()->rect.bottom) {
		//ltpoint.y=CGUIRoot::Instance()->GetPosition()->rect.bottom-height;
		ltpoint.y=point.y-height-CGUIRoot::GetInstance()->m_pMouse->GetCursorSize();
	}

	m_objResource->SetCurrentState();
	m_objResource->SetActiveLayer();

	RECT rcInner, rcOut;
	SetRect(&rcOut,ltpoint.x,ltpoint.y,ltpoint.x+width,ltpoint.y+height);
	SetRect(&rcInner,ltpoint.x+m_nSpacing,ltpoint.y+m_nSpacing,ltpoint.x+width-m_nSpacing,ltpoint.y+height-m_nSpacing);
	m_objResource->SetDrawingRects(&rcInner,0);
	
	if (m_nIndicator>0) 
	{
		//TODO: add indicator support
	}else
	{
		const int offset = 1;
		RECT tempRect;
		// use the left top and right bottom texture element size to calculate the inner rect size. 
		GUITextureElement* pElement = m_objResource->GetTextureElement(1);
		if(pElement)
		{
			pElement->GetRect(&tempRect);
			rcInner.left = RectWidth(tempRect) + rcOut.left;
			rcInner.top = RectHeight(tempRect) + rcOut.top;
		}
		pElement = m_objResource->GetTextureElement(8);
		if(pElement)
		{
			pElement->GetRect(&tempRect);
			rcInner.right = rcOut.right - RectWidth(tempRect);
			rcInner.bottom = rcOut.bottom - RectHeight(tempRect);
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
		SetRect( &tempRect, rcOut.left, rcOut.top, rcInner.left, rcInner.top );
		m_objResource->SetDrawingRects(&tempRect,offset+1);
		SetRect( &tempRect, rcInner.left, rcOut.top, rcInner.right, rcInner.top );
		m_objResource->SetDrawingRects(&tempRect,offset+2);
		SetRect( &tempRect, rcInner.right, rcOut.top, rcOut.right, rcInner.top );
		m_objResource->SetDrawingRects(&tempRect,offset+3);
		SetRect( &tempRect, rcOut.left, rcInner.top, rcInner.left, rcInner.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+4);
		SetRect( &tempRect, rcInner.right, rcInner.top, rcOut.right, rcInner.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+5);
		SetRect( &tempRect, rcOut.left, rcInner.bottom, rcInner.left, rcOut.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+6);
		SetRect( &tempRect, rcInner.left, rcInner.bottom, rcInner.right, rcOut.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+7);
		SetRect( &tempRect, rcInner.right, rcInner.bottom, rcOut.right, rcOut.bottom );
		m_objResource->SetDrawingRects(&tempRect,offset+8);
	}

	m_bNeedUpdate=false;

}

int ParaEngine::CGUIToolTip::Release()
{
	delete this; 
	return 0;
}
