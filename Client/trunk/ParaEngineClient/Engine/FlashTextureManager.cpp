//-----------------------------------------------------------------------------
// Class:	CFlashTextureManager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2007.3.24, revised 2010.2.24
// Note: I used a third party wrapper of the flash player controller called "flash in a box control" or http://www.f-in-box.com
// for more information, please see the ./flashplayer/ directory.
// Note2: I manually decoded the header of SWF file(both compressed and uncompressed are supported) 
// to get the default size of the flash movie.
// - 2010.2.24:  flash windows are put to separate window threads and win messages are dispatched from that thread. 
// NPL can call flash functions and the function does not return until flash has returned (internally it post thread message to the flash window and wait for its return result before returning). 
// Flash can also call NPL functions, but the invocation is always returned immediately, the NPL functions will process any queued messages in its next update. 
// In both windowed and exclusive fullscreen mode, the flash window is created as child window of the main window. Only the rendering differs. In fullscreen mode, we copy from a buffer and render using d3d; in windowed mode, it is the native window rendering. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_FLASH_MANAGER
#include "FrameRateController.h"
#include "ParaWorldAsset.h"
#include "NPLHelper.h"
#include "ParaEngineApp.h"
#include "AISimulator.h"
#include "2dengine/GUIRoot.h"
#include "FlashTextureManager.h"

#include "SceneState.h"
#include "EffectManager.h"
#include "ParaEngineSettings.h"
#include "ParaEngineCore.h"

// f_in_box: third party dll
#include "flashplayer/f_in_box/include/f_in_box.h"

#include "memdebug.h"
#include <vector>
#include <zlib.h>
using namespace std;
using namespace ParaEngine;

/** @def max number of flash players */
#define DEFAULT_MAX_FLASH_PLAYER_NUM	2

/** @def if a flash player is not active for this number of seconds, it will be closed.*/
#define DEFAULT_FLASH_PLAYER_TIMEOUT	10.f

/** @def default player width */
#define 	DEFAULT_PLAYER_WIDTH	128
/** @def default player height */
#define 	DEFAULT_PLAYER_HEIGHT	128

/** @def custom user messages that are posted from the main thread to the flash window thread. */
#define PE_WM_FLASH_FIRST					WM_USER+2350
#define PE_WM_FLASH_LOADMOVIE				WM_USER+2351
#define PE_WM_FLASH_UNLOADMOVIE				WM_USER+2352
#define PE_WM_FLASH_CALL_FLASH_FUNCTION		WM_USER+2353
#define PE_WM_FLASH_SETCAPTURE				WM_USER+2354
#define PE_WM_FLASH_RELEASECAPTURE			WM_USER+2355
#define PE_WM_FLASH_SETFOCUS				WM_USER+2356
#define PE_WM_FLASH_QUIT					WM_USER+2357
#define PE_WM_FLASH_CALL_FLASH_ARG			WM_USER+2358
#define PE_WM_FLASH_LAST					WM_USER+2359

#pragma region SWF headers
/// flash player control handle
static HFPC g_hFPC = NULL;
DWORD g_dwHandlerCookie = 0;
static void WINAPI FPCListener(HWND hwndFlashPlayerControl, LPARAM lParam, NMHDR* pNMHDR);

HRESULT WINAPI GlobalOnLoadExternalResourceHandler(
	LPCTSTR lpszURL, 
	IStream** ppStream, 
	HFPC hFPC, 
	LPARAM lParam);

//////////////////////////////////////////////////////////////////////////
//
// SWF file related ones.
//
//////////////////////////////////////////////////////////////////////////

SWFHeader::SWFHeader()
{
	frameRate = 15;
	frameCount = 1;
}

SWFHeader::SWFHeader( CParaFile& in )
{
	in.read(Signature, 3);
	in.read(&Version, 1);
	fileLength = in.ReadDWORD(); // uncompressed size

	if(Signature[0] == 'C')
	{
		//////////////////////////////////////////////////////////////////////////
		// if it is a compressed SWF file data, all data after the first 8 bytes are compressed
		// so we will decode before getting some header. 
		const int pBufSize = 64; // just decompress at most 64 bytes for the header only
		char pBuf[pBufSize];

		// Setup the inflate stream.
		z_stream stream;
		int err;

		stream.next_in = (Bytef*)in.getPointer();
		stream.avail_in = (uInt)(in.getSize()-in.getPos());
		stream.next_out = (Bytef*)pBuf;
		stream.avail_out = pBufSize;
		stream.zalloc = (alloc_func)0;
		stream.zfree = (free_func)0;

		// Perform inflation. wbits < 0 indicates no zlib header inside the data.
		err = inflateInit(&stream);
		if (err == Z_OK)
		{
			err = inflate(&stream, Z_SYNC_FLUSH);
			if (err == Z_STREAM_END)
				err = Z_OK;
			err = Z_OK;
			inflateEnd(&stream);
		}
		
		if (err == Z_OK)
		{
			CParaFile in2((char*)pBuf, pBufSize);
			frameSize.Init(in2);
			frameRate  = in2.ReadWORD() >> 8; // why shift
			frameCount = in2.ReadWORD();
		}
	}
	else
	{
		frameSize.Init(in);
		frameRate  = in.ReadWORD() >> 8; // why shift
		frameCount = in.ReadWORD();
	}
}


SWFRect::SWFRect()
{
	//default size
	m_minX = 0;
	m_minY = 0;
	m_maxX = 11000;
	m_maxY = 8000;
	m_bitSize = -1;
}

int SWFRect::getMinX()
{
	return m_minX;
}

int SWFRect::getMinY()
{
	return m_minY;
}

int SWFRect::getMaxX()
{
	return m_maxX;
}

int SWFRect::getMaxY()
{
	return m_maxY;
}

void SWFRect::setMinX( int minX )
{
	m_minX = minX; m_bitSize = -1;
}

void SWFRect::setMinY( int minY )
{
	m_minY = minY; m_bitSize = -1;
}

void SWFRect::setMaxX( int maxX )
{
	m_maxX = maxX; m_bitSize = -1;
}

void SWFRect::setMaxY( int maxY )
{
	m_maxY = maxY; m_bitSize = -1;
}

SWFRect::SWFRect( int minX, int minY, int maxX, int maxY )
{
	m_minX = minX;
	m_minY = minY;
	m_maxX = maxX;
	m_maxY = maxY;
}

void SWFRect::Init( CParaFile& in , bool bUnsigned)
{
	in.synchBits();
	m_bitSize = (int)in.readUBits( 5 );
	if (bUnsigned)
	{
		m_minX    = (int)in.readUBits( m_bitSize );
		m_maxX    = (int)in.readUBits( m_bitSize );
		m_minY    = (int)in.readUBits( m_bitSize );
		m_maxY    = (int)in.readUBits( m_bitSize );
	}
	else
	{
		m_minX    = (int)in.readSBits( m_bitSize );
		m_maxX    = (int)in.readSBits( m_bitSize );
		m_minY    = (int)in.readSBits( m_bitSize );
		m_maxY    = (int)in.readSBits( m_bitSize );
	}
}
#pragma endregion SWF headers

