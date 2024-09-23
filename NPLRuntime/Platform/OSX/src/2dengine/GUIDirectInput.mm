//----------------------------------------------------------------------
// Class:	CDirectMouse and CDirectKeyboard
// Authors:	LiXizhi, big
// Date:	2024.9.18
// Desc: 
// Classes encapsulating Direct Input features. We controls the behavior of the 
// keyboard and mouse, gets the state and input of the mouse and keyboard in 
// these class. The user defined mouse cursor is also support here. 
// Refer to reference for further information.
//-----------------------------------------------------------------------
#import <Cocoa/Cocoa.h>
#include "ParaEngine.h"

//#include <strsafe.h>
#include "GUIDirectInput.h"
#include "Globals.h"
#include "ParaWorldAsset.h"
#include "2dengine/EventBinding.h"
#include "2dengine/GUIRoot.h"
#include "util/StringHelper.h"

// #ifdef USE_OPENGL_RENDERER
// 	#include "platform/win32/ParaEngineApp.h"
// #else
// 	#include "ParaEngineApp.h"
// #endif

using namespace ParaEngine;

///////////////////////////////////////////////////////////////////////////////
// 
// CDirectKeyboard
//
///////////////////////////////////////////////////////////////////////////////

CDirectKeyboard::CDirectKeyboard()
//: m_pDI(NULL), m_pKeyboard(NULL)
{
//	memset(m_keystateUserDefined, 0, sizeof(m_keystateUserDefined));
//	CreateDevice(hDlg); 
};

//HRESULT CDirectKeyboard::CreateDevice(HWND hDlg)
//{
//	HRESULT hr;
//	BOOL    bExclusive;
//	BOOL    bForeground;
//	BOOL    bDisableWindowsKey;
//	DWORD   dwCoopFlags;
//
//	// Cleanup any previous call first
//	Free();
//
//	// determine where the buffer would like to be allocated 
//	bExclusive         = false;
//	bForeground        = true;
//	bDisableWindowsKey = false;
//
//	if( bExclusive )
//		dwCoopFlags = DISCL_EXCLUSIVE;
//	else
//		dwCoopFlags = DISCL_NONEXCLUSIVE;
//
//	if( bForeground )
//		dwCoopFlags |= DISCL_FOREGROUND;
//	else
//		dwCoopFlags |= DISCL_BACKGROUND;
//
//	// Disabling the windows key is only allowed only if we are in foreground nonexclusive
//	if( bDisableWindowsKey && !bExclusive && bForeground )
//		dwCoopFlags |= DISCL_NOWINKEY;
//
//	// Create a DInput object
//	if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
//		IID_IDirectInput8, (VOID**)&m_pDI, NULL ) ) )
//		return hr;
//
//	// Obtain an interface to the system keyboard device.
//	if( FAILED( hr = m_pDI->CreateDevice( GUID_SysKeyboard, &m_pKeyboard, NULL ) ) )
//		return hr;
//
//	// Set the data format to "keyboard format" - a predefined data format 
//	//
//	// A data format specifies which controls on a device we
//	// are interested in, and how they should be reported.
//	//
//	// This tells DirectInput that we will be passing an array
//	// of 256 bytes to IDirectInputDevice::GetDeviceState.
//	if( FAILED( hr = m_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) )
//		return hr;
//
//	// Set the cooperative level to let DirectInput know how
//	// this device should interact with the system and with other
//	// DirectInput applications.
//	hr = m_pKeyboard->SetCooperativeLevel(hDlg, dwCoopFlags );
//	if( hr == DIERR_UNSUPPORTED && !bForeground && bExclusive )
//	{
//		Free();
//		MessageBox( hDlg, _T("SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n")
//			_T("For security reasons, background exclusive keyboard\n")
//			_T("access is not allowed."), _T("Keyboard"), MB_OK );
//		return S_OK;
//	}
//	if( FAILED(hr) )
//		return hr;
//
//	// IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
//	//
//	// DirectInput uses unbuffered I/O (buffer size = 0) by default.
//	// If you want to read buffered data, you need to set a nonzero
//	// buffer size.
//	//
//	// Set the buffer size to DINPUT_BUFFERSIZE (defined above) elements.
//	//
//	// The buffer size is a DWORD property associated with the device.
//	DIPROPDWORD dipdw;
//
//	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
//	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
//	dipdw.diph.dwObj        = 0;
//	dipdw.diph.dwHow        = DIPH_DEVICE;
//	dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitrary buffer size
//
//	if( FAILED( hr = m_pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
//		return hr;
//
//	// Acquire the newly created device
//	hr=m_pKeyboard->Acquire();
//
//	ReadImmediateData();
//	// Set a timer to go off 12 times a second, to read input
//	// Note: Typically an application would poll the keyboard
//	//       much faster than this, but this slow rate is simply 
//	//       for the purposes of demonstration
//	return S_OK;
//}

