//----------------------------------------------------------------------
// Class:	CGUIEdit and CGUIIMEEditbox
// Authors:	Liu Weili, LiXizhi
// Company: ParaEngine
// Date:	2005.8.3
// Revised: 2010.3.13
// desc: 
// The Edit control provides text edit without IME support.
// Most are from DirectX 9.0c SDK
// @changes LiXizhi 2006.8.28: PasswordChar implemented.
// @changes LiXizhi 2007.7.26: caret may only be hidden for IMEeditbox. for Editbox, it is always false. 
//-----------------------------------------------------------------------
#include "ParaEngine.h"

#pragma region Header
#include "ParaWorldAsset.h"
#include "GUIEdit.h"
#include "GUIIME.h"
#include "GUIDirectInput.h"
#include "GUIRoot.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "PaintEngine/Painter.h"
#include "util/StringHelper.h"
#include "ic/ICConfigManager.h"
#include "memdebug.h"
using namespace ParaEngine;
using namespace std;

#ifdef USE_OPENGL_RENDERER
#include "OpenGLWrapper.h"
#endif

const IType* CGUIEditBox::m_type = NULL;

#pragma endregion Header

//--------------------------------------------------------------------------------------
// CGUIEditBox class
//--------------------------------------------------------------------------------------

// When scrolling, EDITBOX_SCROLLEXTENT is reciprocal of the amount to scroll.
// If EDITBOX_SCROLLEXTENT = 4, then we scroll 1/4 of the control each time.
#define EDITBOX_SCROLLEXTENT 4

//--------------------------------------------------------------------------------------
CGUIEditBox::CGUIEditBox() :CGUIBase()
{
	if (!m_type){
		m_type = IType::GetType("guieditbox");
	}
	// 	m_objType |= Type_GUIEditBox;

	m_nBorder = 1;  // Default border width
	m_nSpacing = 4;  // Default spacing

	m_bCaretOn = true;
#ifdef WIN32
	m_dfBlink = GetCaretBlinkTime() * 0.001f;
#else
	m_dfBlink = 0.5;
#endif
	m_dfLastBlink = (GetTickCount() / 1000.f);
	m_SelBkColor = COLOR_ARGB(255, 40, 50, 92);
	m_CaretColor = COLOR_ARGB(255, 0, 0, 0);
	m_nFirstVisible = 0;
	m_nCaret = m_nSelStart = 0;
	m_bInsertMode = true;
	m_bReadOnly = false;
	m_bCanHasFocus = true;
	m_bMultipleLine = false;
	m_bMouseDrag = false;

	m_last_keytime = (GetTickCount() / 1000.f);
	m_last_key = 0;
	m_key_state = 0;//0:not pressed; 1:pressed; 2:holding a key
	m_bIsModified = false;
	m_PasswordChar = '\0';
	m_EmptyTextColor = Color(0x0);
}


//--------------------------------------------------------------------------------------
CGUIEditBox::~CGUIEditBox()
{
}

void CGUIEditBox::Clone(IObject* pobj)const
{
	PE_ASSERT(pobj != NULL);
	PERF_BEGIN("Base clone");
	if (pobj == NULL) {
		return;
	}

	CGUIEditBox* pEditbox = (CGUIEditBox*)pobj;
	CGUIBase::Clone(pEditbox);
#ifdef USE_DIRECTX_RENDERER
	pEditbox->m_Buffer.SetText(m_Buffer.GetBuffer());
	pEditbox->m_Buffer.SetMultiline(m_Buffer.GetMultiline());
	//pEditbox->m_Buffer.SetFontNode(m_Buffer.GetFontNode());
	// Mod by LiXizhi 2007.10.2. Tricky. since font element is reference type. it can not be cloned.
	pEditbox->m_Buffer.SetFontNode(pEditbox->m_objResource->GetFontElement(0));
#endif
	pEditbox->m_nBorder = m_nBorder;
	pEditbox->m_nSpacing = m_nSpacing;
	pEditbox->m_bNeedUpdate = true;
	pEditbox->m_dfBlink = m_dfBlink;
	pEditbox->m_dfLastBlink = 0;
	pEditbox->m_bCaretOn = m_bCaretOn;
	pEditbox->m_bInsertMode = m_bInsertMode;
	pEditbox->m_nSelStart = m_nSelStart;
	pEditbox->m_nFirstVisible = pEditbox->m_nFirstVisible;
	pEditbox->m_CaretColor = m_CaretColor;
	pEditbox->m_SelBkColor = m_SelBkColor;
	pEditbox->m_bMultipleLine = m_bMultipleLine;
	pEditbox->m_bMouseDrag = false;
	pEditbox->m_bReadOnly = m_bReadOnly;
	pEditbox->PlaceCaret(m_nCaret);
	pEditbox->m_last_key = 0;
	pEditbox->m_last_keytime = (GetTickCount() / 1000.f);
	pEditbox->m_key_state = 0;
	PERF_END("Base clone");
}

IObject* CGUIEditBox::Clone()const
{
	CGUIEditBox *pEditbox = new CGUIEditBox();
	Clone(pEditbox);
	return pEditbox;
}

void CGUIEditBox::StaticInit()
{
	CObjectManager *pOm = &CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIBase")) {
		CGUIBase::StaticInit();
	}
	//load the default CGUIBase object and copy all its value to the new button
	CGUIEditBox *pEditbox = new CGUIEditBox();
	pOm->CloneObject("default_CGUIBase", pEditbox);
	// 	pEditbox->m_objType=CGUIBase::Type_GUIEditBox;
	pEditbox->m_nBorder = 1;  // Default border width
	pEditbox->m_nSpacing = 4;  // Default spacing
	pEditbox->m_bCaretOn = true;
#ifdef WIN32
	pEditbox->m_dfBlink = GetCaretBlinkTime() * 0.001f;
#else
	pEditbox->m_dfBlink = 0.5;
