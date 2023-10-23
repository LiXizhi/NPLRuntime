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
#include <iostream>
#ifdef EMSCRIPTEN
#include "emscripten.h"
#include "Js.h"
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
	std::string sCmdLine = R"(noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true")";
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
		emscripten_set_main_loop_arg(mainloop, nullptr, -1, 1);
	#endif
	std::cout << "========================stop paracraft=======================" << std::endl;
	return 0;
}

