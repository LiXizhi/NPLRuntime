//----------------------------------------------------------------------
// Class:	CGUIIMEEditbox
// Authors:	LiXizhi, Liu Weili
// Company: ParaEngine
// Date:	2005.8.3
// Revised: 2010.3.13
// desc: 
// The Edit control provides text edit with IME support.
//
// Most are from DirectX 9.0c SDK
// @changes LiXizhi 2009.11.7: Information on IME: http://msdn.microsoft.com/en-us/library/ee419002(VS.85).aspx 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#pragma region Header
#include "ParaWorldAsset.h"
#include "IParaEngineApp.h"
#include "PaintEngine/Painter.h"
#include <strsafe.h>
#include "GUIDirectInput.h"
#include "GUIResource.h"
#include "GUIRoot.h"
#include "ObjectManager.h"
#include "EventBinding.h"
#include "util/StringHelper.h"
#include "ic/ICConfigManager.h"
#include "GUIIMEEditBox.h"
#include "memdebug.h"
using namespace ParaEngine;
using namespace std;

const IType* CGUIIMEEditBox::m_type=NULL;

#pragma endregion Header

//--------------------------------------------------------------------------------------
// CGUIIMEEditBox class
//--------------------------------------------------------------------------------------

CGUIIMEEditBox::CGUIIMEEditBox()
	: CGUIEditBox()
{
	m_bInputMethodEnabled = true;
	if (!m_type){
		m_type=IType::GetType("guiimeeditbox");
	}
// 	m_objType=(m_objType&(~CGUIBase::COMMON_CONTROL_MASK))|Type_GUIIMEEditBox;
	m_parent=NULL;

	m_ReadingWinColor = COLOR_ARGB( 128, 0, 0, 0 );
	m_ReadingSelBkColor = COLOR_ARGB( 128, 80, 80, 80 );
	m_CandidateWinColor = COLOR_ARGB( 128, 0, 0, 0 );
	m_CandidateSelBkColor = COLOR_ARGB( 128, 158, 158, 158 );
	m_CompWinColor = COLOR_ARGB( 198, 0, 0, 0 );
	m_CompCaretColor = COLOR_ARGB( 255, 255, 255, 255 );
	m_CompTargetBkColor = COLOR_ARGB( 255, 150, 150, 150 );
	m_CompTargetNonBkColor = COLOR_ARGB( 255, 150, 150, 150 );
	m_nIndicatorWidth = 18;
	m_IndicatorBkColor = COLOR_ARGB( 128, 0, 0, 0 );
}

CGUIIMEEditBox::~CGUIIMEEditBox()
{
	if(this == CGUIRoot::GetInstance()->GetIMEFocus())
	{
		CGUIRoot::GetInstance()->SetIMEFocus(NULL);
	}
}

void CGUIIMEEditBox::Clone(IObject *pobj)const
{
	PE_ASSERT(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}
	CGUIIMEEditBox *pIME=(CGUIIMEEditBox*)pobj;
	CGUIEditBox::Clone(pobj);
	pIME->m_ReadingWinColor=m_ReadingWinColor;
	pIME->m_ReadingSelBkColor=m_ReadingSelBkColor;
	pIME->m_CandidateWinColor=m_CandidateWinColor;
	pIME->m_CandidateSelBkColor=m_CandidateSelBkColor;
	pIME->m_CompWinColor=m_CompWinColor;
	pIME->m_CompCaretColor=m_CompCaretColor;
	pIME->m_CompTargetBkColor=m_CompTargetBkColor;
	pIME->m_CompTargetNonBkColor=m_CompTargetNonBkColor;
	pIME->m_IndicatorBkColor=m_IndicatorBkColor;
	pIME->m_nIndicatorWidth=m_nIndicatorWidth;
	pIME->m_rcIndicator=m_rcIndicator;
}

IObject* CGUIIMEEditBox::Clone()const
{
	CGUIIMEEditBox * pIME=new CGUIIMEEditBox();
	Clone(pIME);
	return pIME;
}

void CGUIIMEEditBox::StaticInit()
{
	// CGUIIME::Initialize();
	
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIEditBox")) {
		CGUIEditBox::StaticInit();
	}
	//load the default CGUIEditBox object and copy all its value to the new ImeEditbox
	CGUIIMEEditBox *pEditbox=new CGUIIMEEditBox();
	pOm->CloneObject("default_CGUIEditBox",pEditbox);

	CEventBinding* pBinding=pEditbox->m_event->GetEventBindingObj();