#endif
	pEditbox->m_dfLastBlink = (GetTickCount() / 1000.f);
	pEditbox->m_SelBkColor = COLOR_ARGB(255, 40, 50, 92);
	pEditbox->m_CaretColor = COLOR_ARGB(255, 0, 0, 0);
	pEditbox->m_nFirstVisible = 0;
	pEditbox->m_nCaret = 0;
	pEditbox->m_nSelStart = 0;
	pEditbox->m_bInsertMode = true;
	pEditbox->m_bReadOnly = false;
	pEditbox->m_bCanHasFocus = true;
	pEditbox->m_bMultipleLine = false;
	pEditbox->m_bMouseDrag = false;

	pEditbox->m_last_keytime = (GetTickCount() / 1000.f);
	pEditbox->m_last_key = 0;
	pEditbox->m_key_state = 0;//0:not pressed; 1:pressed; 2:holding a key


	CEventBinding* pBinding = pEditbox->m_event->GetEventBindingObj();
	pBinding->EnableKeyboard();
	pBinding->EnableMouse();
	//in button, i define that a click on the button will get the button key focus
	//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_CTRL_CAPTUREMOUSE);
	//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_CTRL_RELEASEMOUSE);
	//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_EB_SELECTSTART);
	//	pBinding->MapEvent(EM_MOUSE_LEFTCLICK,EM_EB_SELECTEND);
	//	pBinding->MapEvent(EM_MOUSE_LEFTUP,EM_EB_SELECTEND);
	//	pBinding->MapEvent(EM_MOUSE_LEFTDBCLICK,EM_EB_SELECTALL);
	pBinding->DisableEvent(EM_MOUSE_WHEEL); // LXZ 2007.9.19: this means that scrollable event will leak to its parent container.

	int a;
	for (a = EM_KEY_TEXT_BEGIN; a <= EM_KEY_TEXT_END; a++) {
		pBinding->MapEvent(a, EM_CTRL_HOLDKEY);
	}
	pBinding->MapEvent(EM_KEY_DELETE, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_INSERT, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_HOME, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_END, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_PAGE_DOWN, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_PAGE_UP, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_LEFT, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_RIGHT, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_DOWN, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_UP, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_BACKSPACE, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_TAB, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_SPACE, EM_CTRL_HOLDKEY);
	pBinding->MapEvent(EM_KEY_RETURN, EM_CTRL_HOLDKEY);

	using namespace ParaInfoCenter;
	CICConfigManager *cm = CGlobals::GetICConfigManager();
	string value0, value1;
	int event0, event1;
	DWORD b;
	HRESULT hr;
	hr = cm->GetSize("GUI_editbox_control_mapping", &b);
	if (hr == E_INVALIDARG || hr == E_ACCESSDENIED) {
		//error
	}
	else{
		for (a = 0; a < (int)b; a += 2) {
			hr = cm->GetTextValue("GUI_editbox_control_mapping", value0, a);
			if (hr != S_OK) {
				break;
			}
			hr = cm->GetTextValue("GUI_editbox_control_mapping", value1, a + 1);
			if (hr != S_OK) {
				break;
			}
			event0 = CEventBinding::StringToEventValue(value0);
			event1 = CEventBinding::StringToEventValue(value1);
			pBinding->MapEvent(event0, event1);
		}
	}
	int tempint;;
	if (cm->GetIntValue("GUI_editbox_control_9element", &tempint) == S_OK) {
		if (tempint == 0) {
			pEditbox->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
		}
		else
			pEditbox->m_objResource->SetLayerType(GUILAYER::NINE_ELEMENT);
	}
	if (cm->GetIntValue("GUI_editbox_control_careton", &tempint) == S_OK) {
		if (tempint == 0) {
			pEditbox->m_bCaretOn = false;
		}
		else
			pEditbox->m_bCaretOn = true;
	}
	if (cm->GetIntValue("GUI_editbox_control_readonly", &tempint) == S_OK) {
		if (tempint == 0) {
			pEditbox->m_bReadOnly = false;
		}
		else
			pEditbox->m_bReadOnly = true;
	}
	if (cm->GetIntValue("GUI_editbox_control_multipleline", &tempint) == S_OK) {
		if (tempint == 0) {
			pEditbox->m_bMultipleLine = false;
		}
		else
			pEditbox->m_bMultipleLine = true;
	}
	if (cm->GetIntValue("GUI_editbox_control_borderwidth", &tempint) == S_OK) {
		pEditbox->m_nBorder = tempint;
	}
	if (cm->GetIntValue("GUI_editbox_control_spacing", &tempint) == S_OK) {
		pEditbox->m_nSpacing = tempint;
	}
	if (cm->GetIntValue("GUI_editbox_control_visible", &tempint) == S_OK) {
		if (tempint == 0) {
			pEditbox->m_bIsVisible = false;
		}
		else
			pEditbox->m_bIsVisible = true;
	}
	if (cm->GetIntValue("GUI_editbox_control_enable", &tempint) == S_OK) {
		if (tempint == 0) {
			pEditbox->m_bIsEnabled = false;
		}
		else
			pEditbox->m_bIsEnabled = true;
	}
	if (cm->GetIntValue("GUI_editbox_control_canhasfocus", &tempint) == S_OK) {
		if (tempint == 0) {
			pEditbox->m_bCanHasFocus = false;
		}
		else
			pEditbox->m_bCanHasFocus = true;
	}
	if (cm->GetIntValue("GUI_editbox_control_receivedrag", &tempint) == S_OK) {
		if (tempint == 0) {
			pEditbox->m_bReceiveDrag = false;
		}
		else
			pEditbox->m_bReceiveDrag = true;
	}
	if (cm->GetIntValue("GUI_editbox_control_candrag", &tempint) == S_OK) {
		if (tempint == 0) {
			pEditbox->SetCandrag(false);
		}
		else
			pEditbox->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_editbox_control_lifetime", &tempint) == S_OK) {
		pEditbox->m_nLifeTimeCountDown = tempint;
	}
	//could load repeat delay, to repeat delay, default font, default texture, mouse mapping, key mapping

	//Set the default texture and font of the default button
	//-------------------------------------
	// CGUIEditBox
	//-------------------------------------
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
	// Font Element assignment:
	//   0 - text
	//   1 - selected text
	pEditbox->m_objResource->SetActiveLayer();
	pEditbox->m_objResource->SetCurrentState();

	RECT *prect = NULL;
	RECT rect;
	SpriteFontEntity* pFont;
	TextureEntity* pTexture = NULL;
	pFont = CGlobals::GetAssetManager()->GetFont("sys");
	GUIFontElement font_;
	GUIFontElement* pFontElement = &font_;
	GUITextureElement tex_;
	GUITextureElement* pElement = &tex_;
	pFontElement->SetElement(pFont, COLOR_ARGB(255, 16, 16, 16), DT_LEFT | DT_TOP);
	pEditbox->m_objResource->AddElement(pFontElement, "text");
	pFontElement->SetElement(pFont, COLOR_ARGB(255, 255, 255, 255), DT_LEFT | DT_TOP);
	pEditbox->m_objResource->AddElement(pFontElement, "selected_text");

	string background, filename;
	bool bLoadDefault = true;
	if (cm->GetTextValue("GUI_editbox_control_background", background) == S_OK) {

		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 14, 90, 241, 113);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pEditbox->m_objResource->AddElement(pElement, "background");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_editbox_control_topleft", background) == S_OK) {

		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 8, 82, 14, 90);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pEditbox->m_objResource->AddElement(pElement, "top_left_border");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_editbox_control_top", background) == S_OK) {

		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 14, 82, 241, 90);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pEditbox->m_objResource->AddElement(pElement, "top_border");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_editbox_control_topright", background) == S_OK) {

		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 241, 82, 246, 90);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pEditbox->m_objResource->AddElement(pElement, "top_right_border");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_editbox_control_left", background) == S_OK) {

		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 8, 90, 14, 113);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pEditbox->m_objResource->AddElement(pElement, "left_border");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_editbox_control_right", background) == S_OK) {

		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 241, 90, 246, 113);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pEditbox->m_objResource->AddElement(pElement, "right_border");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_editbox_control_bottomleft", background) == S_OK) {

		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 8, 113, 14, 121);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pEditbox->m_objResource->AddElement(pElement, "bottom_left_border");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_editbox_control_bottom", background) == S_OK) {

		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 14, 113, 241, 121);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pEditbox->m_objResource->AddElement(pElement, "bottom_border");

	bLoadDefault = true;
	if (cm->GetTextValue("GUI_editbox_control_bottomright", background) == S_OK) {

		prect = StringHelper::GetImageAndRect(background, filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(), filename.c_str(), TextureEntity::StaticTexture);
		if (pTexture != NULL)
			bLoadDefault = false;
	}
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture = CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds", TextureEntity::StaticTexture);
		prect = &rect;
		SetRect(prect, 241, 113, 246, 121);
	}
	pElement->SetElement(pTexture, prect, COLOR_ARGB(255, 255, 255, 255));
	pEditbox->m_objResource->AddElement(pElement, "bottom_right_border");

	pEditbox->m_objResource->SetCurrentState();
	pEditbox->m_objResource->SetActiveLayer();
