//----------------------------------------------------------------------
// Class:	CEventBinding
// Authors:	Liu Weili, LiXizhi
// Date:	2006.3.17
// Revised: 2006.3.17
//
// desc: 
// This class stores the necessary tables for mapping an event to another event,
// mapping an event to a script, translating between string and event value. 
// It implements the IObject interface. 
// Clone() is implemented as a lazy-copier. This is : copy on write.
// 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "EventBinding.h"
#include "util/ObjectManager.h"
#include "util/cow_ptr.hpp"
#include "memdebug.h"
#include "Framework/InputSystem/VirtualKey.h"

using namespace std;
using namespace ParaEngine;
map<string,int> CEventBinding::StringToEventTable;
map<int,string> CEventBinding::EventToStringTable;
DWORD CEventBinding::ScancodeToKeyTable[(int)EVirtualKey::COUNT];

void CEventBinding::InitMsg(MSG *pevent,DWORD time,DWORD message,POINT& pt)
{
	ZeroMemory(pevent,sizeof(MSG));
	pevent->hwnd=0;
	pevent->message=message;
	pevent->pt=pt;
	pevent->time=time;
}
void CEventBinding::StaticInit()
{
	CObjectManager *pOm=&CSingleton<CObjectManager>::Instance();
	CEventBinding *pEvent=new CEventBinding();
	pEvent->InitEventMappingTable(false);
	pEvent->DefaultMap_Control();
	pEvent->DefaultMap_Mouse();
	pOm->SetObject("default_CEventBinding",pEvent);
	SAFE_RELEASE(pEvent);
	//release done

	//building the event to string and string to event table
	EventToStringTable[EM_NONE]="EM_NONE";
	StringToEventTable["EM_NONE"]=EM_NONE;
	EventToStringTable[EM_PL_FORWARD]="EM_PL_FORWARD";
	StringToEventTable["EM_PL_FORWARD"]=EM_PL_FORWARD;
	EventToStringTable[EM_PL_BACKWARD]="EM_PL_BACKWARD";
	StringToEventTable["EM_PL_BACKWARD"]=EM_PL_BACKWARD;
	EventToStringTable[EM_PL_LEFT]="EM_PL_LEFT";
	StringToEventTable["EM_PL_LEFT"]=EM_PL_LEFT;
	EventToStringTable[EM_PL_RIGHT]="EM_PL_RIGHT";
	StringToEventTable["EM_PL_RIGHT"]=EM_PL_RIGHT;
	EventToStringTable[EM_PL_SHIFTLEFT]="EM_PL_SHIFTLEFT";
	StringToEventTable["EM_PL_SHIFTLEFT"]=EM_PL_SHIFTLEFT;
	EventToStringTable[EM_PL_SHIFTRIGHT]="EM_PL_SHIFTRIGHT";
	StringToEventTable["EM_PL_SHIFTRIGHT"]=EM_PL_SHIFTRIGHT;
	EventToStringTable[EM_PL_JUMP]="EM_PL_JUMP";
	StringToEventTable["EM_PL_JUMP"]=EM_PL_JUMP;
	EventToStringTable[EM_PL_CROUCH]="EM_PL_CROUCH";
	StringToEventTable["EM_PL_CROUCH"]=EM_PL_CROUCH;
	EventToStringTable[EM_CAM_LOCK]="EM_CAM_LOCK";
	StringToEventTable["EM_CAM_LOCK"]=EM_CAM_LOCK;
	EventToStringTable[EM_CAM_RESET]="EM_CAM_RESET";
	StringToEventTable["EM_CAM_RESET"]=EM_CAM_RESET;
	EventToStringTable[EM_CAM_FORWARD]="EM_CAM_FORWARD";
	StringToEventTable["EM_CAM_FORWARD"]=EM_CAM_FORWARD;
	EventToStringTable[EM_CAM_BACKWARD]="EM_CAM_BACKWARD";
	StringToEventTable["EM_CAM_BACKWARD"]=EM_CAM_BACKWARD;
	EventToStringTable[EM_CAM_LEFT]="EM_CAM_LEFT";
	StringToEventTable["EM_CAM_LEFT"]=EM_CAM_LEFT;
	EventToStringTable[EM_CAM_RIGHT]="EM_CAM_RIGHT";
	StringToEventTable["EM_CAM_RIGHT"]=EM_CAM_RIGHT;
	EventToStringTable[EM_CAM_SHIFTLEFT]="EM_CAM_SHIFTLEFT";
	StringToEventTable["EM_CAM_SHIFTLEFT"]=EM_CAM_SHIFTLEFT;
	EventToStringTable[EM_CAM_SHIFTRIGHT]="EM_CAM_SHIFTRIGHT";
	StringToEventTable["EM_CAM_SHIFTRIGHT"]=EM_CAM_SHIFTRIGHT;
	EventToStringTable[EM_CTRL_CHANGE]="EM_CTRL_CHANGE";
	StringToEventTable["EM_CTRL_CHANGE"]=EM_CTRL_CHANGE;
	EventToStringTable[EM_CTRL_MODIFY]="EM_CTRL_MODIFY";
	StringToEventTable["EM_CTRL_MODIFY"]=EM_CTRL_MODIFY;
	EventToStringTable[EM_CTRL_CHAR]="EM_CTRL_CHAR";
	StringToEventTable["EM_CTRL_CHAR"]=EM_CTRL_CHAR;
	EventToStringTable[EM_CTRL_KEYDOWN]="EM_CTRL_KEYDOWN";
	StringToEventTable["EM_CTRL_KEYDOWN"]=EM_CTRL_KEYDOWN;
	EventToStringTable[EM_CTRL_KEYUP]="EM_CTRL_KEYUP";
	StringToEventTable["EM_CTRL_KEYUP"]=EM_CTRL_KEYUP;
	EventToStringTable[EM_CTRL_HOLDKEY]="EM_CTRL_HOLDKEY";
	StringToEventTable["EM_CTRL_HOLDKEY"]=EM_CTRL_HOLDKEY;
	EventToStringTable[EM_CTRL_SELECT]="EM_CTRL_SELECT";
	StringToEventTable["EM_CTRL_SELECT"]=EM_CTRL_SELECT;
	EventToStringTable[EM_CTRL_FOCUSIN]="EM_CTRL_FOCUSIN";
	StringToEventTable["EM_CTRL_FOCUSIN"]=EM_CTRL_FOCUSIN;
	EventToStringTable[EM_CTRL_FOCUSOUT]="EM_CTRL_FOCUSOUT";
	StringToEventTable["EM_CTRL_FOCUSOUT"]=EM_CTRL_FOCUSOUT;
	EventToStringTable[EM_CTRL_FRAMEMOVE]="EM_CTRL_FRAMEMOVE";
	StringToEventTable["EM_CTRL_FRAMEMOVE"]=EM_CTRL_FRAMEMOVE;
	EventToStringTable[EM_CTRL_UPDATEKEY]="EM_CTRL_UPDATEKEY";
	StringToEventTable["EM_CTRL_UPDATEKEY"]=EM_CTRL_UPDATEKEY;
	EventToStringTable[EM_CTRL_CAPTUREMOUSE]="EM_CTRL_CAPTUREMOUSE";
	StringToEventTable["EM_CTRL_CAPTUREMOUSE"]=EM_CTRL_CAPTUREMOUSE;
	EventToStringTable[EM_CTRL_RELEASEMOUSE]="EM_CTRL_RELEASEMOUSE";
	StringToEventTable["EM_CTRL_RELEASEMOUSE"]=EM_CTRL_RELEASEMOUSE;
	EventToStringTable[EM_CTRL_NEXTKEYFOCUS]="EM_CTRL_NEXTKEYFOCUS";
	StringToEventTable["EM_CTRL_NEXTKEYFOCUS"]=EM_CTRL_NEXTKEYFOCUS;
	EventToStringTable[EM_BTN_DOWN]="EM_BTN_DOWN";
	StringToEventTable["EM_BTN_DOWN"]=EM_BTN_DOWN;
	EventToStringTable[EM_BTN_UP]="EM_BTN_UP";
	StringToEventTable["EM_BTN_UP"]=EM_BTN_UP;
	EventToStringTable[EM_BTN_CLICK]="EM_BTN_CLICK";
	StringToEventTable["EM_BTN_CLICK"]=EM_BTN_CLICK;
	EventToStringTable[EM_SB_ACTIONEND]="EM_SB_ACTIONEND";
	StringToEventTable["EM_SB_ACTIONEND"]=EM_SB_ACTIONEND;
	EventToStringTable[EM_SB_ACTIONBEGIN]="EM_SB_ACTIONBEGIN";
	StringToEventTable["EM_SB_ACTIONBEGIN"]=EM_SB_ACTIONBEGIN;
	EventToStringTable[EM_SB_PAGEDOWN]="EM_SB_PAGEDOWN";
	StringToEventTable["EM_SB_PAGEDOWN"]=EM_SB_PAGEDOWN;
	EventToStringTable[EM_SB_PAGEUP]="EM_SB_PAGEUP";
	StringToEventTable["EM_SB_PAGEUP"]=EM_SB_PAGEUP;
	EventToStringTable[EM_SB_STEPDOWN]="EM_SB_STEPDOWN";
	StringToEventTable["EM_SB_STEPDOWN"]=EM_SB_STEPDOWN;
	EventToStringTable[EM_SB_STEPUP]="EM_SB_STEPUP";
	StringToEventTable["EM_SB_STEPUP"]=EM_SB_STEPUP;
	EventToStringTable[EM_SB_SCROLL]="EM_SB_SCROLL";
	StringToEventTable["EM_SB_SCROLL"]=EM_SB_SCROLL;
	EventToStringTable[EM_EB_SELECTSTART]="EM_EB_SELECTSTART";
	StringToEventTable["EM_EB_SELECTSTART"]=EM_EB_SELECTSTART;
	EventToStringTable[EM_EB_SELECTEND]="EM_EB_SELECTEND";
	StringToEventTable["EM_EB_SELECTEND"]=EM_EB_SELECTEND;
	EventToStringTable[EM_EB_SELECTALL]="EM_EB_SELECTALL";
	StringToEventTable["EM_EB_SELECTALL"]=EM_EB_SELECTALL;
	EventToStringTable[EM_IME_SELECT]="EM_IME_SELECT";
	StringToEventTable["EM_IME_SELECT"]=EM_IME_SELECT;
	EventToStringTable[EM_LB_ACTIONEND]="EM_LB_ACTIONEND";
	StringToEventTable["EM_LB_ACTIONEND"]=EM_LB_ACTIONEND;
	EventToStringTable[EM_LB_ACTIONBEGIN]="EM_LB_ACTIONBEGIN";
	StringToEventTable["EM_LB_ACTIONBEGIN"]=EM_LB_ACTIONBEGIN;
	EventToStringTable[EM_SL_ACTIONBEGIN]="EM_SL_ACTIONBEGIN";
	StringToEventTable["EM_SL_ACTIONBEGIN"]=EM_SL_ACTIONBEGIN;
	EventToStringTable[EM_SL_ACTIONEND]="EM_SL_ACTIONEND";
	StringToEventTable["EM_SL_ACTIONEND"]=EM_SL_ACTIONEND;
	EventToStringTable[EM_SL_CHANGEVALUE]="EM_SL_CHANGEVALUE";
	StringToEventTable["EM_SL_CHANGEVALUE"]=EM_SL_CHANGEVALUE;
	EventToStringTable[EM_CV_ROTATEBEGIN]="EM_CV_ROTATEBEGIN";
	StringToEventTable["EM_CV_ROTATEBEGIN"]=EM_CV_ROTATEBEGIN;
	EventToStringTable[EM_CV_ROTATEEND]="EM_CV_ROTATEEND";
	StringToEventTable["EM_CV_ROTATEEND"]=EM_CV_ROTATEEND;
	EventToStringTable[EM_CV_PANBEGIN]="EM_CV_PANBEGIN";
	StringToEventTable["EM_CV_PANBEGIN"]=EM_CV_PANBEGIN;
	EventToStringTable[EM_CV_PANEND]="EM_CV_PANEND";
	StringToEventTable["EM_CV_PANEND"]=EM_CV_PANEND;
	EventToStringTable[EM_KEY_BACKSPACE]="EM_KEY_BACKSPACE";
	StringToEventTable["EM_KEY_BACKSPACE"]=EM_KEY_BACKSPACE;
	EventToStringTable[EM_KEY_TAB]="EM_KEY_TAB";
	StringToEventTable["EM_KEY_TAB"]=EM_KEY_TAB;
	EventToStringTable[EM_KEY_RETURN]="EM_KEY_RETURN";
	StringToEventTable["EM_KEY_RETURN"]=EM_KEY_RETURN;
	EventToStringTable[EM_KEY_SHIFT]="EM_KEY_SHIFT";
	StringToEventTable["EM_KEY_SHIFT"]=EM_KEY_SHIFT;
	EventToStringTable[EM_KEY_CONTROL]="EM_KEY_CONTROL";
	StringToEventTable["EM_KEY_CONTROL"]=EM_KEY_CONTROL;
	EventToStringTable[EM_KEY_ALT]="EM_KEY_ALT";
	StringToEventTable["EM_KEY_ALT"]=EM_KEY_ALT;
	EventToStringTable[EM_KEY_PAUSE]="EM_KEY_PAUSE";
	StringToEventTable["EM_KEY_PAUSE"]=EM_KEY_PAUSE;
	EventToStringTable[EM_KEY_CAPSLOCK]="EM_KEY_CAPSLOCK";
	StringToEventTable["EM_KEY_CAPSLOCK"]=EM_KEY_CAPSLOCK;
	EventToStringTable[EM_KEY_ESCAPE]="EM_KEY_ESCAPE";
	StringToEventTable["EM_KEY_ESCAPE"]=EM_KEY_ESCAPE;
	EventToStringTable[EM_KEY_SPACE]="EM_KEY_SPACE";
	StringToEventTable["EM_KEY_SPACE"]=EM_KEY_SPACE;
	EventToStringTable[EM_KEY_PAGE_DOWN]="EM_KEY_PAGE_DOWN";
	StringToEventTable["EM_KEY_PAGE_DOWN"]=EM_KEY_PAGE_DOWN;
	EventToStringTable[EM_KEY_PAGE_UP]="EM_KEY_PAGE_UP";
	StringToEventTable["EM_KEY_PAGE_UP"]=EM_KEY_PAGE_UP;
	EventToStringTable[EM_KEY_END]="EM_KEY_END";
	StringToEventTable["EM_KEY_END"]=EM_KEY_END;
	EventToStringTable[EM_KEY_HOME]="EM_KEY_HOME";
	StringToEventTable["EM_KEY_HOME"]=EM_KEY_HOME;
	EventToStringTable[EM_KEY_LEFT]="EM_KEY_LEFT";
	StringToEventTable["EM_KEY_LEFT"]=EM_KEY_LEFT;
	EventToStringTable[EM_KEY_UP]="EM_KEY_UP";
	StringToEventTable["EM_KEY_UP"]=EM_KEY_UP;
	EventToStringTable[EM_KEY_RIGHT]="EM_KEY_RIGHT";
	StringToEventTable["EM_KEY_RIGHT"]=EM_KEY_RIGHT;
	EventToStringTable[EM_KEY_DOWN]="EM_KEY_DOWN";
	StringToEventTable["EM_KEY_DOWN"]=EM_KEY_DOWN;
	EventToStringTable[EM_KEY_PRINT]="EM_KEY_PRINT";
	StringToEventTable["EM_KEY_PRINT"]=EM_KEY_PRINT;
	EventToStringTable[EM_KEY_INSERT]="EM_KEY_INSERT";
	StringToEventTable["EM_KEY_INSERT"]=EM_KEY_INSERT;
	EventToStringTable[EM_KEY_DELETE]="EM_KEY_DELETE";
	StringToEventTable["EM_KEY_DELETE"]=EM_KEY_DELETE;
	EventToStringTable[EM_KEY_HELP]="EM_KEY_HELP";
	StringToEventTable["EM_KEY_HELP"]=EM_KEY_HELP;
	EventToStringTable[EM_KEY_WIN_WINDOW]="EM_KEY_WIN_WINDOW";
	StringToEventTable["EM_KEY_WIN_WINDOW"]=EM_KEY_WIN_WINDOW;
	EventToStringTable[EM_KEY_WIN_LWINDOW]="EM_KEY_WIN_LWINDOW";
	StringToEventTable["EM_KEY_WIN_LWINDOW"]=EM_KEY_WIN_LWINDOW;
	EventToStringTable[EM_KEY_WIN_RWINDOW]="EM_KEY_WIN_RWINDOW";
	StringToEventTable["EM_KEY_WIN_RWINDOW"]=EM_KEY_WIN_RWINDOW;
	EventToStringTable[EM_KEY_WIN_APPS]="EM_KEY_WIN_APPS";
	StringToEventTable["EM_KEY_WIN_APPS"]=EM_KEY_WIN_APPS;
	EventToStringTable[EM_KEY_NUMPAD_BEGIN]="EM_KEY_NUMPADBEGIN";
	StringToEventTable["EM_KEY_NUMPADBEGIN"]=EM_KEY_NUMPAD_BEGIN;
	EventToStringTable[EM_KEY_NUMPAD0]="EM_KEY_NUMPAD0";
	StringToEventTable["EM_KEY_NUMPAD0"]=EM_KEY_NUMPAD0;
	EventToStringTable[EM_KEY_UPNUMPAD0]="EM_KEY_UPNUMPAD0";
	StringToEventTable["EM_KEY_UPNUMPAD0"]=EM_KEY_UPNUMPAD0;
	EventToStringTable[EM_KEY_NUMPAD1]="EM_KEY_NUMPAD1";
	StringToEventTable["EM_KEY_NUMPAD1"]=EM_KEY_NUMPAD1;
	EventToStringTable[EM_KEY_UPNUMPAD1]="EM_KEY_UPNUMPAD1";
	StringToEventTable["EM_KEY_UPNUMPAD1"]=EM_KEY_UPNUMPAD1;
	EventToStringTable[EM_KEY_NUMPAD2]="EM_KEY_NUMPAD2";
	StringToEventTable["EM_KEY_NUMPAD2"]=EM_KEY_NUMPAD2;
	EventToStringTable[EM_KEY_UPNUMPAD2]="EM_KEY_UPNUMPAD2";
	StringToEventTable["EM_KEY_UPNUMPAD2"]=EM_KEY_UPNUMPAD2;
	EventToStringTable[EM_KEY_NUMPAD3]="EM_KEY_NUMPAD3";
	StringToEventTable["EM_KEY_NUMPAD3"]=EM_KEY_NUMPAD3;
	EventToStringTable[EM_KEY_UPNUMPAD3]="EM_KEY_UPNUMPAD3";
	StringToEventTable["EM_KEY_UPNUMPAD3"]=EM_KEY_UPNUMPAD3;
	EventToStringTable[EM_KEY_NUMPAD4]="EM_KEY_NUMPAD4";
	StringToEventTable["EM_KEY_NUMPAD4"]=EM_KEY_NUMPAD4;
	EventToStringTable[EM_KEY_UPNUMPAD4]="EM_KEY_UPNUMPAD4";
	StringToEventTable["EM_KEY_UPNUMPAD4"]=EM_KEY_UPNUMPAD4;
	EventToStringTable[EM_KEY_NUMPAD5]="EM_KEY_NUMPAD5";
	StringToEventTable["EM_KEY_NUMPAD5"]=EM_KEY_NUMPAD5;
	EventToStringTable[EM_KEY_UPNUMPAD5]="EM_KEY_UPNUMPAD5";
	StringToEventTable["EM_KEY_UPNUMPAD5"]=EM_KEY_UPNUMPAD5;
	EventToStringTable[EM_KEY_NUMPAD6]="EM_KEY_NUMPAD6";
	StringToEventTable["EM_KEY_NUMPAD6"]=EM_KEY_NUMPAD6;
	EventToStringTable[EM_KEY_UPNUMPAD6]="EM_KEY_UPNUMPAD6";
	StringToEventTable["EM_KEY_UPNUMPAD6"]=EM_KEY_UPNUMPAD6;
	EventToStringTable[EM_KEY_NUMPAD7]="EM_KEY_NUMPAD7";
	StringToEventTable["EM_KEY_NUMPAD7"]=EM_KEY_NUMPAD7;
	EventToStringTable[EM_KEY_UPNUMPAD7]="EM_KEY_UPNUMPAD7";
	StringToEventTable["EM_KEY_UPNUMPAD7"]=EM_KEY_UPNUMPAD7;
	EventToStringTable[EM_KEY_NUMPAD8]="EM_KEY_NUMPAD8";
	StringToEventTable["EM_KEY_NUMPAD8"]=EM_KEY_NUMPAD8;
	EventToStringTable[EM_KEY_UPNUMPAD8]="EM_KEY_UPNUMPAD8";
	StringToEventTable["EM_KEY_UPNUMPAD8"]=EM_KEY_UPNUMPAD8;
	EventToStringTable[EM_KEY_NUMPAD9]="EM_KEY_NUMPAD9";
	StringToEventTable["EM_KEY_NUMPAD9"]=EM_KEY_NUMPAD9;
	EventToStringTable[EM_KEY_UPNUMPAD9]="EM_KEY_UPNUMPAD9";
	StringToEventTable["EM_KEY_UPNUMPAD9"]=EM_KEY_UPNUMPAD9;
	EventToStringTable[EM_KEY_MULTIPLY]="EM_KEY_MULTIPLY";
	StringToEventTable["EM_KEY_MULTIPLY"]=EM_KEY_MULTIPLY;
	EventToStringTable[EM_KEY_ADD]="EM_KEY_ADD";
	StringToEventTable["EM_KEY_ADD"]=EM_KEY_ADD;
	EventToStringTable[EM_KEY_SEPARATOR]="EM_KEY_SEPARATOR";
	StringToEventTable["EM_KEY_SEPARATOR"]=EM_KEY_SEPARATOR;
	EventToStringTable[EM_KEY_SUBTRACT]="EM_KEY_SUBTRACT";
	StringToEventTable["EM_KEY_SUBTRACT"]=EM_KEY_SUBTRACT;
	EventToStringTable[EM_KEY_DECIMAL]="EM_KEY_DECIMAL";
	StringToEventTable["EM_KEY_DECIMAL"]=EM_KEY_DECIMAL;
	EventToStringTable[EM_KEY_UPDECIMAL]="EM_KEY_UPDECIMAL";
	StringToEventTable["EM_KEY_UPDECIMAL"]=EM_KEY_UPDECIMAL;
	EventToStringTable[EM_KEY_DIVIDE]="EM_KEY_DIVIDE";
	StringToEventTable["EM_KEY_DIVIDE"]=EM_KEY_DIVIDE;
	EventToStringTable[EM_KEY_NUMPADENTER]="EM_KEY_NUMPADENTER";
	StringToEventTable["EM_KEY_NUMPADENTER"]=EM_KEY_NUMPADENTER;
	EventToStringTable[EM_KEY_NUMPADEQUALS]="EM_KEY_NUMPADEQUALS";
	StringToEventTable["EM_KEY_NUMPADEQUALS"]=EM_KEY_NUMPADEQUALS;
	EventToStringTable[EM_KEY_F1]="EM_KEY_F1";
	StringToEventTable["EM_KEY_F1"]=EM_KEY_F1;
	EventToStringTable[EM_KEY_F2]="EM_KEY_F2";
	StringToEventTable["EM_KEY_F2"]=EM_KEY_F2;
	EventToStringTable[EM_KEY_F3]="EM_KEY_F3";
	StringToEventTable["EM_KEY_F3"]=EM_KEY_F3;
	EventToStringTable[EM_KEY_F4]="EM_KEY_F4";
	StringToEventTable["EM_KEY_F4"]=EM_KEY_F4;
	EventToStringTable[EM_KEY_F5]="EM_KEY_F5";
	StringToEventTable["EM_KEY_F5"]=EM_KEY_F5;
	EventToStringTable[EM_KEY_F6]="EM_KEY_F6";
	StringToEventTable["EM_KEY_F6"]=EM_KEY_F6;
	EventToStringTable[EM_KEY_F7]="EM_KEY_F7";
	StringToEventTable["EM_KEY_F7"]=EM_KEY_F7;
	EventToStringTable[EM_KEY_F8]="EM_KEY_F8";
	StringToEventTable["EM_KEY_F8"]=EM_KEY_F8;
	EventToStringTable[EM_KEY_F9]="EM_KEY_F9";
	StringToEventTable["EM_KEY_F9"]=EM_KEY_F9;
	EventToStringTable[EM_KEY_F10]="EM_KEY_F10";
	StringToEventTable["EM_KEY_F10"]=EM_KEY_F10;
	EventToStringTable[EM_KEY_F11]="EM_KEY_F11";
	StringToEventTable["EM_KEY_F11"]=EM_KEY_F11;
	EventToStringTable[EM_KEY_F12]="EM_KEY_F12";
	StringToEventTable["EM_KEY_F12"]=EM_KEY_F12;
	EventToStringTable[EM_KEY_F13]="EM_KEY_F13";
	StringToEventTable["EM_KEY_F13"]=EM_KEY_F13;
	EventToStringTable[EM_KEY_F14]="EM_KEY_F14";
	StringToEventTable["EM_KEY_F14"]=EM_KEY_F14;
	EventToStringTable[EM_KEY_F15]="EM_KEY_F15";
	StringToEventTable["EM_KEY_F15"]=EM_KEY_F15;
	EventToStringTable[EM_KEY_NUMLOCK]="EM_KEY_NUMLOCK";
	StringToEventTable["EM_KEY_NUMLOCK"]=EM_KEY_NUMLOCK;
	EventToStringTable[EM_KEY_SCROLLLOCK]="EM_KEY_SCROLLLOCK";
	StringToEventTable["EM_KEY_SCROLLLOCK"]=EM_KEY_SCROLLLOCK;
	EventToStringTable[EM_KEY_LSHIFT]="EM_KEY_LSHIFT";
	StringToEventTable["EM_KEY_LSHIFT"]=EM_KEY_LSHIFT;
	EventToStringTable[EM_KEY_RSHIFT]="EM_KEY_RSHIFT";
	StringToEventTable["EM_KEY_RSHIFT"]=EM_KEY_RSHIFT;
	EventToStringTable[EM_KEY_LCONTROL]="EM_KEY_LCONTROL";
	StringToEventTable["EM_KEY_LCONTROL"]=EM_KEY_LCONTROL;
	EventToStringTable[EM_KEY_RCONTROL]="EM_KEY_RCONTROL";
	StringToEventTable["EM_KEY_RCONTROL"]=EM_KEY_RCONTROL;
	EventToStringTable[EM_KEY_LALT]="EM_KEY_LALT";
	StringToEventTable["EM_KEY_LALT"]=EM_KEY_LALT;
	EventToStringTable[EM_KEY_RALT]="EM_KEY_RALT";
	StringToEventTable["EM_KEY_RALT"]=EM_KEY_RALT;
	EventToStringTable[EM_KEY_TILDE]="EM_KEY_TILDE";
	StringToEventTable["EM_KEY_TILDE"]=EM_KEY_TILDE;
	EventToStringTable[EM_KEY_UPTILDE]="EM_KEY_UPTILDE";
	StringToEventTable["EM_KEY_UPTILDE"]=EM_KEY_UPTILDE;
	EventToStringTable[EM_KEY_MINUS]="EM_KEY_MINUS";
	StringToEventTable["EM_KEY_MINUS"]=EM_KEY_MINUS;
	EventToStringTable[EM_KEY_UPMINUS]="EM_KEY_UPMINUS";
	StringToEventTable["EM_KEY_UPMINUS"]=EM_KEY_UPMINUS;
	EventToStringTable[EM_KEY_EQUALS]="EM_KEY_EQUALS";
	StringToEventTable["EM_KEY_EQUALS"]=EM_KEY_EQUALS;
	EventToStringTable[EM_KEY_UPEQUALS]="EM_KEY_UPEQUALS";
	StringToEventTable["EM_KEY_UPEQUALS"]=EM_KEY_UPEQUALS;
	EventToStringTable[EM_KEY_LBRACKET]="EM_KEY_LBRACKET";
	StringToEventTable["EM_KEY_LBRACKET"]=EM_KEY_LBRACKET;
	EventToStringTable[EM_KEY_UPLBRACKET]="EM_KEY_UPLBRACKET";
	StringToEventTable["EM_KEY_UPLBRACKET"]=EM_KEY_UPLBRACKET;
	EventToStringTable[EM_KEY_RBRACKET]="EM_KEY_RBRACKET";
	StringToEventTable["EM_KEY_RBRACKET"]=EM_KEY_RBRACKET;
	EventToStringTable[EM_KEY_UPRBRACKET]="EM_KEY_UPRBRACKET";
	StringToEventTable["EM_KEY_UPRBRACKET"]=EM_KEY_UPRBRACKET;
	EventToStringTable[EM_KEY_BACKSLASH]="EM_KEY_BACKSLASH";
	StringToEventTable["EM_KEY_BACKSLASH"]=EM_KEY_BACKSLASH;
	EventToStringTable[EM_KEY_UPBACKSLASH]="EM_KEY_UPBACKSLASH";
	StringToEventTable["EM_KEY_UPBACKSLASH"]=EM_KEY_UPBACKSLASH;
	EventToStringTable[EM_KEY_SEMICOLON]="EM_KEY_SEMICOLON";
	StringToEventTable["EM_KEY_SEMICOLON"]=EM_KEY_SEMICOLON;
	EventToStringTable[EM_KEY_UPSEMICOLON]="EM_KEY_UPSEMICOLON";
	StringToEventTable["EM_KEY_UPSEMICOLON"]=EM_KEY_UPSEMICOLON;
	EventToStringTable[EM_KEY_APOSTROPHE]="EM_KEY_APOSTROPHE";
	StringToEventTable["EM_KEY_APOSTROPHE"]=EM_KEY_APOSTROPHE;
	EventToStringTable[EM_KEY_UPAPOSTROPHE]="EM_KEY_UPAPOSTROPHE";
	StringToEventTable["EM_KEY_UPAPOSTROPHE"]=EM_KEY_UPAPOSTROPHE;
	EventToStringTable[EM_KEY_GRAVE]="EM_KEY_GRAVE";
	StringToEventTable["EM_KEY_GRAVE"]=EM_KEY_GRAVE;
	EventToStringTable[EM_KEY_UPGRAVE]="EM_KEY_UPGRAVE";
	StringToEventTable["EM_KEY_UPGRAVE"]=EM_KEY_UPGRAVE;
	EventToStringTable[EM_KEY_COMMA]="EM_KEY_COMMA";
	StringToEventTable["EM_KEY_COMMA"]=EM_KEY_COMMA;
	EventToStringTable[EM_KEY_UPCOMMA]="EM_KEY_UPCOMMA";
	StringToEventTable["EM_KEY_UPCOMMA"]=EM_KEY_UPCOMMA;
	EventToStringTable[EM_KEY_PERIOD]="EM_KEY_PERIOD";
	StringToEventTable["EM_KEY_PERIOD"]=EM_KEY_PERIOD;
	EventToStringTable[EM_KEY_UPPERIOD]="EM_KEY_UPPERIOD";
	StringToEventTable["EM_KEY_UPPERIOD"]=EM_KEY_UPPERIOD;
	EventToStringTable[EM_KEY_SLASH]="EM_KEY_SLASH";
	StringToEventTable["EM_KEY_SLASH"]=EM_KEY_SLASH;
	EventToStringTable[EM_KEY_UPSLASH]="EM_KEY_UPSLASH";
	StringToEventTable["EM_KEY_UPSLASH"]=EM_KEY_UPSLASH;
	EventToStringTable[EM_KEY_0]="EM_KEY_0";
	StringToEventTable["EM_KEY_0"]=EM_KEY_0;
	EventToStringTable[EM_KEY_UP0]="EM_KEY_UP0";
	StringToEventTable["EM_KEY_UP0"]=EM_KEY_UP0;
	EventToStringTable[EM_KEY_1]="EM_KEY_1";
	StringToEventTable["EM_KEY_1"]=EM_KEY_1;
	EventToStringTable[EM_KEY_UP1]="EM_KEY_UP1";
	StringToEventTable["EM_KEY_UP1"]=EM_KEY_UP1;
	EventToStringTable[EM_KEY_2]="EM_KEY_2";
	StringToEventTable["EM_KEY_2"]=EM_KEY_2;
	EventToStringTable[EM_KEY_UP2]="EM_KEY_UP2";
	StringToEventTable["EM_KEY_UP2"]=EM_KEY_UP2;
	EventToStringTable[EM_KEY_3]="EM_KEY_3";
	StringToEventTable["EM_KEY_3"]=EM_KEY_3;
	EventToStringTable[EM_KEY_UP3]="EM_KEY_UP3";
	StringToEventTable["EM_KEY_UP3"]=EM_KEY_UP3;
	EventToStringTable[EM_KEY_4]="EM_KEY_4";
	StringToEventTable["EM_KEY_4"]=EM_KEY_4;
	EventToStringTable[EM_KEY_UP4]="EM_KEY_UP4";
	StringToEventTable["EM_KEY_UP4"]=EM_KEY_UP4;
	EventToStringTable[EM_KEY_5]="EM_KEY_5";
	StringToEventTable["EM_KEY_5"]=EM_KEY_5;
	EventToStringTable[EM_KEY_UP5]="EM_KEY_UP5";
	StringToEventTable["EM_KEY_UP5"]=EM_KEY_UP5;
	EventToStringTable[EM_KEY_6]="EM_KEY_6";
	StringToEventTable["EM_KEY_6"]=EM_KEY_6;
	EventToStringTable[EM_KEY_UP6]="EM_KEY_UP6";
	StringToEventTable["EM_KEY_UP6"]=EM_KEY_UP6;
	EventToStringTable[EM_KEY_7]="EM_KEY_7";
	StringToEventTable["EM_KEY_7"]=EM_KEY_7;
	EventToStringTable[EM_KEY_UP7]="EM_KEY_UP7";
	StringToEventTable["EM_KEY_UP7"]=EM_KEY_UP7;
	EventToStringTable[EM_KEY_8]="EM_KEY_8";
	StringToEventTable["EM_KEY_8"]=EM_KEY_8;
	EventToStringTable[EM_KEY_UP8]="EM_KEY_UP8";
	StringToEventTable["EM_KEY_UP8"]=EM_KEY_UP8;
	EventToStringTable[EM_KEY_9]="EM_KEY_9";
	StringToEventTable["EM_KEY_9"]=EM_KEY_9;
	EventToStringTable[EM_KEY_UP9]="EM_KEY_UP9";
	StringToEventTable["EM_KEY_UP9"]=EM_KEY_UP9;
	EventToStringTable[EM_KEY_Z]="EM_KEY_Z";
	StringToEventTable["EM_KEY_Z"]=EM_KEY_Z;
	EventToStringTable[EM_KEY_UPZ]="EM_KEY_UPZ";
	StringToEventTable["EM_KEY_UPZ"]=EM_KEY_UPZ;
	EventToStringTable[EM_KEY_Y]="EM_KEY_Y";
	StringToEventTable["EM_KEY_Y"]=EM_KEY_Y;
	EventToStringTable[EM_KEY_UPY]="EM_KEY_UPY";
	StringToEventTable["EM_KEY_UPY"]=EM_KEY_UPY;
	EventToStringTable[EM_KEY_X]="EM_KEY_X";
	StringToEventTable["EM_KEY_X"]=EM_KEY_X;
	EventToStringTable[EM_KEY_UPX]="EM_KEY_UPX";
	StringToEventTable["EM_KEY_UPX"]=EM_KEY_UPX;
	EventToStringTable[EM_KEY_W]="EM_KEY_W";
	StringToEventTable["EM_KEY_W"]=EM_KEY_W;
	EventToStringTable[EM_KEY_UPW]="EM_KEY_UPW";
	StringToEventTable["EM_KEY_UPW"]=EM_KEY_UPW;
	EventToStringTable[EM_KEY_V]="EM_KEY_V";
	StringToEventTable["EM_KEY_V"]=EM_KEY_V;
	EventToStringTable[EM_KEY_UPV]="EM_KEY_UPV";
	StringToEventTable["EM_KEY_UPV"]=EM_KEY_UPV;
	EventToStringTable[EM_KEY_U]="EM_KEY_U";
	StringToEventTable["EM_KEY_U"]=EM_KEY_U;
	EventToStringTable[EM_KEY_UPU]="EM_KEY_UPU";
	StringToEventTable["EM_KEY_UPU"]=EM_KEY_UPU;
	EventToStringTable[EM_KEY_T]="EM_KEY_T";
	StringToEventTable["EM_KEY_T"]=EM_KEY_T;
	EventToStringTable[EM_KEY_UPT]="EM_KEY_UPT";
	StringToEventTable["EM_KEY_UPT"]=EM_KEY_UPT;
	EventToStringTable[EM_KEY_S]="EM_KEY_S";
	StringToEventTable["EM_KEY_S"]=EM_KEY_S;
	EventToStringTable[EM_KEY_UPS]="EM_KEY_UPS";
	StringToEventTable["EM_KEY_UPS"]=EM_KEY_UPS;
	EventToStringTable[EM_KEY_R]="EM_KEY_R";
	StringToEventTable["EM_KEY_R"]=EM_KEY_R;
	EventToStringTable[EM_KEY_UPR]="EM_KEY_UPR";
	StringToEventTable["EM_KEY_UPR"]=EM_KEY_UPR;
	EventToStringTable[EM_KEY_Q]="EM_KEY_Q";
	StringToEventTable["EM_KEY_Q"]=EM_KEY_Q;
	EventToStringTable[EM_KEY_UPQ]="EM_KEY_UPQ";
	StringToEventTable["EM_KEY_UPQ"]=EM_KEY_UPQ;
	EventToStringTable[EM_KEY_P]="EM_KEY_P";
	StringToEventTable["EM_KEY_P"]=EM_KEY_P;
	EventToStringTable[EM_KEY_UPP]="EM_KEY_UPP";
	StringToEventTable["EM_KEY_UPP"]=EM_KEY_UPP;
	EventToStringTable[EM_KEY_O]="EM_KEY_O";
	StringToEventTable["EM_KEY_O"]=EM_KEY_O;
	EventToStringTable[EM_KEY_UPO]="EM_KEY_UPO";
	StringToEventTable["EM_KEY_UPO"]=EM_KEY_UPO;
	EventToStringTable[EM_KEY_N]="EM_KEY_N";
	StringToEventTable["EM_KEY_N"]=EM_KEY_N;
	EventToStringTable[EM_KEY_UPN]="EM_KEY_UPN";
	StringToEventTable["EM_KEY_UPN"]=EM_KEY_UPN;
	EventToStringTable[EM_KEY_M]="EM_KEY_M";
	StringToEventTable["EM_KEY_M"]=EM_KEY_M;
	EventToStringTable[EM_KEY_UPM]="EM_KEY_UPM";
	StringToEventTable["EM_KEY_UPM"]=EM_KEY_UPM;
	EventToStringTable[EM_KEY_L]="EM_KEY_L";
	StringToEventTable["EM_KEY_L"]=EM_KEY_L;
	EventToStringTable[EM_KEY_UPL]="EM_KEY_UPL";
	StringToEventTable["EM_KEY_UPL"]=EM_KEY_UPL;
	EventToStringTable[EM_KEY_K]="EM_KEY_K";
	StringToEventTable["EM_KEY_K"]=EM_KEY_K;
	EventToStringTable[EM_KEY_UPK]="EM_KEY_UPK";
	StringToEventTable["EM_KEY_UPK"]=EM_KEY_UPK;
	EventToStringTable[EM_KEY_J]="EM_KEY_J";
	StringToEventTable["EM_KEY_J"]=EM_KEY_J;
	EventToStringTable[EM_KEY_UPJ]="EM_KEY_UPJ";
	StringToEventTable["EM_KEY_UPJ"]=EM_KEY_UPJ;
	EventToStringTable[EM_KEY_I]="EM_KEY_I";
	StringToEventTable["EM_KEY_I"]=EM_KEY_I;
	EventToStringTable[EM_KEY_UPI]="EM_KEY_UPI";
	StringToEventTable["EM_KEY_UPI"]=EM_KEY_UPI;
	EventToStringTable[EM_KEY_H]="EM_KEY_H";
	StringToEventTable["EM_KEY_H"]=EM_KEY_H;
	EventToStringTable[EM_KEY_UPH]="EM_KEY_UPH";
	StringToEventTable["EM_KEY_UPH"]=EM_KEY_UPH;
	EventToStringTable[EM_KEY_G]="EM_KEY_G";
	StringToEventTable["EM_KEY_G"]=EM_KEY_G;
	EventToStringTable[EM_KEY_UPG]="EM_KEY_UPG";
	StringToEventTable["EM_KEY_UPG"]=EM_KEY_UPG;
	EventToStringTable[EM_KEY_F]="EM_KEY_F";
	StringToEventTable["EM_KEY_F"]=EM_KEY_F;
	EventToStringTable[EM_KEY_UPF]="EM_KEY_UPF";
	StringToEventTable["EM_KEY_UPF"]=EM_KEY_UPF;
	EventToStringTable[EM_KEY_E]="EM_KEY_E";
	StringToEventTable["EM_KEY_E"]=EM_KEY_E;
	EventToStringTable[EM_KEY_UPE]="EM_KEY_UPE";
	StringToEventTable["EM_KEY_UPE"]=EM_KEY_UPE;
	EventToStringTable[EM_KEY_D]="EM_KEY_D";
	StringToEventTable["EM_KEY_D"]=EM_KEY_D;
	EventToStringTable[EM_KEY_UPD]="EM_KEY_UPD";
	StringToEventTable["EM_KEY_UPD"]=EM_KEY_UPD;
	EventToStringTable[EM_KEY_C]="EM_KEY_C";
	StringToEventTable["EM_KEY_C"]=EM_KEY_C;
	EventToStringTable[EM_KEY_UPC]="EM_KEY_UPC";
	StringToEventTable["EM_KEY_UPC"]=EM_KEY_UPC;
	EventToStringTable[EM_KEY_B]="EM_KEY_B";
	StringToEventTable["EM_KEY_B"]=EM_KEY_B;
	EventToStringTable[EM_KEY_UPB]="EM_KEY_UPB";
	StringToEventTable["EM_KEY_UPB"]=EM_KEY_UPB;
	EventToStringTable[EM_KEY_A]="EM_KEY_A";
	StringToEventTable["EM_KEY_A"]=EM_KEY_A;
	EventToStringTable[EM_KEY_UPA]="EM_KEY_UPA";
	StringToEventTable["EM_KEY_UPA"]=EM_KEY_UPA;
	EventToStringTable[EM_KEY]="EM_KEY";
	StringToEventTable["EM_KEY"]=EM_KEY;
	EventToStringTable[EM_MOUSE_LEFTDOWN]="EM_MOUSE_LEFTDOWN";
	StringToEventTable["EM_MOUSE_LEFTDOWN"]=EM_MOUSE_LEFTDOWN;
	EventToStringTable[EM_MOUSE_LEFTUP]="EM_MOUSE_LEFTUP";
	StringToEventTable["EM_MOUSE_LEFTUP"]=EM_MOUSE_LEFTUP;
	EventToStringTable[EM_MOUSE_LEFTCLICK]="EM_MOUSE_LEFTCLICK";
	StringToEventTable["EM_MOUSE_LEFTCLICK"]=EM_MOUSE_LEFTCLICK;
	EventToStringTable[EM_MOUSE_LEFTDBCLICK]="EM_MOUSE_LEFTDBCLICK";
	StringToEventTable["EM_MOUSE_LEFTDBCLICK"]=EM_MOUSE_LEFTDBCLICK;
	EventToStringTable[EM_MOUSE_RIGHTDOWN]="EM_MOUSE_RIGHTDOWN";
	StringToEventTable["EM_MOUSE_RIGHTDOWN"]=EM_MOUSE_RIGHTDOWN;
	EventToStringTable[EM_MOUSE_RIGHTUP]="EM_MOUSE_RIGHTUP";
	StringToEventTable["EM_MOUSE_RIGHTUP"]=EM_MOUSE_RIGHTUP;
	EventToStringTable[EM_MOUSE_RIGHTCLICK]="EM_MOUSE_RIGHTCLICK";
	StringToEventTable["EM_MOUSE_RIGHTCLICK"]=EM_MOUSE_RIGHTCLICK;
	EventToStringTable[EM_MOUSE_RIGHTDBCLICK]="EM_MOUSE_RIGHTDBCLICK";
	StringToEventTable["EM_MOUSE_RIGHTDBCLICK"]=EM_MOUSE_RIGHTDBCLICK;
	EventToStringTable[EM_MOUSE_MIDDLEDOWN]="EM_MOUSE_MIDDLEDOWN";
	StringToEventTable["EM_MOUSE_MIDDLEDOWN"]=EM_MOUSE_MIDDLEDOWN;
	EventToStringTable[EM_MOUSE_MIDDLEUP]="EM_MOUSE_MIDDLEUP";
	StringToEventTable["EM_MOUSE_MIDDLEUP"]=EM_MOUSE_MIDDLEUP;
	EventToStringTable[EM_MOUSE_MIDDLECLICK]="EM_MOUSE_MIDDLECLICK";
	StringToEventTable["EM_MOUSE_MIDDLECLICK"]=EM_MOUSE_MIDDLECLICK;
	EventToStringTable[EM_MOUSE_MIDDLEDBCLICK]="EM_MOUSE_MIDDLEDBCLICK";
	StringToEventTable["EM_MOUSE_MIDDLEDBCLICK"]=EM_MOUSE_MIDDLEDBCLICK;
	EventToStringTable[EM_MOUSE_LEFTDRAGBEGIN]="EM_MOUSE_LEFTDRAGBEGIN";
	StringToEventTable["EM_MOUSE_LEFTDRAGBEGIN"]=EM_MOUSE_LEFTDRAGBEGIN;
	EventToStringTable[EM_MOUSE_LEFTDRAGEND]="EM_MOUSE_LEFTDRAGEND";
	StringToEventTable["EM_MOUSE_LEFTDRAGEND"]=EM_MOUSE_LEFTDRAGEND;
	EventToStringTable[EM_MOUSE_RIGHTDRAGBEGIN]="EM_MOUSE_RIGHTDRAGBEGIN";
	StringToEventTable["EM_MOUSE_RIGHTDRAGBEGIN"]=EM_MOUSE_RIGHTDRAGBEGIN;
	EventToStringTable[EM_MOUSE_RIGHTDRAGEND]="EM_MOUSE_RIGHTDRAGEND";
	StringToEventTable["EM_MOUSE_RIGHTDRAGEND"]=EM_MOUSE_RIGHTDRAGEND;
	EventToStringTable[EM_MOUSE_MIDDLEDRAGBEGIN]="EM_MOUSE_MIDDLEDRAGBEGIN";
	StringToEventTable["EM_MOUSE_MIDDLEDRAGBEGIN"]=EM_MOUSE_MIDDLEDRAGBEGIN;
	EventToStringTable[EM_MOUSE_MIDDLEDRAGEND]="EM_MOUSE_MIDDLEDRAGEND";
	StringToEventTable["EM_MOUSE_MIDDLEDRAGEND"]=EM_MOUSE_MIDDLEDRAGEND;
	EventToStringTable[EM_MOUSE_DRAGBEGIN]="EM_MOUSE_DRAGBEGIN";
	StringToEventTable["EM_MOUSE_DRAGBEGIN"]=EM_MOUSE_DRAGBEGIN;
	EventToStringTable[EM_MOUSE_DRAGOVER]="EM_MOUSE_DRAGOVER";
	StringToEventTable["EM_MOUSE_DRAGOVER"]=EM_MOUSE_DRAGOVER;
	EventToStringTable[EM_MOUSE_DRAGEND]="EM_MOUSE_DRAGEND";
	StringToEventTable["EM_MOUSE_DRAGEND"]=EM_MOUSE_DRAGEND;
	EventToStringTable[EM_MOUSE_MOVE]="EM_MOUSE_MOVE";
	StringToEventTable["EM_MOUSE_MOVE"]=EM_MOUSE_MOVE;
	EventToStringTable[EM_MOUSE_HOVER]="EM_MOUSE_HOVER";
	StringToEventTable["EM_MOUSE_HOVER"]=EM_MOUSE_HOVER;
	EventToStringTable[EM_MOUSE_WHEEL]="EM_MOUSE_WHEEL";
	StringToEventTable["EM_MOUSE_WHEEL"]=EM_MOUSE_WHEEL;
	EventToStringTable[EM_MOUSE_ENTER]="EM_MOUSE_ENTER";
	StringToEventTable["EM_MOUSE_ENTER"]=EM_MOUSE_ENTER;
	EventToStringTable[EM_MOUSE_LEAVE]="EM_MOUSE_LEAVE";
	StringToEventTable["EM_MOUSE_LEAVE"]=EM_MOUSE_LEAVE;
	EventToStringTable[EM_MOUSE_DOWN]="EM_MOUSE_DOWN";
	StringToEventTable["EM_MOUSE_DOWN"]=EM_MOUSE_DOWN;
	EventToStringTable[EM_MOUSE_UP]="EM_MOUSE_UP";
	StringToEventTable["EM_MOUSE_UP"]=EM_MOUSE_UP;
	EventToStringTable[EM_MOUSE_CLICK]="EM_MOUSE_CLICK";
	StringToEventTable["EM_MOUSE_CLICK"]=EM_MOUSE_CLICK;
	EventToStringTable[EM_MOUSE_DBCLICK]="EM_MOUSE_DBCLICK";
	StringToEventTable["EM_MOUSE_DBCLICK"]=EM_MOUSE_DBCLICK;
	EventToStringTable[EM_MOUSE_LEFT]="EM_MOUSE_LEFT";
	StringToEventTable["EM_MOUSE_LEFT"]=EM_MOUSE_LEFT;
	EventToStringTable[EM_MOUSE_RIGHT]="EM_MOUSE_RIGHT";
	StringToEventTable["EM_MOUSE_RIGHT"]=EM_MOUSE_RIGHT;
	EventToStringTable[EM_MOUSE_MIDDLE]="EM_MOUSE_MIDDLE";
	StringToEventTable["EM_MOUSE_MIDDLE"]=EM_MOUSE_MIDDLE;
	EventToStringTable[EM_MOUSE]="EM_MOUSE";
	StringToEventTable["EM_MOUSE"]=EM_MOUSE;

	//building the scan code mapping table
	ZeroMemory(ScancodeToKeyTable,sizeof(ScancodeToKeyTable));
	ScancodeToKeyTable[(int)EVirtualKey::KEY_ESCAPE]=EM_KEY_ESCAPE;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_1]=EM_KEY_1;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_2]=EM_KEY_2;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_3]=EM_KEY_3;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_4]=EM_KEY_4;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_5]=EM_KEY_5;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_6]=EM_KEY_6;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_7]=EM_KEY_7;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_8]=EM_KEY_8;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_9]=EM_KEY_9;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_0]=EM_KEY_0;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_MINUS]=EM_KEY_MINUS;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_EQUALS]=EM_KEY_EQUALS;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_BACK]=EM_KEY_BACKSPACE;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_TAB]=EM_KEY_TAB;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_A]=EM_KEY_A;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_B]=EM_KEY_B;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_C]=EM_KEY_C;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_D]=EM_KEY_D;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_E]=EM_KEY_E;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F]=EM_KEY_F;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_G]=EM_KEY_G;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_H]=EM_KEY_H;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_I]=EM_KEY_I;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_J]=EM_KEY_J;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_K]=EM_KEY_K;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_L]=EM_KEY_L;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_M]=EM_KEY_M;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_N]=EM_KEY_N;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_O]=EM_KEY_O;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_P]=EM_KEY_P;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_Q]=EM_KEY_Q;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_R]=EM_KEY_R;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_S]=EM_KEY_S;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_T]=EM_KEY_T;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_U]=EM_KEY_U;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_V]=EM_KEY_V;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_W]=EM_KEY_W;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_X]=EM_KEY_X;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_Y]=EM_KEY_Y;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_Z]=EM_KEY_Z;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_LBRACKET]=EM_KEY_LBRACKET;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_RBRACKET]=EM_KEY_RBRACKET;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_RSHIFT]=EM_KEY_RSHIFT;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_LSHIFT]=EM_KEY_LSHIFT;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_LCONTROL]=EM_KEY_LCONTROL;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_RCONTROL]=EM_KEY_RCONTROL;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_LMENU]=EM_KEY_RALT;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_LMENU]=EM_KEY_LALT;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_RETURN]=EM_KEY_RETURN;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_SEMICOLON]=EM_KEY_SEMICOLON;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_APOSTROPHE]=EM_KEY_APOSTROPHE;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_GRAVE]=EM_KEY_GRAVE;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_BACKSLASH]=EM_KEY_BACKSLASH;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_COMMA]=EM_KEY_COMMA;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_PERIOD]=EM_KEY_PERIOD;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_SLASH]=EM_KEY_SLASH;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_MULTIPLY]=EM_KEY_MULTIPLY;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_SPACE]=EM_KEY_SPACE;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_CAPITAL]=EM_KEY_CAPSLOCK;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F1]=EM_KEY_F1;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F2]=EM_KEY_F2;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F3]=EM_KEY_F3;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F4]=EM_KEY_F4;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F5]=EM_KEY_F5;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F6]=EM_KEY_F6;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F7]=EM_KEY_F7;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F8]=EM_KEY_F8;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F9]=EM_KEY_F9;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F10]=EM_KEY_F10;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F11]=EM_KEY_F11;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F12]=EM_KEY_F12;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F13] = EM_KEY_F13;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F14] = EM_KEY_F14;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_F15] = EM_KEY_F15;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMLOCK]=EM_KEY_NUMLOCK;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_SCROLL]=EM_KEY_SCROLLLOCK;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD0]=EM_KEY_NUMPAD0;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD1]=EM_KEY_NUMPAD1;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD2]=EM_KEY_NUMPAD2;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD3]=EM_KEY_NUMPAD3;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD4]=EM_KEY_NUMPAD4;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD5]=EM_KEY_NUMPAD5;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD6]=EM_KEY_NUMPAD6;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD7]=EM_KEY_NUMPAD7;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD8]=EM_KEY_NUMPAD8;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPAD9]=EM_KEY_NUMPAD9;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPADENTER]=EM_KEY_NUMPADENTER;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NUMPADENTER]=EM_KEY_NUMPADEQUALS;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_SUBTRACT]=EM_KEY_SUBTRACT;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_ADD]=EM_KEY_ADD;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_DECIMAL]=EM_KEY_DECIMAL;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_PAUSE]=EM_KEY_PAUSE;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_DIVIDE]=EM_KEY_DIVIDE;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_HOME]=EM_KEY_HOME;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_UP]=EM_KEY_UP;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_LEFT]=EM_KEY_LEFT;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_RIGHT]=EM_KEY_RIGHT;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_DOWN]=EM_KEY_DOWN;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_END]=EM_KEY_END;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_PRIOR]=EM_KEY_PAGE_UP;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_NEXT]=EM_KEY_PAGE_DOWN;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_INSERT]=EM_KEY_INSERT;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_DELETE]=EM_KEY_DELETE;

	ScancodeToKeyTable[(int)EVirtualKey::KEY_SHIFT] = EM_KEY_SHIFT;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_CONTROL] = EM_KEY_CONTROL;
	ScancodeToKeyTable[(int)EVirtualKey::KEY_ALT] = EM_KEY_ALT;
}

