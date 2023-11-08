#pragma once
using namespace std;

#include "EventBinding.h"
// decide how far is the control dragged away from the mouse pressed location when the dragging operation starts, in pixels
#define EVENT_DRAG_DISTANCE 2

// the maximum between two successive clicks that can be interpreted as double click, in milliseconds
#define EVENT_DOUBLECLICK_TIME 300

// this should be equal to the one in guidirectinput.h
#define SAMPLE_BUFFER_SIZE 1024


/// some marco for key interpretation
#define ShiftPressed (((CGUIEvent::KeyStates[EM_KEY_LSHIFT]&0x80)!=0)||((CGUIEvent::KeyStates[EM_KEY_RSHIFT]&0x80)!=0) || ((CGUIEvent::KeyStates[EM_KEY_SHIFT]&0x80)!=0))
#define ControlPressed (((CGUIEvent::KeyStates[EM_KEY_LCONTROL]&0x80)!=0)||((CGUIEvent::KeyStates[EM_KEY_RCONTROL]&0x80)!=0) || ((CGUIEvent::KeyStates[EM_KEY_CONTROL]&0x80)!=0))
#define AltPressed (((CGUIEvent::KeyStates[EM_KEY_LALT]&0x80)!=0)||((CGUIEvent::KeyStates[EM_KEY_RALT]&0x80)!=0) || ((CGUIEvent::KeyStates[EM_KEY_ALT]&0x80)!=0))
#define CapsLockPressed ((CGUIEvent::KeyStates[EM_KEY_CAPSLOCK]&0x01)!=0)
#define NumLockPressed ((CGUIEvent::KeyStates[EM_KEY_NUMLOCK]&0x01)!=0)

#include <map>
#include <list>
#include <vector>
#include <bitset>
using namespace std;
#define RetainLeftState(x) ((x)&0x7)
#define RetainRightState(x) (((x)&0x38)>>3)
#define RetainMiddleState(x) (((x)&0x1c0)>>6)
#define RetainDragState(x) (((x)&0xe00)>>9)
#define RetainKeyState(x) (((x)&0x3000)>>12)
namespace ParaEngine
{
	class CEventBinding;
	class CGUIKeyboardVirtual;
	class CGUIMouseVirtual;
	class CGUIBase;
	class IObjectDrag;

	/**
	Place of the 
	*/
	class CGUIEvent:public IObject
	{
	public:
		enum GUI_EVENT_TYPE{
			NONE=0,
			SIMULATE=1,//if the event is use to simulate user input such as moving the mouse or type something
			MOUSE=2,//any kind of mouse event
			KEYBOARD=4,//any kind of keyboard event
/*			MOVE_RESIZE=8,//move or resize a control, the parameter is its new rectangle.
			CREATE=16,//creates a new control
			SET_RESOURCE=32,//set or change a resource
			PROCEDURE_CALL=64,//calls member function of a control, something like remote 
							  //procedure call, not implemented in current version
			RESERVED1=128,//three reserved input event type for future development
			RESERVED2=256,
			RESERVED3=512,*/
			WINDOWS=1048576//other windows event
		};
		/**new
		* 00000000000xxx: Left
		* 00000000xxx000: Right
		* 00000xxx000000: Middle
		* 00xxx000000000: Drag
		* xx000000000000: Key
		*/
		enum GUI_EVENT_STATE{
			GUIESNone=0,
			GUIESLeftNone=1,
			GUIESLeftDown=2,
			GUIESLeftClick=3,
			GUIESLeftDBClick=4,
			GUIESLeftMask=7,//00000000000111
			GUIESRightNone=8,
			GUIESRightDown=16,
			GUIESRightClick=24,
			GUIESRightDBClick=32,
			GUIESRightMask=56,//00000000111000
			GUIESMiddleNone=64,
			GUIESMiddleDown=128,
			GUIESMiddleClick=192,
			GUIESMiddleDBClick=256,
			GUIESMiddleMask=448,//00000111000000
			GUIESDragBegin=512,
			GUIESDragOver=1024,
			GUIESDragEnd=1536,
			GUIESDragMask=3584
		};
		enum GUI_KEY_OPTION{
			DEFAULT=0,
			REPEAT_PRESSED_KEYS=1,
			IGNORE_UP_EVENTS=2,
			IGNORE_DOWN_EVENTS=4
		};
		//this struct can be memcpy
		struct GUI_SIMPLE_EVENT_MOUSE{
		public:
			short x,y;
			DWORD nTime;
			void init(short x, short y, DWORD time)
			{
				this->x=x;this->y=y;this->nTime=time;
			}
			void init()
			{
				memset(this,0,sizeof(GUI_SIMPLE_EVENT_MOUSE));
			}
		};
		//can be memcpy
		struct GUI_EVENT_MOUSE {
			public:
			short x,y;
			short WheelDelta;//if the mouse wheel scrolls, it is the the number of detents the mouse wheel has rotated.
			int HoverDuration;//how long the mouse is on the control, set by InterpretEvent()
			GUI_SIMPLE_EVENT_MOUSE LastLDown,LastRDown,LastMDown;//store the last click 
			void init()
			{
				x=y=0;
				HoverDuration=0;
				WheelDelta=0;
				LastMDown.init();
				LastRDown.init();
				LastLDown.init();
			}
		};
		//can be memcpy
		struct GUI_KEYBOARD_HOLDKEY{
			DWORD key;
			DWORD state;
			DWORD lasttime;
		};
		//can not be memcpy
		struct GUI_EVENT_KEYBOARD {
		public:
			DIDEVICEOBJECTDATA KeyEvents[SAMPLE_BUFFER_SIZE];