#ifdef USE_DIRECTX_RENDERER
	pEditbox->m_Buffer.SetFontNode(pEditbox->m_objResource->GetFontElement(0));
#endif
	pEditbox->PlaceCaret(pEditbox->m_nCaret);  // Call PlaceCaret now that we have the font info (node),

	pOm->SetObject("default_CGUIEditBox", pEditbox);
	SAFE_RELEASE(pEditbox);
}
string CGUIEditBox::ToScript(int option)
{
	string script = CGUIBase::ToScript(option);
	//add "text:SetText("ȷ��");"like script
	script += "__this.text=\"";
	std::string buf;
	GetTextA(buf);
	script += buf;
	script += "\";\n";
	return script;
}

void CGUIEditBox::SetCaretVisible(bool bVisible)
{
	m_bCaretOn = bVisible;
}

bool CGUIEditBox::IsCaretVisible()
{
	return m_bCaretOn;
}
//--------------------------------------------------------------------------------------
// PlaceCaret: Set the caret to a character position, and adjust the scrolling if
//             necessary.
//--------------------------------------------------------------------------------------
void CGUIEditBox::PlaceCaret(int nCP)
{
	if (!(nCP >= 0 && nCP <= m_Buffer.GetTextSize()))
	{
		OUTPUT_LOG("warn: place caret out of range: nCP: %d\n", nCP);
		return;
	}

	m_nCaret = nCP;
	if (m_parent == 0)
	{
		// By LiXizhi, 2007.9.25
		// this is tricky when setting text when the client area is unknown and text position can not be calculated. 
		m_nCaret = 0;
		m_nFirstVisible = 0;
		return;
	}

	RECT rcText = m_objResource->GetDrawingRects(9);
	if (rcText.left >= rcText.right)
	{
		// tricky. 2011.5.19. this will crash the computer if rcText is negative. 
		m_nCaret = 0;
		m_nFirstVisible = 0;
		return;
	}

	// Obtain the X offset of the character.
	int nX1st, nX, nX2, nY1st, nY, nY2;
	CPtoXY(m_nFirstVisible, FALSE, &nX1st, &nY1st);  // 1st visible char
	CPtoXY(nCP, FALSE, &nX, &nY);  // LEAD
	// If nCP is the NULL terminator, get the leading edge instead of trailing.
	if (nCP == m_Buffer.GetTextSize())
	{
		nX2 = nX; nY2 = nY;
	}
	else
		CPtoXY(nCP, TRUE, &nX2, &nY2);  // TRAIL

	// If the left edge of the char is smaller than the left edge of the 1st visible char,
	// we need to scroll left until this char is visible.
	if (nX < nX1st)
	{
		// Simply make the first visible character the char at the new caret position.
		m_nFirstVisible = nCP;
	}
	// If the right of the character is bigger than the offset of the control's
	// right edge, we need to scroll right to this character.
	else if (nX2 > nX1st + RectWidth(rcText))
	{
		// Compute the X of the new left-most pixel
		int nXNewLeft = nX2 - RectWidth(rcText);
		int nYNewTop = 0;
		// Compute the char position of this character
		int nCPNew1st, nNewTrail;
		//TODO: multiline support
		XYtoCP(nXNewLeft, nYNewTop, &nCPNew1st, &nNewTrail);

		// If this coordinate is not on a character border,
		// start from the next character so that the caret
		// position does not fall outside the text rectangle.
		int nXNew1st, nYNew1st;
		CPtoXY(nCPNew1st, FALSE, &nXNew1st, &nYNew1st);
		if (nXNew1st < nXNewLeft)
			++nCPNew1st;

		m_nFirstVisible = nCPNew1st;
	}
}


//--------------------------------------------------------------------------------------
void CGUIEditBox::ClearText()
{
	m_Buffer.Clear();
	m_nFirstVisible = 0;
	PlaceCaret(0);
	m_nSelStart = 0;
	m_bIsModified = true;
	SetDirty(true);
}


//--------------------------------------------------------------------------------------
void CGUIEditBox::SetTextA(LPCSTR wszText, bool bSelected)
{
	m_Buffer.SetTextA(wszText);
	m_nFirstVisible = 0;
	// Move the caret to the beginning of the text
	PlaceCaret(0);
	m_nSelStart = bSelected ? 0 : m_nCaret;
	SetDirty(true);
}

void CGUIEditBox::SetText(const char16_t* wszText)
{
	m_Buffer.SetText(wszText);
	m_nFirstVisible = 0;
	// Move the caret to the beginning of the text
	PlaceCaret(0);
	m_nSelStart = m_nCaret;
	SetDirty(true);
}

void CGUIEditBox::SetModified()
{
	m_bIsModified = true;
	MSG newMsg;
	newMsg.message = EM_CTRL_MODIFY;
	MsgProc(&newMsg);
}

//--------------------------------------------------------------------------------------
HRESULT CGUIEditBox::GetTextCopy(std::u16string& out)
{
	out = m_Buffer.GetBuffer();
	return S_OK;
}


//--------------------------------------------------------------------------------------
void CGUIEditBox::DeleteSelectionText()
{
	int nFirst = Math::Min(m_nCaret, m_nSelStart);
	int nLast = Math::Max(m_nCaret, m_nSelStart);
	// Update caret and selection
	PlaceCaret(nFirst);
	m_nSelStart = m_nCaret;
	// Remove the characters
	for (int i = nFirst; i < nLast; ++i)
		m_Buffer.RemoveChar(nFirst);
	if (nFirst <= nLast){
		m_bIsModified = true;
	}
	SetDirty(true);
}


//--------------------------------------------------------------------------------------
void CGUIEditBox::UpdateRects()
{
	//CDXUTControl::UpdateRects();
	RECT rcText;
	CGUIPosition tempPos;
	GetAbsolutePosition(&tempPos, &m_position);
	if (m_position.IsRelativeTo3DObject())
	{
		SetDepth(tempPos.GetDepth());
	}
	// Update the text rectangle
	RECT rcInner = tempPos.rect;
	rcText = tempPos.rect;
	// First inflate by m_nBorder to compute render rects
	InflateRect(&rcText, -m_nBorder, -m_nBorder);

	// Update the render rectangles according to the layer type
	switch (m_objResource->GetLayerType()) {
	case GUILAYER::ONE_ELEMENT:
		m_objResource->SetDrawingRects(&rcText, 0);
		// Inflate further by m_nSpacing
		InflateRect(&rcText, -m_nSpacing, -m_nSpacing);
		m_objResource->SetDrawingRects(&rcText, 9);

		break;
	case GUILAYER::NINE_ELEMENT:
		RECT tempRect;
		/** old 2006 code based on m_nBorder
		m_objResource->SetDrawingRects(&rcText,0);
		SetRect( &tempRect, tempPos.rect.left, tempPos.rect.top, rcText.left, rcText.top );
		m_objResource->SetDrawingRects(&tempRect,1);
		SetRect( &tempRect, rcText.left, tempPos.rect.top, rcText.right, rcText.top );
		m_objResource->SetDrawingRects(&tempRect,2);
		SetRect( &tempRect, rcText.right, tempPos.rect.top, tempPos.rect.right, rcText.top );
		m_objResource->SetDrawingRects(&tempRect,3);
		SetRect( &tempRect, tempPos.rect.left, rcText.top, rcText.left, rcText.bottom );
		m_objResource->SetDrawingRects(&tempRect,4);
		SetRect( &tempRect, rcText.right, rcText.top, tempPos.rect.right, rcText.bottom );
		m_objResource->SetDrawingRects(&tempRect,5);
		SetRect( &tempRect, tempPos.rect.left, rcText.bottom, rcText.left, tempPos.rect.bottom );
		m_objResource->SetDrawingRects(&tempRect,6);
		SetRect( &tempRect, rcText.left, rcText.bottom, rcText.right, tempPos.rect.bottom );
		m_objResource->SetDrawingRects(&tempRect,7);
		SetRect( &tempRect, rcText.right, rcText.bottom, tempPos.rect.right, tempPos.rect.bottom );
		m_objResource->SetDrawingRects(&tempRect,8);
		*/
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
		const int offset = 0;
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

		// Inflate further by m_nSpacing
		InflateRect(&rcText, -m_nSpacing, -m_nSpacing);
		m_objResource->SetDrawingRects(&rcText, 9);

		break;
	}
	m_bNeedUpdate = false;
}