//////////////////////////////////////////////////////////////////////////
//
// flash player instance
//
//////////////////////////////////////////////////////////////////////////

CFlashPlayer::CFlashPlayer(int nFlashPlayerIndex)
:m_nFlashPlayerIndex(nFlashPlayerIndex), m_nTextureWidth(0), m_nTextureHeight(0), m_dwWinThreadID(0), m_bWinThreadMovieLoaded(false), m_bWindowCreated(false), m_bInFlashFunc(false), m_bHasFocus(false)
{
	m_fTimeOut = DEFAULT_FLASH_PLAYER_TIMEOUT;
	m_fLastUsedTime = 0.f;
	m_bTextureUpdatedThisFrame = false;
	m_bFree = true;
	m_bTransparent = false; // disable transparency by default to increase speed.
	m_hwndFlashPlayerControl = NULL;
	m_nX = 0;
	m_nY = 0;
	m_nWidth = DEFAULT_PLAYER_WIDTH;
	m_nHeight = DEFAULT_PLAYER_HEIGHT;

	m_nBufWidth = 0;
	m_nBufHeight = 0;
	m_lpPixels = NULL;
	m_bNeedFlashUpdate = true;
	// default to false in release build. 
	m_bWindowMode = false; 
	m_bFlipVertical = false;

	m_nWindowX = 0;
	m_nWindowY = 0;
	m_nWindowWidth = 0;
	m_nWindowHeight = 0;

	m_pTexture = NULL;
	m_bUseDynamicTextures=false;
	m_TextureFormat = D3DFMT_X8R8G8B8;
	m_backgroundColor = 0x00ffffd0; // background color
}

CFlashPlayer::~CFlashPlayer()
{
	if(m_hwndFlashPlayerControl!=0)
	{
		if(m_win_thread && m_dwWinThreadID!=0)
		{
			PostWinThreadMessage(PE_WM_FLASH_QUIT, 0,0);
			m_win_thread->join();
		}
	}
	SAFE_DELETE_ARRAY(m_lpPixels);
	InvalidateDeviceObjects();
}

HWND CFlashPlayer::GetWindowHandle()
{
	return m_hwndFlashPlayerControl;
}

void CFlashPlayer::SetWindowMode(bool bIsWindowMode)
{
	m_bWindowMode = bIsWindowMode;
	if(bIsWindowMode)
	{
		m_bNeedFlashUpdate = false;
		// add WS_CLIPCHILDREN to the parent window. 
		SetWindowLong(CGlobals::GetAppHWND(), GWL_STYLE, GetWindowLong(CGlobals::GetAppHWND(), GWL_STYLE) | WS_CLIPCHILDREN);
		/*if(m_hwndFlashPlayerControl!=0)
			::ShowWindow(m_hwndFlashPlayerControl, SW_SHOW);*/
	}
	else
	{
		if(m_hwndFlashPlayerControl!=0)
			::ShowWindow(m_hwndFlashPlayerControl, SW_HIDE);
	}
}

void CFlashPlayer::MoveWindow(int x, int y, int width, int height, bool bRepaint)
{
	m_nX = x;
	m_nY = y;
	m_nWidth = width;
	m_nHeight = height;
	CParaEngineApp::GetInstance()->GameToClient(x, y);
	CParaEngineApp::GetInstance()->GameToClient(width, height);

	float fScaleX = 1.f;
	float fScaleY = 1.f;
	CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);

	if(fScaleX!=1.f)
	{
		x  = (int)((float)x * fScaleX);
		width = (int)((float)width * fScaleX);
	}

	if(fScaleY!=1.f)
	{
		y  = (int)((float)y * fScaleY);
		height = (int)((float)height * fScaleY);
	}

	m_nWindowX = x;
	m_nWindowY = y;
	m_nWindowWidth = width;
	m_nWindowHeight = height;
	::MoveWindow(m_hwndFlashPlayerControl, x, y, width, height, bRepaint ? TRUE : FALSE);
}

void CFlashPlayer::Repaint()
{
	if(!IsFree() && IsWindowMode())
	{
		InvalidateRect (m_hwndFlashPlayerControl, NULL, TRUE);
		UpdateWindow (m_hwndFlashPlayerControl);
	}
}

void CFlashPlayer::OnSizeChange()
{
	if(m_hwndFlashPlayerControl !=0 )
	{
		MoveWindow(m_nX, m_nY, m_nWidth, m_nHeight);
	}
}

void CFlashPlayer::SetWindowVisible(bool bVisible)
{
	if(bVisible)
	{
		m_bWindowMode = true;
		m_bNeedFlashUpdate = false;
	}
	::ShowWindow(m_hwndFlashPlayerControl, bVisible ? SW_SHOW : SW_HIDE);
}

LRESULT CFlashPlayer::PostMessage( UINT Msg,WPARAM wParam,LPARAM lParam )
{
	if(m_hwndFlashPlayerControl == 0)
		return E_FAIL;

	return ::PostMessage(m_hwndFlashPlayerControl, Msg, wParam, lParam);
}

LRESULT CFlashPlayer::SendMessage( UINT Msg,WPARAM wParam,LPARAM lParam )
{
	if(m_hwndFlashPlayerControl == 0)
		return E_FAIL;
	return ::SendMessage(m_hwndFlashPlayerControl, Msg, wParam, lParam);
}

void CFlashPlayer::InvalidateDeviceObjects()
{	
	SAFE_RELEASE(m_pTexture);
	m_bTextureUpdatedThisFrame = false;
}
bool CFlashPlayer::GetTextureInfo( int* width, int* height )
{
	/*if(m_nBufWidth>0)
		*width = m_nBufWidth;
	if(m_nBufHeight>0)
		*height = m_nBufHeight;*/
	if(m_nWidth>0)
		*width = m_nWidth;
	if(m_nHeight>0)
		*height = m_nHeight;
	return true;
}

LPDIRECT3DTEXTURE9 CFlashPlayer::GetTexture()
{
	m_fLastUsedTime = (float)(CGlobals::GetFrameRateController(FRC_RENDER)->GetTime());
	if(!m_bTextureUpdatedThisFrame)
	{
		m_bTextureUpdatedThisFrame = true;
		if(m_lpPixels == 0 || IsFree())
			return NULL;
		//////////////////////////////////////////////////////////////////////////
		//
		// copy from m_lpPixels to d3d texture
		//
		//////////////////////////////////////////////////////////////////////////

		if(m_pTexture == 0)
		{
			// TODO: create a new texture if the resolution of flash changes.
			CreateTexture(&m_pTexture);
		}
		if(m_pTexture)
		{
			UpdateTexture(m_pTexture);
		}
	}
	return m_pTexture;
}

void CFlashPlayer::RenderFrameMove(float fCurTime)
{
	if(IsFree())
		return;
	if(IsWindowMode())
	{
		m_bNeedFlashUpdate = false;
		m_bTextureUpdatedThisFrame = false;
		return;
	}
	
	m_bNeedFlashUpdate = true;
	m_bTextureUpdatedThisFrame = false;
	if((m_fLastUsedTime+m_fTimeOut)<=fCurTime)
	{
		UnloadMovie();
	}
	else
	{
		// If size is changed, forcing repaint each frame
		MoveWindow(m_nX, m_nY, m_nWidth, m_nHeight);
	}
}

