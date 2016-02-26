#pragma once
#ifdef USE_DIRECTX_RENDERER
#include "util/mutex.h"
#include "GrowableArray.h"
#include "GUIUniBuffer.h"

namespace ParaEngine
{
	enum IMESTATE
	{
		IMEUI_STATE_OFF,
		IMEUI_STATE_ON,
		IMEUI_STATE_ENGLISH,
	};

	enum IMELanguage
	{
		IMELanguage_Unknown,
		IMELanguage_English,
		IMELanguage_ChineseSimplified,
		IMELanguage_ChineseTraditional,
		IMELanguage_Korean,
		IMELanguage_Japanese,
	};

	/** IME system interface
	*/
	class IIMESystem
	{
	public:
		/// Get whether the reading window should be shown.
		virtual bool GetShowImeReadingWindow() = 0;
		/// Get whether the candidate window should be shown.
		virtual bool GetShowImeCandidateWindow() = 0;
		/// Get whether the it is a vertical candidate.
		virtual bool GetIsVerticalImeCandidate() = 0;
		/// Get whether using horizontal reading.
		virtual bool GetIsHorizontalImeReading() = 0;

		/// Set the current IME state.
		virtual void SetImeState(IMESTATE aState) = 0;
		/// Get the current IME state.
		virtual IMESTATE GetImeState() = 0;
		/// Get the current IME language.
		virtual IMELanguage GetImeLanguage() = 0;
		/// Get the IME indicator string.
		virtual const char16_t* GetImeIndicator() = 0;
		/// Get the number of IME candidates.
		virtual int GetNumImeCandidates() = 0;
		/// Get the IME candidate selected.
		virtual int GetSelectedImeCandidate() = 0;
		/// Get the IME candidate at the specified index.
		virtual const char16_t* GetImeCandidate(int anIndex) = 0;

		/// Finalize the Ime String.
		/// @param bSend whether to send the composition string to IME window.
		virtual void ImeFinalizeString(bool bSend) = 0;
		/// Get the current IME composition string.
		virtual const char16_t* GetImeString() = 0;
		/// Get the IME cursor character index in the current composition string.
		virtual int GetImeCursorPosition() = 0;
	};

	/** lock before using IME object. */
	class CIMELock : ParaEngine::mutex::ScopedLock
	{
	public:
		CIMELock();
	};

#define MAX_CANDLIST 10
#define MAX_COMPSTRING_SIZE 256
	
	/** GUI IME message system. 
	* all functions are static
	* it should be run in the focus window thread. However the GUI system may be in a different (main) thread. 
	*/
	class CGUIIME : public IIMESystem
	{
	public:
		CGUIIME(){};
		
	public:
		/// Get whether the reading window should be shown.
		virtual bool GetShowImeReadingWindow();
		/// Get whether the candidate window should be shown.
		virtual bool GetShowImeCandidateWindow();
		/// Get whether the it is a vertical candidate.
		virtual bool GetIsVerticalImeCandidate();
		/// Get whether using horizontal reading.
		virtual bool GetIsHorizontalImeReading();

		/// Set the current IME state.
		virtual void SetImeState(IMESTATE aState);
		/// Get the current IME state.
		virtual IMESTATE GetImeState();
		/// Get the current IME language.
		virtual IMELanguage GetImeLanguage();
		/// Get the IME indicator string.
		virtual const char16_t* GetImeIndicator();
		/// Get the number of IME candidates.
		virtual int GetNumImeCandidates();
		/// Get the IME candidate selected.
		virtual int GetSelectedImeCandidate();
		/// Get the IME candidate at the specified index.
		virtual const char16_t* GetImeCandidate(int anIndex);

		/// Finalize the Ime String.
		/// @param bSend whether to send the composition string to IME window.
		virtual void ImeFinalizeString(bool bSend);
		/// Get the current IME composition string.
		virtual const char16_t* GetImeString();
		/// Get the IME cursor character index in the current composition string.
		virtual int GetImeCursorPosition();

