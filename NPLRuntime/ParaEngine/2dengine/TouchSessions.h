#pragma once

namespace ParaEngine
{
	class TouchEventSession;
	class CTouchGestureBase;
	struct TouchEvent;

	/** manages all global touch sessions.
	*/
	class TouchSessions
	{
	public:
		TouchSessions();
		~TouchSessions();

		/** singleton object */
		static TouchSessions& GetInstance();

		int GetSessionCount();

		/** get touch session by index. */
		TouchEventSession* operator [] (int nIndex);

		/** get touch session by touch id. */
		TouchEventSession* GetTouchSession(int nTouchId);

		void AddToTouchSession(const TouchEvent &touch);

		void ClearTouchSessions();

		void TickTouchSessions();

		/** return true if at least one gesture is recognized. 
		@param touch: the most recent touch event
		@return true if active gesture is recognized. 
		*/
		bool InterpreteTouchGestures(const TouchEvent* touch);

		void RegisterGestureRecognizer(CTouchGestureBase* gesture);

		void CleanupAllGestures();
	public:
		/** mapping from touch id to touch session. */
		std::map<int, TouchEventSession*> m_touch_sessions;
		std::vector<TouchEventSession*> m_touch_arrays;
		/** current active gesture */
		CTouchGestureBase* activeGesture;
		/** all registered gestures */
		std::vector<CTouchGestureBase*> gestures;
	};
}

