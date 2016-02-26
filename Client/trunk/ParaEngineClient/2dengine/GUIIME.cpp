//----------------------------------------------------------------------
// Class:	CGUIIME
// Authors:	LiXizhi, Liu Weili
// Company: ParaEngine
// Date:	2005.8.3
// Revised: 2010.3.10
//
// desc: 
// The Edit control provides text edit without IME support.
//
// Most are from DirectX 9.0c SDK
// @changes LiXizhi 2006.8.28: PasswordChar implemented.
// @changes LiXizhi 2007.7.26: caret may only be hidden for IMEeditbox. for Editbox, it is always false. 
// @changes LiXizhi 2009.11.7: Information on IME: http://msdn.microsoft.com/en-us/library/ee419002(VS.85).aspx 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "GUIRoot.h"
#include "GUIBase.h"
#include <strsafe.h>
#include <vector>
#include "EventBinding.h"
#include "ParaEngineApp.h"
#include "GUIIME.h"
#include "memdebug.h"

using namespace ParaEngine;
using namespace std;
#define UNISCRIBE_DLLNAME "\\usp10.dll"

#define GETPROCADDRESS( Module, APIName, Temp ) \
	Temp = GetProcAddress( Module, #APIName ); \
	if( Temp ) \
	*(FARPROC*)&_##APIName = Temp

#define PLACEHOLDERPROC( APIName ) \
	_##APIName = Dummy_##APIName

#define DXUT_MAX_EDITBOXLENGTH 0xFFFF

#define IMM32_DLLNAME "\\imm32.dll"
#define VER_DLLNAME "\\version.dll"

//--------------------------------------------------------------------------------------
// CGUIIME class
//--------------------------------------------------------------------------------------
// IME constants
#define CHT_IMEFILENAME1    "TINTLGNT.IME" // New Phonetic
#define CHT_IMEFILENAME2    "CINTLGNT.IME" // New Chang Jie
#define CHT_IMEFILENAME3    "MSTCIPHA.IME" // Phonetic 5.1
#define CHS_IMEFILENAME1    "PINTLGNT.IME" // MSPY1.5/2/3
#define CHS_IMEFILENAME2    "MSSCIPYA.IME" // MSPY3 for OfficeXP

#define LANG_CHT            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define _CHT_HKL            ( (HKL)(INT_PTR)0xE0080404 ) // New Phonetic
#define _CHT_HKL2           ( (HKL)(INT_PTR)0xE0090404 ) // New Chang Jie
#define _CHS_HKL            ( (HKL)(INT_PTR)0xE00E0804 ) // MSPY
#define MAKEIMEVERSION( major, minor )      ( (DWORD)( ( (BYTE)( major ) << 24 ) | ( (BYTE)( minor ) << 16 ) ) )

#define IMEID_CHT_VER42 ( LANG_CHT | MAKEIMEVERSION( 4, 2 ) )   // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43 ( LANG_CHT | MAKEIMEVERSION( 4, 3 ) )   // New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44 ( LANG_CHT | MAKEIMEVERSION( 4, 4 ) )   // New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50 ( LANG_CHT | MAKEIMEVERSION( 5, 0 ) )   // New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51 ( LANG_CHT | MAKEIMEVERSION( 5, 1 ) )   // New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52 ( LANG_CHT | MAKEIMEVERSION( 5, 2 ) )   // New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/Whistler)
#define IMEID_CHT_VER60 ( LANG_CHT | MAKEIMEVERSION( 6, 0 ) )   // New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // IME XP(w/WinXP SP1)
#define IMEID_CHS_VER41 ( LANG_CHS | MAKEIMEVERSION( 4, 1 ) )   // MSPY1.5  // SCIME97 or MSPY1.5 (w/Win98, Office97)
#define IMEID_CHS_VER42 ( LANG_CHS | MAKEIMEVERSION( 4, 2 ) )   // MSPY2    // Win2k/WinME
#define IMEID_CHS_VER53 ( LANG_CHS | MAKEIMEVERSION( 5, 3 ) )   // MSPY3    // WinXP

namespace ParaEngine
{

// Function pointers
INPUTCONTEXT* (WINAPI * CGUIIME::_ImmLockIMC)( HIMC ) = CGUIIME::Dummy_ImmLockIMC;
BOOL (WINAPI * CGUIIME::_ImmUnlockIMC)( HIMC ) = CGUIIME::Dummy_ImmUnlockIMC;
LPVOID (WINAPI * CGUIIME::_ImmLockIMCC)( HIMCC ) = CGUIIME::Dummy_ImmLockIMCC;
BOOL (WINAPI * CGUIIME::_ImmUnlockIMCC)( HIMCC ) = CGUIIME::Dummy_ImmUnlockIMCC;
BOOL (WINAPI * CGUIIME::_ImmDisableTextFrameService)( DWORD ) = CGUIIME::Dummy_ImmDisableTextFrameService;
LONG (WINAPI * CGUIIME::_ImmGetCompositionStringW)( HIMC, DWORD, LPVOID, DWORD ) = CGUIIME::Dummy_ImmGetCompositionStringW;
DWORD (WINAPI * CGUIIME::_ImmGetCandidateListW)( HIMC, DWORD, LPCANDIDATELIST, DWORD ) = CGUIIME::Dummy_ImmGetCandidateListW;
HIMC (WINAPI * CGUIIME::_ImmCreateContext)() = CGUIIME::Dummy_ImmCreateContext;
BOOL (WINAPI * CGUIIME::_ImmDestroyContext)( HIMC hIMC ) = CGUIIME::Dummy_ImmDestroyContext;
HIMC (WINAPI * CGUIIME::_ImmGetContext)( HWND ) = CGUIIME::Dummy_ImmGetContext;
BOOL (WINAPI * CGUIIME::_ImmReleaseContext)( HWND, HIMC ) = CGUIIME::Dummy_ImmReleaseContext;
HIMC (WINAPI * CGUIIME::_ImmAssociateContext)( HWND, HIMC ) = CGUIIME::Dummy_ImmAssociateContext;
BOOL (WINAPI * CGUIIME::_ImmGetOpenStatus)( HIMC ) = CGUIIME::Dummy_ImmGetOpenStatus;
BOOL (WINAPI * CGUIIME::_ImmSetOpenStatus)( HIMC, BOOL ) = CGUIIME::Dummy_ImmSetOpenStatus;
BOOL (WINAPI * CGUIIME::_ImmGetConversionStatus)( HIMC, LPDWORD, LPDWORD ) = CGUIIME::Dummy_ImmGetConversionStatus;
HWND (WINAPI * CGUIIME::_ImmGetDefaultIMEWnd)( HWND ) = CGUIIME::Dummy_ImmGetDefaultIMEWnd;
UINT (WINAPI * CGUIIME::_ImmGetIMEFileNameA)( HKL, LPSTR, UINT ) = CGUIIME::Dummy_ImmGetIMEFileNameA;
UINT (WINAPI * CGUIIME::_ImmGetVirtualKey)( HWND ) = CGUIIME::Dummy_ImmGetVirtualKey;
BOOL (WINAPI * CGUIIME::_ImmNotifyIME)( HIMC, DWORD, DWORD, DWORD ) = CGUIIME::Dummy_ImmNotifyIME;
BOOL (WINAPI * CGUIIME::_ImmSetConversionStatus)( HIMC, DWORD, DWORD ) = CGUIIME::Dummy_ImmSetConversionStatus;
BOOL (WINAPI * CGUIIME::_ImmSimulateHotKey)( HWND, DWORD ) = CGUIIME::Dummy_ImmSimulateHotKey;
BOOL (WINAPI * CGUIIME::_ImmIsIME)( HKL ) = CGUIIME::Dummy_ImmIsIME;
BOOL (WINAPI * CGUIIME::_ImmSetCandidateWindow)( HIMC , LPCANDIDATEFORM) = CGUIIME::Dummy_ImmSetCandidateWindow;
UINT (WINAPI * CGUIIME::_GetReadingString)( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT ) = CGUIIME::Dummy_GetReadingString; // Traditional Chinese IME
BOOL (WINAPI * CGUIIME::_ShowReadingWindow)( HIMC, BOOL ) = CGUIIME::Dummy_ShowReadingWindow; // Traditional Chinese IME
BOOL (APIENTRY * CGUIIME::_VerQueryValueA)( const LPVOID, LPSTR, LPVOID *, PUINT ) = CGUIIME::Dummy_VerQueryValueA;
BOOL (APIENTRY * CGUIIME::_GetFileVersionInfoA)( LPSTR, DWORD, DWORD, LPVOID )= CGUIIME::Dummy_GetFileVersionInfoA;
DWORD (APIENTRY * CGUIIME::_GetFileVersionInfoSizeA)( LPSTR, LPDWORD ) = CGUIIME::Dummy_GetFileVersionInfoSizeA;

HINSTANCE CGUIIME::s_hDllImm32;      // IMM32 DLL handle
HINSTANCE CGUIIME::s_hDllVer;        // Version DLL handle
HKL       CGUIIME::s_hklCurrent;     // Current keyboard layout of the process
bool      CGUIIME::s_bVerticalCand;  // Indicates that the candidates are listed vertically
WCHAR     CGUIIME::s_aszIndicator[5][3] = // string to draw to indicate current input locale
{
	L"En",
	L"\x7B80",
	L"\x7E41",
	L"\xAC00",
	L"\x3042",
};

std::wstring CGUIIME::s_sLastWindowsChars;

std::wstring CGUIIME::s_sCurrentCompString;

LPWSTR    CGUIIME::s_wszCurrIndicator = CGUIIME::s_aszIndicator[0];  // Points to an indicator string that corresponds to current input locale
bool      CGUIIME::s_bInsertOnType;     // Insert the character as soon as a key is pressed (Korean behavior)
HINSTANCE CGUIIME::s_hDllIme;           // Instance handle of the current IME module
HIMC      CGUIIME::s_hImcDef = NULL;           // Default input context
IMESTATE  CGUIIME::s_ImeState = IMEUI_STATE_OFF;
bool      CGUIIME::s_bEnableImeSystem = true;  // Whether the IME system is active
POINT     CGUIIME::s_ptCompString;      // Composition string position. Updated every frame.
int       CGUIIME::s_nCompCaret;
int       CGUIIME::s_nFirstTargetConv;  // Index of the first target converted char in comp string.  If none, -1.
CUniBuffer CGUIIME::s_CompString = CUniBuffer( 0 );
BYTE      CGUIIME::s_abCompStringAttr[MAX_COMPSTRING_SIZE];
DWORD     CGUIIME::s_adwCompStringClause[MAX_COMPSTRING_SIZE];
WCHAR     CGUIIME::s_wszReadingString[32];
CGUIIME::CCandList CGUIIME::s_CandList;       // Data relevant to the candidate list
bool      CGUIIME::s_bShowReadingWindow; // Indicates whether reading window is visible
bool      CGUIIME::s_bHorizontalReading; // Indicates whether the reading window is vertical or horizontal
bool      CGUIIME::s_bChineseIME;
CGrowableArray< CGUIIME::CInputLocale > CGUIIME::s_Locale; // Array of loaded keyboard layout on system
#if defined(DEBUG) | defined(_DEBUG)
bool      CGUIIME::m_bIMEStaticMsgProcCalled = false;
#endif
POINT CGUIIME::m_nCompositionWindowPoint = {0,0};

ParaEngine::mutex CGUIIME::s_mutex; // IME lock

CIMELock::CIMELock() :ParaEngine::mutex::ScopedLock(CGUIIME::s_mutex){}

bool CGUIIME::s_bHideCaret = false;   // If true, we don't render the caret.

/** whether to render locale indicator at the end of imeeditor box */
bool CGUIIME::s_bRenderLocaleIndicator = true;
bool CGUIIME::s_bIMETrap = false;
bool CGUIIME::s_bIMEHasFocus = false;
// true to create our own IME context instead of using the default one shared by the window thread. The default context is created automatically by windows and used by all windows of that thread. 
bool CGUIIME::s_bCreateIMEContext = false;
std::wstring CGUIIME::s_sLastCompString;

bool CGUIIME::m_bInsertMode = false;
bool CGUIIME::m_bCaretOn = false;
double CGUIIME::m_dfLastBlink = 0;

// whether IME is created in the window thread. 
static bool s_is_ime_created = false;
}

//--------------------------------------------------------------------------------------
//  GetImeId( UINT uIndex )
//      returns 
//  returned value:
//  0: In the following cases
//      - Non Chinese IME input locale
//      - Older Chinese IME
//      - Other error cases
//
//  Othewise:
//      When uIndex is 0 (default)
//          bit 31-24:  Major version
//          bit 23-16:  Minor version
//          bit 15-0:   Language ID
//      When uIndex is 1
//          pVerFixedInfo->dwFileVersionLS
//
//  Use IMEID_VER and IMEID_LANG macro to extract version and language information.
//  

// We define the locale-invariant ID ourselves since it doesn't exist prior to WinXP
// For more information, see the CompareString() reference.
#define LCID_INVARIANT MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

DWORD CGUIIME::GetImeId( UINT uIndex )
{
	CIMELock lock;
	static HKL hklPrev = 0;
	static DWORD dwID[2] = { 0, 0 };  // Cache the result

	DWORD   dwVerSize;
	DWORD   dwVerHandle;
	LPVOID  lpVerBuffer;
	LPVOID  lpVerData;
	UINT    cbVerData;
	char    szTmp[1024];

	if( uIndex >= sizeof( dwID ) / sizeof( dwID[0] ) )
		return 0;

	if( hklPrev == s_hklCurrent )
		return dwID[uIndex];

	hklPrev = s_hklCurrent;  // Save for the next invocation

	// Check if we are using an older Chinese IME
	if( !( ( s_hklCurrent == _CHT_HKL ) || ( s_hklCurrent == _CHT_HKL2 ) || ( s_hklCurrent == _CHS_HKL ) ) )
	{
		dwID[0] = dwID[1] = 0;
		return dwID[uIndex];
	}

	// Obtain the IME file name
	if ( !_ImmGetIMEFileNameA( s_hklCurrent, szTmp, ( sizeof(szTmp) / sizeof(szTmp[0]) ) - 1 ) )
	{
		dwID[0] = dwID[1] = 0;
		return dwID[uIndex];
	}

	// Check for IME that doesn't implement reading string API
	if ( !_GetReadingString )
	{
		if( ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHT_IMEFILENAME1, -1 ) != CSTR_EQUAL ) &&
			( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHT_IMEFILENAME2, -1 ) != CSTR_EQUAL ) &&
			( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHT_IMEFILENAME3, -1 ) != CSTR_EQUAL ) &&
			( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHS_IMEFILENAME1, -1 ) != CSTR_EQUAL ) &&
			( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHS_IMEFILENAME2, -1 ) != CSTR_EQUAL ) )
		{
			dwID[0] = dwID[1] = 0;
			return dwID[uIndex];
		}
	}

	dwVerSize = _GetFileVersionInfoSizeA( szTmp, &dwVerHandle );
	if( dwVerSize )
	{
		lpVerBuffer = HeapAlloc( GetProcessHeap(), 0, dwVerSize );
		if( lpVerBuffer )
		{
			if( _GetFileVersionInfoA( szTmp, dwVerHandle, dwVerSize, lpVerBuffer ) )
			{
				if( _VerQueryValueA( lpVerBuffer, "\\", &lpVerData, &cbVerData ) )
				{
					DWORD dwVer = ( (VS_FIXEDFILEINFO*)lpVerData )->dwFileVersionMS;
					dwVer = ( dwVer & 0x00ff0000 ) << 8 | ( dwVer & 0x000000ff ) << 16;
					if( _GetReadingString
						||
						( GetLanguage() == LANG_CHT &&
						( dwVer == MAKEIMEVERSION(4, 2) || 
						dwVer == MAKEIMEVERSION(4, 3) || 
						dwVer == MAKEIMEVERSION(4, 4) || 
						dwVer == MAKEIMEVERSION(5, 0) ||
						dwVer == MAKEIMEVERSION(5, 1) ||
						dwVer == MAKEIMEVERSION(5, 2) ||
						dwVer == MAKEIMEVERSION(6, 0) ) )
						||
						( GetLanguage() == LANG_CHS &&
						( dwVer == MAKEIMEVERSION(4, 1) ||
						dwVer == MAKEIMEVERSION(4, 2) ||
						dwVer == MAKEIMEVERSION(5, 3) ) )
						)
					{
						dwID[0] = dwVer | GetLanguage();
						dwID[1] = ( (VS_FIXEDFILEINFO*)lpVerData )->dwFileVersionLS;
					}
				}
			}
			HeapFree( GetProcessHeap(), 0, lpVerBuffer );
		}
	}

	return dwID[uIndex];
}