//	pBinding->MapEvent(EM_MOUSE_LEFTDOWN,EM_IME_SELECT);
//	pBinding->MapEvent(EM_MOUSE_LEFTCLICK,EM_IME_SELECT);
	using namespace ParaInfoCenter;
	CICConfigManager *cm=CGlobals::GetICConfigManager();
	string value0,value1;
	int event0,event1,a;
	DWORD b;
	HRESULT hr;
	hr=cm->GetSize("GUI_imeeditbox_control_mapping",&b);
	if (hr==E_INVALIDARG||hr==E_ACCESSDENIED) {
		//error
	}else{
		for (a=0;a<(int)b;a+=2) {
			hr=cm->GetTextValue("GUI_imeeditbox_control_mapping",value0,a);
			if (hr!=S_OK) {
				break;
			}
			hr=cm->GetTextValue("GUI_imeeditbox_control_mapping",value1,a+1);
			if (hr!=S_OK) {
				break;
			}
			event0=CEventBinding::StringToEventValue(value0);
			event1=CEventBinding::StringToEventValue(value1);
			pBinding->MapEvent(event0,event1);
		}
	}
	int tempint;;
	if (cm->GetIntValue("GUI_imeditbox_control_9element",&tempint)==S_OK) {
		if (tempint==0) {
			pEditbox->m_objResource->SetLayerType(GUILAYER::ONE_ELEMENT);
		}else
			pEditbox->m_objResource->SetLayerType(GUILAYER::NINE_ELEMENT);
	}
	if (cm->GetIntValue("GUI_imeeditbox_control_careton",&tempint)==S_OK) {
		if (tempint==0) {
			pEditbox->SetCaretVisible(false);
		}else
			pEditbox->SetCaretVisible(true);
	}
	if (cm->GetIntValue("GUI_imeeditbox_control_readonly",&tempint)==S_OK) {
		if (tempint==0) {
			pEditbox->m_bReadOnly=false;
		}else
			pEditbox->m_bReadOnly=true;
	}
	if (cm->GetIntValue("GUI_imeeditbox_control_multipleline",&tempint)==S_OK) {
		if (tempint==0) {
			pEditbox->m_bMultipleLine=false;
		}else
			pEditbox->m_bMultipleLine=true;
	}
	if (cm->GetIntValue("GUI_imeeditbox_control_borderwidth",&tempint)==S_OK) {
		pEditbox->m_nBorder=tempint;
	}
	if (cm->GetIntValue("GUI_imeeditbox_control_spacing",&tempint)==S_OK) {
		pEditbox->m_nSpacing=tempint;
	}
	if (cm->GetIntValue("GUI_imeditbox_control_visible",&tempint)==S_OK) {
		if (tempint==0) {
			pEditbox->m_bIsVisible=false;
		}else
			pEditbox->m_bIsVisible=true;
	}
	if (cm->GetIntValue("GUI_imeditbox_control_enable",&tempint)==S_OK) {
		if (tempint==0) {
			pEditbox->m_bIsEnabled=false;
		}else
			pEditbox->m_bIsEnabled=true;
	}
	if (cm->GetIntValue("GUI_imeditbox_control_canhasfocus",&tempint)==S_OK) {
		if (tempint==0) {
			pEditbox->m_bCanHasFocus=false;
		}else
			pEditbox->m_bCanHasFocus=true;
	}
	if (cm->GetIntValue("GUI_imeditbox_control_receivedrag",&tempint)==S_OK) {
		if (tempint==0) {
			pEditbox->m_bReceiveDrag=false;
		}else
			pEditbox->m_bReceiveDrag=true;
	}
	if (cm->GetIntValue("GUI_imeditbox_control_candrag",&tempint)==S_OK) {
		if (tempint==0) {
			pEditbox->SetCandrag(false);
		}else
			pEditbox->SetCandrag(true);
	}
	if (cm->GetIntValue("GUI_imeditbox_control_lifetime",&tempint)==S_OK) {
		pEditbox->m_nLifeTimeCountDown=tempint;
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
	pFontElement->SetElement(pFont,COLOR_ARGB( 255, 16, 16, 16 ),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement, 0 ); // "text"
	pFontElement->SetElement(pFont,COLOR_ARGB( 255, 255, 255, 255 ),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement, 1); //"selected_text"

	string background,filename;
	bool bLoadDefault=true;
	if (cm->GetTextValue("GUI_imeeditbox_control_candidate",background)==S_OK) {
		prect=StringHelper::GetImageAndRect(background,filename, &rect);
		pTexture = CGlobals::GetAssetManager()->LoadTexture(filename.c_str(),filename.c_str(),TextureEntity::StaticTexture);
		if (pTexture!=NULL) 
			bLoadDefault=false;
	} 
	if (bLoadDefault) {
		//Set the default texture and font of the default button
		pTexture=CGlobals::GetAssetManager()->LoadTexture("__Default_GUI", "Texture/dxutcontrols.dds",TextureEntity::StaticTexture);
		prect=&rect;
		SetRect( prect, 0, 0, 136, 54 );
	}
	pElement->SetElement(pTexture,prect,COLOR_ARGB(255,255,255,255));
	pEditbox->m_objResource->AddElement(pElement,"candidate");	
	
	pFontElement->SetElement(pFont,COLOR_ARGB(188, 255, 255, 255 ),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement,2);//"reading_text"
	pFontElement->SetElement(pFont,COLOR_ARGB(255, 255, 0, 0 ),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement,3);	//"selected_reading_text"
	pFontElement->SetElement(pFont,COLOR_ARGB(255, 200, 200, 200),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement,4);	//"candidate_text"
	pFontElement->SetElement(pFont,COLOR_ARGB(255, 255, 255, 255),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement,5);	//"selected_candidate_text"
	pFontElement->SetElement(pFont,COLOR_ARGB(255, 200, 200, 255),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement,6);	//"composition_text"
	pFontElement->SetElement(pFont,COLOR_ARGB(255, 255, 255, 255),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement,7);	//"converted_composition_text"
	pFontElement->SetElement(pFont,COLOR_ARGB(255, 255, 255, 0),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement,8);	//"nonconverted_composition_text"
	pFontElement->SetElement(pFont,COLOR_ARGB(255, 200, 200, 200),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement,9);	//"indicator_ime_text"
	pFontElement->SetElement(pFont,COLOR_ARGB(255, 255, 255, 255),DT_LEFT | DT_TOP);
	pEditbox->m_objResource->SetElement(pFontElement,10);	//"indicator_english_text"

	pEditbox->m_objResource->SetCurrentState();
	pEditbox->m_objResource->SetActiveLayer();
	
	pOm->SetObject("default_CGUIIMEEditBox",pEditbox);
	SAFE_RELEASE(pEditbox);
}

