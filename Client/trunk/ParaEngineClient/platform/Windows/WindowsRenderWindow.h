#pragma once
#include <string>
#include <windows.h>
#include <windowsx.h>

namespace ParaEngine
{
	class WindowsRenderWindow
	{
	public:
		~WindowsRenderWindow() = default;
		WindowsRenderWindow(HINSTANCE hInstance,int width, int height, std::string title, std::string className, bool windowed);
	public:
		bool ShouldClose() const;
		void PollEvents();
		HWND GetHandle() const;
		int GetWidth() const;
		int GetHeight() const;
		bool IsWindowed() const;
	
	private:
		HWND m_hWnd;
		HACCEL m_hAccel;
		int m_Width;
		int m_Height;
		bool m_Windowed;
		static bool m_IsQuit;
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
}