//--------------------------------------------------------------------------------------
void CGUIIME::CheckInputLocale()
{
	CIMELock lock;
	static HKL hklPrev = 0;
	s_hklCurrent = GetKeyboardLayout( 0 );
	if ( hklPrev == s_hklCurrent )
		return;

	hklPrev = s_hklCurrent;
	switch ( GetPrimaryLanguage() )
	{
		// Simplified Chinese
	case LANG_CHINESE:
		s_bVerticalCand = true;
		switch ( GetSubLanguage() )
		{
		case SUBLANG_CHINESE_SIMPLIFIED:
			s_wszCurrIndicator = s_aszIndicator[INDICATOR_CHS];
			s_bVerticalCand = GetImeId() == 0;
			break;
		case SUBLANG_CHINESE_TRADITIONAL:
			s_wszCurrIndicator = s_aszIndicator[INDICATOR_CHT];
			s_bVerticalCand = GetImeId() == 0;
			break;
		default:    // unsupported sub-language
			s_wszCurrIndicator = s_aszIndicator[INDICATOR_NON_IME];
			break;
		}
		break;
		// Korean
	case LANG_KOREAN:
		s_wszCurrIndicator = s_aszIndicator[INDICATOR_KOREAN];
		s_bVerticalCand = false;
		break;
		// Japanese
	case LANG_JAPANESE:
		s_wszCurrIndicator = s_aszIndicator[INDICATOR_JAPANESE];
		s_bVerticalCand = true;
		break;
	default:
		// A non-IME language.  Obtain the language abbreviation
		// and store it for rendering the indicator later.
		s_wszCurrIndicator = s_aszIndicator[INDICATOR_NON_IME];
	}
	
	// If non-IME, use the language abbreviation.
	if( s_wszCurrIndicator == s_aszIndicator[INDICATOR_NON_IME] )
	{
		WCHAR wszLang[5];
		GetLocaleInfoW( MAKELCID( LOWORD( s_hklCurrent ), SORT_DEFAULT ), LOCALE_SABBREVLANGNAME, wszLang, 5 );
		s_wszCurrIndicator[0] = wszLang[0];
		s_wszCurrIndicator[1] = towlower( wszLang[1] );
	}
}


