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
#include "GUIMouseVirtual.h"

using namespace ParaEngine;

ParaEngine::CGUIMouseVirtual::CGUIMouseVirtual()
	:m_bLock(false), m_bUseWindowMessage(true), m_isTouchInputting(false), m_bLastMouseReset(false), m_buffered_mouse_msgs_count(0), m_bSwapMouseButton(false), m_x(0), m_y(0), m_objCaptured(NULL)
{
	Reset();
}

ParaEngine::CGUIMouseVirtual::~CGUIMouseVirtual()
{

}

void CGUIMouseVirtual::PushMouseEvent(const DeviceMouseEventPtr& e)
{
	if (m_buffered_mouse_msgs_count < SAMPLE_BUFFER_SIZE / 2)
	{
		// merge all mouse wheel events
		if (e->GetEventType() == EMouseEventType::Wheel)
		{
			auto pEvent = static_cast<const DeviceMouseWheelEvent*>(e.get());
			for (int i = 0; i < m_buffered_mouse_msgs_count; i++)
			{
				if (m_buffered_mouse_msgs[i].message == WM_MOUSEWHEEL)
				{
					m_buffered_mouse_msgs[i].wParam += MAKEWPARAM(0, pEvent->GetWheel());
					return;
				}
			}
			m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].lParam = 0;
			m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].wParam = MAKEWPARAM(0, pEvent->GetWheel());
			m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].message = WM_MOUSEWHEEL;
			++m_buffered_mouse_msgs_count;
		}
		else if (e->GetEventType() == EMouseEventType::Button)
		{
			auto pEvent = static_cast<const DeviceMouseButtonEvent*>(e.get());
			m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].lParam = MAKELPARAM(pEvent->GetX(), pEvent->GetY());
			m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].wParam = 0;
			m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].message = (pEvent->GetButton() == EMouseButton::LEFT) ? (pEvent->GetKeyState() == EKeyState::PRESS ? WM_LBUTTONDOWN : WM_LBUTTONUP) : 
				((pEvent->GetButton() == EMouseButton::RIGHT) ? (pEvent->GetKeyState() == EKeyState::PRESS ? WM_RBUTTONDOWN : WM_RBUTTONUP) : (WM_MOUSEMOVE));
			++m_buffered_mouse_msgs_count;
		}
		else if (e->GetEventType() == EMouseEventType::Move)
		{
			auto pEvent = static_cast<const DeviceMouseMoveEvent*>(e.get());
			m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].lParam = MAKELPARAM(pEvent->GetX(), pEvent->GetY());
			m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].wParam = 0;
			m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].message = WM_MOUSEMOVE;
			++m_buffered_mouse_msgs_count;
		}
	}
	else
	{
		OUTPUT_LOG("PushMouseEvent: failed because buffer is full. msg: %s \n", e->ToString().c_str());
	}
}


void CGUIMouseVirtual::PushMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_buffered_mouse_msgs_count < SAMPLE_BUFFER_SIZE / 2) {
		m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].lParam = lParam;
		m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].wParam = wParam;
		m_buffered_mouse_msgs[m_buffered_mouse_msgs_count].message = uMsg;
	}
	++m_buffered_mouse_msgs_count;
}

void CGUIMouseVirtual::PushMouseEvent(const MSG &msg)
{
	if (m_buffered_mouse_msgs_count < SAMPLE_BUFFER_SIZE / 2) {
		m_buffered_mouse_msgs[m_buffered_mouse_msgs_count] = msg;
	}
	++m_buffered_mouse_msgs_count;
}

bool ParaEngine::CGUIMouseVirtual::IsUseWindowsMessage()
{
	return m_bUseWindowMessage;
}

void ParaEngine::CGUIMouseVirtual::SetUseWindowsMessage(bool bUseWinMsg)
{
	m_bUseWindowMessage = bUseWinMsg;
}

bool ParaEngine::CGUIMouseVirtual::IsLocked()
{
	return m_bLock;
}

void ParaEngine::CGUIMouseVirtual::SetLock(bool bLock)
{
	m_bLock = bLock;
}