	public:
		static void Initialize();
		static void Uninitialize();
		// Empty implementation of the IMM32 API
		static INPUTCONTEXT* WINAPI Dummy_ImmLockIMC( HIMC ) { return NULL; }
		static BOOL WINAPI Dummy_ImmUnlockIMC( HIMC ) { return FALSE; }
		static LPVOID WINAPI Dummy_ImmLockIMCC( HIMCC ) { return NULL; }
		static BOOL WINAPI Dummy_ImmUnlockIMCC( HIMCC ) { return FALSE; }
		static BOOL WINAPI Dummy_ImmDisableTextFrameService( DWORD ) { return TRUE; }
		static LONG WINAPI Dummy_ImmGetCompositionStringW( HIMC, DWORD, LPVOID, DWORD ) { return IMM_ERROR_GENERAL; }
		static DWORD WINAPI Dummy_ImmGetCandidateListW( HIMC, DWORD, LPCANDIDATELIST, DWORD ) { return 0; }
		// added 2010.4.24 Xizhi, for interprocess IME
		static HIMC WINAPI Dummy_ImmCreateContext() {return NULL;};
		static BOOL WINAPI Dummy_ImmDestroyContext( HIMC hIMC) {return FALSE;};
		static HIMC WINAPI Dummy_ImmGetContext( HWND ) { return NULL; }
		static BOOL WINAPI Dummy_ImmReleaseContext( HWND, HIMC ) { return FALSE; }
		static HIMC WINAPI Dummy_ImmAssociateContext( HWND, HIMC ) { return NULL; }
		static BOOL WINAPI Dummy_ImmGetOpenStatus( HIMC ) { return 0; }
		static BOOL WINAPI Dummy_ImmSetOpenStatus( HIMC, BOOL ) { return 0; }
		static BOOL WINAPI Dummy_ImmGetConversionStatus( HIMC, LPDWORD, LPDWORD ) { return 0; }
		static HWND WINAPI Dummy_ImmGetDefaultIMEWnd( HWND ) { return NULL; }
		static UINT WINAPI Dummy_ImmGetIMEFileNameA( HKL, LPSTR, UINT ) { return 0; }
		static UINT WINAPI Dummy_ImmGetVirtualKey( HWND ) { return 0; }
		static BOOL WINAPI Dummy_ImmNotifyIME( HIMC, DWORD, DWORD, DWORD ) { return FALSE; }
		static BOOL WINAPI Dummy_ImmSetConversionStatus( HIMC, DWORD, DWORD ) { return FALSE; }
		static BOOL WINAPI Dummy_ImmSimulateHotKey( HWND, DWORD ) { return FALSE; }
		static BOOL WINAPI Dummy_ImmIsIME( HKL ) { return FALSE; }
		static BOOL WINAPI Dummy_ImmSetCandidateWindow(HIMC, LPCANDIDATEFORM) {return FALSE;};

		// Traditional Chinese IME
		static UINT WINAPI Dummy_GetReadingString( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT ) { return 0; }
		static BOOL WINAPI Dummy_ShowReadingWindow( HIMC, BOOL ) { return FALSE; }

		// Verion library imports
		static BOOL APIENTRY Dummy_VerQueryValueA( const LPVOID, LPSTR, LPVOID *, PUINT ) { return 0; }
		static BOOL APIENTRY Dummy_GetFileVersionInfoA( LPSTR, DWORD, DWORD, LPVOID ) { return 0; }
		static DWORD APIENTRY Dummy_GetFileVersionInfoSizeA( LPSTR, LPDWORD ) { return 0; }

		// Function pointers: IMM32
		static INPUTCONTEXT* (WINAPI * _ImmLockIMC)( HIMC );
		static BOOL (WINAPI * _ImmUnlockIMC)( HIMC );
		static LPVOID (WINAPI * _ImmLockIMCC)( HIMCC );
		static BOOL (WINAPI * _ImmUnlockIMCC)( HIMCC );
		static BOOL (WINAPI * _ImmDisableTextFrameService)( DWORD );
		static LONG (WINAPI * _ImmGetCompositionStringW)( HIMC, DWORD, LPVOID, DWORD );
		static DWORD (WINAPI * _ImmGetCandidateListW)( HIMC, DWORD, LPCANDIDATELIST, DWORD );
		static HIMC (WINAPI* _ImmCreateContext)();
		static BOOL (WINAPI* _ImmDestroyContext)( HIMC hIMC);
		static HIMC (WINAPI * _ImmGetContext)( HWND );
		static BOOL (WINAPI * _ImmReleaseContext)( HWND, HIMC );
		static HIMC (WINAPI * _ImmAssociateContext)( HWND, HIMC );
		static BOOL (WINAPI * _ImmGetOpenStatus)( HIMC );
		static BOOL (WINAPI * _ImmSetOpenStatus)( HIMC, BOOL );
		static BOOL (WINAPI * _ImmGetConversionStatus)( HIMC, LPDWORD, LPDWORD );
		static HWND (WINAPI * _ImmGetDefaultIMEWnd)( HWND );
		static UINT (WINAPI * _ImmGetIMEFileNameA)( HKL, LPSTR, UINT );
		static UINT (WINAPI * _ImmGetVirtualKey)( HWND );
		static BOOL (WINAPI * _ImmNotifyIME)( HIMC, DWORD, DWORD, DWORD );
		static BOOL (WINAPI * _ImmSetConversionStatus)( HIMC, DWORD, DWORD );
		static BOOL (WINAPI * _ImmSimulateHotKey)( HWND, DWORD );
		static BOOL (WINAPI * _ImmIsIME)( HKL );
		static BOOL (WINAPI * _ImmSetCandidateWindow)(HIMC, LPCANDIDATEFORM);