//--------------------------------------------------------------------------------------
void CGUIIME::CheckToggleState()
{
	CIMELock lock;
	CheckInputLocale();

	bool bIme = _ImmIsIME( s_hklCurrent ) != 0;
	//bool bIme = _ImmIsIME( s_hklCurrent ) != 0 
	//	&& ( ( 0xF0000000 & (DWORD)s_hklCurrent ) == 0xE0000000 ); // Hack to detect IME correctly. When IME is running as TIP, ImmIsIME() returns true for CHT US keyboard.

	s_bChineseIME = ( GetPrimaryLanguage() == LANG_CHINESE ) && bIme;

	HIMC hImc;
	if( NULL != ( hImc = _ImmGetContext(  CGlobals::GetAppHWND()) ) )
	{
		if( s_bChineseIME )
		{
			DWORD dwConvMode, dwSentMode;
			BOOL bRes = _ImmGetConversionStatus( hImc, &dwConvMode, &dwSentMode );
			if(bRes)
			{
				s_ImeState = ( dwConvMode & IME_CMODE_NATIVE ) ? IMEUI_STATE_ON : IMEUI_STATE_ENGLISH;
			}
			else
			{
				s_ImeState = IMEUI_STATE_OFF;
			}
		}
		else
		{
			s_ImeState = ( bIme && _ImmGetOpenStatus( hImc ) != 0 ) ? IMEUI_STATE_ON : IMEUI_STATE_OFF;
		}
		_ImmReleaseContext(  CGlobals::GetAppHWND(), hImc );
	}
	else
		s_ImeState = IMEUI_STATE_OFF;
}


//--------------------------------------------------------------------------------------
// Enable/disable the entire IME system.  When disabled, the default IME handling
// kicks in.
void CGUIIME::EnableImeSystem( bool bEnable )
{
	CIMELock lock;
	s_bEnableImeSystem = bEnable;
}

bool CGUIIME::IsEnableImeSystem()
{
	CIMELock lock;
	bool  bEnable = s_bEnableImeSystem;
	return bEnable;
}

void CGUIIME::SetCompositionPosition( int x, int y )
{
	CIMELock lock;
	m_nCompositionWindowPoint.x = x;
	m_nCompositionWindowPoint.y = y;
}


//--------------------------------------------------------------------------------------
// Sets up IME-specific APIs for the IME edit controls.  This is called every time
// the input locale changes.
void CGUIIME::SetupImeApi()
{
	CIMELock lock;
	char szImeFile[MAX_PATH + 1];

	_GetReadingString = NULL;
	_ShowReadingWindow = NULL;
	if( _ImmGetIMEFileNameA( s_hklCurrent, szImeFile, sizeof(szImeFile)/sizeof(szImeFile[0]) - 1 ) == 0 )
		return;

	if( s_hDllIme ) FreeLibrary( s_hDllIme );
	s_hDllIme = LoadLibraryA( szImeFile );
	if ( !s_hDllIme )
		return;

	// Yes, the following functions can be 0 in good situations.
	_GetReadingString = (UINT (WINAPI*)(HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT))
		( GetProcAddress( s_hDllIme, "GetReadingString" ) );
	_ShowReadingWindow =(BOOL (WINAPI*)(HIMC, BOOL))
		( GetProcAddress( s_hDllIme, "ShowReadingWindow" ) );
}


//--------------------------------------------------------------------------------------
// Resets the composition string.
void CGUIIME::ResetCompositionString()
{
	CIMELock lock;
	s_nCompCaret = 0;
	s_CompString.SetText( (const char16_t*)L"" );
	ZeroMemory( s_abCompStringAttr, sizeof(s_abCompStringAttr) );
}


//--------------------------------------------------------------------------------------
// Truncate composition string by sending keystrokes to the window.
void CGUIIME::TruncateCompString( bool bUseBackSpace, int iNewStrLen )
{
	CIMELock lock;
	if( !s_bInsertOnType )
		return;

	int cc = (int) wcslen( (const WCHAR*)s_CompString.GetBuffer() );
	PE_ASSERT( iNewStrLen == 0 || iNewStrLen >= cc );

	// Send right arrow keystrokes to move the caret
	//   to the end of the composition string.
	for (int i = 0; i < cc - s_nCompCaret; ++i )
		SendMessage( CGlobals::GetAppHWND(), WM_KEYDOWN, VK_RIGHT, 0 );
	SendMessage( CGlobals::GetAppHWND(), WM_KEYUP, VK_RIGHT, 0 );

	if( bUseBackSpace || m_bInsertMode )
		iNewStrLen = 0;

	// The caller sets bUseBackSpace to false if there's possibility of sending
	// new composition string to the app right after this function call.
	// 
	// If the app is in overwriting mode and new comp string is 
	// shorter than current one, delete previous comp string 
	// till it's same long as the new one. Then move caret to the beginning of comp string.
	// New comp string will overwrite old one.
	if( iNewStrLen < cc )
	{
		for( int i = 0; i < cc - iNewStrLen; ++i )
		{
			SendMessage( CGlobals::GetAppHWND(), WM_KEYDOWN, VK_BACK, 0 );  // Backspace character
			SendMessageW( CGlobals::GetAppHWND(), WM_CHAR, VK_BACK, 0 );
		}
		SendMessage( CGlobals::GetAppHWND(), WM_KEYUP, VK_BACK, 0 );
	}
	else
		iNewStrLen = cc;

	// Move the caret to the beginning by sending left keystrokes
	for (int i = 0; i < iNewStrLen; ++i )
		SendMessage( CGlobals::GetAppHWND(), WM_KEYDOWN, VK_LEFT, 0 );
	SendMessage( CGlobals::GetAppHWND(), WM_KEYUP, VK_LEFT, 0 );
}


//--------------------------------------------------------------------------------------
// Sends the current composition string to the application by sending keystroke
// messages.
void CGUIIME::SendCompString()
{
	CIMELock lock;
	// Note: we can send via a special WM_IME_CHAR or WM_CHAR message or just send to the s_sLastCompString buffer. 
	/*int nSize = lstrlenW( s_CompString.GetBuffer() );
	for( int i = 0; i < nSize; ++i )
	{
	WinMsgProc( WM_CHAR, (WPARAM)s_CompString[i], 0 );
	}*/
	s_sLastCompString = (const WCHAR*)s_CompString.GetBuffer();
}

std::wstring CGUIIME::GetLastCompString( bool bRemove /*= true*/ )
{
	CIMELock lock;
	std::wstring sText = s_sLastCompString;
	if (bRemove){
		s_sCurrentCompString = s_sLastCompString;
		s_sLastCompString.clear();
	}
	return sText;
}


std::wstring CGUIIME::GetCurrentCompString()
{
	CIMELock lock;
	std::wstring sText = s_sCurrentCompString;
	return sText;
}


void CGUIIME::SendWinMsgChar( WCHAR c )
{
	CIMELock lock;
	s_sLastWindowsChars.push_back(c);
	// we will only keep the most recent 64 letters, and discard older chars.  
	if(s_sLastWindowsChars.size()>64)
		s_sLastWindowsChars.resize(64);
}

std::wstring CGUIIME::GetWinMsgChar( bool bRemove /*= true*/ )
{
	CIMELock lock;
	std::wstring sText = s_sLastWindowsChars;
	if(bRemove)
		s_sLastWindowsChars.clear();
	return sText;
}
//--------------------------------------------------------------------------------------
// Outputs current composition string then cleans up the composition task.
void CGUIIME::FinalizeString( bool bSend )
{
	CIMELock lock;
	HIMC hImc;
	if( NULL == ( hImc = _ImmGetContext(  CGlobals::GetAppHWND() ) ) )
		return;

	static bool bProcessing = false;
	if( bProcessing )    // avoid infinite recursion
	{
	//	DXUTTRACE( L"CGUIIME::FinalizeString: Reentrant detected!\n" );
		_ImmReleaseContext(  CGlobals::GetAppHWND(), hImc );
		return;
	}
	bProcessing = true;

	if( !s_bInsertOnType && bSend )
	{
		// Send composition string to app.
		LONG lLength = lstrlenW( (const WCHAR*)s_CompString.GetBuffer() );
		// In case of CHT IME, don't send the trailing double byte space, if it exists.
		if( GetLanguage() == LANG_CHT
			&& s_CompString[lLength - 1] == 0x3000 )
		{
			s_CompString[lLength - 1] = 0;
		}
		SendCompString();
		s_CandList.HoriCand.SetText((const char16_t*)L"");
	}

	ResetCompositionString();
	// Clear composition string in IME
	_ImmNotifyIME( hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );
	// the following line is necessary as Korean IME doesn't close cand list
	// when comp string is cancelled.
	_ImmNotifyIME( hImc, NI_CLOSECANDIDATE, 0, 0 ); 
	_ImmReleaseContext(  CGlobals::GetAppHWND(), hImc );
	bProcessing = false;
}