void CGUIEditBox::CopyToClipboard()
{
#ifdef WIN32
	// Copy the selection text to the clipboard
	if (m_nCaret != m_nSelStart && OpenClipboard(NULL))
	{
		EmptyClipboard();

		HGLOBAL hBlock = GlobalAlloc(GMEM_MOVEABLE, sizeof(WCHAR) * (m_Buffer.GetTextSize() + 1));
		if (hBlock)
		{
			WCHAR *pwszText = (WCHAR*)GlobalLock(hBlock);
			if (pwszText)
			{
				int nFirst = Math::Min(m_nCaret, m_nSelStart);
				int nLast = Math::Max(m_nCaret, m_nSelStart);
				if (nLast - nFirst > 0)
				{
					if (m_PasswordChar == '\0')
					{
						CopyMemory(pwszText, m_Buffer.GetBuffer() + nFirst, (nLast - nFirst) * sizeof(WCHAR));
					}
					else
					{
						for (int i = 0; i < (nLast - nFirst); ++i)
						{
							pwszText[i] = m_PasswordChar;
						}
					}
				}
				pwszText[nLast - nFirst] = '\0';  // Terminate it
				GlobalUnlock(hBlock);
			}
			SetClipboardData(CF_UNICODETEXT, hBlock);
		}
		CloseClipboard();
		// We must not free the object until CloseClipboard is called.
		if (hBlock)
			GlobalFree(hBlock);
	}
#endif
}


void CGUIEditBox::PasteFromClipboard()
{
	DeleteSelectionText();
#ifdef WIN32
	if (OpenClipboard(NULL))
	{
		HANDLE handle = GetClipboardData(CF_UNICODETEXT);
		if (handle)
		{
			// Convert the ANSI string to Unicode, then
			// insert to our buffer.
			char16_t *pwszText = (char16_t*)GlobalLock(handle);
			if (pwszText)
			{
				// Copy all characters up to null.
				if (m_Buffer.InsertString(m_nCaret, pwszText))
					PlaceCaret(m_nCaret + lstrlenW((WCHAR*)pwszText));
				m_nSelStart = m_nCaret;
				m_bIsModified = true;
				GlobalUnlock(handle);
			}
		}
		CloseClipboard();
	}
#endif
}


//--------------------------------------------------------------------------------------
bool CGUIEditBox::OnFocusIn()
{
	bool bHandled = CGUIBase::OnFocusIn();
#ifdef PARAENGINE_MOBILE
	attachWithIME();
#endif
	PlaceCaret(m_nCaret);
	ResetCaretBlink();
	return bHandled;
}

bool CGUIEditBox::OnFocusOut()
{
#ifdef USE_DIRECTX_RENDERER
	CGUIIME::OnFocusOut();
#endif
#ifdef PARAENGINE_MOBILE
	detachWithIME();
#endif

	CGUIRoot::GetInstance()->SetIMEFocus(NULL);

	bool bHandled = false;
	bHandled |= CGUIBase::OnFocusOut();
	if (m_bMultipleLine == false) {
		bHandled |= OnChange();
	}
	return bHandled;
}

bool CGUIEditBox::OnChange(const char* code)
{
	if (m_bIsModified)
	{
		m_bIsModified = false;
		char buf[256];
		StringHelper::fast_snprintf(buf, 255, "virtual_key=%d;", m_event->m_keyboard.nAlterKey);
		return CGUIBase::OnChange(buf);
	}
	return false;
}

bool CGUIEditBox::OnMouseLeave()
{
	return CGUIBase::OnMouseLeave();
}

bool CGUIEditBox::OnMouseEnter()
{
	bool bHandled = CGUIBase::OnMouseEnter();
#ifdef USE_DIRECTX_RENDERER
	CDirectMouse *mouse = CGUIRoot::GetInstance()->m_pMouse;
	if (mouse&&!L_PRESSED(mouse)){
		m_bMouseDrag = false;
	}
#endif
	return bHandled;
}

