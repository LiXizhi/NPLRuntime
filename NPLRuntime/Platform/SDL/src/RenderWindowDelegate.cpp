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

	static std::unordered_map<EVirtualKey, uint64_t> s_keydown_map;
	static EVirtualKey s_ctrl_shift_alt_keys[] = {EVirtualKey::KEY_LCONTROL, EVirtualKey::KEY_RCONTROL, EVirtualKey::KEY_LSHIFT, EVirtualKey::KEY_RSHIFT, EVirtualKey::KEY_LMENU, EVirtualKey::KEY_RMENU};
	static int s_ctrl_shift_alt_keys_size = sizeof(s_ctrl_shift_alt_keys) / sizeof(s_ctrl_shift_alt_keys[0]);

	static void RefreshKeyDownMap()
	{
		static uint64_t last_refresh_timestamp = 0;
		uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		if (timestamp - last_refresh_timestamp < 1000)
		{
			return;
		}

		last_refresh_timestamp = timestamp;
		for (auto it = s_keydown_map.begin(); it != s_keydown_map.end();)
		{
			int timeout = 2000;
			for (int i = 0; i < s_ctrl_shift_alt_keys_size; i++)
			{
				if (s_ctrl_shift_alt_keys[i] == it->first)
				{
					timeout = 5000;
				}
			}

			if ((it->second + timeout) < timestamp)
			{
				CGUIRoot::GetInstance()->SendKeyUpEvent(it->first);
				it = s_keydown_map.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

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

	void RenderWindowDelegate::PollEvents()
	{
		RefreshKeyDownMap();
		RenderWindowSDL2::PollEvents();
	}

	void RenderWindowDelegate::OnKey(EVirtualKey key, EKeyState state)
	{
		if (CGlobals::GetApp()->GetAppState() != PEAppState_Ready)
		{
			return;
		}
		uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		bool pressed = state == EKeyState::PRESS ? true : false;
		if (pressed)
		{
			CGUIRoot::GetInstance()->SendKeyDownEvent(key);
			s_keydown_map.insert_or_assign(key, timestamp);

			for (int i = 0; i < s_ctrl_shift_alt_keys_size; i++)
			{
				auto i_key = s_ctrl_shift_alt_keys[i];
				if (s_keydown_map.find(i_key) != s_keydown_map.end())
				{
					s_keydown_map.insert_or_assign(i_key, timestamp);
				}
			}
		}
		else
		{
			CGUIRoot::GetInstance()->SendKeyUpEvent(key);
			s_keydown_map.erase(key);
		}

		RefreshKeyDownMap();
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

	void RenderWindowDelegate::OnClearChar(std::string text)
	{
		auto pGUI = CGUIRoot::GetInstance()->GetUIKeyFocus();
		if (pGUI)
			pGUI->SetTextA("");
		OnChar(text);
	}

	void RenderWindowDelegate::OnChar(std::string sTextUTF8)
	{
		auto app = CGlobals::GetApp();
		if (app->GetAppState() != PEAppState_Ready)
			return;

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
		auto app = CGlobals::GetApp();
		// static int s_last_width = 0;
		// static int s_last_height = 0;
		if (app->IsTouchInputting())
		{
			// if (s_last_width != w && s_last_height != h)
			// {
			// 	std::cout << "last_width: " << s_last_width << " last_height: " << s_last_height << " width: " << w << " height: " << h << std::endl;
			// 	s_last_width = w;
			// 	s_last_height = h;
			// }
			// else if (s_last_width == w && s_last_height != h)
			// {
			// 	// 触屏方式忽略垂直高度变化(输入法影响)
			// 	return;
			// }

			if (((CSDL2Application *)(CGlobals::GetApp()))->IsInputing())
			{
				return;
			}
		}

		std::cout << "OnSize" << " w: " << w << " h: " << h << std::endl;
		if (w > 0 && h > 0)
		{
			app->SetResolution((float)w, (float)h);
			app->UpdateScreenMode();
		}
	}
} // end namespace
