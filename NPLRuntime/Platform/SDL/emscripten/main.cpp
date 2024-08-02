#include "ParaEngine.h"
#include "NPLHelper.h"
#include "NPLRuntime.h"
#include "resource.h"

#include "ParaEngineService.h"
#include "ParaEngineCore.h"

#include "PluginAPI.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"

#include "CommandLineParams.h"
#include "Framework/FileSystem/ParaFileUtils.h"

#include "ParaEngineSettings.h"

#include "RenderWindowDelegate.h"
#include "SDL2Application.h"
#include "ParaFile.h"
#include "NPLInterface.hpp"
#include "ViewportManager.h"
#include "IParaWebXR.h"
#include <iostream>

#ifdef EMSCRIPTEN
#include "emscripten.h"
#include "Js.h"
#include "webxr.h"
#endif

using namespace ParaEngine;

#ifdef _WIN32
CParaFileUtils* CParaFileUtils::GetInstance()
{
    static CParaFileUtils win32Impl;
    return &win32Impl;
}
#endif
class EmscriptenApplication: public CSDL2Application
{
public:
    EmscriptenApplication()
    {
        m_inited = false;
        m_fs_inited = false;
        m_paused = false;
    }

    virtual void RunLoopOnce()
    {
        auto pWindow = (RenderWindowDelegate*)m_pRenderWindow;
        assert(pWindow);
        // std::cout << "===============Run begin=======================" << std::endl;
        if (!pWindow->ShouldClose())
        {
            pWindow->PollEvents();
            UpdateScreenDevice();
            this->DoWork();
        }
        // std::cout << "===============Run end=======================" << std::endl;

#ifdef EMSCRIPTEN
        if (m_exit) emscripten_cancel_main_loop();
#endif
    }

    virtual void OnChar(std::string text)
    {
        m_renderWindow.OnChar(text);
    }

    RenderWindowDelegate * GetRenderWindowDelegate()
    {
        return (RenderWindowDelegate*)&m_renderWindow;
    }

    virtual void OnClearChar(std::string text)
    {
        m_renderWindow.OnClearChar(text);
    }

    virtual void OnKeyDown(int keycode)
    {
        m_renderWindow.OnKey(m_renderWindow.SDL2VirtualKeyToParaVK(keycode), EKeyState::PRESS);
    }

    virtual void OnKeyUp(int keycode)
    {
        m_renderWindow.OnKey(m_renderWindow.SDL2VirtualKeyToParaVK(keycode), EKeyState::RELEASE);
    }
    void SetPaused(bool paused)
    {
        setRenderEnabled(!paused);
        // auto pWindow = (RenderWindowDelegate*)m_pRenderWindow;
        // pWindow->m_paused = paused;
        // if (paused)
        // {
        // 	OnPause();
        // }
        // else 
        // {
        // 	OnResume();
        // }
    }
public:
    std::string m_cmdline;
    bool m_fs_inited;
    bool m_inited;
    bool m_paused;
};

static EmscriptenApplication* GetApp()
{
    static EmscriptenApplication s_app;
    return &s_app;
}

void mainloop(void* arg)
{
    // auto begin_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    auto app = GetApp();
    if (!app->m_fs_inited) return;
    if (!app->m_inited)
    {
        app->m_inited = true;
        app->InitApp(nullptr, app->m_cmdline.c_str());
        EM_ASM({
            if (Module.HideLoading != undefined) Module.HideLoading();
        });
    }
    if (app->m_paused) return;
    app->RunLoopOnce();
    // auto end_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    // std::cout << "==========time:" << (end_time - begin_time) << std::endl;
}

// 暂停运行
EM_PORT_API(void) SetAppPaused(bool paused)
{
    std::cout << "Is Paused App Run:" << paused << std::endl;
    GetApp()->SetPaused(paused);
}

// 设置可写路径
EM_PORT_API(void) emscripten_filesystem_inited()
{
    GetApp()->m_fs_inited = true;
}

