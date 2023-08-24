#pragma once

#include "ParaEngineRenderBase.h"
#include "Framework/Common/Math/Vector2.h"
#include "Framework/InputSystem/VirtualKey.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <stdint.h>

#include "SDL2/SDL.h"
#ifndef EMSCRIPTEN
#include <glad/glad.h>
#endif

namespace ParaEngine
{
	EVirtualKey SDL2VirtualKeyToParaVK(unsigned long vk);
	unsigned long ParaVKToSDL2VirtualKey(EVirtualKey key);

	class RenderWindowSDL2 : public CParaEngineRenderBase
	{
	public:
		virtual ~RenderWindowSDL2();
		RenderWindowSDL2();
		const static WCHAR* ClassName;
		bool ShouldClose() const;
		void PollEvents();
		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;
		bool IsWindowed() const;
		EKeyState GetMouseButtonState(EMouseButton button);
		EKeyState GetKeyState(EVirtualKey key);
		CVector2 GetMousePos();

		virtual intptr_t GetNativeHandle() const override;

		bool Create(int defaultWdith, int defaultHeight);
		void SetSize(int w, int h);

	protected:
		virtual void OnMouseButton(EMouseButton button, EKeyState state, uint32_t x, uint32_t y) {};
		virtual void OnMouseMove(uint32_t x, uint32_t y) {};
		virtual void OnMouseWheel(float x, float y, float delta) {};
		virtual void OnKey(EVirtualKey key, EKeyState state) {};
		virtual void OnChar(unsigned int character) {};
		virtual void OnChar(std::string text) {};
		virtual void OnSize(int w, int h) {};
		virtual void OnDropFiles(const std::string& files) {};


	private:
		SDL_GLContext m_gl_context;
		SDL_Window* m_sdl2_window;
		int m_Width;
		int m_Height;
		int m_mouse_x;
		int m_mouse_y;
		bool m_Windowed;
		bool m_IsQuit;
		bool m_bLostFocus;
		static std::unordered_map<SDL_Window*, RenderWindowSDL2*> g_WindowMap;
	private:
		void InitInput();
		void ProcessInput();
		void CheckFocus();
		// Key State.
		EKeyState m_MouseState[(uint32_t)EMouseButton::COUNT];
		EKeyState m_KeyState[(uint32_t)EVirtualKey::COUNT];
		CVector2 m_MousePos;
	public:
		bool m_paused;
	};
}