CEventBinding::CEventBinding()
{
	m_pEventEnable.reset();
	m_pEventToScript.clear();
}
CEventBinding::~CEventBinding()
{
}

CEventBinding::CEventBinding(const CEventBinding& eventbinding)
{
	eventbinding.Clone(this);
}
CEventBinding & CEventBinding::operator =(CEventBinding &input)
{
	input.Clone(this);
	return *this;
}

void CEventBinding::Clone(IObject* pobj)const
{
	PE_ASSERT(pobj!=NULL);
	if (pobj==NULL) {
		return;
	}
	// PERF_BEGIN("Event Binding Clone");
	CEventBinding *output=(CEventBinding *)pobj;
	output->m_pEventEnable=m_pEventEnable;
	output->m_pEventToScript=m_pEventToScript;
	//copy the event mapping table
	output->m_pEventMappingTable = m_pEventMappingTable;
	// PERF_END("Event Binding Clone");
}

IObject* CEventBinding::Clone()const
{
	CEventBinding* pobj=new CEventBinding();
	Clone(pobj);
	return pobj;
}

int CEventBinding::Release()
{
	delete this;
	return 0;
}

bool CEventBinding::Equals(const IObject *obj)const
{
	return obj==this;
}
int CEventBinding::StringToEventValue(const string &str)
{
	map<string,int>::iterator iter;
	if ((iter=StringToEventTable.find(str))!=StringToEventTable.end()) {
		return iter->second;
	}
	return 0;
}
string& CEventBinding::EventValueToString(int value)
{
	map<int,string>::iterator iter;
	if ((iter=EventToStringTable.find(value))!=EventToStringTable.end()) {
		return iter->second;
	}

	return EventToStringTable[0];
}

