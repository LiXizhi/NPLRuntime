#pragma once
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
namespace ParaEngine
{
    class AppDelegate 
    {
    public:
        AppDelegate(struct android_app* app);
        ~AppDelegate();
        void Run();
    private:
        struct android_app* m_State;
        EGLDisplay m_Display;
        EGLSurface m_Surface;
        EGLContext m_Context;
        int32_t m_Width;
        int32_t m_Height;

        static void app_handle_command(struct android_app* app, int32_t cmd);
        static int32_t app_handle_input(struct android_app* app, AInputEvent* event);
    protected:
        // App commands
        virtual void OnStart();
        virtual void OnStop();
        virtual void OnPause();
        virtual void OnResume();
        virtual void OnDestroy();
        virtual void OnInitWindow();
        virtual void OnTermWindow();
        virtual void OnWindowResized();
		virtual void Draw();
    };
}