void CGUIIMEEditBox::SendKey( BYTE nVirtKey )
{
	keybd_event( nVirtKey, 0, 0,               0 );
	keybd_event( nVirtKey, 0, KEYEVENTF_KEYUP, 0 );
}

void CGUIIMEEditBox::UpdateRects()
{
	// Temporary adjust m_width so that CDXUTEditBox can compute
	// the correct rects for its rendering since we need to make space
	// for the indicator button
	//int nWidth = m_position.Width();
	//m_position.SetWidth(nWidth - m_nIndicatorWidth + m_nBorder * 2); // Make room for the indicator button
	CGUIEditBox::UpdateRects();
	//m_position.SetWidth(nWidth);  // Restore

	if(s_bRenderLocaleIndicator)
	{
		CGUIPosition tempPos;
		GetAbsolutePosition(&tempPos,&m_position);
		// Compute the indicator button rectangle
		// SetRect(  &m_rcIndicator, tempPos.rect.right-m_nBorder, tempPos.rect.top, tempPos.rect.right+m_nIndicatorWidth+m_nBorder, tempPos.rect.bottom );
		// display within
		SetRect(  &m_rcIndicator, tempPos.rect.right-m_nBorder-m_nIndicatorWidth, tempPos.rect.top, tempPos.rect.right-m_nBorder, tempPos.rect.bottom );
	}
	m_bNeedUpdate=false;
}

bool CGUIIMEEditBox::OnFocusIn()
{
	return CGUIEditBox::OnFocusIn();
}

bool CGUIIMEEditBox::OnFocusOut()
{
	CGUIBase::OnFocusOut();
	return true;
}