bool CEventBinding::IsMapTo(int eSrcEvent, int eDestEvent)const
{
	if (!m_pEventEnable.test(eSrcEvent)||!m_pEventEnable.test(eDestEvent)) {
		return false;
	}
	if (eSrcEvent==eDestEvent) {
		return true;
	}
	if (m_pEventMappingTable.c_ref()[eSrcEvent].find(eDestEvent)!=m_pEventMappingTable.c_ref()[eSrcEvent].end()){
		return true;
	}else{
		return false;
	}
}


void CEventBinding::DisableEvent(int eEvent)
{
	m_pEventEnable[eEvent]=false;
}

void CEventBinding::EnableEvent(int eEvent)
{
	m_pEventEnable[eEvent]=true;
}

void CEventBinding::MapEvent(int eSrcEvent, int eDestEvent)
{
	(*m_pEventMappingTable)[eSrcEvent].insert(eDestEvent);
	//m_pEventMappingTable[eSrcEvent][eDestEvent]=eDestEvent;
}

void CEventBinding::UnmapEvent(int eSrcEvent, int eDestEvent)
{
	(*m_pEventMappingTable)[eSrcEvent].erase(eDestEvent);
}

void CEventBinding::InitEventMappingTable(bool bDisable/* =false */)
{
	if(!m_pEventMappingTable)
	{
		EventMapping_type* pEventMappingTable = new EventMapping_type();
		for (int a=0;a<EventCount;a++) 
		{
			(*pEventMappingTable)[a].clear();
			(*pEventMappingTable)[a].insert(a);
		}
		m_pEventMappingTable = EventMapping_cow_type(pEventMappingTable);
	}
	else
	{
		for (int a=0;a<EventCount;a++) 
		{
			(*m_pEventMappingTable)[a].clear();
			(*m_pEventMappingTable)[a].insert(a);
		}
	}
	
	if (bDisable) {
		m_pEventEnable.reset();
	}else{
		m_pEventEnable.set();
	}
}


