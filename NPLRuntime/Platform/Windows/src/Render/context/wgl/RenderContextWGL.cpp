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

namespace
{
	LRESULT CALLBACK TempWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
}

namespace ParaEngine
{
	RenderContextWGL::~RenderContextWGL()
	{
		if (mWGLRenderingContext)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(mWGLRenderingContext);
			mWGLRenderingContext = nullptr;
		}
		if (m_hDC && m_hWnd)
		{
			ReleaseDC(m_hWnd, m_hDC);
			m_hDC = nullptr;
		}
	}

	IRenderContext* CreateRenderContextWGL()
	{
		HINSTANCE hInstance = GetModuleHandleW(NULL);

		WNDCLASSEXW wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEXW));
		wc.cbSize = sizeof(WNDCLASSEXW);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.lpszClassName = L"ParaEngineGLTemp";
		wc.lpfnWndProc = TempWindowProc;
		RegisterClassExW(&wc);

		HWND hWndTemp = CreateWindowExW(
			0,
			L"ParaEngineGLTemp",
			L"ParaEngineGL-TempWindow",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			64,
			64,
			NULL,
			NULL,
			hInstance,
			NULL);
		if (!hWndTemp)
		{
			return nullptr;
		}

		HDC deviceContext = GetDC(hWndTemp);
		if (!deviceContext)
		{
			DestroyWindow(hWndTemp);
			return nullptr;
		}

		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int tempPf = ChoosePixelFormat(deviceContext, &pfd);
		if (tempPf == 0)
			tempPf = 1;

		if (!SetPixelFormat(deviceContext, tempPf, &pfd))
		{
			ReleaseDC(hWndTemp, deviceContext);
			DestroyWindow(hWndTemp);
			return nullptr;
		}

		HGLRC tempRenderingContext = wglCreateContext(deviceContext);
		if (!tempRenderingContext)
		{
			ReleaseDC(hWndTemp, deviceContext);
			DestroyWindow(hWndTemp);
			return nullptr;
		}

		if (!wglMakeCurrent(deviceContext, tempRenderingContext))
		{
			wglDeleteContext(tempRenderingContext);
			ReleaseDC(hWndTemp, deviceContext);
			DestroyWindow(hWndTemp);
			return nullptr;
		}

		const int wglLoaded = gladLoadWGL(deviceContext);

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(tempRenderingContext);
		ReleaseDC(hWndTemp, deviceContext);
		DestroyWindow(hWndTemp);

		if (!wglLoaded)
		{
			return nullptr;
		}

		return new RenderContextWGL();
	}

#ifndef PARA_RHI_MULTI_BACKEND
	IRenderContext* IRenderContext::Create()
	{
		return CreateRenderContextWGL();
	}
#endif

	IRenderDevice* RenderContextWGL::CreateDevice(const RenderConfiguration& cfg)
	{
		if (!cfg.renderWindow)
			return nullptr;

		RenderWindowWin32* pWindow = static_cast<RenderWindowWin32*>(cfg.renderWindow);
		HWND hWnd = pWindow->GetHandle();
		if (!hWnd)
			return nullptr;

		HDC deviceContext = GetDC(hWnd);
		if (!deviceContext)
			return nullptr;

		PIXELFORMATDESCRIPTOR pixelFormatDesc;
		ZeroMemory(&pixelFormatDesc, sizeof(PIXELFORMATDESCRIPTOR));
		pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pixelFormatDesc.nVersion = 1;
		pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
		pixelFormatDesc.cColorBits = 32;
		pixelFormatDesc.cDepthBits = 24;
		pixelFormatDesc.cStencilBits = 8;
		pixelFormatDesc.iLayerType = PFD_MAIN_PLANE;

		int chosenFormat = 0;
		if (GLAD_WGL_ARB_pixel_format && wglChoosePixelFormatARB != nullptr)
		{
			const int attrib[] = {
				WGL_SUPPORT_OPENGL_ARB, TRUE,
				WGL_DRAW_TO_WINDOW_ARB, TRUE,
				WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
				WGL_COLOR_BITS_ARB, 24,
				WGL_ALPHA_BITS_ARB, 8,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
				WGL_DOUBLE_BUFFER_ARB, TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				0
			};
			int pixelFormat[1] = { 0 };
			UINT formatCount = 0;
			if (wglChoosePixelFormatARB(deviceContext, attrib, NULL, 1, pixelFormat, &formatCount) && formatCount > 0)
			{
				chosenFormat = pixelFormat[0];
			}
		}

		if (chosenFormat == 0)
		{
			chosenFormat = ChoosePixelFormat(deviceContext, &pixelFormatDesc);
		}

		if (chosenFormat == 0 || !SetPixelFormat(deviceContext, chosenFormat, &pixelFormatDesc))
		{
			ReleaseDC(hWnd, deviceContext);
			return nullptr;
		}

		HGLRC renderingContext = nullptr;
		if (GLAD_WGL_ARB_create_context && wglCreateContextAttribsARB != nullptr)
		{
			// Request OpenGL 3.3 Compatibility Profile first so legacy GLSL 1.10/1.20 shaders
			// AND modern MRT/Instancing/FBO Blit/OcclusionQuery/Float RTs all work together.
			const int versions[][2] = { { 3, 3 }, { 3, 0 }, { 2, 1 }, { 2, 0 } };
			for (const auto& ver : versions)
			{
				if (ver[0] >= 3 && GLAD_WGL_ARB_create_context_profile)
				{
					const int attribsCompat[] = {
						WGL_CONTEXT_MAJOR_VERSION_ARB, ver[0],
						WGL_CONTEXT_MINOR_VERSION_ARB, ver[1],
						WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
						0
					};
					renderingContext = wglCreateContextAttribsARB(deviceContext, 0, attribsCompat);
				}
				else
				{
					const int attribsBasic[] = {
						WGL_CONTEXT_MAJOR_VERSION_ARB, ver[0],
						WGL_CONTEXT_MINOR_VERSION_ARB, ver[1],
						0
					};
					renderingContext = wglCreateContextAttribsARB(deviceContext, 0, attribsBasic);
				}
				if (renderingContext != nullptr)
					break;
			}
		}

		if (!renderingContext)
		{
			renderingContext = wglCreateContext(deviceContext);
		}

		if (!renderingContext)
		{
			ReleaseDC(hWnd, deviceContext);
			return nullptr;
		}

		if (!wglMakeCurrent(deviceContext, renderingContext))
		{
			wglDeleteContext(renderingContext);
			ReleaseDC(hWnd, deviceContext);
			return nullptr;
		}

		if (!gladLoadGL())
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(renderingContext);
			ReleaseDC(hWnd, deviceContext);
			return nullptr;
		}

		m_hWnd = hWnd;
		m_hDC = deviceContext;
		mWGLRenderingContext = renderingContext;

		auto* pDevice = new RenderDeviceOpenWGL(deviceContext);
		pDevice->InitCaps();
		return pDevice;
	}

	bool RenderContextWGL::ResetDevice(IRenderDevice* /*device*/, const RenderConfiguration& /*cfg*/)
	{
		return true;
	}
}
