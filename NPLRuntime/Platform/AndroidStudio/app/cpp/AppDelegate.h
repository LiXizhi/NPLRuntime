//-----------------------------------------------------------------------------
// AppDelegate.h
// Authors: YuanQuanwei, LanZhihong, LiXizhi, big
// CreateDate: 2018.3.29
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#pragma once

#include <EGL/egl.h>
#include "RenderDeviceOpenGL.h"
#include "EventClasses.h"
#include "Framework/InputSystem/VirtualKey.h"
#include "jni/ParaEngineActivity.h"

namespace ParaEngine {
    class CParaEngineAppAndroid;
    class IRenderDevice;
    class IRenderContext;
    class IRenderWindow;

    class AppDelegate {
    public:
        static AppDelegate &getInstance();

        void onCmdLine(const std::string &cmd);
        void init(int w, int h, const std::string &intentData);
        void handle_mouse_presses_begin(int keyType, int id, float x, float y);
        void handle_mouse_presses_end(int keyType, int id, float x, float y);
        void handle_mouse_move(int id[], float x[], float y[], size_t size);
        void handle_mouse_scroll(int forward);
        void handle_touches_begin(int id, float x, float y);
        void handle_touches_end(int id, float x, float y);
        void handle_touches_move(int id[], float x[], float y[], size_t size);
        void handle_touches_cancel(int id[], float x[], float y[], size_t size);
        bool handle_key_input(int code, bool bPressed);
        void handle_mainloop();
    private:
        AppDelegate();
        ~AppDelegate();
    public:
        // App commands
        virtual void OnStart();
        virtual void OnStop();
        virtual void OnPause();
        virtual void OnResume();
        virtual void OnDestroy();
        virtual void OnInitWindow();
        virtual void OnTermWindow();
        virtual void OnWindowResized();
    protected:
        virtual void OnTouch(const std::vector<TouchEventPtr> &events);
        virtual void OnKey(const EVirtualKey &key, const EKeyState &state);
    protected:
        CParaEngineAppAndroid *m_ParaEngineApp;
        bool m_isPaused;
        bool m_curMouseKey = -1;
        float m_fRefreshTimerInterval; // in seconds.
    };
}
