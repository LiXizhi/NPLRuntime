//----------------------------------------------------------------------
// Class:	AppDelegate
// Authors:	YuanQuanwei, LanZhihong, LiXizhi
// Company: ParaEngine
// Date: 2018.3.29
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "AppDelegate.h"
#include "ParaAppAndroid.h"
#include "RenderWindowAndroid.h"
#include "RenderDeviceEGL.h"
#include "RenderContextEGL.h"
#include "ParaTime.h"
#include "NPLRuntime.h"
#include "jni/ParaEngineHelper.h"

#include <boost/bind.hpp>
#include <android/log.h>
#include <android/asset_manager.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <cassert>
#include <ctime>
#include <unordered_map>

using namespace ParaEngine;

void AppDelegate::app_handle_command(struct android_app* app, int32_t cmd)
{
	AppDelegate* myApp = static_cast<AppDelegate*>(app->userData);
	switch (cmd)
	{
	case APP_CMD_SAVE_STATE:
	{
		//OUTPUT_LOG("save state");
		saved_state state;
		state.app = myApp;
		app->savedState = malloc(sizeof(saved_state));
		app->savedStateSize = sizeof(struct saved_state);
		memcpy(app->savedState, &state, app->savedStateSize);
		LOGI("state saved");

	}break;
	case APP_CMD_START:
	{
		myApp->OnStart();
	}break;
	case APP_CMD_RESUME:
	{
		myApp->OnResume();
	}break;
	case APP_CMD_PAUSE:
	{
		myApp->OnPause();
	}break;
	case APP_CMD_STOP:
	{
		myApp->OnStop();
	}break;
	case APP_CMD_DESTROY:
	{
		myApp->OnDestroy();
	}break;
	case APP_CMD_INIT_WINDOW:
	{
		myApp->OnInitWindow();
	}break;
	case APP_CMD_TERM_WINDOW:
	{
		myApp->OnTermWindow();
	}break;
	case APP_CMD_WINDOW_RESIZED:
	{
		myApp->OnWindowResized();
	}break;
	}
}