void CGUIEditBox::Reset()
{
	CGUIBase::Reset();
	m_bMouseDrag = false;

	m_last_keytime = (GetTickCount() / 1000.f);
	m_last_key = 0;
	m_key_state = 0;//0:not pressed; 1:pressed; 2:holding a key
	m_nSelStart = m_nCaret;


}
bool CGUIEditBox::MsgProc(MSG *event)
{
	//one call to this function will trigger at most one event
	if (!m_bIsEnabled)
		return false;
	bool bHandled = false;

	RECT rcText = m_objResource->GetDrawingRects(9);
	if (event != NULL&&!m_event->InterpretMessage(event)) {
		return false;
	}
	POINT pt;
	pt.x = m_event->m_mouse.x;
	pt.y = m_event->m_mouse.y;
	int nEvent = m_event->GetTriggerEvent();
	MSG newMsg;
	DWORD dCurrTime = event->time;
	DWORD static dLastTime = 0;
	CEventBinding::InitMsg(&newMsg, event->time, nEvent, pt);
	if (m_bIsVisible&&!m_bCandrag){
		if (m_bMouseDrag && (m_event->IsMapTo(nEvent, EM_MOUSE_MOVE) || m_event->IsMapTo(nEvent, EM_CTRL_FRAMEMOVE))
			&& (dCurrTime - dLastTime > CGUIRoot::MouseRepeat))
		{
			// Determine the character corresponding to the coordinates.
			int nCP, nTrail, nX1st, nY1st;
			CPtoXY(m_nFirstVisible, FALSE, &nX1st, &nY1st);  // X offset of the 1st visible char
			if (SUCCEEDED(XYtoCP(pt.x - rcText.left + nX1st, pt.y - rcText.top + nY1st, &nCP, &nTrail, true)))
			{
				// Cap at the NULL character.
				if (nTrail && nCP < m_Buffer.GetTextSize())
					PlaceCaret(nCP + 1);
				else
					PlaceCaret(nCP);
			}
			dLastTime = dCurrTime;
			bHandled = true;
		}
		if (!m_bReadOnly){
			//select start and end are often mapped to up and down event, so give the default handler a chance to handle it
			if (m_event->IsMapTo(nEvent, EM_EB_SELECTSTART)) {

				OnSelectStart();

				newMsg.message = EM_CTRL_CAPTUREMOUSE;
				CGUIBase::MsgProc(&newMsg);
				m_bMouseDrag = true;
				// Determine the character corresponding to the coordinates.
				int nCP, nTrail, nX1st, nY1st;
				CPtoXY(m_nFirstVisible, FALSE, &nX1st, &nY1st);  // X offset of the 1st visible char

				if (SUCCEEDED(XYtoCP(pt.x - rcText.left + nX1st, pt.y - rcText.top + nY1st, &nCP, &nTrail, true)))
				{
					// Cap at the NULL character.
					if (nTrail && nCP < m_Buffer.GetTextSize())
						PlaceCaret(nCP + 1);
					else
						PlaceCaret(nCP);
					m_nSelStart = m_nCaret;
					ResetCaretBlink();
				}
				dLastTime = dCurrTime;
				bHandled = true;
			}
			else if (m_event->IsMapTo(nEvent, EM_EB_SELECTEND)) {
				m_bMouseDrag = false;
				newMsg.message = EM_CTRL_RELEASEMOUSE;
				CGUIBase::MsgProc(&newMsg);
				bHandled = true;
			}
			else if (m_event->IsMapTo(nEvent, EM_EB_SELECTALL)) {
				m_nSelStart = 0;
				PlaceCaret(m_Buffer.GetTextSize());
				ResetCaretBlink();
				OnSelect();
				bHandled = true;
			}

			if (m_event->IsMapTo(nEvent, EM_KEY)) {
				if ((event->lParam & 0x80) != 0) {
					if (ControlPressed) {
						if (m_event->IsMapTo(nEvent, EM_KEY_X)) {
							CopyToClipboard();
							DeleteSelectionText();
							newMsg.message = EM_CTRL_MODIFY;
							MsgProc(&newMsg);
							bHandled = true;
						}
						else if (m_event->IsMapTo(nEvent, EM_KEY_C)) {
							CopyToClipboard();
							bHandled = true;
						}
						else if (m_event->IsMapTo(nEvent, EM_KEY_V)) {
							PasteFromClipboard();
							newMsg.message = EM_CTRL_MODIFY;
							MsgProc(&newMsg);
							bHandled = true;
						}
						else if (m_event->IsMapTo(nEvent, EM_KEY_A)) {
							if (m_nSelStart == m_nCaret)
							{
								m_nSelStart = 0;
								PlaceCaret(m_Buffer.GetTextSize());
							}
							bHandled = true;
						}
						else if (m_event->IsMapTo(nEvent, EM_KEY_Z)) {
							// even though, we
							bHandled = true;
						}
						else if (m_event->IsMapTo(nEvent, EM_KEY_Y)) {
							bHandled = true;
						}

					}
					/* tab key should be used as input.
					if (m_event->IsMapTo(nEvent,EM_CTRL_NEXTKEYFOCUS)) {
					if (m_parent) {
					m_parent->NextKeyFocus();
					return true;
					}
					bHandled=true;
					}else
					*/
					if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_HOME)){
						PlaceCaret(0);
						if (!ShiftPressed)
							// Shift is not down. Update selection
							// start along with the caret.
							m_nSelStart = m_nCaret;
						ResetCaretBlink();
						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_END)){
						PlaceCaret(m_Buffer.GetTextSize());
						if (!ShiftPressed)
							// Shift is not down. Update selection
							// start along with the caret.
							m_nSelStart = m_nCaret;
						ResetCaretBlink();
						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_INSERT)){
						if (ControlPressed)
						{
							// Control Insert. Copy to clipboard
							CopyToClipboard();
						}
						else if (ShiftPressed)
						{
							// Shift Insert. Paste from clipboard
							PasteFromClipboard();
						}
						else
						{
							// Toggle caret insert mode
							m_bInsertMode = !m_bInsertMode;
						}
						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_DELETE)){
						// Check if there is a text selection.
						if (m_nCaret != m_nSelStart)
						{
							DeleteSelectionText();
							newMsg.message = EM_CTRL_MODIFY;
							MsgProc(&newMsg);
						}
						else
						{
							// Deleting one character
							if (m_Buffer.RemoveChar(m_nCaret)){
								newMsg.message = EM_CTRL_MODIFY;
								MsgProc(&newMsg);
							}
						}
						m_bIsModified = true;
						ResetCaretBlink();
						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_LEFT)){
						if (ControlPressed)
						{
							// Control is down. Move the caret to a new item
							// instead of a character.
							m_Buffer.GetPriorItemPos(m_nCaret, &m_nCaret);
							PlaceCaret(m_nCaret);
						}
						else
							if (m_nCaret > 0)
								PlaceCaret(m_nCaret - 1);
						if (!ShiftPressed)
							// Shift is not down. Update selection
							// start along with the caret.
							m_nSelStart = m_nCaret;
						ResetCaretBlink();
						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_RIGHT)){
						if (ControlPressed)
						{
							// Control is down. Move the caret to a new item
							// instead of a character.
							m_Buffer.GetNextItemPos(m_nCaret, &m_nCaret);
							PlaceCaret(m_nCaret);
						}
						else
							if (m_nCaret < m_Buffer.GetTextSize())
								PlaceCaret(m_nCaret + 1);
						if (!ShiftPressed)
							// Shift is not down. Update selection
							// start along with the caret.
							m_nSelStart = m_nCaret;
						ResetCaretBlink();
						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_UP)){
						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_DOWN)){
						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_TAB) && !m_bMultipleLine) {
						if (m_parent) {
							m_parent->ActivateNextEdit(this);
						}

						bHandled = true;
					}

					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_RETURN)){
						if (!m_bMultipleLine) {
							newMsg.message = EM_CTRL_CHANGE;
							MsgProc(&newMsg);
							if (m_parent) {
								m_parent->ActivateDefaultButton();
							}
						}
						else{
							if (m_nCaret != m_nSelStart)
								DeleteSelectionText();
							// If we are in overwrite mode and there is already
							// a char at the caret's position, simply replace it.
							// Otherwise, we insert the char as normal.
							if (!m_bInsertMode && m_nCaret < m_Buffer.GetTextSize())
							{
								m_Buffer[m_nCaret] = L'\n';
								PlaceCaret(m_nCaret + 1);
								m_nSelStart = m_nCaret;
							}
							else
							{
								// Insert the char
								if (m_Buffer.InsertChar(m_nCaret, L'\n'))
								{
									PlaceCaret(m_nCaret + 1);
									m_nSelStart = m_nCaret;
								}
							}
							ResetCaretBlink();
							m_bIsModified = true;
							newMsg.message = EM_CTRL_MODIFY;
							MsgProc(&newMsg);
						}

						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_BACKSPACE)){
						// If there's a selection, treat this
						// like a delete key.
						if (m_nCaret != m_nSelStart)
						{
							DeleteSelectionText();
							newMsg.message = EM_CTRL_MODIFY;
							MsgProc(&newMsg);
						}
						else if (m_nCaret > 0)
						{
							// Move the caret, then delete the char.
							PlaceCaret(m_nCaret - 1);
							m_nSelStart = m_nCaret;
							m_Buffer.RemoveChar(m_nCaret);
							m_bIsModified = true;
							newMsg.message = EM_CTRL_MODIFY;
							MsgProc(&newMsg);
						}
						ResetCaretBlink();
						bHandled = true;
					}
					else if (m_event->IsMapTo(m_event->m_keyboard.nAlterKey, EM_KEY_SHIFT)){
						// added by LXZ. Just stop leaking key to 3d. 
						bHandled = true;
					}
					else{
						//Use original key to get the char
						TCHAR temp = CGUIEvent::GetChar(nEvent);
						//skip if it is not a valid character
						if (temp != '\0') {
							/*
							// If there's a selection and the user
							// starts to type, the selection should
							// be deleted.
							if( m_nCaret != m_nSelStart )
							DeleteSelectionText();
							// If we are in overwrite mode and there is already
							// a char at the caret's position, simply replace it.
							// Otherwise, we insert the char as normal.
							if( !m_bInsertMode && m_nCaret < m_Buffer.GetTextSize() )
							{
							m_Buffer[m_nCaret] = temp;
							PlaceCaret( m_nCaret + 1 );
							m_nSelStart = m_nCaret;
							} else
							{
							// Insert the char
							if( m_Buffer.InsertChar( m_nCaret, temp) )
							{
							PlaceCaret( m_nCaret + 1 );
							m_nSelStart = m_nCaret;
							}
							}
							ResetCaretBlink();
							m_bIsModified=true;
							newMsg.message=EM_CTRL_MODIFY;
							MsgProc(&newMsg);
							*/
							bHandled = true;
						}
					}

					if (bHandled)
					{
						CGUIBase::OnKeyUp();
					}
				}
				//			CGUIEvent::KeyStates[nEvent]=(byte)event->lParam;

			}
		}
		//TODO: keyboard

	}

	if (!bHandled) {
		MSG msg = m_event->GenerateMessage();
		return CGUIBase::MsgProc(&msg);
	}
	else
		return true;
}

