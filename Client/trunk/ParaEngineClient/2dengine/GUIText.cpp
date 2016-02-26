//-----------------------------------------------------------------------------
// Class:	CGUIText, LiXizhi
// Authors: Liu, Weili
// Date:	2006.3.21
// desc:
// This class represents a string inside a rectangular box in the screen coordinates
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "GUIText.h"
#include "GUIResource.h"
#include "GUIRoot.h"
#include "GUIEdit.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "PaintEngine/Painter.h"
#include "ic/ICConfigManager.h"
#include "util/StringHelper.h"
#include "memdebug.h"
using namespace ParaEngine;
using namespace std;
const IType* CGUIText::m_type=NULL;

CGUIText::CGUIText(void):CGUIBase(), m_text_scale(1.f)
{
	if (!m_type){
		m_type=IType::GetType("guitext");
	}
// 	m_objType |= CGUIBase::Type_GUIText;
	m_bAutoSize=true;
	m_bNeedCalRect=true;
	m_bCanHasFocus=false;
	m_bClickThrough = true;
}

CGUIText::~CGUIText(void)
{
}

void CGUIText::Clone(IObject* pobj)const
{
	PE_ASSERT(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}
	CGUIText* pText=(CGUIText*)pobj;
	CGUIBase::Clone(pText);
	pText->m_bAutoSize=m_bAutoSize;
	pText->m_bNeedCalRect=m_bNeedCalRect;
	pText->m_szText=m_szText;
}

IObject *CGUIText::Clone()const 
{
	CGUIText *pText=new CGUIText();
	Clone(pText);
	return pText;
}

void CGUIText::StaticInit()
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIBase")) {
		CGUIBase::StaticInit();
	}
	//load the default CGUIBase object and copy all its value to the new button
	CGUIText *pText=new CGUIText();
	pOm->CloneObject("default_CGUIBase",pText);
// 	pText->m_objType=CGUIBase::Type_GUIText;
	pText->m_szText.clear();
	pText->m_bAutoSize=true;
	pText->m_bNeedCalRect=true;
	pText->m_bClickThrough = true;
	CEventBinding* pBinding=pText->m_event->GetEventBindingObj();
	pBinding->DisableKeyboard();
	pBinding->DisableMouse();
	//selectively enable some mouse events.
	pBinding->EnableEvent(EM_MOUSE);
	pBinding->EnableEvent(EM_MOUSE_MOVE);
	pBinding->EnableEvent(EM_MOUSE_LEFTDOWN);
	pBinding->EnableEvent(EM_MOUSE_LEFTUP);
	pBinding->EnableEvent(EM_MOUSE_DRAGBEGIN);
	pBinding->EnableEvent(EM_MOUSE_DRAGOVER);
	pBinding->EnableEvent(EM_MOUSE_DRAGEND);
	pBinding->EnableEvent(EM_MOUSE_LEFTDRAGBEGIN);
	pBinding->EnableEvent(EM_MOUSE_LEFTDRAGEND);
	pBinding->EnableEvent(EM_MOUSE_RIGHTDRAGBEGIN);
	pBinding->EnableEvent(EM_MOUSE_RIGHTDRAGEND);
	pBinding->EnableEvent(EM_MOUSE_MIDDLEDRAGBEGIN);
	pBinding->EnableEvent(EM_MOUSE_MIDDLEDRAGEND);

	using namespace ParaInfoCenter;
	CICConfigManager *cm=CGlobals::GetICConfigManager();
	int tempint;
	if (cm->GetIntValue("GUI_text_control_visible",&tempint)==S_OK) {
		if (tempint==0) {
			pText->m_bIsVisible=false;
		}else
			pText->m_bIsVisible=true;
	}
	if (cm->GetIntValue("GUI_text_control_enable",&tempint)==S_OK) {
		if (tempint==0) {
			pText->m_bIsEnabled=false;
		}else
			pText->m_bIsEnabled=true;
	}
	if (cm->GetIntValue("GUI_text_control_canhasfocus",&tempint)==S_OK) {
		if (tempint==0) {
			pText->m_bCanHasFocus=false;
		}else
			pText->m_bCanHasFocus=true;
	}
	if (cm->GetIntValue("GUI_text_control_receivedrag",&tempint)==S_OK) {
		if (tempint==0) {
			pText->m_bReceiveDrag=false;
		}else
			pText->m_bReceiveDrag=true;
	}
	if (cm->GetIntValue("GUI_text_control_candrag",&tempint)==S_OK) {
		if (tempint==0) {
			pText->SetCandrag(false);
		}else
			pText->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_text_control_lifetime",&tempint)==S_OK) {
		pText->m_nLifeTimeCountDown=tempint;
	}

	//initialize default texture and font
	pText->m_objResource->SetActiveLayer();
	pText->m_objResource->SetCurrentState();
	pText->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
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
	if (cm->GetTextValue("GUI_text_control_background",background)==S_OK) {
		prect=StringHelper::GetImageAndRect(background,filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
		if (pTexture!=NULL) 
			bLoadDefault=false;
	} 
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture=CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds",TextureEntity::StaticTexture);
		prect=&rect;
		SetRect( prect, 0,0,0,0 );
	}

	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	pFontElement->SetElement(pFont,COLOR_ARGB(255,0,0,0),DT_LEFT|DT_TOP|DT_WORDBREAK);
	pText->m_objResource->AddElement(pElement,"background");
	pText->m_objResource->AddElement(pFontElement,"text");

	pText->m_objResource->SetCurrentState();
	pText->m_objResource->SetActiveLayer();
	pOm->SetObject("default_CGUIText",pText);
	SAFE_RELEASE(pText);
}

