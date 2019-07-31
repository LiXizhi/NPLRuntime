#pragma once
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include "RenderDeviceOpenGL.h"
#include "EventClasses.h"
#include "Framework/InputSystem/VirtualKey.h"
#include "jni/AppActivity.h"

#include "Core/MainLoopBase.h"

namespace ParaEngine
{
	class CParaEngineAppAndroid;
	class IRenderDevice;
	class IRenderContext;
	class IRenderWindow;
    class AppDelegate : public MainLoopBase
    {
    public:
		static AppDelegate& getInstance();
        
        void Run(android_app* app);

		void onCmdLine(const std::string& cmd);
    private:
		AppDelegate();
		~AppDelegate();

        struct android_app* m_State;
        static void app_handle_command(struct android_app* app, int32_t cmd);
        static int32_t app_handle_input(struct android_app* app, AInputEvent* event);
		static int32_t handle_key_input(AppDelegate* app, AInputEvent* event);
		static void handle_touch_input(AppDelegate* app, AInputEvent* event);
		static void handle_mouse_input(AppDelegate* app, AInputEvent* event);

		void handle_mainloop_timer(const boost::system::error_code& err);
		
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
		virtual void OnTouch(const std::vector<TouchEventPtr>& events);
		virtual void OnKey(const EVirtualKey& key, const EKeyState& state);

	protected:
		CParaEngineAppAndroid* m_ParaEngineApp;
		bool m_isPaused;


		AppActivity m_appActivity;

		float m_fRefreshTimerInterval; //  in seconds.
    };


	struct saved_state
	{
		ParaEngine::AppDelegate* app;
	};

}