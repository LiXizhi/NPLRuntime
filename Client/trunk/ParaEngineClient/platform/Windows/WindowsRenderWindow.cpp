#include "ParaEngine.h"
#include "WindowsRenderWindow.h"
#include "resource.h"
#include "Winuser.h"

using namespace ParaEngine;

std::unordered_map<HWND,WindowsRenderWindow*> WindowsRenderWindow::g_WindowMap;
const WCHAR* WindowsRenderWindow::ClassName = L"ParaWorld";

LRESULT WindowsRenderWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{


	if (g_WindowMap.find(hWnd) == g_WindowMap.end())
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	WindowsRenderWindow* window = g_WindowMap[hWnd];
	assert(window);
	assert(window->GetHandle() == hWnd);

	if (message == WM_DESTROY)
	{
		// close the application entirely
		PostQuitMessage(0);
		window->m_IsQuit = true;
	}

	if (window->m_MessageCallBack)
	{
		return window->m_MessageCallBack(window, message, wParam, lParam);
	}

	// Handle any messages the switch statement didn't
	return  DefWindowProcW(hWnd, message, wParam, lParam);
} 


WindowsRenderWindow::WindowsRenderWindow(HINSTANCE hInstance,int width, int height,bool windowed)
	: m_hWnd(NULL)
	, m_hAccel(NULL)
	, m_Width(width)
	, m_Height(height)
	, m_Windowed(windowed)
	, m_IsQuit(false)
{

	WNDCLASSW wndClass = { 0, WindowsRenderWindow::WindowProc, 0, 0, hInstance,
		NULL,
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH),
		NULL,
		L"ParaWorld"
	};

	RegisterClassW(&wndClass);

	// Set the window's initial style
	DWORD dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME |
		WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
	// Create the render window
	RECT rect;
	rect.left = 0;
	rect.right = width;
	rect.top = 0;
	rect.bottom = height;
	
	if (!AdjustWindowRect(&rect, dwWindowStyle, false))
	{
		OUTPUT_LOG("AdjustWindowRect failed.");
		return;
	}

	m_hWnd = CreateWindowW(WindowsRenderWindow::ClassName, L"ParaEngine Window", dwWindowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left ,rect.bottom - rect.top, 0,
		NULL, hInstance, 0);

	g_WindowMap[m_hWnd] = this;



	// Load keyboard accelerators
	m_hAccel = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDR_MAIN_ACCEL));

	

}

WindowsRenderWindow::~WindowsRenderWindow()
{
	if (g_WindowMap.find(m_hWnd)!=g_WindowMap.end())
	{
		g_WindowMap.erase(m_hWnd);
	}

	m_hWnd = NULL;
}



bool WindowsRenderWindow::ShouldClose() const
{
	return m_IsQuit;
}

void WindowsRenderWindow::PollEvents()
{
	MSG  msg;
	if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (TranslateAcceleratorW(m_hWnd, m_hAccel, &msg) != 0) return;
		//if (CGlobals::GetApp()->MsgProcWinThreadCustom(msg.message, msg.wParam, msg.lParam) != 0) return;
		// translate keystroke messages into the right format
		TranslateMessage(&msg);

		// send the message to the WindowProc function
		DispatchMessageW(&msg);	
	}
}

HWND WindowsRenderWindow::GetHandle() const
{
	return m_hWnd;
}

unsigned int ParaEngine::WindowsRenderWindow::GetWidth() const
{
	return m_Width;
}

unsigned int ParaEngine::WindowsRenderWindow::GetHeight() const
{
	return m_Height;
}

bool ParaEngine::WindowsRenderWindow::IsWindowed() const
{
	return m_Windowed;
}

void ParaEngine::WindowsRenderWindow::SetMessageCallBack(std::function<LRESULT(WindowsRenderWindow*, UINT, WPARAM, LPARAM)> callback)
{
	m_MessageCallBack = callback;
}