void CFlashPlayer::LoadMovie( const char* filename )
{
	// if the flash player is already occupied, return immediately.
	if(!IsFree() && !IsWindowMode())
		return;
	bool bSameAsLast = m_filename == filename;
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);
		m_filename = filename;
	}

	string fileExt = CParaFile::GetFileExtension(m_filename);
	bool bIsFLV = (fileExt == "flv");
	bool bIsSWF = (fileExt == "swf");
	if(!(bIsSWF || bIsFLV))
		return;

	if(g_hFPC)
	{
		FPC_EnableSound(g_hFPC, true);
	}

	m_bFree = false;

	// Load movie from resource
	{
		if(!bSameAsLast)
		{
			//////////////////////////////////////////////////////////////////////////
			// add search path
#ifdef USE_FLASH_MANAGER
			{
				// TODO: shall we remove search path when loading a new movie or the old movie is unloaded.
				//  we will never remove search path since if multiple movie share the same search path. it will be wrong to remove.
				//CGlobals::GetAssetManager()->GetFlashManager().RemoveSearchPath(m_filename);
				CGlobals::GetAssetManager()->GetFlashManager().AddSearchPath(CParaFile::GetParentDirectoryFromPath(filename).c_str());
			}
#endif
			//////////////////////////////////////////////////////////////////////////
			// recalculate render and window size 

			OUTPUT_LOG("flash movie %s is loaded\n", filename);

			int nWidth = 0;
			int nHeight = 0;
			string fileExt = CParaFile::GetFileExtension(m_filename);
			if(fileExt == "flv")
			{
				// - "flv" texture is supported in addtional to "swf". extract movie dimension from file name instead of file content. 
				// file name should be "filename_width_height.flv". e.g. "paraengine_400_300.flv". If size is not provided. 256*256 is used. 
				string fileName = CParaFile::GetFileName(m_filename);
				size_t nPos = fileName.find('_');
				if(nPos!=string::npos)
				{
					sscanf(&(fileName[nPos+1]), "%d_%d", &nWidth, &nHeight);
				}
				bIsFLV = true;
			}
			else // if(fileExt == "swf")
			{
				// TODO: we do not need to open the file twice just for seeing its size. 
				CParaFile file;
				file.OpenAssetFile(m_filename.c_str());
				if(!file.isEof())
				{
					SWFHeader header(file);
					// do a pseudo conversion between twips and pixels
					const int TWIPS_TO_PIXELS = 20;
					nWidth = header.frameSize.getMaxX()/TWIPS_TO_PIXELS; 
					nHeight = header.frameSize.getMaxY()/TWIPS_TO_PIXELS;
				}
			}
			// use default size
			if(nWidth == 0)
				nWidth = 256;
			if(nHeight == 0)
				nHeight = 256;

			if(nWidth != m_nWidth || nHeight != m_nHeight)
			{
				m_nWidth = nWidth;
				m_nHeight = nHeight;
				// forcing the device object to be recreated, since the size of the flash texture has changed.
				InvalidateDeviceObjects();
			}
		}
		
		if(g_hFPC == NULL)
		{
			// Load flash ocx

			DWORD dwInstalledFlashVersion = GetInstalledFlashVersion();
			int FlashVersion1 = HIBYTE(HIWORD(dwInstalledFlashVersion));
			int FlashVersion2 = LOBYTE(HIWORD(dwInstalledFlashVersion));
			int FlashVersion3 = HIBYTE(LOWORD(dwInstalledFlashVersion)); 
			int FlashVersion4 = LOBYTE(LOWORD(dwInstalledFlashVersion));

			bool bUseLocalVersion = (FlashVersion1 >= 9);

			//  if "flash" appears in commands line or config.txt, we will use provided flash.ocx file. 
			const char* sUseProvidedFlash = CParaEngineApp::GetInstance()->GetAppCommandLineByParam("flash", NULL);
			if(sUseProvidedFlash!=0)
				bUseLocalVersion = false;
			
			ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();
			CDynamicAttributeField* pField = NULL;
			if((pField = settings.GetDynamicField("flash")))
				bUseLocalVersion = false;

			if(bUseLocalVersion)
			{
				OUTPUT_LOG("Local flash verion %d.%d.%d.%d is dicovered and used\n", FlashVersion1, FlashVersion2, FlashVersion3,FlashVersion4);

				// use the flash active control on the user's computer
				g_hFPC = FPC_LoadRegisteredOCX();
			}
			else
			{
				OUTPUT_LOG("Local flash verion %d.%d.%d.%d is ignored, we will use our own flash activex control\n", FlashVersion1, FlashVersion2, FlashVersion3,FlashVersion4);

				// use the one provided in the ParaEngine. 
				CParaFile file;
				file.OpenAssetFile("Texture/Flash.ocx");
				if(!file.isEof())
				{
					LPVOID lpFlashOCXCodeData = file.getBuffer();
					DWORD dwFlashOCXCodeSize = (DWORD)file.getSize();

					g_hFPC = FPC_LoadOCXCodeFromMemory(lpFlashOCXCodeData, dwFlashOCXCodeSize);
				}
				else
				{
					OUTPUT_LOG("error: Texture/Flash.ocx is not found\r\n");
				}
			}

			if (g_hFPC != 0)
			{
				g_dwHandlerCookie = FPC_AddOnLoadExternalResourceHandler(g_hFPC, &GlobalOnLoadExternalResourceHandler, 0);
			}
			else
			{
				OUTPUT_LOG("Failed loading flash player control. \r\n");
				return;
			}

		}

		if(m_hwndFlashPlayerControl==0)
		{
			if(StartWindowThread() == S_OK)
			{
				bool bWindowCreated = false;
				while(!bWindowCreated)
				{
					::Sleep(0);
					ParaEngine::Lock lock_(m_win_thread_mutex);
					bWindowCreated = m_bWindowCreated;
				}
				if(m_hwndFlashPlayerControl == 0)
					return;
			}
			else
			{
				return;
			}
		}
		// set the window mode. 
		SetWindowMode(m_bWindowMode);
		
		// force the window size
		MoveWindow(m_nX,m_nY, m_nWidth, m_nHeight, false);
		
		if(m_hwndFlashPlayerControl!=0)
		{
			if(PostWinThreadMessage(PE_WM_FLASH_LOADMOVIE, bIsFLV ? 1 : 0, 0))
			{
				// the main thread just wait until the movie is fully loaded. 
				bool bMovieLoaded = false;
				while(!bMovieLoaded)
				{
					::Sleep(100);
					ParaEngine::Lock lock_(m_win_thread_mutex);
					bMovieLoaded = m_bWinThreadMovieLoaded;
				}
				if(m_bWindowMode)
					SetWindowVisible(true);
			}
		}
	}
}

