#pragma once
#include "TouchGestureBase.h"

namespace ParaEngine
{
	/** Touch gesture recognizer
	*/
	class CTouchGesturePinch : public CTouchGestureBase
	{
	public:
		CTouchGesturePinch();
		virtual ~CTouchGesturePinch();

		enum PinchMode
		{
			PinchMode_Nil = 0,
			PinchMode_Open,
			PinchMode_Closed,
		};

		typedef std::function<bool(CTouchGesturePinch& pinch_gesture)> GestureRecognized_callback;
	public:
		int GetPinchThreshold() const { return m_nPinchThreshold; }
		void SetPinchThreshold(int val) { m_nPinchThreshold = val; }

		bool IsActive() const { return m_isActive; }
		PinchMode Pinch_mode() const { return m_pinch_mode; }

		void ResetLastDistance();

		int GetDeltaDistance();

		virtual bool InterpreteTouchGesture(const TouchEvent* touch, TouchSessions* touch_sessions = NULL);
	public:
		int m_nPinchThreshold;
		PinchMode m_pinch_mode;
		int m_lastDistance;
		int m_distance;
		bool m_isActive;
		
		/** return true if handled */
		GestureRecognized_callback handleGestureRecognized;
	};
}



