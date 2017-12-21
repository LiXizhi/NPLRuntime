#include "ParaEngine.h"
#include "WindowsRenderWindow.h"
#include "resource.h"
#include "Winuser.h"

using namespace ParaEngine;

bool WindowsRenderWindow::m_IsQuit = false;


const std::wstring s2ws(const std::string& s)
{
	std::locale old_loc =
		std::locale::global(std::locale(""));
	const char* src_str = s.c_str();
	const size_t buffer_size = std::mbstowcs(NULL, src_str, 0);
	wchar_t* dst_wstr = new wchar_t[buffer_size];
	wmemset(dst_wstr, 0, buffer_size);
	std::mbstowcs(dst_wstr, src_str, buffer_size);
	std::wstring result = dst_wstr;
	delete[]dst_wstr;
	std::locale::global(old_loc);
	return result;
}


WindowsRenderWindow::WindowsRenderWindow(HINSTANCE hInstance,int width, int height, std::string title, std::string className, bool windowed)
	: m_hWnd(NULL)
	, m_hAccel(NULL)
	, m_Width(width)
	, m_Height(height)
	, m_Windowed(windowed)
{

	std::wstring wClassName = s2ws(className);

	WNDCLASSW wndClass = { 0, WindowsRenderWindow::WindowProc, 0, 0, hInstance,
		NULL,
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(WHITE_BRUSH),
		NULL,
		wClassName.c_str()
	};

	RegisterClassW(&wndClass);

	// Set the window's initial style
	DWORD dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME |
		WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;

	HMENU hMenu = NULL;
#ifdef SHOW_DEFAULT_MENU
	hMenu = LoadMenuW(hInstance, MAKEINTRESOURCEW(IDR_MENU));
#endif

	// Set the window's initial width
	RECT rc;
	SetRect(&rc, 0, 0, width, height);
	AdjustWindowRect(&rc, dwWindowStyle, (hMenu != NULL) ? true : false);

	// Create the render window
	std::wstring wTitle = s2ws(title);
	m_hWnd = CreateWindowW(wClassName.c_str(),wTitle.c_str(), dwWindowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		(rc.right - rc.left), (rc.bottom - rc.top), 0,
		hMenu, hInstance, 0);

	//
	// Dispatching window messages in this window thread. 
	//

	// Load keyboard accelerators
	m_hAccel = LoadAcceleratorsW(hInstance, MAKEINTRESOURCEW(IDR_MAIN_ACCEL));
}

bool WindowsRenderWindow::ShouldClose() const
{
	return m_IsQuit;
}

void WindowsRenderWindow::PollEvents()
{
	MSG  msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		// translate keystroke messages into the right format
		TranslateMessage(&msg);

		// send the message to the WindowProc function
		DispatchMessage(&msg);

		TranslateAcceleratorW(m_hWnd, m_hAccel, &msg);
	}
}

HWND WindowsRenderWindow::GetHandle() const
{
	return m_hWnd;
}

LRESULT WindowsRenderWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_DESTROY)
	{
		// close the application entirely
		PostQuitMessage(0);
		WindowsRenderWindow::m_IsQuit = true;
		return 0;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int ParaEngine::WindowsRenderWindow::GetWidth() const
{
	return m_Width;
}

int ParaEngine::WindowsRenderWindow::GetHeight() const
{
	return m_Height;
}

bool ParaEngine::WindowsRenderWindow::IsWindowed() const
{
	return m_Windowed;
}
