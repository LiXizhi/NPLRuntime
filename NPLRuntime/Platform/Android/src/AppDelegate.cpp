#include "ParaEngine.h"
#include "AppDelegate.h"
#include <android/log.h>
#include <android/asset_manager.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <cassert>
#include "ParaAppAndroid.h"
#include "RenderWindowAndroid.h"
#include "RenderDeviceEGL.h"
#include "RenderContextEGL.h"
using namespace ParaEngine;


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ParaEngine", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ParaEngine", __VA_ARGS__))



void AppDelegate::app_handle_command(struct android_app* app, int32_t cmd)
{
    AppDelegate* myApp = static_cast<AppDelegate*>(app->userData);
    switch(cmd)
    {
		case APP_CMD_SAVE_STATE:
		{
			//OUTPUT_LOG("save state");
			//saved_state state;
			//state.app = myApp;
			//app->savedState = malloc(sizeof(saved_state));
			//app->savedStateSize = sizeof(struct saved_state);
			//memcpy(app->savedState, &state, app->savedStateSize);
			LOGI("state saved");

		}break;
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
			myApp->OnStop();
			myApp->m_isQuited = true;
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
int32_t AppDelegate::app_handle_input(struct android_app* app, AInputEvent* event)
{
	AppDelegate* myApp = static_cast<AppDelegate*>(app->userData);
	int32_t eventType = AInputEvent_getType(event);
	if (eventType == AINPUT_EVENT_TYPE_MOTION)
	{
		auto eventSource = AInputEvent_getSource(event);
		if (eventSource == AINPUT_SOURCE_TOUCHSCREEN)
		{
			int action = AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
			TouchEvent::TouchEventMsgType touchType = TouchEvent::TouchEvent_POINTER_UP;
			switch(action) {
			case AMOTION_EVENT_ACTION_DOWN:
			{
				touchType = TouchEvent::TouchEvent_POINTER_DOWN;
			}
			break;
			case AMOTION_EVENT_ACTION_UP:
			{
				touchType = TouchEvent::TouchEvent_POINTER_UP;
			}
			break;
			case AMOTION_EVENT_ACTION_MOVE:
			{
				touchType = TouchEvent::TouchEvent_POINTER_UPDATE;
			}
			break;
			case AMOTION_EVENT_ACTION_CANCEL:
			{
				touchType = TouchEvent::TouchEvent_POINTER_UP;
			}
			break;
			}
			auto touchCount = AMotionEvent_getPointerCount(event);
			std::vector<TouchEventPtr> touchEvents;
			for (int i = 0; i < touchCount; i++)
			{
				int32_t touchId = AMotionEvent_getPointerId(event, i);
				float touchX = AMotionEvent_getX(event, i);
				float touchY = AMotionEvent_getY(event, i);
				int64_t eventTime = AMotionEvent_getEventTime(event);
				TouchEventPtr touchEvent = std::make_shared<TouchEvent>(EH_TOUCH, touchType, touchId,
					touchX, touchY, eventTime);
				touchEvents.push_back(touchEvent);
			}
			switch (action) {
			case AMOTION_EVENT_ACTION_DOWN:
			{
				myApp->OnTouchBegan(touchEvents);
			}
			break;
			case AMOTION_EVENT_ACTION_UP:
			{
				myApp->OnTouchEnded(touchEvents);
			}
			break;
			case AMOTION_EVENT_ACTION_MOVE:
			{
				myApp->OnTouchMoved(touchEvents);
			}
			break;
			case AMOTION_EVENT_ACTION_CANCEL:
			{
				myApp->OnTouchCancelled(touchEvents);
			}
			break;
			}

		}
	}
    return 0;
}


AppDelegate::AppDelegate()
:m_State(nullptr)
,m_ParaEngineApp(nullptr)
,m_isQuited(false)
,m_isPaused(false)
{

}
AppDelegate::~AppDelegate()
{
	OUTPUT_LOG("~AppDelegate");
}

void AppDelegate::Run(struct android_app* app)
{
	m_State = app;
	app->userData = this;
	app->onAppCmd = AppDelegate::app_handle_command;
	app->onInputEvent = AppDelegate::app_handle_input;
    LOGI("app:run");
    while(!m_State->destroyRequested)
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

		if (m_ParaEngineApp && !m_isPaused)
		{
			m_ParaEngineApp->DoWork();
		}
    }
    LOGI("app:exit");
}

void AppDelegate::OnStart()
{
    LOGI("app:OnStart");
}
void AppDelegate::OnStop()
{
    LOGI("app:OnStop");
}
void AppDelegate::OnPause()
{
    LOGI("app:OnPause");
	if (m_ParaEngineApp)
	{
		m_ParaEngineApp->OnPause();
	}
}
void AppDelegate::OnResume()
{
    LOGI("app:OnResume");
	if (m_ParaEngineApp)
	{
		m_ParaEngineApp->OnResume();
	}
}
void AppDelegate::OnDestroy()
{
    LOGI("app:OnDestroy");
}

void AppDelegate::OnInitWindow()
{
    LOGI("app:OnInitWindow");
	if (m_ParaEngineApp == nullptr)
	{
		auto renderWindow = new RenderWindowAndroid(m_State->window);
		m_ParaEngineApp = new CParaEngineAppAndroid(m_State);
		m_ParaEngineApp->InitApp(renderWindow, "");
	}
	else
	{
		LOGI("app:window is recreaded.");
		auto renderWindow = new RenderWindowAndroid(m_State->window);
		m_ParaEngineApp->OnRendererRecreated(renderWindow);
	}
}
void AppDelegate::OnTermWindow()
{
	LOGI("app:OnTermWindow");
	if (m_ParaEngineApp)
	{
		m_ParaEngineApp->OnRendererDestroyed();
	}
}
void AppDelegate::OnWindowResized()
{
    LOGI("app:OnWindowResized");
}


void inline PostTouchEvents(const std::vector<TouchEventPtr>& events)
{
		auto gui = CGUIRoot::GetInstance();
		if (gui)
		{
			for (int i = 0; i < events.size(); i++)
			{
				if (events[i])
				{
					TouchEvent event = *(events[i].get());
					gui->handleTouchEvent(event);
				}

			}
		}
}

void ParaEngine::AppDelegate::OnTouchBegan(const std::vector<TouchEventPtr>& events)
{
	if (m_ParaEngineApp)
	{
		PostTouchEvents(events);
	}
}

void ParaEngine::AppDelegate::OnTouchMoved(const std::vector<TouchEventPtr>& events)
{
	if (m_ParaEngineApp)
	{
		PostTouchEvents(events);
	}
}

void ParaEngine::AppDelegate::OnTouchEnded(const std::vector<TouchEventPtr>& events)
{
	if (m_ParaEngineApp)
	{
		PostTouchEvents(events);
	}
}

void ParaEngine::AppDelegate::OnTouchCancelled(const std::vector<TouchEventPtr>& events)
{
	if (m_ParaEngineApp)
	{
		PostTouchEvents(events);
	}
}