void CGUIEditBox::SetPasswordChar(char PasswordChar)
{
	m_PasswordChar = PasswordChar;
}
char CGUIEditBox::GetPasswordChar()
{
	return m_PasswordChar;
}

int CGUIEditBox::GetCaretPosition()
{
	return m_nCaret;
}


void CGUIEditBox::SetCaretPosition(int nCharacterPos)
{
	if (nCharacterPos == -1)
		nCharacterPos = m_Buffer.GetTextSize();
	if (nCharacterPos > m_Buffer.GetTextSize())
		nCharacterPos = m_Buffer.GetTextSize() - 1;
	if (nCharacterPos < 0)
		nCharacterPos = 0;
	PlaceCaret(nCharacterPos);
	m_nSelStart = m_nCaret; // makes no selection.
}

int CGUIEditBox::GetTextSize()
{
	return m_Buffer.GetTextSize();
}


void CGUIEditBox::GetPriorWordPos(int nCP, int *pPrior)
{
	m_Buffer.GetPriorItemPos(nCP, pPrior);
}

void CGUIEditBox::GetNextWordPos(int nCP, int *pNext)
{
	m_Buffer.GetNextItemPos(nCP, pNext);
}

HRESULT CGUIEditBox::CPtoXY(int nCP, BOOL bTrail, int *pX, int *pY, bool absolute)
{

	int retX = 0, retY = 0;
	HRESULT ret;

	do
	{
		if (m_PasswordChar == '\0')
		{
			ret = m_Buffer.CPtoXY(nCP, bTrail, &retX, &retY);
			break;
		}
		else
		{
			std::u16string strPassword = m_Buffer.GetBuffer();

			// use the PasswordChar for position calculation
			int nSize = m_Buffer.GetTextSize();
			strPassword.resize(nSize, (char16_t)m_PasswordChar);
			for (int i = 0; i < nSize; ++i)
				m_Buffer[i] = (char16_t)m_PasswordChar;

			ret = m_Buffer.CPtoXY(nCP, bTrail, &retX, &retY);
			for (int i = 0; i < nSize; ++i)
				m_Buffer[i] = strPassword[i];
			break;
		}
	}while (false);


	if (absolute)
	{
		auto pFontElement = m_objResource->GetFontElement(0);
		if (pFontElement)
		{
			auto textFormat = pFontElement->dwTextFormat;
			RECT rcWindow = m_objResource->GetDrawingRects(9);

			int textWidth, textHeight;
			GetTextLineSize(&textWidth, &textHeight);

			if (textFormat & DT_LEFT)
			{

			}
			else if (textFormat & DT_CENTER)
			{
				int offset = ((rcWindow.right - rcWindow.left) - textWidth) / 2;
				retX += offset;
			}
			else if (textFormat & DT_RIGHT)
			{
				int offset = (rcWindow.right - rcWindow.left) - textWidth;
				retX += offset;
			}

			if (textFormat & DT_TOP)
			{

			}
			else if (textFormat & DT_VCENTER)
			{
				int offset = ((rcWindow.bottom - rcWindow.top) - textHeight) / 2;
				retY += offset;
			}
			else if (textFormat & DT_BOTTOM)
			{
				int offset = (rcWindow.bottom - rcWindow.top) - textHeight;
				retY += offset;
			}
		}
	}

	*pX = retX; *pY = retY;

	return ret;
}

