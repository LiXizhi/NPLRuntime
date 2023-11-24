#pragma once
#include <ctime>
#include <cstdint>
#include <memory>

namespace ParaEngine
{
	enum class EMouseEventType
	{
		Unknown,
		Button,
		Move,
		Wheel
	};

	enum class EMouseButton
	{
		LEFT = 0,
		RIGHT,
		MIDDLE,
		COUNT
	};

	enum class EKeyState
	{
		PRESS = 0,
		RELEASE
	};

	enum class EVirtualKey
	{
		KEY_UNKNOWN = 0,
		KEY_ESCAPE = 1,
		KEY_1 = 2,
		KEY_2 = 3,
		KEY_3 = 4,
		KEY_4 = 5,
		KEY_5 = 6,
		KEY_6 = 7,
		KEY_7 = 8,
		KEY_8 = 9,
		KEY_9 = 10,
		KEY_0 = 11,
		KEY_MINUS = 12, /* - on main keyboard */
		KEY_EQUALS = 13,
		KEY_BACK = 14, /* backspace */
		KEY_TAB = 15,
		KEY_Q = 16,
		KEY_W = 17,
		KEY_E = 18,
		KEY_R = 19,
		KEY_T = 20,
		KEY_Y = 21,
		KEY_U = 22,
		KEY_I = 23,
		KEY_O = 24,
		KEY_P = 25,
		KEY_LBRACKET = 26,
		KEY_RBRACKET = 27,
		KEY_RETURN = 28, /* Enter on main keyboard */
		KEY_LCONTROL = 29,
		KEY_A = 30,
		KEY_S = 31,
		KEY_D = 32,
		KEY_F = 33,
		KEY_G = 34,
		KEY_H = 35,
		KEY_J = 36,
		KEY_K = 37,
		KEY_L = 38,
		KEY_SEMICOLON = 39,
		KEY_APOSTROPHE = 40,
		KEY_GRAVE = 41, /* accent grave */
		KEY_LSHIFT = 42,
		KEY_BACKSLASH = 43,
		KEY_Z = 44,
		KEY_X = 45,
		KEY_C = 46,
		KEY_V = 47,
		KEY_B = 48,
		KEY_N = 49,
		KEY_M = 50,
		KEY_COMMA = 51,
		KEY_PERIOD = 52, /* . on main keyboard */
		KEY_SLASH = 53, /* / on main keyboard */
		KEY_RSHIFT = 54,
		KEY_MULTIPLY = 55, /* * on numeric keypad */
		KEY_LMENU = 56, /* left Alt */
		KEY_SPACE = 57,
		KEY_CAPITAL = 58,
		KEY_F1 = 59,
		KEY_F2 = 60,
		KEY_F3 = 61,
		KEY_F4 = 62,
		KEY_F5 = 63,
		KEY_F6 = 64,
		KEY_F7 = 65,
		KEY_F8 = 66,
		KEY_F9 = 67,
		KEY_F10 = 68,
		KEY_NUMLOCK = 69,
		KEY_SCROLL = 70, /* Scroll Lock */
		KEY_NUMPAD7 = 71,
		KEY_NUMPAD8 = 72,
		KEY_NUMPAD9 = 73,
		KEY_SUBTRACT = 74, /* - on numeric keypad */
		KEY_NUMPAD4 = 75,
		KEY_NUMPAD5 = 76,
		KEY_NUMPAD6 = 77,
		KEY_ADD = 78, /* + on numeric keypad */
		KEY_NUMPAD1 = 79,
		KEY_NUMPAD2 = 80,
		KEY_NUMPAD3 = 81,
		KEY_NUMPAD0 = 82,
		KEY_DECIMAL = 83, /* . on numeric keypad */
		KEY_OEM_102 = 86, /* <> or \| on RT 102-key keyboard (Non-U.S.) */
		KEY_F11 = 87,
		KEY_F12 = 88,
		KEY_F13 = 100, /* (NEC PC98) */
		KEY_F14 = 101, /* (NEC PC98) */
		KEY_F15 = 102, /* (NEC PC98) */
		KEY_KANA = 112, /* (Japanese keyboard) */
		KEY_ABNT_C1 = 115, /* /? on Brazilian keyboard */
		KEY_CONVERT = 121, /* (Japanese keyboard)            */
		KEY_NOCONVERT = 123, /* (Japanese keyboard)            */
		KEY_YEN = 125, /* (Japanese keyboard)            */
		KEY_ABNT_C2 = 126, /* Numpad . on Brazilian keyboard */
		KEY_NUMPADEQUALS = 141, /* =tonumber(" on numeric keypad (NEC PC98) */
		KEY_PREVTRACK = 144, /* Previous Track (KEY_CIRCUMFLEX on Japanese keyboard) */
		KEY_AT = 145, /* (NEC PC98) */
		KEY_COLON = 146, /* (NEC PC98) */
		KEY_UNDERLINE = 147, /* (NEC PC98) */
		KEY_KANJI = 148, /* (Japanese keyboard) */
		KEY_STOP = 149, /* (NEC PC98) */
		KEY_AX = 150, /* (Japan AX) */
		KEY_UNLABELED = 151, /* (J3100) */
		KEY_NEXTTRACK = 153, /* Next Track */
		KEY_NUMPADENTER = 156, /* Enter on numeric keypad */
		KEY_RCONTROL = 157,
		KEY_MUTE = 160, /* Mute */
		KEY_CALCULATOR = 161, /* Calculator */
		KEY_PLAYPAUSE = 162, /* Play / Pause */
		KEY_MEDIASTOP = 164, /* Media Stop */
		KEY_VOLUMEDOWN = 174, /* Volume - */
		KEY_VOLUMEUP = 176, /* Volume + */
		KEY_WEBHOME = 178, /* Web home */
		KEY_NUMPADCOMMA = 179, /* ",16), on numeric keypad (NEC PC98) */
		KEY_DIVIDE = 181, /* / on numeric keypad */
		KEY_SYSRQ = 183,
		KEY_RMENU = 184, /* right Alt */
		KEY_PAUSE = 197, /* Pause */
		KEY_HOME = 199, /* Home on arrow keypad */
		KEY_UP = 200, /* UpArrow on arrow keypad */
		KEY_PRIOR = 201, /* PgUp on arrow keypad */
		KEY_LEFT = 203, /* LeftArrow on arrow keypad */
		KEY_RIGHT = 205, /* RightArrow on arrow keypad */
		KEY_END = 207, /* End on arrow keypad */
		KEY_DOWN = 208, /* DownArrow on arrow keypad */
		KEY_NEXT = 209, /* PgDn on arrow keypad */
		KEY_INSERT = 210, /* Insert on arrow keypad */
		KEY_DELETE = 211, /* Delete on arrow keypad */
		KEY_LWIN = 219, /* Left Windows key */
		KEY_RWIN = 220, /* Right Windows key */
		KEY_APPS = 221, /* AppMenu key */
		KEY_POWER = 222, /* System Power */
		KEY_SLEEP = 223, /* System Sleep */
		KEY_WAKE = 227, /* System Wake */
		KEY_WEBSEARCH = 229, /* Web Search */
		KEY_WEBFAVORITES = 230, /* Web Favorites */
		KEY_WEBREFRESH = 231, /* Web Refresh */
		KEY_WEBSTOP = 232, /* Web Stop */
		KEY_WEBFORWARD = 233, /* Web Forward */
		KEY_WEBBACK = 234, /* Web Back */
		KEY_MYCOMPUTER = 235, /* My Computer */
		KEY_MAIL = 236, /* Mail */
		KEY_MEDIASELECT = 237, /* Media Select */