void CFlashPlayer::UnloadMovie()
{
	m_bFree = true;
	if(m_hwndFlashPlayerControl)
	{
		PostWinThreadMessage(PE_WM_FLASH_UNLOADMOVIE, 0,0);

		// set focus back to the 3d main window frame. 
		CGlobals::GetApp()->PostWinThreadMessage(PE_WM_SETFOCUS, (WPARAM)(CGlobals::GetAppHWND()), 0);
		
		if(g_hFPC)
		{
			FPC_EnableSound(g_hFPC, false);
			// FPC_SetSoundVolume(g_hFPC, 0);
		}
	}
}

void CFlashPlayer::SetFocus()
{
	PostWinThreadMessage(PE_WM_FLASH_SETFOCUS, 0,0);
}

static void WINAPI FPCListener(HWND hwndFlashPlayerControl, LPARAM lParam, NMHDR* pNMHDR)
{
	// TODO: shall we use a lock for thread safety here? Seems that this is always called in the same thread.

	CFlashPlayer* pFlashPlayer = (CFlashPlayer*)lParam;
	if(pFlashPlayer==0 || pFlashPlayer->IsFree())
		return;

	
	switch (pNMHDR->code)
	{
	case FPCN_UPDATE_RECT:
	{
		if(pFlashPlayer->IsWindowMode())
			return;
		/// this will prevent this function be called multiple times between two render frames
		if(!pFlashPlayer->m_bNeedFlashUpdate)
			return;
		if (pFlashPlayer->m_bTransparent)
		{
			SFPCGetFrameBitmap FPCGetFrameBitmap = { 0 };
			::SendMessage(pFlashPlayer->m_hwndFlashPlayerControl, FPCM_GET_FRAME_BITMAP, 0, (LPARAM)&FPCGetFrameBitmap);
			HBITMAP hBitmap = FPCGetFrameBitmap.hBitmap;
			{
				BITMAP bmp_info;
				GetObject(hBitmap, sizeof(bmp_info), &bmp_info);

				RECT rc;
				GetClientRect(hwndFlashPlayerControl, &rc);

				ParaEngine::Lock lock_(pFlashPlayer->m_win_thread_mutex);

				if (pFlashPlayer->m_nBufWidth != rc.right || pFlashPlayer->m_nBufHeight != rc.bottom)
				{
					SAFE_DELETE_ARRAY(pFlashPlayer->m_lpPixels);
					
					pFlashPlayer->m_nBufWidth = rc.right;
					pFlashPlayer->m_nBufHeight = rc.bottom;

					pFlashPlayer->m_lpPixels = new DWORD[pFlashPlayer->m_nBufWidth * pFlashPlayer->m_nBufHeight];
				}

				if(bmp_info.bmBits)
				{
					CopyMemory(pFlashPlayer->m_lpPixels, bmp_info.bmBits, pFlashPlayer->m_nBufWidth * pFlashPlayer->m_nBufHeight * sizeof(DWORD));
				}
				pFlashPlayer->m_bNeedFlashUpdate = false;
			}
			DeleteObject(hBitmap);
		}
		else
		{
			// FPCN_PAINT will be sent
			SendMessage(hwndFlashPlayerControl, WM_PAINT, 0, 0);
		}
		break;
	}
	case FPCN_PAINT:
	{
		/// this will prevent this function be called multiple times between two render frames
		if(pFlashPlayer->IsWindowMode())
			return;
		if(!pFlashPlayer->m_bNeedFlashUpdate)
			return;
		SFPCNPaint* pFPCNPaint = (SFPCNPaint*)pNMHDR;
		LPDWORD lpPixels = pFPCNPaint->lpPixels; // <-- pixels buffer

		RECT rc;
		GetClientRect(hwndFlashPlayerControl, &rc);

		ParaEngine::Lock lock_(pFlashPlayer->m_win_thread_mutex);
		if (pFlashPlayer->m_nBufWidth != rc.right || pFlashPlayer->m_nBufHeight != rc.bottom)
		{
			SAFE_DELETE_ARRAY(pFlashPlayer->m_lpPixels);

			pFlashPlayer->m_nBufWidth = rc.right;
			pFlashPlayer->m_nBufHeight = rc.bottom;

			pFlashPlayer->m_lpPixels = new DWORD[pFlashPlayer->m_nBufWidth * pFlashPlayer->m_nBufHeight];
		}

		if(lpPixels)
		{
			CopyMemory(pFlashPlayer->m_lpPixels, lpPixels, pFlashPlayer->m_nBufWidth * pFlashPlayer->m_nBufHeight * sizeof(DWORD));
		}

		pFlashPlayer->m_bNeedFlashUpdate = false;
		// You can copy lpPixels into internal buffer or create what you want from this buffer (bitmaps, textures, etc.)
		// Note that lpPixels is temporary and you shouldn't save this pointer for further using
		break;
	}
	case FPCN_ONPROGRESS:
		{
			SFPCOnProgressInfoStruct* pInfo = (SFPCOnProgressInfoStruct*)pNMHDR;

			OUTPUT_LOG("FPCN_ONPROGRESS: percentDone(%d)\n", pInfo->percentDone);
			break;
		}

	case FPCN_FSCOMMAND:
		{
			SFPCFSCommandInfoStruct* pInfo = (SFPCFSCommandInfoStruct*)pNMHDR;

			OUTPUT_LOG("FPCN_FSCOMMAND: command(%s), args(%s)\n", pInfo->command, pInfo->args);
			break;
		}
	case FPCN_FLASHCALL:
		{
			// Call an application function from a flash script
			SFPCFlashCallInfoStruct* pInfo = (SFPCFlashCallInfoStruct*)pNMHDR;   

			string request = "commonlib.flash_call_back(";
			char temp[101];
			itoa(pFlashPlayer->GetIndex(), temp, 10);
			request.append(temp);
			request.append(",");
			pFlashPlayer->SetFlashReturnValue(NULL);
			
			NPL::NPLHelper::EncodeStringInQuotation(request, (int)(request.size()), pInfo->request);
			request.append(")");
			CGlobals::GetAISim()->AddNPLCommand(request.c_str());

			// NOTE: since it is executed in a different thread, we do not have return values immediately. 
			FPCSetReturnValue(hwndFlashPlayerControl, pFlashPlayer->GetFlashReturnValue());
			break;
		}
	case FPCN_ONREADYSTATECHANGE:
		{
			SFPCOnReadyStateChangeInfoStruct* pInfo = (SFPCOnReadyStateChangeInfoStruct*)pNMHDR;

			OUTPUT_LOG("FPCN_ONREADYSTATECHANGE: newState(%d)\n", pInfo->newState);
			break;
		}
	case FPCN_LOADEXTERNALRESOURCE:
		{
			// The FPCN_LOADEXTERNALRESOURCE is sent when a movie tries to load an external resource (*.swf, *.jpg, *.mp3, BUT EXCEPT *.flv - flash video) using a relative path. 
			// For instance, a movie loads an image using the following code: loadMovie("images/external_image.jpg", "square");
			SFPCLoadExternalResource* pInfo = (SFPCLoadExternalResource*)pNMHDR;

			OUTPUT_LOG("warning: FPCN_LOADEXTERNALRESOURCE: %s \n", pInfo->lpszRelativePath);

			// TODO: how to make this thread safe? LiXizhi 2010.2
			// load locally.
			string fullFileName = CGlobals::GetAssetManager()->GetFlashManager().FindFile(pInfo->lpszRelativePath, true);
			if(!fullFileName.empty())
			{
				CParaFile file;
				file.OpenAssetFile(fullFileName.c_str());
				if(!file.isEof())
				{
					ULONG nWritten = 0;
					pInfo->lpStream->Write(file.getBuffer(), (ULONG) file.getSize(), &nWritten);
				}
			}
			break;
		}
	default:
		break;
	}
}