/** get the text line size in pixels, supposing the current font and text will be rendered in a single line. */
void CGUIEditBox::GetTextLineSize(int* width, int* height)
{
#ifdef USE_DIRECTX_RENDERER
	int nX = 0;
	int nY = 0;
	m_Buffer.CPtoXY(m_Buffer.GetTextSize(), FALSE, &nX, &nY);
	if (width != 0)
		*width = nX;
	if (height != 0)
		*height = nY;
#elif defined USE_OPENGL_RENDERER
	RECT rcText = { 0, 0, 0, 0 };
	GetPainter()->CalcTextRect(GetText(), m_objResource->GetFontElement(0), &rcText, 0);
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

HRESULT CGUIEditBox::XYtoCP(int nX, int nY, int *pCP, int *pnTrail, bool absolute)
{
	if (absolute)
	{
		auto pFontElement = m_objResource->GetFontElement(0);
		if (pFontElement)
		{
			auto textFormat = pFontElement->dwTextFormat;
			RECT rcWindow = m_objResource->GetDrawingRects(9);

			int textWidth, textHeight;
			GetTextLineSize(&textWidth, &textHeight);

			if (textFormat & DT_LEFT)
			{

			}
			else if (textFormat & DT_CENTER)
			{
				int offset = ((rcWindow.right - rcWindow.left) - textWidth) / 2;
				nX -= offset;
			}
			else if (textFormat & DT_RIGHT)
			{
				int offset = (rcWindow.right - rcWindow.left) - textWidth;
				nX -= offset;
			}

			if (textFormat & DT_TOP)
			{

			}
			else if (textFormat & DT_VCENTER)
			{
				int offset = ((rcWindow.bottom - rcWindow.top) - textHeight) / 2;
				nY -= offset;
			}
			else if (textFormat & DT_BOTTOM)
			{
				int offset = (rcWindow.bottom - rcWindow.top) - textHeight;
				nY -= offset;
			}

		}
	}


	do
	{
		if (m_PasswordChar == '\0')
		{
			return m_Buffer.XYtoCP(nX, nY, pCP, pnTrail);
		}
		else
		{
			u16string strPassword = m_Buffer.GetBuffer();

			// use the PasswordChar for position calculation
			int nSize = m_Buffer.GetTextSize();
			strPassword.resize(nSize, (char16_t)m_PasswordChar);
			for (int i = 0; i < nSize; ++i)
				m_Buffer[i] = (char16_t)m_PasswordChar;

			HRESULT res = m_Buffer.XYtoCP(nX, nY, pCP, pnTrail);
			for (int i = 0; i < nSize; ++i)
				m_Buffer[i] = strPassword[i];
			return res;
		}
	} while (false);

}
HRESULT CGUIEditBox::Render(GUIState* pGUIState, float fElapsedTime)
{
	if (m_bIsVisible == false)
		return S_OK;

	SetDirty(false);
	HRESULT hr;
	int nSelStartX = 0, nSelStartY = 0, nCaretX = 0, nCaretY = 0;  // Left and right X coordinates of the selection region


	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();

	int elementNumber = 0;
	switch (m_objResource->GetLayerType()) {
	case GUILAYER::ONE_ELEMENT:
		elementNumber = 1;
		break;
	case GUILAYER::NINE_ELEMENT:
		elementNumber = 9;
		break;
	}

	RECT rcWindow = m_objResource->GetDrawingRects(9);
	// Render the control graphics
	for (int e = 0; e < elementNumber; ++e)
	{
		//pElement->TextureColor.Blend( DXUT_STATE_NORMAL, fElapsedTime );
		RECT rc = m_objResource->GetDrawingRects(e);
		DrawElement(m_objResource->GetTextureElement(e), &rc, &rcWindow);
	}

	//
	// Compute the X coordinates of the first visible character.
	//
	int nXFirst, nYFirst;
	CPtoXY(m_nFirstVisible, FALSE, &nXFirst, &nYFirst);

	//
	// Compute the X coordinates of the selection rectangle
	//
	hr = CPtoXY(m_nCaret, FALSE, &nCaretX, &nCaretY, true);
	if (m_nCaret != m_nSelStart)
		hr = CPtoXY(m_nSelStart, FALSE, &nSelStartX, &nSelStartY, true);
	else
		nSelStartX = nCaretX;

	GUIFontElement* pFontElement = NULL;

	RECT rcText = rcWindow;
	//
	// Render the selection rectangle
	//
	RECT rcSelection;  // Make this available for rendering selected text
	if (m_nCaret != m_nSelStart)
	{
		int nSelLeftX = nCaretX, nSelRightX = nSelStartX;
		// Swap if left is bigger than right
		if (nSelLeftX > nSelRightX)
		{
			//int nTemp = nSelLeftX; nSelLeftX = nSelRightX; nSelRightX = nTemp;
			std::swap(nSelLeftX, nSelRightX);
		}

		SetRect(&rcSelection, nSelLeftX, rcText.top, nSelRightX, rcText.bottom);
		OffsetRect(&rcSelection, rcText.left - nXFirst, 0);
		if (!IntersectRect(&rcSelection, rcText, rcSelection))
			rcSelection = rcText;
		GetPainter(pGUIState)->DrawRect(&rcSelection, m_SelBkColor, m_position.GetDepth());
	}

	//
	// Render the text
	//
	// Element 0 for text
	pFontElement = m_objResource->GetFontElement(0);
	//	pFontElement.FontColor = m_TextColor;

	const char16_t* texBuffer = NULL;
	static std::u16string strPassword;
	bool bIsEmptyText = false;
	LinearColor oldColor;
	if (m_Buffer.IsEmpty() && !m_bHasFocus) 
	{
		texBuffer = m_empty_text.c_str();
		if (!m_empty_text.empty())
		{
			bIsEmptyText = true;
			oldColor = pFontElement->FontColor;

			if (m_EmptyTextColor.a != 0) {
				pFontElement->FontColor = m_EmptyTextColor;
			} else {
                pFontElement->FontColor.a *= 0.25f;
			}
		}
	}
	else
	{
		if (m_PasswordChar == '\0')
		{
			texBuffer = m_Buffer.GetBuffer();
		}
		else
		{
			// in case a password character is specified, we will display it.
			int nSize = m_Buffer.GetTextSize();
			strPassword.resize(nSize, (char16_t)m_PasswordChar);
			if (!strPassword.empty() && strPassword[0] != ((char16_t)m_PasswordChar))
			{
				for (int i = 0; i < nSize; ++i)
				{
					strPassword[i] = (char16_t)m_PasswordChar;
				}
			}
			texBuffer = strPassword.c_str();
		}
	}

	DrawText(texBuffer + m_nFirstVisible, pFontElement, &rcText, &rcWindow, m_bUseTextShadow, -1, m_textShadowQuality, m_textShadowColor);

	if (bIsEmptyText)
	{
		pFontElement->FontColor = oldColor;
	}

	// Render the selected text
	if (m_nCaret != m_nSelStart)
	{
		int nFirstToRender = Math::Max(m_nFirstVisible, Math::Min(m_nSelStart, m_nCaret));
		int nNumCharToRender = Math::Max(m_nSelStart, m_nCaret) - nFirstToRender;
		pFontElement = m_objResource->GetFontElement(1);
		DrawText(texBuffer + nFirstToRender, pFontElement, &rcSelection, &rcWindow, m_bUseTextShadow, nNumCharToRender);
	}

	//
	// Blink the caret
	//
	if ((GetTickCount() / 1000.f) - m_dfLastBlink >= m_dfBlink)
	{
		m_bCaretOn = !m_bCaretOn;
		m_dfLastBlink = (GetTickCount() / 1000.f);
	}

	//
	// Render the caret if this control has the focus
	//
	if (m_bHasFocus && m_bCaretOn && !IsHideCaret() && !m_bReadOnly)
	{
		// Start the rectangle with insert mode caret
#ifdef PARAENGINE_MOBILE
		// mobile version does not support CPtoXY function, so we will just calculate the text width. 
		RECT rcCaret = {0,0,0,0};
		GetPainter(pGUIState)->CalcTextRect(texBuffer + m_nFirstVisible, pFontElement, &rcCaret, 0, m_nCaret - m_nFirstVisible);
		rcCaret.left = rcText.left + rcCaret.right;
		rcCaret.right = rcCaret.left + 2;
		rcCaret.top = rcText.top;
		rcCaret.bottom = rcText.bottom;
#else
		RECT rcCaret = { rcText.left - nXFirst + nCaretX - 1, rcText.top,
			rcText.left - nXFirst + nCaretX + 1, rcText.bottom };

		// If we are in overwrite mode, adjust the caret rectangle
		// to fill the entire character.
		if (!m_bInsertMode)
		{
			// Obtain the right edge X coord of the current character
			int nRightEdgeX, nRightEdgeY;
			CPtoXY(m_nCaret, TRUE, &nRightEdgeX, &nRightEdgeY, true);
			rcCaret.right = rcText.left + nXFirst + nRightEdgeX;
		}
#endif
		GetPainter(pGUIState)->DrawRect(&rcCaret, m_CaretColor, m_position.GetDepth());
	}
	return S_OK;
}


void CGUIEditBox::SetTextFloatArray(const float *pNumbers, int nCount)
{
	char wszBuffer[512] = { 0 };
	char wszTmp[64];

	if (pNumbers == NULL)
		return;

	for (int i = 0; i < nCount; ++i)
	{
		snprintf(wszTmp, 64, "%.4f ", pNumbers[i]);
		strcat(wszBuffer, wszTmp);
	}

#ifdef USE_DIRECTX_RENDERER
	// Don't want the last space
	if (nCount > 0 && _tcslen(wszBuffer) > 0)
		wszBuffer[_tcslen(wszBuffer) - 1] = 0;
#endif
	SetTextA(wszBuffer);
}
//--------------------------------------------------------------------------------------
void CGUIEditBox::ResetCaretBlink()
{
	m_bCaretOn = true;
	m_dfLastBlink = (GetTickCount() / 1000.f);
}
void CGUIEditBox::SetTextColor(Color Color)
{
	GUIFontElement* pElement = m_objResource->GetFontElement(0);
	pElement->FontColor = Color;
}

void CGUIEditBox::SetSelectedTextColor(Color Color)
{
	GUIFontElement* pElement = m_objResource->GetFontElement(1);
	pElement->FontColor = Color;
}

void CGUIEditBox::SetSelectedBackColor(Color Color)
{
	m_SelBkColor = Color;
}

void CGUIEditBox::SetEmptyTextColor( Color Color )
{
    m_EmptyTextColor = Color;
}

void CGUIEditBox::SetCaretColor(Color Color)
{
	m_CaretColor = Color;
}

Color CGUIEditBox::GetEmptyTextColor()
{
	return m_EmptyTextColor;
}

Color CGUIEditBox::GetSelectedBackColor()
{
	return m_SelBkColor;
}

Color CGUIEditBox::GetCaretColor()
{
	return m_CaretColor;
}

void CGUIEditBox::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm = &CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIEditBox")) {
		CGUIEditBox::StaticInit();
	}
	pOm->CloneObject("default_CGUIEditBox", this);
	CGUIBase::InitObject(strObjectName, alignment, x, y, width, height);

}