void CGUIText::SetTextScale(float fScale)
{
	m_text_scale = fScale;
}

float CGUIText::GetTextScale()
{
	return m_text_scale;
}

string CGUIText::ToScript(int option)
{
	string script=CGUIBase::ToScript(option);
	//add "text:SetText("È·¶¨");"like script
	script+="__this.text=\"";
	std::string buf;
	GetTextA(buf);
	script+=buf;
	script+="\";\n";
	//add "text:SetAutoSize(true);"like script
	script+="__this.autosize=";
	if (m_bAutoSize) script+="true";
	else script+="false";
	script+=";\n";
	return script;
}
//--------------------------------------------------------------------------------
// name : InitObject
/// desc: 
/// pFontEntity [in]:  Pointer to a Font Asset object
/// pString	[in]:  Pointer to a string to draw.
/// pPos		[in]: Pointer to a FUI_POSITION  structure that contains the rectangle, in logical coordinates, 
/// in which the text is to be formatted. 
/// Format	[in]: Specifies the method of formatting the text. It can be any combination of 
///                the following values. 
///		DT_BOTTOM
///				Justifies the text to the bottom of the rectangle. This value must be combined with DT_SINGLELINE. 
///		DT_CALCRECT
///				Determines the width and height of the rectangle. If there are multiple lines of text, ID3DXFont::DrawText uses the width of the rectangle pointed to by the pRect parameter and extends the base of the rectangle to bound the last line of text. If there is only one line of text, ID3DXFont::DrawText modifies the right side of the rectangle so that it bounds the last character in the line. In either case, ID3DXFont::DrawText returns the height of the formatted text but does not draw the text. 
///		DT_CENTER
///				Centers text horizontally in the rectangle. 
///		DT_EXPANDTABS
///				Expands tab characters. The default number of characters per tab is eight.
///		DT_LEFT
///				Aligns text to the left. 
///		DT_NOCLIP
///				Draws without clipping. ID3DXFont::DrawText is somewhat faster when DT_NOCLIP is used. 
///		DT_RIGHT
///				Aligns text to the right. 
///		DT_RTLREADING
///				Displays text in right-to-left reading order for bi-directional text when a Hebrew or Arabic font is selected. The default reading order for all text is left-to-right. 
///		DT_SINGLELINE
///				Displays text on a single line only. Carriage returns and line feeds do not break the line. 
///		DT_TOP
///				Top-justifies text. 
///		DT_VCENTER
///				Centers text vertically (single line only). 
///		DT_WORDBREAK
///				Breaks words. Lines are automatically broken between words if a word would extend past the edge of the rectangle specified by the pRect parameter. A carriage return/line feed sequence also breaks the line. 
/// Color	[in]: Color of the text. For more information, see Color.
//--------------------------------------------------------------------------------
void CGUIText::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIText")) {
		CGUIText::StaticInit();
	}
	pOm->CloneObject("default_CGUIText",this);
	CGUIBase::InitObject(strObjectName,alignment,x,y,width,height);
}

