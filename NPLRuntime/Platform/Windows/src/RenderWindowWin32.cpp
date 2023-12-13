#include "ParaEngine.h"
#include "RenderWindowWin32.h"
#include "resource.h"
#include "Winuser.h"
#include "2dengine/GUIRoot.h"
#include <shellapi.h>

#include <unordered_map>


namespace ParaEngine {

	std::unordered_map<DWORD, EVirtualKey> s_keymap;

	static void InitVirtualKeyMap()
	{
		if (s_keymap.size() == 0)
		{
			// Init keymap.

			s_keymap[VK_SPACE] = EVirtualKey::KEY_SPACE;
			s_keymap[VK_OEM_7] = EVirtualKey::KEY_APOSTROPHE;
			s_keymap[VK_OEM_COMMA] = EVirtualKey::KEY_COMMA;
			s_keymap[VK_OEM_PERIOD] = EVirtualKey::KEY_PERIOD;
			s_keymap[VK_OEM_2] = EVirtualKey::KEY_SLASH;
			s_keymap['0'] = EVirtualKey::KEY_0;
			s_keymap['1'] = EVirtualKey::KEY_1;
			s_keymap['2'] = EVirtualKey::KEY_2;
			s_keymap['3'] = EVirtualKey::KEY_3;
			s_keymap['4'] = EVirtualKey::KEY_4;
			s_keymap['5'] = EVirtualKey::KEY_5;
			s_keymap['6'] = EVirtualKey::KEY_6;
			s_keymap['7'] = EVirtualKey::KEY_7;
			s_keymap['8'] = EVirtualKey::KEY_8;
			s_keymap['9'] = EVirtualKey::KEY_9;
			s_keymap[VK_OEM_1] = EVirtualKey::KEY_SEMICOLON;
			s_keymap[VK_OEM_PLUS] = EVirtualKey::KEY_EQUALS;
			s_keymap['A'] = EVirtualKey::KEY_A;
			s_keymap['B'] = EVirtualKey::KEY_B;
			s_keymap['C'] = EVirtualKey::KEY_C;
			s_keymap['D'] = EVirtualKey::KEY_D;
			s_keymap['E'] = EVirtualKey::KEY_E;
			s_keymap['F'] = EVirtualKey::KEY_F;
			s_keymap['G'] = EVirtualKey::KEY_G;
			s_keymap['H'] = EVirtualKey::KEY_H;
			s_keymap['I'] = EVirtualKey::KEY_I;
			s_keymap['G'] = EVirtualKey::KEY_J;
			s_keymap['K'] = EVirtualKey::KEY_K;
			s_keymap['L'] = EVirtualKey::KEY_L;
			s_keymap['M'] = EVirtualKey::KEY_M;
			s_keymap['N'] = EVirtualKey::KEY_N;
			s_keymap['O'] = EVirtualKey::KEY_O;
			s_keymap['P'] = EVirtualKey::KEY_P;
			s_keymap['Q'] = EVirtualKey::KEY_Q;
			s_keymap['R'] = EVirtualKey::KEY_R;
			s_keymap['S'] = EVirtualKey::KEY_S;
			s_keymap['T'] = EVirtualKey::KEY_T;
			s_keymap['U'] = EVirtualKey::KEY_U;
			s_keymap['V'] = EVirtualKey::KEY_V;
			s_keymap['W'] = EVirtualKey::KEY_W;
			s_keymap['X'] = EVirtualKey::KEY_X;
			s_keymap['Y'] = EVirtualKey::KEY_Y;
			s_keymap['Z'] = EVirtualKey::KEY_Z;
			s_keymap[VK_OEM_4] = EVirtualKey::KEY_LBRACKET;
			s_keymap[VK_OEM_5] = EVirtualKey::KEY_BACKSLASH;
			s_keymap[VK_OEM_6] = EVirtualKey::KEY_RBRACKET;
			s_keymap[VK_OEM_3] = EVirtualKey::KEY_GRAVE;

			/// Function Keys
			s_keymap[VK_ESCAPE] = EVirtualKey::KEY_ESCAPE;
			s_keymap[VK_RETURN] = EVirtualKey::KEY_RETURN;
			s_keymap[VK_TAB] = EVirtualKey::KEY_TAB;
			s_keymap[VK_BACK] = EVirtualKey::KEY_BACK;
			s_keymap[VK_INSERT] = EVirtualKey::KEY_INSERT;
			s_keymap[VK_DELETE] = EVirtualKey::KEY_DELETE;
			s_keymap[VK_RIGHT] = EVirtualKey::KEY_RIGHT;
			s_keymap[VK_LEFT] = EVirtualKey::KEY_LEFT;
			s_keymap[VK_DOWN] = EVirtualKey::KEY_DOWN;
			s_keymap[VK_UP] = EVirtualKey::KEY_UP;
			s_keymap[VK_PRIOR] = EVirtualKey::KEY_PERIOD;
			s_keymap[VK_NEXT] = EVirtualKey::KEY_NEXT;
			s_keymap[VK_HOME] = EVirtualKey::KEY_HOME;
			s_keymap[VK_END] = EVirtualKey::KEY_END;
			s_keymap[VK_CAPITAL] = EVirtualKey::KEY_CAPITAL;
			s_keymap[VK_SCROLL] = EVirtualKey::KEY_SCROLL;
			s_keymap[VK_NUMLOCK] = EVirtualKey::KEY_NUMLOCK;
			s_keymap[VK_SNAPSHOT] = EVirtualKey::KEY_UNKNOWN;
			s_keymap[VK_PAUSE] = EVirtualKey::KEY_PAUSE;
			s_keymap[VK_F1] = EVirtualKey::KEY_F1;
			s_keymap[VK_F2] = EVirtualKey::KEY_F2;
			s_keymap[VK_F3] = EVirtualKey::KEY_F3;
			s_keymap[VK_F4] = EVirtualKey::KEY_F4;
			s_keymap[VK_F5] = EVirtualKey::KEY_F5;
			s_keymap[VK_F6] = EVirtualKey::KEY_F6;
			s_keymap[VK_F7] = EVirtualKey::KEY_F7;
			s_keymap[VK_F8] = EVirtualKey::KEY_F8;
			s_keymap[VK_F9] = EVirtualKey::KEY_F9;
			s_keymap[VK_F10] = EVirtualKey::KEY_F10;
			s_keymap[VK_F11] = EVirtualKey::KEY_F11;
			s_keymap[VK_F12] = EVirtualKey::KEY_F12;

			s_keymap[VK_NUMPAD0] = EVirtualKey::KEY_NUMPAD0;
			s_keymap[VK_NUMPAD1] = EVirtualKey::KEY_NUMPAD1;
			s_keymap[VK_NUMPAD2] = EVirtualKey::KEY_NUMPAD2;
			s_keymap[VK_NUMPAD3] = EVirtualKey::KEY_NUMPAD3;
			s_keymap[VK_NUMPAD4] = EVirtualKey::KEY_NUMPAD4;
			s_keymap[VK_NUMPAD5] = EVirtualKey::KEY_NUMPAD5;
			s_keymap[VK_NUMPAD6] = EVirtualKey::KEY_NUMPAD6;
			s_keymap[VK_NUMPAD7] = EVirtualKey::KEY_NUMPAD7;
			s_keymap[VK_NUMPAD8] = EVirtualKey::KEY_NUMPAD8;
			s_keymap[VK_NUMPAD9] = EVirtualKey::KEY_NUMPAD9;

			s_keymap[VK_DECIMAL] = EVirtualKey::KEY_DECIMAL;
			s_keymap[VK_DIVIDE] = EVirtualKey::KEY_DIVIDE;
			s_keymap[VK_MULTIPLY] = EVirtualKey::KEY_MULTIPLY;
			s_keymap[VK_SUBTRACT] = EVirtualKey::KEY_SUBTRACT;
			s_keymap[VK_ADD] = EVirtualKey::KEY_ADD;
			s_keymap[VK_SEPARATOR] = EVirtualKey::KEY_RETURN;
			s_keymap[VK_LSHIFT] = EVirtualKey::KEY_LSHIFT;
			s_keymap[VK_LCONTROL] = EVirtualKey::KEY_LCONTROL;
			s_keymap[VK_RSHIFT] = EVirtualKey::KEY_RSHIFT;
			s_keymap[VK_RCONTROL] = EVirtualKey::KEY_RCONTROL;
			s_keymap[VK_LMENU] = EVirtualKey::KEY_LMENU;
			s_keymap[VK_RMENU] = EVirtualKey::KEY_RMENU;

			s_keymap[VK_SHIFT] = EVirtualKey::KEY_SHIFT;
			s_keymap[VK_CONTROL] = EVirtualKey::KEY_CONTROL;
			s_keymap[VK_MENU] = EVirtualKey::KEY_ALT;
		}
	}

