#pragma once

#include "Framework/Interface/Render/IRenderWindow.h"
#include "Framework/Common/Math/Vector2.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <stdint.h>
#include <windows.h>
#include <windowsx.h>



namespace ParaEngine
{
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
		KEY_UNKNOWN = -1,
		/// Printable keys
		KEY_SPACE,
		KEY_APOSTROPHE,		// '
		KEY_COMMA,			// ,
		KEY_MINUS ,			// -
		KEY_PERIOD,			// .
		KEY_SLASH,			// /
		KEY_0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_SEMICOLON,		// ;
		KEY_EQUAL,			// =
		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,
		KEY_LEFT_BRACKET,		/** [ **/
 		KEY_BACKSLASH,			/** \ **/
		KEY_RIGHT_BRACKET,		/** ]  **/
		KEY_GRAVE_ACCENT,		/** ` **/

		/// Function Keys
		KEY_ESCAPE,
		KEY_ENTER,
		KEY_TAB,
		KEY_BACKSPACE,
		KEY_INSERT,
		KEY_DELETE,
		KEY_RIGHT,
		KEY_LEFT,
		KEY_DOWN,
		KEY_UP,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,
		KEY_HOME,
		KEY_END,
		KEY_CAPS_LOCK,
		KEY_SCROLL_LOCK,
		KEY_NUM_LOCK,
		KEY_PRINT_SCREEN,
		KEY_PAUSE,
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,
		KEY_NUMPAD_0,
		KEY_NUMPAD_1,
		KEY_NUMPAD_2,
		KEY_NUMPAD_3,
		KEY_NUMPAD_4,
		KEY_NUMPAD_5,
		KEY_NUMPAD_6,
		KEY_NUMPAD_7,
		KEY_NUMPAD_8,
		KEY_NUMPAD_9,
		KEY_NUMPAD_DECIMAL, /** . **/
		KEY_NUMPAD_DIVIDE,
		KEY_NUMPAD_MULTIPLY,
		KEY_NUMPAD_SUBTRACT,
		KEY_NUMPAD_ADD,
		KEY_NUMPAD_ENTER,
		KEY_NUMPAD_EQUAL,
		KEY_LEFT_SHIFT,
		KEY_LEFT_CONTROL,
		KEY_LEFT_ALT,
		KEY_RIGHT_SHIFT,
		KEY_RIGHT_CONTROL,
		KEY_RIGHT_ALT,
		COUNT,
	};


	EVirtualKey Win32VirtualKeyToParaVK(DWORD vk);
	DWORD ParaVKToWin32VirtualKey(EVirtualKey key);



	class WindowsRenderWindow : public IRenderWindow
	{
	public:
		virtual ~WindowsRenderWindow();
		WindowsRenderWindow(HINSTANCE hInstance,int width, int height,bool windowed);
		const static WCHAR* ClassName;
		bool ShouldClose() const;
		void PollEvents();
		HWND GetHandle() const;
		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;
		bool IsWindowed() const;
		EKeyState GetMouseButtonState(EMouseButton button);
		EKeyState GetKeyState(EVirtualKey key);
		CVector2 GetMousePos();

	protected:
		virtual void OnMouseButton(EMouseButton button, EKeyState state) {};
		virtual void OnMouseMove(uint32_t x, uint32_t y) {};
		virtual void OnMouseWhell(float x, float y, float delta) {};
		virtual void OnKey(EVirtualKey key, EKeyState state) {};
		virtual void OnChar(char character) {};


	private:
		HWND m_hWnd;
		HACCEL m_hAccel;
		int m_Width;
		int m_Height;
		bool m_Windowed;
		bool m_IsQuit;
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static std::unordered_map<HWND, WindowsRenderWindow*> g_WindowMap;
	private:
		void InitInput();
		void ProcessInput(const MSG& msg);
		// Key State.
		EKeyState m_MouseState[(uint32_t)EMouseButton::COUNT];
		EKeyState m_KeyState[(uint32_t)EVirtualKey::COUNT];
		CVector2 m_MousePos;

	};
}