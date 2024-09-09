//-----------------------------------------------------------------------------
// Class: Event classes
// Authors:	LiXizhi
// Emails:	
// Date: 2014.9.19
// Desc: event classes
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Events_def.h"
#include "EventsCenter.h"
#include "EventClasses.h"

using namespace ParaEngine;


//////////////////////////////////////////////////////////////////////////
//
// event
//
//////////////////////////////////////////////////////////////////////////

std::string Event::ToScriptCode() const
{
	return m_sCode;
}

Event::Event(int nEventType, const char* sCode)
{
	m_nEventType = nEventType;
	m_sCode = sCode;
	m_nEventID = -1;
}
Event::Event(int nEventType, int nID, const char* sCode)
{
	m_nEventType = nEventType;
	m_sCode = sCode;
	m_nEventID = nID;
}


//////////////////////////////////////////////////////////////////////////
//
// mouse event
//
//////////////////////////////////////////////////////////////////////////

MouseEvent::MouseEvent(DWORD MouseState, int x, int y)
	:m_MouseState(MouseState), m_x(x), m_y(y), m_nEventType(EM_MOUSE_CLICK)
{
}


MouseEvent::MouseEvent(DWORD MouseState, int x, int y, int nEventType)
	: m_MouseState(MouseState), m_x(x), m_y(y), m_nEventType(nEventType)
{
}

std::string MouseEvent::ToScriptCode()const
{
	return GenerateOnMouseScript(m_MouseState, m_x, m_y);
};

//////////////////////////////////////////////////////////////////////////
//
// key event
//
//////////////////////////////////////////////////////////////////////////

std::string KeyEvent::ToScriptCode() const
{
	char buf[100];
	snprintf(buf, 100, "virtual_key=%d;", m_nKey);
	return buf;
}

//////////////////////////////////////////////////////////////////////////
//
// editor event
//
//////////////////////////////////////////////////////////////////////////

std::string EditorEvent::ToScriptCode() const
{
	char buf[100];
	snprintf(buf, 100, "event_type=%d;", m_nType);
	return buf;
}

//////////////////////////////////////////////////////////////////////////
//
// system event
//
//////////////////////////////////////////////////////////////////////////

std::string SystemEvent::ToScriptCode() const
{
	char buf[100];
	snprintf(buf, 100, "event_type=%d;", m_nType);
	return m_sCode + buf;
}

//////////////////////////////////////////////////////////////////////////
//
// network event
//
//////////////////////////////////////////////////////////////////////////

std::string NetworkEvent::ToScriptCode()const
{
	char buf[100];
	snprintf(buf, 100, "event_type=%d;", m_nType);
	return m_sCode + buf;
}

//////////////////////////////////////////////////////////////////////////
//
// touch event
//
//////////////////////////////////////////////////////////////////////////

std::string ParaEngine::TouchEvent::ToScriptCode() const
{
	if (!m_sCode.empty())
		return m_sCode;
	else
	{
		char tmp[150];
		std::string sCode;
		snprintf(tmp, sizeof(tmp), "msg={type=\"%s\",x=%d,y=%d,id=%d,time=%d};", GetTouchTypeAsString(m_nTouchType), (int)m_x, (int)m_y, m_touch_id, m_nTime);
		sCode = tmp;
		return sCode;
	}
}

ParaEngine::TouchEvent::TouchEvent(int nType, TouchEventMsgType nTouchType, int touch_id, float x, float y, int nTimeTick)
	: m_nType(nType), m_nTouchType(nTouchType), m_touch_id(touch_id), m_x(x), m_y(y), m_nTime(nTimeTick)
{
}

ParaEngine::TouchEvent::TouchEvent(int nType, const std::string& sCode)
	: m_nType(nType), m_sCode(sCode), m_nTouchType(TouchEvent_POINTER_INVALID), m_touch_id(0), m_x(0), m_y(0)
{

}

const char* ParaEngine::TouchEvent::GetTouchTypeAsString(TouchEventMsgType nTouchType)
{
	static const char* s_touch_type_names[] = {
		"WM_POINTERENTER",
		"WM_POINTERDOWN",
		"WM_POINTERUPDATE",
		"WM_POINTERUP",
		"WM_POINTERLEAVE",
		"TouchEvent_POINTER_INVALID",
	};
	return s_touch_type_names[(int)nTouchType];
}

int ParaEngine::TouchEvent::GetClientPosX() const
{
	return (int)m_x;
}

int ParaEngine::TouchEvent::GetClientPosY() const
{
	return (int)m_y;
}

int ParaEngine::TouchEvent::GetTouchId() const
{
	return m_touch_id;
}

int ParaEngine::TouchEvent::GetTime() const
{
	return m_nTime;
}

bool ParaEngine::TouchEvent::operator==(const TouchEvent& r)
{
	return (m_nTouchType == r.m_nTouchType && m_touch_id == r.m_touch_id);
}

ParaEngine::TouchEvent::TouchEventMsgType ParaEngine::TouchEvent::GetTouchType() const
{
	return m_nTouchType;
}

ParaEngine::AccelerometerEvent::AccelerometerEvent(double x /*= 0*/, double y /*= 0*/, double z /*= 0*/, double timestamp /*= 0*/)
	:m_x(x), m_y(y), m_z(z), m_timestamp(timestamp)
{

}

std::string ParaEngine::AccelerometerEvent::ToScriptCode() const
{
	if (!m_sCode.empty())
		return m_sCode;
	else
	{
		char tmp[150];
		std::string sCode;
		snprintf(tmp, sizeof(tmp), "msg={x=%f,y=%f,z=%f,timestamp=%f};", m_x, m_y, m_z, m_timestamp);
		sCode = tmp;
		return sCode;
	}
}

ParaEngine::MouseMoveEvent::MouseMoveEvent(int x, int y, int dx, int dy, int nEventType)
	:MouseEvent(0, x, y, nEventType), m_dx(dx), m_dy(dy)
{
}

std::string MouseMoveEvent::ToScriptCode()const
{
	return GenerateOnMouseScript(m_MouseState, m_dx, m_dy);
};