bool CGUIIMEEditBox::MsgProc(MSG *event)
{
	if( !m_bIsEnabled )
		return false;
	bool bHandled=false;

	CDirectMouse *pMouse=CGUIRoot::GetInstance()->m_pMouse;
	CDirectKeyboard *pKeyboard=CGUIRoot::GetInstance()->m_pKeyboard;
	GUIFontElement* pFontElement;
	if (event!=NULL&&!m_event->InterpretMessage(event)) {
		return false;
	}
	POINT pt;
	pt.x=m_event->m_mouse.x;
	pt.y=m_event->m_mouse.y;
	int nEvent=m_event->GetTriggerEvent();
	MSG newMsg;
	DWORD dCurrTime=event->time;
	DWORD static dLastTime=0;
	CEventBinding::InitMsg(&newMsg,event->time,nEvent,pt);
#define ENABLE_IME_CLICK_SELECT
#ifdef ENABLE_IME_CLICK_SELECT
	if (m_bIsVisible){
		//		bHandled=true;//editbox intercepts all mouse event regardless of the mouse event type
		if (nEvent==EM_NONE) {
			return false;
		}
		if (m_event->IsMapTo(nEvent,EM_IME_SELECT))
		{
			CIMELock lock;
			pFontElement=m_objResource->GetFontElement(6);
			SpriteFontEntity* pFont = pFontElement->GetFont();
			
			// Check if this click is on top of the composition string
			int nCompStrWidth,nCompStrHeight;
			s_CompString.CPtoXY( s_CompString.GetTextSize(), FALSE, &nCompStrWidth,&nCompStrHeight );

			if( s_ptCompString.x <= pt.x &&
				s_ptCompString.y <= pt.y &&
				s_ptCompString.x + nCompStrWidth > pt.x &&
				s_ptCompString.y + pFont->m_nFontSize > pt.y )
			{
				int nCharBodyHit, nCharHit;
				int nTrail;

				// Determine the character clicked on.
				s_CompString.XYtoCP( pt.x - s_ptCompString.x,pt.y - s_ptCompString.y, &nCharBodyHit, &nTrail );
				if( nTrail && nCharBodyHit < s_CompString.GetTextSize() )
					nCharHit = nCharBodyHit + 1;
				else
					nCharHit = nCharBodyHit;

				// Now generate keypress events to move the comp string cursor
				// to the click point.  First, if the candidate window is displayed,
				// send Esc to close it.
				HIMC hImc = _ImmGetContext(  CGlobals::GetAppHWND() );
				if( !hImc )
					return true;

				_ImmNotifyIME( hImc, NI_CLOSECANDIDATE, 0, 0 );
				_ImmReleaseContext(  CGlobals::GetAppHWND(), hImc );

				switch( GetPrimaryLanguage() )
				{
				case LANG_JAPANESE:
					// For Japanese, there are two cases.  If s_nFirstTargetConv is
					// -1, the comp string hasn't been converted yet, and we use
					// s_nCompCaret.  For any other value of s_nFirstTargetConv,
					// the string has been converted, so we use clause information.

					if( s_nFirstTargetConv != -1 )
					{
						int nClauseClicked = 0;
						while( (int)s_adwCompStringClause[nClauseClicked + 1] <= nCharBodyHit )
							++nClauseClicked;

						int nClauseSelected = 0;
						while( (int)s_adwCompStringClause[nClauseSelected + 1] <= s_nFirstTargetConv )
							++nClauseSelected;

						BYTE nVirtKey = nClauseClicked > nClauseSelected ? VK_RIGHT : VK_LEFT;
						int nSendCount = abs( nClauseClicked - nClauseSelected );
						while( nSendCount-- > 0 )
							SendKey( nVirtKey );

					}

					// Not converted case. Fall thru to Chinese case.

				case LANG_CHINESE:
					{
						// For Chinese, use s_nCompCaret.
						BYTE nVirtKey = nCharHit > s_nCompCaret ? VK_RIGHT : VK_LEFT;
						int nSendCount = abs( nCharHit - s_nCompCaret );
						while( nSendCount-- > 0 )
							SendKey( nVirtKey );
						break;
					}
				}

				bHandled=true;
			}


			if( s_CandList.bShowWindow && PtInRect( s_CandList.rcCandidate, pt ) )
			{
				if( s_bVerticalCand )
				{
					// Vertical candidate window

					// Compute the row the click is on
					int nRow = ( pt.y - s_CandList.rcCandidate.top ) / pFont->m_nFontSize;

					if( nRow < (int)s_CandList.dwCount )
					{
						// nRow is a valid entry.
						// Now emulate keystrokes to select the candidate at this row.
						switch( GetPrimaryLanguage() )
						{
						case LANG_CHINESE:
						case LANG_KOREAN:
							// For Chinese and Korean, simply send the number keystroke.
							SendKey( (BYTE) ('0' + nRow + 1) );
							break;

						case LANG_JAPANESE:
							// For Japanese, move the selection to the target row,
							// then send Right, then send Left.

							BYTE nVirtKey;
							if( nRow > (int)s_CandList.dwSelection )
								nVirtKey = VK_DOWN;
							else
								nVirtKey = VK_UP;
							int nNumToHit = abs( int( nRow - s_CandList.dwSelection ) );
							for( int nStrike = 0; nStrike < nNumToHit; ++nStrike )
								SendKey( nVirtKey );

							// Do this to close the candidate window without ending composition.
							SendKey( VK_RIGHT );
							SendKey( VK_LEFT );

							break;
						}
					}
				} else{
					// Horizontal candidate window

					// Determine which the character the click has hit.
					int nCharHit;
					int nTrail;
					s_CandList.HoriCand.XYtoCP( pt.x - s_CandList.rcCandidate.left,pt.y - s_CandList.rcCandidate.top, &nCharHit, &nTrail );

					// Determine which candidate string the character belongs to.
					int nCandidate = s_CandList.dwCount - 1;

					int nEntryStart = 0;
					for( UINT i = 0; i < s_CandList.dwCount; ++i )
					{
						if( nCharHit >= nEntryStart )
						{
							// Haven't found it.
							nEntryStart += lstrlenW( s_CandList.awszCandidate[i] ) + 1;  // plus space separator
						} else
						{
							// Found it.  This entry starts at the right side of the click point,
							// so the char belongs to the previous entry.
							nCandidate = i - 1;
							break;
						}
					}

					// Now emulate keystrokes to select the candidate entry.
					switch( GetPrimaryLanguage() )
					{
					case LANG_CHINESE:
					case LANG_KOREAN:
						// For Chinese and Korean, simply send the number keystroke.
						SendKey( (BYTE) ('0' + nCandidate + 1) );
						break;
					}
				}
				bHandled=true;
			}
		}
		//TODO: keyboard

	}
#endif
	if (!bHandled) 
	{
		MSG msg = m_event->GenerateMessage();
		return CGUIEditBox::MsgProc(&msg);
	}else
		return true;

}