//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
//void CDirectKeyboard::Free()
//{
//	// Unacquire the device one last time just in case 
//	// the app tried to exit while the device is still acquired.
//	if( m_pKeyboard ) 
//		m_pKeyboard->Unacquire();
//
//	// Release any DirectInput objects.
//	SAFE_RELEASE( m_pKeyboard );
//	SAFE_RELEASE( m_pDI );
//}


//HRESULT CDirectKeyboard::ReadBufferedData()
//{
//	if(m_bUseWindowMessage)
//	{
//		CGUIKeyboardVirtual::ReadBufferedData();
//	}
//	else
//	{
//		HRESULT hr = S_OK;
//
//		if( NULL == m_pKeyboard ) 
//			return S_OK;
//
//		m_dwElements = SAMPLE_BUFFER_SIZE;
//		hr =-1;
//		
//		hr = m_pKeyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),m_didod, &m_dwElements, 0 );
//		if( hr != DI_OK ) 
//		{
//			// read buffered events from direct input
//			hr = m_pKeyboard->Acquire();
//			while( hr == DIERR_INPUTLOST ) 
//				hr = m_pKeyboard->Acquire();
//			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
//			// may occur when the app is minimized or in the process of 
//			// switching, so just try again later 
//			m_dwElements=0;
//		}
//	}
//	return S_OK;
//}

//HRESULT CDirectKeyboard::ReadImmediateData(  )
//{
//	HRESULT hr;
//
//	if( NULL == m_pKeyboard ) 
//		return S_OK;
//	
//	memcpy(m_lastkeystate,m_keystate,sizeof(m_lastkeystate));
//	ZeroMemory(m_keystate,sizeof(m_keystate));
//	//save the previous device state
//	hr = m_pKeyboard->GetDeviceState( sizeof(m_keystate), m_keystate );
//	if( FAILED(hr) ) 
//	{
//		// DirectInput may be telling us that the input stream has been
//		// interrupted.  We aren't tracking any state between polls, so
//		// we don't have any special reset that needs to be done.
//		// We just re-acquire and try again.
//
//		// If input is lost then acquire and keep trying 
//		hr = m_pKeyboard->Acquire();
//		while( hr == DIERR_INPUTLOST ) 
//			hr = m_pKeyboard->Acquire();
//
//		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
//		// may occur when the app is minimized or in the process of 
//		// switching, so just try again later 
//		return S_OK; 
//	}
//
//	// update the lock keys' state, using windows api
//	m_keystate[DIK_NUMLOCK]=(m_keystate[DIK_NUMLOCK]&0xf0)|(GetKeyState(VK_NUMLOCK)&0x0f);
//	m_keystate[DIK_SCROLL]=(m_keystate[DIK_SCROLL]&0xf0)|(GetKeyState(VK_SCROLL)&0x0f);
//	m_keystate[DIK_CAPITAL]=(m_keystate[DIK_CAPITAL]&0xf0)|(GetKeyState(VK_CAPITAL)&0x0f);
//	return S_OK;
//}


