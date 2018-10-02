#include "ParaEngine.h"
#include <stdexcept>
#include <cassert>
#include <windowsx.h>
#include <windows.h>
#include <tchar.h>
#include <glad/glad_wgl.h>
#include <glad/glad.h>

#include "RenderWindowWin32.h"
#include "RenderContextWGL.h"
#include "RenderDeviceWGL.h"

using namespace ParaEngine;



LRESULT CALLBACK TempWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}


ParaEngine::IRenderContext* ParaEngine::IRenderContext::Create()
{

	HINSTANCE hInstance = GetModuleHandle(NULL);

	// 创建临时窗口
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = _T("ParaEngineGL");
	wc.lpfnWndProc = TempWindowProc;
	RegisterClassEx(&wc);
	HWND hWndTemp = CreateWindowExW(0,
		L"ParaEngineGL",      // name of the window class
		L"ParaEngineGL-TempWindow",             // title of the window
		WS_OVERLAPPEDWINDOW,              // window style
		CW_USEDEFAULT,                    // x-position of the window
		CW_USEDEFAULT,                    // y-position of the window
		400,             // width of the window
		300,            // height of the window
		NULL,                             // we have no parent window, NULL
		NULL,                             // we aren't using menus, NULL
		hInstance,                        // application handle
		NULL);                            // used with multiple windows, NULL


										  ///////////////////////
										  //// 创建临时上下文

	HDC deviceContext;
	PIXELFORMATDESCRIPTOR pixelFormat;
	int error;

	deviceContext = GetDC(hWndTemp);
	if (!deviceContext)
	{
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}
	// 设置一个临时的默认像素格式
	error = SetPixelFormat(deviceContext, 1, &pixelFormat);
	if (error != 1)
	{
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}

	// 创建一个临时渲染上下文
	HGLRC tempRenderingContext = wglCreateContext(deviceContext);
	if (!tempRenderingContext)
	{
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}

	// 将刚才创建的临时渲染上下文,设置为这个窗口的当前渲染上下文.
	error = wglMakeCurrent(deviceContext, tempRenderingContext);
	if (error != 1)
	{
		wglDeleteContext(tempRenderingContext);
		tempRenderingContext = nullptr;
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}

	///////////////////////
	// 加载WGL
	///////////////////////

	if (!gladLoadWGL(deviceContext))
	{
		wglDeleteContext(tempRenderingContext);
		tempRenderingContext = nullptr;
		DestroyWindow(hWndTemp);
		hWndTemp = nullptr;
		return nullptr;
	}
	// 销毁临时窗口
	DestroyWindow(hWndTemp);
	hWndTemp = nullptr;
	return new RenderContextWGL();
}

ParaEngine::IRenderDevice* ParaEngine::RenderContextWGL::CreateDevice(const RenderConfiguration& cfg)
{
	assert(cfg.renderWindow);
	RenderWindowWin32* pWindow = (RenderWindowWin32*)cfg.renderWindow;
	HWND hWnd = pWindow->GetHandle();
	HDC deviceContext = GetDC(hWnd);
	if (!deviceContext) {
		return nullptr;
	}
	int attrib[19];
	attrib[0] = WGL_SUPPORT_OPENGL_ARB;
	attrib[1] = TRUE;

	attrib[2] = WGL_DRAW_TO_WINDOW_ARB;
	attrib[3] = TRUE;

	attrib[4] = WGL_ACCELERATION_ARB;
	attrib[5] = WGL_FULL_ACCELERATION_ARB;

	// Support for 24bit color.
	attrib[6] = WGL_COLOR_BITS_ARB;
	attrib[7] = 24;

	// Support for 24 bit depth buffer.
	attrib[8] = WGL_DEPTH_BITS_ARB;
	attrib[9] = 24;

	// Support for double buffer.
	attrib[10] = WGL_DOUBLE_BUFFER_ARB;
	attrib[11] = TRUE;

	// Support for swapping front and back buffer.
	attrib[12] = WGL_SWAP_METHOD_ARB;
	attrib[13] = WGL_SWAP_EXCHANGE_ARB;

	// Support for the RGBA pixel type.
	attrib[14] = WGL_PIXEL_TYPE_ARB;
	attrib[15] = WGL_TYPE_RGBA_ARB;

	// Support for a 8 bit stencil buffer.
	attrib[16] = WGL_STENCIL_BITS_ARB;
	attrib[17] = 8;

	// Null terminate the attribute list.
	attrib[18] = 0;

	int pixelFormat[1];
	uint32_t formatCount = 0;
	PIXELFORMATDESCRIPTOR pixelFormatDesc;

	// Query for a pixel format that fits the attributes we want.
	int result = wglChoosePixelFormatARB(deviceContext, attrib, NULL, 1, pixelFormat, &formatCount);
	if (!result)
	{
		ReleaseDC(hWnd, deviceContext);
		return nullptr;
	}
	result = SetPixelFormat(deviceContext, pixelFormat[0], &pixelFormatDesc);
	if (!result)
	{
		ReleaseDC(hWnd, deviceContext);
		return nullptr;
	}

	// Set the 2.0 version of OpenGL in the attribute list.
	int attributeList[5];
	attributeList[0] = WGL_CONTEXT_MAJOR_VERSION_ARB;
	attributeList[1] = 2;
	attributeList[2] = WGL_CONTEXT_MINOR_VERSION_ARB;
	attributeList[3] = 0;
	// Null terminate the attribute list.
	attributeList[4] = 0;

	// Create a OpenGL 2.0 rendering context.
	HGLRC renderingContext = wglCreateContextAttribsARB(deviceContext, 0, attributeList);
	if (!renderingContext) {
		return nullptr;
	}
	result = wglMakeCurrent(deviceContext, renderingContext);
	if (!result)
	{
		ReleaseDC(hWnd, deviceContext);
		wglDeleteContext(renderingContext);
		return nullptr;
	}

	
	if (!gladLoadGL())
	{
		ReleaseDC(hWnd, deviceContext);
		wglDeleteContext(renderingContext);
		return nullptr;
	}


	mWGLRenderingContext = renderingContext;

	return new RenderDeviceOpenWGL(deviceContext);
}

bool ParaEngine::RenderContextWGL::ResetDevice(IRenderDevice* device, const RenderConfiguration& cfg)
{
	return true;
}
