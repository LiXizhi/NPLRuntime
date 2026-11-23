#include "ParaEngine.h"
#include "RenderWindowWin32.h"
#include "resource.h"
#include "Winuser.h"
#include "IParaEngineApp.h"
#include "ParaEngineAppBase.h"
#include "EventClasses.h"
#include "EventsCenter.h"
#include "util/StringHelper.h"
#include "2dengine/GUIRoot.h"
#ifdef PARA_ENABLE_WIN32_IME
#include "2dengine/GUIIME.h"
#endif
#include <shellapi.h>
#include <Imm.h>

#include <unordered_map>

#pragma comment(lib, "Imm32.lib")

#ifndef ID_GAME_COMMANDLINE
#define ID_GAME_COMMANDLINE 40061
#endif
#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDATA 0x0049
#endif
#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif
#ifndef WM_POINTERUPDATE
#define WM_POINTERUPDATE 0x0245
#define WM_POINTERDOWN 0x0246
#define WM_POINTERUP 0x0247
#define WM_POINTERENTER 0x0249
#define WM_POINTERLEAVE 0x024A
#endif
#ifndef GET_POINTERID_WPARAM
#define GET_POINTERID_WPARAM(wParam) (LOWORD(wParam))
#endif


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
			s_keymap['J'] = EVirtualKey::KEY_J;
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
			s_keymap[VK_PRIOR] = EVirtualKey::KEY_PRIOR;
			s_keymap[VK_NEXT] = EVirtualKey::KEY_NEXT;
			s_keymap[VK_HOME] = EVirtualKey::KEY_HOME;
			s_keymap[VK_END] = EVirtualKey::KEY_END;
			s_keymap[VK_CAPITAL] = EVirtualKey::KEY_CAPITAL;
			s_keymap[VK_SCROLL] = EVirtualKey::KEY_SCROLL;
			s_keymap[VK_NUMLOCK] = EVirtualKey::KEY_NUMLOCK;
			s_keymap[VK_SNAPSHOT] = EVirtualKey::KEY_SYSRQ;
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
			s_keymap[VK_SHIFT] = EVirtualKey::KEY_SHIFT;
			s_keymap[VK_CONTROL] = EVirtualKey::KEY_CONTROL;
			s_keymap[VK_MENU] = EVirtualKey::KEY_LMENU;
			s_keymap[VK_LMENU] = EVirtualKey::KEY_LMENU;
			s_keymap[VK_RMENU] = EVirtualKey::KEY_RMENU;
			s_keymap[VK_LWIN] = EVirtualKey::KEY_LWIN;
			s_keymap[VK_RWIN] = EVirtualKey::KEY_RWIN;
			s_keymap[VK_APPS] = EVirtualKey::KEY_APPS;
			s_keymap[VK_OEM_MINUS] = EVirtualKey::KEY_MINUS;
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

	static bool IsSynthesizedTouchMouseMessage()
	{
		// MI_WP_SIGNATURE = 0xFF515700, SIGNATURE_MASK = 0xFFFFFF00 (touch/pen = 0xFF515780)
		return (::GetMessageExtraInfo() & 0xFFFFFF80) == 0xFF515780;
	}

#ifdef PARA_ENABLE_WIN32_IME
	static bool HasActiveWin32IMEFocus()
	{
		auto* pRoot = CGUIRoot::GetInstance();
		if (!pRoot)
			return false;
		auto* pKeyTarget = pRoot->GetUIKeyFocus();
		if (!pKeyTarget || !pKeyTarget->IsInputMethodEnabled() || !pKeyTarget->GetVisibleRecursive())
			return false;
		if (pRoot->HasIMEFocus())
			return true;
		auto* pImeFocus = pRoot->GetIMEFocus();
		return pImeFocus != nullptr && pImeFocus->GetVisibleRecursive();
	}