void CGUIIMEEditBox::RenderCandidateReadingWindow( GUIState* pGUIState , bool bReading ,float fElapsedTime)
{
	RECT rc;
	UINT nNumEntries = bReading ? 4 : MAX_CANDLIST;
	Color TextBkColor, SelBkColor;
	int nX,nY, nXFirst,nYFirst, nXComp,nYComp;
	CPtoXY( m_nCaret, FALSE, &nX, &nY );
	CPtoXY( m_nFirstVisible, FALSE, &nXFirst,&nYFirst );
	GUIFontElement* pFontText; 
	GUIFontElement* pFontSelected;
	RECT rcText=m_objResource->GetDrawingRects(9);
	if( bReading )
	{
		pFontText = m_objResource->GetFontElement(2);
		pFontSelected = m_objResource->GetFontElement(3);
		SelBkColor= m_ReadingSelBkColor;
		TextBkColor= m_ReadingWinColor;
	} else
	{
		pFontText = m_objResource->GetFontElement(4);
		pFontSelected = m_objResource->GetFontElement(5);
		SelBkColor= m_CandidateSelBkColor;
		TextBkColor= m_CandidateWinColor;
	}

	// For Japanese IME, align the window with the first target converted character.
	// For all other IMEs, align with the caret.  This is because the caret
	// does not move for Japanese IME.
	if ( GetImeLanguage() == IMELanguage_ChineseTraditional && !GetImeId() )
		nXComp = 0;
	else
		if( GetPrimaryLanguage() == LANG_JAPANESE )
			s_CompString.CPtoXY( s_nFirstTargetConv, FALSE, &nXComp ,&nYComp);
		else
			s_CompString.CPtoXY( s_nCompCaret, FALSE, &nXComp ,&nYComp );

	// Compute the size of the candidate window
	int nWidthRequired = 0;
	int nHeightRequired = 0;
	int nSingleLineHeight = 0;

	if( ( s_bVerticalCand && !bReading ) ||
		( !s_bHorizontalReading && bReading ) )
	{
		// Vertical window
		for( UINT i = 0; i < nNumEntries; ++i )
		{
			if( s_CandList.awszCandidate[i][0] == L'\0' )
				break;
			SetRect( &rc, 0, 0, 0, 0 );
			GetPainter(pGUIState)->CalcTextRect((const char16_t*)s_CandList.awszCandidate[i], pFontText, &rc, m_position.GetDepth());
			nWidthRequired = Math::Max(nWidthRequired, (int)(rc.right - rc.left));
			nSingleLineHeight = Math::Max( nSingleLineHeight, (int)(rc.bottom - rc.top) );
		}
		nHeightRequired = nSingleLineHeight * nNumEntries;
	} else
	{
		// Horizontal window
		SetRect( &rc, 0, 0, 0, 0 );
		if( bReading )
			GetPainter(pGUIState)->CalcTextRect((const char16_t*)s_wszReadingString, pFontText, &rc, m_position.GetDepth());
		else
			GetPainter(pGUIState)->CalcTextRect(s_CandList.HoriCand.GetBuffer(), pFontText, &rc, m_position.GetDepth());
		nWidthRequired = rc.right - rc.left;
		nSingleLineHeight = nHeightRequired = rc.bottom - rc.top;
	}

	// Now that we have the dimension, calculate the location for the candidate window.
	// We attempt to fit the window in this order:
	// bottom, top, right, left.

	bool bHasPosition = false;

	// Bottom
	SetRect( &rc, s_ptCompString.x + nXComp, s_ptCompString.y + rcText.bottom - rcText.top,
		s_ptCompString.x + nXComp + nWidthRequired, s_ptCompString.y + rcText.bottom - rcText.top + nHeightRequired );
	// if the right edge is cut off, move it left.
	if( rc.right > (long)pGUIState ->nBkbufWidth )
	{
		rc.left -= rc.right - pGUIState ->nBkbufWidth;
		rc.right = pGUIState ->nBkbufWidth;
	}
	if( rc.bottom <= (long)pGUIState->nBkbufHeight )
		bHasPosition = true;

	// Top
	if( !bHasPosition )
	{
		SetRect( &rc, s_ptCompString.x + nXComp, s_ptCompString.y - nHeightRequired,
			s_ptCompString.x + nXComp + nWidthRequired, s_ptCompString.y );
		// if the right edge is cut off, move it left.
		if( rc.right > (long)pGUIState ->nBkbufWidth )
		{
			rc.left -= rc.right - pGUIState ->nBkbufWidth;
			rc.right = pGUIState ->nBkbufWidth;
		}
		if( rc.top >= 0 )
			bHasPosition = true;
	}

	// Right
	if( !bHasPosition )
	{
		int nXCompTrail,nYCompTrail;
		s_CompString.CPtoXY( s_nCompCaret, TRUE, &nXCompTrail,&nYCompTrail );
		SetRect( &rc, s_ptCompString.x + nXCompTrail, 0,
			s_ptCompString.x + nXCompTrail + nWidthRequired, nHeightRequired );
		if( rc.right <= (long)pGUIState ->nBkbufWidth )
			bHasPosition = true;
	}

	// Left
	if( !bHasPosition )
	{
		SetRect( &rc, s_ptCompString.x + nXComp - nWidthRequired, 0,
			s_ptCompString.x + nXComp, nHeightRequired );
		if( rc.right >= 0 )
			bHasPosition = true;
	}

	if( !bHasPosition )
	{
		// The dialog is too small for the candidate window.
		// Fall back to render at 0, 0.  Some part of the window
		// will be cut off.
		rc.left = 0;
		rc.right = nWidthRequired;
	}

	// If we are rendering the candidate window, save the position
	// so that mouse clicks are checked properly.
	if( !bReading )
		s_CandList.rcCandidate = rc;

	// Render the elements
	GetPainter(pGUIState)->DrawRect(&rc, TextBkColor, m_position.GetDepth());
	if( ( s_bVerticalCand && !bReading ) ||
		( !s_bHorizontalReading && bReading ) )
	{
		// Vertical candidate window
		for( UINT i = 0; i < nNumEntries; ++i )
		{
			// Here we are rendering one line at a time
			rc.bottom = rc.top + nSingleLineHeight;
			// Use a different color for the selected string
			if( s_CandList.dwSelection == i )
			{
				GetPainter(pGUIState)->DrawRect(&rc, SelBkColor, m_position.GetDepth());
				GetPainter(pGUIState)->DrawText((const char16_t*)s_CandList.awszCandidate[i], pFontSelected, &rc, m_position.GetDepth());
			} else
				GetPainter(pGUIState)->DrawText((const char16_t*)s_CandList.awszCandidate[i], pFontText, &rc, m_position.GetDepth());


			rc.top += nSingleLineHeight;
		}
	} else
	{
		// Horizontal candidate window
		if( bReading )
			GetPainter(pGUIState)->DrawText((const char16_t*)s_wszReadingString, pFontText, &rc, m_position.GetDepth());
		else
			GetPainter(pGUIState)->DrawText((const char16_t*)s_CandList.HoriCand.GetBuffer(), pFontText, &rc, m_position.GetDepth());

		// Render the selected entry differently
		if( !bReading )
		{
			int nXLeft, nXRight,nYLeft, nYRight;
			s_CandList.HoriCand.CPtoXY( s_CandList.nFirstSelected, FALSE, &nXLeft, &nYLeft );
			s_CandList.HoriCand.CPtoXY( s_CandList.nFirstSelected + s_CandList.nHoriSelectedLen, FALSE, &nXRight, &nYRight );

			rc.right = rc.left + nXRight;
			rc.left += nXLeft;
			GetPainter(pGUIState)->DrawRect(&rc, SelBkColor, m_position.GetDepth());
			GetPainter(pGUIState)->DrawText(s_CandList.HoriCand.GetBuffer() + s_CandList.nFirstSelected,
				pFontSelected, &rc,m_position.GetDepth(), false, s_CandList.nHoriSelectedLen );
		}
	}
}