//--------------------------------------------------------------------------------------
// Determine whether the reading window should be vertical or horizontal.
void CGUIIME::GetReadingWindowOrientation( DWORD dwId )
{
	CIMELock lock;
	s_bHorizontalReading = ( s_hklCurrent == _CHS_HKL ) || ( s_hklCurrent == _CHT_HKL2 ) || ( dwId == 0 );
	if( !s_bHorizontalReading && ( dwId & 0x0000FFFF ) == LANG_CHT )
	{
		WCHAR wszRegPath[MAX_PATH];
		HKEY hKey;
		DWORD dwVer = dwId & 0xFFFF0000;
		StringCchCopyW( wszRegPath, MAX_PATH, L"software\\microsoft\\windows\\currentversion\\" );
		StringCchCatW( wszRegPath, MAX_PATH, ( dwVer >= MAKEIMEVERSION( 5, 1 ) ) ? L"MSTCIPH" : L"TINTLGNT" );
		LONG lRc = RegOpenKeyExW( HKEY_CURRENT_USER, wszRegPath, 0, KEY_READ, &hKey );
		if (lRc == ERROR_SUCCESS)
		{
			DWORD dwSize = sizeof(DWORD), dwMapping, dwType;
			lRc = RegQueryValueExW( hKey, L"Keyboard Mapping", NULL, &dwType, (PBYTE)&dwMapping, &dwSize );
			if (lRc == ERROR_SUCCESS)
			{
				if ( ( dwVer <= MAKEIMEVERSION( 5, 0 ) && 
					( (BYTE)dwMapping == 0x22 || (BYTE)dwMapping == 0x23 ) )
					||
					( ( dwVer == MAKEIMEVERSION( 5, 1 ) || dwVer == MAKEIMEVERSION( 5, 2 ) ) &&
					(BYTE)dwMapping >= 0x22 && (BYTE)dwMapping <= 0x24 )
					)
				{
					s_bHorizontalReading = true;
				}
			}
			RegCloseKey( hKey );
		}
	}
}


//--------------------------------------------------------------------------------------
// Obtain the reading string upon WM_IME_NOTIFY/INM_PRIVATE notification.
void CGUIIME::GetPrivateReadingString()
{
	CIMELock lock;
	DWORD dwId = GetImeId();
	if( !dwId )
	{
		s_bShowReadingWindow = false;
		return;
	}

	HIMC hImc;
	hImc = _ImmGetContext(  CGlobals::GetAppHWND() );
	if( !hImc )
	{
		s_bShowReadingWindow = false;
		return;
	}

	DWORD dwReadingStrLen = 0;
	DWORD dwErr = 0;
	WCHAR *pwszReadingStringBuffer = NULL;  // Buffer for when the IME supports GetReadingString()
	WCHAR *wstr = 0;
	bool bUnicodeIme = false;  // Whether the IME context component is Unicode.
	INPUTCONTEXT *lpIC = NULL;

	if( _GetReadingString )
	{
		UINT uMaxUiLen;
		BOOL bVertical;
		// Obtain the reading string size
		dwReadingStrLen = _GetReadingString( hImc, 0, NULL, (PINT)&dwErr, &bVertical, &uMaxUiLen );
		if( dwReadingStrLen )
		{
			wstr = pwszReadingStringBuffer = (LPWSTR)HeapAlloc( GetProcessHeap(), 0, sizeof(WCHAR) * dwReadingStrLen );
			if( !pwszReadingStringBuffer )
			{
				// Out of memory. Exit.
				_ImmReleaseContext( CGlobals::GetAppHWND(), hImc );
				return;
			}

			// Obtain the reading string
			dwReadingStrLen = _GetReadingString( hImc, dwReadingStrLen, wstr, (PINT)&dwErr, &bVertical, &uMaxUiLen );
		}

		s_bHorizontalReading = !bVertical;
		bUnicodeIme = true;
	}
	else
	{
		// IMEs that doesn't implement Reading string API

		lpIC = _ImmLockIMC( hImc );

		LPBYTE p = 0;
		switch( dwId )
		{
		case IMEID_CHT_VER42: // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
		case IMEID_CHT_VER43: // New(Phonetic/ChanJie)IME98a : 4.3.x.x // WinMe, Win2k
		case IMEID_CHT_VER44: // New ChanJie IME98b          : 4.4.x.x // WinXP
			p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC( lpIC->hPrivate ) + 24 );
			if( !p ) break;
			dwReadingStrLen = *(DWORD *)( p + 7 * 4 + 32 * 4 );
			dwErr = *(DWORD *)( p + 8 * 4 + 32 * 4 );
			wstr = (WCHAR *)( p + 56 );
			bUnicodeIme = true;
			break;

		case IMEID_CHT_VER50: // 5.0.x.x // WinME
			p = *(LPBYTE *)( (LPBYTE)_ImmLockIMCC( lpIC->hPrivate ) + 3 * 4 );
			if( !p ) break;
			p = *(LPBYTE *)( (LPBYTE)p + 1*4 + 5*4 + 4*2 );
			if( !p ) break;
			dwReadingStrLen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16);
			dwErr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 + 1*4);
			wstr = (WCHAR *)(p + 1*4 + (16*2+2*4) + 5*4);
			bUnicodeIme = false;
			break;

		case IMEID_CHT_VER51: // 5.1.x.x // IME2002(w/OfficeXP)
		case IMEID_CHT_VER52: // 5.2.x.x // (w/whistler)
		case IMEID_CHS_VER53: // 5.3.x.x // SCIME2k or MSPY3 (w/OfficeXP and Whistler)
			p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC( lpIC->hPrivate ) + 4);
			if( !p ) break;
			p = *(LPBYTE *)((LPBYTE)p + 1*4 + 5*4);
			if( !p ) break;
			dwReadingStrLen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * 2);
			dwErr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * 2 + 1*4);
			wstr  = (WCHAR *) (p + 1*4 + (16*2+2*4) + 5*4);
			bUnicodeIme = true;
			break;

			// the code tested only with Win 98 SE (MSPY 1.5/ ver 4.1.0.21)
		case IMEID_CHS_VER41:
			{
				int nOffset;
				nOffset = ( GetImeId( 1 ) >= 0x00000002 ) ? 8 : 7;

				p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC( lpIC->hPrivate ) + nOffset * 4);
				if( !p ) break;
				dwReadingStrLen = *(DWORD *)(p + 7*4 + 16*2*4);
				dwErr = *(DWORD *)(p + 8*4 + 16*2*4);
				dwErr = Math::Min( dwErr, dwReadingStrLen );
				wstr = (WCHAR *)(p + 6*4 + 16*2*1);
				bUnicodeIme = true;
				break;
			}

		case IMEID_CHS_VER42: // 4.2.x.x // SCIME98 or MSPY2 (w/Office2k, Win2k, WinME, etc)
			{
				OSVERSIONINFOW osi;
				osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
				GetVersionExW( &osi );

				int nTcharSize = ( osi.dwPlatformId == VER_PLATFORM_WIN32_NT ) ? sizeof(WCHAR) : sizeof(char);
				p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC( lpIC->hPrivate ) + 1*4 + 1*4 + 6*4);
				if( !p ) break;
				dwReadingStrLen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * nTcharSize);
				dwErr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * nTcharSize + 1*4);
				wstr  = (WCHAR *) (p + 1*4 + (16*2+2*4) + 5*4);
				bUnicodeIme = ( osi.dwPlatformId == VER_PLATFORM_WIN32_NT ) ? true : false;
			}
		}   // switch
	}

	// Copy the reading string to the candidate list first
	s_CandList.awszCandidate[0][0] = 0;
	s_CandList.awszCandidate[1][0] = 0;
	s_CandList.awszCandidate[2][0] = 0;
	s_CandList.awszCandidate[3][0] = 0;
	s_CandList.dwCount = dwReadingStrLen;
	s_CandList.dwSelection = (DWORD)-1; // do not select any char
	if( bUnicodeIme )
	{
		UINT i;
		for( i = 0; i < dwReadingStrLen; ++i ) // dwlen > 0, if known IME
		{
			if( dwErr <= i && s_CandList.dwSelection == (DWORD)-1 )
			{
				// select error char
				s_CandList.dwSelection = i;
			}

			s_CandList.awszCandidate[i][0] = wstr[i];
			s_CandList.awszCandidate[i][1] = 0;
		}
		s_CandList.awszCandidate[i][0] = 0;
	}
	else
	{
		char *p = (char *)wstr;
		DWORD i, j;
		for( i = 0, j = 0; i < dwReadingStrLen; ++i, ++j ) // dwlen > 0, if known IME
		{
			if( dwErr <= i && s_CandList.dwSelection == (DWORD)-1 )
			{
				s_CandList.dwSelection = j;
			}
			// Obtain the current code page
			WCHAR wszCodePage[8];
			UINT uCodePage = DEFAULT_GUI_ENCODING;  // Default code page
			if( GetLocaleInfoW( MAKELCID( GetLanguage(), SORT_DEFAULT ),
				LOCALE_IDEFAULTANSICODEPAGE,
				wszCodePage,
				sizeof(wszCodePage)/sizeof(wszCodePage[0]) ) )
			{
				uCodePage = wcstoul( wszCodePage, NULL, 0 );
			}
			if(MultiByteToWideChar( uCodePage, MB_PRECOMPOSED, p + i, IsDBCSLeadByteEx( uCodePage, p[i] ) ? 2 : 1,
				s_CandList.awszCandidate[j], 1 )==0){
					string strtemp="error: Can't translate \"";
					strtemp.append(p+i,IsDBCSLeadByteEx( uCodePage, p[i] ) ? 2 : 1);
					strtemp+="\" to wide char.";
					OUTPUT_LOG("%s", strtemp.c_str());
				}
			if( IsDBCSLeadByteEx( uCodePage, p[i] ) )
				++i;
		}
		s_CandList.awszCandidate[j][0] = 0;
		s_CandList.dwCount = j;
	}
	if( !_GetReadingString )
	{
		_ImmUnlockIMCC( lpIC->hPrivate );
		_ImmUnlockIMC( hImc );
		GetReadingWindowOrientation( dwId );
	}
	_ImmReleaseContext(  CGlobals::GetAppHWND(), hImc );

	if( pwszReadingStringBuffer )
		HeapFree( GetProcessHeap(), 0, pwszReadingStringBuffer );

	// Copy the string to the reading string buffer
	if( s_CandList.dwCount > 0 )
		s_bShowReadingWindow = true;
	else
		s_bShowReadingWindow = false;
	if( s_bHorizontalReading )
	{
		s_CandList.nReadingError = -1;
		s_wszReadingString[0] = 0;
		for( UINT i = 0; i < s_CandList.dwCount; ++i )
		{
			if( s_CandList.dwSelection == i )
				s_CandList.nReadingError = lstrlenW( s_wszReadingString );
			StringCchCatW( s_wszReadingString, 32, s_CandList.awszCandidate[i] );
		}
	}

	s_CandList.dwPageSize = MAX_CANDLIST;
}