bool ParaEngine::CGUIMouseVirtual::IsButtonDown(const EMouseButton nMouseButton)
{
	if (!m_bSwapMouseButton)
	{
		return ((m_curMouseState.rgbButtons[(BYTE)nMouseButton] & 0x80) != 0);
	}
	else
	{
		return ((m_curMouseState.rgbButtons[(1 - (BYTE)nMouseButton)] & 0x80) != 0);
	}
}
void CGUIMouseVirtual::UpdateX(int delta)
{
	if (!IsLocked()) {
		int delta1 = delta;
		m_x += delta1;
	}
}

void CGUIMouseVirtual::UpdateY(int delta)
{
	if (!IsLocked()) {
		int delta1 = delta;
		m_y += delta1;
	}
}
int ParaEngine::CGUIMouseVirtual::GetMouseYDeltaSteps()
{
	return (int)(m_dims2.lY);
}

int ParaEngine::CGUIMouseVirtual::GetMouseXDeltaSteps()
{
	return (int)(m_dims2.lX);
}

int ParaEngine::CGUIMouseVirtual::GetMouseWheelDeltaSteps()
{
	return (int)(m_dims2.lZ / 120);
}

void ParaEngine::CGUIMouseVirtual::Reset()
{
	::memset(&m_dims2, 0, sizeof(m_dims2));
	::memset(&m_curMouseState, 0, sizeof(m_curMouseState));
	::memset(&m_lastMouseState, 0, sizeof(m_lastMouseState));
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
		m_lastMouseState.lX = m_curMouseState.lX;
		m_lastMouseState.lY = m_curMouseState.lY;
		m_dims2.lX = m_dims2.lY = 0;
	}
}

HRESULT ParaEngine::CGUIMouseVirtual::ReadBufferedData()
{
	/** we do not use Read Buffered Data, instead, mouse events are translated from windows messages.
	Only immediate button data are read from direct input. */
	bool bHasMouseMove = false;
	m_dwElements = 0;
	int x = m_curMouseState.lX, y = m_curMouseState.lY;
	//translating windows message into DirextMouse-like events, in order to maintain consistency of the interface
	for (int a = 0; a<m_buffered_mouse_msgs_count; m_dwElements++, a++) {
		int mouse_x = GET_X_LPARAM(m_buffered_mouse_msgs[a].lParam);
		int mouse_y = GET_Y_LPARAM(m_buffered_mouse_msgs[a].lParam);
		
		m_didod[m_dwElements].x = (float)mouse_x;
		m_didod[m_dwElements].y = (float)mouse_y;
		switch (m_buffered_mouse_msgs[a].message) {
		case WM_MOUSEMOVE:
			m_didod[m_dwElements].dwData = mouse_x - x;
			m_didod[m_dwElements].dwOfs = DIMOFS_X;
			m_didod[m_dwElements].dwTimeStamp = m_buffered_mouse_msgs[a].time;
			m_dwElements++;
			m_didod[m_dwElements].dwData = mouse_y - y;
			m_didod[m_dwElements].dwOfs = DIMOFS_Y;
			m_didod[m_dwElements].x = (float)mouse_x;
			m_didod[m_dwElements].y = (float)mouse_y;
			x = mouse_x;
			y = mouse_y;
			m_curMouseState.lX = mouse_x; m_curMouseState.lY = mouse_y;
			bHasMouseMove = true;
			break;
		case WM_MOUSEWHEEL:
			m_didod[m_dwElements].dwData = GET_WHEEL_DELTA_WPARAM(m_buffered_mouse_msgs[a].wParam);
			m_didod[m_dwElements].dwOfs = DIMOFS_Z;
			m_curMouseState.lZ = m_didod[m_dwElements].dwData;
			break;
		case WM_LBUTTONDOWN:
			m_didod[m_dwElements].dwData = 0x80;
			m_didod[m_dwElements].dwOfs = DIMOFS_BUTTON0;
			m_curMouseState.rgbButtons[0] = (BYTE)m_didod[m_dwElements].dwData;
			m_curMouseState.lX = mouse_x; m_curMouseState.lY = mouse_y;
			ResetLastMouseState();
			break;
		case WM_LBUTTONUP:
			m_didod[m_dwElements].dwData = 0;
			m_didod[m_dwElements].dwOfs = DIMOFS_BUTTON0;
			m_curMouseState.rgbButtons[0] = (BYTE)m_didod[m_dwElements].dwData;
			m_curMouseState.lX = mouse_x; m_curMouseState.lY = mouse_y;
			ResetLastMouseState();
			break;
		case WM_RBUTTONDOWN:
			m_didod[m_dwElements].dwData = 0x80;
			m_didod[m_dwElements].dwOfs = DIMOFS_BUTTON1;
			m_curMouseState.rgbButtons[1] = (BYTE)m_didod[m_dwElements].dwData;
			m_curMouseState.lX = mouse_x; m_curMouseState.lY = mouse_y;
			ResetLastMouseState();
			break;
		case WM_RBUTTONUP:
			m_didod[m_dwElements].dwData = 0;
			m_didod[m_dwElements].dwOfs = DIMOFS_BUTTON1;
			m_curMouseState.rgbButtons[1] = (BYTE)m_didod[m_dwElements].dwData;
			m_curMouseState.lX = mouse_x; m_curMouseState.lY = mouse_y;
			ResetLastMouseState();
			break;
		case WM_MBUTTONDOWN:
			m_didod[m_dwElements].dwData = 0x80;
			m_didod[m_dwElements].dwOfs = DIMOFS_BUTTON2;
			m_curMouseState.rgbButtons[2] = (BYTE)m_didod[m_dwElements].dwData;
			m_curMouseState.lX = mouse_x; m_curMouseState.lY = mouse_y;
			ResetLastMouseState();
			break;
		case WM_MBUTTONUP:
			m_didod[m_dwElements].dwData = 0;
			m_didod[m_dwElements].dwOfs = DIMOFS_BUTTON2;
			m_curMouseState.rgbButtons[2] = (BYTE)m_didod[m_dwElements].dwData;
			m_curMouseState.lX = mouse_x; m_curMouseState.lY = mouse_y;
			ResetLastMouseState();
			break;
		}
		m_didod[m_dwElements].dwTimeStamp = m_buffered_mouse_msgs[a].time;
	}
	
	m_buffered_mouse_msgs_count = 0;
	return S_OK;
}