			DWORD Size;//size of KeyEvents
			//the alternative key for the current trigger event.
			//This is the translation of the original key code with the control keys.
			//For example, NUMPAD0, if NUMLOCK is off, the nAlterKey will be INSERT key.
			int nAlterKey;
			std::vector <GUI_KEYBOARD_HOLDKEY> HoldKey;
			void init()
			{
				//memset(KeyStates,0,sizeof(KeyStates));
				memset(KeyEvents,0,sizeof(KeyEvents[0])*SAMPLE_BUFFER_SIZE);
				Size=0;
				HoldKey.clear();
			}
		};//keyboard event represents the current state of the keyboard, 
		
		GUI_EVENT_MOUSE m_mouse;

		GUI_EVENT_KEYBOARD m_keyboard;

		///the time when the event happen, get using GetTickCount()
		int m_nTime;
		
//		bool m_bLeftdown,m_bRightdown,m_bMiddledown;
		CGUIEvent();
		~CGUIEvent();

		//new
		bool IsMapTo(int eSrcEvent, int eDestEvent);
		/**
		* Set the CEventBinding object of this event. 
		* This function will delete the current CEventBinding object of this event and
		* copies the new CEventBinding object using the Clone() method. So the caller should 
		* release the CEventBinding object it passed to this function.
		**/
// 		void SetEventBinding(const Policy::CopyOnWriteHolder<CEventBinding> *eventbinding);
		int GetTriggerEvent()const{return m_nTriggerEvent;}
		bool InterpretMessage(MSG *msg,int option=0);
		CEventBinding *GetEventBindingObj(){return m_eventbinding.get_ptr();}
		const CEventBinding *GetConstEventBindingObj(){return m_eventbinding.c_ptr();}

		bool IsMousePressed(int mouse);

		/**
		gets a string of buffered char sequence
		*/
		string GetCharSequence();

		/**
		Interprets a char with reference to an input key state.
		@param key: The key needs to be interpret.
		@return Return the interpret character. If the key is not a valid character (such as control keys), it will return a '\0'.
		*/
		static CHAR GetChar(DWORD key);


		/// tests if the given key is pressed in the current key event
		bool IsKeyPressed(int key);

		void SetBinding(IObjectDrag *obj);

		/**
		* Add a holding key to the keyboard
		*/
		void AddHoldKey(DWORD key,DWORD nTime);

		void Initialize();

		void UpdateKey(int option);

		virtual int Release();
		virtual void Clone(IObject* obj)const;
		virtual IObject* Clone()const;
		virtual bool Equals(const IObject *obj)const;

		void ResetState(){m_eState=GUIESNone;/*m_bLeftdown=false;m_bRightdown=false;m_bMiddledown=false;*/}
		MSG GenerateMessage();

		EventBinding_cow_type m_eventbinding;
	protected:
		int m_nTriggerEvent;
		short m_eState;//GUI_EVENT_STATE
		IObjectDrag *m_binding;
	//static members
	public:
		//this is a static member to temporarily store the keyboard state, 
		//all mapping are in the EM_XX index
		static unsigned char KeyStates[EventCount];
		static void StaticInit();
	protected:
		static int m_nDragBeginDistance;
		static int m_nDBClickInterval;

	};
}
