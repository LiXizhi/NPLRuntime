//----------------------------------------------------------------------
// Class:	Touch event session
// Authors:	LiXizhi
// Date:	2014.9.29
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "TouchEventSession.h"

using namespace ParaEngine;

// default to 10 pixels
const float default_finger_size = 10.f;

ParaEngine::TouchEventSession::TouchEventSession(const TouchEvent& startEvent)
	:m_max_delta(0.f), m_tag(0)
{
	SetStartEvent(startEvent);
}

TouchEvent& ParaEngine::TouchEventSession::GetStartEvent()
{
	return m_start_event;
}

TouchEvent& ParaEngine::TouchEventSession::GetPrevEvent()
{
	return m_previous_event;
}

TouchEvent& ParaEngine::TouchEventSession::GetCurrentEvent()
{
	return m_current_event;
}

void ParaEngine::TouchEventSession::SetStartEvent(const TouchEvent& event)
{
	m_start_event = event;
	m_previous_event = event;
	m_current_event = event;
	m_max_delta = 0.f;
	//OUTPUT_LOG("start touch session %d\n", GetTouchId());
}

bool ParaEngine::TouchEventSession::IsExpired(int nCurTime, int nExpireTicks)
{
	return ((nCurTime - GetCurrentEvent().GetTime()) >= nExpireTicks);
}

void ParaEngine::TouchEventSession::SetCurrentEvent(const TouchEvent& event)
{
	m_previous_event = m_current_event;
	m_current_event = event;
	if (event.GetTouchType() == TouchEvent::TouchEvent_POINTER_UPDATE)
	{
		float delta_x = (m_current_event.m_x - m_start_event.m_x);
		float delta_y = (m_current_event.m_y - m_start_event.m_y);
		if (delta_x!= 0.f)
			m_max_delta = Math::Max(m_max_delta, Math::Abs(delta_x));
		if (delta_y!= 0.f)
			m_max_delta = Math::Max(m_max_delta, Math::Abs(delta_y));
		//OUTPUT_LOG("update %d: max delta: %f\n", GetTouchId(), m_max_delta);
	}
}

int ParaEngine::TouchEventSession::GetTouchId()
{
	return GetStartEvent().GetTouchId();
}

bool ParaEngine::TouchEventSession::IsEnd()
{
	return (GetCurrentEvent().GetTouchType() == TouchEvent::TouchEvent_POINTER_UP);
}

float ParaEngine::TouchEventSession::GetMaxDragDistance()
{
	return m_max_delta;
}

ParaEngine::Vector2 ParaEngine::TouchEventSession::GetOffsetFromStartLocation()
{
	return Vector2(m_current_event.m_x - m_start_event.m_x, m_current_event.m_y - m_start_event.m_y);
}

ParaEngine::Vector2 ParaEngine::TouchEventSession::GetOffsetFromPrevLocation()
{
	return Vector2(m_current_event.m_x - m_previous_event.m_x, m_current_event.m_y - m_previous_event.m_y);
}

bool ParaEngine::TouchEventSession::IsTouchClick()
{
	return ((m_current_event.m_nTime - m_start_event.m_nTime) < 300 && (GetMaxDragDistance() < default_finger_size));
}

int32 ParaEngine::TouchEventSession::GetTag() const
{
	return m_tag;
}

void ParaEngine::TouchEventSession::SetTag(int32 val)
{
	m_tag = val;
}

bool ParaEngine::TouchEventSession::IsRightClick()
{
	return ((m_current_event.m_nTime - m_start_event.m_nTime) > 300 && (GetMaxDragDistance() < default_finger_size));
}

int32 ParaEngine::TouchEventSession::GetFingerSize()
{
	return (int32)default_finger_size;
}

int32 ParaEngine::TouchEventSession::GetDuration()
{
	return (m_current_event.m_nTime - m_start_event.m_nTime);
}

int32 ParaEngine::TouchEventSession::GetTouchDistanceBetween(TouchEvent* touch1, TouchEvent* touch2)
{
	if (touch1 && touch2)
	{
		int distSq = (int)(Math::Sqr(touch1->m_x - touch2->m_x) + Math::Sqr(touch1->m_y - touch2->m_y));
		if (distSq >= 1)
			return (int32)Math::Sqrt((double)distSq);
		else
			return 0;
	}
	return 0;
}






