//=====================================================================================================
//
//=====================================================================================================

//=====================================================================================================
#ifndef __FLASHWINDOWCONTROL_INCLUDED_9C53B31_2B63_4f94_8DF9_FCC1F620494__
#define __FLASHWINDOWCONTROL_INCLUDED_9C53B31_2B63_4f94_8DF9_FCC1F620494__
//=====================================================================================================

//=====================================================================================================
// Includes

#include <oleauto.h>
#include <pshpack4.h>
#include <mmsystem.h>

//=====================================================================================================
#ifdef __cplusplus
extern "C" {
#endif
//=====================================================================================================

//====================================================================================================
// Flash control class name

#define WC_FLASHA                               "FlashPlayerControl"
#define WC_FLASHW                               L"FlashPlayerControl"

#ifdef UNICODE
#define WC_FLASH                                WC_FLASHW
#else
#define WC_FLASH                                WC_FLASHA
#endif // UNICODE

//=====================================================================================================
// Flash control styles

#define FPCS_TRANSPARENT			(0x00000001L)

#define FPCS_NEED_ALL_KEYS			(0x00000002L)

//=====================================================================================================
// Flash Control messages

#define FPCM_FIRST           (WM_USER + 0x1000)      // Flash Control messages

typedef struct SFPCQueryInterface
{
    // [in]
    // Identifier of the requested interface
    IID iid;
    // [in]
    // Receives the interface pointer requested in iid
    void* pvObject;
    // [out]
    HRESULT hr;

} SFPCQueryInterface;

#define FPCM_QUERYINTERFACE                     (FPCM_FIRST + 1)

typedef struct SFPCPutMovieFromMemory
{
    // [in]
    LPVOID lpData;
    // [in]
    DWORD dwSize;

} SFPCPutMovieFromMemory;

#define FPCM_PUTMOVIEFROMMEMORY                 (FPCM_FIRST + 2)

typedef struct SFPCLoadMovieFromMemory
{
	// [in] layer
	int layer;

    // [in]
    LPVOID lpData;
    // [in]
    DWORD dwSize;

} SFPCLoadMovieFromMemory;

#define FPCM_LOADMOVIEFROMMEMORY                (FPCM_FIRST + 3)

//================================================================================
//

#define FPCM_GET_FRAME_BITMAP                (FPCM_FIRST + 4)

typedef struct SFPCGetFrameBitmap
{
    // [out]
    HBITMAP hBitmap;

} SFPCGetFrameBitmap;

//================================================================================

//================================================================================

typedef struct SFPCPutStandardMenu
{
	// [in]
	VARIANT_BOOL StandardMenu;

} SFPCPutStandardMenu;

#define FPCM_PUT_STANDARD_MENU		(FPCM_FIRST + 5)

typedef struct SFPCGetStandardMenu
{
	// [out]
	VARIANT_BOOL StandardMenu;

} SFPCGetStandardMenu;

#define FPCM_GET_STANDARD_MENU		(FPCM_FIRST + 6)

//================================================================================

//================================================================================

typedef struct SFPCPutOverallOpaque
{
	// [in]
	// 0 - 255
	DWORD Value;

} SFPCPutOverallOpaque;

#define FPCM_PUT_OVERALL_OPAQUE		(FPCM_FIRST + 8)

typedef struct SFPCGetOverallOpaque
{
	// [out]
	// 0 - 255
	DWORD Value;

} SFPCGetOverallOpaque;

#define FPCM_GET_OVERALL_OPAQUE		(FPCM_FIRST + 9)

//================================================================================

typedef struct SFPCPutMovieUsingStream
{
    // [out]
    IStream* pStream;

} SFPCPutMovieUsingStream;

#define FPCM_PUT_MOVIE_USING_STREAM                 (FPCM_FIRST + 10)

typedef struct SFPCLoadMovieUsingStream
{
	// [in] layer
	int layer;

    // [out]
    IStream* pStream;

} SFPCLoadMovieUsingStream;

#define FPCM_LOAD_MOVIE_USING_STREAM                (FPCM_FIRST + 11)

// Flash external API

typedef struct SFPCCallFunction
{
    // [in]
    BSTR bstrRequest;
    // [out]
    BSTR bstrResponse;

    HRESULT hr;

} SFPCCallFunction;

#define FPCM_CALL_FUNCTION                          (FPCM_FIRST + 12)

typedef struct SFPCSetReturnValueA
{
    // [in]
    LPCSTR lpszReturnValue;

    HRESULT hr;

} SFPCSetReturnValueA;

#define FPCM_SET_RETURN_VALUEA                      (FPCM_FIRST + 14)

typedef struct SFPCSetReturnValueW
{
    // [in]
    LPCWSTR lpszReturnValue;

    HRESULT hr;

} SFPCSetReturnValueW;

#define FPCM_SET_RETURN_VALUEW                      (FPCM_FIRST + 15)

#ifdef UNICODE
#define FPCM_SET_RETURN_VALUE FPCM_SET_RETURN_VALUEW
#else
#define FPCM_SET_RETURN_VALUE FPCM_SET_RETURN_VALUEA
#endif // UNICODE

//=====================================================================================================

#include "messages.inl"

//=====================================================================================================
// Flash Control notifications

#define FPCN_FIRST                              (FPCM_FIRST - 1)

//=====================================================================================================
// 

//=====================================================================================================
struct SFPCLoadExternalResourceA
{    
    NMHDR hdr;
    
    // [in] RelativePath
    LPCSTR lpszRelativePath;
    // [in / out]
    LPSTREAM lpStream;
};

struct SFPCLoadExternalResourceW
{    
    NMHDR hdr;
    
    // [in] RelativePath
    LPCWSTR lpszRelativePath;
    // [in / out]
    LPSTREAM lpStream;
};

#ifdef UNICODE
#define SFPCLoadExternalResource                            SFPCLoadExternalResourceW
#else
#define SFPCLoadExternalResource                            SFPCLoadExternalResourceA
#endif // UNICODE

#define FPCN_LOADEXTERNALRESOURCEA              (FPCN_FIRST - 2)
#define FPCN_LOADEXTERNALRESOURCEW              (FPCN_FIRST - 3)

#ifdef UNICODE
#define FPCN_LOADEXTERNALRESOURCE                           FPCN_LOADEXTERNALRESOURCEW
#else
#define FPCN_LOADEXTERNALRESOURCE                           FPCN_LOADEXTERNALRESOURCEA
#endif // UNICODE
//=====================================================================================================

//=====================================================================================================
// External API
// [id(0x000000c5)]
// void FlashCall([in] BSTR request);
struct SFPCFlashCallInfoStructA
{    
    NMHDR hdr;
    // [in] request
    LPCSTR request;
};

struct SFPCFlashCallInfoStructW
{    
    NMHDR hdr;
    // [in] request
    LPCWSTR request;
};

#ifdef UNICODE
#define SFPCFlashCallInfoStruct                            SFPCFlashCallInfoStructW
#else
#define SFPCFlashCallInfoStruct                            SFPCFlashCallInfoStructA
#endif // UNICODE

#define FPCN_FLASHCALLA                       (FPCN_FIRST - 4)
#define FPCN_FLASHCALLW                       (FPCN_FIRST - 5)

#ifdef UNICODE
#define FPCN_FLASHCALL                                         FPCN_FLASHCALLW
#else
#define FPCN_FLASHCALL                                         FPCN_FLASHCALLA
#endif // UNICODE
//=====================================================================================================

//=====================================================================================================
struct SFPCLoadExternalResourceExA
{    
    NMHDR hdr;
    
    // [in] RelativePath
    LPCSTR lpszRelativePath;
    // [in]
    LPSTREAM lpStream;
	// [out]
	BOOL bHandled;
};

struct SFPCLoadExternalResourceExW
{    
    NMHDR hdr;
    
    // [in] RelativePath
    LPCWSTR lpszRelativePath;
    // [in]
    LPSTREAM lpStream;
	// [out]
	BOOL bHandled;
};

#ifdef UNICODE
#define SFPCLoadExternalResourceEx                            SFPCLoadExternalResourceExW
#else
#define SFPCLoadExternalResourceEx                            SFPCLoadExternalResourceExA
#endif // UNICODE

#define FPCN_LOADEXTERNALRESOURCEEXA              (FPCN_FIRST - 6)
#define FPCN_LOADEXTERNALRESOURCEEXW              (FPCN_FIRST - 7)

#ifdef UNICODE
#define FPCN_LOADEXTERNALRESOURCEEX                           FPCN_LOADEXTERNALRESOURCEEXW
#else
#define FPCN_LOADEXTERNALRESOURCEEX                           FPCN_LOADEXTERNALRESOURCEEXA
#endif // UNICODE
//=====================================================================================================

//================================================================================

typedef struct SFPCNPaint
{
    NMHDR hdr;

	// [in]
	LPDWORD lpPixels;

} SFPCNPaint;

#define FPCN_PAINT								  (FPCN_FIRST - 8)

//================================================================================

//================================================================================

typedef struct SFPCNUpdateRect
{
    NMHDR hdr;

	// [in]
	RECT rc;

} SFPCNUpdateRect;

#define FPCN_UPDATE_RECT						  (FPCN_FIRST - 9)

//================================================================================

//================================================================================

#define DEF_F_IN_BOX__PREPAINT_STAGE		(0)
#define DEF_F_IN_BOX__AFTERPAINT_STAGE		(1)

typedef struct SFPCNPaintStage
{
   NMHDR hdr;

   DWORD dwStage; // DEF_F_IN_BOX__*
   HDC hdc;
} SFPCNPaintStage;

#define FPCN_PAINT_STAGE						  (FPCN_FIRST - 10)

//================================================================================

//=====================================================================================================

#include "notification_messages.inl"

#include "functions_decl.inl"

//=====================================================================================================
// Functions

// FlashPlayerControl Window Class registration: using a registered flash
BOOL WINAPI RegisterFlashWindowClass();
// FlashPlayerControl Window Class registration: using a provided swflash.ocx/flash.ocx
BOOL WINAPI RegisterFlashWindowClassEx(LPVOID lpFlashOCXCodeData, DWORD dwSizeOfFlashOCXCode);

// FlashPlayerControl Window Class unregistration
void WINAPI UnregisterFlashWindowClass();

// Get installed flash version
DWORD WINAPI GetInstalledFlashVersion();
// Get using flash version
DWORD WINAPI GetUsingFlashVersion();

// Turn on/off flash sounds
void WINAPI FPCSetAudioEnabled(BOOL bEnable);
BOOL WINAPI FPCGetAudioEnabled();

//
BOOL WINAPI FPCIsTransparentAvailable();

//
BOOL WINAPI FPCIsFlashInstalled();

// 
typedef HRESULT (WINAPI *PLOADEXTERNALRESOURCEHANDLER)(LPCSTR lpszURL, IStream** ppStream);
void WINAPI FPCSetGlobalOnLoadExternalResourceHandler(PLOADEXTERNALRESOURCEHANDLER pHandler);

BOOL WINAPI FPCLoadMovieUsingStream(HWND hwndFlashPlayerControl, int layer, IStream** ppStream);
BOOL WINAPI FPCPutMovieUsingStream(HWND hwndFlashPlayerControl, IStream** ppStream);

BOOL WINAPI FPCLoadMovieFromStream(HWND hwndFlashPlayerControl, int layer, IStream* pStream);
BOOL WINAPI FPCPutMovieFromStream(HWND hwndFlashPlayerControl, IStream* pStream);

//
BOOL WINAPI FPCLoadMovieFromMemory(HWND hwndFlashPlayerControl, int layer, LPVOID lpData, DWORD dwSize);
BOOL WINAPI FPCPutMovieFromMemory(HWND hwndFlashPlayerControl, LPVOID lpData, DWORD dwSize);


BOOL WINAPI FPCLoadMovieFromResourceA(HWND hwndFlashPlayerControl, int layer, HINSTANCE hInstance, LPCSTR lpName, LPCSTR lpType);
BOOL WINAPI FPCLoadMovieFromResourceW(HWND hwndFlashPlayerControl, int layer, HINSTANCE hInstance, LPCWSTR lpName, LPCWSTR lpType);

#ifdef UNICODE
#define FPCLoadMovieFromResource FPCLoadMovieFromResourceW
#else
#define FPCLoadMovieFromResource FPCLoadMovieFromResourceA
#endif // UNICODE


BOOL WINAPI FPCPutMovieFromResourceA(HWND hwndFlashPlayerControl, HINSTANCE hInstance, LPCSTR lpName, LPCSTR lpType);
BOOL WINAPI FPCPutMovieFromResourceW(HWND hwndFlashPlayerControl, HINSTANCE hInstance, LPCWSTR lpName, LPCWSTR lpType);

#ifdef UNICODE
#define FPCPutMovieFromResource FPCPutMovieFromResourceW
#else
#define FPCPutMovieFromResource FPCPutMovieFromResourceA
#endif // UNICODE


// Set events listener
typedef void (WINAPI *PFLASHPLAYERCONTROLEVENTLISTENER)(HWND hwndFlashPlayerControl, LPARAM lParam, NMHDR* pNMHDR);
BOOL WINAPI FPCSetEventListener(HWND hwndFlashPlayerControl, PFLASHPLAYERCONTROLEVENTLISTENER pListener, LPARAM lParam);

// External API
HRESULT WINAPI FPCCallFunctionA(HWND hwndFlashPlayerControl, LPCSTR lpszRequest, LPSTR lpszResponse, DWORD* pdwResponseLength);
HRESULT WINAPI FPCCallFunctionW(HWND hwndFlashPlayerControl, LPCWSTR lpszRequest, LPWSTR lpszResponse, DWORD* pdwResponseLength);

#ifdef UNICODE
#define FPCCallFunction FPCCallFunctionW
#else
#define FPCCallFunction FPCCallFunctionA
#endif // UNICODE

HRESULT WINAPI FPCCallFunctionBSTR(HWND hwndFlashPlayerControl, BSTR bstrRequest, BSTR* bstrResponse);

HRESULT WINAPI FPCSetReturnValueA(HWND hwndFlashPlayerControl, LPCSTR lpszReturnValue);
HRESULT WINAPI FPCSetReturnValueW(HWND hwndFlashPlayerControl, LPCWSTR lpszReturnValue);

#ifdef UNICODE
#define FPCSetReturnValue FPCSetReturnValueW
#else
#define FPCSetReturnValue FPCSetReturnValueA
#endif // UNICODE

//
HRESULT WINAPI FPC_PutStandardMenu(HWND hwndFlashPlayerControl, BOOL bEnable);
HRESULT WINAPI FPC_GetStandardMenu(HWND hwndFlashPlayerControl, BOOL* pbEnable);

// Audio volume
#define DEF_MAX_FLASH_AUDIO_VOLUME	(0xFFFF)
// 0 <= nVolume <= DEF_MAX_FLASH_AUDIO_VOLUME
HRESULT WINAPI FPC_PutAudioVolume(DWORD dwVolume);
HRESULT WINAPI FPC_GetAudioVolume(DWORD* pdwVolume);

// Internals
// Don't use

//
HMODULE WINAPI FPC_Internal_GetFlashOCXHandle();

//
BOOL WINAPI FPC_Internal_HookFunc(LPCSTR lpszDLLName, LPCSTR lpszFuncName, LPVOID* pOldFunc, LPVOID lpNewFunc);

//=====================================================================================================
// API for supporting separate OCXs

typedef struct HFPC_
{
	void* p;

} *HFPC;

HFPC WINAPI FPC_LoadOCXCodeFromMemory(LPVOID lpFlashOCXCodeData, DWORD dwSizeOfFlashOCXCode);
HFPC WINAPI FPC_LoadRegisteredOCX();
BOOL WINAPI FPC_UnloadCode(HFPC hFPC);
BOOL WINAPI FPC_CanUnloadNow(HFPC hFPC);

ATOM WINAPI FPC_GetClassAtomA(HFPC hFPC);
ATOM WINAPI FPC_GetClassAtomW(HFPC hFPC);

#ifdef UNICODE
#define FPC_GetClassAtom                                FPC_GetClassAtomW
#else
#define FPC_GetClassAtom                                FPC_GetClassAtomA
#endif // UNICODE

LPCSTR WINAPI FPC_GetClassNameA(HFPC hFPC);
LPCWSTR WINAPI FPC_GetClassNameW(HFPC hFPC);

#ifdef UNICODE
#define FPC_GetClassName                                FPC_GetClassNameW
#else
#define FPC_GetClassName                                FPC_GetClassNameA
#endif // UNICODE

HWND WINAPI FPC_CreateWindowA(HFPC hFPC, 
                       DWORD dwExStyle,
                       LPCSTR lpWindowName,
                       DWORD dwStyle,
                       int x,
                       int y,
                       int nWidth,
                       int nHeight,
                       HWND hWndParent,
                       HMENU hMenu,
                       HINSTANCE hInstance,
                       LPVOID lpParam);

HWND WINAPI FPC_CreateWindowW(HFPC hFPC, 
                       DWORD dwExStyle,
                       LPCWSTR lpWindowName,
                       DWORD dwStyle,
                       int x,
                       int y,
                       int nWidth,
                       int nHeight,
                       HWND hWndParent,
                       HMENU hMenu,
                       HINSTANCE hInstance,
                       LPVOID lpParam);

#ifdef UNICODE
#define FPC_CreateWindow                                FPC_CreateWindowW
#else
#define FPC_CreateWindow                                FPC_CreateWindowA
#endif // UNICODE

// BETA
void WINAPI FPC_AttachToWindow(HFPC hFPC, HWND hWnd);

// Get flash version
DWORD WINAPI FPC_GetVersion(HFPC hFPC);

// Turn on/off flash sounds
void WINAPI FPC_EnableSound(HFPC hFPC, BOOL bEnable);
BOOL WINAPI FPC_IsSoundEnabled(HFPC hFPC);

typedef HRESULT (WINAPI *PLOAD_EXTERNAL_RESOURCE_HANDLERA)(LPCSTR lpszURL, IStream** ppStream, HFPC hFPC, LPARAM lParam);
typedef HRESULT (WINAPI *PLOAD_EXTERNAL_RESOURCE_HANDLERW)(LPCWSTR lpszURL, IStream** ppStream, HFPC hFPC, LPARAM lParam);

#ifdef UNICODE
#define PLOAD_EXTERNAL_RESOURCE_HANDLER PLOAD_EXTERNAL_RESOURCE_HANDLERW
#else
#define PLOAD_EXTERNAL_RESOURCE_HANDLER PLOAD_EXTERNAL_RESOURCE_HANDLERA
#endif // UNICODE

// Returns cookie
DWORD WINAPI FPC_AddOnLoadExternalResourceHandlerA(HFPC hFPC, PLOAD_EXTERNAL_RESOURCE_HANDLERA pHandler, LPARAM lParam);
DWORD WINAPI FPC_AddOnLoadExternalResourceHandlerW(HFPC hFPC, PLOAD_EXTERNAL_RESOURCE_HANDLERW pHandler, LPARAM lParam);

#ifdef UNICODE
#define FPC_AddOnLoadExternalResourceHandler FPC_AddOnLoadExternalResourceHandlerW
#else
#define FPC_AddOnLoadExternalResourceHandler FPC_AddOnLoadExternalResourceHandlerA
#endif // UNICODE

HRESULT WINAPI FPC_RemoveOnLoadExternalResourceHandler(HFPC hFPC, DWORD dwCookie);

// 0 <= nVolume <= DEF_MAX_FLASH_AUDIO_VOLUME
HRESULT WINAPI FPC_SetSoundVolume(HFPC hFPC, DWORD dwVolume);
DWORD WINAPI FPC_GetSoundVolume(HFPC hFPC);

//
HWND WINAPI FPC_GetAxHWND(HWND hWndFPC);

void WINAPI FPC_SetContext(HWND hWnd, LPCSTR lpszContext);

// IStream helpers
// useful for VB6 ;)

ULONG WINAPI FPC_IStream_AddRef(IStream* pStream);
ULONG WINAPI FPC_IStream_Release(IStream* pStream);
HRESULT WINAPI FPC_IStream_Write(IStream* pStream, const void *pv, ULONG cb, ULONG *pcbWritten);
HRESULT WINAPI FPC_IStream_SetSize(IStream* pStream, ULONG nSize);

typedef HRESULT (WINAPI *PGET_BIND_INFO_HANDLER)(HFPC hFPC, DWORD* grfBINDF, BINDINFO* pbindinfo, LPARAM lParam);
DWORD WINAPI FPC_AddGetBindInfoHandler(HFPC hFPC, PGET_BIND_INFO_HANDLER pHandler, LPARAM lParam);
HRESULT WINAPI FPC_RemoveGetBindInfoHandler(HFPC hFPC, DWORD dwCookie);

LPVOID* WINAPI FPC_GetImportTableEntry(HFPC hFPC, LPCSTR lpszDLLName, LPCSTR lpszFuncName);

int WINAPI FPC_StartMinimizeMemoryTimer(int nInterval);
void WINAPI FPC_StopMinimizeMemoryTimer(int nTimerId);
void WINAPI FPC_StopFPCMinimizeMemoryTimer(HFPC hFPC);

// Sound capturing API
typedef HRESULT (WINAPI *PSOUNDLISTENER)(HFPC hFPC, LPARAM lParam, PWAVEFORMATEX pWaveFormat, LPWAVEHDR pWaveHeader, UINT nHeaderSize);
DWORD WINAPI FPC_SetSoundListener(HFPC hFPC, PSOUNDLISTENER pSoundListener, LPARAM lParam);

// PreProcessURL
typedef void (WINAPI *PPREPROCESSURLHANDLER)(HFPC hFPC, LPARAM lParam, LPWSTR* pszURL, BOOL* pbContinue);
DWORD WINAPI FPC_SetPreProcessURLHandler(HFPC hFPC, PPREPROCESSURLHANDLER pHandler, LPARAM lParam);

// Useful macro to work with flash versions
// Example: FPV_GetVersion(hFPC) >= DEF_MAKE_FLASH_VERSION(9, 0, 28, 0)
#define DEF_MAKE_FLASH_VERSION(a, b, c, d) ( (d) | (c << 8) | (b << 16) | (a << 24) )

// Fullscreen
BOOL FPC_EnableFullScreen(HWND hwndFlashPlayerControl, BOOL bEnable);
BOOL FPC_IsFullScreenEnabled(HWND hwndFlashPlayerControl);

#define DEF_MINIMAL_FLASH_VERSION_THAT_ALLOWS_FULLSCREEN DEF_MAKE_FLASH_VERSION(9, 0, 28, 0)

// QueryInterface
HRESULT FPC_QueryInterface(HWND hwndFlashPlayerControl, REFIID iid, void** ppObject);

HFPC FPC_GetHFPC(HWND hwndFlashPlayerControl);

//=====================================================================================================

//=====================================================================================================
#ifdef __cplusplus
}
#endif
//=====================================================================================================

//=====================================================================================================
// Includes

#include <poppack.h>

//=====================================================================================================
#endif // !__FLASHWINDOWCONTROL_INCLUDED_9C53B31_2B63_4f94_8DF9_FCC1F620494__
//=====================================================================================================
