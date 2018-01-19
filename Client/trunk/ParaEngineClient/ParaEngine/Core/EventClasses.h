#pragma once
#include "IEvent.h"
namespace ParaEngine
{
	class TouchEventSession;

	/** a general event */
	struct Event : public IEvent
	{
		int m_nEventType;
		int m_nEventID;
		string m_sCode;

		/** get event type */
		virtual int GetEventType() const { return m_nEventType; }
		/** build script code and return. it does not cache the script code. Use GetScriptCode() instead for performance reason. */
		virtual string ToScriptCode() const;

		/** get event id */
		virtual int GetEventID() const { return m_nEventID; }
	public:
		Event(int nEventType, const char* sCode);
		Event(int nEventType, int nID, const char* sCode);
	};

	/** simple mouse event struct*/
	struct MouseEvent : public IEvent
	{
		DWORD m_MouseState;
		int m_x;
		int m_y;
		/** EM_MOUSE_MOVE or EM_MOUSE_CLICK, EM_MOUSE_DOWN, EM_MOUSE_UP, etc.*/
		int m_nEventType;

	public:
		MouseEvent(DWORD MouseState, int x, int y);
		MouseEvent(DWORD MouseState, int x, int y, int nEventType);
		/** get event type */
		virtual int GetEventType()  const { return m_nEventType; }
		string ToScriptCode()const;
	};

	/** simple key events struct*/
	struct KeyEvent : public IEvent
	{
		DWORD m_KeyState;
		int m_nKey;
		int m_nEventType;

	public:
		KeyEvent(DWORD KeyState, int nKey, int nEventType = EVENT_KEY) :m_KeyState(KeyState), m_nKey(nKey), m_nEventType(nEventType) {};
		/** get event type */
		virtual int GetEventType()  const { return m_nEventType; }
		string ToScriptCode()const;
	};

	/** system events struct*/
	struct SystemEvent : public IEvent
	{
		// value of SystemEventType
		int m_nType;
		// the string code
		string m_sCode;
		bool m_bIsAsyncMode;
		enum SystemEventType
		{
			// this is fired when the same application receives a message from the command line.
			// event_type=0 and msg=command line
			SYS_COMMANDLINE = 0,
			// the user requests to close the application, such as clicking the x or alt-f4
			SYS_WM_CLOSE = 1,
			// a file drop event whenever user drags from windows explorer to this window. 
			SYS_WM_DROPFILES = 2,
			// window is being destroyed, do final clean up and save states. 
			SYS_WM_DESTROY = 3,
			// when system settings changed. such as the slate mode is changed, mostly for intel 2in1 pad. 
			SYS_WM_SETTINGCHANGE = 4,
		};
	public:
		SystemEvent(int nType, const string& sCode) :m_nType(nType), m_sCode(sCode), m_bIsAsyncMode(true){};
		/** get event type */
		virtual int GetEventType()  const { return EVENT_SYSTEM; }
		string ToScriptCode()const;
		/** return true, if firing event does not immediately invoke the handler. */
		virtual bool IsAsyncMode() { return m_bIsAsyncMode; }
		/** set whether firing event does not immediately invoke the handler.
		* caution: almost all event should be fired in async mode. Sync Mode event is only used internally by a few system messages like the WM_CLOSED message.
		*/
		void SetAsyncMode(bool bAsync){ m_bIsAsyncMode = bAsync; }
	};

	/** world editor events, such as scene selection, etc.*/
	struct EditorEvent : public IEvent
	{
	public:
		int m_nType;
		EditorEvent(int nType) :m_nType(nType){};
		/** get event type */
		virtual int GetEventType()  const { return EVENT_EDITOR; }
		string ToScriptCode()const;
	};

	/** network event */
	struct NetworkEvent : public IEvent
	{
	public:
		// see NPLGlobals::NPL_PACKET_TYPE
		int m_nType;
		string m_sCode;
		NetworkEvent(int nType, const string& sCode) :m_nType(nType), m_sCode(sCode){};

		/** get event type */
		virtual int GetEventType()  const { return EVENT_NETWORK; }
		string ToScriptCode()const;
	};

	/** touch event */
	struct TouchEvent : public IEvent
	{
	public:
		enum TouchEventMsgType{
			TouchEvent_POINTER_ENTER = 0,
			TouchEvent_POINTER_DOWN,
			TouchEvent_POINTER_UPDATE,
			TouchEvent_POINTER_UP,
			TouchEvent_POINTER_LEAVE,
			TouchEvent_POINTER_INVALID,
		};
		// see NPLGlobals::NPL_PACKET_TYPE
		int m_nType;
		std::string m_sCode;
		TouchEventMsgType m_nTouchType;
		
		int m_touch_id;
		float m_x;
		float m_y;
		// in ms seconds ticks
		int m_nTime;
				
		TouchEvent(int nType, const string& sCode);
		TouchEvent(int nType = EH_TOUCH, TouchEventMsgType nTouchType = TouchEvent_POINTER_INVALID, int touch_id = 0, float x = 0.f, float y = 0.f, int nTimeTick = 0);
		
		int GetClientPosX() const;
		int GetClientPosY() const;
		int GetTouchId() const;
		int GetTime() const;
		TouchEventMsgType GetTouchType() const;

		bool operator==(const TouchEvent& r);

		static const char* GetTouchTypeAsString(TouchEventMsgType nTouchType);
		/** get event type */
		virtual int GetEventType()  const { return EVENT_TOUCH; }
		std::string ToScriptCode() const;
	};

	/** the total 3d vector of force that is currently on the device, including gravity. 
	* we can use this to decide current roll and pitch of the device(usually mobile phone) in its own reference frame, 
	* and it can also detect sudden movement like shaking. 
	* Some device provides other sensors like eletro-magnetometer and gyroscope, which provides additional info to device motion. 
	*/
	struct AccelerometerEvent : public IEvent
	{
	public:
		// see NPLGlobals::NPL_PACKET_TYPE
		int m_nType;
		std::string m_sCode;

		AccelerometerEvent(double x = 0, double y = 0, double z = 0, double timestamp = 0);
		
		/** get event type */
		virtual int GetEventType()  const { return EVENT_ACCELEROMETER; }

		std::string ToScriptCode() const;

	public:
		double m_x;
		double m_y;
		double m_z;
		// current time in nano seconds when the event is received. 
		double m_timestamp;
	};
}