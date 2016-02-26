#pragma once
#include "Events_def.h"
#include <string>
#include <bitset>
#include <set>
#include <map>
#include <boost/array.hpp>
#include "util/cow_ptr.hpp"

#define EventCount 512
using namespace std;

#ifndef IS_KEYDOWN
#define IS_KEYDOWN(x) ((0x80&(x))!=0)
#endif

#ifndef IS_KEYUP
#define IS_KEYUP(x) ((0x80&(x))==0)
#endif

#ifndef WIN32
typedef struct tagMSG {
	HWND        hwnd;
	UINT        message;
	WPARAM      wParam;
	LPARAM      lParam;
	DWORD       time;
	POINT       pt;

} MSG;
#endif
namespace ParaEngine
{	
	struct StructEventMappingTableItem {
		int data;
		StructEventMappingTableItem* next;
		StructEventMappingTableItem(){
			next=NULL;data=EM_NONE;
		}
	};

	class SimpleScript{
	public:
		string szFile;
		string szCode;
	};
	/**
	 * This class is for maintaining the event binding table and script binding
	 * table. It also contains method to translate string to event and also backwards
	 **/
	class CEventBinding:public IObject
	{
	public:
		CEventBinding();
		~CEventBinding();
		CEventBinding(const CEventBinding&);
//		CEventBinding(const &)
		static void StaticInit();
		//initialize the event mapping table, the old mappings are erased.
		void InitEventMappingTable(bool bDisable=false);

		//enable/disable an event
		void EnableEvent(int eEvent);
		void DisableEvent(int eEvent);
		/**
		* Mapping an event to another event.
		*/
		void MapEvent(int eSrcEvent, int eDestEvent);
		/**
		* Unmap an event to any event 
		*/
		void UnmapEvent(int eSrcEvent, int eDestEvent);
		/**
		* Decide if the first event is mapped to the second event
		*/
		bool IsMapTo(int eSrcEvent, int eDestEvent)const;

		//maps the default relation between the visible characters.
		void DefaultMap_Text();

		//maps the default relation between the control keys
		void DefaultMap_Control();

		//maps the default relation between mouse events
		void DefaultMap_Mouse();

		//disable all the keyboard events
		void DisableKeyboard();
		//enable all the keyboard events
		void EnableKeyboard();

		//disable all the mouse events
		void DisableMouse();
		//enable all the mouse events
		void EnableMouse();
		
		//create an event's script, if it already has a script, the old one will be replace by the new one
		void MapEventToScript(int eEvent,const SimpleScript *script);

		//delete an event's script, if any
		void UnmapEventToScript(int eEvent);
		
		//return the script of a given event, null if not exists
		const SimpleScript *GetEventScript(int eEvent)const;

		//if an event has script,
		bool HasEventScript(int eEvent)const ;

		CEventBinding & operator =(CEventBinding &input);


		//inherits from IObject
		virtual void Clone(IObject* pobj)const;
		virtual IObject* Clone()const;
		virtual int Release();
		virtual bool Equals(const IObject *obj)const;

		//two helper functions for translating between string and the event value.
		static int StringToEventValue(const string &str);
		static string& EventValueToString(int value);

		//helper function for initializing a message
		static void InitMsg(MSG *event,DWORD time,DWORD message,POINT& pt);

		/** converting a windows virtual key to direct input key scan code. */
		static DWORD TranslateVKToDIK(DWORD dwVKey);

		static DWORD ScancodeToKeyTable[256];

		/** windows virtual key code to direct input key. */
		static DWORD WinVirtualKeyToDIK[256];
	protected:
		typedef std::bitset<EventCount> EventEnabledArray_type;
		typedef std::map<int,SimpleScript> EventScriptMap_type;
		typedef boost::array< set<int>, EventCount > EventMapping_type;
		typedef cow_ptr< boost::array< set<int>, EventCount > > EventMapping_cow_type; // copy on write

		EventEnabledArray_type m_pEventEnable;
		EventScriptMap_type m_pEventToScript;
		
		EventMapping_cow_type m_pEventMappingTable;
		static map<string,int> StringToEventTable;
		static map<int,string> EventToStringTable;
	};

	// since most event binding is stored as copy on write pointer. 
	typedef cow_ptr<CEventBinding> EventBinding_cow_type;
}
