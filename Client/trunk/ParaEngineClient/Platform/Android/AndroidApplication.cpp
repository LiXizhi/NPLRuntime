#include "AndroidApplication.h"
#include <android/log.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <cassert>
using namespace ParaEngine;


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ParaEngine", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ParaEngine", __VA_ARGS__))


void AndroidApplication::app_handle_command(struct android_app* app, int32_t cmd)
{
    AndroidApplication* myApp = static_cast<AndroidApplication*>(app->userData);
    switch(cmd)
    {
        case APP_CMD_START:
        {
            myApp->OnStart();
        }break;
        case APP_CMD_RESUME:
        {
            myApp->OnResume();
        }break;
        case APP_CMD_PAUSE:
        {
            myApp->OnPause();
        }break;   
        case APP_CMD_STOP:
        {
            myApp->OnPause();
        }break;   
        case APP_CMD_DESTROY:
        {
            myApp->OnDestroy();
        }break;  
        case APP_CMD_INIT_WINDOW:
        {
            myApp->OnInitWindow();
        }break;  
        case APP_CMD_TERM_WINDOW:
        {
            myApp->OnTermWindow();
        }break; 
        case APP_CMD_WINDOW_RESIZED:
        {
            myApp->OnWindowResized();
        }break;  



    }
}
int32_t AndroidApplication::app_handle_input(struct android_app* app, AInputEvent* event)
{
    return 0;
}


AndroidApplication::AndroidApplication(struct android_app* app)
:m_State(app)
,m_Display(EGL_NO_DISPLAY)
,m_Surface(EGL_NO_SURFACE)
,m_Context(EGL_NO_CONTEXT)
,m_Width(0)
,m_Height(0)
{
    app->userData = this;
    app->onAppCmd =  AndroidApplication::app_handle_command;
    app->onInputEvent = AndroidApplication::app_handle_input;
}
AndroidApplication::~AndroidApplication()
{

}

void AndroidApplication::Run()
{
    LOGI("app:run");
    while(1)
    {
        int ident = 0;
        int events = 0;
        struct android_poll_source* source = nullptr;
        ident = ALooper_pollAll(0, NULL, &events,(void**)&source);
        if(ident>=0)
        {
            // Process this event.
            if (source != NULL) {
                source->process(m_State, source);
            }
            // Check if we are exiting
            if (m_State->destroyRequested != 0) {
                 LOGI("app:destroy");
                return;
            }
        }
		if (m_Context != EGL_NO_CONTEXT)
		{
			Draw();
		}

    }
    LOGI("app:exit");
}

void AndroidApplication::OnStart()
{
    LOGI("app:OnStart");
}
void AndroidApplication::OnStop()
{
    LOGI("app:OnStop");
}
void AndroidApplication::OnPause()
{
    LOGI("app:OnPause");
}
void AndroidApplication::OnResume()
{
    LOGI("app:OnResume");
}
void AndroidApplication::OnDestroy()
{
    LOGI("app:OnDestroy");
}

void AndroidApplication::OnInitWindow()
{
    LOGI("app:OnInitWindow");

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

	/* Here, the application chooses the configuration it desires. In this
	* sample, we have a very simplified selection process, where we pick
	* the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(m_State->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, m_State->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return;
	}
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	m_Context = context;
	m_Surface = surface;
	m_Display = display;
	m_Width = w;
	m_Height = h;
}
void AndroidApplication::OnTermWindow()
{
    LOGI("app:OnTermWindow");
}
void AndroidApplication::OnWindowResized()
{
    LOGI("app:OnWindowResized");
}

void ParaEngine::AndroidApplication::Draw()
{
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	eglSwapBuffers(m_Display, m_Surface);
}
