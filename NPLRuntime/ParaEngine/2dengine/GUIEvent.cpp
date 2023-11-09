//----------------------------------------------------------------------
// Class:	CGUIEvent
// Authors:	Liu Weili
// Date:	2005.8.1
// Revised: 2005.8.1
//
// desc: 
// We wish to handle the default windows message on a constant rate(the same as the frame 
// rate of the game), so we do not waste time in handling unnecessary or duplicate messages.
// This will increase consistency of the controls and provides a batch handling process to 
// increse efficiency. We also wish to handle messages from the game engine, which is very similar
// to the default windows message and will suffer the same problems as stated above. 
//
// CGUIEvent is the basic element of such interaction. It provides interpretation of a windows
// message or a game engine script. The results will be a unified format that the 2D GUI engine
// can recognize. 
// 
// We use standard windows mouse and keyboard messages in the current version. In future development,
// we will change the input method completely to DirectInput. 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "GUIEvent.h"
#include "Globals.h"
#include "GUIRoot.h"
#include "MiscEntity.h"
#include "EventBinding.h"
#include "InfoCenter/ICConfigManager.h"
#include "ObjectManager.h"
#include "IObjectDrag.h"
#include "memdebug.h"
using namespace std;
using namespace ParaEngine;
#ifndef WM_MOUSEWHEEL  
#define WM_MOUSEWHEEL                   0x020A
#endif


//////////////////////////////////////////////////////////////////////////
//CGUIEvent
//////////////////////////////////////////////////////////////////////////
int CGUIEvent::m_nDragBeginDistance=EVENT_DRAG_DISTANCE;
int CGUIEvent::m_nDBClickInterval=EVENT_DOUBLECLICK_TIME;
unsigned char CGUIEvent::KeyStates[EventCount];
CGUIEvent::CGUIEvent()
	:m_eventbinding(EventBinding_cow_type::eNo)
{
	m_mouse.init();
	m_keyboard.init();

	m_nTime=GetTickCount();
	m_binding=NULL;
	m_eState=GUIESNone;
	m_nTriggerEvent=EM_NONE;
// 	m_bLeftdown=false;
// 	m_bRightdown=false;
// 	m_bMiddledown=false;
};

CGUIEvent::~CGUIEvent()
{
}

int CGUIEvent::Release()
{
	delete this;
	return 0;
}

/*
 * We only copy the eventbinding object here
 */
void CGUIEvent::Clone(IObject* obj)const
{
	PE_ASSERT(obj!=NULL);
	if (obj==NULL) {
		return ;
	}
	CGUIEvent *pEvent=(CGUIEvent*)obj;
	pEvent->m_binding=m_binding;
	pEvent->m_eventbinding=m_eventbinding;
}

IObject* CGUIEvent::Clone()const
{
	CGUIEvent *pEvent=new CGUIEvent();
	Clone(pEvent);
	return pEvent;
}
bool CGUIEvent::Equals(const IObject *obj)const
{
	return obj==this;
}

void CGUIEvent::SetBinding(IObjectDrag *obj)
{
	m_binding=obj;
}
void CGUIEvent::Initialize()
{
	m_eState=EM_NONE;
}
void CGUIEvent::StaticInit()
{
	using namespace ParaInfoCenter;
	CICConfigManager *pCm=CGlobals::GetICConfigManager();
	HRESULT hr= pCm->GetIntValue("Drag_begin_distance",&m_nDragBeginDistance);
	hr= pCm->GetIntValue("DBClick_interval",&m_nDBClickInterval);
// 	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
// 	CGUIEvent* pEvent=new CGUIEvent();
// 	if (!pOm->IsExist("default_CEventBinding")) {
// 		CEventBinding::StaticInit();
// 	}
// 	CEventBinding* pBinding=(CEventBinding*)pOm->CloneObject("default_CEventBinding");
// 	pEvent->SetEventBinding(pBinding);
// 	pOm->SetObject("default_CGUIEvent",pEvent);
// 	SAFE_RELEASE(pEvent);
// 	SAFE_RELEASE(pBinding);

}

bool CGUIEvent::IsMapTo(int eSrcEvent, int eDestEvent)
{
	return m_eventbinding.c_ptr()->IsMapTo(eSrcEvent,eDestEvent);
}