//bool CDirectKeyboard::IsKeyPressed(const EVirtualKey& nKey)
//{
//	return ((m_keystateUserDefined[(BYTE)nKey] & 0x80) != 0) || CGUIKeyboardVirtual::IsKeyPressed(nKey);
//}

//void CDirectKeyboard::SetKeyPressed(const EVirtualKey& nKey, bool bPressed)
//{
//	m_keystateUserDefined[(BYTE)nKey] = bPressed ? 0x80 : 0;
//	CGUIKeyboardVirtual::SetKeyPressed(nKey, bPressed);
//}

//void CDirectKeyboard::Reset()
//{
//	memset(m_keystateUserDefined, 0, sizeof(m_keystateUserDefined));
//	CGUIKeyboardVirtual::Reset();
//}

//////////////////////////////////////////////////////////////////////////
//
// CDirectMouse
//
//////////////////////////////////////////////////////////////////////////

CDirectMouse::CDirectMouse()
	: m_bUseDirectInput(false)
{
//	m_pDI=NULL;
//	m_pMouse=NULL; 
//	CreateDevice(hDlg);
//	m_bShowCursor=true;
//	m_XHotSpot=0;
//	m_YHotSpot = 0;
//	m_szCursorName = ""; //":IDR_DEFAULT_CURSOR";
//	m_bSwapMouseButton = GetSystemMetrics( SM_SWAPBUTTON )!=0;
};

CDirectMouse::~CDirectMouse()
{
	//m_bExitMouseProcess=true;
	//if (m_bLock)
	//	::ReleaseCapture();


//	Free();
};

void CDirectMouse::Reset()
{
	CGUIMouseVirtual::Reset();

	//if (m_bLock)
	//{
	//	m_bLock = false;
	//	::ReleaseCapture();
	//}
}

//HRESULT CDirectMouse::CreateDevice( HWND hDlg )
//{
//	HRESULT hr;
//	BOOL    bExclusive;
//	BOOL    bForeground;
//	DWORD   dwCoopFlags;
//	 
//	// save window handle
//	m_hwnd = hDlg;
//
//	if (!m_bUseDirectInput)
//		return S_OK;
//
//	// Cleanup any previous call first
//	Free();
//
//	// Detrimine where the buffer would like to be allocated 
//	bExclusive         = false;
//	bForeground        = true;
//
//	if( bExclusive )
//		dwCoopFlags = DISCL_EXCLUSIVE;
//	else
//		dwCoopFlags = DISCL_NONEXCLUSIVE;
//
//	if( bForeground )
//		dwCoopFlags |= DISCL_FOREGROUND;
//	else
//		dwCoopFlags |= DISCL_BACKGROUND;
//
//	// Create a DInput object
//	if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
//		IID_IDirectInput8, (VOID**)&m_pDI, NULL ) ) )
//		return hr;
//
//	// Obtain an interface to the system mouse device.
//	if( FAILED( hr = m_pDI->CreateDevice( GUID_SysMouse, &m_pMouse, NULL ) ) )
//		return hr;
//
//	// Set the data format to "mouse format" - a predefined data format 
//	//
//	// A data format specifies which controls on a device we
//	// are interested in, and how they should be reported.
//	//
//	// This tells DirectInput that we will be passing a
//	// DIMOUSESTATE2 structure to IDirectInputDevice::GetDeviceState.
//	if( FAILED( hr = m_pMouse->SetDataFormat( &c_dfDIMouse2 ) ) )
//		return hr;
//
//	// Set the cooperativity level to let DirectInput know how
//	// this device should interact with the system and with other
//	// DirectInput applications.
//	hr = m_pMouse->SetCooperativeLevel( hDlg, dwCoopFlags );
//	if( hr == DIERR_UNSUPPORTED && !bForeground && bExclusive )
//	{
//		Free();
//		MessageBox( hDlg, _T("SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n")
//			_T("For security reasons, background exclusive mouse\n")
//			_T("access is not allowed."), 
//			_T("Mouse"), MB_OK );
//		return S_OK;
//	}
//
//	if( FAILED(hr) )
//		return hr;
//
//	// IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
//	//
//	// DirectInput uses unbuffered I/O (buffer size = 0) by default.
//	// If you want to read buffered data, you need to set a nonzero
//	// buffer size.
//	//
//	// Set the buffer size to SAMPLE_BUFFER_SIZE (defined above) elements.
//	//
//	// The buffer size is a DWORD property associated with the device.
//	DIPROPDWORD dipdw;
//	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
//	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
//	dipdw.diph.dwObj        = 0;
//	dipdw.diph.dwHow        = DIPH_DEVICE;
//	dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitary buffer size
//
//	if( FAILED( hr = m_pMouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
//		return hr;
//
//	// Acquire the newly created device
//	m_pMouse->Acquire();
//	
//	return S_OK;
//}

