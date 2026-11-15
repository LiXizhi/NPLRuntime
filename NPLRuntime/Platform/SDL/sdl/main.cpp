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
		if (argv[i] && argv[i][0] != '\0')
		{
			std::string token = argv[i];
			// Preserve spaces across POSIX argv splitting for key=value or standalone paths
			if (token.find(' ') != std::string::npos && token.find('"') == std::string::npos)
			{
				auto eqPos = token.find('=');
				if (eqPos != std::string::npos && token.find("://") == std::string::npos)
					token = token.substr(0, eqPos + 1) + "\"" + token.substr(eqPos + 1) + "\"";
				else
					token = "\"" + token + "\"";
			}
			if (!sCmdLine.empty())
				sCmdLine += " ";
			sCmdLine += token;
		}
	}

	// Single-instance URL protocol (paracraft://) / .p3d forwarding on Linux desktop
	CCommandLineParams cmdParams(sCmdLine.c_str());
	const char* sSingle = cmdParams.GetValue("singleinstance", nullptr);
	const char* sMulti = cmdParams.GetValue("multipleinstance", nullptr);
	bool isExplicitMulti = (sMulti && strcmp(sMulti, "true") == 0);
	bool isSingleOrProtocol = (sSingle && strcmp(sSingle, "true") == 0)
		|| sCmdLine.find("paracraft://") != std::string::npos
		|| sCmdLine.find(".p3d") != std::string::npos;
	if (!isExplicitMulti && isSingleOrProtocol)
	{
		if (CSDL2Application::TryForwardToExistingInstance(sCmdLine))
		{
			return 0;
		}
	}

	CSDL2Application app;
	if (!app.InitApp(nullptr, sCmdLine.c_str()))
		return 1;
	return app.Run();
}