MSG CGUIEvent::GenerateMessage()
{
	MSG newMsg;
	newMsg.hwnd=0;
	newMsg.pt.x=m_mouse.x;
	newMsg.pt.y=m_mouse.y;
	newMsg.time=m_nTime;
	newMsg.lParam=m_mouse.WheelDelta;
	newMsg.wParam=m_keyboard.nAlterKey;
	newMsg.message=m_nTriggerEvent;
	return newMsg;
}

bool CGUIEvent::InterpretMessage(MSG *msg,int option)
{
	if (m_binding==NULL||msg==NULL) {
		//something goes wrong
		return false;
	}
	m_mouse.WheelDelta=(short)msg->lParam;
	m_nTime=msg->time;
	if (m_eState==GUIESNone) {
		m_eState|=GUIESLeftNone;
		m_eState|=GUIESRightNone;
		m_eState|=GUIESMiddleNone;
	}

	m_nTriggerEvent=msg->message;
	if (IsMapTo(msg->message,EM_MOUSE)) {
		m_mouse.x=(short)msg->pt.x;m_mouse.y=(short)msg->pt.y;
		// during dragging operation
		if((m_eState&GUIESDragMask) != 0)
		{
			if ( ! IsMousePressed((int)EMouseButton::LEFT) )
			{
				
				m_nTriggerEvent=EM_MOUSE_LEFTDRAGEND;
				m_eState=(m_eState&~GUIESDragMask)|GUIESLeftNone;
				m_mouse.LastLDown.init();
				//				m_bLeftdown=false;
				return true;
			}
			switch(m_eState&GUIESDragMask) {
			case GUIESDragBegin:
				if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
					m_nTriggerEvent=EM_MOUSE_DRAGOVER;
					m_eState=(m_eState&~GUIESDragMask)|GUIESDragOver;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_LEFTDOWN)) {
					//				m_bLeftdown=true;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_LEFTUP)) {
					m_nTriggerEvent=EM_MOUSE_LEFTDRAGEND;
					m_eState=(m_eState&~GUIESDragMask)|GUIESLeftNone;
					m_mouse.LastLDown.init();
					//				m_bLeftdown=false;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTDOWN)) {
					//				m_bRightdown=true;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTUP)) {
					m_nTriggerEvent=EM_MOUSE_RIGHTDRAGEND;
					m_eState=(m_eState&~GUIESDragMask)|GUIESRightNone;
					m_mouse.LastRDown.init();
					//				m_bRightdown=false;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEDOWN)) {
					//				m_bMiddledown=true;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEUP)) {
					m_nTriggerEvent=EM_MOUSE_MIDDLEDRAGEND;
					m_eState=(m_eState&~GUIESDragMask)|GUIESMiddleNone;
					m_mouse.LastMDown.init();
					//				m_bMiddledown=false;
					return true;
				}
				break;
			case GUIESDragOver:
				if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
					m_nTriggerEvent=EM_MOUSE_DRAGOVER;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_LEFTDOWN)) {
					//				m_bLeftdown=true;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_LEFTUP)) {
					m_nTriggerEvent=EM_MOUSE_LEFTDRAGEND;
					m_eState=(m_eState&~GUIESDragMask)|GUIESLeftNone;
					m_mouse.LastLDown.init();
					//				m_bLeftdown=false;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTDOWN)) {
					//				m_bRightdown=true;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTUP)) {
					m_nTriggerEvent=EM_MOUSE_RIGHTDRAGEND;
					m_eState=(m_eState&~GUIESDragMask)|GUIESRightNone;
					m_mouse.LastRDown.init();
					//				m_bRightdown=false;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEDOWN)) {
					//				m_bMiddledown=true;
					return true;
				}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEUP)) {
					m_nTriggerEvent=EM_MOUSE_MIDDLEDRAGEND;
					m_eState=(m_eState&~GUIESDragMask)|GUIESMiddleNone;
					m_mouse.LastMDown.init();
					//				m_bMiddledown=false;
					return true;
				}
				break;
			}
		}

		// during one of the left button operation. 
		switch(m_eState&GUIESLeftMask) {
		case GUIESLeftNone:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				m_nTriggerEvent=EM_MOUSE_MOVE;
			}else if (IsMapTo(msg->message,EM_MOUSE_LEFTDOWN)) {
				m_eState=(m_eState&~GUIESLeftMask)|GUIESLeftDown;
				m_mouse.LastLDown.init((short)msg->pt.x,(short)msg->pt.y,msg->time);
				m_nTriggerEvent=EM_MOUSE_LEFTDOWN;
				if(m_binding->GetCandrag())
				{
					IObjectDrag::DraggingObject.SetDraggingCandidate(m_binding);
				}
//				m_bLeftdown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_LEFTUP)) {
				m_nTriggerEvent=EM_MOUSE_LEFTUP;
//				m_bLeftdown=false;
			}
			break;
		case GUIESLeftDown:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				//if farther than a distant, and candrag, drag begin
				if (IsMapTo(EM_MOUSE_LEFTDRAGBEGIN,EM_MOUSE_DRAGBEGIN)
					&&(abs(m_mouse.LastLDown.x-m_mouse.x)+abs(m_mouse.LastLDown.y-m_mouse.y))>m_nDragBeginDistance
					&&m_binding->GetCandrag()) {
					m_nTriggerEvent=EM_MOUSE_LEFTDRAGBEGIN;
					m_eState=(m_eState&~GUIESLeftMask)|GUIESDragBegin;
					return true;
				}else{
					m_nTriggerEvent=EM_MOUSE_MOVE;
				}
			}else if (IsMapTo(msg->message,EM_MOUSE_LEFTDOWN)) {
//				m_bLeftdown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_LEFTUP)) {
				if (m_binding->IsOnObject(msg->pt.x,msg->pt.y,0)
					&&IsMapTo(EM_MOUSE_LEFTCLICK,EM_MOUSE_CLICK)){
					m_eState=(m_eState&~GUIESLeftMask)|GUIESLeftClick;
					m_nTriggerEvent=EM_MOUSE_LEFTCLICK;
				}else{
					m_eState=(m_eState&~GUIESLeftMask)|GUIESLeftNone;
					m_nTriggerEvent=EM_MOUSE_LEFTUP;
					m_mouse.LastLDown.init();
				}
//				m_bLeftdown=false;
			}
			break;
		case GUIESLeftClick:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				m_nTriggerEvent=EM_MOUSE_MOVE;
			}else if (IsMapTo(msg->message,EM_MOUSE_LEFTDOWN)) {
				if ((int(msg->time-m_mouse.LastLDown.nTime)<m_nDBClickInterval)
					&&IsMapTo(EM_MOUSE_LEFTDBCLICK,EM_MOUSE_DBCLICK)) {
					m_nTriggerEvent=EM_MOUSE_LEFTDBCLICK;
					m_eState=(m_eState&~GUIESLeftMask)|GUIESLeftDBClick;
				}else{
					m_nTriggerEvent=EM_MOUSE_LEFTDOWN;
					m_mouse.LastLDown.init((short)msg->pt.x,(short)msg->pt.y,msg->time);
					m_eState=(m_eState&~GUIESLeftMask)|GUIESLeftDown;
				}
//				m_bLeftdown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_LEFTUP)) {
//				m_bLeftdown=false;
			}
			break;
		case GUIESLeftDBClick:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				//do nothing
			}else if (IsMapTo(msg->message,EM_MOUSE_LEFTDOWN)) {
//				m_bLeftdown=true;
				return false;
			}else if (IsMapTo(msg->message,EM_MOUSE_LEFTUP)) {
				m_eState=(m_eState&~GUIESLeftMask)|GUIESLeftNone;
				m_nTriggerEvent=EM_MOUSE_LEFTUP;
				m_mouse.LastLDown.init();
//				m_bLeftdown=false;
			}
			break;
		}
		//right button
		switch(m_eState&GUIESRightMask) {
		case GUIESRightNone:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				m_nTriggerEvent=EM_MOUSE_MOVE;
			}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTDOWN)) {
				m_eState=(m_eState&~GUIESRightMask)|GUIESRightDown;
				m_mouse.LastRDown.init((short)msg->pt.x,(short)msg->pt.y,msg->time);
				m_nTriggerEvent=EM_MOUSE_RIGHTDOWN;
//				m_bRightdown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTUP)) {
				m_nTriggerEvent=EM_MOUSE_RIGHTUP;
//				m_bRightdown=false;
			}
			break;
		case GUIESRightDown:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				if (IsMapTo(EM_MOUSE_RIGHTDRAGBEGIN,EM_MOUSE_DRAGBEGIN)
					&&(abs(m_mouse.LastRDown.x-m_mouse.x)+abs(m_mouse.LastRDown.y-m_mouse.y))>m_nDragBeginDistance
					&&m_binding->GetCandrag()) {
					m_nTriggerEvent=EM_MOUSE_RIGHTDRAGBEGIN;
					m_eState=(m_eState&~GUIESRightMask)|GUIESDragBegin;
					return true;
				}else{
					m_nTriggerEvent=EM_MOUSE_MOVE;
				}
			}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTDOWN)) {
//				m_bRightdown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTUP)) {
				if (m_binding->IsOnObject(msg->pt.x,msg->pt.y,0)
					&&IsMapTo(EM_MOUSE_RIGHTCLICK,EM_MOUSE_CLICK)){
					m_eState=(m_eState&~GUIESRightMask)|GUIESRightClick;
					m_nTriggerEvent=EM_MOUSE_RIGHTCLICK;
				}else{
					m_eState=(m_eState&~GUIESRightMask)|GUIESRightNone;
					m_nTriggerEvent=EM_MOUSE_RIGHTUP;
					m_mouse.LastRDown.init();
				}
//				m_bRightdown=false;
			}
			break;
		case GUIESRightClick:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				m_nTriggerEvent=EM_MOUSE_MOVE;
			}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTDOWN)) {
				if (int(msg->time-m_mouse.LastRDown.nTime)<m_nDBClickInterval // Mod by LXZ 2007.9.30
					&&IsMapTo(EM_MOUSE_RIGHTDBCLICK,EM_MOUSE_DBCLICK)) {
					m_nTriggerEvent=EM_MOUSE_RIGHTDBCLICK;
					m_eState=(m_eState&~GUIESRightMask)|GUIESRightDBClick;
				}else{
					m_nTriggerEvent=EM_MOUSE_RIGHTDOWN;
					m_mouse.LastRDown.init((short)msg->pt.x,(short)msg->pt.y,msg->time);
					m_eState=(m_eState&~GUIESRightMask)|GUIESRightDown;
				}
//				m_bRightdown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTUP)) {
//				m_bRightdown=false;
			}
			break;
		case GUIESRightDBClick:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				//do nothing
			}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTDOWN)) {
//				m_bRightdown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_RIGHTUP)) {
				m_eState=(m_eState&~GUIESRightMask)|GUIESRightNone;
				m_nTriggerEvent=EM_MOUSE_RIGHTUP;
				m_mouse.LastRDown.init();
//				m_bRightdown=false;
			}
			break;
		}
		//middle button
		switch(m_eState&GUIESMiddleMask) {
		case GUIESMiddleNone:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				m_nTriggerEvent=EM_MOUSE_MOVE;
			}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEDOWN)) {
				m_eState=(m_eState&~GUIESMiddleMask)|GUIESMiddleDown;
				m_mouse.LastRDown.init((short)msg->pt.x,(short)msg->pt.y,msg->time);
				m_nTriggerEvent=EM_MOUSE_MIDDLEDOWN;
//				m_bMiddledown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEUP)) {
				m_nTriggerEvent=EM_MOUSE_MIDDLEUP;
//				m_bMiddledown=false;
			}
			break;
		case GUIESMiddleDown:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				if (IsMapTo(EM_MOUSE_MIDDLEDRAGBEGIN,EM_MOUSE_DRAGBEGIN)
					&&(abs(m_mouse.LastMDown.x-m_mouse.x)+abs(m_mouse.LastMDown.y-m_mouse.y))>m_nDragBeginDistance\
					&&m_binding->GetCandrag()) {
						m_nTriggerEvent=EM_MOUSE_MIDDLEDRAGBEGIN;
						m_eState=(m_eState&~GUIESMiddleMask)|GUIESDragBegin;
						return true;
					}else{
						m_nTriggerEvent=EM_MOUSE_MOVE;
					}
			}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEDOWN)) {
//				m_bMiddledown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEUP)) {
				if (m_binding->IsOnObject(msg->pt.x,msg->pt.y,0)
					&&IsMapTo(EM_MOUSE_MIDDLECLICK,EM_MOUSE_CLICK)){
					m_eState=(m_eState&~GUIESMiddleMask)|GUIESMiddleClick;
					m_nTriggerEvent=EM_MOUSE_MIDDLECLICK;
				}else{
					m_eState=(m_eState&~GUIESMiddleMask)|GUIESMiddleNone;
					m_nTriggerEvent=EM_MOUSE_MIDDLEUP;
					m_mouse.LastRDown.init();
				}
//				m_bMiddledown=false;
			}
			break;
		case GUIESMiddleClick:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				m_nTriggerEvent=EM_MOUSE_MOVE;
			}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEDOWN)) {
				if (int(msg->time-m_mouse.LastMDown.nTime)<m_nDBClickInterval // Mod by LXZ 2007.9.30
					&&IsMapTo(EM_MOUSE_MIDDLEDBCLICK,EM_MOUSE_DBCLICK)) {
					m_nTriggerEvent=EM_MOUSE_MIDDLEDBCLICK;
					m_eState=(m_eState&~GUIESMiddleMask)|GUIESMiddleDBClick;
				}else{
					m_nTriggerEvent=EM_MOUSE_MIDDLEDOWN;
					m_mouse.LastMDown.init((short)msg->pt.x,(short)msg->pt.y,msg->time);
					m_eState=(m_eState&~GUIESMiddleMask)|GUIESMiddleDown;
				}
//				m_bMiddledown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEUP)) {
//				m_bMiddledown=false;
			}
			break;
		case GUIESMiddleDBClick:
			if (IsMapTo(msg->message,EM_MOUSE_MOVE)) {
				//do nothing
			}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEDOWN)) {
//				m_bMiddledown=true;
			}else if (IsMapTo(msg->message,EM_MOUSE_MIDDLEUP)) {
				m_eState=(m_eState&~GUIESMiddleMask)|GUIESMiddleNone;
				m_nTriggerEvent=EM_MOUSE_MIDDLEUP;
				m_mouse.LastMDown.init();
//				m_bMiddledown=false;
			}
			break;
		}
	}else if (IsMapTo(msg->message,EM_KEY)) {
		m_keyboard.nAlterKey=msg->message;
#if PARA_TARGET_PLATFORM != PARA_PLATFORM_MAC && PARA_TARGET_PLATFORM != PARA_PLATFORM_EMSCRIPTEN
		if (IsMapTo(msg->message,EM_KEY_NUMPAD0)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_INSERT;
			}else{
				m_keyboard.nAlterKey=EM_KEY_0;
			}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPAD1)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_END;
			}else{
				m_keyboard.nAlterKey=EM_KEY_1;
			}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPAD2)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_DOWN;
			}else{
				m_keyboard.nAlterKey=EM_KEY_2;
			}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPAD3)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_PAGE_DOWN;
			}else{
				m_keyboard.nAlterKey=EM_KEY_3;
			}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPAD4)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_LEFT;
			}else{
				m_keyboard.nAlterKey=EM_KEY_4;
			}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPAD5)) {
			if (NumLockPressed&&!ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_5;
			}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPAD6)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_RIGHT;
			}else{
				m_keyboard.nAlterKey=EM_KEY_6;
			}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPAD7)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_HOME;
			}else{
				m_keyboard.nAlterKey=EM_KEY_7;
		}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPAD8)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_UP;
			}else{
				m_keyboard.nAlterKey=EM_KEY_8;
			}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPAD9)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_PAGE_UP;
			}else{
				m_keyboard.nAlterKey=EM_KEY_9;
			}
		}else if (IsMapTo(msg->message,EM_KEY_DECIMAL)) {
			if (!NumLockPressed||ShiftPressed) {
				m_keyboard.nAlterKey=EM_KEY_DELETE;
			}else{
				m_keyboard.nAlterKey=EM_KEY_PERIOD;
			}
		}else if (IsMapTo(msg->message,EM_KEY_NUMPADENTER)) {
			m_keyboard.nAlterKey=EM_KEY_RETURN;
		}
