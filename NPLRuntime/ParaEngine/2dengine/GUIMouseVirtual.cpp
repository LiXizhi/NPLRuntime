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
                    int delta = (static_cast<const DeviceMouseWheelEvent*>(e.get()))->GetWheel();
                    delta += (static_cast<const DeviceMouseWheelEvent*>(m_buffered_mouse_msgs[i].get()))->GetWheel();
                    m_buffered_mouse_msgs[i] = DeviceMouseEventPtr(new DeviceMouseWheelEvent(delta));
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
	// m_curMouseState needs to be translated from UI space to device space
	 float fScaleX = 1.f, fScaleY = 1.f;
	 CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);

    if (x>-500)
		m_curMouseState.x = (fScaleX == 1.f) ? x : (int32)(x*fScaleX);
	if (y>-500)
		m_curMouseState.y = (fScaleY == 1.f) ? y : (int32)(y*fScaleY);

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