//VOID CDirectMouse::Free()
//{
//	// Unacquire the device one last time just in case 
//	// the app tried to exit while the device is still acquired.
//	if( m_pMouse ) 
//		m_pMouse->Unacquire();
//
//	// Release any DirectInput objects.
//	SAFE_RELEASE( m_pMouse );
//	SAFE_RELEASE( m_pDI );
//}

void CDirectMouse::Update()
{
	ReadBufferedData();

	if (m_bLastMouseReset)
	{
        OUTPUT_LOG("update reset!!!!!");
		m_lastMouseState.x = m_curMouseState.x;
		m_lastMouseState.y = m_curMouseState.y;
        OUTPUT_LOG("last x: %d, last y: %d, cur x: %d, cur y: %d\n", m_lastMouseState.x, m_lastMouseState.y, m_curMouseState.x, m_curMouseState.y);
		m_dims2.x = m_dims2.y = 0;
        m_bLastMouseReset = false;
    } else {
        ReadImmediateData();
    }
}

void CDirectMouse::SetMousePosition(int x, int y)
{
    CGUIMouseVirtual::SetMousePosition(x, y);

	if (this->IsLocked() && !m_bUseDirectInput)
	{
		ResetCursorPosition();
    }
}

//HRESULT CDirectMouse::ReadImmediateData( )
//{
//	if (m_isTouchInputting)
//	{
//		CGUIMouseVirtual::ReadImmediateData();
//		return S_OK;
//	}
//
//	HRESULT       hr;
//
//	if (!m_bUseDirectInput)
//	{
//		/** Fix: in teamviewer, vmware, parallel desktop, remote desktop, etc. DirectInput does not give correct mouse cursor position.
//		* so we will use disable direct input by default, unless for full screen mode maybe.
//		*/
//		bool bNeedReset = false;
//
//		POINT pt;
//		if (::GetCursorPos(&pt))
//		{
//			if (this->IsLocked())
//			{
//				RECT rc;
//				::GetWindowRect(m_hwnd, &rc);
//				auto width = rc.right - rc.left;
//				auto height = rc.bottom - rc.top;
//
//				rc.left += width / 8;
//				rc.right -= width / 8;
//				rc.top += height / 8;
//				rc.bottom -= height / 8;
//
//				bNeedReset = !::PtInRect(&rc, pt);
//			}
//
//
//			m_curMouseState.lX = pt.x;
//			m_curMouseState.lY = pt.y;
//		}
//
//		CGUIMouseVirtual::ReadImmediateData();
//		//OUTPUT_LOG("%d %d dx:%d dy:%d\n", m_curMouseState.lX, m_curMouseState.lY, m_dims2.lX, m_dims2.lY);
//
//		m_dims2.rgbButtons[0] = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 0x80 : 0;
//		m_dims2.rgbButtons[1] = (::GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 0x80 : 0;
//		m_dims2.rgbButtons[2] = (::GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 0x80 : 0;
//
//		if (bNeedReset)
//		{
//			ResetCursorPosition();
//		}
//
//		return S_OK;
//	}
//
//
//	if( NULL == m_pMouse ) 
//		return S_OK;
//	memcpy(&m_lastMouseState, &m_curMouseState, sizeof(m_curMouseState));
//	// Get the input's device state, and put the state in dims
//	ZeroMemory( &m_dims2, sizeof(m_dims2) );
//	hr = m_pMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &m_dims2 );
//
////	{
////#ifdef WIN32
////		const float x_radio = 26.0f;
////		const float y_radio = 45.0f;
////
////
////		if (GetSystemMetrics(SM_REMOTESESSION) != 0) //Is Remote Session
////		{
////			m_dims2.lX /= x_radio;
////			m_dims2.lY /= y_radio;
////		}
////#endif
////	}
//
//	if( FAILED(hr) ) 
//	{
//		// DirectInput may be telling us that the input stream has been
//		// interrupted.  We aren't tracking any state between polls, so
//		// we don't have any special reset that needs to be done.
//		// We just re-acquire and try again.
//
//		// If input is lost then acquire and keep trying 
//		hr = m_pMouse->Acquire();
//		while( hr == DIERR_INPUTLOST ) 
//			hr = m_pMouse->Acquire();
//
//		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
//		// may occur when the app is minimized or in the process of 
//		// switching, so just try again later 
//		return S_OK; 
//	}
//	return S_OK;
//}


