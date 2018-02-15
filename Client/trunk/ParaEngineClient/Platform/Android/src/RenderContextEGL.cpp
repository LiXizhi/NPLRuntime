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
	RenderWindowAndroid* renderWindow = static_cast<RenderWindowAndroid*>(cfg.renderWindow);
	ANativeWindow* nativeWindow = renderWindow->GetNativeWindow();
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);
	surface = eglCreateWindowSurface(display, config, nativeWindow, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return nullptr;
	}

	if (!loadGL())
	{
		LOGW("Unable to load gl ext.");
	}

	return new RenderDeviceEGL(display,surface);
}

bool RenderContextEGL::ResetDevice(IRenderDevice* device, const RenderConfiguration & cfg)
{
	return false;
}