HRESULT ParaEngine::CGUIMouseVirtual::ReadImmediateData()
{
	m_dims2.lX = m_curMouseState.lX - m_lastMouseState.lX;
	m_dims2.lY = m_curMouseState.lY - m_lastMouseState.lY;
	m_dims2.lZ = m_curMouseState.lZ - m_lastMouseState.lZ;

	for (int i = 0; i < 3;++i)
	{
		m_dims2.rgbButtons[i] = m_curMouseState.rgbButtons[i];
	}
	memcpy(&m_lastMouseState, &m_curMouseState, sizeof(m_curMouseState));

	// OUTPUT_LOG("dx %d,dy %d,dz %d: %d %d %d\n", m_dims2.lX, m_dims2.lY, m_dims2.lZ, m_dims2.rgbButtons[0], m_dims2.rgbButtons[1], m_dims2.rgbButtons[2]);
	return S_OK;
}

bool ParaEngine::CGUIMouseVirtual::IsMouseButtonSwapped()
{
	return m_bSwapMouseButton;
}

void ParaEngine::CGUIMouseVirtual::SetMouseButtonSwapped(bool bSwapped)
{
	m_bSwapMouseButton = bSwapped;
}

void ParaEngine::CGUIMouseVirtual::GetDeviceCursorPos(int& x, int&y)
{
	x = m_curMouseState.lX;
	y = m_curMouseState.lY;
}

void ParaEngine::CGUIMouseVirtual::SetDeviceCursorPos(int x, int y)
{
	m_curMouseState.lX = x;
	m_curMouseState.lY = y;
}

void ParaEngine::CGUIMouseVirtual::SetMousePosition(int x, int y)
{
	// m_curMouseState needs to be translated from UI space to device space
	float fScaleX = 1.f, fScaleY = 1.f;
	CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);
	if (x>-500)
		m_curMouseState.lX = (fScaleX == 1.f) ? x : (uint32)(x*fScaleX);
	if (y>-500)
		m_curMouseState.lY = (fScaleY == 1.f) ? y : (uint32)(y*fScaleY);

	m_x = x;
	m_y = y;
}

int ParaEngine::CGUIMouseVirtual::GetBufferedMessageCount()
{
	return m_buffered_mouse_msgs_count;
}

void ParaEngine::CGUIMouseVirtual::ResetLastMouseState()
{
	m_bLastMouseReset = true;
}