		// Function pointers: Traditional Chinese IME
		static UINT (WINAPI * _GetReadingString)( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT );
		static BOOL (WINAPI * _ShowReadingWindow)( HIMC, BOOL );

		// Function pointers: Verion library imports
		static BOOL (APIENTRY * _VerQueryValueA)( const LPVOID, LPSTR, LPVOID *, PUINT );
		static BOOL (APIENTRY * _GetFileVersionInfoA)( LPSTR, DWORD, DWORD, LPVOID );
		static DWORD (APIENTRY * _GetFileVersionInfoSizeA)( LPSTR, LPDWORD );

	public:
		/** called when IME window is created. */
		static  HRESULT StaticOnCreateDevice();

		/** it just calls StaticMsgProc() followed by WinMsgProc */
		static bool HandleWinThreadMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

		/** called when IME window is created. */
		static  bool StaticMsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

		/** enable IME system. */
		static  void EnableImeSystem( bool bEnable );
		static  bool IsEnableImeSystem();

		/** [Thread-Safe] This function opens or closes the IME programmatically. 
		* In most cases, we should never programatically open or closes IME, instead the user usually pressed Ctrl+Space to change it. 
		* however, in some rare cases, such as we are opening a windowed mode flash window, and wants to disable IME programmatically. 
		* @param bOpen: true to open. 
		* @param bForceOpen: if true, we will force IME API to be called. otherwise API is only called if we are not toggling it. 
		*/
		static void SetIMEOpenStatus(bool bOpen, bool bForce=true);
		static void SetIMEOpenStatus_imp(bool bOpen);
		
		/** To be called when the application gains focus.*/
		static bool SwitchIn();

		/** To be called when the application looses focus. */
		static bool SwitchOut();

		/** To be called when a GUI virtual window gains focus.*/
		static bool OnFocusIn();
		/** [thread safe] To be called when a GUI virtual window gains focus.*/
		static bool OnFocusIn_imp();

		/** To be called when a GUI virtual window loses focus.*/
		static bool OnFocusOut();
		/** [thread safe] To be called when a GUI virtual window loses focus.*/
		static bool OnFocusOut_imp();

		/** This function opens or closes the IME programmtically. 
		* In most cases, we should never programmatically open or closes IME, instead the user usually pressed Ctrl+Space to change it. 
		* however, in some rare cases, such as we are opening a windowed mode flash window, and wants to disable IME programmatically. 
		* @param bOpen: true to open. 
		*/
		static bool GetIMEOpenStatus();

		/***/
		static bool WinMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
		
		static void ResetCaretBlink();

		/** whether IME has focus.*/
		static bool IMEHasFocus();

		/** [thread safe] */
		static IMESTATE GetImeStateS();

		
		/** send window message to a cache queue */
		static void SendWinMsgChar(WCHAR c);

		/** call this function to receive the latest composition string. 
		* @param bRemove: if true the composition string will be removed. 
		*/
		static std::wstring GetWinMsgChar(bool bRemove = true);

		/** when we use the default IME window to render, the application (EditBox control) should call this function to set the ime window position. */
		static void SetCompositionPosition(int x, int y);

		/** call this function to receive the latest composition string.
		* @param bRemove: if true the composition string will be removed.
		*/
		static std::wstring GetLastCompString(bool bRemove = true);
		static std::wstring GetCurrentCompString();

	protected:
		static WORD GetLanguage() { return LOWORD( s_hklCurrent ); }
		static WORD GetPrimaryLanguage() { return PRIMARYLANGID( LOWORD( s_hklCurrent ) ); }
		static WORD GetSubLanguage() { return SUBLANGID( LOWORD( s_hklCurrent ) ); }
		static void SendKey( BYTE nVirtKey );
		static DWORD GetImeId( UINT uIndex = 0 );
		static void CheckInputLocale();
		static void CheckToggleState();
		static void SetupImeApi();
		static void ResetCompositionString();
		static void TruncateCompString( bool bUseBackSpace = true, int iNewStrLen = 0 );
		static void FinalizeString( bool bSend );
		static void GetReadingWindowOrientation( DWORD dwId );
		static void GetPrivateReadingString();

