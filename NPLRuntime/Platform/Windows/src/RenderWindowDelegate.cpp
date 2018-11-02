#include "ParaEngine.h"
#include "RenderWindowDelegate.h"
#include "2dengine/GUIRoot.h"
#include "NPL/NPLHelper.h"
#include "EventsCenter.h"


extern HINSTANCE g_hAppInstance;
ParaEngine::IRenderWindow* CreateParaRenderWindow(const int width, const int height)
{
	auto p = new ParaEngine::RenderWindowDelegate();
	p->Create(g_hAppInstance, width, height);
	return p;
}

namespace ParaEngine {

	void RenderWindowDelegate::OnMouseButton(EMouseButton button, EKeyState state, uint32_t x, uint32_t y)
	{

		if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
		{
			return;
		}

		CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseButtonEvent(button, state, x, y)));
	}

	void RenderWindowDelegate::OnMouseMove(uint32_t x, uint32_t y)
	{
		if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
		{
			return;
		}
		CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseMoveEvent(x, y)));
	}

	void RenderWindowDelegate::OnMouseWhell(float x, float y, float delta)
	{
		if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
		{
			return;
		}
		CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseWheelEvent(y)));
	}

	void RenderWindowDelegate::OnKey(EVirtualKey key, EKeyState state)
	{
		if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
		{
			return;
		}
		bool pressed = state == EKeyState::PRESS ? true : false;
		if (pressed)
		{
			CGUIRoot::GetInstance()->SendKeyDownEvent(key);
		}
		else {
			CGUIRoot::GetInstance()->SendKeyUpEvent(key);
		}
	}

	void RenderWindowDelegate::OnChar(unsigned int character)
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

	void RenderWindowDelegate::OnDropFiles(const std::string& files)
	{
		if (!files.empty())
		{
			if (CGlobals::GetEventsCenter())
			{
				// msg = command line.
				std::string msg = "msg=";
				NPL::NPLHelper::EncodeStringInQuotation(msg, (int)msg.size(), files.c_str());
				msg.append(";");
				SystemEvent event(SystemEvent::SYS_WM_DROPFILES, msg);
				CGlobals::GetEventsCenter()->FireEvent(event);
			}
		}
	}

	void RenderWindowDelegate::OnSize(int w, int h)
	{
		auto app = CGlobals::GetApp();

		if (w > 0 && h > 0)
		{
			app->SetResolution((float)w, (float)h);
			app->UpdateScreenMode();
		}
	}
} // end namespace
