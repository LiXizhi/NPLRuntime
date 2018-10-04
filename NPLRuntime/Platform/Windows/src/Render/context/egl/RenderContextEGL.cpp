#include "ParaEngine.h"

#include <stdexcept>
#include <cassert>
#include <windowsx.h>
#include <windows.h>
#include <tchar.h>


#include "OpenGL.h"

#include "RenderWindowWin32.h"
#include "RenderContextEGL.h"
#include "RenderDeviceEGL.h"

using namespace ParaEngine;



LRESULT CALLBACK TempWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}


ParaEngine::IRenderContext* ParaEngine::IRenderContext::Create()
{

	return new RenderContextEGL();
}

ParaEngine::IRenderDevice* ParaEngine::RenderContextEGL::CreateDevice(const RenderConfiguration& cfg)
{

	RenderWindowWin32* pWindow = static_cast<RenderWindowWin32*>(cfg.renderWindow);
	HWND hWnd = pWindow->GetHandle();
	HDC hdc = GetDC(hWnd);
	EGLDisplay eglDisplay = eglGetDisplay(hdc);

	const EGLint attribs[] = { EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES2_BIT, //Request opengl ES2.0
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_NONE };
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	if (!numConfigs)
	{
		//Fall back to 16bit depth buffer
		const EGLint attribs_d16[] = { EGL_RENDERABLE_TYPE,
			EGL_OPENGL_ES2_BIT, //Request opengl ES2.0
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_NONE };
		eglChooseConfig(display, attribs_d16, &config, 1, &numConfigs);
	}


	if (!numConfigs)
	{
		printf("Unable to retrieve EGL config");
		return nullptr;
	}

	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);


	surface = eglCreateWindowSurface(display, config, hWnd, NULL);
	const EGLint context_attrib_list[] = {
		// request a context using Open GL ES 2.0
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	context = eglCreateContext(display, config, NULL, context_attrib_list);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		printf("Unable to eglMakeCurrent");
		return nullptr;
	}

	if (!loadGL())
	{
		printf("Unable to load gl ext.");
	}

	auto version = glGetString(GL_VERSION);
	printf("GL_VERSION:%s", version);

	m_surface = surface;
	m_context = context;
	m_display = display;

	return new RenderDeviceEGL(m_display, m_surface);
}

bool RenderContextEGL::ResetDevice(IRenderDevice* device, const RenderConfiguration & cfg)
{

	return false;
}