		/** composition exit, so we will send characters to the application. */
		static void SendCompString();
		
	protected:
		enum { INDICATOR_NON_IME, INDICATOR_CHS, INDICATOR_CHT, INDICATOR_KOREAN, INDICATOR_JAPANESE };
		
		struct CCandList
		{
			WCHAR awszCandidate[MAX_CANDLIST][256];
			CUniBuffer HoriCand; // Candidate list string (for horizontal candidate window)
			int   nFirstSelected; // First character position of the selected string in HoriCand
			int   nHoriSelectedLen; // Length of the selected string in HoriCand
			DWORD dwCount;       // Number of valid entries in the candidate list
			DWORD dwSelection;   // Currently selected candidate entry relative to page top
			DWORD dwPageSize;
			int   nReadingError; // Index of the error character
			bool  bShowWindow;   // Whether the candidate list window is visible
			RECT  rcCandidate;   // Candidate rectangle computed and filled each time before rendered
		};

		struct CInputLocale
		{
			HKL   m_hKL;            // Keyboard layout
			WCHAR m_wszLangAbb[3];  // Language abbreviation
			WCHAR m_wszLang[64];    // Localized language name
		};

		// Application-wide data
		static HINSTANCE s_hDllImm32;         // IMM32 DLL handle
		static HINSTANCE s_hDllVer;           // Version DLL handle
		static HIMC      s_hImcDef;           // Default input context

		static HKL     s_hklCurrent;          // Current keyboard layout of the process
		static bool    s_bVerticalCand;       // Indicates that the candidates are listed vertically
		static LPWSTR  s_wszCurrIndicator;    // Points to an indicator string that corresponds to current input locale
		static WCHAR   s_aszIndicator[5][3];  // string to draw to indicate current input locale
		static bool    s_bInsertOnType;       // Insert the character as soon as a key is pressed (Korean behavior)
		static HINSTANCE s_hDllIme;           // Instance handle of the current IME module
		static IMESTATE  s_ImeState;          // IME global state
		static bool    s_bEnableImeSystem;    // Whether the IME system is active
		static POINT   s_ptCompString;        // Composition string position. Updated every frame.
		static int     s_nCompCaret;          // Caret position of the composition string
		static int     s_nFirstTargetConv;    // Index of the first target converted char in comp string.  If none, -1.
		static CUniBuffer s_CompString;       // Buffer to hold the composition string (we fix its length)
		static BYTE    s_abCompStringAttr[MAX_COMPSTRING_SIZE];
		static DWORD   s_adwCompStringClause[MAX_COMPSTRING_SIZE];
		static WCHAR   s_wszReadingString[32];// Used only with horizontal reading window (why?)
		static CCandList s_CandList;          // Data relevant to the candidate list
		static bool    s_bShowReadingWindow;  // Indicates whether reading window is visible
		static bool    s_bHorizontalReading;  // Indicates whether the reading window is vertical or horizontal
		static bool    s_bChineseIME;
		static CGrowableArray< CInputLocale > s_Locale; // Array of loaded keyboard layout on system

		// Static
		static bool s_bHideCaret;   // If true, we don't render the caret.
		static bool s_bRenderLocaleIndicator;
		static bool s_bIMETrap;
		static bool s_bIMEHasFocus;

		static bool m_bInsertMode;
		static bool m_bCaretOn;
		static double m_dfLastBlink;

		static POINT m_nCompositionWindowPoint;

		/** do not use the default window IME context, instead we will create a separate IME context to use with the main game window. */
		static bool s_bCreateIMEContext;

#if defined(DEBUG) | defined(_DEBUG)
		static bool    m_bIMEStaticMsgProcCalled;
#endif
		static ParaEngine::mutex s_mutex;
		static std::wstring s_sLastCompString;
		/** last chars received from WM_CHAR message. */
		static std::wstring s_sLastWindowsChars;
		static std::wstring s_sCurrentCompString;

		friend class CIMELock;
	};
}
#else
namespace ParaEngine
{
	/** GUI IME message system. 
	* all functions are static
	*/
	class CGUIIME
	{
	public:
		static  void EnableImeSystem( bool bEnable){};
		static  bool IsEnableImeSystem() {return false;};

		/** when we use the default IME window to render, the application (EditBox control) should call this function to set the ime window position. */
		static void SetCompositionPosition(int x, int y){};

		/** To be called when a GUI virtual window gains focus.*/
		static bool OnFocusIn() {return true;};
		
		/** To be called when a GUI virtual window loses focus.*/
		static bool OnFocusOut() {return true;};
	};
}

#endif