//-----------------------------------------------------------------------------
// Name: ReadBufferedData()
// Desc: Read the input device's state when in buffered mode and display it.
//-----------------------------------------------------------------------------
bool CDirectMouse::ReadBufferedData()
{
	///** we do not use Read Buffered Data, instead, mouse events are translated from windows messages.
	//Only immediate button data are read from direct input. */
	// bool bHasMouseMove = false;
	m_dwElements = 0;
	int nFirstMouseMoveIndex = -1;
	// int lastX = m_curMouseState.x, lastY = m_curMouseState.y;

    //translating windows message into DirectMouse-like events, in order to maintain consistency of the interface
	for (int a = 0; a < m_buffered_mouse_msgs_count; a++)
	{
		auto &e = m_buffered_mouse_msgs[a];

        switch (e->GetEventType())
		{
			case EMouseEventType::Button:
			{
				DeviceMouseButtonEvent* buttonEvent = (DeviceMouseButtonEvent*)(e.get());
				m_curMouseState.buttons[(int)buttonEvent->GetButton()] = buttonEvent->GetKeyState();
				m_curMouseState.x = buttonEvent->GetX(); m_curMouseState.y = buttonEvent->GetY();
				ResetLastMouseState();

                if (nFirstMouseMoveIndex > 0) {
					// this will disable merging mouse move event, if there is any button event in the middle.
					nFirstMouseMoveIndex = -1;
				}
				break;
			}
			case EMouseEventType::Move:
			{
				const DeviceMouseMoveEvent* moveEvent = (DeviceMouseMoveEvent*)(e.get());
                if (!m_bLastMouseReset) {
                    m_curMouseState.x = moveEvent->GetX();
                    m_curMouseState.y = moveEvent->GetY();
                }
                
                OUTPUT_LOG("1111 cur x: %d, cur y: %d\n", m_curMouseState.x, m_curMouseState.y);
                
//				 m_dims2.x = m_curMouseState.x - lastX;
//				 m_dims2.y = m_curMouseState.y - lastY;
				if (nFirstMouseMoveIndex < 0)
					nFirstMouseMoveIndex = m_dwElements;
			
				// bHasMouseMove = true;
				break;
			}
			case EMouseEventType::Wheel:
			{
				const DeviceMouseWheelEvent* wheelEvent = (DeviceMouseWheelEvent*)(e.get());
				m_curMouseState.z = wheelEvent->GetWheel();
				break;
			}
			case EMouseEventType::Unknown:
			default:
			{
				assert(false);
				break;
			}
		}

		if (e->GetEventType() == EMouseEventType::Move && nFirstMouseMoveIndex>=0 && nFirstMouseMoveIndex != m_dwElements) {
			// merge with previous mouse move event
			m_didod[nFirstMouseMoveIndex] = e;
		} else {
			m_didod[m_dwElements] = e;
			m_dwElements++;
		}
	}

	m_buffered_mouse_msgs_count = 0;
	return S_OK;
}