void CGUIText::SetColor(DWORD nColor)
{
	m_objResource->SetFontColor((Color)(nColor));
}

void CGUIText::SetTransparency(DWORD transparency)
{
	m_objResource->SetFontTransparency(transparency);

}
void CGUIText::SetTextA(const char* szText)
{
	PE_ASSERT( szText != NULL );
	StringHelper::UTF8ToUTF16_Safe(szText, m_szText);
	m_bNeedCalRect = true;
}

void CGUIText::SetText(const char16_t* wszText)
{
	PE_ASSERT( wszText != NULL );
	m_szText=wszText;
	m_bNeedCalRect=true;
}

int CGUIText::GetTextA(std::string& out)
{
	return StringHelper::UTF16ToUTF8(m_szText, out) ? (int)out.size() : 0;
}

void CGUIText::SetTextFormat(DWORD dwFormat)
{
	m_objResource->SetFontFormat(dwFormat);
}

void CGUIText::GetTextLineSize(int* width, int* height)
{
#ifdef USE_DIRECTX_RENDERER
	CUniLine textline;
	textline.SetText(m_szText.c_str());
	
	textline.SetFontNode(m_objResource->GetFontElement(0));
	int nX = 0;
	textline.CPtoX((int)m_szText.size(), FALSE, &nX);
	int nY = textline.GetTextMetric().tmInternalLeading+textline.GetTextMetric().tmExternalLeading+textline.GetTextMetric().tmHeight;
	if(width!=0)
		*width = nX;
	if(height!=0)
		*height = nY;
#elif defined USE_OPENGL_RENDERER
	RECT rcText = {0,0,0,0};
	GetPainter()->CalcTextRect(m_szText.c_str(), m_objResource->GetFontElement(0), &rcText, 0);
	if (width != 0)
		*width = rcText.right;
	if (height != 0)
		*height = rcText.bottom;
#else
	
	if (width != 0)
		*width = 0;
	if (height != 0)
		*height = 0;
#endif
}

bool CGUIText::DoAutoSize()
{
	RECT rcText=m_position.rect;
	GUIFontElement *pFontElement=m_objResource->GetFontElement(0);
	GetPainter()->CalcTextRect(m_szText.c_str(), pFontElement, &rcText, m_position.GetDepth());
	if ((pFontElement->dwTextFormat&DT_WORDBREAK)!=0) {
		m_position.rect.right=m_position.rect.right>rcText.right?m_position.rect.right:rcText.right;// why add 8 pixels?
		m_position.rect.bottom=rcText.bottom; // why add 6 pixels?
	}
	if ((pFontElement->dwTextFormat&DT_WORDBREAK)==0&&(pFontElement->dwTextFormat&DT_SINGLELINE)!=0) {
		m_position.rect.right=rcText.right;// why add 8 pixels?
		m_position.rect.bottom=rcText.bottom;
	}
	// in case it is bold type, we will increase the height by 2 pixels. 
	/*if(pFontElement->GetFont() && pFontElement->GetFont()->m_nWeight >=  FW_BOLD)
	{
		m_position.rect.bottom+=2;
	}*/
	if(m_nSpacing > 0){
		m_position.rect.right += m_nSpacing*2;
		m_position.rect.bottom += m_nSpacing*2;
	}

	/*UpdateRects();*/
	m_bNeedUpdate=true;
	// 		if (m_parent) {
	// 			m_parent->UpdateClientRect(m_position);
	// 		}
	m_bNeedCalRect=false;
	SetDirty(true);
	return true;
}