int main(int argc, char* argv[])
{
    std::cout << "========================start paracraft=======================" << std::endl;
    // std::cout << "main thread id: " << std::this_thread::get_id() << std::endl;
    JS::StaticInit();
    JS::SetTextInputCallback([](const std::string text) { GetApp()->OnChar(text);});
    JS::SetClearTextInputCallback([](const std::string text) { GetApp()->OnClearChar(text);});
    JS::SetKeyDownCallback([](int keycode) { GetApp()->OnKeyDown(keycode);});
    JS::SetKeyUpCallback([](int keycode) { GetApp()->OnKeyUp(keycode);});
    JS::SetRecvMsgFromJSCallback(std::function<void(const std::string, const std::string)>([](const std::string filename, const std::string msg_data_json){
        NPL::NPLRuntimeState_ptr pState = CGlobals::GetNPLRuntime()->GetMainRuntimeState();
        // std::cout << "JS::SetRecvMsgFromJSCallback => " << filename << " : " << msg_data_json << std::endl;
        // NPL::CNPLWriter writer;
        // writer.WriteName("msg");
        // writer.BeginTable();
        // writer.WriteName("msg");
        // writer.WriteValue(msg_data_json);
        // writer.EndTable();
        // pState->activate(filename.c_str(), writer.ToString().c_str(), (int)(writer.ToString().size()));

        NPLInterface::NPLObjectProxy data;
        data["msg"] = msg_data_json;
        std::string data_string;
        NPLInterface::NPLHelper::NPLTableToString("msg", data, data_string);
        pState->activate(filename.c_str(), data_string.c_str(), (int)data_string.length());
    }));

    int js_language = JS::GetBrowserLanguage();
    ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();
    settings.SetCurrentLanguage(js_language == JS::JS_LANGUAGE_ZH ? LanguageType::CHINESE : LanguageType::ENGLISH);

    // std::string sCmdLine = R"(noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua")";
    std::string sCmdLine = R"(noupdate="true" debug="main" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true")";
    // std::string sCmdLine = R"(noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true" channelId="tutorial" isDevMode="true")";
    sCmdLine += JS::IsTouchDevice() ? R"( IsTouchDevice="true")" : "";
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i])
        {
            if (sCmdLine.empty())
                sCmdLine = argv[i];
            else
            {
                sCmdLine += " ";
                sCmdLine += argv[i];
            }
        }
    }
    std::string username = JS::GetQueryStringArg("username");
    std::string http_env = JS::GetQueryStringArg("http_env");
    std::string token = JS::GetQueryStringArg("token");
    std::string channelId = JS::GetQueryStringArg("channelId");
    std::string world = JS::GetQueryStringArg("world");
    std::string cmdline = JS::GetQueryStringArg("cmdline");
    std::string worldcmd = JS::GetQueryStringArg("cmd");
    std::string mc = JS::GetQueryStringArg("mc");
    std::string version = JS::GetQueryStringArg("version");
    if (mc.empty()) 
    {
        sCmdLine = sCmdLine + " mc=\"true\" noclientupdate=\"true\" noupdate=\"true\"" ;
    }
    else
    {
        sCmdLine = sCmdLine + " mc=\"" + mc + "\" noclientupdate=\"false\" noupdate=\"false\"";
    }
    if (!version.empty()) sCmdLine = sCmdLine + " version=\"" + version + "\"";
    if (!username.empty()) sCmdLine = sCmdLine + " username=\"" + username + "\"";
    if (!http_env.empty()) sCmdLine = sCmdLine + " http_env=\"" + http_env + "\"";
    if (!channelId.empty()) sCmdLine = sCmdLine + " channelId=\"" + channelId + "\"";
    if (!world.empty()) sCmdLine = sCmdLine + " world=\"" + world + "\"";
    std::string pid = JS::GetQueryStringArg("pid");
    std::string worldfile = JS::GetQueryStringArg("worldfile", false);
    if (pid.empty())
    {
        if (!worldfile.empty()) sCmdLine += " paracraft://cmd/loadworld/" + worldfile;
    }
    else
    {
        sCmdLine += " paracraft://cmd/loadworld/" + pid;
    }
    if (!token.empty()) sCmdLine = sCmdLine + " paracraft://usertoken=\"" + token + "\"";
    if (!worldcmd.empty()) sCmdLine = sCmdLine + " world/cmd(" + worldcmd + ")";
    sCmdLine = sCmdLine + " " + cmdline;
    std::cout << "cmdline: " << sCmdLine << std::endl;
    GetApp()->m_cmdline = sCmdLine;

    EM_ASM({
        FS.mkdir('/apps');
        FS.mount(IDBFS, { root: '/apps' }, '/apps');
        FS.mkdir('/worlds');
        FS.mount(IDBFS, { root: '/worlds' }, '/worlds');
        FS.mkdir('/Database');
        FS.mount(IDBFS, { root: '/Database' }, '/Database');
        FS.syncfs(true, function(err) {
            console.log("加载IDBFS!!!");
            Module._emscripten_filesystem_inited();
        });
        setTimeout(function(){ FS.syncfs(false, function(err) { if (err) { console.log("FS.syncfs",err); } });}, 180000);  // 3分钟后同步到idbfs
        setInterval(function(){ FS.syncfs(false, function(err) { if (err) { console.log("FS.syncfs",err); } });}, 600000);  // 10分钟后同步到idbfs
    });

    #ifdef EMSCRIPTEN
        static bool isWebXRinited = false;
        webxr_init(
            [](void* userData, int time, ParaWebXRRigidTransform *headPose, ParaWebXRView views[2], int viewCount) {
                if (!isWebXRinited)
                {
                    isWebXRinited = true;
                    static_cast<EmscriptenApplication*>(userData)->GetRenderWindowDelegate()->SetSDLWindowSize(views[0].viewport[2] * 2, views[0].viewport[3]);
                }

                // view information.
                Vector4 leftView = Vector4(views[0].viewport[0], views[0].viewport[1], views[0].viewport[2], views[0].viewport[3]);
                Vector3 leftPosition = Vector3(views[0].viewPose.position[0], views[0].viewPose.position[1], views[0].viewPose.position[2]);
                Vector4 leftOrientation = Vector4(views[0].viewPose.orientation[0], views[0].viewPose.orientation[1], views[0].viewPose.orientation[2], views[0].viewPose.orientation[3]);

                Vector4 rightView = Vector4(views[1].viewport[0], views[1].viewport[1], views[1].viewport[2], views[1].viewport[3]);
                Vector3 rightPosition = Vector3(views[1].viewPose.position[0], views[1].viewPose.position[1], views[1].viewPose.position[2]);
                Vector4 rightOrientation = Vector4(views[1].viewPose.orientation[0], views[1].viewPose.orientation[1], views[1].viewPose.orientation[2], views[1].viewPose.orientation[3]);

                // hand information.
                // static ParaWebXRRigidTransform _controllerTransformations[2];
                ParaWebXRInputSource sources[2];
                int sourcesCount = 0;
                webxr_get_input_sources(sources, 5, &sourcesCount);

                Vector3 leftHandPosition;
                Vector4 leftHandOrientation;
                Vector3 rightHandPosition;
                Vector4 rightHandOrientation;

                int leftHandKey[7];
                int rightHandKey[7];
                Vector4 leftAxes = Vector4(0, 0, 0, 0);
                Vector4 rightAxes = Vector4(0, 0, 0, 0);

                for (int i = 0; i < sourcesCount; ++i) {
                    static ParaWebXRRigidTransform *_controllerTransformations;
                    webxr_get_input_pose(&sources[i], _controllerTransformations, WEBXR_INPUT_POSE_TARGET_RAY);

                    if (sources[i].handedness == WEBXR_HANDEDNESS_LEFT) {
                        leftHandPosition = Vector3(_controllerTransformations->position[0], _controllerTransformations->position[1], _controllerTransformations->position[2]);
                        leftHandOrientation = Vector4(
                            _controllerTransformations->orientation[0],
                            _controllerTransformations->orientation[1],
                            _controllerTransformations->orientation[2],
                            _controllerTransformations->orientation[3]);

                        memcpy(leftHandKey, sources[i].gamepad.buttons, sizeof(sources[i].gamepad.buttons));

                        leftAxes = Vector4(sources[i].gamepad.axes[0], sources[i].gamepad.axes[1], sources[i].gamepad.axes[2], sources[i].gamepad.axes[3]);
                    } else if (sources[i].handedness == WEBXR_HANDEDNESS_RIGHT) {
                        rightHandPosition = Vector3(_controllerTransformations->position[0], _controllerTransformations->position[1], _controllerTransformations->position[2]);
                        rightHandOrientation = Vector4(
                            _controllerTransformations->orientation[0],
                            _controllerTransformations->orientation[1],
                            _controllerTransformations->orientation[2],
                            _controllerTransformations->orientation[3]);

                        memcpy(rightHandKey, sources[i].gamepad.buttons, sizeof(sources[i].gamepad.buttons));
                        
                        rightAxes = Vector4(sources[i].gamepad.axes[0], sources[i].gamepad.axes[1], sources[i].gamepad.axes[2], sources[i].gamepad.axes[3]);
                    }
                }

                ((IParaWebXR *)CGlobals::GetViewportManager()->GetChildAttributeObject("WebXR"))->UpdateWebXRView(
                    time,
                    leftView, leftPosition, leftOrientation,
                    rightView, rightPosition, rightOrientation,
                    viewCount,
                    leftHandPosition, rightHandPosition, leftHandOrientation, rightHandOrientation,
                    leftHandKey, rightHandKey, leftAxes, rightAxes);

                static_cast<EmscriptenApplication*>(userData)->RunLoopOnce();
            },
            [](void* userData, int mode) {
                std::cout << "webxr_session_callback_func start" << std::endl;
            },
            [](void* userData, int mode) {
                std::cout << "webxr_session_callback_func end" << std::endl;
                // Rebind frame buffer 0.
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                ((IParaWebXR *)CGlobals::GetViewportManager()->GetChildAttributeObject("WebXR"))->SetIsXR(false);
                CGlobals::GetViewportManager()->SetLayout(VIEW_LAYOUT_DEFAULT);

                isWebXRinited = false;
                static_cast<EmscriptenApplication*>(userData)->
                    GetRenderWindowDelegate()->
                    SetSDLWindowSize(
                        EM_ASM_INT({ return document.documentElement.clientWidth * window.devicePixelRatio; }),
                        EM_ASM_INT({ return document.documentElement.clientHeight * window.devicePixelRatio; })
                    );
            },
            [](void* userData, int error) {
                std::cout << "webxr_error_callback_func" << std::endl;
            },
            GetApp());
        emscripten_set_main_loop_arg(mainloop, nullptr, -1, 1);
    #endif
    std::cout << "========================stop paracraft=======================" << std::endl;
    return 0;
}