		KEY_SHIFT,
		KEY_CONTROL,
		KEY_ALT,

		COUNT = KEY_ALT + 1,
	};

	class DeviceKeyEvent
	{
	public:
		DeviceKeyEvent() :m_state(EKeyState::RELEASE), m_virtualKey(EVirtualKey::KEY_UNKNOWN), m_nTime(0) {}
		DeviceKeyEvent(EKeyState state, EVirtualKey vKey, int32 nTime=0): m_state(state), m_virtualKey(vKey), m_nTime(nTime) {}

		EKeyState m_state;
		EVirtualKey m_virtualKey;
		int32 m_nTime;
	};

	struct DeviceMouseState
	{
		int32_t x;
		int32_t y;
		int32_t z;

		EKeyState buttons[(int)EMouseButton::COUNT];

		DeviceMouseState():x(0) ,y(0) ,z(0)
		{
			for (int i = 0;i < (int)EMouseButton::COUNT;i++)
			{
				buttons[i] = EKeyState::RELEASE;
			}
		}
	};

	class DeviceMouseEvent
	{
	public:
		DeviceMouseEvent():m_timestamp(0) {
			m_timestamp = (uint32_t)std::time(nullptr);
		}

		virtual EMouseEventType GetEventType() const { return EMouseEventType::Unknown; }

        uint32_t GetTimestamp() const { return m_timestamp; }
        
		virtual std::string ToString();
	private:
		uint32_t m_timestamp;
	};

	using DeviceMouseEventPtr = std::shared_ptr<DeviceMouseEvent>;

	class DeviceMouseMoveEvent : public DeviceMouseEvent
	{

	public:
		DeviceMouseMoveEvent(int x,int y):m_x(x),m_y(y) {};
		inline int GetX() const { return m_x; };
		inline int GetY() const { return m_y; };
		virtual EMouseEventType GetEventType()const override { return EMouseEventType::Move; };

		virtual std::string ToString();
	private:
		int m_x;
		int m_y;
	};

	class DeviceMouseButtonEvent : public DeviceMouseEvent
	{
	public:
		DeviceMouseButtonEvent(const EMouseButton button, const EKeyState state, int x, int y, bool bSimulated = false)
			: m_button(button)
			, m_state(state) 
			, m_x(x)
			, m_y(y), m_bFromTouchInput(bSimulated){};

        inline EMouseButton GetButton() const { return m_button; };
		void SetButton(EMouseButton btn) { m_button = btn; }
		inline EKeyState GetKeyState() const { return m_state; };

		inline int GetX() const { return m_x; };
		inline int GetY() const { return m_y; };

		virtual EMouseEventType GetEventType() const { return EMouseEventType::Button; };
		inline bool IsFromTouchInput() const { return m_bFromTouchInput; };

		virtual std::string ToString();
	private:
		EMouseButton m_button;
		EKeyState m_state;
		int m_x;
		int m_y;
		// whether this is simulated from touch input
		bool m_bFromTouchInput;
	};

	class DeviceMouseWheelEvent : public DeviceMouseEvent
	{
	public:
		DeviceMouseWheelEvent(int wheel) :m_wheel(wheel) {}
		virtual EMouseEventType GetEventType() const { return EMouseEventType::Wheel; }
		virtual int GetWheel() const { return m_wheel; }

		virtual std::string ToString();
	private:
		int m_wheel;
	};
}
