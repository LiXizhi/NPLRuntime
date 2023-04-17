// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c
#include <iostream>
#include <assert.h>
#include <windows.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <thread>
#include <codecvt>
#include <locale>
#include "webview.h"
// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("WebView sample");

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	// Store instance handle in our global variable
	hInst = hInstance;

	// The parameters to CreateWindow explained:
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application does not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
	HWND hWnd = CreateWindowEx(0,
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1380, 800,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 不支持下载安装WV2RT
	auto init_webview = std::function<void()>([hInstance, hWnd](){
		// 创建WebView  公用主窗口
		WebView::GetInstance()->SetWnd(hWnd);
		// 创建WebView  独立窗口
		// WebView::GetInstance()->Create(hInstance, hWnd);
	
		// begin debug
		// WebView::GetInstance()->Open(L"file:///D:/workspace/c/webview/webview.html");
		WebView::GetInstance()->Open(L"https://webparacraft.keepwork.com");
		//WebView::GetInstance()->Open(L"https://edu.palaka.cn");
		// msg callback
		// WebView::GetInstance()->OnWebMessage(std::function<void(const std::wstring&)>([](const std::wstring& msg){
		// 	std::wcout << L"OnMessage: " << msg << std::endl;
		// }));
		// end debug
	});

	if (!WebView::IsSupported())
	{
		std::thread(WebView::DownloadAndInstallWV2RT, init_webview, nullptr).detach();
	}
	else
	{
		init_webview();
	}

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// thread.join();
	return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
		RECT bounds;
		GetClientRect(hWnd, &bounds);
		WebView::GetInstance()->SetPosition(bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top, false);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}


std::string WStringToString(std::wstring wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string str = converter.to_bytes(wstr);
	return str;
}

std::wstring StringToWString(std::string str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring wstr = converter.from_bytes(str);
	return wstr;
}


void WriteLog(const char* sFormat, ...) {
	char buf_[1024 + 1];
	va_list args;
	va_start(args, sFormat);
	vsnprintf(buf_, 1024, sFormat, args);
	std::cout << buf_;
	va_end(args);
}