void AppDelegate::handle_touch_input(AppDelegate* app, AInputEvent* event)
{
	int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
	int index = (AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
	int touchId = AMotionEvent_getPointerId(event, index);

	TouchEvent::TouchEventMsgType touchType = TouchEvent::TouchEvent_POINTER_UP;
	switch (action) {
	case AMOTION_EVENT_ACTION_DOWN:
	case AMOTION_EVENT_ACTION_POINTER_DOWN:
	{
		touchType = TouchEvent::TouchEvent_POINTER_DOWN;
		break;
	}
	case AMOTION_EVENT_ACTION_UP:
	case AMOTION_EVENT_ACTION_POINTER_UP:
	{
		touchType = TouchEvent::TouchEvent_POINTER_UP;
		break;
	}
	case AMOTION_EVENT_ACTION_MOVE:
	{
		touchType = TouchEvent::TouchEvent_POINTER_UPDATE;
		break;
	}
	case AMOTION_EVENT_ACTION_CANCEL:
	{
		touchType = TouchEvent::TouchEvent_POINTER_UP;
		break;
	}
	default:
		return;
	}
	
	int msCurTime = ::GetTickCount();
	float touchX = AMotionEvent_getX(event, index);
	float touchY = AMotionEvent_getY(event, index);
	TouchEvent touchEvent(EH_TOUCH, touchType, touchId, touchX, touchY, msCurTime);
	
	std::vector<TouchEventPtr> touchEvents;

	if (touchType == TouchEvent::TouchEvent_POINTER_UPDATE)
	{
		auto touchCount = AMotionEvent_getPointerCount(event);
		for (int i = 0; i < touchCount; i++)
		{
			int32_t touchId = AMotionEvent_getPointerId(event, i);
			float touchX = AMotionEvent_getX(event, i);
			float touchY = AMotionEvent_getY(event, i);
			TouchEventPtr touchEvent = std::make_shared<TouchEvent>(EH_TOUCH, touchType, touchId, touchX, touchY, msCurTime);
			touchEvents.push_back(touchEvent);
		}
	}
	else
	{
		TouchEventPtr touchEvent = std::make_shared<TouchEvent>(EH_TOUCH, touchType, touchId, touchX, touchY, msCurTime);
		touchEvents.push_back(touchEvent);
	}
	app->OnTouch(touchEvents);
}


inline EVirtualKey toVirtualKey(int32_t keycode)
{
	static std::unordered_map<int32_t, EVirtualKey> s_keymap;
	if (s_keymap.size() == 0)
	{
		s_keymap[AKEYCODE_UNKNOWN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_SOFT_LEFT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_SOFT_RIGHT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_HOME] = EVirtualKey::KEY_HOME;
		s_keymap[AKEYCODE_BACK] = EVirtualKey::KEY_ESCAPE; // LiXizhi:  we will match back button to esc key on android
		s_keymap[AKEYCODE_CALL] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_ENDCALL] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_0] = EVirtualKey::KEY_0;
		s_keymap[AKEYCODE_1] = EVirtualKey::KEY_1;
		s_keymap[AKEYCODE_2] = EVirtualKey::KEY_2;
		s_keymap[AKEYCODE_3] = EVirtualKey::KEY_3;
		s_keymap[AKEYCODE_4] = EVirtualKey::KEY_4;
		s_keymap[AKEYCODE_5] = EVirtualKey::KEY_5;
		s_keymap[AKEYCODE_6] = EVirtualKey::KEY_6;
		s_keymap[AKEYCODE_7] = EVirtualKey::KEY_7;
		s_keymap[AKEYCODE_8] = EVirtualKey::KEY_8;
		s_keymap[AKEYCODE_9] = EVirtualKey::KEY_9;
		s_keymap[AKEYCODE_STAR] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_POUND] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_DPAD_UP] = EVirtualKey::KEY_UP;
		s_keymap[AKEYCODE_DPAD_DOWN] = EVirtualKey::KEY_DOWN;
		s_keymap[AKEYCODE_DPAD_LEFT] = EVirtualKey::KEY_LEFT;
		s_keymap[AKEYCODE_DPAD_RIGHT] = EVirtualKey::KEY_RIGHT;
		s_keymap[AKEYCODE_DPAD_CENTER] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_VOLUME_UP] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_VOLUME_DOWN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_POWER] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_CAMERA] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_CLEAR] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_A] = EVirtualKey::KEY_A;
		s_keymap[AKEYCODE_B] = EVirtualKey::KEY_B;
		s_keymap[AKEYCODE_C] = EVirtualKey::KEY_C;
		s_keymap[AKEYCODE_D] = EVirtualKey::KEY_D;
		s_keymap[AKEYCODE_E] = EVirtualKey::KEY_E;
		s_keymap[AKEYCODE_F] = EVirtualKey::KEY_F;
		s_keymap[AKEYCODE_G] = EVirtualKey::KEY_G;
		s_keymap[AKEYCODE_H] = EVirtualKey::KEY_H;
		s_keymap[AKEYCODE_I] = EVirtualKey::KEY_I;
		s_keymap[AKEYCODE_J] = EVirtualKey::KEY_J;
		s_keymap[AKEYCODE_K] = EVirtualKey::KEY_K;
		s_keymap[AKEYCODE_L] = EVirtualKey::KEY_L;
		s_keymap[AKEYCODE_M] = EVirtualKey::KEY_M;
		s_keymap[AKEYCODE_N] = EVirtualKey::KEY_N;
		s_keymap[AKEYCODE_O] = EVirtualKey::KEY_O;
		s_keymap[AKEYCODE_P] = EVirtualKey::KEY_P;
		s_keymap[AKEYCODE_Q] = EVirtualKey::KEY_Q;
		s_keymap[AKEYCODE_R] = EVirtualKey::KEY_R;
		s_keymap[AKEYCODE_S] = EVirtualKey::KEY_S;
		s_keymap[AKEYCODE_T] = EVirtualKey::KEY_T;
		s_keymap[AKEYCODE_U] = EVirtualKey::KEY_U;
		s_keymap[AKEYCODE_V] = EVirtualKey::KEY_V;
		s_keymap[AKEYCODE_W] = EVirtualKey::KEY_W;
		s_keymap[AKEYCODE_X] = EVirtualKey::KEY_X;
		s_keymap[AKEYCODE_Y] = EVirtualKey::KEY_Y;
		s_keymap[AKEYCODE_Z] = EVirtualKey::KEY_Z;
		s_keymap[AKEYCODE_COMMA] = EVirtualKey::KEY_COMMA;
		s_keymap[AKEYCODE_PERIOD] = EVirtualKey::KEY_PERIOD;
		s_keymap[AKEYCODE_ALT_LEFT] = EVirtualKey::KEY_LMENU;
		s_keymap[AKEYCODE_ALT_RIGHT] = EVirtualKey::KEY_RMENU;
		s_keymap[AKEYCODE_SHIFT_LEFT] = EVirtualKey::KEY_LSHIFT;
		s_keymap[AKEYCODE_SHIFT_RIGHT] = EVirtualKey::KEY_RSHIFT;
		s_keymap[AKEYCODE_TAB] = EVirtualKey::KEY_TAB;
		s_keymap[AKEYCODE_SPACE] = EVirtualKey::KEY_SPACE;
		s_keymap[AKEYCODE_SYM] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_EXPLORER] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_ENVELOPE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_ENTER] = EVirtualKey::KEY_RETURN;
		s_keymap[AKEYCODE_DEL] = EVirtualKey::KEY_DELETE;
		s_keymap[AKEYCODE_GRAVE] = EVirtualKey::KEY_GRAVE;
		s_keymap[AKEYCODE_MINUS] = EVirtualKey::KEY_MINUS;
		s_keymap[AKEYCODE_EQUALS] = EVirtualKey::KEY_EQUALS;
		s_keymap[AKEYCODE_LEFT_BRACKET] = EVirtualKey::KEY_LBRACKET;
		s_keymap[AKEYCODE_RIGHT_BRACKET] = EVirtualKey::KEY_RBRACKET;
		s_keymap[AKEYCODE_BACKSLASH] = EVirtualKey::KEY_BACKSLASH;
		s_keymap[AKEYCODE_SEMICOLON] = EVirtualKey::KEY_SEMICOLON;
		s_keymap[AKEYCODE_APOSTROPHE] = EVirtualKey::KEY_APOSTROPHE;
		s_keymap[AKEYCODE_SLASH] = EVirtualKey::KEY_SLASH;
		s_keymap[AKEYCODE_AT] = EVirtualKey::KEY_AT;
		s_keymap[AKEYCODE_NUM] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_HEADSETHOOK] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_FOCUS] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_PLUS] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MENU] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_NOTIFICATION] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_SEARCH] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_PLAY_PAUSE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_STOP] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_NEXT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_PREVIOUS] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_REWIND] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_FAST_FORWARD] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MUTE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_PAGE_UP] = EVirtualKey::KEY_PERIOD;
		s_keymap[AKEYCODE_PAGE_DOWN] = EVirtualKey::KEY_NEXT;
		s_keymap[AKEYCODE_PICTSYMBOLS] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_SWITCH_CHARSET] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_A] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_B] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_C] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_X] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_Y] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_Z] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_L1] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_R1] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_L2] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_R2] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_THUMBL] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_THUMBR] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_START] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_SELECT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_MODE] = EVirtualKey::KEY_UNKNOWN;


		s_keymap[AKEYCODE_ESCAPE] = EVirtualKey::KEY_ESCAPE;
		s_keymap[AKEYCODE_FORWARD_DEL] = EVirtualKey::KEY_BACK;
		s_keymap[AKEYCODE_CTRL_LEFT] = EVirtualKey::KEY_LCONTROL;
		s_keymap[AKEYCODE_CTRL_RIGHT] = EVirtualKey::KEY_RCONTROL;
		s_keymap[AKEYCODE_CAPS_LOCK] = EVirtualKey::KEY_CAPITAL;
		s_keymap[AKEYCODE_SCROLL_LOCK] = EVirtualKey::KEY_SCROLL;
		s_keymap[AKEYCODE_META_LEFT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_META_RIGHT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_FUNCTION] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_SYSRQ] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BREAK] = EVirtualKey::KEY_LBRACKET;
		s_keymap[AKEYCODE_MOVE_HOME] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MOVE_END] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_INSERT] = EVirtualKey::KEY_INSERT;
		s_keymap[AKEYCODE_FORWARD] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_PLAY] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_PAUSE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_CLOSE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_EJECT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_RECORD] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_F1] = EVirtualKey::KEY_F1;
		s_keymap[AKEYCODE_F2] = EVirtualKey::KEY_F2;
		s_keymap[AKEYCODE_F3] = EVirtualKey::KEY_F3;
		s_keymap[AKEYCODE_F4] = EVirtualKey::KEY_F4;
		s_keymap[AKEYCODE_F5] = EVirtualKey::KEY_F5;
		s_keymap[AKEYCODE_F6] = EVirtualKey::KEY_F6;
		s_keymap[AKEYCODE_F7] = EVirtualKey::KEY_F7;
		s_keymap[AKEYCODE_F8] = EVirtualKey::KEY_F8;
		s_keymap[AKEYCODE_F9] = EVirtualKey::KEY_F9;
		s_keymap[AKEYCODE_F10] = EVirtualKey::KEY_F10;
		s_keymap[AKEYCODE_F11] = EVirtualKey::KEY_F11;
		s_keymap[AKEYCODE_F12] = EVirtualKey::KEY_F12;
		s_keymap[AKEYCODE_NUM_LOCK] = EVirtualKey::KEY_NUMLOCK;
		s_keymap[AKEYCODE_NUMPAD_0] = EVirtualKey::KEY_NUMPAD0;
		s_keymap[AKEYCODE_NUMPAD_1] = EVirtualKey::KEY_NUMPAD1;
		s_keymap[AKEYCODE_NUMPAD_2] = EVirtualKey::KEY_NUMPAD2;
		s_keymap[AKEYCODE_NUMPAD_3] = EVirtualKey::KEY_NUMPAD3;
		s_keymap[AKEYCODE_NUMPAD_4] = EVirtualKey::KEY_NUMPAD4;
		s_keymap[AKEYCODE_NUMPAD_5] = EVirtualKey::KEY_NUMPAD5;
		s_keymap[AKEYCODE_NUMPAD_6] = EVirtualKey::KEY_NUMPAD6;
		s_keymap[AKEYCODE_NUMPAD_7] = EVirtualKey::KEY_NUMPAD0;
		s_keymap[AKEYCODE_NUMPAD_8] = EVirtualKey::KEY_NUMPAD0;
		s_keymap[AKEYCODE_NUMPAD_9] = EVirtualKey::KEY_NUMPAD0;
		s_keymap[AKEYCODE_NUMPAD_DIVIDE] = EVirtualKey::KEY_DIVIDE;
		s_keymap[AKEYCODE_NUMPAD_MULTIPLY] = EVirtualKey::KEY_MULTIPLY;
		s_keymap[AKEYCODE_NUMPAD_SUBTRACT] = EVirtualKey::KEY_SUBTRACT;
		s_keymap[AKEYCODE_NUMPAD_ADD] = EVirtualKey::KEY_ADD;
		s_keymap[AKEYCODE_NUMPAD_DOT] = EVirtualKey::KEY_DECIMAL;
		s_keymap[AKEYCODE_NUMPAD_COMMA] = EVirtualKey::KEY_COMMA;
		s_keymap[AKEYCODE_NUMPAD_ENTER] = EVirtualKey::KEY_NUMPADENTER;
		s_keymap[AKEYCODE_NUMPAD_EQUALS] = EVirtualKey::KEY_NUMPADEQUALS;
		s_keymap[AKEYCODE_NUMPAD_LEFT_PAREN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_NUMPAD_RIGHT_PAREN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_VOLUME_MUTE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_INFO] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_CHANNEL_UP] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_CHANNEL_DOWN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_ZOOM_IN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_ZOOM_OUT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_TV] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_WINDOW] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_GUIDE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_DVR] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BOOKMARK] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_CAPTIONS] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_SETTINGS] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_TV_POWER] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_TV_INPUT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_STB_POWER] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_STB_INPUT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_AVR_POWER] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_AVR_INPUT] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_PROG_RED] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_PROG_GREEN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_PROG_YELLOW] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_PROG_BLUE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_APP_SWITCH] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_1] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_2] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_3] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_4] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_5] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_6] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_7] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_8] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_9] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_10] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_11] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_12] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_13] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_14] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_15] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BUTTON_16] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_LANGUAGE_SWITCH] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MANNER_MODE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_3D_MODE] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_CONTACTS] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_CALENDAR] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MUSIC] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_CALCULATOR] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_ZENKAKU_HANKAKU] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_EISU] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MUHENKAN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_HENKAN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_KATAKANA_HIRAGANA] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_YEN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_RO] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_KANA] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_ASSIST] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BRIGHTNESS_DOWN] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_BRIGHTNESS_UP] = EVirtualKey::KEY_UNKNOWN;
		s_keymap[AKEYCODE_MEDIA_AUDIO_TRACK] = EVirtualKey::KEY_UNKNOWN;
	}
	auto ret = s_keymap.find(keycode);
	if (ret != s_keymap.end())
	{
		return ret->second;
	}
	return EVirtualKey::KEY_UNKNOWN;
}