void CEventBinding::MapEventToScript(int eEvent,const SimpleScript *script)
{
	if (script==NULL) {
		m_pEventToScript.erase(eEvent);
	}else
		m_pEventToScript[eEvent]=*script;
}

void CEventBinding::UnmapEventToScript(int eEvent)
{
	if (m_pEventToScript.find(eEvent)!=m_pEventToScript.end()) {
		m_pEventToScript.erase(eEvent);
	}
}

bool CEventBinding::HasEventScript(int eEvent)const
{
	return (m_pEventToScript.find(eEvent)!=m_pEventToScript.end());
}

const SimpleScript *CEventBinding::GetEventScript(int eEvent)const
{
	map<int,SimpleScript>::const_iterator iter;
	if ((iter=m_pEventToScript.find(eEvent))!=m_pEventToScript.end()) {
		return &iter->second;
	}else
		return NULL;
}

void CEventBinding::DefaultMap_Control()
{
	//control key mapping
	MapEvent(EM_KEY_LSHIFT,EM_KEY_SHIFT);
	MapEvent(EM_KEY_RSHIFT,EM_KEY_SHIFT);
	MapEvent(EM_KEY_LALT,EM_KEY_ALT);
	MapEvent(EM_KEY_RALT,EM_KEY_ALT);
	MapEvent(EM_KEY_LCONTROL,EM_KEY_CONTROL);
	MapEvent(EM_KEY_RCONTROL,EM_KEY_CONTROL);
	MapEvent(EM_KEY_WIN_LWINDOW,EM_KEY_WIN_WINDOW);
	MapEvent(EM_KEY_WIN_RWINDOW,EM_KEY_WIN_WINDOW);

	//map Number pad control keys with corresponding control keys
//	MapEvent(EM_KEY_UPNUMPAD0,EM_KEY_INSERT);
//	MapEvent(EM_KEY_INSERT,EM_KEY_UPNUMPAD0);
//	MapEvent(EM_KEY_UPNUMPAD1,EM_KEY_END);
//	MapEvent(EM_KEY_END,EM_KEY_UPNUMPAD1);
//	MapEvent(EM_KEY_UPNUMPAD2,EM_KEY_DOWN);
//	MapEvent(EM_KEY_DOWN,EM_KEY_UPNUMPAD2);
//	MapEvent(EM_KEY_UPNUMPAD3,EM_KEY_PAGE_DOWN);
//	MapEvent(EM_KEY_PAGE_DOWN,EM_KEY_UPNUMPAD3);
//	MapEvent(EM_KEY_UPNUMPAD4,EM_KEY_LEFT);
//	MapEvent(EM_KEY_LEFT,EM_KEY_UPNUMPAD4);
//	MapEvent(EM_KEY_UPNUMPAD6,EM_KEY_RIGHT);
//	MapEvent(EM_KEY_RIGHT,EM_KEY_UPNUMPAD6);
//	MapEvent(EM_KEY_UPNUMPAD7,EM_KEY_HOME);
//	MapEvent(EM_KEY_HOME,EM_KEY_UPNUMPAD7);
//	MapEvent(EM_KEY_UPNUMPAD8,EM_KEY_UP);
//	MapEvent(EM_KEY_UP,EM_KEY_UPNUMPAD8);
//	MapEvent(EM_KEY_UPNUMPAD9,EM_KEY_PAGE_UP);
//	MapEvent(EM_KEY_PAGE_UP,EM_KEY_UPNUMPAD9);
//	MapEvent(EM_KEY_UPDECIMAL,EM_KEY_DELETE);
//	MapEvent(EM_KEY_DELETE,EM_KEY_UPDECIMAL);
//	MapEvent(EM_KEY_NUMPADENTER,EM_KEY_RETURN);
//	MapEvent(EM_KEY_RETURN,EM_KEY_NUMPADENTER);

	//common mapping
	int a;
	for (a=EM_KEY_BEGIN;a<EM_KEY;a++) {
		MapEvent(a,EM_KEY);
	}

	//map the text keys with their upper case.
	for (a=EM_KEY_TEXT_BEGIN;a<=EM_KEY_TEXT_END;a+=2 ) {
		MapEvent(a,a+1);
		MapEvent(a+1,a);
	}

}