bool CGUIIME::OnFocusIn()
{
	s_bIMEHasFocus = true;
	return CGlobals::GetApp()->PostWinThreadMessage(PE_IME_SETFOCUS, 0, 1);
}
bool CGUIIME::OnFocusIn_imp()
{
	StaticOnCreateDevice();
	CIMELock lock;
	if( s_bEnableImeSystem )
	{
		HIMC hImcLast = _ImmAssociateContext(  CGlobals::GetAppHWND(), s_hImcDef );
		CheckToggleState();
	} 
	else
	{
		// because we want to use the default windows ime. 
		_ImmAssociateContext(  CGlobals::GetAppHWND(), s_hImcDef );
	}

	//
	// Set up the IME global state according to the current instance state
	//
	HIMC hImc;
	if( NULL != ( hImc = _ImmGetContext(  CGlobals::GetAppHWND() ) ) ) 
	{
		if( !s_bEnableImeSystem )
			s_ImeState = IMEUI_STATE_OFF;

		_ImmReleaseContext(  CGlobals::GetAppHWND(), hImc );
		CheckToggleState();
	}
	return true;
}

bool CGUIIME::OnFocusOut()
{
	s_bIMEHasFocus = false;
	return CGlobals::GetApp()->PostWinThreadMessage(PE_IME_SETFOCUS, 0, 0);
}

bool CGUIIME::OnFocusOut_imp()
{
	StaticOnCreateDevice();
	_ImmAssociateContext(  CGlobals::GetAppHWND(), NULL );
	s_ImeState = IMEUI_STATE_OFF;
	return true;
}

bool CGUIIME::SwitchIn() 
{
	CIMELock lock;
	// Populate s_Locale with the list of keyboard layouts.
	UINT cKL = GetKeyboardLayoutList( 0, NULL );
	s_Locale.RemoveAll();
	HKL *phKL = new HKL[cKL];
	if( phKL )
	{
		GetKeyboardLayoutList( cKL, phKL );
		for( UINT i = 0; i < cKL; ++i )
		{
			CInputLocale Locale;

			// Filter out East Asian languages that are not IME.
			if( ( PRIMARYLANGID( LOWORD( phKL[i] ) ) == LANG_CHINESE ||
				PRIMARYLANGID( LOWORD( phKL[i] ) ) == LANG_JAPANESE ||
				PRIMARYLANGID( LOWORD( phKL[i] ) ) == LANG_KOREAN ) &&
				!_ImmIsIME( phKL[i] ) )
				continue;

			// If this language is already in the list, don't add it again.
			bool bBreak = false;
			for( int e = 0; e < s_Locale.GetSize(); ++e )
				if( LOWORD( s_Locale.GetAt( e ).m_hKL ) ==
					LOWORD( phKL[i] ) )
				{
					bBreak = true;
					break;
				}
				if( bBreak )
					break;

				Locale.m_hKL = phKL[i];
				WCHAR wszDesc[128] = L"";
				switch( PRIMARYLANGID( LOWORD( phKL[i] ) ) )
				{
					// Simplified Chinese
				case LANG_CHINESE:
					switch( SUBLANGID( LOWORD( phKL[i] ) ) )
					{
					case SUBLANG_CHINESE_SIMPLIFIED:
						StringCchCopyW( Locale.m_wszLangAbb, 3, s_aszIndicator[INDICATOR_CHS] );
						break;
					case SUBLANG_CHINESE_TRADITIONAL:
						StringCchCopyW( Locale.m_wszLangAbb, 3, s_aszIndicator[INDICATOR_CHT] );
						break;
					default:    // unsupported sub-language
						GetLocaleInfoW( MAKELCID( LOWORD( phKL[i] ), SORT_DEFAULT ), LOCALE_SABBREVLANGNAME, wszDesc, 128 );
						Locale.m_wszLangAbb[0] = wszDesc[0];
						Locale.m_wszLangAbb[1] = towlower( wszDesc[1] );
						Locale.m_wszLangAbb[2] = L'\0';
						break;
					}
					break;
					// Korean
				case LANG_KOREAN:
					StringCchCopyW( Locale.m_wszLangAbb, 3, s_aszIndicator[INDICATOR_KOREAN] );
					break;
					// Japanese
				case LANG_JAPANESE:
					StringCchCopyW( Locale.m_wszLangAbb, 3, s_aszIndicator[INDICATOR_JAPANESE] );
					break;         
				default:
					// A non-IME language.  Obtain the language abbreviation
					// and store it for rendering the indicator later.
					GetLocaleInfoW( MAKELCID( LOWORD( phKL[i] ), SORT_DEFAULT ), LOCALE_SABBREVLANGNAME, wszDesc, 128 );
					Locale.m_wszLangAbb[0] = wszDesc[0];
					Locale.m_wszLangAbb[1] = towlower( wszDesc[1] );
					Locale.m_wszLangAbb[2] = L'\0';
					break;
				}

				GetLocaleInfoW( MAKELCID( LOWORD( phKL[i] ), SORT_DEFAULT ), LOCALE_SLANGUAGE, wszDesc, 128 );
				StringCchCopyW( Locale.m_wszLang, 64, wszDesc );

				s_Locale.Add( Locale );
		}
		delete[] phKL;
	}
	return true;
}


bool CGUIIME::SwitchOut() 
{
	_ImmAssociateContext(  CGlobals::GetAppHWND(), NULL );
	return true;
}

bool CGUIIME::GetIMEOpenStatus()
{
	HIMC hImc = _ImmGetContext(  CGlobals::GetAppHWND() );
	return _ImmGetOpenStatus(hImc) ? true : false;
}

void CGUIIME::SetIMEOpenStatus(bool bOpen, bool bForce)
{
	CIMELock lock;
	s_sLastCompString.clear();
	if( bForce || (!bOpen && s_bIMETrap) )
	{
		CGlobals::GetApp()->PostWinThreadMessage(PE_IME_SETOPENSTATUS, 0, bOpen ? 1 : 0);
	}
}
void CGUIIME::SetIMEOpenStatus_imp(bool bOpen)
{
	StaticOnCreateDevice();
	CIMELock lock;
	if( s_bEnableImeSystem )
	{
		// _ImmAssociateContext(  CGlobals::GetAppHWND(), s_hImcDef );
		//HIMC hImc = _ImmGetContext(  CGlobals::GetAppHWND() );
		//_ImmSetOpenStatus(hImc, bOpen ? TRUE : FALSE);
		_ImmSetOpenStatus(s_hImcDef, bOpen ? TRUE : FALSE);
		// TODO: the following does not take effect. So when focus back, the indicator may be wrong on first use. 
		//CheckToggleState();
	}
}

//--------------------------------------------------------------------------------------
bool CGUIIME::StaticMsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HIMC hImc;

	if( !s_bEnableImeSystem )
	{
		if(uMsg == WM_IME_STARTCOMPOSITION )
		{
			HWND hwndImeDef = _ImmGetDefaultIMEWnd(CGlobals::GetAppHWND());
			if ( hwndImeDef )
			{
				// adjusts the composition window position
				COMPOSITIONFORM compostionForm;
				compostionForm.dwStyle = CFS_FORCE_POSITION;
				{
					CIMELock lock;
					compostionForm.ptCurrentPos = m_nCompositionWindowPoint;
				}
				SendMessageW(hwndImeDef, WM_IME_CONTROL, IMC_SETCOMPOSITIONWINDOW, (LPARAM)(&compostionForm));	
			}	
		}
		return false;
	}