#else // PARA_TARGET_PLATFORM != PARA_PLATFORM_MAC
        if (IsMapTo(msg->message,EM_KEY_NUMPADENTER)) {
            m_keyboard.nAlterKey=EM_KEY_RETURN;
        }
#endif //PARA_TARGET_PLATFORM != PARA_PLATFORM_MAC
		if (IsMapTo(m_keyboard.nAlterKey,EM_CTRL_HOLDKEY)&&msg->lParam==0x80&&msg->hwnd!=0) {
			AddHoldKey((DWORD)msg->wParam,msg->time);
		}
	}
	return true;
}

void CGUIEvent::UpdateKey(int option)
{
	//adding hold key and copying the key events to this event object in the following
	CGUIKeyboardVirtual *pKeyboard=CGUIRoot::GetInstance()->m_pKeyboard;
	
	vector<GUI_KEYBOARD_HOLDKEY>::iterator keyiter,keyend;
	for (keyiter=m_keyboard.HoldKey.begin();keyiter!=m_keyboard.HoldKey.end();) {
	
		if (!pKeyboard->IsKeyPressed((EVirtualKey)(*keyiter).key)) {
			keyiter = m_keyboard.HoldKey.erase(keyiter);
		}else
			keyiter++;
	}
	m_keyboard.Size=0;
	//repeat the keys
	if ((option&REPEAT_PRESSED_KEYS)!=0&&(option&IGNORE_DOWN_EVENTS)==0) {
		vector<GUI_KEYBOARD_HOLDKEY>::iterator keyiter,keyend=m_keyboard.HoldKey.end();
		GUI_KEYBOARD_HOLDKEY *holdkey;
		for (keyiter=m_keyboard.HoldKey.begin();keyiter!=keyend&&m_keyboard.Size<SAMPLE_BUFFER_SIZE;keyiter++) {
			holdkey=&(*keyiter);
			if ((holdkey->state==0)&&(m_nTime - holdkey->lasttime>CGUIRoot::KeyDelay)) {
				holdkey->state=1;
				holdkey->lasttime=m_nTime;
			}
			if ((holdkey->state==1)&&(m_nTime-holdkey->lasttime>CGUIRoot::KeyRepeat)) {
				holdkey->lasttime=m_nTime;
				m_keyboard.KeyEvents[m_keyboard.Size].dwData=0x80;
				m_keyboard.KeyEvents[m_keyboard.Size].dwOfs=holdkey->key;
				m_keyboard.KeyEvents[m_keyboard.Size].dwTimeStamp=m_nTime;
				m_keyboard.KeyEvents[m_keyboard.Size].uAppData=CEventBinding::ScancodeToKeyTable[holdkey->key];
				m_keyboard.KeyEvents[m_keyboard.Size].dwSequence=0;
				m_keyboard.Size++;
			}
		}
	}	
	//copy the key events
	for(DWORD i = 0; i < pKeyboard->GetElementsCount() && m_keyboard.Size<SAMPLE_BUFFER_SIZE; i++ ) {
		if (((pKeyboard->GetDeviceObjectData()[i].dwData&0x80)!=0&&(option&IGNORE_DOWN_EVENTS)==0)||
			((pKeyboard->GetDeviceObjectData()[i].dwData&0x80)==0&&(option&IGNORE_UP_EVENTS)==0)){
				m_keyboard.KeyEvents[m_keyboard.Size]=pKeyboard->GetDeviceObjectData()[i];
				m_keyboard.KeyEvents[m_keyboard.Size].uAppData=CEventBinding::ScancodeToKeyTable[pKeyboard->GetDeviceObjectData()[i].dwOfs];
				m_keyboard.Size++;
			}
	}
}
/** Interprets the raw mouse or keyboard state to meaningful events.
 The interpreted mouse events are Click, Double Click, Hover, Mouse Enter, Drag Begin, Drag Over, Drag End
 We advise user to call this to interpret mouse events because it's fast and convenient.
 However, we don't advise user to call this to interpret keyboard events because it's time consuming and
 we can only interpret two events.*/