void CEventBinding::DefaultMap_Text()
{
	//common mapping
	int a;
	for (a=EM_KEY_BEGIN;a<EM_KEY;a++) {
		MapEvent(a,EM_KEY);
	}
	//unmap the text keys with their upper case.
	for (a=EM_KEY_TEXT_BEGIN;a<=EM_KEY_TEXT_END;a++ ) {
		MapEvent(a,EM_NONE);
	}
	//map the number pad text keys with corresponding text keys.
//	MapEvent(EM_KEY_UPNUMPAD0,EM_KEY_0);
//	MapEvent(EM_KEY_0,EM_KEY_UPNUMPAD0);
//	MapEvent(EM_KEY_UPNUMPAD1,EM_KEY_1);
//	MapEvent(EM_KEY_1,EM_KEY_UPNUMPAD1);
//	MapEvent(EM_KEY_UPNUMPAD2,EM_KEY_2);
//	MapEvent(EM_KEY_2,EM_KEY_UPNUMPAD2);
//	MapEvent(EM_KEY_UPNUMPAD3,EM_KEY_3);
//	MapEvent(EM_KEY_3,EM_KEY_UPNUMPAD3);
//	MapEvent(EM_KEY_UPNUMPAD4,EM_KEY_4);
//	MapEvent(EM_KEY_4,EM_KEY_UPNUMPAD4);
//	MapEvent(EM_KEY_UPNUMPAD5,EM_KEY_5);
//	MapEvent(EM_KEY_5,EM_KEY_UPNUMPAD5);
//	MapEvent(EM_KEY_UPNUMPAD6,EM_KEY_6);
//	MapEvent(EM_KEY_6,EM_KEY_UPNUMPAD6);
//	MapEvent(EM_KEY_UPNUMPAD7,EM_KEY_7);
//	MapEvent(EM_KEY_7,EM_KEY_UPNUMPAD7);
//	MapEvent(EM_KEY_UPNUMPAD8,EM_KEY_8);
//	MapEvent(EM_KEY_8,EM_KEY_UPNUMPAD8);
//	MapEvent(EM_KEY_UPNUMPAD9,EM_KEY_9);
//	MapEvent(EM_KEY_9,EM_KEY_UPNUMPAD9);
	MapEvent(EM_KEY_DIVIDE,EM_KEY_SLASH);
	MapEvent(EM_KEY_SLASH,EM_KEY_DIVIDE);
	MapEvent(EM_KEY_MULTIPLY,EM_KEY_UP8);
	MapEvent(EM_KEY_UP8,EM_KEY_MULTIPLY);
	MapEvent(EM_KEY_SUBTRACT,EM_KEY_MINUS);
	MapEvent(EM_KEY_MINUS,EM_KEY_SUBTRACT);
	MapEvent(EM_KEY_ADD,EM_KEY_UPEQUALS);
	MapEvent(EM_KEY_UPEQUALS,EM_KEY_ADD);

}

