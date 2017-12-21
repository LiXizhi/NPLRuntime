#pragma once
#include <string>
#include <unordered_map>
#include <functional>

#include <windows.h>
#include <windowsx.h>



namespace ParaEngine
{

	class WindowsRenderWindow
	{
	public:
		~WindowsRenderWindow();
		WindowsRenderWindow(HINSTANCE hInstance,int width, int height, std::string title, std::string className, bool windowed);
	public:
		bool ShouldClose() const;
		void PollEvents();
		HWND GetHandle() const;
		int GetWidth() const;
		int GetHeight() const;
		bool IsWindowed() const;
		void SetMessageCallBack(std::function<LRESULT(WindowsRenderWindow*, UINT, WPARAM, LPARAM)> callback);
	
	private:
		HWND m_hWnd;
		HACCEL m_hAccel;
		int m_Width;
		int m_Height;
		bool m_Windowed;
		std::function<LRESULT(WindowsRenderWindow*, UINT, WPARAM, LPARAM)> m_MessageCallBack;
		bool m_IsQuit;
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		static std::unordered_map<HWND, WindowsRenderWindow*> g_WindowMap;
	};
}