#endif

	LRESULT RenderWindowWin32::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto itWnd = g_WindowMap.find(hWnd);
		if (itWnd == g_WindowMap.end())
		{
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}

		RenderWindowWin32* window = itWnd->second;
		assert(window);
		assert(window->GetHandle() == hWnd);
		WNDPROC fallbackProc = window->m_pOldWndProc ? window->m_pOldWndProc : DefWindowProcW;

		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		if (CGlobals::GetApp())
		{
			CGlobals::GetApp()->ClientToGame(xPos, yPos, true);
		}
		else
		{
			window->WindowXYToRenderXY(xPos, yPos, xPos, yPos);
		}

		// Cross-thread ParaEngine custom messages
		if (message >= PE_WM_FIRST && message <= PE_WM_LAST)
		{
			switch (message)
			{
			case PE_WM_SHOWCURSOR:
				if (CGUIRoot::GetInstance() && CGUIRoot::GetInstance()->GetMouse())
					CGUIRoot::GetInstance()->GetMouse()->ForceShowCursor(wParam != 0);
				return 0;
			case PE_WM_SETCAPTURE:
				::SetCapture(hWnd);
				return 0;
			case PE_WM_RELEASECAPTURE:
				::ReleaseCapture();
				return 0;
			case PE_WM_SETFOCUS:
				::SetFocus(wParam ? reinterpret_cast<HWND>(wParam) : hWnd);
				return 0;
			case PE_WM_QUIT:
				window->ForceQuit();
				return 0;
#ifdef PARA_ENABLE_WIN32_IME
			case PE_IME_SETFOCUS:
				if (lParam == 1)
					CGUIIME::OnFocusIn_imp(hWnd);
				else
					CGUIIME::OnFocusOut_imp(hWnd);
				return 0;
			case PE_IME_SETOPENSTATUS:
				CGUIIME::SetIMEOpenStatus_imp(lParam != 0);
				return 0;
#endif
			default:
				break;
			}
		}

#ifdef PARA_ENABLE_WIN32_IME
		if (message == WM_IME_SETCONTEXT)
		{
			if (!HasActiveWin32IMEFocus())
			{
				lParam = 0;
			}
			else if (CGUIIME::IsEnableImeSystem())
			{
				lParam = 0;
				CGUIIME::HandleWinThreadMsg(message, wParam, lParam);
			}
			else
			{
				lParam = ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUICANDIDATEWINDOW;
			}
			return ::CallWindowProcW(fallbackProc, hWnd, message, wParam, lParam);
		}
		if (message == WM_KEYUP
			|| message == WM_INPUTLANGCHANGE
			|| message == WM_IME_STARTCOMPOSITION
			|| message == WM_IME_COMPOSITION
			|| message == WM_IME_ENDCOMPOSITION
			|| message == WM_IME_NOTIFY)
		{
			if (HasActiveWin32IMEFocus())
			{
				CGUIIME::HandleWinThreadMsg(message, wParam, lParam);
			}
			else if (message == WM_INPUTLANGCHANGE)
			{
				CGUIIME::OnFocusOut_imp(hWnd);
			}
			if (message != WM_KEYUP && CGUIIME::IsEnableImeSystem())
			{
				return 1;
			}
		}