void CEventBinding::DefaultMap_Mouse()
{
	//default mouse mapping is:
	//left, right amd middle's up, down, click and dbclick all map to up, down, click, dbclick respectively
	//only left dragbegin and dragend map to dragbegin and dragend
	int a;
	for (a=EM_MOUSE_BEGIN;a<EM_MOUSE;a++) {
		MapEvent(a,EM_MOUSE);
	}
	MapEvent(EM_MOUSE_LEFTDOWN,EM_MOUSE_DOWN);
	MapEvent(EM_MOUSE_RIGHTDOWN,EM_MOUSE_DOWN);
	MapEvent(EM_MOUSE_MIDDLEDOWN,EM_MOUSE_DOWN);
	MapEvent(EM_MOUSE_LEFTUP,EM_MOUSE_UP);
	MapEvent(EM_MOUSE_RIGHTUP,EM_MOUSE_UP);
	MapEvent(EM_MOUSE_MIDDLEUP,EM_MOUSE_UP);
	MapEvent(EM_MOUSE_LEFTCLICK,EM_MOUSE_CLICK);
	MapEvent(EM_MOUSE_RIGHTCLICK,EM_MOUSE_CLICK);
	MapEvent(EM_MOUSE_MIDDLECLICK,EM_MOUSE_CLICK);
	MapEvent(EM_MOUSE_LEFTDBCLICK,EM_MOUSE_DBCLICK);
	MapEvent(EM_MOUSE_RIGHTDBCLICK,EM_MOUSE_DBCLICK);
	MapEvent(EM_MOUSE_MIDDLEDBCLICK,EM_MOUSE_DBCLICK);
	MapEvent(EM_MOUSE_LEFTDRAGBEGIN,EM_MOUSE_DRAGBEGIN);
	MapEvent(EM_MOUSE_LEFTDRAGEND,EM_MOUSE_DRAGEND);
	MapEvent(EM_MOUSE_LEFTDOWN,EM_MOUSE_LEFT);
	MapEvent(EM_MOUSE_LEFTUP,EM_MOUSE_LEFT);
	MapEvent(EM_MOUSE_LEFTCLICK,EM_MOUSE_LEFT);
	MapEvent(EM_MOUSE_LEFTDBCLICK,EM_MOUSE_LEFT);
	MapEvent(EM_MOUSE_LEFTDRAGBEGIN,EM_MOUSE_LEFT);
	MapEvent(EM_MOUSE_LEFTDRAGEND,EM_MOUSE_LEFT);
	MapEvent(EM_MOUSE_RIGHTDOWN,EM_MOUSE_RIGHT);
	MapEvent(EM_MOUSE_RIGHTUP,EM_MOUSE_RIGHT);
	MapEvent(EM_MOUSE_RIGHTCLICK,EM_MOUSE_RIGHT);
	MapEvent(EM_MOUSE_RIGHTDBCLICK,EM_MOUSE_RIGHT);
	MapEvent(EM_MOUSE_RIGHTDRAGBEGIN,EM_MOUSE_RIGHT);
	MapEvent(EM_MOUSE_RIGHTDRAGEND,EM_MOUSE_RIGHT);
	MapEvent(EM_MOUSE_MIDDLEDOWN,EM_MOUSE_MIDDLE);
	MapEvent(EM_MOUSE_MIDDLEUP,EM_MOUSE_MIDDLE);
	MapEvent(EM_MOUSE_MIDDLECLICK,EM_MOUSE_MIDDLE);
	MapEvent(EM_MOUSE_MIDDLEDBCLICK,EM_MOUSE_MIDDLE);
	MapEvent(EM_MOUSE_MIDDLEDRAGBEGIN,EM_MOUSE_MIDDLE);
	MapEvent(EM_MOUSE_MIDDLEDRAGEND,EM_MOUSE_MIDDLE);


}

void CEventBinding::DisableMouse()
{
	int a;
	for(a=EM_MOUSE_BEGIN;a<=EM_MOUSE;a++)
		DisableEvent(a);
}

void CEventBinding::DisableKeyboard()
{
	int a;
	for(a=EM_KEY_BEGIN;a<=EM_KEY;a++)
		DisableEvent(a);
}

void CEventBinding::EnableMouse()
{
	int a;
	for(a=EM_MOUSE_BEGIN;a<=EM_MOUSE;a++)
		EnableEvent(a);
}

void CEventBinding::EnableKeyboard()
{
	int a;
	for(a=EM_KEY_BEGIN;a<=EM_KEY;a++)
		EnableEvent(a);
}