#if defined(DEBUG) | defined(_DEBUG)
	m_bIMEStaticMsgProcCalled = true;
#endif

	switch( uMsg )
	{
	case WM_ACTIVATEAPP:
		if ( wParam ) 
		{
			CIMELock lock;
			SwitchIn();
		}
		break;

	case WM_INPUTLANGCHANGE:
		{
			CIMELock lock;
			//if(CGUIRoot::Instance()->GetUIKeyFocus() == 0)
			//{
			//	// prevent IME open when there is no IME editbox selected. 
			//	// CGUIIME::SwitchOut();
			//	SetIMEOpenStatus(false);
			//	return true;
			//}
			UINT uLang = GetPrimaryLanguage();
			CheckToggleState();
			if ( uLang != GetPrimaryLanguage() )
			{
				// Korean IME always inserts on keystroke.  Other IMEs do not.
				s_bInsertOnType = ( GetPrimaryLanguage() == LANG_KOREAN );
			}

			// IME changed.  Setup the new IME.
			SetupImeApi();
			if( _ShowReadingWindow )
			{
				if ( NULL != ( hImc = _ImmGetContext(  CGlobals::GetAppHWND() ) ) )
				{
					_ShowReadingWindow( hImc, false );
					_ImmReleaseContext(  CGlobals::GetAppHWND(), hImc );
				}
			}
		}
		return true;

	case WM_IME_SETCONTEXT:
		//
		// We don't want anything to display, so we have to clear this
		//
		lParam = 0;
		return true;

		// Handle WM_IME_STARTCOMPOSITION here since
		// we do not want the default IME handler to see
		// this when our fullscreen app is running.
	case WM_IME_ENDCOMPOSITION:
		{
			CIMELock lock;
			s_bIMETrap = false;
			return true;
		}
	case WM_IME_STARTCOMPOSITION:
		{
			CIMELock lock;
			// OUTPUT_LOG( "WM_IME_STARTCOMPOSITION\n" );
			s_bIMETrap =  true;
			ResetCompositionString();
			// Since the composition string has its own caret, we don't render
			// the edit control's own caret to avoid double carets on screen.
			if(s_bIMEHasFocus)
				s_bHideCaret = true;
			return true;
		}
	case WM_IME_COMPOSITION:
		{
			//DXUTTRACE( L"WM_IME_COMPOSITION\n" );
			return true;
		}
	}
	return false;
}


//--------------------------------------------------------------------------------------
bool CGUIIME::WinMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if defined(DEBUG) | defined(_DEBUG)
	// DXUT.cpp used to call CGUIIME::StaticMsgProc() so that, but now
	// this is the application's responsibility.  To do this, call 
	// CDXUTDialogResourceManager::MsgProc() before calling this function.
	//PE_ASSERT( m_bIMEStaticMsgProcCalled  );