int32_t AppDelegate::handle_key_input(AppDelegate* app, AInputEvent* event)
{
	EKeyState state = EKeyState::RELEASE;
	EVirtualKey key = EVirtualKey::KEY_UNKNOWN;
	int32_t action = AKeyEvent_getAction(event);
	if (action == AKEY_EVENT_ACTION_DOWN)
		state = EKeyState::PRESS;
	else if (action == AKEY_EVENT_ACTION_UP)
		state = EKeyState::RELEASE;
	else
		return 0;
	int32_t keycode = AKeyEvent_getKeyCode(event);
	key = toVirtualKey(keycode);
	app->OnKey(key, state);
	return 0;
}

int32_t AppDelegate::app_handle_input(struct android_app* app, AInputEvent* event)
{
	AppDelegate* myApp = static_cast<AppDelegate*>(app->userData);
	int32_t eventType = AInputEvent_getType(event);
	if (eventType == AINPUT_EVENT_TYPE_MOTION)
	{
		auto eventSource = AInputEvent_getSource(event);
		if (eventSource == AINPUT_SOURCE_TOUCHSCREEN)
		{
			handle_touch_input(myApp, event);
		}
	}
	else if (eventType == AINPUT_EVENT_TYPE_KEY)
	{
		handle_key_input(myApp, event);
	}
	return 0;
}