#endif

		switch (message)
		{
		case WM_MOUSEMOVE:
		{
			if (IsSynthesizedTouchMouseMessage())
				return 0;
			window->m_MousePos.x = (float)xPos;
			window->m_MousePos.y = (float)yPos;
			window->OnMouseMove(xPos, yPos);
		}
		break;
		case WM_LBUTTONDOWN:
			if (IsSynthesizedTouchMouseMessage())
				return 0;
			if (CGlobals::GetApp())
				CGlobals::GetApp()->SetTouchInputting(false);
			window->m_MouseState[(uint32_t)EMouseButton::LEFT] = EKeyState::PRESS;
			window->OnMouseButton(EMouseButton::LEFT, EKeyState::PRESS, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_LBUTTONUP:
			if (IsSynthesizedTouchMouseMessage())
				return 0;
			window->m_MouseState[(uint32_t)EMouseButton::LEFT] = EKeyState::RELEASE;
			window->OnMouseButton(EMouseButton::LEFT, EKeyState::RELEASE, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_RBUTTONDOWN:
			if (IsSynthesizedTouchMouseMessage())
				return 0;
			if (CGlobals::GetApp())
				CGlobals::GetApp()->SetTouchInputting(false);
			window->m_MouseState[(uint32_t)EMouseButton::RIGHT] = EKeyState::PRESS;
			window->OnMouseButton(EMouseButton::RIGHT, EKeyState::PRESS, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_RBUTTONUP:
			if (IsSynthesizedTouchMouseMessage())
				return 0;
			window->m_MouseState[(uint32_t)EMouseButton::RIGHT] = EKeyState::RELEASE;
			window->OnMouseButton(EMouseButton::RIGHT, EKeyState::RELEASE, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_MBUTTONDOWN:
			if (IsSynthesizedTouchMouseMessage())
				return 0;
			if (CGlobals::GetApp())
				CGlobals::GetApp()->SetTouchInputting(false);
			window->m_MouseState[(uint32_t)EMouseButton::MIDDLE] = EKeyState::PRESS;
			window->OnMouseButton(EMouseButton::MIDDLE, EKeyState::PRESS, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_MBUTTONUP:
			if (IsSynthesizedTouchMouseMessage())
				return 0;
			window->m_MouseState[(uint32_t)EMouseButton::MIDDLE] = EKeyState::RELEASE;
			window->OnMouseButton(EMouseButton::MIDDLE, EKeyState::RELEASE, xPos, yPos);
			window->CheckFocus();
			break;
		case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			POINT ptWheel = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			::ScreenToClient(hWnd, &ptWheel);
			if (CGlobals::GetApp())
				CGlobals::GetApp()->ClientToGame(reinterpret_cast<int&>(ptWheel.x), reinterpret_cast<int&>(ptWheel.y), true);
			window->OnMouseWheel((float)ptWheel.x, (float)ptWheel.y, (float)zDelta);
		}
		break;

		case WM_POINTERENTER:
		case WM_POINTERDOWN:
		case WM_POINTERUPDATE:
		case WM_POINTERUP:
		case WM_POINTERLEAVE:
		{
			if (CGUIRoot::GetInstance() && CGlobals::GetApp() && CGlobals::GetApp()->GetAppState() == PEAppState_Ready)
			{
				CGlobals::GetApp()->SetTouchInputting(true);
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(hWnd, &pt);
				int tx = pt.x;
				int ty = pt.y;
				CGlobals::GetApp()->ClientToGame(tx, ty, true);
				TouchEvent::TouchEventMsgType touchType = TouchEvent::TouchEvent_POINTER_INVALID;
				switch (message)
				{
				case WM_POINTERENTER: touchType = TouchEvent::TouchEvent_POINTER_ENTER; break;
				case WM_POINTERDOWN: touchType = TouchEvent::TouchEvent_POINTER_DOWN; break;
				case WM_POINTERUPDATE: touchType = TouchEvent::TouchEvent_POINTER_UPDATE; break;
				case WM_POINTERUP: touchType = TouchEvent::TouchEvent_POINTER_UP; break;
				case WM_POINTERLEAVE: touchType = TouchEvent::TouchEvent_POINTER_LEAVE; break;
				default: break;
				}
				int pointerId = static_cast<int>(GET_POINTERID_WPARAM(wParam));
				TouchEvent touch(EH_TOUCH, touchType, pointerId, static_cast<float>(tx), static_cast<float>(ty), static_cast<int>(::GetTickCount()));
				CGUIRoot::GetInstance()->handleTouchEvent(touch);
				return 0;
			}
			break;
		}

		case WM_CHAR:
		case WM_UNICHAR:
		{
			unsigned int code = (unsigned int)wParam;

			if (message == WM_UNICHAR && wParam == UNICODE_NOCHAR)
			{
				return TRUE;
			}
#ifdef PARA_ENABLE_WIN32_IME
			if (HasActiveWin32IMEFocus())
			{
				CGUIIME::SendWinMsgChar((WCHAR)code);
				return 0;
			}
#endif
			window->OnChar(code);
		}
		break;

		case WM_ACTIVATEAPP:
		{
			bool bActive = (wParam != FALSE);
			if (!bActive)
			{
				::ClipCursor(NULL);
				window->ClearInputStates();
			}
#ifdef PARA_ENABLE_WIN32_IME
			else
			{
				CGUIIME::StaticMsgProc(message, wParam, lParam);
			}
#endif
			if (CGlobals::GetApp())
			{
				CGlobals::GetApp()->ActivateApp(bActive);
			}
			break;
		}

		case WM_MOUSEACTIVATE:
		{
			if (window->m_bIsChildWindow || window->m_bIsExternalWindow)
			{
				::SetFocus(hWnd);
				if (CGlobals::GetApp() && !CGlobals::GetApp()->IsAppActive())
					CGlobals::GetApp()->ActivateApp(true);
				return MA_ACTIVATE;
			}
			break;
		}

		case WM_KILLFOCUS:
			window->m_bLostFocus = true;
			window->ClearInputStates();
			::ClipCursor(NULL);
			break;

		case WM_SETFOCUS:
			window->m_bLostFocus = false;
#ifdef PARA_ENABLE_WIN32_IME
			if (!HasActiveWin32IMEFocus())
			{
				CGUIIME::OnFocusOut_imp(hWnd);
			}
#endif
			break;

		case WM_SETCURSOR:
		{
			if (LOWORD(lParam) == HTCLIENT && CGUIRoot::GetInstance() && CGUIRoot::GetInstance()->GetMouse())
			{
				if (CGUIRoot::GetInstance()->GetMouse()->ApplySystemCursor())
					return TRUE;
			}
			break;
		}

		case WM_NCHITTEST:
		{
			LRESULT hit = ::CallWindowProcW(fallbackProc, hWnd, message, wParam, lParam);
			if (window->IsWindowed() && !window->m_bIsChildWindow
				&& CGUIRoot::GetInstance() && CGlobals::GetApp()
				&& CGlobals::GetApp()->GetAppState() == PEAppState_Ready)
			{
				POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				::ScreenToClient(hWnd, &pt);
				int gx = pt.x;
				int gy = pt.y;
				CGlobals::GetApp()->ClientToGame(gx, gy, true);
				MouseEvent me(0, gx, gy, (hit == HTCLIENT) ? 1 : 0);
				CGUIRoot::GetInstance()->handleNonClientTest(me);
				if (hit == HTCLIENT && CGUIRoot::GetInstance()->IsNonClient())
				{
					return HTCAPTION;
				}
			}
			return hit;
		}

		case WM_DPICHANGED:
		{
			if (window->IsWindowed() && !window->m_bIsChildWindow && lParam != 0)
			{
				const RECT* prcNewWindow = reinterpret_cast<const RECT*>(lParam);
				::SetWindowPos(hWnd, NULL,
					prcNewWindow->left,
					prcNewWindow->top,
					prcNewWindow->right - prcNewWindow->left,
					prcNewWindow->bottom - prcNewWindow->top,
					SWP_NOZORDER | SWP_NOACTIVATE);
				return 0;
			}
			break;
		}

		case WM_SETTINGCHANGE:
		{
			if (lParam != 0 && CGlobals::GetEventsCenter())
			{
				const wchar_t* settingName = reinterpret_cast<const wchar_t*>(lParam);
				if (wcscmp(settingName, L"ConvertibleSlateMode") == 0 || wcscmp(settingName, L"SystemDockMode") == 0)
				{
					SystemEvent ev(SystemEvent::SYS_WM_SETTINGCHANGE, "");
					CGlobals::GetEventsCenter()->FireEvent(ev);
				}
			}
			break;
		}

		case WM_COPYDATA:
		{
			const COPYDATASTRUCT* pCDS = reinterpret_cast<const COPYDATASTRUCT*>(lParam);
			if (pCDS && pCDS->dwData == ID_GAME_COMMANDLINE && pCDS->lpData && pCDS->cbData > 0)
			{
				const char* pChars = static_cast<const char*>(pCDS->lpData);
				size_t len = 0;
				while (len < static_cast<size_t>(pCDS->cbData) && pChars[len] != '\0')
					++len;
				std::string cmdStr(pChars, len);
				if (auto* pAppBase = dynamic_cast<CParaEngineAppBase*>(CGlobals::GetApp()))
				{
					pAppBase->onCmdLine(cmdStr);
				}
				return TRUE;
			}
			break;
		}

		case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED)
			{
				int nWidth = LOWORD(lParam);
				int nHeight = HIWORD(lParam);
				if (nWidth > 0 && nHeight > 0)
				{
					window->SetWindowSize(nWidth, nHeight);
				}
			}
			break;
		}

		case WM_DROPFILES:
		{
			HDROP query = (HDROP)wParam;
			UINT count = ::DragQueryFileW(query, 0xFFFFFFFF, nullptr, 0);
			std::string cmds;
			for (UINT n = 0; n < count; ++n)
			{
				UINT charLen = ::DragQueryFileW(query, n, nullptr, 0);
				if (charLen > 0)
				{
					std::wstring wFilename(charLen + 1, L'\0');
					if (::DragQueryFileW(query, n, &wFilename[0], charLen + 1) != 0)
					{
						wFilename.resize(wcslen(wFilename.c_str()));
						std::string utf8File = StringHelper::WideCharToMultiByte(wFilename.c_str(), CP_UTF8);
						OUTPUT_LOG("drop files: %s\n", utf8File.c_str());
						cmds += utf8File;
						cmds += ";";
					}
				}
			}
			window->OnDropFiles(cmds);
			::DragFinish(query);
			return 0;
		}

		case WM_CLOSE:
		{
			if (CGlobals::GetApp() && !CGlobals::GetApp()->IsWindowClosingAllowed() && !window->m_bForceClosing)
			{
				if (CGlobals::GetEventsCenter())
				{
					SystemEvent event(SystemEvent::SYS_WM_CLOSE, "msg={type=\"WM_CLOSE\"};");
					CGlobals::GetEventsCenter()->FireEvent(event);
				}
				return 0;
			}
			if (CGlobals::GetApp())
			{
				CGlobals::GetApp()->SetHasClosingRequest(true);
			}
			break;
		}

		case WM_DESTROY:
			::ClipCursor(NULL);
			if (!window->m_bIsExternalWindow)
			{
				PostQuitMessage(0);
			}
			window->m_IsQuit = true;
			break;

		case WM_SYSCOMMAND:
			if ((wParam & 0xFFF0) == SC_KEYMENU)
			{
				return 0;
			}
			break;
		
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			if ((wParam == VK_F4 || wParam == VK_SPACE) && (lParam & (1 << 29)) != 0)
			{
				break; // Allow Alt+F4 and Alt+Space
			}
			return 0;
		
		default:
			break;
		}

		return ::CallWindowProcW(fallbackProc, hWnd, message, wParam, lParam);
	}

	void RenderWindowWin32::WindowXYToRenderXY(int window_x, int window_y, int& render_x, int& render_y)
	{
		const int srcX = window_x;
		const int srcY = window_y;
		if (m_screen_rotated)
		{
			// 逆时针旋转
			render_x = m_window_height - 1 - srcY;
			render_y = srcX;
		}
		else
		{
			render_x = srcX;
			render_y = srcY;
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
		
		// m_screen_rotated = true;
		
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

		if (m_bIsChildWindow || m_bIsExternalWindow || !m_Windowed)
		{
			if (m_bIsChildWindow)
			{
				::MoveWindow(m_hWnd, 0, 0, w, h, TRUE);
			}
			m_window_width = w;
			m_window_height = h;
			m_Width = m_screen_rotated ? m_window_height : m_window_width;
			m_Height = m_screen_rotated ? m_window_width : m_window_height;
			return;
		}

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

		m_window_width = w;
		m_window_height = h;
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
	}

	static void AllowUipiWindowMessages(HWND hWnd)
	{
		HMODULE hUser32 = ::GetModuleHandleW(L"user32.dll");
		if (!hUser32 || !hWnd)
			return;
		typedef BOOL(WINAPI* PChangeWindowMessageFilterEx)(HWND, UINT, DWORD, PCHANGEFILTERSTRUCT);
		auto pFilterEx = reinterpret_cast<PChangeWindowMessageFilterEx>(::GetProcAddress(hUser32, "ChangeWindowMessageFilterEx"));
		if (pFilterEx)
		{
			// MSGFLT_ALLOW = 1
			pFilterEx(hWnd, WM_DROPFILES, 1, nullptr);
			pFilterEx(hWnd, WM_COPYDATA, 1, nullptr);
			pFilterEx(hWnd, WM_COPYGLOBALDATA, 1, nullptr);
		}
	}

	bool RenderWindowWin32::AttachExternalWindow(HWND hExternalWnd)
	{
		if (!hExternalWnd || !::IsWindow(hExternalWnd))
			return false;
		if (m_hWnd == hExternalWnd)
			return true;

		m_hWnd = hExternalWnd;
		m_bIsExternalWindow = true;
		m_Windowed = true;

		RECT rcClient = { 0, 0, 0, 0 };
		::GetClientRect(m_hWnd, &rcClient);
		m_Width = (rcClient.right > rcClient.left) ? (rcClient.right - rcClient.left) : 1020;
		m_Height = (rcClient.bottom > rcClient.top) ? (rcClient.bottom - rcClient.top) : 680;
		m_window_width = m_Width;
		m_window_height = m_Height;

		g_WindowMap[m_hWnd] = this;
		WNDPROC curProc = reinterpret_cast<WNDPROC>(::GetWindowLongPtrW(m_hWnd, GWLP_WNDPROC));
		if (curProc != RenderWindowWin32::WindowProc)
		{
			m_pOldWndProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(RenderWindowWin32::WindowProc)));
		}

		AllowUipiWindowMessages(m_hWnd);
		::DragAcceptFiles(m_hWnd, TRUE);
#ifdef PARA_ENABLE_WIN32_IME
		CGUIIME::StaticOnCreateDevice(m_hWnd);
#else
		::ImmAssociateContext(m_hWnd, NULL);
#endif
		return true;
	}

	bool RenderWindowWin32::Create(HINSTANCE hInstance, int defaultWdith, int defaultHeight, HWND hParentWnd)
	{
		if (m_hWnd)
			return true;

		if (!hInstance)
			hInstance = ::GetModuleHandleW(nullptr);

		m_hParentWnd = (hParentWnd && ::IsWindow(hParentWnd)) ? hParentWnd : nullptr;
		m_bIsChildWindow = (m_hParentWnd != nullptr);

		if (m_bIsChildWindow && (defaultWdith <= 0 || defaultHeight <= 0))
		{
			RECT rcParent = { 0, 0, 0, 0 };
			if (::GetClientRect(m_hParentWnd, &rcParent))
			{
				if (defaultWdith <= 0) defaultWdith = rcParent.right - rcParent.left;
				if (defaultHeight <= 0) defaultHeight = rcParent.bottom - rcParent.top;
			}
		}
		if (defaultWdith <= 0) defaultWdith = 1020;
		if (defaultHeight <= 0) defaultHeight = 680;

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

		DWORD exStyle = m_bIsChildWindow ? 0 : WS_EX_APPWINDOW;
		DWORD dwWindowStyle = m_bIsChildWindow
			? (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
			: (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE);

		RECT rect = { 0, 0, defaultWdith, defaultHeight };
		if (!m_bIsChildWindow)
		{
			if (!AdjustWindowRect(&rect, dwWindowStyle, false))
			{
				OUTPUT_LOG("AdjustWindowRect failed.");
				return false;
			}
		}

		m_hWnd = CreateWindowExW(exStyle
			, RenderWindowWin32::ClassName
			, L"ParaEngine Window"
			, dwWindowStyle,
			m_bIsChildWindow ? 0 : CW_USEDEFAULT,
			m_bIsChildWindow ? 0 : CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top
			, m_hParentWnd
			, NULL
			, hInstance
			, NULL);

		if (!m_hWnd)
		{
			OUTPUT_LOG("CreateWindowExW failed (err=%lu).\n", ::GetLastError());
			return false;
		}

		g_WindowMap[m_hWnd] = this;

		AllowUipiWindowMessages(m_hWnd);

#ifdef PARA_ENABLE_WIN32_IME
		// Captures default HIMC (s_hImcDef) first, then disassociates IME until a CGUIIMEEditBox focuses
		CGUIIME::StaticOnCreateDevice(m_hWnd);
#else
		ImmAssociateContext(m_hWnd, NULL);
#endif

		// Load keyboard accelerators
		m_hAccel = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDR_MAIN_ACCEL));

		DragAcceptFiles(m_hWnd, TRUE);
		ShowWindow(m_hWnd, SW_SHOW);
		UpdateWindow(m_hWnd);

		return true;
	}

	RenderWindowWin32::RenderWindowWin32()
		: m_hWnd(nullptr)
		, m_hParentWnd(nullptr)
		, m_pOldWndProc(nullptr)
		, m_hAccel(nullptr)
		, m_Width(0)
		, m_Height(0)
		, m_Windowed(true)
		, m_IsQuit(false)
		, m_bForceClosing(false)
		, m_bIsChildWindow(false)
		, m_bIsExternalWindow(false)
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
		if (m_hWnd && m_pOldWndProc && ::IsWindow(m_hWnd))
		{
			::SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_pOldWndProc));
		}
		if (g_WindowMap.find(m_hWnd) != g_WindowMap.end())
		{
			g_WindowMap.erase(m_hWnd);
		}

		m_hWnd = nullptr;
		m_pOldWndProc = nullptr;
	}

	void RenderWindowWin32::ForceQuit()
	{
		m_bForceClosing = true;
		m_IsQuit = true;
		if (m_hWnd && !m_bIsExternalWindow && ::IsWindow(m_hWnd))
		{
			::DestroyWindow(m_hWnd);
		}
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
			if (m_hWnd && m_hAccel && TranslateAcceleratorW(m_hWnd, m_hAccel, &msg) != 0) continue;
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
				else if (msgKey == VK_MENU)
				{
					msgKey = (msg.lParam & (1 << 24)) ? VK_RMENU : VK_LMENU;
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

	void RenderWindowWin32::ClearInputStates()
	{
		// Clear all mouse button states and send release events for any pressed buttons
		for (uint32_t i = 0; i < (uint32_t)EMouseButton::COUNT; i++)
		{
			if (m_MouseState[i] == EKeyState::PRESS)
			{
				m_MouseState[i] = EKeyState::RELEASE;
				// Send mouse release event for any buttons that were pressed
				OnMouseButton((EMouseButton)i, EKeyState::RELEASE, (int)m_MousePos.x, (int)m_MousePos.y);
			}
		}
		// Clear all key states and send release events for any pressed keys
		for (uint32_t i = 0; i < (uint32_t)EVirtualKey::COUNT; i++)
		{
			if (m_KeyState[i] == EKeyState::PRESS)
			{
				m_KeyState[i] = EKeyState::RELEASE;
				// Send key release event for any keys that were pressed
				OnKey((EVirtualKey)i, EKeyState::RELEASE);
			}
		}
	}

} // end namespace