//HRESULT CDirectMouse::RenderCursor(GUIState* pGUIState, float fElapsedTime)
//{
//	if (m_bShowCursor) 
//	{
//		/*RECT rcWindow;
//		rcWindow.left=m_x+m_pCursor.m_position.rect.left-m_XHotSpot;
//		rcWindow.top=m_y+m_pCursor.m_position.rect.top- m_YHotSpot;;
//		rcWindow.bottom=rcWindow.top+m_pCursor.m_position.GetHeight();
//		rcWindow.right=rcWindow.left+m_pCursor.m_position.GetWidth();
//
//		if(m_pCursor.TextureElement.GetTexture() == NULL)
//		{
//		TextureEntity* pTexture=CGlobals::GetAssetManager()->LoadTexture("", m_szCursorName, TextureEntity::StaticTexture);
//		RECT rcTexture;
//		SetRect( &rcTexture, 0, 0, 32, 32 );
//		SetCursorTexture(pTexture,&rcTexture);
//		}
//		CGUIResource::DrawSprite( &m_pCursor.TextureElement, &rcWindow,0 );*/
//	}
//	return S_OK;
//}

//void CDirectMouse::SetCursorTextA(const char* szText){
//	if( szText )
//		SetCursorText( (const char16_t*)StringHelper::MultiByteToWideChar(szText, DEFAULT_GUI_ENCODING));
//}
//
//void CDirectMouse::SetCursorFont(SpriteFontEntity* pFont, Color defaultFontColor, DWORD dwTextFormat)//default dwTextFormat=
//{
//	//m_pCursor.FontElement.SetElement(pFont,defaultFontColor,dwTextFormat);
//}
//
//void CDirectMouse::SetCursorText(const char16_t* wszText)
//{
//	//m_pCursor.m_szText = wszText;
//}

//const char* ParaEngine::CDirectMouse::GetCursorFile( int * pXHotSpot/*=NULL*/, int* pYHotSpot/*=NULL*/ )
//{
//	if(pXHotSpot!=0)
//	{
//		*pXHotSpot = m_XHotSpot;
//	}
//	if(pYHotSpot!=0)
//	{
//		*pYHotSpot = m_YHotSpot;
//	}
//	return m_szCursorName.c_str();
//}

//const std::string& ParaEngine::CDirectMouse::GetCursorName() const
//{
//	return m_szCursorName;
//}

//void ParaEngine::CDirectMouse::GetDeviceCursorPos(int& x, int&y)
//{
//	if (m_isTouchInputting)
//	{
//		CGUIMouseVirtual::GetDeviceCursorPos(x, y);
//	}
//	else
//	{
//		POINT pos;
//		::GetCursorPos(&pos);
//		x = pos.x;
//		y = pos.y;
//	}
//}

//void ParaEngine::CDirectMouse::SetDeviceCursorPos(int x, int y)
//{
//	CGlobals::GetRenderDevice()->SetCursorPosition(x, y, D3DCURSOR_IMMEDIATE_UPDATE);
//}

//bool ParaEngine::CDirectMouse::IsButtonDown(const EMouseButton nMouseButton)
//{
//	if (m_isTouchInputting)
//		return CGUIMouseVirtual::IsButtonDown(nMouseButton);
//	else
//	{
//		if (!m_bSwapMouseButton)
//		{
//			return ((m_dims2.rgbButtons[(BYTE)nMouseButton] & 0x80) != 0);
//		}
//		else
//		{
//			return ((m_dims2.rgbButtons[(BYTE)(1 - (BYTE)nMouseButton)] & 0x80) != 0);
//		}
//	}
//}

