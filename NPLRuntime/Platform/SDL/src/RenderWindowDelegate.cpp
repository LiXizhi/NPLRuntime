#include "ParaEngine.h"
#include "RenderWindowDelegate.h"
#include "2dengine/GUIRoot.h"
#include "NPL/NPLHelper.h"
#include "SDL2Application.h"
#include "EventsCenter.h"
#include "util/StringHelper.h"

extern HINSTANCE g_hAppInstance;
ParaEngine::IRenderWindow *CreateParaRenderWindow(const int width, const int height)
{
	auto p = new ParaEngine::RenderWindowDelegate();
	p->Create(width, height);
	return p;
}

namespace ParaEngine
{

	void RenderWindowDelegate::OnTouch(int nType, TouchEvent::TouchEventMsgType nTouchType, int touch_id, float x, float y, int nTimeTick)
	{
		if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
		{
			return;
		}

		if (nTouchType == TouchEvent::TouchEvent_POINTER_DOWN)
		{
			((CSDL2Application *)(CGlobals::GetApp()))->SetTouchInputting(true);
		}

		TouchEvent e(nType, nTouchType, touch_id, x, y, nTimeTick);
		CGUIRoot::GetInstance()->handleTouchEvent(e);
	}

	void RenderWindowDelegate::OnMouseButton(EMouseButton button, EKeyState state, uint32_t x, uint32_t y)
	{
		if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
		{
			return;
		}

		if (state == EKeyState::PRESS)
		{
			((CSDL2Application *)(CGlobals::GetApp()))->SetTouchInputting(false);
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

	void RenderWindowDelegate::OnMouseWheel(float x, float y, float delta)
	{
		if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
		{
			return;
		}
		CGUIRoot::GetInstance()->GetMouse()->PushMouseEvent(DeviceMouseEventPtr(new DeviceMouseWheelEvent(delta)));
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
		else
		{
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

	void RenderWindowDelegate::OnChar(std::string sTextUTF8)
	{
		if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
		{
			return;
		}
		auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();
		if (pGUI)
		{
			std::u16string sTextWide;
			if (StringHelper::UTF8ToUTF16(sTextUTF8, sTextWide))
			{
				pGUI->OnHandleWinMsgChars(std::wstring(sTextWide.begin(), sTextWide.end()));
			}
		}
	}
	void RenderWindowDelegate::OnDropFiles(const std::string &files)
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
		static int s_last_width = 0;
		static int s_last_height = 0;
		auto app = CGlobals::GetApp();
		if (app->IsTouchInputting())
		{
			if (s_last_width != w && s_last_height != h)
			{
				std::cout << "last_width: " << s_last_width << " last_height: " << s_last_height << " width: " << w << " height: " << h << std::endl;
				s_last_width = w;
				s_last_height = h;
			}
			else if (s_last_width == w && s_last_height != h)
			{
				// 触屏方式忽略垂直高度变化(输入法影响)
				return;
			}

			if (((CSDL2Application *)(CGlobals::GetApp()))->IsInputing())
			{
				return;
			}
		}

		std::cout << "====OnSize====" << std::endl;

		if (w > 0 && h > 0)
		{
			app->SetResolution((float)w, (float)h);
			app->UpdateScreenMode();
		}
	}
} // end namespace
