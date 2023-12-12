#pragma once

#include "ParaEngineRenderBase.h"
#include "Framework/Common/Math/Vector2.h"
#include "Framework/InputSystem/VirtualKey.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <stdint.h>
#include <windows.h>
#include <windowsx.h>



namespace ParaEngine
{
	


	EVirtualKey Win32VirtualKeyToParaVK(DWORD vk);
	DWORD ParaVKToWin32VirtualKey(EVirtualKey key);

	class RenderWindowWin32 : public CParaEngineRenderBase
	{
	public:
		virtual ~RenderWindowWin32();
		RenderWindowWin32();
		const static WCHAR* ClassName;
		bool ShouldClose() const;
		void PollEvents();
		HWND GetHandle() const;
		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;
		bool IsWindowed() const;
		EKeyState GetMouseButtonState(EMouseButton button);
		EKeyState GetKeyState(EVirtualKey key);
		CVector2 GetMousePos();

		virtual intptr_t GetNativeHandle() const override;

		bool Create(HINSTANCE hInstance, int defaultWdith, int defaultHeight);
		void SetSize(int w, int h);

	protected:
		virtual void OnMouseButton(EMouseButton button, EKeyState state,uint32_t x,uint32_t y) {};
		virtual void OnMouseMove(uint32_t x, uint32_t y) {};
		virtual void OnMouseWheel(float x, float y, float delta) {};
		virtual void OnKey(EVirtualKey key, EKeyState state) {};
		virtual void OnChar(unsigned int character) {};
		virtual void OnSize(int w, int h) {};
		virtual void OnDropFiles(const std::string& files) {};

	public:
	    static const int s_screen_orientation_auto      = 0; // 保持真实窗口方向
		static const int s_screen_orientation_landscape = 1; // 横屏 渲染宽大于等于渲染高
		static const int s_screen_orientation_portrait  = 2; // 竖屏 渲染宽小于等于渲染高

		void SetScreenOrientation(int orientation) { m_screen_orientation = orientation; SetWindowSize(m_window_width, m_window_height); }
		int GetScreenOrientation() { return m_screen_orientation; }
		void SetWindowSize(int width, int height);
    	void WindowXYToRenderXY(int window_x, int window_y, int& render_x, int& render_y);
		bool IsRotateScreen() { return m_screen_rotated; }

	public:
		int m_window_width;
		int m_window_height;
		int m_screen_orientation;  // 屏幕方向
    	bool m_screen_rotated;     // 屏幕是否旋转
	private:
		HWND m_hWnd;
		HACCEL m_hAccel;
		int m_Width;
		int m_Height;
		bool m_Windowed;
		bool m_IsQuit;
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static std::unordered_map<HWND, RenderWindowWin32*> g_WindowMap;
		bool m_bLostFocus;
	private:
		void InitInput();
		void ProcessInput(const MSG& msg);
		void CheckFocus();
		// Key State.
		EKeyState m_MouseState[(uint32_t)EMouseButton::COUNT];
		EKeyState m_KeyState[(uint32_t)EVirtualKey::COUNT];
		CVector2 m_MousePos;

	};
}