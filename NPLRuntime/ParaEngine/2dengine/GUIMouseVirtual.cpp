//-----------------------------------------------------------------------------
// Class: GUI mouse base class
// Authors:	LiXizhi
// Company:	ParaEngine
// Date: 2015.2.1
// Desc:
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "EventBinding.h"
#include "IParaEngineApp.h"
#include "GUIRoot.h"
#include "ParaImage.h"
#include "util/StringHelper.h"
#include "GUIMouseVirtual.h"

using namespace ParaEngine;

ParaEngine::CGUIMouseVirtual::CGUIMouseVirtual()
	:m_bLock(false),
    m_bUseWindowMessage(true),
    m_isTouchInputting(false),
    m_bLastMouseReset(false),
    m_buffered_mouse_msgs_count(0),
    m_bSwapMouseButton(false),
    m_x(0),
    m_y(0),
    m_objCaptured(NULL),
    m_dwElements(0),
	m_bHasSimulatedMouseEvent(false)
{
	Reset();
}

ParaEngine::CGUIMouseVirtual::~CGUIMouseVirtual()
{
#if defined(WIN32) && !defined(NPLRUNTIME_SERVER)
	for (auto& kv : m_cursorCache)
	{
		if (kv.second.first && kv.second.second)
		{
			::DestroyCursor(kv.second.first);
		}
	}
	m_cursorCache.clear();
	m_hCurrentCursor = NULL;
#endif
}

void CGUIMouseVirtual::PushMouseEvent(const DeviceMouseEventPtr &e)
{
	if (m_buffered_mouse_msgs_count < SAMPLE_BUFFER_SIZE / 2)
    {
        // merge all mouse wheel events
        if(e->GetEventType() == EMouseEventType::Wheel)
        {
            for (int i=0;i<m_buffered_mouse_msgs_count; i++)
            {
                if(m_buffered_mouse_msgs[i]->GetEventType() == EMouseEventType::Wheel)
                {
                    const auto* wheel = static_cast<const DeviceMouseWheelEvent*>(e.get());
                    const auto* buffered = static_cast<const DeviceMouseWheelEvent*>(m_buffered_mouse_msgs[i].get());
                    if (buffered->IsHorizontal() != wheel->IsHorizontal() || buffered->IsPrecise() != wheel->IsPrecise())
                        continue;
                    int delta = wheel->GetWheel() + buffered->GetWheel();
                    m_buffered_mouse_msgs[i] = DeviceMouseEventPtr(new DeviceMouseWheelEvent(delta, wheel->IsHorizontal(), wheel->IsPrecise()));
                    return;
                }
            }
        }

		m_buffered_mouse_msgs[m_buffered_mouse_msgs_count] = e;
		++m_buffered_mouse_msgs_count;
	}
	else
	{
		OUTPUT_LOG("PushMouseEvent: failed because buffer is full. msg: %s \n", e->ToString().c_str());
	}
}

bool ParaEngine::CGUIMouseVirtual::IsLocked()
{
	return m_bLock;
}

void ParaEngine::CGUIMouseVirtual::SetLock(bool bLock)
{
#if defined(WIN32) && !defined(NPLRUNTIME_SERVER)
	static POINT s_last_lock_pos = { 0, 0 };
	HWND hWnd = CGlobals::GetAppHWND();
	if (hWnd && ::IsWindow(hWnd))
	{
		if (bLock && !m_bLock)
		{
			ShowCursor(false);
			::GetCursorPos(&s_last_lock_pos);
			RECT rc = { 0, 0, 0, 0 };
			if (::GetClientRect(hWnd, &rc))
			{
				POINT ptTL = { rc.left, rc.top };
				POINT ptBR = { rc.right, rc.bottom };
				::ClientToScreen(hWnd, &ptTL);
				::ClientToScreen(hWnd, &ptBR);
				RECT rcScreen = { ptTL.x, ptTL.y, ptBR.x, ptBR.y };
				::SetCursorPos(ptTL.x + (ptBR.x - ptTL.x) / 2, ptTL.y + (ptBR.y - ptTL.y) / 2);
				::ClipCursor(&rcScreen);
			}
		}
		else if (!bLock && m_bLock)
		{
			ShowCursor(true);
			::ClipCursor(NULL);
			::SetCursorPos(s_last_lock_pos.x, s_last_lock_pos.y);
		}
	}
#endif
	m_bLock = bLock;
}

