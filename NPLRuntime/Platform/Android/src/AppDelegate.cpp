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
			std::vector<TouchEventPtr> touchEvents(touchCount);
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


AppDelegate::AppDelegate(struct android_app* app)
:m_State(app)
,m_RenderWindow(nullptr)
,m_RenderContext(nullptr)
,m_RenderDevice(nullptr)
,m_ParaEngineApp(nullptr)
{
    app->userData = this;
    app->onAppCmd =  AppDelegate::app_handle_command;
    app->onInputEvent = AppDelegate::app_handle_input;
}
AppDelegate::~AppDelegate()
{

}

void AppDelegate::Run()
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

		if (m_ParaEngineApp)
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
}
void AppDelegate::OnResume()
{
    LOGI("app:OnResume");
}
void AppDelegate::OnDestroy()
{
    LOGI("app:OnDestroy");
}

void AppDelegate::OnInitWindow()
{
    LOGI("app:OnInitWindow");
	auto assetManager = m_State->activity->assetManager;
	auto asset = AAssetManager_open(assetManager, "config.txt", AASSET_MODE_STREAMING);
	auto assetLength = AAsset_getLength(asset);
	char* buf = new char[assetLength];
	AAsset_read(asset, buf, assetLength);
	AAsset_close(asset);


	JNIEnv* jni;
	m_State->activity->vm->AttachCurrentThread(&jni, NULL);

	jclass activityClass = jni->GetObjectClass(m_State->activity->clazz);
	jmethodID getFilesDir = jni->GetMethodID(activityClass, "getFilesDir", "()Ljava/io/File;");
	jobject fileObject = jni->CallObjectMethod(m_State->activity->clazz, getFilesDir);
	jclass fileClass = jni->GetObjectClass(fileObject);
	jmethodID getAbsolutePath = jni->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
	jobject pathObject = jni->CallObjectMethod(fileObject, getAbsolutePath);
	auto path = jni->GetStringUTFChars((jstring)pathObject, NULL);

	jni->DeleteLocalRef(pathObject);
	jni->DeleteLocalRef(fileClass);
	jni->DeleteLocalRef(fileObject);
	jni->DeleteLocalRef(activityClass);

	m_State->activity->vm->DetachCurrentThread();


	m_RenderWindow = new RenderWindowAndroid(m_State->window);
	m_ParaEngineApp = new CParaEngineAppAndroid(m_State);
	m_ParaEngineApp->InitApp(m_RenderWindow, "");
	
}
void AppDelegate::OnTermWindow()
{
    LOGI("app:OnTermWindow");
}
void AppDelegate::OnWindowResized()
{
    LOGI("app:OnWindowResized");
}

void ParaEngine::AppDelegate::OnTouchBegan(const std::vector<TouchEventPtr>& events)
{
	if (m_ParaEngineApp)
	{
		auto gui = CGUIRoot::GetInstance();
		if (gui)
		{
			for (int i =0;i<events.size();i++)
			{
				TouchEvent event = *(events[i].get());
				gui->handleTouchEvent(event);
			}		
		}
	}
}

void ParaEngine::AppDelegate::OnTouchMoved(const std::vector<TouchEventPtr>& events)
{
	if (m_ParaEngineApp)
	{
		auto gui = CGUIRoot::GetInstance();
		if (gui)
		{
			for (int i = 0; i < events.size(); i++)
			{
				TouchEvent event = *(events[i].get());
				gui->handleTouchEvent(event);
			}
		}
	}
}

void ParaEngine::AppDelegate::OnTouchEnded(const std::vector<TouchEventPtr>& events)
{
	if (m_ParaEngineApp)
	{
		auto gui = CGUIRoot::GetInstance();
		if (gui)
		{
			for (int i = 0; i < events.size(); i++)
			{
				TouchEvent event = *(events[i].get());
				gui->handleTouchEvent(event);
			}
		}
	}
}

void ParaEngine::AppDelegate::OnTouchCancelled(const std::vector<TouchEventPtr>& events)
{
	if (m_ParaEngineApp)
	{
		auto gui = CGUIRoot::GetInstance();
		if (gui)
		{
			for (int i = 0; i < events.size(); i++)
			{
				TouchEvent event = *(events[i].get());
				gui->handleTouchEvent(event);
			}
		}
	}
}