#endif

	bool trappedData;
	bool *trapped = &trappedData;

	HIMC hImc;
	static LPARAM lAlt = 0x80000000, lCtrl = 0x80000000, lShift = 0x80000000;

	*trapped = false;
	if( !s_bEnableImeSystem )
		return false;
	switch( uMsg )
	{
	//
	//  IME Handling
	//
	case WM_IME_COMPOSITION:
	{
		CIMELock lock;
		//OUTPUT_LOG( "WM_IME_COMPOSITION\n" ); // remove this for debugging

		LONG lRet;  // Returned count in CHARACTERS
		WCHAR wszCompStr[MAX_COMPSTRING_SIZE];

		*trapped = true;
		if( NULL == ( hImc = _ImmGetContext(  CGlobals::GetAppHWND() ) ) )
		{
			break;
		}

		// Get the caret position in composition string
		if ( lParam & GCS_CURSORPOS )
		{
			s_nCompCaret = _ImmGetCompositionStringW( hImc, GCS_CURSORPOS, NULL, 0 );
			if( s_nCompCaret < 0 )
				s_nCompCaret = 0; // On error, set caret to pos 0.
		}

		// ResultStr must be processed before composition string.
		//
		// This is because for some IMEs, such as CHT, pressing Enter
		// to complete the composition sends WM_IME_COMPOSITION with both
		// GCS_RESULTSTR and GCS_COMPSTR.  Retrieving the result string
		// gives the correct string, while retrieving the comp string
		// (GCS_COMPSTR) gives empty string.  GCS_RESULTSTR should be
		// handled first so that the application receives the string.  Then
		// GCS_COMPSTR can be handled to clear the comp string buffer.

		if ( lParam & GCS_RESULTSTR )
		{
			//DXUTTRACE( L"  GCS_RESULTSTR\n" );
			lRet = _ImmGetCompositionStringW( hImc, GCS_RESULTSTR, wszCompStr, sizeof( wszCompStr ) );
			if( lRet > 0 )
			{
				lRet /= sizeof(WCHAR);
				wszCompStr[lRet] = 0;  // Force terminate
				TruncateCompString( false, (int)wcslen( wszCompStr ) );
				s_CompString.SetText((const char16_t*)wszCompStr);
				SendCompString();
				ResetCompositionString();
			}
		}

		//
		// Reads in the composition string.
		//
		if ( lParam & GCS_COMPSTR )
		{
			//DXUTTRACE( L"  GCS_COMPSTR\n" );
			//////////////////////////////////////////////////////
			// Retrieve the latest user-selected IME candidates
			lRet = _ImmGetCompositionStringW( hImc, GCS_COMPSTR, wszCompStr, sizeof( wszCompStr ) );
			if( lRet > 0 )
			{
				lRet /= sizeof(WCHAR);  // Convert size in byte to size in char
				wszCompStr[lRet] = 0;  // Force terminate
				//
				// Remove the whole of the string
				//
				TruncateCompString( false, (int)wcslen( wszCompStr ) );

				s_CompString.SetText((const char16_t*)wszCompStr);

				// Older CHT IME uses composition string for reading string
				if ( GetLanguage() == LANG_CHT && !GetImeId() )
				{
					if (lstrlenW((const WCHAR*)s_CompString.GetBuffer()))
					{
						s_CandList.dwCount = 4;             // Maximum possible length for reading string is 4
						s_CandList.dwSelection = (DWORD)-1; // don't select any candidate

						// Copy the reading string to the candidate list
						for( int i = 3; i >= 0; --i )
						{
							if (i > lstrlenW((const WCHAR*)s_CompString.GetBuffer()) - 1)
								s_CandList.awszCandidate[i][0] = 0;  // Doesn't exist
							else
							{
								s_CandList.awszCandidate[i][0] = s_CompString[i];
								s_CandList.awszCandidate[i][1] = 0;
							}
						}
						s_CandList.dwPageSize = MAX_CANDLIST;
						// Clear comp string after we are done copying
						ZeroMemory( (LPVOID)s_CompString.GetBuffer(), 4 * sizeof(WCHAR) );
						s_bShowReadingWindow = true;
						GetReadingWindowOrientation( 0 );
						if( s_bHorizontalReading )
						{
							s_CandList.nReadingError = -1;  // Clear error

							// Create a string that consists of the current
							// reading string.  Since horizontal reading window
							// is used, we take advantage of this by rendering
							// one string instead of several.
							//
							// Copy the reading string from the candidate list
							// to the reading string buffer.
							s_wszReadingString[0] = 0;
							for( UINT i = 0; i < s_CandList.dwCount; ++i )
							{
								if( s_CandList.dwSelection == i )
									s_CandList.nReadingError = lstrlenW( s_wszReadingString );
								StringCchCatW( s_wszReadingString, 32, s_CandList.awszCandidate[i] );
							}
						}
					}
					else
					{
						s_CandList.dwCount = 0;
						s_bShowReadingWindow = false;
					}
				}

				if( s_bInsertOnType )
				{
					// Send composition string to the edit control
					SendCompString();
					// Restore the caret to the correct location.
					// It's at the end right now, so compute the number
					// of times left arrow should be pressed to
					// send it to the original position.
					int nCount = lstrlenW((const WCHAR*)s_CompString.GetBuffer() + s_nCompCaret);
					// Send left keystrokes
//						for( int i = 0; i < nCount; ++i )
//							SendMessage( CGlobals::GetAppHWND(), WM_KEYDOWN, VK_LEFT, 0 );
//						SendMessage( CGlobals::GetAppHWND(), WM_KEYUP, VK_LEFT, 0 );
				}
			}

			ResetCaretBlink();
		}

		// Retrieve comp string attributes
		if( lParam & GCS_COMPATTR )
		{
			lRet = _ImmGetCompositionStringW( hImc, GCS_COMPATTR, s_abCompStringAttr, sizeof( s_abCompStringAttr ) );
			if( lRet > 0 )
				s_abCompStringAttr[lRet] = 0;  // ??? Is this needed for attributes?
		}

		// Retrieve clause information
		if( lParam & GCS_COMPCLAUSE )
		{
			/** LXZ 2009.11.13.  this fixed a bug, when entering some text in sogou, switch to ABC, and enter some text, application will crash. */
			if(GetPrimaryLanguage() == LANG_JAPANESE)
			{
				lRet = _ImmGetCompositionStringW(hImc, GCS_COMPCLAUSE, s_adwCompStringClause, sizeof( s_adwCompStringClause ) );
				s_adwCompStringClause[lRet / sizeof(DWORD)] = 0;  // Terminate
			}
		}

		_ImmReleaseContext(  CGlobals::GetAppHWND(), hImc );
		break;
	}
	case WM_IME_STARTCOMPOSITION:
		*trapped=true;
		break;
	case WM_IME_ENDCOMPOSITION:
		{
			CIMELock lock;
			//OUTPUT_LOG("WM_IME_ENDCOMPOSITION\n"); // remove this for debugging

			TruncateCompString();
			ResetCompositionString();
			// We can show the edit control's caret again.
			s_bHideCaret = false;
			// Hide reading window
			s_bShowReadingWindow = false;
			// send on key up with empty key state. 
			/*int nOld = m_event->m_keyboard.nAlterKey;
			m_event->m_keyboard.nAlterKey = 0;
			CGUIBase::OnKeyUp();
			m_event->m_keyboard.nAlterKey = nOld;*/
			//1
			*trapped=true;
			break;
		}
	case WM_IME_NOTIFY:
		{
			CIMELock lock;
		
			//OUTPUT_LOG( "WM_IME_NOTIFY %u\n", wParam ); // remove this for debugging
			switch( wParam )
			{
			case IMN_SETCONVERSIONMODE:
				//OUTPUT_LOG( "  IMN_SETCONVERSIONMODE\n" );
			case IMN_SETOPENSTATUS:
				//OUTPUT_LOG( "  IMN_SETOPENSTATUS\n" );
				CheckToggleState();
				break;

			case IMN_OPENCANDIDATE:
			case IMN_CHANGECANDIDATE:
				{
					//DXUTTRACE( wParam == IMN_CHANGECANDIDATE ? L"  IMN_CHANGECANDIDATE\n" : L"  IMN_OPENCANDIDATE\n" );

					s_CandList.bShowWindow = true;
					*trapped = true;
					if( NULL == ( hImc = _ImmGetContext(  CGlobals::GetAppHWND() ) ) )
						break;

					LPCANDIDATELIST lpCandList = NULL;
					DWORD dwLenRequired;

					s_bShowReadingWindow = false;
					// Retrieve the candidate list
					dwLenRequired = _ImmGetCandidateListW( hImc, 0, NULL, 0 );
					if( dwLenRequired )
					{
						lpCandList = (LPCANDIDATELIST)HeapAlloc( GetProcessHeap(), 0, dwLenRequired );
						dwLenRequired = _ImmGetCandidateListW( hImc, 0, lpCandList, dwLenRequired );
					}

					if( lpCandList )
					{
						// Update candidate list data
						s_CandList.dwSelection = lpCandList->dwSelection;
						s_CandList.dwCount = lpCandList->dwCount;

						int nPageTopIndex = 0;
						s_CandList.dwPageSize = Math::Min((int)lpCandList->dwPageSize, (int)MAX_CANDLIST);
						if( GetPrimaryLanguage() == LANG_JAPANESE )
						{
							// Japanese IME organizes its candidate list a little
							// differently from the other IMEs.
							nPageTopIndex = ( s_CandList.dwSelection / s_CandList.dwPageSize ) * s_CandList.dwPageSize;
						}
						else
							nPageTopIndex = lpCandList->dwPageStart;

						// Make selection index relative to first entry of page
						s_CandList.dwSelection = ( GetLanguage() == LANG_CHS && !GetImeId() ) ? (DWORD)-1
							: s_CandList.dwSelection - nPageTopIndex;

						ZeroMemory( s_CandList.awszCandidate, sizeof(s_CandList.awszCandidate) );
						for( UINT i = nPageTopIndex, j = 0;
							(DWORD)i < lpCandList->dwCount && j < s_CandList.dwPageSize;
							i++, j++ )
						{
							// Initialize the candidate list strings
							LPWSTR pwsz = s_CandList.awszCandidate[j];
							// For every candidate string entry,
							// write [index] + Space + [string] if vertical,
							// write [index] + [string] + Space if horizontal.
							*pwsz++ = (WCHAR)( L'0' + ( (j + 1) % 10 ) );  // Index displayed is 1 based
							if( s_bVerticalCand )
								*pwsz++ = L' ';
							WCHAR *pwszNewCand = (LPWSTR)( (LPBYTE)lpCandList + lpCandList->dwOffset[i] );
							while ( *pwszNewCand )
								*pwsz++ = *pwszNewCand++;
							if( !s_bVerticalCand )
								*pwsz++ = L' ';
							*pwsz = 0;  // Terminate
						}

						// Make dwCount in s_CandList be number of valid entries in the page.
						s_CandList.dwCount = lpCandList->dwCount - lpCandList->dwPageStart;
						if( s_CandList.dwCount > lpCandList->dwPageSize )
							s_CandList.dwCount = lpCandList->dwPageSize;

						HeapFree( GetProcessHeap(), 0, lpCandList );
						_ImmReleaseContext(  CGlobals::GetAppHWND(), hImc );

						// Korean and old Chinese IME can't have selection.
						// User must use the number hotkey or Enter to select
						// a candidate.
						if( GetPrimaryLanguage() == LANG_KOREAN ||
							GetLanguage() == LANG_CHT && !GetImeId() )
						{
							s_CandList.dwSelection = (DWORD)-1;
						}

						// Initialize s_CandList.HoriCand if we have a
						// horizontal candidate window.
						if( !s_bVerticalCand )
						{
							WCHAR wszCand[256] = L"";

							s_CandList.nFirstSelected = 0;
							s_CandList.nHoriSelectedLen = 0;
							for( UINT i = 0; i < MAX_CANDLIST; ++i )
							{
								if( s_CandList.awszCandidate[i][0] == L'\0' )
									break;

								WCHAR wszEntry[32];
								StringCchPrintfW( wszEntry, 32, L"%s ", s_CandList.awszCandidate[i] );
								// If this is the selected entry, mark its char position.
								if( s_CandList.dwSelection == i )
								{
									s_CandList.nFirstSelected = lstrlenW( wszCand );
									s_CandList.nHoriSelectedLen = lstrlenW( wszEntry ) - 1;  // Minus space
								}
								StringCchCatW( wszCand, 256, wszEntry );
							}
							wszCand[lstrlenW(wszCand) - 1] = L'\0';  // Remove the last space
							s_CandList.HoriCand.SetText((const char16_t*)wszCand);
						}
					}
					break;
				}

			case IMN_CLOSECANDIDATE:
				{
					//DXUTTRACE( L"  IMN_CLOSECANDIDATE\n" );
					s_CandList.bShowWindow = false;
					if( !s_bShowReadingWindow )
					{
						s_CandList.dwCount = 0;
						ZeroMemory( s_CandList.awszCandidate, sizeof(s_CandList.awszCandidate) );
					}
					*trapped = true;
					break;
				}

			case IMN_PRIVATE:
				//DXUTTRACE( L"  IMN_PRIVATE\n" );
				{
					if( !s_CandList.bShowWindow )
						GetPrivateReadingString();

					// Trap some messages to hide reading window
					DWORD dwId = GetImeId();
					switch( dwId )
					{
					case IMEID_CHT_VER42:
					case IMEID_CHT_VER43:
					case IMEID_CHT_VER44:
					case IMEID_CHS_VER41:
					case IMEID_CHS_VER42:
						if( ( lParam == 1 ) || ( lParam == 2 ) )
						{
							*trapped = true;
						}
						break;

					case IMEID_CHT_VER50:
					case IMEID_CHT_VER51:
					case IMEID_CHT_VER52:
					case IMEID_CHT_VER60:
					case IMEID_CHS_VER53:
						if( (lParam == 16) || (lParam == 17) || (lParam == 26) || (lParam == 27) || (lParam == 28) )
						{
							*trapped = true;
						}
						break;
					}
				}
				break;

			default:
				*trapped = true;
				break;
			}
			//1
			*trapped=true;
			break;
		}
	case WM_KEYUP:
		{
			if( wParam == VK_SHIFT)
			{
				// this fixed a bug, when the user presses shift key to toggle the conversion status in Chinese language IME, the conversion status indicator is not updated. 
				CheckToggleState();
			}
			break;
		}
/*
		// When Text Service Framework is installed in Win2K, Alt+Shift and Ctrl+Shift combination (to switch input
		// locale / keyboard layout) doesn't send WM_KEYUP message for the key that is released first. We need to check
		// if these keys are actually up whenever we receive key up message for other keys.
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if ( !( lAlt & 0x80000000 ) && wParam != VK_MENU && ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) == 0 )
		{
			PostMessageW( GetFocus(), WM_KEYUP, (WPARAM)VK_MENU, ( lAlt & 0x01ff0000 ) | 0xC0000001 );
		}   
		else if ( !( lCtrl & 0x80000000 ) && wParam != VK_CONTROL && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) == 0 )
		{
			PostMessageW( GetFocus(), WM_KEYUP, (WPARAM)VK_CONTROL, ( lCtrl & 0x01ff0000 ) | 0xC0000001 );
		}
		else if ( !( lShift & 0x80000000 ) && wParam != VK_SHIFT && ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
		{
			PostMessageW( GetFocus(), WM_KEYUP, (WPARAM)VK_SHIFT, ( lShift & 0x01ff0000 ) | 0xC0000001 );
		}
		// fall through WM_KEYDOWN / WM_SYSKEYDOWN
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		switch ( wParam )
		{
		case VK_MENU:
			lAlt = lParam;
			break;
		case VK_SHIFT:
			lShift = lParam;
			break;
		case VK_CONTROL:
			lCtrl = lParam;
			break;
		}
		//break;
		// Fall through to default case
		// so we invoke the parent.
*/
	default:
		// Let the parent handle the message that we
		// don't handle.
		return false;//CDXUTEditBox::MsgProc( uMsg, wParam, lParam );

	}  // switch
	
	return *trapped;
}

