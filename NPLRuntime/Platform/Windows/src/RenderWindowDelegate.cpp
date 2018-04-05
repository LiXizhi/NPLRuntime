#include "ParaEngine.h"
#include "RenderWindowDelegate.h"
#include "2dengine/GUIRoot.h"
using namespace ParaEngine;

extern HINSTANCE g_hAppInstance;
IRenderWindow* CreateParaRenderWindow(const int width, const int height)
{
	return new RenderWindowDelegate(g_hAppInstance, width, height);
}

void RenderWindowDelegate::OnMouseButton(EMouseButton button, EKeyState state,uint32_t x,uint32_t y)
{

	if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
	{
		return;
	}

	CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseButtonEvent(button,state,x,y)));
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
	CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseWheelEvent(y)));
}

void ParaEngine::RenderWindowDelegate::OnKey(EVirtualKey key, EKeyState state)
{
    if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
    {
        return;
    }
    bool pressed = state == EKeyState::PRESS ? true : false;
    if(pressed)
    {
        CGUIRoot::GetInstance()->SendKeyDownEvent(key);
    }else{
        CGUIRoot::GetInstance()->SendKeyUpEvent(key);
    }
}

void ParaEngine::RenderWindowDelegate::OnChar(unsigned int character)
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