	static EVirtualKey Win32VirtualKeyToParaVK(DWORD vk)
	{

		InitVirtualKeyMap();
		auto result = s_keymap.find(vk);
		if (result != s_keymap.end())
		{
			return result->second;
		}
		return EVirtualKey::KEY_UNKNOWN;
	}



	static DWORD ParaVKToWin32VirtualKey(EVirtualKey key)
	{
		InitVirtualKeyMap();
		for (auto kv : s_keymap)
		{
			if (kv.second == key)
			{
				return kv.first;
			}
		}
		//assert(false);
		return -1;
	}



	std::unordered_map<HWND, RenderWindowWin32*> RenderWindowWin32::g_WindowMap;
	const WCHAR* RenderWindowWin32::ClassName = L"ParaWorld";

	LRESULT RenderWindowWin32::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (g_WindowMap.find(hWnd) == g_WindowMap.end())
		{
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}

		RenderWindowWin32* window = g_WindowMap[hWnd];
		assert(window);
		assert(window->GetHandle() == hWnd);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		
		window->WindowXYToRenderXY(xPos, yPos, xPos, yPos);

		switch (message)
		{
		case WM_MOUSEMOVE:
		{
			window->m_MousePos.x = (float)xPos;
			window->m_MousePos.y = (float)yPos;
			window->OnMouseMove(xPos, yPos);
		}
		break;
		case WM_LBUTTONDOWN:
			window->m_MouseState[(uint32_t)EMouseButton::LEFT] = EKeyState::PRESS;
			window->OnMouseButton(EMouseButton::LEFT, EKeyState::PRESS, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_LBUTTONUP:
			window->m_MouseState[(uint32_t)EMouseButton::LEFT] = EKeyState::RELEASE;
			window->OnMouseButton(EMouseButton::LEFT, EKeyState::RELEASE, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_RBUTTONDOWN:
			window->m_MouseState[(uint32_t)EMouseButton::RIGHT] = EKeyState::PRESS;
			window->OnMouseButton(EMouseButton::RIGHT, EKeyState::PRESS, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_RBUTTONUP:
			window->m_MouseState[(uint32_t)EMouseButton::RIGHT] = EKeyState::RELEASE;
			window->OnMouseButton(EMouseButton::RIGHT, EKeyState::RELEASE, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_MBUTTONDOWN:
			window->m_MouseState[(uint32_t)EMouseButton::MIDDLE] = EKeyState::PRESS;
			window->OnMouseButton(EMouseButton::MIDDLE, EKeyState::PRESS, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_MBUTTONUP:
			window->m_MouseState[(uint32_t)EMouseButton::MIDDLE] = EKeyState::RELEASE;
			window->OnMouseButton(EMouseButton::MIDDLE, EKeyState::RELEASE, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			window->OnMouseWheel((float)xPos, (float)yPos, (float)zDelta);
		}
		break;
		case WM_CHAR:
		case WM_UNICHAR:
		{
			unsigned int code = (unsigned int)wParam;

			if (message == WM_UNICHAR && wParam == UNICODE_NOCHAR)
			{
				// WM_UNICHAR is not sent by Windows, but is sent by some
				// third-party input method engine
				// Returning TRUE here announces support for this message
				return TRUE;
			}

			window->OnChar(code);
		}
		break;

		case WM_KILLFOCUS:

			window->m_bLostFocus = true;

			break;

		case WM_SETFOCUS:

			window->m_bLostFocus = false;

			break;

		case WM_SIZE:
		{
			int nWidth = LOWORD(lParam);
			int nHeight = HIWORD(lParam);
			// window->OnSize(nWidth, nHeight);
			window->SetWindowSize(nWidth, nHeight);
			break;
		}
		case WM_DROPFILES:
		{
			HDROP query = (HDROP)wParam;
			int n = 0, count = DragQueryFile(query, 0xFFFFFFFF, 0, 0);
			std::string cmds = "";
			while (n < count) {
				char filename[MAX_PATH];
				if (DragQueryFile(query, n, filename, MAX_PATH) != 0)
				{
					OUTPUT_LOG("drop files: %s\n", filename);
					cmds += filename;
					cmds += ";";
				}
				n++;
			}

			window->OnDropFiles(cmds);

			DragFinish(query);
			break;
		}

		case WM_DESTROY:
			// close the application entirely
			PostQuitMessage(0);
			window->m_IsQuit = true;
			break;
		default:
			break;
		}

		// Handle any messages the switch statement didn't
		return  DefWindowProcW(hWnd, message, wParam, lParam);
	}

	void RenderWindowWin32::WindowXYToRenderXY(int window_x, int window_y, int& render_x, int& render_y)
	{
		if (m_screen_rotated)
		{
			// 逆时针旋转
			render_x = m_window_height - 1 - window_y;
			render_y = window_x;
		}
		else
		{
			render_x = window_x;
			render_y = window_y;
		}
	}

	void RenderWindowWin32::SetScreenOrientation(int orientation) 
	{
		m_screen_orientation = orientation; 
		SetWindowSize(m_window_width, m_window_height); 
	}

	void RenderWindowWin32::SetWindowSize(int width, int height)
	{
		m_window_width = width;
		m_window_height = height;

		m_screen_rotated = m_screen_orientation == s_screen_orientation_landscape && m_window_width < m_window_height;
		m_screen_rotated = m_screen_rotated || (m_screen_orientation == s_screen_orientation_portrait && m_window_width > m_window_height);
		
		if (m_screen_rotated)
		{
			m_Width = m_window_height;
			m_Height = m_window_width;
		}
		else
		{
			m_Width = m_window_width;
			m_Height = m_window_height;
		}
		OnSize(m_Width, m_Height);
	}

	void RenderWindowWin32::SetSize(int w, int h)
	{
		if (!m_hWnd)
			return;

		RECT rect;
		::GetWindowRect(m_hWnd, &rect);

		rect.right = rect.left + w;
		rect.bottom = rect.top + h;

		RECT oldRect = rect;

		AdjustWindowRectEx(&rect, GetWindowLong(m_hWnd, GWL_STYLE),
			FALSE, GetWindowLong(m_hWnd, GWL_EXSTYLE));

		auto offsetX = rect.left - oldRect.left;
		auto offsetY = rect.top - oldRect.top;

		rect.left = oldRect.left;
		rect.top = oldRect.top;
		rect.right -= offsetX;
		rect.bottom -= offsetY;

		MoveWindow(m_hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

		m_Width = w;
		m_Height = h;
	}

	bool RenderWindowWin32::Create(HINSTANCE hInstance, int defaultWdith, int defaultHeight)
	{
		if (m_hWnd)
			return true;

		m_Width = defaultWdith;
		m_Height = defaultHeight;

		m_window_width = m_Width;
		m_window_height = m_Height;

		WNDCLASSW wndClass = { 0, RenderWindowWin32::WindowProc, 0, 0, hInstance,
			NULL,
			LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)GetStockObject(WHITE_BRUSH),
			NULL,
			L"ParaWorld"
		};
		wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndClass.hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PARAWORLD_ICON), IMAGE_ICON, 48, 48, 0);
		RegisterClassW(&wndClass);

		DWORD exStyle = WS_EX_APPWINDOW;

		// Set the window's initial style
		DWORD dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME |
			WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
		// Create the render window
		RECT rect;
		rect.left = 0;
		rect.right = defaultWdith;
		rect.top = 0;
		rect.bottom = defaultHeight;

		if (!AdjustWindowRect(&rect, dwWindowStyle, false))
		{
			OUTPUT_LOG("AdjustWindowRect failed.");
			return false;
		}

		m_hWnd = CreateWindowExW(exStyle
			, RenderWindowWin32::ClassName
			, L"ParaEngine Window"
			, dwWindowStyle,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top
			, NULL
			, NULL
			, hInstance
			, NULL);

		g_WindowMap[m_hWnd] = this;

		// Load keyboard accelerators
		m_hAccel = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDR_MAIN_ACCEL));

		DragAcceptFiles(m_hWnd, TRUE);
		ShowWindow(m_hWnd, SW_SHOW);
		UpdateWindow(m_hWnd);

		return true;
	}