HRESULT CFlashPlayer::CreateTexture( LPDIRECT3DTEXTURE9 *ppTexture )
{
	// Create the texture that maps to this media type
	HRESULT hr = E_UNEXPECTED;
	UINT uintWidth = 2;
	UINT uintHeight = 2;

	// here let's check if we can use dynamic textures
	D3DCAPS9 caps;
	ZeroMemory( &caps, sizeof(D3DCAPS9));
	LPDIRECT3DDEVICE9 pd3dDevice=CGlobals::GetRenderDevice();
	hr = pd3dDevice->GetDeviceCaps( &caps );
	if( caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES )
	{
		m_bUseDynamicTextures = true;
	}

	// use true to test for graphics card that only supports pow2 textures. 
	if(/*true || */(caps.TextureCaps & D3DPTEXTURECAPS_POW2) )
	{
		while( (LONG)uintWidth < m_nBufWidth )
		{
			uintWidth = uintWidth << 1;
		}
		while( (LONG)uintHeight < m_nBufHeight )
		{
			uintHeight = uintHeight << 1;
		}
	}
	else
	{
		uintWidth = m_nBufWidth;
		uintHeight = m_nBufHeight;
	}
	m_nTextureWidth = uintWidth;
	m_nTextureHeight = uintHeight;
	// use the size in buffer
	
	if( m_bUseDynamicTextures )
	{
		hr = pd3dDevice->CreateTexture(uintWidth, uintHeight, 1, D3DUSAGE_DYNAMIC,
			D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,
			ppTexture, NULL);
		if( FAILED(hr))
		{
			m_bUseDynamicTextures = FALSE;
		}
	}
	else
	{
		hr = pd3dDevice->CreateTexture(uintWidth, uintHeight, 1, 0,
			D3DFMT_X8R8G8B8,D3DPOOL_MANAGED,
			ppTexture, NULL);
	}

	if( FAILED(hr))
	{
		OUTPUT_LOG(TEXT("Could not create the D3DX texture!  hr=0x%x"), hr);
		return hr;
	}

	// CreateTexture can silently change the parameters on us
	D3DSURFACE_DESC ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));

	if ( FAILED( hr = (*ppTexture)->GetLevelDesc( 0, &ddsd ) ) ) {
		OUTPUT_LOG(TEXT("Could not get level Description of D3DX texture! hr = 0x%x"), hr);
		return hr;
	}


	IDirect3DSurface9*  pSurf = NULL;

	if (SUCCEEDED(hr = (*ppTexture)->GetSurfaceLevel(0, &pSurf)))
		pSurf->GetDesc(&ddsd);

	// Save format info
	m_TextureFormat = ddsd.Format;

	if (m_TextureFormat != D3DFMT_X8R8G8B8) {
		OUTPUT_LOG(TEXT("Texture is format we can't handle! Format = 0x%x"), m_TextureFormat);
		SAFE_RELEASE(pSurf);
		return E_FAIL;
	}
	SAFE_RELEASE(pSurf);

	return S_OK;
}