AppDelegate::AppDelegate()
	:m_State(nullptr)
	, m_ParaEngineApp(nullptr)
	, m_isPaused(false)
	, m_fRefreshTimerInterval(1.0f / 60.0f)
{

}

AppDelegate::~AppDelegate()
{
	OUTPUT_LOG("~AppDelegate");
}


void AppDelegate::handle_mainloop_timer(const boost::system::error_code& err)
{
	if (!err && !m_State->destroyRequested)
	{
		int ident = 0;
		int events = 0;
		struct android_poll_source* source = nullptr;
		ident = ALooper_pollAll(0, NULL, &events, (void**)&source);
		if (ident >= 0)
		{
			// Process this event.
			if (source != NULL) {
				source->process(m_State, source);
			}
			// Check if we are exiting
			if (m_State->destroyRequested != 0) {
				LOGI("app:destroy");
				return;
			}
		}
		m_appActivity.processGLEventJNI(m_State);

		double fNextInterval = 0.033f; // as fast as possible
		if (m_ParaEngineApp && !m_isPaused)
		{
			m_ParaEngineApp->DoWork();
			fNextInterval = m_ParaEngineApp->GetRefreshTimer() - (ParaTimer::GetAbsoluteTime() - m_ParaEngineApp->GetAppTime());
			fNextInterval = std::min(0.1, std::max(0.0, fNextInterval));
			// OUTPUT_LOG("%f", fNextInterval);
		}


		NextLoop((int)(fNextInterval * 1000), &AppDelegate::handle_mainloop_timer, this);
	}
}

