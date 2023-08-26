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

	// choose two sessons that are not handled by GUI objects. 
	for (int i = 0; i < touch_sessions->GetSessionCount(); i++)
	{
		auto touch = (*touch_sessions)[i];
		if (!touch->IsHandledByGUI()) {
			nTouchSessionCount++;
			if (nTouchSessionCount == 1)
				touch1 = touch;
			else if (nTouchSessionCount == 2)
				touch2 = touch;
		}
	}

	if (nTouchSessionCount == 2)
	{
		m_distance = TouchEventSession::GetTouchDistanceBetween(&touch1->GetCurrentEvent(), &touch2->GetCurrentEvent());
		if (m_lastDistance < 0) {
			m_lastDistance = m_distance;
		}
		if (touch1->GetMaxDragDistance() < touch1->GetFingerSize() && touch2->GetMaxDragDistance() < touch1->GetFingerSize()) {
			m_isActive = false;
			return false;
		}

		// decide pinch mode
		int deltaDistance = m_distance - m_lastDistance;
		if (deltaDistance > 0)
			m_pinch_mode = PinchMode_Open;
		else
			m_pinch_mode = PinchMode_Closed;
		m_isActive = handleGestureRecognized ? handleGestureRecognized(*this) : false;
		return m_isActive;
	}
	else
	{
		m_isActive = false;
		m_lastDistance = -1;
	}
	return false;
}