//-----------------------------------------------------------------------------------
// name: Render
/// desc: 
/// if pGUIState->bIsBatchRender is false, then Add to batch rendering list
/// otherwise, render the text with sprite. It is up to the caller to ensure that
/// both the font and the sprite object used to render the text are valid.
//-----------------------------------------------------------------------------------
HRESULT CGUIText::Render(GUIState* pGUIState,float fElapsedTime )
{
	HRESULT hr = S_OK;
	SetDirty(false);
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();
	if (m_position.IsRelativeTo3DObject()) 
	{
		BeginDepthTest(pGUIState);
		UpdateRects();
	}
	if (m_bAutoSize&&m_bNeedCalRect) {
		DoAutoSize();
	}
	RECT rcWindow = m_objResource->GetDrawingRects(0);
	RECT tempRect = rcWindow;

	if(m_objResource->GetLayerType() == GUILAYER::ONE_ELEMENT) 
	{
		hr=DrawElement(m_objResource->GetTextureElement(0),&tempRect,&rcWindow);
	}	
	else if(m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT) 
	{
		// Render the control graphics
		for( int e = 0; e < 9; ++e )
		{
			RECT rc = m_objResource->GetDrawingRects(e + 1);
			DrawElement( m_objResource->GetTextureElement(e), &rc, &rcWindow, pGUIState);
		}
	}
	
	InflateRect( &tempRect, -m_nSpacing, -m_nSpacing );

	tempRect.top += m_textOffsetY;
	tempRect.bottom += m_textOffsetY;
	tempRect.left += m_textOffsetX;
	tempRect.right += m_textOffsetX;

	hr|=DrawText(m_szText.c_str(),m_objResource->GetFontElement(0),&tempRect, &rcWindow, m_bUseTextShadow,-1,m_textShadowQuality,m_textShadowColor);
	if (m_position.IsRelativeTo3DObject()) 
	{
		EndDepthTest(pGUIState);
	}

	OnFrameMove(fElapsedTime);
	return hr;
}

HRESULT CGUIText::RenderInBatch(GUIState* pGUIState)
{
	HRESULT hr = S_OK;
	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();
	if (m_position.IsRelativeTo3DObject()) {
		UpdateRects();
	}
	if (m_bAutoSize&&m_bNeedCalRect) {
		DoAutoSize();
	}
	RECT rcWindow = m_objResource->GetDrawingRects(0);
	RECT tempRect = rcWindow;
	if(m_objResource->GetLayerType() == GUILAYER::ONE_ELEMENT) 
	{
		hr=DrawElement(m_objResource->GetTextureElement(0),&tempRect,&rcWindow);
	}	
	else if(m_objResource->GetLayerType() == GUILAYER::NINE_ELEMENT) 
	{
		// Render the control graphics
		for( int e = 0; e < 9; ++e )
		{
			RECT rc = m_objResource->GetDrawingRects(e + 1);
			DrawElement( m_objResource->GetTextureElement(e), &rc, &rcWindow);
		}
	}

	InflateRect( &tempRect, -m_nSpacing, -m_nSpacing );

	tempRect.top += m_textOffsetY;
	tempRect.bottom += m_textOffsetY;
	tempRect.left += m_textOffsetX;
	tempRect.right += m_textOffsetX;

	hr|=DrawText(m_szText.c_str(),m_objResource->GetFontElement(0),&tempRect, &rcWindow, m_bUseTextShadow,-1,m_textShadowQuality,m_textShadowColor);
	return hr;
}

void CGUIText::UpdateRects()
{
	CGUIPosition tempPos;
	GetAbsolutePosition(&tempPos,&m_position);
	if(m_position.IsRelativeTo3DObject())
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

const char16_t* ParaEngine::CGUIText::GetText()
{
	return m_szText.c_str();
}

void ParaEngine::CGUIText::SetAutoSize(bool val)
{
	m_bAutoSize = val;
}

bool ParaEngine::CGUIText::IsAutoSize() const
{
	return m_bAutoSize;
}