HRESULT CFlashPlayer::UpdateTexture( LPDIRECT3DTEXTURE9 pTexture )
{
	if(pTexture == 0 ||  m_lpPixels == 0 || m_TextureFormat != D3DFMT_X8R8G8B8)
		return E_FAIL;
	
	D3DSURFACE_DESC d3dsd;
	pTexture->GetLevelDesc(0, &d3dsd);

	// Size
	SIZE size = { min(m_nWidth, m_nBufWidth), min(m_nHeight, m_nBufHeight) };

	D3DLOCKED_RECT rcLockedRect = { 0 };
	RECT rc = { 0, 0, size.cx, size.cy };

	if( m_bUseDynamicTextures )
	{
		if( FAILED(pTexture->LockRect(0, &rcLockedRect, 0, D3DLOCK_DISCARD)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(pTexture->LockRect(0, &rcLockedRect, 0, 0)))
			return E_FAIL;
	}
	ParaEngine::Lock lock_(m_win_thread_mutex);
	BYTE* pTextureBits = (BYTE*)rcLockedRect.pBits;
	DWORD* pBitmapBits = m_lpPixels;

	if(!m_bFlipVertical)
	{
		BYTE* pLineTextureBits = pTextureBits + rcLockedRect.Pitch * (size.cy - 1);
		for (int j = 0; j < size.cy; j++)
		{
			DWORD* pPixels = (DWORD*)pLineTextureBits;

			for (int i = 0; i < size.cx; i++)
			{
				*pPixels = *pBitmapBits;

				pPixels++;
				pBitmapBits++;
			}

			pLineTextureBits -= rcLockedRect.Pitch;
		}
	}
	else
	{
		BYTE* pLineTextureBits = pTextureBits;
		for (int j = 0; j < size.cy; j++)
		{
			DWORD* pPixels = (DWORD*)pLineTextureBits;
			for (int i = 0; i < size.cx; i++)
			{
				*pPixels = *pBitmapBits;
				pPixels++;
				pBitmapBits++;
			}
			pLineTextureBits += rcLockedRect.Pitch;
		}
	}
	// Unlock the Texture
	if (FAILED(pTexture->UnlockRect(0)))
		return E_FAIL;
	return S_OK;
}


const char* CFlashPlayer::GetFileName()
{
	return m_filename.c_str();
}

// TODO: add this to scripting interface
void CFlashPlayer::PutFlashVars(const char* args)
{
	if(args == 0)
		return;
	m_bInFlashFunc = true;
	m_sCurrentFlashFunc = args;
	if(PostWinThreadMessage(PE_WM_FLASH_CALL_FLASH_ARG, 0, 0))
	{
		bool bInFlashFunc = true;
		while (bInFlashFunc)
		{
			::Sleep(10);
			ParaEngine::Lock lock_(m_win_thread_mutex);
			bInFlashFunc = m_bInFlashFunc;
		}
	}
}

const char* CFlashPlayer::CallFlashFunction(const char* sFunc)
{
	if(sFunc == NULL)
	{
		return CGlobals::GetString(0).c_str();
	}
	m_bInFlashFunc = true;
	m_sCurrentFlashFunc = sFunc;
	
	if(PostWinThreadMessage(PE_WM_FLASH_CALL_FLASH_FUNCTION, 0, 0))
	{
		bool bInFlashFunc = true;
		while (bInFlashFunc)
		{
			::Sleep(10);
			ParaEngine::Lock lock_(m_win_thread_mutex);
			bInFlashFunc = m_bInFlashFunc;
		}
		return m_sCurrentFlashFuncResponse.c_str();
	}
	else
	{
		return CGlobals::GetString(0).c_str();
	}
}


void CFlashPlayer::SetFlashReturnValue(const char* sResult)
{
	if(sResult)
	{
		m_sFlashReturnValue = sResult;
	}
	else
	{
		m_sFlashReturnValue = "<string></string>";
	}
}

const char* CFlashPlayer::GetFlashReturnValue()
{
	return m_sFlashReturnValue.c_str();
}

void CFlashPlayer::UpdatePixelsBuffer()
{
	SFPCGetFrameBitmap FPCGetFrameBitmap = { 0 };
	::SendMessage(m_hwndFlashPlayerControl, FPCM_GET_FRAME_BITMAP, 0, (LPARAM)&FPCGetFrameBitmap);
	HBITMAP hBitmap = FPCGetFrameBitmap.hBitmap;
	{
		BITMAP bmp_info;
		GetObject(hBitmap, sizeof(bmp_info), &bmp_info);

		RECT rc;
		GetClientRect(m_hwndFlashPlayerControl, &rc);

		if (m_nBufWidth != rc.right || m_nBufHeight != rc.bottom)
		{
			SAFE_DELETE_ARRAY(m_lpPixels);

			m_nBufWidth = rc.right;
			m_nBufHeight = rc.bottom;

			m_lpPixels = new DWORD[m_nBufWidth * m_nBufHeight];
		}
		if(bmp_info.bmBits == 0)
		{
			m_bFlipVertical = true;
			GetBitmapBits(hBitmap,bmp_info.bmWidth*bmp_info.bmHeight*sizeof(DWORD),m_lpPixels);
		}
		else
		{
			m_bFlipVertical = false;
			CopyMemory(m_lpPixels, bmp_info.bmBits, m_nBufWidth * m_nBufHeight * sizeof(DWORD));
		}
		m_bNeedFlashUpdate = false;
	}
	DeleteObject(hBitmap);
}



bool CFlashPlayer::PostWinThreadMessage(UINT message,WPARAM wParam,LPARAM lParam)
{
	if(m_dwWinThreadID!=0)
	{
		return !!::PostThreadMessage(m_dwWinThreadID, message, wParam, lParam);
	}
	return false;
}


bool CFlashPlayer::MsgProcCustom(UINT message,WPARAM wParam,LPARAM lParam)
{
	if (message>=PE_WM_FLASH_FIRST && message<=PE_WM_FLASH_LAST)
	{
		switch (message)
		{
		case PE_WM_FLASH_LOADMOVIE:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				m_bWinThreadMovieLoaded = true;

				if(wParam == 1)
				{
					// video file. 
					const char DEFAULT_FLV_PLAYER[] = "Texture/flvplayer.swf";
					CParaFile file;
					file.OpenAssetFile(DEFAULT_FLV_PLAYER);
					if(!file.isEof())
					{
						// Note LXZ 2007.10.5: we will use a speciial URL that begins with "http://FLV" to mark that the flv file should be loaded to memory from paraengine file system. 
						string strFlashVars = "FLVPath=";
						strFlashVars += "http://FLV/";
						strFlashVars += m_filename;
						FPC_PutFlashVars(m_hwndFlashPlayerControl, strFlashVars.c_str());

						// flash vars must be put before movie is loaded to memory.
						FPCPutMovieFromMemory(m_hwndFlashPlayerControl, file.getBuffer(), (DWORD)file.getSize());
					}
					else
					{
						OUTPUT_LOG("warning: default flvplayer is not found %s", DEFAULT_FLV_PLAYER);
						return 1;
					}
				}
				else // swf file here
				{
					CParaFile file;
					file.OpenAssetFile(m_filename.c_str());
					if(!file.isEof())
					{
						// for local file. just load from file system
						FPCPutMovieFromMemory(m_hwndFlashPlayerControl, file.getBuffer(), (DWORD)file.getSize());
					}
					else
					{
						// deal with non local file. We shall play from remote server 
						if(m_filename[0] == 'h' && m_filename[1] == 't' && m_filename[2] == 't' && m_filename[3] == 'p')
						{
							FPC_LoadMovie(m_hwndFlashPlayerControl, 0, m_filename.c_str());
						}
						else
							OUTPUT_LOG("warning: flash file %s is not found", m_filename.c_str());
						return 1;
					}
				}
				// Play: when we load a move,  it is played by default, so no need to play it twice here 
				// FPC_Play(m_hwndFlashPlayerControl);
				FPCSetEventListener(m_hwndFlashPlayerControl, &FPCListener, (LPARAM)this);
				FPC_PutBackgroundColor(m_hwndFlashPlayerControl, (DWORD)m_backgroundColor);

				break;
			}
		case PE_WM_FLASH_UNLOADMOVIE:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				m_bWinThreadMovieLoaded = false;

				// TODO: how to stop a movie. FPCM_STOPPLAY does not stop music. And one needs to call play when this control is reused next time.
				SFPCStop info;
				::SendMessage(m_hwndFlashPlayerControl, FPCM_STOP, 0, (LPARAM)&info);
				if (FAILED(info.hr))
				{
				}
				// TODO: Bug: To unload you can load a movie from non-existent path ; still sound playing. 
				FPC_PutMovie(m_hwndFlashPlayerControl, ""); 

				if(g_hFPC)
				{
					FPC_EnableSound(g_hFPC, false);
					// FPC_SetSoundVolume(g_hFPC, 0);
				}
				OUTPUT_LOG("flash movie %s is unloaded\n", m_filename.c_str());
				break;
			}
		case PE_WM_FLASH_CALL_FLASH_ARG:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				FPC_PutFlashVars(m_hwndFlashPlayerControl, m_sCurrentFlashFunc.c_str());	
				m_bInFlashFunc = false;
				break;
			}
		case PE_WM_FLASH_CALL_FLASH_FUNCTION:
			{
				ParaEngine::Lock lock_(m_win_thread_mutex);
				m_sCurrentFlashFuncResponse.resize(2048);
				m_sCurrentFlashFuncResponse[0] = '\0';
				DWORD dwLength = (DWORD)(m_sCurrentFlashFuncResponse.size());
				FPCCallFunction(m_hwndFlashPlayerControl, m_sCurrentFlashFunc.c_str(), &(m_sCurrentFlashFuncResponse[0]), &dwLength); 
				m_bInFlashFunc = false;
				break;
			}
		case PE_WM_FLASH_SETFOCUS:
			{
				::SetFocus(m_hwndFlashPlayerControl);
				break;
			}
		case PE_WM_FLASH_SETCAPTURE:
			{
				::SetCapture(m_hwndFlashPlayerControl);
				break;
			}
		case PE_WM_FLASH_RELEASECAPTURE:
			{
				::ReleaseCapture();
				break;
			}
		case PE_WM_FLASH_QUIT:
			{
				DestroyWindow( m_hwndFlashPlayerControl );
				PostQuitMessage((int)wParam);
				break;
			}
		default:
			break;
		}
		return 1;
	}
	return 0;
}