//we currently processed the first key event, and clear all other key events

void CGUIEvent::AddHoldKey(DWORD key,DWORD nTime)
{
	vector<GUI_KEYBOARD_HOLDKEY>::iterator iter,iterend;
	iterend=m_keyboard.HoldKey.end();
	for (iter=m_keyboard.HoldKey.begin();iter!=iterend;iter++) {
		if ((*iter).key==key) {
			break;
		}
	}
	if (iter==iterend) {
		GUI_KEYBOARD_HOLDKEY holdkey;
		holdkey.key=key;
		holdkey.lasttime=nTime;
		holdkey.state=0;
		m_keyboard.HoldKey.push_back(holdkey);
	}

}

CHAR CGUIEvent::GetChar(DWORD key)
{
	const unsigned char *tempstate=CGUIEvent::KeyStates;

	if (!ControlPressed&&!AltPressed) {
		switch(key){
		case EM_KEY_1:
			return ShiftPressed?'!':'1';
			break;
		case EM_KEY_2:
			return ShiftPressed?'@':'2';
			break;
		case EM_KEY_3:
			return ShiftPressed?'#':'3';
			break;
		case EM_KEY_4:
			return ShiftPressed?'$':'4';
			break;
		case EM_KEY_5:
			return ShiftPressed?'%':'5';
			break;
		case EM_KEY_6:
			return ShiftPressed?'^':'6';
			break;
		case EM_KEY_7:
			return ShiftPressed?'&':'7';
			break;
		case EM_KEY_8:
			return ShiftPressed?'*':'8';
			break;
		case EM_KEY_9:
			return ShiftPressed?'(':'9';
			break;
		case EM_KEY_0:
			return ShiftPressed?')':'0';
			break;
		case EM_KEY_MINUS:
			return ShiftPressed?'_':'-';
			break;
		case EM_KEY_EQUALS:
			return ShiftPressed?'+':'=';
			break;
		case EM_KEY_Q:
			return (ShiftPressed^CapsLockPressed)?'Q':'q';
			break;
		case EM_KEY_W:
			return (ShiftPressed^CapsLockPressed)?'W':'w';
			break;
		case EM_KEY_E:
			return (ShiftPressed^CapsLockPressed)?'E':'e';
			break;
		case EM_KEY_R:
			return (ShiftPressed^CapsLockPressed)?'R':'r';
			break;
		case EM_KEY_T:
			return (ShiftPressed^CapsLockPressed)?'T':'t';
			break;
		case EM_KEY_Y:
			return (ShiftPressed^CapsLockPressed)?'Y':'y';
			break;
		case EM_KEY_U:
			return (ShiftPressed^CapsLockPressed)?'U':'u';
			break;
		case EM_KEY_I:
			return (ShiftPressed^CapsLockPressed)?'I':'i';
			break;
		case EM_KEY_O:
			return (ShiftPressed^CapsLockPressed)?'O':'o';
			break;
		case EM_KEY_P:
			return (ShiftPressed^CapsLockPressed)?'P':'p';
			break;
		case EM_KEY_LBRACKET:
			return ShiftPressed?'{':'[';
			break;
		case EM_KEY_RBRACKET:
			return ShiftPressed?'}':']';
			break;
		case EM_KEY_A:
			return (ShiftPressed^CapsLockPressed)?'A':'a';
			break;
		case EM_KEY_S:
			return (ShiftPressed^CapsLockPressed)?'S':'s';
			break;
		case EM_KEY_D:
			return (ShiftPressed^CapsLockPressed)?'D':'d';
			break;
		case EM_KEY_F:
			return (ShiftPressed^CapsLockPressed)?'F':'f';
			break;
		case EM_KEY_G:
			return (ShiftPressed^CapsLockPressed)?'G':'g';
			break;
		case EM_KEY_H:
			return (ShiftPressed^CapsLockPressed)?'H':'h';
			break;
		case EM_KEY_J:
			return (ShiftPressed^CapsLockPressed)?'J':'j';
			break;
		case EM_KEY_K:
			return (ShiftPressed^CapsLockPressed)?'K':'k';
			break;
		case EM_KEY_L:
			return (ShiftPressed^CapsLockPressed)?'L':'l';
			break;
		case EM_KEY_SEMICOLON:
			return ShiftPressed?':':';';
			break;
		case EM_KEY_APOSTROPHE:
			return ShiftPressed?'\"':'\'';
			break;
		case EM_KEY_GRAVE:
			return ShiftPressed?'~':'`';
			break;
		case EM_KEY_BACKSLASH:
			return ShiftPressed?'|':'\\';
			break;
		case EM_KEY_Z:
			return (ShiftPressed^CapsLockPressed)?'Z':'z';
			break;
		case EM_KEY_X:
			return (ShiftPressed^CapsLockPressed)?'X':'x';
			break;
		case EM_KEY_C:
			return (ShiftPressed^CapsLockPressed)?'C':'c';
			break;
		case EM_KEY_V:
			return (ShiftPressed^CapsLockPressed)?'V':'v';
			break;
		case EM_KEY_B:
			return (ShiftPressed^CapsLockPressed)?'B':'b';
			break;
		case EM_KEY_N:
			return (ShiftPressed^CapsLockPressed)?'N':'n';
			break;
		case EM_KEY_M:
			return (ShiftPressed^CapsLockPressed)?'M':'m';
			break;
		case EM_KEY_COMMA:
			return ShiftPressed?'<':',';
				break;
		case EM_KEY_PERIOD:
			return ShiftPressed?'>':'.';
				break;
		case EM_KEY_SLASH:
			return ShiftPressed?'?':'/';
				break;
		case EM_KEY_MULTIPLY:
			return '*';
			break;
		case EM_KEY_TAB:
			return '\t';
			break;
		case EM_KEY_SPACE:
			return ' ';
			break;
		case EM_KEY_NUMPAD7:
			return ShiftPressed||(!NumLockPressed)?'\0':'7';
			break;
		case EM_KEY_NUMPAD8:
			return ShiftPressed||(!NumLockPressed)?'\0':'8';
			break;
		case EM_KEY_NUMPAD9:
			return ShiftPressed||(!NumLockPressed)?'\0':'9';
			break;
		case EM_KEY_SUBTRACT:
			return '-';
			break;
		case EM_KEY_NUMPAD4:
			return ShiftPressed||(!NumLockPressed)?'\0':'4';
			break;
		case EM_KEY_NUMPAD5:
			return ShiftPressed||(!NumLockPressed)?'\0':'5';
			break;
		case EM_KEY_NUMPAD6:
			return ShiftPressed||(!NumLockPressed)?'\0':'6';
			break;
		case EM_KEY_ADD:
			return '+';
			break;
		case EM_KEY_NUMPAD1:
			return ShiftPressed||(!NumLockPressed)?'\0':'1';
			break;
		case EM_KEY_NUMPAD2:
			return ShiftPressed||(!NumLockPressed)?'\0':'2';
			break;
		case EM_KEY_NUMPAD3:
			return ShiftPressed||(!NumLockPressed)?'\0':'3';
			break;
		case EM_KEY_NUMPAD0:
			return ShiftPressed||(!NumLockPressed)?'\0':'0';
			break;
		case EM_KEY_DECIMAL:
			return ShiftPressed||(!NumLockPressed)?'\0':'.';
			break;
		case EM_KEY_DIVIDE:
			return '/';
			break;
		default:
			return '\0';
			break;
		}
	}
	return '\0';
}