void CGUIIME::ResetCaretBlink()
{
	m_bCaretOn = true;
	m_dfLastBlink = (GetTickCount()/1000.f);
}

// This function should be called in the window thread, since IME context is a shared per thread resource. 
HRESULT CGUIIME::StaticOnCreateDevice()
{
	if(!s_is_ime_created)
	{
		CIMELock lock;

		s_is_ime_created = true;
		Initialize();

		if(s_hImcDef == 0)
		{
			// Save the default input context
			if(s_bCreateIMEContext)
			{
				s_hImcDef = _ImmCreateContext();
				_ImmAssociateContext(  CGlobals::GetAppHWND(), s_hImcDef);
			}
			else
			{
				// use the default IME context created by windows
				s_hImcDef = _ImmGetContext( CGlobals::GetAppHWND() );
				_ImmReleaseContext( CGlobals::GetAppHWND(), s_hImcDef );
			}

			// By default we will not associate IME context with the window, unless an IME editbox is focused. 
			_ImmAssociateContext(  CGlobals::GetAppHWND(), NULL );
		}
	}
	return S_OK;
}

void CGUIIME::Initialize()
{
	if( s_hDllImm32 ) // Only need to do once
		return;

	FARPROC Temp;

	s_CompString.SetBufferSize( MAX_COMPSTRING_SIZE );

	CHAR wszPath[MAX_PATH+1];
	if( !::GetSystemDirectory( wszPath, MAX_PATH+1 ) )
		return;
	StringCchCat( wszPath, MAX_PATH, IMM32_DLLNAME );
	s_hDllImm32 = LoadLibrary( wszPath );
	if( s_hDllImm32 )
	{
		GETPROCADDRESS( s_hDllImm32, ImmLockIMC, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmUnlockIMC, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmLockIMCC, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmUnlockIMCC, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmDisableTextFrameService, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmGetCompositionStringW, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmGetCandidateListW, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmCreateContext, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmDestroyContext, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmGetContext, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmReleaseContext, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmAssociateContext, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmGetOpenStatus, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmSetOpenStatus, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmGetConversionStatus, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmGetDefaultIMEWnd, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmGetIMEFileNameA, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmGetVirtualKey, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmNotifyIME, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmSetConversionStatus, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmSimulateHotKey, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmIsIME, Temp );
		GETPROCADDRESS( s_hDllImm32, ImmSetCandidateWindow, Temp );
	}

	if( !::GetSystemDirectory( wszPath, MAX_PATH+1 ) )
		return;
	StringCchCat( wszPath, MAX_PATH, VER_DLLNAME );
	s_hDllVer = LoadLibrary( wszPath );
	if( s_hDllVer )
	{
		GETPROCADDRESS( s_hDllVer, VerQueryValueA, Temp );
		GETPROCADDRESS( s_hDllVer, GetFileVersionInfoA, Temp );
		GETPROCADDRESS( s_hDllVer, GetFileVersionInfoSizeA, Temp );
	}
}


//--------------------------------------------------------------------------------------
void CGUIIME::Uninitialize()
{
	if(s_bCreateIMEContext && s_hImcDef)
	{
		_ImmDestroyContext(s_hImcDef);
		s_hImcDef = NULL;
	}

	if( s_hDllImm32 )
	{
		PLACEHOLDERPROC( ImmLockIMC );
		PLACEHOLDERPROC( ImmUnlockIMC );
		PLACEHOLDERPROC( ImmLockIMCC );
		PLACEHOLDERPROC( ImmUnlockIMCC );
		PLACEHOLDERPROC( ImmDisableTextFrameService );
		PLACEHOLDERPROC( ImmGetCompositionStringW );
		PLACEHOLDERPROC( ImmGetCandidateListW );

		PLACEHOLDERPROC( ImmCreateContext );
		PLACEHOLDERPROC( ImmDestroyContext );

		PLACEHOLDERPROC( ImmGetContext );
		PLACEHOLDERPROC( ImmReleaseContext );
		PLACEHOLDERPROC( ImmAssociateContext );
		PLACEHOLDERPROC( ImmGetOpenStatus );
		PLACEHOLDERPROC( ImmSetOpenStatus );
		PLACEHOLDERPROC( ImmGetConversionStatus );
		PLACEHOLDERPROC( ImmGetDefaultIMEWnd );
		PLACEHOLDERPROC( ImmGetIMEFileNameA );
		PLACEHOLDERPROC( ImmGetVirtualKey );
		PLACEHOLDERPROC( ImmNotifyIME );
		PLACEHOLDERPROC( ImmSetConversionStatus );
		PLACEHOLDERPROC( ImmSimulateHotKey );
		PLACEHOLDERPROC( ImmIsIME );
		PLACEHOLDERPROC( ImmSetCandidateWindow );

		FreeLibrary( s_hDllImm32 );
		s_hDllImm32 = NULL;
	}
	if( s_hDllIme )
	{
		PLACEHOLDERPROC( GetReadingString );
		PLACEHOLDERPROC( ShowReadingWindow );

		FreeLibrary( s_hDllIme );
		s_hDllIme = NULL;
	}
	if( s_hDllVer )
	{
		PLACEHOLDERPROC( VerQueryValueA );
		PLACEHOLDERPROC( GetFileVersionInfoA );
		PLACEHOLDERPROC( GetFileVersionInfoSizeA );

		FreeLibrary( s_hDllVer );
		s_hDllVer = NULL;
	}
}

bool CGUIIME::IMEHasFocus()
{
	return s_bIMEHasFocus;
}

bool CGUIIME::HandleWinThreadMsg( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	StaticOnCreateDevice();
	
	bool bRes = StaticMsgProc(uMsg,wParam,lParam);
	WinMsgProc(uMsg,wParam,lParam);
	return bRes;
}

bool CGUIIME::GetShowImeReadingWindow()
{
	CIMELock lock;
	return s_bShowReadingWindow;
}

bool CGUIIME::GetShowImeCandidateWindow()
{
	CIMELock lock;
	return s_CandList.bShowWindow;
}

bool CGUIIME::GetIsVerticalImeCandidate()
{
	CIMELock lock;
	return !s_bHorizontalReading;
}

bool CGUIIME::GetIsHorizontalImeReading()
{
	CIMELock lock;
	return s_bHorizontalReading;
}

void CGUIIME::SetImeState( IMESTATE aState )
{
	SetIMEOpenStatus(aState==IMEUI_STATE_ON);
}

ParaEngine::IMESTATE CGUIIME::GetImeState()
{
	CIMELock lock;
	return s_ImeState;
}

ParaEngine::IMESTATE CGUIIME::GetImeStateS()
{
	CIMELock lock;
	return s_ImeState;
}

ParaEngine::IMELanguage CGUIIME::GetImeLanguage()
{
	CIMELock lock;
	IMELanguage lang = IMELanguage_English;
	switch ( GetPrimaryLanguage() )
	{
		// Simplified Chinese
	case LANG_CHINESE:
		switch ( GetSubLanguage() )
		{
		case SUBLANG_CHINESE_SIMPLIFIED:
			lang = IMELanguage_ChineseSimplified;
			break;
		case SUBLANG_CHINESE_TRADITIONAL:
			lang = IMELanguage_ChineseTraditional;
			break;
		default:    // unsupported sub-language
			lang = IMELanguage_Unknown;
			break;
		}
		break;
		// Korean
	case LANG_KOREAN:
		lang = IMELanguage_Korean;
		break;
		// Japanese
	case LANG_JAPANESE:
		lang = IMELanguage_Japanese;
		break;
	default:
		break;
	};
	return lang;
}

const char16_t* CGUIIME::GetImeIndicator()
{
	CIMELock lock;
	return (const char16_t*)s_wszCurrIndicator;
}

int CGUIIME::GetNumImeCandidates()
{
	CIMELock lock;
	return s_CandList.dwCount;
}

int CGUIIME::GetSelectedImeCandidate()
{
	CIMELock lock;
	return s_CandList.dwSelection;
}

const char16_t* CGUIIME::GetImeCandidate( int nIndex )
{
	static std::wstring g_candidate;
	g_candidate = s_CandList.awszCandidate[nIndex];
	return (const char16_t*)g_candidate.c_str();
}

void CGUIIME::ImeFinalizeString( bool bSend )
{
	// s_CompString
}

const char16_t* CGUIIME::GetImeString()
{
	return NULL;
}

int CGUIIME::GetImeCursorPosition()
{
	return 0;
}
#endif