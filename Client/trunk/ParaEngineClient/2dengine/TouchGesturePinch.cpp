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
	if(!touch_sessions)
		touch_sessions = &(TouchSessions::GetInstance());
	if (touch_sessions->GetSessionCount() == 2)
	{
		TouchEventSession* touch1 = (*touch_sessions)[0];
		TouchEventSession* touch2 = (*touch_sessions)[1];
		if (touch1->IsHandledByGUI() || touch2->IsHandledByGUI() || 
			(touch1->GetMaxDragDistance() < m_nPinchThreshold  && touch2->GetMaxDragDistance() < m_nPinchThreshold)){
			m_isActive = false;
			m_lastDistance = -1;
			return false;
		}
		m_distance = TouchEventSession::GetTouchDistanceBetween(&touch1->GetCurrentEvent(), &touch2->GetCurrentEvent());
		if (m_lastDistance < 0){
			m_isActive = false;
			m_lastDistance = m_distance;
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
