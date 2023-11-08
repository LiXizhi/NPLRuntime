#include "ParaEngine.h"
#include "RenderWindowSDL2.h"
#include "2dengine/GUIRoot.h"
#include <unordered_map>

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

namespace ParaEngine
{
	static std::unordered_map<unsigned long, EVirtualKey> s_keymap;

	static void InitVirtualKeyMap()
	{
		if (s_keymap.size() == 0)
		{
			s_keymap[SDLK_UNKNOWN] = EVirtualKey::KEY_UNKNOWN;
			s_keymap[SDLK_SPACE] = EVirtualKey::KEY_SPACE;
			s_keymap[SDLK_QUOTEDBL] = EVirtualKey::KEY_APOSTROPHE;
			s_keymap[SDLK_COMMA] = EVirtualKey::KEY_COMMA;
			s_keymap[SDLK_PERIOD] = EVirtualKey::KEY_PERIOD;
			s_keymap[SDLK_SLASH] = EVirtualKey::KEY_SLASH;
			s_keymap[SDLK_0] = EVirtualKey::KEY_0;
			s_keymap[SDLK_1] = EVirtualKey::KEY_1;
			s_keymap[SDLK_2] = EVirtualKey::KEY_2;
			s_keymap[SDLK_3] = EVirtualKey::KEY_3;
			s_keymap[SDLK_4] = EVirtualKey::KEY_4;
			s_keymap[SDLK_5] = EVirtualKey::KEY_5;
			s_keymap[SDLK_6] = EVirtualKey::KEY_6;
			s_keymap[SDLK_7] = EVirtualKey::KEY_7;
			s_keymap[SDLK_8] = EVirtualKey::KEY_8;
			s_keymap[SDLK_9] = EVirtualKey::KEY_9;
			s_keymap[SDLK_SEMICOLON] = EVirtualKey::KEY_SEMICOLON;
			s_keymap[SDLK_PLUS] = EVirtualKey::KEY_EQUALS;
			s_keymap[SDLK_a] = EVirtualKey::KEY_A;
			s_keymap[SDLK_b] = EVirtualKey::KEY_B;
			s_keymap[SDLK_c] = EVirtualKey::KEY_C;
			s_keymap[SDLK_d] = EVirtualKey::KEY_D;
			s_keymap[SDLK_e] = EVirtualKey::KEY_E;
			s_keymap[SDLK_f] = EVirtualKey::KEY_F;
			s_keymap[SDLK_g] = EVirtualKey::KEY_G;
			s_keymap[SDLK_h] = EVirtualKey::KEY_H;
			s_keymap[SDLK_i] = EVirtualKey::KEY_I;
			s_keymap[SDLK_j] = EVirtualKey::KEY_J;
			s_keymap[SDLK_k] = EVirtualKey::KEY_K;
			s_keymap[SDLK_l] = EVirtualKey::KEY_L;
			s_keymap[SDLK_m] = EVirtualKey::KEY_M;
			s_keymap[SDLK_n] = EVirtualKey::KEY_N;
			s_keymap[SDLK_o] = EVirtualKey::KEY_O;
			s_keymap[SDLK_p] = EVirtualKey::KEY_P;
			s_keymap[SDLK_q] = EVirtualKey::KEY_Q;
			s_keymap[SDLK_r] = EVirtualKey::KEY_R;
			s_keymap[SDLK_s] = EVirtualKey::KEY_S;
			s_keymap[SDLK_t] = EVirtualKey::KEY_T;
			s_keymap[SDLK_u] = EVirtualKey::KEY_U;
			s_keymap[SDLK_v] = EVirtualKey::KEY_V;
			s_keymap[SDLK_w] = EVirtualKey::KEY_W;
			s_keymap[SDLK_x] = EVirtualKey::KEY_X;
			s_keymap[SDLK_y] = EVirtualKey::KEY_Y;
			s_keymap[SDLK_z] = EVirtualKey::KEY_Z;
			s_keymap[SDLK_LEFTBRACKET] = EVirtualKey::KEY_LBRACKET;
			s_keymap[SDLK_BACKSLASH] = EVirtualKey::KEY_BACKSLASH;
			s_keymap[SDLK_RIGHTBRACKET] = EVirtualKey::KEY_RBRACKET;
			s_keymap[SDLK_BACKQUOTE] = EVirtualKey::KEY_GRAVE;

			/// Function Keys
			s_keymap[SDLK_ESCAPE] = EVirtualKey::KEY_ESCAPE;
			s_keymap[SDLK_RETURN] = EVirtualKey::KEY_RETURN;
			s_keymap[SDLK_TAB] = EVirtualKey::KEY_TAB;
			s_keymap[SDLK_BACKSPACE] = EVirtualKey::KEY_BACK;
			s_keymap[SDLK_INSERT] = EVirtualKey::KEY_INSERT;
			s_keymap[SDLK_DELETE] = EVirtualKey::KEY_DELETE;
			s_keymap[SDLK_RIGHT] = EVirtualKey::KEY_RIGHT;
			s_keymap[SDLK_LEFT] = EVirtualKey::KEY_LEFT;
			s_keymap[SDLK_DOWN] = EVirtualKey::KEY_DOWN;
			s_keymap[SDLK_UP] = EVirtualKey::KEY_UP;
			s_keymap[SDLK_PRIOR] = EVirtualKey::KEY_PERIOD;
			s_keymap[SDLK_PAGEDOWN] = EVirtualKey::KEY_NEXT;
			s_keymap[SDLK_HOME] = EVirtualKey::KEY_HOME;
			s_keymap[SDLK_END] = EVirtualKey::KEY_END;
			s_keymap[SDLK_CAPSLOCK] = EVirtualKey::KEY_CAPITAL;
			s_keymap[SDLK_SCROLLLOCK] = EVirtualKey::KEY_SCROLL;
			s_keymap[SDLK_NUMLOCKCLEAR] = EVirtualKey::KEY_NUMLOCK;
			s_keymap[SDLK_PAUSE] = EVirtualKey::KEY_PAUSE;
			s_keymap[SDLK_F1] = EVirtualKey::KEY_F1;
			s_keymap[SDLK_F2] = EVirtualKey::KEY_F2;
			s_keymap[SDLK_F3] = EVirtualKey::KEY_F3;
			s_keymap[SDLK_F4] = EVirtualKey::KEY_F4;
			s_keymap[SDLK_F5] = EVirtualKey::KEY_F5;
			s_keymap[SDLK_F6] = EVirtualKey::KEY_F6;
			s_keymap[SDLK_F7] = EVirtualKey::KEY_F7;
			s_keymap[SDLK_F8] = EVirtualKey::KEY_F8;
			s_keymap[SDLK_F9] = EVirtualKey::KEY_F9;
			s_keymap[SDLK_F10] = EVirtualKey::KEY_F10;
			s_keymap[SDLK_F11] = EVirtualKey::KEY_F11;
			s_keymap[SDLK_F12] = EVirtualKey::KEY_F12;

			s_keymap[SDLK_KP_0] = EVirtualKey::KEY_NUMPAD0;
			s_keymap[SDLK_KP_1] = EVirtualKey::KEY_NUMPAD1;
			s_keymap[SDLK_KP_2] = EVirtualKey::KEY_NUMPAD2;
			s_keymap[SDLK_KP_3] = EVirtualKey::KEY_NUMPAD3;
			s_keymap[SDLK_KP_4] = EVirtualKey::KEY_NUMPAD4;
			s_keymap[SDLK_KP_5] = EVirtualKey::KEY_NUMPAD5;
			s_keymap[SDLK_KP_6] = EVirtualKey::KEY_NUMPAD6;
			s_keymap[SDLK_KP_7] = EVirtualKey::KEY_NUMPAD7;
			s_keymap[SDLK_KP_8] = EVirtualKey::KEY_NUMPAD8;
			s_keymap[SDLK_KP_9] = EVirtualKey::KEY_NUMPAD9;

			s_keymap[SDLK_KP_DECIMAL] = EVirtualKey::KEY_DECIMAL;
			s_keymap[SDLK_KP_DIVIDE] = EVirtualKey::KEY_DIVIDE;
			s_keymap[SDLK_KP_MULTIPLY] = EVirtualKey::KEY_MULTIPLY;
			s_keymap[SDLK_KP_MINUS] = EVirtualKey::KEY_SUBTRACT;
			s_keymap[SDLK_MINUS] = EVirtualKey::KEY_MINUS;
			s_keymap[SDLK_KP_PLUS] = EVirtualKey::KEY_ADD;
			s_keymap[SDLK_SEPARATOR] = EVirtualKey::KEY_RETURN;
			s_keymap[SDLK_LSHIFT] = EVirtualKey::KEY_LSHIFT;
			s_keymap[SDLK_LCTRL] = EVirtualKey::KEY_LCONTROL;
			s_keymap[SDLK_RSHIFT] = EVirtualKey::KEY_RSHIFT;
			s_keymap[SDLK_RCTRL] = EVirtualKey::KEY_RCONTROL;
			s_keymap[SDLK_LALT] = EVirtualKey::KEY_LMENU;
			s_keymap[SDLK_RALT] = EVirtualKey::KEY_RMENU;
			s_keymap[SDLK_MENU] = EVirtualKey::KEY_ALT;
		}
	}

