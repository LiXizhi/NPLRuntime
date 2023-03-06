#include "ParaEngine.h"
#include "resource.h"

#include "ParaEngineService.h"
#include "ParaEngineCore.h"

#include "PluginAPI.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"

#include "CommandLineParams.h"
#include "Framework/FileSystem/ParaFileUtils.h"

#include "RenderWindowDelegate.h"
#include "SDL2Application.h"

using namespace ParaEngine;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include<WinSock.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "OpenGL32.lib")
#endif 

#ifdef _WIN32
CParaFileUtils* CParaFileUtils::GetInstance()
{
	static CParaFileUtils win32Impl;
	return &win32Impl;
}
#endif

int main(int argc, char* argv[])
{
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

	CSDL2Application app;
	// app.InitApp(nullptr, sCmdLine.c_str());
	app.InitApp(nullptr, R"(cmdline=noupdate="true" debug="main" mc="true" bootstrapper="script/apps/Aries/main_loop.lua" noclientupdate="true" world="worlds/DesignHouse/_user/xiaoyao/testabc")");
	
	app.Run();

	return 0;
}