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
	if (!GetApp()->m_fs_inited) return;
	if (!GetApp()->m_inited)
	{
		GetApp()->m_inited = true;
		GetApp()->InitApp(nullptr, GetApp()->m_cmdline.c_str());
		// GetApp()->InitApp(nullptr, R"(cmdline=noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true" world="worlds/DesignHouse/_user/xiaoyao/testabc")");
		// GetApp()->InitApp(nullptr, R"(cmdline=noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true")");

		EM_ASM({
			if (Module.HideLoading != undefined) Module.HideLoading();
		});
	}
	GetApp()->RunLoopOnce();
}

// 设置可写路径
EM_PORT_API(void) emscripten_filesystem_inited()
{
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

	std::string sCmdLine = R"(noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true")";
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
	int pid = EM_ASM_INT({
		var url = location.search;                //获取url中"?"符后的字串
   		if (url.indexOf("?") != -1) {             //判断是否有参数
      		var str = url.substr(1);              //从第一个字符开始 因为第0个是?号 获取所有除问号的所有符串
      		strs = str.split("=");                //用等号进行分隔 （因为知道只有一个参数 所以直接用等号进分隔 如果有多个参数 要用&号分隔 再用等号进行分隔）
      		var pid = parseInt(strs[1]);          //直接弹出第一个参数 （如果有多个参数 还要进行循环的）
			if (isNaN(pid)) return 0;
			else return pid;
		}
	});

	if (pid != 0) sCmdLine += " paracraft://cmd/loadworld/" + std::to_string(pid);
	std::cout << "cmdline: " << sCmdLine << std::endl;

	GetApp()->m_cmdline = sCmdLine;

	EM_ASM({
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

	std::cout << "========================start paracraft=======================" << std::endl;
	// GetApp()->InitApp(nullptr, R"(cmdline=noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true" world="worlds/DesignHouse/_user/xiaoyao/testabc")");
	#ifdef EMSCRIPTEN
		emscripten_set_main_loop_arg(mainloop, nullptr, -1, 1);
	#endif
	std::cout << "========================stop paracraft=======================" << std::endl;
	return 0;
}
