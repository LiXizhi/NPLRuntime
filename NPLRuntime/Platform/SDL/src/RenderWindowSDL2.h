#pragma once
#include "EventClasses.h"
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
	class RenderWindowSDL2 : public CParaEngineRenderBase
	{
	public:
		char GetKeyChar(unsigned long vk);
		EVirtualKey SDL2VirtualKeyToParaVK(unsigned long vk);
		unsigned long ParaVKToSDL2VirtualKey(EVirtualKey key);
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
		void SetWindowText(std::string title);
		
	protected:
		virtual void OnTouch(int nType, TouchEvent::TouchEventMsgType nTouchType, int touch_id, float x, float y, int nTimeTick) {};
		virtual void OnMouseButton(EMouseButton button, EKeyState state, uint32_t x, uint32_t y) {};
		virtual void OnMouseMove(uint32_t x, uint32_t y) {};
		virtual void OnMouseWheel(float x, float y, float delta) {};
		virtual void OnKey(EVirtualKey key, EKeyState state) {};
		virtual void OnChar(unsigned int character) {};
		virtual void OnChar(std::string text) {};
		virtual void OnSize(int w, int h) {};
		virtual void OnDropFiles(const std::string& files) {};
	public:
	    static const int s_screen_orientation_auto      = 0; // 保持真实窗口方向
		static const int s_screen_orientation_landscape = 1; // 横屏 渲染宽大于等于渲染高
		static const int s_screen_orientation_portrait  = 2; // 竖屏 渲染宽小于等于渲染高

		void SetScreenOrientation(int orientation) { m_screen_orientation = orientation; SetSDLWindowSize(m_sdl_window_width, m_sdl_window_height); }
		int GetScreenOrientation() { return m_screen_orientation; }
		void SetSDLWindowSize(int width, int height);
    	void WindowXYToRenderXY(int window_x, int window_y, int& render_x, int& render_y);
		bool IsRotateScreen() { return m_screen_rotated; }
	private:
		SDL_GLContext m_gl_context;
		GLuint m_scene_fbo;
    	GLuint m_scene_texture;
		SDL_Window* m_sdl2_window;
		int m_window_width;
		int m_window_height;
		int m_sdl_window_width;
		int m_sdl_window_height;
    	int m_screen_orientation;  // 屏幕方向
    	bool m_screen_rotated;     // 屏幕是否旋转

		int m_isNumLockEnabled;
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