AppDelegate& AppDelegate::getInstance()
{
	static AppDelegate ins;
	return ins;
}


void AppDelegate::Run(struct android_app* app)
{
	m_State = app;
	app->userData = this;
	app->onAppCmd = AppDelegate::app_handle_command;
	app->onInputEvent = AppDelegate::app_handle_input;

	m_appActivity.init(app);
	ParaEngineHelper::init();

	LOGI("app:run");

#define USE_ASYNC_TIMER_MAIN_LOOP
#ifdef USE_ASYNC_TIMER_MAIN_LOOP
	// start the main loop timer. 

	NextLoop(50, &AppDelegate::handle_mainloop_timer, this);
	MainLoopRun();

#else
	while (!m_State->destroyRequested)
	{
		int ident = 0;
		int events = 0;
		struct android_poll_source* source = nullptr;
		ident = ALooper_pollAll(0, NULL, &events, (void**)&source);
		if (ident >= 0)
		{
			// Process this event.
			if (source != NULL) {
				source->process(m_State, source);
			}
			// Check if we are exiting
			if (m_State->destroyRequested != 0) {
				LOGI("app:destroy");
				return;
			}
		}
		if (m_ParaEngineApp && !m_isPaused)
		{
			m_ParaEngineApp->DoWork();
		}
		// appActivity.processGLEventJNI(app);
	}
#endif

	LOGI("app:exit");
}