void CGUIIMEEditBox::RenderComposition( GUIState* pGUIState ,float fElapsedTime )
{
	RECT rcCaret = { 0, 0, 0, 0 };
	int nX, nXFirst,nY, nYFirst;
	RECT rcText=m_objResource->GetDrawingRects(9);
	CPtoXY( m_nCaret, FALSE, &nX,&nY );
	CPtoXY( m_nFirstVisible, FALSE, &nXFirst,&nYFirst );
	//GUITextureElement pElement = m_objResource->GetTextureElement(1);
	GUIFontElement* pFontElement = m_objResource->GetFontElement(6);
	// Get the required width
	RECT rc = { rcText.left + nX - nXFirst, rcText.top,
		rcText.left + nX - nXFirst, rcText.bottom };
	GetPainter(pGUIState)->CalcTextRect(s_CompString.GetBuffer(), pFontElement, &rc, m_position.GetDepth());

	// If the composition string is too long to fit within
	// the text area, move it to below the current line.
	// This matches the behavior of the default IME.
	if( rc.right > rcText.right )
		OffsetRect( &rc, rcText.left - rc.left, rc.bottom - rc.top );

	// Save the rectangle position for processing highlighted text.
	RECT rcFirst = rc;

	// Update s_ptCompString for RenderCandidateReadingWindow().
	s_ptCompString.x = rc.left; s_ptCompString.y = rc.top;

	// Render the window and string.
	// If the string is too long, we must wrap the line.
	//pFontElement.FontColor = TextColor;
	const WCHAR *pwszComp = (const WCHAR*)s_CompString.GetBuffer();
	int nCharLeft = s_CompString.GetTextSize();
	for( ; pwszComp !=0 ; )
	{
		// Find the last character that can be drawn on the same line.
		int nLastInLine;
		int bTrail;
		s_CompString.XYtoCP( rcText.right - rc.left,rcText.top - rc.top, &nLastInLine, &bTrail );
		int nNumCharToDraw = Math::Min( nCharLeft, nLastInLine );
		GetPainter(pGUIState)->CalcTextRect((const char16_t*)pwszComp, pFontElement, &rc, m_position.GetDepth(), nNumCharToDraw);

		// Draw the background
		// For Korean IME, blink the composition window background as if it
		// is a cursor.
		if( GetPrimaryLanguage() == LANG_KOREAN )
		{
			if( IsCaretVisible() )
			{
				GetPainter(pGUIState)->DrawRect(&rc, m_CompWinColor, m_position.GetDepth());
			}
			else
			{
				// Not drawing composition string background. We
				// use the editbox's text color for composition
				// string text.
				//here get normal color
				//TextColor = m_objResource->GetFontElement(1).FontColor;
				pFontElement = m_objResource->GetFontElement(0);
			}
		} else
		{
			// Non-Korean IME. Always draw composition background.
			GetPainter(pGUIState)->DrawRect(&rc, m_CompWinColor, m_position.GetDepth());
		}

		// Draw the text
//		pFontElement.FontColor = TextColor;
		GetPainter(pGUIState)->DrawText((const char16_t*)pwszComp, pFontElement, &rc, m_position.GetDepth(), false, nNumCharToDraw);

		// Advance pointer and counter
		nCharLeft -= nNumCharToDraw;
		pwszComp += nNumCharToDraw;
		if( nCharLeft <= 0 )
			break;

		// Advance rectangle coordinates to beginning of next line
		OffsetRect( &rc, rcText.left - rc.left, rc.bottom - rc.top );
	}

	// Load the rect for the first line again.
	rc = rcFirst;

	// Inspect each character in the comp string.
	// For target-converted and target-non-converted characters,
	// we display a different background color so they appear highlighted.
	int nCharFirst = 0;
	nXFirst = 0;
	s_nFirstTargetConv = -1;
	BYTE *pAttr = s_abCompStringAttr;
	const WCHAR *pcComp = (const WCHAR *)s_CompString.GetBuffer();
	for( ;pcComp!=0 && *pcComp != L'\0'; ++pcComp, ++pAttr )
	{
		Color bkColor;

		// Render a different background for this character
		int nXLeft, nXRight, nYLeft, nYRight;
		s_CompString.CPtoXY( int(pcComp - (const WCHAR*)s_CompString.GetBuffer()), FALSE, &nXLeft, &nYLeft );
		s_CompString.CPtoXY(int(pcComp - (const WCHAR*)s_CompString.GetBuffer()), TRUE, &nXRight, &nYRight);

		// Check if this character is off the right edge and should
		// be wrapped to the next line.
		if( nXRight - nXFirst > rcText.right - rc.left )
		{
			// Advance rectangle coordinates to beginning of next line
			OffsetRect( &rc, rcText.left - rc.left, rc.bottom - rc.top );

			// Update the line's first character information
			nCharFirst = int(pcComp - (const WCHAR*)s_CompString.GetBuffer());
			s_CompString.CPtoXY( nCharFirst, FALSE, &nXFirst, &nYFirst );
		}

		// If the caret is on this character, save the coordinates
		// for drawing the caret later.
		if (s_nCompCaret == int(pcComp - (const WCHAR*)s_CompString.GetBuffer()))
		{
			rcCaret = rc;
			rcCaret.left += nXLeft - nXFirst - 1;
			rcCaret.right = rcCaret.left + 2;
		}

		// Set up color based on the character attribute
		if( *pAttr == ATTR_TARGET_CONVERTED )
		{
			pFontElement=m_objResource->GetFontElement(7);
			bkColor = m_CompTargetBkColor;
		}
		else
			if( *pAttr == ATTR_TARGET_NOTCONVERTED )
			{
				pFontElement=m_objResource->GetFontElement(8);
				bkColor = m_CompTargetNonBkColor;
			}
			else
			{
				continue;
			}

			RECT rcTarget = { rc.left + nXLeft - nXFirst, rc.top, rc.left + nXRight - nXFirst, rc.bottom };
			GetPainter(pGUIState)->DrawRect(&rcTarget, bkColor, m_position.GetDepth());
			GetPainter(pGUIState)->DrawText((const char16_t*)pcComp, pFontElement, &rcTarget, m_position.GetDepth(), false, 1);

			// Record the first target converted character's index
			if( -1 == s_nFirstTargetConv )
				s_nFirstTargetConv = int(pAttr - s_abCompStringAttr);
	}

	// Render the composition caret
	if( IsCaretVisible() )
	{
		// If the caret is at the very end, its position would not have
		// been computed in the above loop.  We compute it here.
		if( s_nCompCaret == s_CompString.GetTextSize() )
		{
			s_CompString.CPtoXY( s_nCompCaret, FALSE, &nX,&nY );
			rcCaret = rc;
			rcCaret.left += nX - nXFirst - 1;
			rcCaret.right = rcCaret.left + 2;
		}

		GetPainter(pGUIState)->DrawRect(&rcCaret, m_CompCaretColor, m_position.GetDepth());
	}
}