//void CDirectMouse::SetCursorFromFile(const char *szCursor, int XHotSpot, int YHotSpot, bool force)
//{
//#ifdef USE_DIRECTX_RENDERER
//	HRESULT hr = E_FAIL;
//	
//	//OUTPUT_LOG("SetCursorFromFile: %s\r\n", szCursor==0?"none":szCursor);
//
//	if (szCursor==NULL || szCursor[0] == '\0') {
//		szCursor=m_szCursorName.c_str();
//		if (m_szCursorName.size()==0) {
//			return;
//		}
//	}
//	else
//	{
//		if(m_szCursorName == szCursor && ((XHotSpot<0 || m_XHotSpot == XHotSpot) && (YHotSpot<0 || m_YHotSpot == YHotSpot)) && !force)
//		{
//			return;
//		}
//		else
//		{
//			if(XHotSpot>=0)
//				m_XHotSpot = XHotSpot;
//			if(YHotSpot>=0)
//				m_YHotSpot = YHotSpot;
//		}
//	}
//	LPDIRECT3DDEVICE9 pD3dDevice = CGlobals::GetRenderDevice();
//	if(pD3dDevice == NULL)
//		return;
//
//	// cursor file is loaded from memory to increase speed when cursor images changes frequently.
//	string keyName = szCursor;
//	keyName = "cursor:" + keyName;
//
//	TextureEntity* pCursor = (TextureEntity*) CGlobals::GetAssetManager()->GetTextureManager().get(keyName);
//	if(pCursor == 0)
//	{
//		pCursor = CGlobals::GetAssetManager()->LoadTexture(keyName, keyName, TextureEntity::SysMemoryTexture);
//		if(pCursor)
//		{
//			pCursor->SetTextureInfo(TextureEntity::TextureInfo(32, 32, TextureEntity::TextureInfo::FMT_A8R8G8B8, TextureEntity::TextureInfo::TYPE_UNKNOWN));
//			pCursor->SetLocalFileName(szCursor);
//		}
//	}
//
//	if(pCursor)
//	{
//		LPDIRECT3DSURFACE9 pCursorSurface = ((TextureEntityDirectX*)pCursor)->GetSurface();
//		if (pCursorSurface) 
//		{
//			if(SUCCEEDED(hr = pD3dDevice->SetCursorProperties( m_XHotSpot,m_YHotSpot, pCursorSurface))) 
//			{
//				// Set the device cursor
//				m_szCursorName=szCursor;
//
//				if (m_bShowCursor)
//					ForceShowCursor(true);
//			}
//			else
//			{
//				OUTPUT_LOG("Load cursor from file failed\n");
//			}
//		}
//		else
//		{
//			// cursor is not fully loaded yet, simply ignore it. 
//			m_szCursorName="";
//			hr = S_OK;
//		}
//	}
//	
//	if(FAILED(hr))
//	{
//		//OUTPUT_LOG("Failed loading cursor texture\n %s\n we shall load the default cursor instead\n", szCursor);
//		// Set the cursor name as if it is the real cursor
//		m_szCursorName=szCursor;
//	}
//	return;
//#endif
//
//}

//void CDirectMouse::SetCursorTexture( TextureEntity* pTexture, RECT* prcTexture, Color defaultTextureColor)
//{
//}

/** true to show the cursor */
//void CDirectMouse::ShowCursor(bool bShowCursor)
//{
//	if(!CGlobals::GetGUI()->GetUseSystemCursor())
//	{
//		if (m_bShowCursor != bShowCursor) {
//			ForceShowCursor(bShowCursor);
//		}
//		m_bShowCursor = bShowCursor;
//	}
//}

//void CDirectMouse::ForceShowCursor(bool bShow)
//{
//	CGlobals::GetApp()->PostWinThreadMessage(PE_WM_SHOWCURSOR, bShow ? 1 : 0, 0);
//}