void AppDelegate::OnStart()
{
	LOGI("app:OnStart");
}

void AppDelegate::OnStop()
{
	LOGI("app:OnStop");

	// TODO: kill app , this is temporary measures since we have not fixed the texture lost issue when app stopped. 
	//exit(0);
}

void AppDelegate::OnPause()
{
	LOGI("app:OnPause");
	if (m_ParaEngineApp)
	{
		m_ParaEngineApp->OnPause();
	}

	//m_isPaused = true;
}

void AppDelegate::onCmdLine(const std::string& cmd)
{
	if (m_ParaEngineApp)
	{
		m_ParaEngineApp->onCmdLine(cmd);
	}
}

void AppDelegate::OnResume()
{
	LOGI("app:OnResume");
	if (m_ParaEngineApp)
	{
		m_ParaEngineApp->OnResume();

		if (m_State)
		{
			std::string intent_data = AppActivity::getLauncherIntentData(m_State);
			onCmdLine(intent_data);
		}
	}

	//m_isPaused = false;
}
void AppDelegate::OnDestroy()
{
	LOGI("app:OnDestroy");
}


void AppDelegate::OnInitWindow()
{
	LOGI("app:OnInitWindow");
	if (m_ParaEngineApp == nullptr)
	{
		auto renderWindow = new RenderWindowAndroid(m_State->window);
		m_ParaEngineApp = new CParaEngineAppAndroid(m_State);

		std::string intent_data = AppActivity::getLauncherIntentData(m_State);
		LOGI("intent_data:%s", intent_data.c_str());

		m_ParaEngineApp->InitApp(renderWindow, intent_data.c_str());
	}
	else
	{
		LOGI("app:window is recreated.");
		auto renderWindow = new RenderWindowAndroid(m_State->window);
		m_ParaEngineApp->OnRendererRecreated(renderWindow);
	}
}
void AppDelegate::OnTermWindow()
{
	LOGI("app:OnTermWindow");
	if (m_ParaEngineApp)
	{
		m_ParaEngineApp->OnRendererDestroyed();
	}
}
void AppDelegate::OnWindowResized()
{
	LOGI("app:OnWindowResized");
	if (m_ParaEngineApp)
	{
		auto renderWindow = m_ParaEngineApp->GetRenderWindow();
		ParaViewport vp;
		vp.X = 0; vp.Y = 0;
		vp.Width = renderWindow->GetWidth();
		vp.Height = renderWindow->GetHeight();
		vp.MinZ = 0;
		vp.MaxZ = 1.0f;
		CGlobals::GetRenderDevice()->SetViewport(vp);
	}

}

void ParaEngine::AppDelegate::OnTouch(const std::vector<TouchEventPtr>& events)
{
	if (m_ParaEngineApp && m_ParaEngineApp->IsAppActive())
	{
		auto gui = CGUIRoot::GetInstance();
		if (gui)
		{
			for (auto& event : events)
			{
				// LOGI("Touch Event %s", event->ToScriptCode().c_str());
				gui->handleTouchEvent(*event);
			}
		}
	}
}

void ParaEngine::AppDelegate::OnKey(const EVirtualKey& key, const EKeyState& state)
{
	if (m_ParaEngineApp && m_ParaEngineApp->IsAppActive())
	{
		auto gui = CGUIRoot::GetInstance();
		if (gui)
		{
			if(state == EKeyState::PRESS)
				gui->SendKeyDownEvent(key);
			else if (state == EKeyState::RELEASE)
				gui->SendKeyUpEvent(key);
		}
	}
}