void CGUIIMEEditBox::RenderIndicator( GUIState* pGUIState,float fElapsedTime  )
{
	// If IME system is off, draw nothing instead of English indicator.
	if(s_bEnableImeSystem && s_ImeState != IMEUI_STATE_OFF)
	{
		// render a grey background
		GetPainter(pGUIState)->DrawRect(&m_rcIndicator, m_IndicatorBkColor, m_position.GetDepth());
		//m_objResource->DrawSprite( m_objResource->GetTextureElement(9), &m_rcIndicator,m_position.GetDepth() );

		RECT rc = m_rcIndicator;
		InflateRect( &rc, -m_nSpacing, -m_nSpacing );
		GUIFontElement* pFontElement = m_objResource->GetFontElement(9);
		
		RECT rcCalc = { 0, 0, 0, 0 };
		
		WCHAR *pwszIndicator = s_wszCurrIndicator; //  : s_aszIndicator[0]
		if(s_ImeState == IMEUI_STATE_ENGLISH)
		{
			pwszIndicator = CGUIIME::s_aszIndicator[0];
		}
		GetPainter(pGUIState)->CalcTextRect((const char16_t*)pwszIndicator, pFontElement, &rcCalc, m_position.GetDepth());
		GetPainter(pGUIState)->DrawText((const char16_t*)pwszIndicator, pFontElement, &rc, m_position.GetDepth());
	}
}