	EVirtualKey RenderWindowSDL2::SDL2VirtualKeyToParaVK(unsigned long vk)
	{
		InitVirtualKeyMap();
		if (!m_isNumLockEnabled)
		{
			if (vk == SDLK_KP_4)
				return EVirtualKey::KEY_LEFT;
			else if (vk == SDLK_KP_6)
				return EVirtualKey::KEY_RIGHT;
			else if (vk == SDLK_KP_8)
				return EVirtualKey::KEY_UP;
			else if (vk == SDLK_KP_2)
				return EVirtualKey::KEY_DOWN;
			else if (vk == SDLK_KP_7)
				return EVirtualKey::KEY_HOME;
			else if (vk == SDLK_KP_1)
				return EVirtualKey::KEY_END;
			else if (vk == SDLK_KP_9)
				return EVirtualKey::KEY_PRIOR;
			else if (vk == SDLK_KP_3)
				return EVirtualKey::KEY_NEXT;
		}
		auto result = s_keymap.find(vk);
		if (result != s_keymap.end())
		{
			return result->second;
		}
		return EVirtualKey::KEY_UNKNOWN;
	}

	unsigned long RenderWindowSDL2::ParaVKToSDL2VirtualKey(EVirtualKey key)
	{
		InitVirtualKeyMap();
		for (auto kv : s_keymap)
		{
			if (kv.second == key)
			{
				return kv.first;
			}
		}
		// assert(false);
		return -1;
	}

