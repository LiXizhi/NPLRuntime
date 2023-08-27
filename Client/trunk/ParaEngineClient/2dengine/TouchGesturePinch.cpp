//-----------------------------------------------------------------------------
// Class:	TouchGesturePinch
// Authors: LiXizhi
// Contact: lixizhi@yeah.net
// Date:	2015.4.11
// desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "TouchSessions.h"
#include "TouchEventSession.h"
#include "TouchGesturePinch.h"

using namespace ParaEngine;

ParaEngine::CTouchGesturePinch::CTouchGesturePinch()
	:m_nPinchThreshold(20), m_pinch_mode(PinchMode_Nil), m_lastDistance(-1), m_distance(-1), m_isActive(false)
{

}


ParaEngine::CTouchGesturePinch::~CTouchGesturePinch()
{

}

void ParaEngine::CTouchGesturePinch::ResetLastDistance()
{
	m_lastDistance = m_distance;
}

int ParaEngine::CTouchGesturePinch::GetDeltaDistance()
{
	return (m_distance >= 0 && m_lastDistance >= 0) ? (m_distance - m_lastDistance) : 0;
}

bool ParaEngine::CTouchGesturePinch::InterpreteTouchGesture(const TouchEvent* touch, TouchSessions* touch_sessions)
{
	if (!touch_sessions)
		touch_sessions = &(TouchSessions::GetInstance());

	TouchEventSession* touch1 = NULL;
	TouchEventSession* touch2 = NULL;
	int nTouchSessionCount = 0;
	const int PinchTag = 1001;
	// choose two sessons that are not handled by GUI objects or dragging 
	int nSceneTouchCount = 0;
	for (int i = 0; i < touch_sessions->GetSessionCount(); i++)
	{
		auto touch = (*touch_sessions)[i];
		if (!touch->IsHandledByGUI()) {
			nSceneTouchCount++;
			if (nSceneTouchCount == 1) {
				touch1 = touch;
			}
			else if (nSceneTouchCount == 2) {
				touch2 = touch;
			}
		}
	}
	if (nSceneTouchCount == 2)
	{
		// tag must be undetermined and two fingers must be started together
		if ( (touch1->GetTag() == PinchTag && touch2->GetTag() == PinchTag) ||
			 ((touch1->GetDuration() - touch2->GetDuration()) < 500 && 
			 touch1->GetTag() <= 0 && touch2->GetTag() <= 0) )
		{
			nTouchSessionCount = 2;
		}
	}

	if (nTouchSessionCount == 2)
	{
		m_distance = TouchEventSession::GetTouchDistanceBetween(&touch1->GetCurrentEvent(), &touch2->GetCurrentEvent());
		if (m_lastDistance < 0) {
			m_lastDistance = m_distance;
		}
		if (!m_isActive && (Math::Abs(GetDeltaDistance()) < (touch1->GetFingerSize() + touch2->GetFingerSize())) ||
			(touch1->GetMaxDragDistance() < touch1->GetFingerSize() && touch2->GetMaxDragDistance() < touch1->GetFingerSize())) {
			m_isActive = false;
		}
		else
		{
			int deltaDistance = m_distance - m_lastDistance;
			m_pinch_mode = (deltaDistance > 0) ? PinchMode_Open : PinchMode_Closed;
			if (handleGestureRecognized ? handleGestureRecognized(*this) : false) {
				m_isActive = true;
				touch1->SetTag(PinchTag); // disable this touch
				touch2->SetTag(PinchTag); // disable this touch
			}
				
		}
		return m_isActive;
	}
	else
	{
		m_isActive = false;
		m_lastDistance = -1;
	}
	return false;
}