void CGUIEditBox::InsertChar(WCHAR wChar, int index/* =-1 */)
{
	if (index == -1) {
		// If there's a selection and the user
		// starts to type, the selection should
		// be deleted.
		if (m_nCaret != m_nSelStart)
			DeleteSelectionText();
		// If we are in overwrite mode and there is already
		// a char at the caret's position, simply replace it.
		// Otherwise, we insert the char as normal.
		if (!m_bInsertMode && m_nCaret < m_Buffer.GetTextSize())
		{
			m_Buffer[m_nCaret] = wChar;
			PlaceCaret(m_nCaret + 1);
			m_nSelStart = m_nCaret;
		}
		else
		{
			// Insert the char
			if (m_Buffer.InsertChar(m_nCaret, wChar))
			{
				PlaceCaret(m_nCaret + 1);
				m_nSelStart = m_nCaret;
			}
		}
		ResetCaretBlink();
	}
	else
	{
		if (index < m_Buffer.GetTextSize())
		{
			m_Buffer[index] = wChar;
		}
		else
		{
			// Insert the char
			m_Buffer.InsertChar(index, wChar);
		}

	}
	MSG newMsg;
	m_bIsModified = true;
	newMsg.message = EM_CTRL_MODIFY;
	MsgProc(&newMsg);

}

void CGUIEditBox::InsertCharA(CHAR Char, int index/* =-1 */)
{
#ifdef PARAENGINE_CLIENT
	WCHAR wChar[2];
	char tChar[2]; tChar[0] = Char; tChar[1] = '\0';
	if (MultiByteToWideChar(DEFAULT_GUI_ENCODING, MB_PRECOMPOSED, tChar, -1, wChar, 2) == 0)
	{
		OUTPUT_LOG("error: Can't translate: %d to wide char.\n", (int)Char);
		InsertChar(wChar[1], index);
	}
#endif
}

int ParaEngine::CGUIEditBox::OnHandleWinMsgChars(const std::wstring& sChars)
{
	if (sChars.empty())
		return 0;
	CGUIBase::OnHandleWinMsgChars(sChars);

	for (size_t i = 0; i < sChars.size(); ++i)
	{
		WCHAR temp = sChars[i];
		if (temp == L'\t' && m_bMultipleLine)
		{
			temp = L' ';
		}
		if (temp > 31)
		{
			// If there's a selection and the user
			// starts to type, the selection should
			// be deleted.
			if (m_nCaret != m_nSelStart)
				DeleteSelectionText();

			// If we are in overwrite mode and there is already
			// a char at the caret's position, simply replace it.
			// Otherwise, we insert the char as normal.
			if (!m_bInsertMode && m_nCaret < m_Buffer.GetTextSize())
			{
				m_Buffer[m_nCaret] = temp;
				PlaceCaret(m_nCaret + 1);
				m_nSelStart = m_nCaret;
			}
			else
			{
				// Insert the char
				if (m_Buffer.InsertChar(m_nCaret, temp))
				{
					PlaceCaret(m_nCaret + 1);
					m_nSelStart = m_nCaret;
				}
			}
			ResetCaretBlink();
		}
	}
	m_bIsModified = true;
	MSG newMsg;
	newMsg.message = EM_CTRL_MODIFY;
	MsgProc(&newMsg);
	return 0;
}


bool ParaEngine::CGUIEditBox::HasClickEvent()
{
	return true;
}

int ParaEngine::CGUIEditBox::GetTextA(std::string& out)
{
	return m_Buffer.GetBufferA(out);
}

void ParaEngine::CGUIEditBox::OnSelectStart()
{
#ifdef PARAENGINE_MOBILE
	if (m_bHasFocus)
		attachWithIME();
#endif
}

#ifdef PARAENGINE_MOBILE
bool ParaEngine::CGUIEditBox::attachWithIME()
{
	bool ret = GUIIMEDelegate::attachWithIME();
	return ret;
}

bool ParaEngine::CGUIEditBox::detachWithIME()
{
	bool ret = GUIIMEDelegate::detachWithIME();
	return ret;
}

bool ParaEngine::CGUIEditBox::canAttachWithIME()
{
	return true;
}

void ParaEngine::CGUIEditBox::didAttachWithIME()
{
}

bool ParaEngine::CGUIEditBox::canDetachWithIME()
{
	return true;
}

void ParaEngine::CGUIEditBox::didDetachWithIME()
{
}

void ParaEngine::CGUIEditBox::insertText(const char * text, size_t len)
{
	//OUTPUT_LOG("IME insert text %s\n", text);
	std::string str;
	str.reserve(len);
	for (size_t i = 0; i < len; ++i)
	{
		char c = text[i];
		// skip \r\n character. 
		if (c != '\r' && c != '\n' && c != '\0')
			str.push_back(c);
	}
	// OUTPUT_LOG("GUI editbox : insert text: %s (count:%d)\n", str.c_str(), (int)str.size());
	// m_Buffer.SetTextA(str.c_str());
	m_Buffer.InsertStringA(-1, str.c_str(), str.size());
	m_bIsModified = true;
	OnModify();
	SetCaretPosition(-1);
}

void ParaEngine::CGUIEditBox::deleteBackward()
{
	// deletion is handled via MsgProc under win32
#ifndef WIN32
	m_Buffer.RemoveChar(-1);
	SetCaretPosition(-1);
	m_bIsModified = true;
	OnModify();
#endif
}

const std::string& ParaEngine::CGUIEditBox::getContentText()
{
	return m_Buffer.GetUtf8Text();
}
#endif

const std::string& ParaEngine::CGUIEditBox::GetEmptyText()
{
	StringHelper::UTF16ToUTF8(m_empty_text, m_empty_text_utf8);
	return m_empty_text_utf8;
}

void ParaEngine::CGUIEditBox::SetEmptytext(const char* utf8Text)
{
	std::string str;
	if (utf8Text != 0)
	{
		str = utf8Text;
		StringHelper::UTF8ToUTF16_Safe(str, m_empty_text);
	}
	else
		m_empty_text.clear();
}

int CGUIEditBox::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	CGUIBase::InstallFields(pClass, bOverride);
	PE_ASSERT(pClass != NULL);
	pClass->AddField("CaretColor", FieldType_DWORD, (void*)SetCaretColor_s, (void*)GetCaretColor_s, NULL, NULL, bOverride);
	pClass->AddField("SelectedBackColor", FieldType_DWORD, (void*)SetSelectedBackColor_s, (void*)GetSelectedBackColor_s, NULL, NULL, bOverride);
	pClass->AddField("EmptyText", FieldType_String, (void*)SetEmptytext_s, (void*)GetEmptyTextColor_s, NULL, NULL, bOverride);
	pClass->AddField("EmptyTextColor", FieldType_DWORD, (void*)SetEmptyTextColor_s, (void*)GetEmptyTextColor_s, NULL, NULL, bOverride);

	return S_OK;
}