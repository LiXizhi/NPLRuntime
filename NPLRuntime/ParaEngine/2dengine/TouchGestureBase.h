#pragma once

namespace ParaEngine
{
	struct TouchEvent;
	class TouchSessions;

	class CTouchGestureBase : public CRefCounted
	{
	public:
		virtual ~CTouchGestureBase(){};

		virtual bool InterpreteTouchGesture(const TouchEvent* touch, TouchSessions* touch_sessions = NULL) = 0;
	};

}