#pragma once
#include "EventClasses.h"
#include "Framework/InputSystem/VirtualKey.h"

namespace ParaEngine
{
	/** it tracks touch events with same id, and expires when a touch is too long. */
	class TouchEventSession
	{
	public:
		TouchEventSession(const TouchEvent& startEvent, bool bSwapped);

		TouchEvent& GetStartEvent();
		TouchEvent& GetPrevEvent();
		TouchEvent& GetCurrentEvent();
		void SetStartEvent(const TouchEvent& event, bool bSwapped);
		void SetCurrentEvent(const TouchEvent& event);
		bool IsExpired(int nCurTime, int nExpireTicks);
		Vector2 GetOffsetFromStartLocation();
		Vector2 GetOffsetFromPrevLocation();
		float GetMaxDragDistance();
		bool IsTouchClick();
		bool IsRightClick();
		bool IsEnd();
		int GetTouchId();
		/** time in ms, from the start touch event to current time. */
		int32 GetDuration();

		int32 GetTag() const;
		void SetTag(int32 val);

		bool IsHandledByGUI();
		void SetHandledByGUI(bool bHandled);

		const Vector2& GetMouseMoveOffset() const;
		void SetMouseMoveOffset(const Vector2& val);

		bool IsSwapped() const;

		EMouseButton TranslateTouchButton(EMouseButton btn) const;

		static int32 GetFingerSize();
		static int32 GetTouchDistanceBetween(TouchEvent* touch1, TouchEvent* touch2);
	public:
		TouchEvent m_start_event;
		TouchEvent m_previous_event;
		TouchEvent m_current_event;
		float m_max_delta;
		int32 m_tag;

		bool m_swapped;
		
	private:
		Vector2 m_vMouseMoveOffset;
	};
}