	RenderWindowWin32::RenderWindowWin32()
		: m_hWnd(nullptr)
		, m_hAccel(nullptr)
		, m_Width(0)
		, m_Height(0)
		, m_IsQuit(false)
		, m_bLostFocus(false)
	{
		InitInput();

		m_window_width = 0;
		m_window_height = 0;
		m_screen_orientation = s_screen_orientation_auto;
		m_screen_rotated = false;
	}

	RenderWindowWin32::~RenderWindowWin32()
	{
		if (g_WindowMap.find(m_hWnd) != g_WindowMap.end())
		{
			g_WindowMap.erase(m_hWnd);
		}

		m_hWnd = nullptr;
	}

	void RenderWindowWin32::CheckFocus()
	{
		if (m_bLostFocus)
		{
			::SetFocus(m_hWnd);
		}
	}

	bool RenderWindowWin32::ShouldClose() const
	{
		return m_IsQuit;
	}

	void RenderWindowWin32::PollEvents()
	{
		MSG  msg;
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			ProcessInput(msg);
			if (TranslateAcceleratorW(m_hWnd, m_hAccel, &msg) != 0) return;
			// translate keystroke messages into the right format
			TranslateMessage(&msg);

			// send the message to the WindowProc function
			DispatchMessageW(&msg);
		}
	}

	intptr_t RenderWindowWin32::GetNativeHandle() const
	{
		return (intptr_t)GetHandle();
	}

	HWND RenderWindowWin32::GetHandle() const
	{
		return m_hWnd;
	}

	unsigned int RenderWindowWin32::GetWidth() const
	{
		return m_Width;
	}

	unsigned int RenderWindowWin32::GetHeight() const
	{
		return m_Height;
	}

	bool RenderWindowWin32::IsWindowed() const
	{
		return m_Windowed;
	}


	EKeyState RenderWindowWin32::GetMouseButtonState(EMouseButton button)
	{
		assert(button != EMouseButton::COUNT);
		return m_MouseState[(unsigned int)button];
	}

	EKeyState RenderWindowWin32::GetKeyState(EVirtualKey key)
	{
		assert(key != EVirtualKey::COUNT && key != EVirtualKey::KEY_UNKNOWN);
		return m_KeyState[(uint32_t)key];
	}

	CVector2 RenderWindowWin32::GetMousePos()
	{
		return m_MousePos;
	}

	void RenderWindowWin32::ProcessInput(const MSG& msg)
	{

		switch (msg.message)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			
			bool bDown = msg.message == WM_SYSKEYDOWN || msg.message == WM_KEYDOWN;
			EKeyState state = bDown ? EKeyState::PRESS : EKeyState::RELEASE;
		
			DWORD msgKey = (DWORD)msg.wParam;
			{
				if (msgKey == VK_SHIFT)
				{
					auto scancode = (msg.lParam >> 16) & 0x7f;
					msgKey = (scancode == 42) ? VK_LSHIFT : VK_RSHIFT;
				}
				else if (msgKey == VK_CONTROL)
				{
					msgKey = VK_LCONTROL + ((msg.lParam & (1 << 24)) ? 1 : 0);
				}
			}

			auto key = Win32VirtualKeyToParaVK(msgKey);

			if (key != EVirtualKey::KEY_UNKNOWN)
			{
				m_KeyState[(uint32_t)key] = state;
			}
			OnKey(key, state);
		}
		break;

		default:
			break;
		}



	}

	void RenderWindowWin32::InitInput()
	{
		// Init mouse state
		for (uint32_t i = 0; i < (uint32_t)EMouseButton::COUNT; i++)
		{
			m_MouseState[i] = EKeyState::RELEASE;
		}
		// Init key state
		for (uint32_t i = 0; i < (uint32_t)EVirtualKey::COUNT; i++)
		{
			m_KeyState[i] = EKeyState::RELEASE;
		}
	}

} // end namespace