	void RenderWindowSDL2::SetSize(int w, int h)
	{
		SDL_SetWindowSize(m_sdl2_window, w, h);
		m_Width = w;
		m_Height = h;
	}

	bool RenderWindowSDL2::Create(int defaultWdith, int defaultHeight)
	{
		m_sdl2_window = SDL_CreateWindow("Paracraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, defaultWdith, defaultHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		m_gl_context = SDL_GL_CreateContext(m_sdl2_window);
#ifndef EMSCRIPTEN
		gladLoadGLLoader(GLADloadproc)SDL_GL_GetProcAddress);
#endif
		SDL_GL_SetSwapInterval(1);
		std::cout << "GL_VERSION=" << glGetString(GL_VERSION) << std::endl;
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		g_WindowMap[m_sdl2_window] = this;

		m_Width = defaultWdith;
		m_Height = defaultHeight;

		const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

		return true;
	}

	std::unordered_map<SDL_Window *, RenderWindowSDL2 *> RenderWindowSDL2::g_WindowMap;
	RenderWindowSDL2::RenderWindowSDL2()
		: m_sdl2_window(nullptr), m_gl_context(nullptr), m_Width(0), m_Height(0), m_mouse_x(0), m_mouse_y(0), m_IsQuit(false), m_bLostFocus(false)
	{
		InitInput();
		m_paused = false;
		m_isNumLockEnabled = 2;
	}

	RenderWindowSDL2::~RenderWindowSDL2()
	{
		if (g_WindowMap.find(m_sdl2_window) != g_WindowMap.end())
		{
			g_WindowMap.erase(m_sdl2_window);
		}

		m_sdl2_window = nullptr;
	}