void CFlashPlayer::DefaultWinThreadProc()
{
	//
	// create a window to render into. Register the windows class
	// 
	{
		ParaEngine::Lock lock_(m_win_thread_mutex);

		OUTPUT_LOG("flash window thread started. and a window is created in it\n");

		// must have FPCS_TRANSPARENT for FPCN_PAINT to be received. 
		DWORD wndStyle = m_bTransparent ? WS_POPUP : (WS_CHILD | FPCS_TRANSPARENT );
		if(IsWindowMode())
		{
			wndStyle = WS_CHILD;
		}

		m_hwndFlashPlayerControl = 
			FPC_CreateWindow(g_hFPC, 
			m_bTransparent ? WS_EX_LAYERED : 0, 
			NULL, 
			wndStyle, 
			0, 
			0, 
			m_nWidth, 
			m_nHeight, 
			CGlobals::GetAppHWND(), 
			NULL, 
			NULL, 
			NULL);
		if(m_hwndFlashPlayerControl == 0)
		{
			OUTPUT_LOG("error: FPC_CreateWindow failed\n");
			m_bWindowCreated = true;
			return;
		}
		
		m_dwWinThreadID = ::GetWindowThreadProcessId(m_hwndFlashPlayerControl, NULL);

		FPC_SetContext(m_hwndFlashPlayerControl, "6780fbf3-ee96-4ffe-9e6f-14b2b13afb9b");

		// inform to window creation
		m_bWindowCreated = true;
	}

	//
	// Dispatching window messages in this window thread. 
	//

	// Now we're ready to receive and process Windows messages.
	MSG  msg;
	msg.message = WM_NULL;
	PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

	while( WM_QUIT != msg.message  )
	{
		if( GetMessage( &msg, NULL, 0U, 0U ) != 0 )
		{
			if(MsgProcCustom(msg.message, msg.wParam, msg.lParam) == 0)
			{
				// Translate and dispatch the message
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
	}
	OUTPUT_LOG("Flash window thread exit\n");
	m_dwWinThreadID = 0;
}

HRESULT CFlashPlayer::StartWindowThread()
{
	if(m_win_thread.get()==0)
	{
		// start the io thread
		m_win_thread.reset(new boost::thread(boost::bind(&CFlashPlayer::DefaultWinThreadProc, this)));
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

bool ParaEngine::CFlashPlayer::HasFocus()
{
	return m_bHasFocus;
}

bool CFlashPlayer::SetParentWindow( HWND hWnd )
{
	if(m_hwndFlashPlayerControl)
	{
		if(::SetParent(m_hwndFlashPlayerControl, hWnd) == NULL)
		{
			OUTPUT_LOG("warning: CFlashPlayer::SetParentWindow failed\n");
		}
	}
	return true;
}

#pragma region flash_texture_manager 

bool CFlashTextureManager::m_bHasFocus = false;
//////////////////////////////////////////////////////////////////////////
//
// flash texture manager 
//
//////////////////////////////////////////////////////////////////////////

CFlashTextureManager::CFlashTextureManager(void)
{
	m_FlashPlayers.clear();
	SetMaxFlashPlayerNum(DEFAULT_MAX_FLASH_PLAYER_NUM);
}

CFlashTextureManager::~CFlashTextureManager(void)
{
	Cleanup();
}

void CFlashTextureManager::Cleanup()
{
	UnloadAsset();

	if(g_hFPC!=0)
	{
		if(g_dwHandlerCookie!=0)
			FPC_RemoveOnLoadExternalResourceHandler(g_hFPC, g_dwHandlerCookie);
		FPC_UnloadCode(g_hFPC);
		g_hFPC = NULL;
	}
}

void CFlashTextureManager::LoadAsset()
{

}

void CFlashTextureManager::UnloadAsset()
{
	// unload flash controls
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		SAFE_DELETE(m_FlashPlayers[i]);
	}
}

void CFlashTextureManager::GarbageCollectAll()
{
	UnloadAsset();
}

void CFlashTextureManager::SetMaxFlashPlayerNum( int nNum )
{
	if(nNum>=GetMaxFlashPlayerNum())
	{
		int nCount = GetMaxFlashPlayerNum();
		m_FlashPlayers.resize(nNum, NULL);
		for (int i=nCount;i<nNum;++i)
		{
			m_FlashPlayers[i] = 0;
		}
	}
	else
	{
		int nCount = GetMaxFlashPlayerNum();
		for (int i=nNum;i<nCount;++i)
		{
			SAFE_DELETE(m_FlashPlayers[i]);
		}
		m_FlashPlayers.resize(nNum, NULL);
	}
}

int CFlashTextureManager::GetMaxFlashPlayerNum()
{
	return (int)m_FlashPlayers.size();
}

int CFlashTextureManager::GetFlashPlayerIndex( const char* sFileName )
{
	// return the texture if it already exists.
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];
		if(pFlashPlayer && pFlashPlayer->m_filename == sFileName)
		{
			return i;
		}
	}
	return -1;
}

CFlashPlayer* CFlashTextureManager::GetFlashPlayer(int nIndex)
{
	int nCount = GetMaxFlashPlayerNum();
	if(nIndex>=0 && nIndex<nCount)
	{
		return m_FlashPlayers[nIndex];
	}
	return NULL;
}

CFlashPlayer* CFlashTextureManager::GetFlashPlayer(const char* sFileName)
{
	// return the texture if it already exists.
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];
		if(pFlashPlayer && pFlashPlayer->m_filename == sFileName)
		{
			if(pFlashPlayer->IsFree())
				pFlashPlayer->LoadMovie(sFileName);
			return pFlashPlayer;
		}
	}
	// try creating or updating the a player if there is no flash player matching the file name.
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];
		if(pFlashPlayer == 0)
		{
			pFlashPlayer = new CFlashPlayer(i);
			m_FlashPlayers[i] = pFlashPlayer;
		}
		if(pFlashPlayer && pFlashPlayer->IsFree())
		{
			pFlashPlayer->LoadMovie(sFileName);
			return pFlashPlayer;
		}
	}
	return NULL;
}

LPDIRECT3DTEXTURE9 CFlashTextureManager::GetTexture( const char* sFileName )
{
	CFlashPlayer* pFlashPlayer =  GetFlashPlayer(sFileName);
	if(pFlashPlayer)
	{
		return pFlashPlayer->GetTexture();
	}
	return NULL;
}

bool CFlashTextureManager::UnloadTexture( const char* sFileName )
{
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];
		if(pFlashPlayer && pFlashPlayer->m_filename == sFileName)
		{
			//pFlashPlayer->UnloadMovie();
			SAFE_DELETE(m_FlashPlayers[i]);
		}
	}
	return true;
}


void CFlashTextureManager::OnSizeChange()
{
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];
		if(pFlashPlayer)
		{
			pFlashPlayer->OnSizeChange();
		}
	}
}

