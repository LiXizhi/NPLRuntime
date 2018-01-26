#include "ApplicationWindow.h"
#include "ParaEngine.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIDirectInput.h"
using namespace ParaEngine;
void ApplicationWindow::OnMouseButton(EMouseButton button, EKeyState state)
{

	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}


	uint32_t msg = 0;

	if (state == EKeyState::PRESS)
	{
		switch (button)
		{
		case ParaEngine::EMouseButton::LEFT:
			msg = WM_LBUTTONDOWN;
			break;
		case ParaEngine::EMouseButton::RIGHT:
			msg = WM_RBUTTONDOWN;
			break;
		case ParaEngine::EMouseButton::MIDDLE:
			msg = WM_MBUTTONDOWN;
			break;
		default:
			assert(false);
			break;
		}
	}
	else if (state == EKeyState::RELEASE)
	{
		switch (button)
		{
		case ParaEngine::EMouseButton::LEFT:
			msg = WM_LBUTTONUP;
			break;
		case ParaEngine::EMouseButton::RIGHT:
			msg = WM_RBUTTONUP;
			break;
		case ParaEngine::EMouseButton::MIDDLE:
			msg = WM_MBUTTONUP;
			break;
		default:
			assert(false);
			break;
		}
	}
	else
	{
		assert(false);
	}

	auto mousePos = GetMousePos();

	LPARAM param = MAKELPARAM((uint32_t)mousePos.x, (uint32_t)mousePos.y);
	CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(msg, 0, param);
}

void ApplicationWindow::OnMouseMove(uint32_t x, uint32_t y)
{
	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}

	WPARAM wParam = 0;
	if (GetMouseButtonState(EMouseButton::LEFT) == EKeyState::PRESS)
	{
		wParam |= MK_LBUTTON;
	}
	if (GetMouseButtonState(EMouseButton::RIGHT) == EKeyState::PRESS)
	{
		wParam |= MK_RBUTTON;
	}
	if (GetMouseButtonState(EMouseButton::MIDDLE) == EKeyState::PRESS)
	{
		wParam |= MK_MBUTTON;
	}

	LPARAM param = MAKELPARAM(x, y);
	CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MOUSEMOVE, wParam, param);
}

void ParaEngine::ApplicationWindow::OnMouseWhell(float x, float y, float delta)
{
	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}
	WPARAM wParam = MAKEWPARAM(0, (int)y * WHEEL_DELTA);
	CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(WM_MOUSEWHEEL, wParam, 0);
}

void ParaEngine::ApplicationWindow::OnKey(EVirtualKey key, EKeyState state)
{
	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}
	bool pressed = state == EKeyState::PRESS ? true : false;
	CGUIRoot::GetInstance()->GetKeyboard()->SetKeyPressed(key, pressed);
}

void ParaEngine::ApplicationWindow::OnChar(char character)
{
	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}
	auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();
	if (pGUI)
	{
		std::wstring s;
		s += (WCHAR)character;
		pGUI->OnHandleWinMsgChars(s);
	}
}

