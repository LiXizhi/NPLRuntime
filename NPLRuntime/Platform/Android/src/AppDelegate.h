#pragma once
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include "RenderDeviceOpenGL.h"
#include "EventClasses.h"
#include "Framework/InputSystem/VirtualKey.h"
#include "jni/AppActivity.h"

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/asio/steady_timer.hpp>

namespace ParaEngine
{
	class CParaEngineAppAndroid;
	class IRenderDevice;
	class IRenderContext;
	class IRenderWindow;
    class AppDelegate 
    {
    public:
        AppDelegate();
        ~AppDelegate();
        void Run(android_app* app);
    private:
        struct android_app* m_State;
        static void app_handle_command(struct android_app* app, int32_t cmd);
        static int32_t app_handle_input(struct android_app* app, AInputEvent* event);
		static int32_t handle_key_input(AppDelegate* app, AInputEvent* event);
		static void handle_touch_input(AppDelegate* app, AInputEvent* event);

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

		/** the main game loop */
		boost::asio::io_service m_main_io_service;
		/** the main timer that ticks 30 times a second*/
		boost::asio::steady_timer m_main_timer;

		AppActivity m_appActivity;

		float m_fRefreshTimerInterval; //  in seconds.
    };


	struct saved_state
	{
		ParaEngine::AppDelegate* app;
	};

}