#include "ParaEngine.h"
#include "RenderWindowSDL2.h"
#include "2dengine/GUIRoot.h"
#include <unordered_map>


namespace ParaEngine {
	std::unordered_map<unsigned long, EVirtualKey> s_keymap;

	static void InitVirtualKeyMap()
	{
		if (s_keymap.size() == 0)
		{
		}
	}

	static EVirtualKey SDL2VirtualKeyToParaVK(unsigned long vk)
	{

		InitVirtualKeyMap();
		auto result = s_keymap.find(vk);
		if (result != s_keymap.end())
		{
			return result->second;
		}
		return EVirtualKey::KEY_UNKNOWN;
	}

	static unsigned long ParaVKToSDL2VirtualKey(EVirtualKey key)
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

	void RenderWindowSDL2::SetSize(int w, int h)
	{
		SDL_SetWindowSize(m_sdl2_window, w, h);
		m_Width = w;
		m_Height = h;
	}

	bool RenderWindowSDL2::Create(int defaultWdith, int defaultHeight)
	{
		m_sdl2_window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, defaultWdith, defaultHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		m_gl_context = SDL_GL_CreateContext(m_sdl2_window);
		gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
		SDL_GL_SetSwapInterval(1);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		g_WindowMap[m_sdl2_window] = this;

		m_Width = defaultWdith;
		m_Height = defaultHeight;
		return true;
	}

	std::unordered_map<SDL_Window*, RenderWindowSDL2*> RenderWindowSDL2::g_WindowMap;
	RenderWindowSDL2::RenderWindowSDL2()
		: m_sdl2_window(nullptr)
		, m_gl_context(nullptr)
		, m_Width(0)
		, m_Height(0)
		, m_IsQuit(false)
		, m_bLostFocus(false)
	{
		InitInput();
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
			//::SetFocus(m_hWnd);
		}
	}

	bool RenderWindowSDL2::ShouldClose() const
	{
		return m_IsQuit;
	}

	void RenderWindowSDL2::PollEvents()
	{
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

	void RenderWindowSDL2::ProcessInput(const MSG& msg)
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