void CDirectMouse::ResetCursorPosition()
{
    NSLog(@"from reset cursor position!!!!");
    Vector2 contentResolutionOut;
    CGlobals::GetApp()->GetScreenResolution(&contentResolutionOut);
    
    int contentWidth = contentResolutionOut.x;
    int contentHeight = contentResolutionOut.y;
    
    NSLog(@"contentWidth: %d, contentHeight: %d", contentWidth, contentHeight);
    
//    m_dims2.y = 0;
//    m_dims2.x = 0;
//    m_curMouseState.x = contentWidth / 2;
//    m_curMouseState.y = contentWidth / 2;
//    m_lastMouseState.x = contentWidth / 2;
//    m_lastMouseState.y = contentWidth / 2;
//    m_x = contentWidth / 2;
//    m_y = contentHeight / 2;
    

    m_bLastMouseReset = true;

    // Get the main window of the current application.
    NSWindow *mainWindow = [NSApp mainWindow];

    if (mainWindow) {
        // Get the coordinates and size of the window (the frame of the window)
        NSRect windowFrame = [mainWindow frame];

        // Window position (bottom left corner coordinates).
        CGFloat windowX = windowFrame.origin.x;
        CGFloat windowY = windowFrame.origin.y;

        NSLog(@"windowX: %f, windowsY: %f", windowX, windowY);

        // 获取内容视图的frame（不包括标题栏）
//        NSRect contentFrame = [mainWindow contentLayoutRect];
//        CGFloat contentHeight = contentFrame.size.height;

        NSScreen *screen = [mainWindow screen];
        CGFloat screenHeight = [screen frame].size.height;
        CGFloat screenWidth = [screen frame].size.width;
        
        NSLog(@"screenHeight: %f, screenWidth: %f", screenHeight, screenWidth);

        int y = screenHeight - (windowY + contentHeight) + (contentHeight / 2);
        int x = windowX + (contentWidth / 2);

        NSLog(@"x: %d, y: %d", x, y);

        CGPoint newPosition = CGPointMake(x, y);

        // 使用 CGWarpMouseCursorPosition 设置鼠标位置
        // CGAssociateMouseAndMouseCursorPosition(false);
        CGWarpMouseCursorPosition(newPosition);
        // CGAssociateMouseAndMouseCursorPosition(true);

//        // 可选：生成一个鼠标移动事件（在某些应用场景下可能需要）
//        CGEventRef moveEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, newPosition, kCGMouseButtonLeft);
//        CGEventPost(kCGHIDEventTap, moveEvent);
//        CFRelease(moveEvent);
    } else {
        NSLog(@"No main window found.");
    }

//    m_dims2.y = 0;
//    m_dims2.x = 0;
    int realContentWidth = CGlobals::GetApp()->GetRenderWindow()->GetWidth();
    int realContentHeight = CGlobals::GetApp()->GetRenderWindow()->GetHeight();
    m_curMouseState.x = realContentWidth / 2;
    m_curMouseState.y = realContentHeight / 2;
//    m_lastMouseState.x = contentWidth / 2;
//    m_lastMouseState.y = contentWidth / 2;
//    m_x = contentWidth / 2;
//    m_y = contentHeight / 2;
//    

    //m_bLastMouseReset = false;
}

void CDirectMouse::SetLock(bool bLock)
{
//	static POINT s_last_lock_pos = {0, 0};

	if (bLock && !m_bLock) {
		// CGlobals::GetApp()->PostWinThreadMessage(PE_WM_SETCAPTURE, 0, 0);
		ShowCursor(false);
//		::GetCursorPos(&s_last_lock_pos);
		ResetCursorPosition();

//		RECT rc;
//		::GetWindowRect(m_hwnd, &rc);
//		::ClipCursor(&rc);
	}

	if (!bLock && m_bLock) {
		// CGlobals::GetApp()->PostWinThreadMessage(PE_WM_RELEASECAPTURE, 0, 0);
		ShowCursor(true);
//		::ClipCursor(NULL);
//		::SetCursorPos(s_last_lock_pos.x, s_last_lock_pos.y);
	}

	m_bLock = bLock;
}