// TODO: this is not accurate implementation, we will assume flash will have focus if it is visible.
bool ParaEngine::CFlashTextureManager::HasFocus()
{
	// no lock is needed, since we allow inconsistent values. 
	return m_bHasFocus;
}

void CFlashTextureManager::RenderFrameMove(float fElapsedTime)
{
	m_bHasFocus = false;
	float fCurTime = (float)(CGlobals::GetFrameRateController(FRC_RENDER)->GetTime());
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];
		if(pFlashPlayer)
		{
			m_bHasFocus = m_bHasFocus || !pFlashPlayer->IsFree();
			pFlashPlayer->RenderFrameMove(fCurTime);
		}
	}
}

HRESULT CFlashTextureManager::InitDeviceObjects()
{
	return S_OK;
}

HRESULT CFlashTextureManager::RestoreDeviceObjects()
{
	return S_OK;
}

HRESULT CFlashTextureManager::InvalidateDeviceObjects()
{
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];
		if(pFlashPlayer)
		{
			pFlashPlayer->InvalidateDeviceObjects();
		}
	}
	return S_OK;
}

HRESULT CFlashTextureManager::DeleteDeviceObjects()
{
	return S_OK;
}

bool CFlashTextureManager::GetTextureInfo( const char* sFileName, int* width, int* height )
{
	CFlashPlayer* pFlashPlayer =  GetFlashPlayer(sFileName);
	if(pFlashPlayer)
	{
		return pFlashPlayer->GetTextureInfo(width, height);
	}
	return false;
}

string CFlashTextureManager::FindFile(const string& filename, bool bFindInSearchPaths)
{
	string fullFileName;
	list<SearchPath>& searchPaths = GetSearchPaths();

	if(!bFindInSearchPaths ||searchPaths.size()==0 || filename[0] == '/' || filename[0] == '\\')
	{
		if(CParaFile::DoesAssetFileExist(filename.c_str()))
			fullFileName = filename;
	}
	else
	{
		bool bFound = CParaFile::DoesAssetFileExist(filename.c_str());
		if(bFound)
		{
			fullFileName = filename;
		}
		else
		{
			list<SearchPath>::iterator itCurCP, itEndCP = searchPaths.end();
			for( itCurCP = searchPaths.begin(); !bFound && itCurCP != itEndCP; ++ itCurCP)
			{
				bFound = CParaFile::DoesAssetFileExist(((*itCurCP).GetPath()+filename).c_str());
				if(bFound)
				{
					fullFileName = (*itCurCP).GetPath()+filename;
				}
			}
		}
	}
	return fullFileName;
}

void CFlashTextureManager::RepaintAll()
{
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];
		if(pFlashPlayer)
		{
			pFlashPlayer->Repaint();
		}
	}
}

bool ParaEngine::CFlashTextureManager::SetParentWindow( HWND hWnd )
{
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];
		if(pFlashPlayer)
		{
			pFlashPlayer->SetParentWindow(hWnd);
		}
	}
	return true;
}

void CFlashTextureManager::RenderFlashWindows(SceneState& sceneState)
{
	int nWindowCount = 0;
	int nCount = GetMaxFlashPlayerNum();
	for (int i=0;i<nCount;++i)
	{
		CFlashPlayer* pFlashPlayer = m_FlashPlayers[i];

		if(pFlashPlayer!=0 && (!pFlashPlayer->IsFree() && pFlashPlayer->IsWindowMode()))
		{
			//if(pFlashPlayer->m_bNeedFlashUpdate == false)
			{
				pFlashPlayer->Repaint();
				pFlashPlayer->UpdatePixelsBuffer();
				
				//pFlashPlayer->m_bNeedFlashUpdate = true;
				
				LPDIRECT3DTEXTURE9 pTexture = pFlashPlayer->GetTexture();

				if(pTexture!=0)
				{
					nWindowCount ++;
					if(nWindowCount == 1)
					{
						if( ! CGlobals::GetEffectManager()->BeginEffect(TECH_GUI, &(sceneState.m_pCurrentEffect)) )
							break;
					}
					/** draw the mask square in front of the screen. */
					//////////////////////////////////////////////////////////////////////////
					// fixed programming pipeline
					float sx = (float)pFlashPlayer->m_nWindowWidth,sy = (float)pFlashPlayer->m_nWindowHeight;
					float fLeft = (float)pFlashPlayer->m_nWindowX - 0.5f;
					float fTop = (float)pFlashPlayer->m_nWindowY - 0.5f;

					float fU = (pFlashPlayer->m_nWindowWidth == pFlashPlayer->m_nTextureWidth) ? 1.f : ((float)pFlashPlayer->m_nWindowWidth / (float)pFlashPlayer->m_nTextureWidth);
					float fV = (pFlashPlayer->m_nWindowHeight == pFlashPlayer->m_nTextureHeight) ? 1.f : ((float)pFlashPlayer->m_nWindowHeight/ (float)pFlashPlayer->m_nTextureHeight);
					DXUT_SCREEN_VERTEX v[10];
					v[0].x = fLeft;  v[0].y = fTop + sy; v[0].tu = 0;  v[0].tv = fV;
					v[1].x = fLeft;  v[1].y = fTop;  v[1].tu = 0;  v[1].tv = 0;
					v[2].x = fLeft + sx; v[2].y = fTop + sy; v[2].tu = fU; v[2].tv = fV;
					v[3].x = fLeft + sx; v[3].y = fTop;  v[3].tu = fU; v[3].tv = 0;

					DWORD dwColor = LinearColor(1.f,1.f,1.f,1.f);
					int i;
					for(i=0;i<4;i++)
					{
						v[i].color = dwColor;
						v[i].z = 0;
						v[i].h = 1.0f;
					}
					sceneState.m_pd3dDevice->SetTexture(0, pTexture);
					pD3dDevice->DrawPrimitiveUP( sceneState.m_pd3dDevice, RenderDeviceBase::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLESTRIP, 2, v, sizeof(DXUT_SCREEN_VERTEX) );
				}
			}
		}
	}
}

/** for loading global resource. i.e. resource that begins with http:// 
*/
HRESULT WINAPI GlobalOnLoadExternalResourceHandler(
	LPCTSTR lpszURL, 
	IStream** ppStream, 
	HFPC hFPC, 
	LPARAM lParam)
{
	HRESULT hr = E_FAIL;

	string fileName = lpszURL;
	// if the url begins with "http://FLV/",  we will load from memory. In most cases, it will be an FLV file. 
	if(fileName.size()>11 && (fileName.substr(0, 11) == "http://FLV/"))
	{
		fileName = fileName.substr(11);

		string fullFileName = CGlobals::GetAssetManager()->GetFlashManager().FindFile(fileName.c_str(), true);
		if(!fullFileName.empty())
		{
			CParaFile file;
			file.OpenAssetFile(fullFileName.c_str());
			if(!file.isEof())
			{
				ULONG nWritten = 0;
				(*ppStream)->Write(file.getBuffer(), (ULONG) file.getSize(), &nWritten);
			}
		}
	}
	
	// 
	return hr;
}

#pragma endregion flash_texture_manager 
#endif