bool ParaEngine::CGUIMouseVirtual::IsButtonDown(const EMouseButton button)
{
	if (m_bSwapMouseButton)
	{
		if (button == EMouseButton::LEFT) {
			return m_curMouseState.buttons[(int)EMouseButton::RIGHT] == EKeyState::PRESS; 
		} else if(button == EMouseButton::RIGHT) {
			return m_curMouseState.buttons[(int)EMouseButton::LEFT] == EKeyState::PRESS;
		}
	}

	return m_curMouseState.buttons[(int)button] == EKeyState::PRESS;
}

int ParaEngine::CGUIMouseVirtual::GetMouseYDeltaSteps()
{
    return (int)(m_dims2.y);
}

int ParaEngine::CGUIMouseVirtual::GetMouseXDeltaSteps()
{
    return (int)(m_dims2.x);
}

int ParaEngine::CGUIMouseVirtual::GetMouseWheelDeltaSteps()
{
	return (int)(m_dims2.z / 120);
}

void ParaEngine::CGUIMouseVirtual::Reset()
{
	m_dwElements = 0;
}

CGUIBase* ParaEngine::CGUIMouseVirtual::GetCapture()
{
	return m_objCaptured;
}

void CGUIMouseVirtual::SetCapture(CGUIBase* obj)
{
	CGUIRoot::GetInstance()->EnableClipCursor(true);
	m_objCaptured = obj;
}

void CGUIMouseVirtual::ReleaseCapture(CGUIBase* obj)
{
	if (m_objCaptured == obj) {
		CGUIRoot::GetInstance()->EnableClipCursor(false);
		m_objCaptured = NULL;
	}
}

void CGUIMouseVirtual::ReleaseCapture()
{
	if (m_objCaptured)
	{
		CGUIRoot::GetInstance()->EnableClipCursor(false);
	}
	m_objCaptured = NULL;
}

void ParaEngine::CGUIMouseVirtual::Update()
{
	ReadImmediateData();
	ReadBufferedData();

	if (m_bLastMouseReset)
	{
		m_bLastMouseReset = false;
		m_lastMouseState.x = m_curMouseState.x;
		m_lastMouseState.y = m_curMouseState.y;
		m_dims2.x = m_dims2.y = 0;
	}
}

