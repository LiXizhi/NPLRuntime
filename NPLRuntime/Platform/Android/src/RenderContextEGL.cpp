#include "ParaEngine.h"
#include "RenderContextEGL.h"
#include "RenderWindowAndroid.h"
#include "RenderDeviceOpenGL.h"
#include "RenderDeviceEGL.h"
#include <EGL/egl.h>
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ParaEngine", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ParaEngine", __VA_ARGS__))
using namespace ParaEngine;

ParaEngine::IRenderContext* ParaEngine::IRenderContext::Create()
{
	return new RenderContextEGL();
}

IRenderDevice* RenderContextEGL::CreateDevice(const RenderConfiguration & cfg)
{
	ReleaseContext();
	RenderWindowAndroid* renderWindow = static_cast<RenderWindowAndroid*>(cfg.renderWindow);
	ANativeWindow* nativeWindow = renderWindow->GetNativeWindow();
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
		LOGW("Unable to retrieve EGL config");
		return nullptr;
	}

	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);
	surface = eglCreateWindowSurface(display, config, nativeWindow, NULL);
	const EGLint context_attrib_list[] = {
		// request a context using Open GL ES 2.0
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	context = eglCreateContext(display, config, NULL, context_attrib_list);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return nullptr;
	}

	if (!loadGL())
	{
		LOGW("Unable to load gl ext.");
	}

	auto version = glGetString(GL_VERSION);
	LOGI("GL_VERSION:%s", version);

	m_surface = surface;
	m_context = context;
	m_display = display;

	return new RenderDeviceEGL(m_display, m_surface);
}

bool RenderContextEGL::ResetDevice(IRenderDevice* device, const RenderConfiguration & cfg)
{
	ReleaseContext();
	RenderWindowAndroid* renderWindow = static_cast<RenderWindowAndroid*>(cfg.renderWindow);
	ANativeWindow* nativeWindow = renderWindow->GetNativeWindow();
	
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
		LOGW("Unable to retrieve EGL config");
		return false;
	}


	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);
	surface = eglCreateWindowSurface(display, config, nativeWindow, NULL);
	const EGLint context_attrib_list[] = {
		// request a context using Open GL ES 2.0
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	context = eglCreateContext(display, config, NULL, context_attrib_list);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return false;
	}

	if (!loadGL())
	{
		LOGW("Unable to load gl ext.");
	}

	auto version = glGetString(GL_VERSION);
	LOGI("GL_VERSION:%s", version);


	m_surface = surface;
	m_context = context;
	m_display = display;

	RenderDeviceEGL* eglDevice = static_cast<RenderDeviceEGL*>(device);
	eglDevice->Reset(m_display, m_surface);



	return true;
}

ParaEngine::RenderContextEGL::RenderContextEGL()
	:m_display(EGL_NO_DISPLAY)
	,m_surface(EGL_NO_SURFACE)
	,m_context(EGL_NO_CONTEXT)
{

}

ParaEngine::RenderContextEGL::~RenderContextEGL()
{
	ReleaseContext();
}

void ParaEngine::RenderContextEGL::ReleaseContext()
{
	if (m_display != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (m_context != EGL_NO_CONTEXT)
		{
			eglDestroyContext(m_display, m_context);
		}
		if (m_surface != EGL_NO_SURFACE)
		{
			eglDestroySurface(m_display, m_surface);
		}
		eglTerminate(m_display);
	}

	m_display = EGL_NO_DISPLAY;
	m_context = EGL_NO_CONTEXT;
	m_surface = EGL_NO_SURFACE;
}
