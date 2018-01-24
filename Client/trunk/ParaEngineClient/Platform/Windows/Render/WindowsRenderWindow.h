#pragma once

#include "Framework/Interface/Render/IRenderWindow.h"
#include "Framework/Common/Math/Vector2.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <stdint.h>
#include <windows.h>
#include <windowsx.h>



namespace ParaEngine
{
	enum class EMouseButton
	{
		LEFT = 0,
		RIGHT,
		MIDDLE,
		END
	};

	enum class EKeyState
	{
		PRESS = 0,
		RELEASE
	};


	class WindowsRenderWindow : public IRenderWindow
	{
	public:
		virtual ~WindowsRenderWindow();
		WindowsRenderWindow(HINSTANCE hInstance,int width, int height,bool windowed);
		const static WCHAR* ClassName;
		bool ShouldClose() const;
		void PollEvents();
		HWND GetHandle() const;
		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;
		bool IsWindowed() const;
		EKeyState GetMouseButtonState(EMouseButton button);
		CVector2 GetMousePos();

	protected:
		virtual void OnMouseButton(EMouseButton button, EKeyState state) {};
		virtual void OnMouseMove(uint32_t x, uint32_t y) {};


	private:
		HWND m_hWnd;
		HACCEL m_hAccel;
		int m_Width;
		int m_Height;
		bool m_Windowed;
		bool m_IsQuit;
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static std::unordered_map<HWND, WindowsRenderWindow*> g_WindowMap;
	private:
		void InitInput();
		// Key State.
		EKeyState m_MouseState[(uint32_t)EMouseButton::END];
		CVector2 m_MousePos;

	};
}