//string CGUIEvent::GetCharSequence()
//{
//	string text;
//	DWORD a;
//	unsigned char tempstate[256];
//	CGUIKeyboardVirtual *pKeyboard=CGlobals::GetGUI()->m_pKeyboard;
//	memcpy(tempstate,pKeyboard->m_lastkeystate,256);
//	for (a=0;a<m_keyboard.Size;a++) {
//		///process the key down events in the direct keyboard buffer
//        if ((m_keyboard.KeyEvents[a].dwData&0x80)!=0) {
//			switch(m_keyboard.KeyEvents[a].dwOfs) {
//			///Switch the state of the lock keys.
//			case DIK_NUMLOCK:
//			case DIK_SCROLL:
//			case DIK_CAPITAL:
//				tempstate[m_keyboard.KeyEvents[a].dwOfs]^=128;/// switch the high order bit
//				break;
//			///Set the state of control keys
//			case DIK_LCONTROL:
//			case DIK_RCONTROL:
//			case DIK_LSHIFT:
//			case DIK_RSHIFT:
//			case DIK_LALT:
//			case DIK_RALT:
//				tempstate[m_keyboard.KeyEvents[a].dwOfs]=128;/// set the high order bit
//				break;
//			}
//			//Dealing with character keys
//			if (!AltPressed&&!ControlPressed) {
//				TCHAR temp=GetChar(m_keyboard.KeyEvents[a].dwOfs);
//				if (temp!='\0') {
//					text+=temp;
//				}
//			}
//        }
//		if ((m_keyboard.KeyEvents[a].dwData&0x80)!=0) {
//			switch(m_keyboard.KeyEvents[a].dwOfs) {
//				///Switch the state of the lock keys.
//				///Set the state of control keys
//			case DIK_LCONTROL:
//			case DIK_RCONTROL:
//			case DIK_LSHIFT:	
//			case DIK_RSHIFT:
//			case DIK_LALT:
//			case DIK_RALT:
//				tempstate[m_keyboard.KeyEvents[a].dwOfs]=0;/// clean the high order bit
//				break;
//			}
//		}
//	}
//	return text;
//}
//
bool CGUIEvent::IsKeyPressed(int key)
{
	return (CGlobals::GetGUI()->m_pKeyboard->IsKeyPressed((EVirtualKey)key));
}

bool CGUIEvent::IsMousePressed(int mouse)
{
	return CGUIRoot::GetInstance()->m_pMouse->IsButtonDown((EMouseButton)mouse);	
}
