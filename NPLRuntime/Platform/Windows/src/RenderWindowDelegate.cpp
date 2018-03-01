#include "RenderWindowDelegate.h"
#include "ParaEngine.h"
#include "2dengine/GUIRoot.h"
using namespace ParaEngine;
void RenderWindowDelegate::OnMouseButton(EMouseButton button, EKeyState state)
{

	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}

	CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseButtonEvent(button,state)));
}

void RenderWindowDelegate::OnMouseMove(uint32_t x, uint32_t y)
{
	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}
	CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseMoveEvent(x, y)));
}

void ParaEngine::RenderWindowDelegate::OnMouseWhell(float x, float y, float delta)
{
	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}
	CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseWhellEvent(y)));
}

void ParaEngine::RenderWindowDelegate::OnKey(EVirtualKey key, EKeyState state)
{
	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}
	bool pressed = state == EKeyState::PRESS ? true : false;
	CGUIRoot::GetInstance()->GetKeyboard()->SetKeyPressed(key, pressed);
}

void ParaEngine::RenderWindowDelegate::OnChar(char character)
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