HRESULT CGUIIMEEditBox::Render(GUIState* pGUIState,float fElapsedTime )
{
	if( m_bIsVisible == false )
		return S_OK;
	SetDirty(false);
	GUIFontElement* pFontElement;

	m_objResource->SetActiveLayer();
	m_objResource->SetCurrentState();

	// If we have not computed the indicator symbol width,
	// do it.
	if( !m_nIndicatorWidth )
	{
		for( int i = 0; i < 5; ++i )
		{
			RECT rc = { 0, 0, 0, 0 };

			pFontElement=m_objResource->GetFontElement(9);
			GetPainter(pGUIState)->CalcTextRect((const char16_t*)s_aszIndicator[i], pFontElement, &rc, m_position.GetDepth());
			m_nIndicatorWidth = Math::Max(m_nIndicatorWidth, (int)(rc.right - rc.left));
		}
		// Update the rectangles now that we have the indicator's width
		UpdateRects();
	}

	// Let the parent render first (edit control)
	CGUIEditBox::Render( pGUIState,fElapsedTime);

	// now render the IME indicator
	{
		CIMELock lock;
		pFontElement = m_objResource->GetFontElement(0);
		s_CompString.SetFontNode( pFontElement);
		s_CandList.HoriCand.SetFontNode( pFontElement);

		if( m_bHasFocus && !m_bReadOnly )
		{
			if(s_bRenderLocaleIndicator)
			{
				// Render the input locale indicator
				RenderIndicator( pGUIState, fElapsedTime);
			}
		}
	}
	
	return S_OK;
}

/**Font Element
0 - text
1 - selected text
2 - reading text
3 - selected reading text
4 - candidate text
5 - selected candidate text
6 - composition text
7 - composition target converted text
8 - composition target non-converted text
9 - indicator ime text
10 - indicator english text
*/
void CGUIIMEEditBox::InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height)
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	if (!pOm->IsExist("default_CGUIIMEEditBox")) {
		CGUIIMEEditBox::StaticInit();
	}
	pOm->CloneObject("default_CGUIIMEEditBox",this);
	CGUIBase::InitObject(strObjectName,alignment,x,y,width,height);

}

void CGUIIMEEditBox::PostRender(GUIState *pGUIState,float fElapsedTime)
{
	CIMELock lock;
	//
	// Now render the IME elements
	//
	if( m_bHasFocus && GetVisibleRecursive() && (GetColorMask()&0xff000000)!=0)
	{
		//if(s_bRenderLocaleIndicator)
		//{
		//	// Render the input locale indicator
		//	RenderIndicator( pGUIState, fElapsedTime);
		//}

		// Display the composition string.
		// This method should also update s_ptCompString
		// for RenderCandidateReadingWindow.
		RenderComposition( pGUIState,fElapsedTime);

		// Display the reading/candidate window. RenderCandidateReadingWindow()
		// uses s_ptCompString to position itself.  s_ptCompString must have
		// been filled in by RenderComposition().
		if( s_bShowReadingWindow )
		{
			// Reading window
			RenderCandidateReadingWindow( pGUIState, true,fElapsedTime );
		}
		else if( s_CandList.bShowWindow )
		{
			// Candidate list window
			RenderCandidateReadingWindow( pGUIState, false,fElapsedTime );
		}
	}
}
bool CGUIIMEEditBox::IsHideCaret()
{
	return s_bHideCaret;
}

int CGUIIMEEditBox::InstallFields( CAttributeClass* pClass, bool bOverride )
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	CGUIEditBox::InstallFields(pClass, bOverride);
	PE_ASSERT(pClass!=NULL);

	return S_OK;
}

bool ParaEngine::CGUIIMEEditBox::FetchIMEString()
{
	if (CGUIBase::FetchIMEString())
	{
		wstring sText = CGUIIME::GetCurrentCompString();
		if (!sText.empty())
		{
			for (int i = 0; i < (int)(sText.size()); ++i)
			{
				InsertChar(sText[i]);
			}
			return true;
		}
	}
	return false;
}
#endif