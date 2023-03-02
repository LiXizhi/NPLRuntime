#include "ParaEngine.h"
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

#include <iostream>
#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

using namespace ParaEngine;

#ifdef _WIN32
CParaFileUtils* CParaFileUtils::GetInstance()
{
	static CParaFileUtils win32Impl;
	return &win32Impl;
}
#endif

#ifndef EM_PORT_API
#	if defined(__EMSCRIPTEN__)
#		include <emscripten.h>
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype EMSCRIPTEN_KEEPALIVE
#		else
#			define EM_PORT_API(rettype) rettype EMSCRIPTEN_KEEPALIVE
#		endif
#	else
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype
#		else
#			define EM_PORT_API(rettype) rettype
#		endif
#	endif
#endif

class EmscriptenApplication: public CSDL2Application
{
public:
	EmscriptenApplication()
	{
		m_inited = false;
		m_fs_inited = false;
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

public:
	std::string m_cmdline;
	bool m_fs_inited;
	bool m_inited;
};

static EmscriptenApplication* GetApp()
{
	static EmscriptenApplication s_app;
	return &s_app;
}

void mainloop(void* arg)
{
	// std::cout << "\n===============mainloop begin=======================" << std::endl;
	if (!GetApp()->m_fs_inited) return;
	if (!GetApp()->m_inited)
	{
		GetApp()->m_inited = true;
		// GetApp()->InitApp(nullptr, GetApp()->m_cmdline);
		GetApp()->InitApp(nullptr, R"(cmdline=noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true" world="worlds/DesignHouse/_user/xiaoyao/testabc")");
	}
	GetApp()->RunLoopOnce();
	// std::cout << "===============mainloop end=======================" << std::endl;
}

// 设置可写路径
EM_PORT_API(void) emscripten_filesystem_inited()
{
	CParaFile::SetWritablePath("/idbfs");
	GetApp()->m_fs_inited = true;
}

int main(int argc, char* argv[])
{
	std::cout << "===============emscripten paracraft=======================" << std::endl;
	int js_language = EM_ASM_INT(
		{
			var jsSrc =(navigator.language || navigator.browserLanguage).toLowerCase();
			if(jsSrc.indexOf('zh') >= 0){
				console.log("浏览器语言是---中文");
				return 1;
			}else if(jsSrc.indexOf('en') >= 0){
				console.log("浏览器语言是---英文");
				return 0;
			}else{
				console.log("浏览器语言是---其他语言");
				return -1;
			}
		}
	);
	
	ParaEngineSettings& settings = ParaEngineSettings::GetSingleton();
	settings.SetCurrentLanguage(js_language == 1 ? LanguageType::CHINESE : LanguageType::ENGLISH);

	std::string sCmdLine;
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

	GetApp()->m_cmdline = sCmdLine;

	EM_ASM({
	    FS.mkdir('/idbfs');
        FS.mount(IDBFS, { root: '/idbfs' }, '/idbfs');
        FS.syncfs(true, function(err) {
            console.log("加载IDBFS!!!");
            Module._emscripten_filesystem_inited();
        });
		window.onbeforeunload = function (e) { FS.syncfs(false, function() { console.log("onbeforeunload"); }); };
        window.onunload = function (e) { FS.syncfs(false, function() { console.log("onbeforeunload"); }); };
		setTimeout(function(){ FS.syncfs(false, function(err) { if (err) { console.log("FS.syncfs",err); } });}, 120000);  // 3分钟后同步到idbfs
		setTimeout(function(){ FS.syncfs(false, function(err) { if (err) { console.log("FS.syncfs",err); } });}, 600000);  // 10分钟后同步到idbfs
    });

	std::cout << "========================start paracraft=======================" << std::endl;
	// GetApp()->InitApp(nullptr, R"(cmdline=noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true" world="worlds/DesignHouse/_user/xiaoyao/testabc")");
	// GetApp()->InitApp(nullptr, R"(cmdline=noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true")");
	#ifdef EMSCRIPTEN
		emscripten_set_main_loop_arg(mainloop, nullptr, -1, 1);
	#endif
	std::cout << "========================stop paracraft=======================" << std::endl;
	return 0;
}