	void RenderWindowSDL2::CheckFocus()
	{
		if (m_bLostFocus)
		{
			SDL_SetWindowInputFocus(m_sdl2_window);
		}
	}

	bool RenderWindowSDL2::ShouldClose() const
	{
		return m_IsQuit;
	}

	void RenderWindowSDL2::PollEvents()
	{
		SDL_Event sdl_event;
		while (SDL_PollEvent(&sdl_event))
		{
			if (sdl_event.type == SDL_QUIT)
			{
				m_IsQuit = true;
				return;
			}
			else if (m_paused)
			{
				continue;
			}
			else if (sdl_event.type == SDL_WINDOWEVENT)
			{
				switch (sdl_event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
#ifdef EMSCRIPTEN
					OnSize(EM_ASM_INT({ return document.documentElement.clientWidth; }), EM_ASM_INT({ return document.documentElement.clientHeight; }));
#else
					OnSize(sdl_event.window.data1, sdl_event.window.data2);
#endif
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					m_bLostFocus = true;
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					m_bLostFocus = false;
					break;
				default:
					break;
				}
			}
			else if (sdl_event.type == SDL_MOUSEMOTION)
			{
				m_mouse_x = sdl_event.motion.x;
				m_mouse_y = sdl_event.motion.y;
				OnMouseMove(sdl_event.motion.x, sdl_event.motion.y);
			}
			else if (sdl_event.type == SDL_MOUSEBUTTONDOWN)
			{
				// SetTouchInputting(false);
				m_mouse_x = sdl_event.button.x;
				m_mouse_y = sdl_event.button.y;
				if (sdl_event.button.button == SDL_BUTTON_LEFT)
				{
					m_MouseState[(uint32_t)EMouseButton::LEFT] = EKeyState::PRESS;
					OnMouseButton(EMouseButton::LEFT, EKeyState::PRESS, sdl_event.button.x, sdl_event.button.y);
				}
				else if (sdl_event.button.button == SDL_BUTTON_RIGHT)
				{
					m_MouseState[(uint32_t)EMouseButton::RIGHT] = EKeyState::PRESS;
					OnMouseButton(EMouseButton::RIGHT, EKeyState::PRESS, sdl_event.button.x, sdl_event.button.y);
				}
				else if (sdl_event.button.button == SDL_BUTTON_MIDDLE)
				{
					m_MouseState[(uint32_t)EMouseButton::MIDDLE] = EKeyState::PRESS;
					OnMouseButton(EMouseButton::MIDDLE, EKeyState::PRESS, sdl_event.button.x, sdl_event.button.y);
				}
				CheckFocus();
			}
			else if (sdl_event.type == SDL_MOUSEBUTTONUP)
			{
				m_mouse_x = sdl_event.button.x;
				m_mouse_y = sdl_event.button.y;
				if (sdl_event.button.button == SDL_BUTTON_LEFT)
				{
					m_MouseState[(uint32_t)EMouseButton::LEFT] = EKeyState::RELEASE;
					OnMouseButton(EMouseButton::LEFT, EKeyState::RELEASE, sdl_event.button.x, sdl_event.button.y);
				}
				else if (sdl_event.button.button == SDL_BUTTON_RIGHT)
				{
					m_MouseState[(uint32_t)EMouseButton::RIGHT] = EKeyState::RELEASE;
					OnMouseButton(EMouseButton::RIGHT, EKeyState::RELEASE, sdl_event.button.x, sdl_event.button.y);
				}
				else if (sdl_event.button.button == SDL_BUTTON_MIDDLE)
				{
					m_MouseState[(uint32_t)EMouseButton::MIDDLE] = EKeyState::RELEASE;
					OnMouseButton(EMouseButton::MIDDLE, EKeyState::RELEASE, sdl_event.button.x, sdl_event.button.y);
				}
				CheckFocus();
			}
			else if (sdl_event.type == SDL_MOUSEWHEEL)
			{
				OnMouseWheel(m_mouse_x, m_mouse_y, sdl_event.wheel.preciseY);
			}
			else if (sdl_event.type == SDL_KEYDOWN)
			{
				EKeyState state = EKeyState::PRESS;
				DWORD msgKey = (DWORD)sdl_event.key.keysym.sym;
				auto key = SDL2VirtualKeyToParaVK(msgKey);
				if (key != EVirtualKey::KEY_UNKNOWN)
					m_KeyState[(uint32_t)key] = state;
				OnKey(key, state);
			}
			else if (sdl_event.type == SDL_KEYUP)
			{
				if (sdl_event.key.keysym.sym == SDLK_NUMLOCKCLEAR || m_isNumLockEnabled == 2)
				{
					m_isNumLockEnabled = EM_ASM_INT({ return window.isNumLockEnabled; });
				}
				EKeyState state = EKeyState::RELEASE;
				DWORD msgKey = (DWORD)sdl_event.key.keysym.sym;
				auto key = SDL2VirtualKeyToParaVK(msgKey);
				if (key != EVirtualKey::KEY_UNKNOWN)
					m_KeyState[(uint32_t)key] = state;
				OnKey(key, state);
			}
			else if (sdl_event.type == SDL_TEXTINPUT)
			{
				OnChar(sdl_event.text.text);
			}
			else if (sdl_event.type == SDL_FINGERDOWN)
			{
				// SetTouchInputting(true);
				OnTouch(EH_TOUCH, TouchEvent::TouchEvent_POINTER_DOWN, sdl_event.tfinger.fingerId, sdl_event.tfinger.x * m_Width, sdl_event.tfinger.y * m_Height, sdl_event.tfinger.timestamp);
				// std::cout << "Finger Down: id=" << sdl_event.tfinger.fingerId << ", x=" << sdl_event.tfinger.x << ", y=" << sdl_event.tfinger.y << ", timestamp=" << sdl_event.tfinger.timestamp << std::endl;
			}
			else if (sdl_event.type == SDL_FINGERMOTION)
			{
				OnTouch(EH_TOUCH, TouchEvent::TouchEvent_POINTER_UPDATE, sdl_event.tfinger.fingerId, sdl_event.tfinger.x * m_Width, sdl_event.tfinger.y * m_Height, sdl_event.tfinger.timestamp);
				// std::cout << "Finger Motion: id=" << sdl_event.tfinger.fingerId << ", x=" << sdl_event.tfinger.x << ", y=" << sdl_event.tfinger.y << ", timestamp=" << sdl_event.tfinger.timestamp << std::endl;
			}
			else if (sdl_event.type == SDL_FINGERUP)
			{
				OnTouch(EH_TOUCH, TouchEvent::TouchEvent_POINTER_UP, sdl_event.tfinger.fingerId, sdl_event.tfinger.x * m_Width, sdl_event.tfinger.y * m_Height, sdl_event.tfinger.timestamp);
				// std::cout << "Finger Up: id=" << sdl_event.tfinger.fingerId << ", x=" << sdl_event.tfinger.x << ", y=" << sdl_event.tfinger.y << ", timestamp=" << sdl_event.tfinger.timestamp << std::endl;
			}
		}
	}

	intptr_t RenderWindowSDL2::GetNativeHandle() const
	{
		return (intptr_t)m_sdl2_window;
	}

	unsigned int RenderWindowSDL2::GetWidth() const
	{
		return m_Width;
	}

	unsigned int RenderWindowSDL2::GetHeight() const
	{
		return m_Height;
	}

	bool RenderWindowSDL2::IsWindowed() const
	{
		return m_Windowed;
	}

	EKeyState RenderWindowSDL2::GetMouseButtonState(EMouseButton button)
	{
		assert(button != EMouseButton::COUNT);
		return m_MouseState[(unsigned int)button];
	}

	EKeyState RenderWindowSDL2::GetKeyState(EVirtualKey key)
	{
		assert(key != EVirtualKey::COUNT && key != EVirtualKey::KEY_UNKNOWN);
		return m_KeyState[(uint32_t)key];
	}

	CVector2 RenderWindowSDL2::GetMousePos()
	{
		return m_MousePos;
	}

	void RenderWindowSDL2::ProcessInput()
	{
	}

	void RenderWindowSDL2::InitInput()
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
