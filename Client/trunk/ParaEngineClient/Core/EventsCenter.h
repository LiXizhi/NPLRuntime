#pragma once
#include "EventHandler.h"
#include "util/mutex.h"
#include <list>
#include <vector>

namespace ParaEngine
{
	//class CEventHandler;
	using namespace std;
	
	/**
	* a global pool for user registered custom events. 
	* see events page for information about events parameters in scripting interface.
	* events are categorized by types, see PE_types.h, events_def.h. There are also some predefined events in Predefinedevents.h.
	
	Global ParaEngine event handlers are exposed through scripting interface, API interface, and managed API interface. 
	Managed API interface are exposed as multi-cast delegate in the EventCenter class of ParaEngineManaged class.

	Two types of event firing are supported:
		(1) Fire event immediately, similar to SendMessage in Win32 
		(2) Pose Event, which return immediate without executing the event handlers. similar to PostMessages in Win32
	Since event raisers and event handlers may be in the same thread, PoseEvent is there to prevent interlocking of program code. 
	
	Example:  SelectionToolWindow in ParaIDE are implemented through the new managed event handling interface. Whenever the game engine changes selection, 
	the plug-in interface will receive an event, and in the event handler, it will update the GUI to reflect the recent changes in selection.  
	*/
	class CEventsCenter
	{
	public:
		CEventsCenter(void);
		~CEventsCenter(void);
		/**
		* @return the singleton instance.
		*/
		static CEventsCenter* GetInstance();
	public:
		/** remove all pending event from queue and clear all event listener */
		void Reset();
		
		/** register a mouse or key event handler 
		* @param sID: a string identifier of the event handler.
		*   if sID begins with "_m" it is treated as a mouse click event, except that
		*		if sID begins with "_mm" it is treated as a mouse move event.
		*		if sID begins with "_md" it is treated as a mouse down event.
		*		if sID begins with "_mu" it is treated as a mouse up event.
		*   if sID begins with "_k" it is treated as a key down event.
		*   if sID begins with "_ku" it is treated as a key up event.
		*   if sID begins with "_n" it is treated as a network event handler.
		* @param sScript: the script to be executed when the event is triggered.This is usually a function call in NPL.
		*	sScript should be in the following format "{NPL filename};{sCode};". this is the same format in the UI event handler
		*/
		void RegisterEvent(const string& sID, const string& sScript);
		/**
		* same as above RegisterEvent(), except that it allows caller to explicitly specify the event type, instead of deriving it from the event name.
		* @param nEventType any bit combination of EventHandler_type
		* @param sID any unique string identifier
		* @param sScript the NPL script. 
		*/
		void RegisterEvent(DWORD nEventType, const string& sID, const string& sScript);

		/** add a given event handler. 
		* @param pEventHandler: must be created using the default new operator. The caller does not need to delete it. 
		*	The event center will have it deleted automatically. 
		*/
		void AddEventHandler(CEventHandler* pEventHandler);

		/** unregister a mouse or key event handler */
		void UnregisterEvent(const string& sID);
		/** unregister all mouse or key event handler */
		void UnregisterAllEvent();

		/**
		* Fire mouse events, call all of its handler scripts with sCode immediately. 
		* this function will return when all events have been handled. use PoseEvents(), if one wants to return without calling their handlers;
		* [Not thread safe]: this function must be called within the main thread. 
		* @note: default script event handler only insert a message to the NPL message queue and return immediately, 
		*	so the actual handling of events will be delayed actually. 
		* @param e event parameters
		* @return number of handlers called.
		*/
		int FireEvent(const IEvent& e);

		/**
		* Post a event to the unhandled event pool, and return immediately. Events in unhandled event pool are handled during the next frame move. 
		* [Thread Safe]: this function is thread safe, since it will queue all events and call them in the main thread in the next frame move. 
		* @param e : the input event
		* @param bUnique: if true, the event id is unique in the event pool, unless the ID is -1. If one tries to 
		*  insert the event with the same id multiple times within a frame move, only the first one will be fired during each frame move. 
		* @return 
		*/
		bool PostEvent(const Event& e, bool bUnique = true);

		/**
		* fire all events in the unhandled events pool. This function is called automatically during each frame move. 
		*/
		void FireAllUnhandledEvents();
	private:

		/**
		* Get Event Handler By ID
		* @param sID ID
		* @return NULL if not found.
		*/
		CEventHandler* GetEventHandlerByID(string sID);
		void InvalidateEventCounts();
	private:
		typedef std::vector<Event> EventHandler_Pool_t;
		typedef std::vector<CEventHandler*> EventHandler_List_t;

		/// all supported event types that callers can register to. 
		EventHandler_Callback_t		m_events[EVENT_LAST];

		EventHandler_List_t		m_sEventHandlerList;
		EventHandler_Pool_t  m_unhandledEventPool;
		/// maximum number of events which can be in the pool, default value is 50
		int m_nMaxPoolSize;
		/// for statistics
		int m_eventCounts[EVENT_LAST];
		/// mutex for post event pool
		ParaEngine::mutex	m_mutex;
	};

	/**
	* the string format is given below: 
	* mouse_button="{left|middle|right|}";mouse_x=X;mouse_y=Y;
	* @param MouseState 
	* @param X 
	* @param Y 
	* @return 
	*/
	extern string GenerateOnMouseScript(int MouseState, int X, int Y);
	/**
	* Not implemented 
	* @param option 
	* @return 
	*/
	extern string GenerateOnKeyboardScript(int option);
}

