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
};

static EmscriptenApplication* GetApp()
{
	static EmscriptenApplication s_app;
	return &s_app;
}

void mainloop(void* arg)
{
	// std::cout << "\n===============mainloop begin=======================" << std::endl;
	GetApp()->RunLoopOnce();
	// std::cout << "===============mainloop end=======================" << std::endl;
}

// 设置可写路径
EM_PORT_API(void) SetWritablePath()
{
	CParaFile::SetWritablePath("/idbfs");
	if (CParaFile::MakeDirectoryFromFilePath("/idbfs/temp/cache/a/test.txt"))
	{
		std::cout << "=======================create directory success=====================" << std::endl;
	}
	else
	{
		std::cout << "=======================create directory failed=====================" << std::endl;
	}
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
        FS.mount(IDBFS, { root: '.' }, '/idbfs');
        FS.syncfs(true, function(err) {
			if (err)
			{
	            console.log("加载IDBFS失败!!!");
			}
			else
			{
	            Module._SetWritablePath();
			}
        });
    });

	// GetApp()->InitApp(nullptr, GetApp()->m_cmdline);
	std::cout << "========================start paracraft=======================" << std::endl;
	GetApp()->InitApp(nullptr, R"(cmdline=noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true" world="worlds/DesignHouse/_user/xiaoyao/testabc")");
	// GetApp()->InitApp(nullptr, R"(cmdline=noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true")");
	#ifdef EMSCRIPTEN
		emscripten_set_main_loop_arg(mainloop, nullptr, -1, 1);
	#endif

	return 0;
}
