#pragma once
#include "EventClasses.h"

namespace ParaEngine
{
	struct Event;

	/**
	* event handler 
	*/
	class CEventHandler : public boost::signals2::trackable
	{
	public:
		/** the callback function, when a given event is fired. 
		* @param event: the input event object to the callback. 
		* @param sScriptCode: this is usually event.ToScriptCode(); It just avoid the callback to call event.ToScriptCode() to recompute script code.
		* @return : same as NPL.activate return value. 
		*/
		virtual int OnEvent(const IEvent* event, const string& sScriptCode);

	private:
		/// a string identifier of the event handler.
		string			m_sID;
		/// the code to be executed when the event is triggered.This is usually a function call in NPL.
		string			m_sFileName;
		/// the code to be executed when the event is triggered.This is usually a function call in NPL.
		string			m_sCode;
		/// event type
		DWORD			m_type;
	public:
		const string& GetCode() const {return m_sCode;};
		const string& GetFileName() const {return m_sFileName;};
		void SetScript(const string& sScript);
		bool Equals(const CEventHandler& e) const {return (m_sID==e.m_sID);}
		bool Equals(const string& sID) const {return (m_sID==sID);}
		inline bool IsMouseEvent() const {return (m_type & (EH_MOUSE|EH_MOUSE_MOVE|EH_MOUSE_DOWN|EH_MOUSE_UP))>0;}
		inline bool IsKeyEvent()const {return (m_type & (EH_KEY | EH_KEY_UP))>0;}
		inline bool IsEditorEvent()const {return (m_type & EH_EDITOR)>0;}
		inline bool IsSystemEvent()const {return (m_type & EH_SYSTEM)>0;}
		inline bool IsNetworkEvent()const {return (m_type & EH_NETWORK)>0;}
		inline bool IsTouchEvent()const { return (m_type & EH_TOUCH)>0; }
		inline bool IsEvent(int nEventType)const {return (m_type & (0x1<<nEventType))>0;}
		/** get the event type. */
		DWORD GetType(){return m_type;};
	public:
		
		CEventHandler(DWORD nType, const string& sID, const string& sScript);
		
		/**
		* @param sID: 
		*   if sID begins with "_m" it is treated as a mouse click event, except that
		*		if sID begins with "_mm" it is treated as a mouse move event.
		*		if sID begins with "_md" it is treated as a mouse down event.
		*		if sID begins with "_mu" it is treated as a mouse up event.
		*   if sID begins with "_k" it is treated as a key down event.
		*   if sID begins with "_ku" it is treated as a key up event.
		*   if sID begins with "_n" it is treated as a networking event.
		*   if sID begins with "_s" it is treated as a system event.
		*   if sID begins with "_e" it is treated as a editor event.
		*/
		CEventHandler(const string& sID, const string& sScript);
		~CEventHandler(void){};
	};
}