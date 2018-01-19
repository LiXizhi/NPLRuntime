//-----------------------------------------------------------------------------
// Class:	Touch sessions
// Authors: LiXizhi
// Contact: lixizhi@yeah.net
// Date:	2015.4.11
// desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "TouchEventSession.h"
#include "TouchGestureBase.h"
#include "TouchSessions.h"

using namespace ParaEngine;

ParaEngine::TouchSessions::TouchSessions()
	:activeGesture(NULL)
{

}

ParaEngine::TouchSessions::~TouchSessions()
{
	CleanupAllGestures();
}

TouchSessions& ParaEngine::TouchSessions::GetInstance()
{
	static TouchSessions s_singleton;
	return s_singleton;
}

void ParaEngine::TouchSessions::CleanupAllGestures()
{
	for (auto gesture : gestures)
	{
		SAFE_RELEASE(gesture);
	}
	gestures.clear();
}

int ParaEngine::TouchSessions::GetSessionCount()
{
	return (int)m_touch_arrays.size();
}

TouchEventSession* ParaEngine::TouchSessions::operator[](int nIndex)
{
	return (nIndex < (int)m_touch_arrays.size()) ? m_touch_arrays[nIndex] : nullptr;
}

TouchEventSession* ParaEngine::TouchSessions::GetTouchSession(int nTouchId)
{
	auto iter = m_touch_sessions.find(nTouchId);
	if (iter != m_touch_sessions.end())
	{
		return iter->second;
	}
	return NULL;
}

void ParaEngine::TouchSessions::AddToTouchSession(const TouchEvent &touch)
{
	if (touch.m_nTouchType == TouchEvent::TouchEvent_POINTER_DOWN)
	{
		if (m_touch_sessions.find(touch.GetTouchId()) == m_touch_sessions.end())
		{
			if (m_touch_sessions.size() > 10)
			{
				TickTouchSessions();
				return;
			}
			TouchEventSession* touchSession = new TouchEventSession(touch);
			m_touch_sessions[touch.GetTouchId()] = touchSession;
			m_touch_arrays.push_back(touchSession);
		}
		else
		{
			// this should never happen
			m_touch_sessions[touch.GetTouchId()]->SetStartEvent(touch);
		}
	}
	else
	{
		TouchEventSession* touchSession = GetTouchSession(touch.GetTouchId());
		if (touchSession)
		{
			touchSession->SetCurrentEvent(touch);
		}
	}
}

void ParaEngine::TouchSessions::ClearTouchSessions()
{
	for (auto iter = m_touch_sessions.begin(); iter != m_touch_sessions.end();)
	{
		TouchEventSession* touchSession = iter->second;
		SAFE_DELETE(touchSession);
	}
	m_touch_sessions.clear();
	m_touch_arrays.clear();
}

void ParaEngine::TouchSessions::TickTouchSessions()
{
	int nCurTime = GetTickCount();
	// expire in 10 seconds.
	const int nTouchExpireTime = 10000;
	for (auto iter = m_touch_sessions.begin(); iter != m_touch_sessions.end();)
	{
		TouchEventSession* touchSession = iter->second;
		if (touchSession && (touchSession->IsEnd() || touchSession->IsExpired(nCurTime, nTouchExpireTime)))
		{
			auto iter1 = std::find(m_touch_arrays.begin(), m_touch_arrays.end(), touchSession);
			if (iter1 != m_touch_arrays.end())
				m_touch_arrays.erase(iter1);
			else
			{
				OUTPUT_LOG("error: failed remove touch session\n");
			}
			SAFE_DELETE(touchSession);
			iter = m_touch_sessions.erase(iter);
		}
		else
			++iter;
	}
}

void ParaEngine::TouchSessions::RegisterGestureRecognizer(CTouchGestureBase* gesture)
{
	if (gesture)
	{
		gesture->addref();
		gestures.push_back(gesture);
	}
}

bool ParaEngine::TouchSessions::InterpreteTouchGestures(const TouchEvent* touch)
{
	if (!touch)
		return false;

	// process active gesture
	if (activeGesture) 
	{
		if (activeGesture->InterpreteTouchGesture(touch))
			return true;
		else
			activeGesture = nullptr;
	}
	// if no active gesture, iterate over all gestures until one is recognized. 
	for (auto gesture : gestures)
	{
		if (gesture->InterpreteTouchGesture(touch))
		{
			activeGesture = gesture;
			return true;
		}
	}
	return false;
}