bool ParaEngine::CGUIMouseVirtual::ReadBufferedData()
{
	///** we do not use Read Buffered Data, instead, mouse events are translated from windows messages.
	//Only immediate button data are read from direct input. */
	bool bHasMouseMove = false;
	m_dwElements = 0;
	int nFirstMouseMoveIndex = -1;
	int lastX = m_curMouseState.x, lastY = m_curMouseState.y;

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
				m_curMouseState.x = moveEvent->GetX();
				m_curMouseState.y = moveEvent->GetY();
				m_dims2.x = m_curMouseState.x - lastX;
				m_dims2.y = m_curMouseState.y - lastY;
				if (nFirstMouseMoveIndex < 0)
					nFirstMouseMoveIndex = m_dwElements;
			
				bHasMouseMove = true;
				break;
			}
			case EMouseEventType::Wheel:
			{
				const DeviceMouseWheelEvent* wheelEvent = (DeviceMouseWheelEvent*)(e.get());
				if (!wheelEvent->IsHorizontal())
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

		if (e->GetEventType() == EMouseEventType::Move && nFirstMouseMoveIndex >= 0 && nFirstMouseMoveIndex != m_dwElements) {
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

bool ParaEngine::CGUIMouseVirtual::ReadImmediateData()
{
	m_dims2.x = m_curMouseState.x - m_lastMouseState.x;
	m_dims2.y = m_curMouseState.y - m_lastMouseState.y;
	m_dims2.z = m_curMouseState.z - m_lastMouseState.z;

	for (int i = 0; i < 3;++i)
	{
		m_dims2.buttons[i] = m_curMouseState.buttons[i];
	}

	memcpy(&m_lastMouseState, &m_curMouseState, sizeof(m_curMouseState));

	return true;
}

bool ParaEngine::CGUIMouseVirtual::IsMouseButtonSwapped()
{
	return m_bSwapMouseButton;
}

void ParaEngine::CGUIMouseVirtual::SetMouseButtonSwapped(bool bSwapped)
{
	if (m_bSwapMouseButton != bSwapped)
	{
		m_bSwapMouseButton = bSwapped;
	}
}

void ParaEngine::CGUIMouseVirtual::GetDeviceCursorPos(int& x, int&y)
{
	x = m_curMouseState.x;
	y = m_curMouseState.y;
}

void ParaEngine::CGUIMouseVirtual::SetDeviceCursorPos(int x, int y)
{
	m_curMouseState.x = x;
	m_curMouseState.y = y;
}

void ParaEngine::CGUIMouseVirtual::SetMousePosition(int x, int y)
{
	// if(HasSimulatedMouseEvent())
	// {
	// 	// m_curMouseState needs to be translated from UI space to device space
	// 	float fScaleX = 1.f, fScaleY = 1.f;
	// 	CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);

	// 	if (x > -500)
	// 		m_curMouseState.x = (fScaleX == 1.f) ? x : (int32)(x * fScaleX);
	// 	if (y > -500)
	// 		m_curMouseState.y = (fScaleY == 1.f) ? y : (int32)(y * fScaleY);
	// }
	
	m_x = x;
	m_y = y;
}

int ParaEngine::CGUIMouseVirtual::GetBufferedMessageCount()
{
	return m_buffered_mouse_msgs_count;
}

bool CGUIMouseVirtual::HasSimulatedMouseEvent() const {
	return m_bHasSimulatedMouseEvent;
}

void CGUIMouseVirtual::SetHasSimulatedMouseEvent(bool value)
{
	m_bHasSimulatedMouseEvent = value;
}

void ParaEngine::CGUIMouseVirtual::ResetLastMouseState()
{
	m_bLastMouseReset = true;
}

void ParaEngine::CGUIMouseVirtual::ShowCursor(bool bShowCursor)
{
	if (!CGlobals::GetGUI()->GetUseSystemCursor())
	{
		if (m_bShowCursor != bShowCursor)
		{
			m_bShowCursor = bShowCursor;
			ForceShowCursor(bShowCursor);
		}
	}
	else
	{
		m_bShowCursor = bShowCursor;
	}
}

void ParaEngine::CGUIMouseVirtual::ForceShowCursor(bool bShow)
{
	m_bShowCursor = bShow;
#if defined(WIN32) && !defined(NPLRUNTIME_SERVER)
	HWND hWnd = CGlobals::GetAppHWND();
	if (hWnd && ::IsWindow(hWnd) && ::GetWindowThreadProcessId(hWnd, NULL) != ::GetCurrentThreadId())
	{
		CGlobals::GetApp()->PostWinThreadMessage(PE_WM_SHOWCURSOR, bShow ? 1 : 0, 0);
		return;
	}
	ApplySystemCursor();
#endif
}

void ParaEngine::CGUIMouseVirtual::SetCursorFromFile(const char* szCursor, int XHotSpot, int YHotSpot, bool force)
{
	if (szCursor == NULL || szCursor[0] == '\0')
	{
		if (m_szCursorName.empty())
			return;
		szCursor = m_szCursorName.c_str();
	}
	else
	{
		if (m_szCursorName == szCursor
			&& (XHotSpot < 0 || m_XHotSpot == XHotSpot)
			&& (YHotSpot < 0 || m_YHotSpot == YHotSpot)
			&& !force)
		{
			return;
		}
		if (XHotSpot >= 0) m_XHotSpot = XHotSpot;
		if (YHotSpot >= 0) m_YHotSpot = YHotSpot;
	}

	std::string cursorPath = szCursor;
	m_szCursorName = cursorPath;

#if defined(WIN32) && !defined(NPLRUNTIME_SERVER)
	char keyBuf[512];
	snprintf(keyBuf, sizeof(keyBuf), "%s|%d|%d", cursorPath.c_str(), m_XHotSpot, m_YHotSpot);
	std::string cacheKey = keyBuf;

	if (!force)
	{
		auto it = m_cursorCache.find(cacheKey);
		if (it != m_cursorCache.end())
		{
			m_hCurrentCursor = it->second.first;
			if (m_bShowCursor)
				ApplySystemCursor();
			return;
		}
	}
	else
	{
		auto it = m_cursorCache.find(cacheKey);
		if (it != m_cursorCache.end())
		{
			if (it->second.first && it->second.second)
				::DestroyCursor(it->second.first);
			m_cursorCache.erase(it);
		}
	}

	HCURSOR hLoaded = NULL;
	bool bOwned = false;

	// 1. Native .cur / .ani on disk
	std::string ext = CParaFile::GetFileExtension(cursorPath);
	StringHelper::make_lower(ext);
	if ((ext == "cur" || ext == "ani") && CParaFile::DoesFileExist2(cursorPath.c_str(), FILE_ON_DISK))
	{
		std::wstring wPath = StringHelper::MultiByteToWideChar(cursorPath.c_str(), DEFAULT_FILE_ENCODING);
		hLoaded = ::LoadCursorFromFileW(wPath.c_str());
	}

	// 2. Decode image via CParaFile + ParaImage (supports .tga, .png, .dds, .jpg in disk/pkg/embedded)
	if (!hLoaded)
	{
		CParaFile file(cursorPath.c_str());
		if (!file.isEof() && file.getSize() > 0)
		{
			ParaImage img;
			if (img.initWithImageData(reinterpret_cast<const unsigned char*>(file.getBuffer()), static_cast<size_t>(file.getSize())))
			{
				int w = static_cast<int>(img.getWidth());
				int h = static_cast<int>(img.getHeight());
				if (w > 0 && h > 0)
				{
					unsigned char* rgbaData = nullptr;
					size_t rgbaLen = 0;
					const unsigned char* srcPixels = img.getData();
					bool freeRgba = false;
					if (img.getRenderFormat() != PixelFormat::A8R8G8B8)
					{
						ParaImage::convertDataToFormat(img.getData(), static_cast<size_t>(img.getDataLen()), img.getRenderFormat(), PixelFormat::A8R8G8B8, &rgbaData, &rgbaLen);
						srcPixels = rgbaData;
						freeRgba = (rgbaData != nullptr);
					}

					if (srcPixels)
					{
						BITMAPV5HEADER bi = {};
						bi.bV5Size = sizeof(BITMAPV5HEADER);
						bi.bV5Width = w;
						bi.bV5Height = -h; // top-down
						bi.bV5Planes = 1;
						bi.bV5BitCount = 32;
						bi.bV5Compression = BI_BITFIELDS;
						bi.bV5RedMask = 0x00FF0000;
						bi.bV5GreenMask = 0x0000FF00;
						bi.bV5BlueMask = 0x000000FF;
						bi.bV5AlphaMask = 0xFF000000;

						void* lpBits = nullptr;
						HDC hdc = ::GetDC(NULL);
						HBITMAP hColorBmp = ::CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS, &lpBits, NULL, 0);
						::ReleaseDC(NULL, hdc);

						if (hColorBmp && lpBits)
						{
							uint8_t* dst = static_cast<uint8_t*>(lpBits);
							const size_t pixelCount = static_cast<size_t>(w) * static_cast<size_t>(h);
							for (size_t i = 0; i < pixelCount; ++i)
							{
								dst[i * 4 + 0] = srcPixels[i * 4 + 2]; // B
								dst[i * 4 + 1] = srcPixels[i * 4 + 1]; // G
								dst[i * 4 + 2] = srcPixels[i * 4 + 0]; // R
								dst[i * 4 + 3] = srcPixels[i * 4 + 3]; // A
							}

							HBITMAP hMaskBmp = ::CreateBitmap(w, h, 1, 1, NULL);
							ICONINFO ii = {};
							ii.fIcon = FALSE;
							ii.xHotspot = static_cast<DWORD>(m_XHotSpot >= 0 ? m_XHotSpot : 0);
							ii.yHotspot = static_cast<DWORD>(m_YHotSpot >= 0 ? m_YHotSpot : 0);
							ii.hbmMask = hMaskBmp;
							ii.hbmColor = hColorBmp;
							hLoaded = reinterpret_cast<HCURSOR>(::CreateIconIndirect(&ii));
							if (hLoaded)
								bOwned = true;
							if (hMaskBmp)
								::DeleteObject(hMaskBmp);
						}
						if (hColorBmp)
							::DeleteObject(hColorBmp);
					}

					if (freeRgba)
						free(rgbaData);
				}
			}
		}
	}

	if (!hLoaded)
	{
		hLoaded = ::LoadCursor(NULL, IDC_ARROW);
		bOwned = false;
	}

	m_cursorCache[cacheKey] = std::make_pair(hLoaded, bOwned);
	m_hCurrentCursor = hLoaded;
	if (m_bShowCursor)
		ApplySystemCursor();
#endif
}

#if defined(WIN32) && !defined(NPLRUNTIME_SERVER)
bool ParaEngine::CGUIMouseVirtual::ApplySystemCursor()
{
	if (!m_bShowCursor)
	{
		::SetCursor(NULL);
		return true;
	}
	if (CGlobals::GetGUI() && CGlobals::GetGUI()->GetUseSystemCursor())
	{
		return false;
	}
	if (m_hCurrentCursor)
	{
		::SetCursor(m_hCurrentCursor);
		return true;
	}
	return false;
}
#endif
