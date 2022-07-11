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

#ifdef _WIN32
CParaFileUtils* CParaFileUtils::GetInstance()
{
	static CParaFileUtils win32Impl;
	return &win32Impl;
}
#endif

int main(int argc, char* argv[])
{
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_Init(SDL_INIT_EVERYTHING);
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

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

	RenderWindowDelegate renderWindow;
	CSDL2Application app;
	app.InitApp(&renderWindow, sCmdLine.c_str());
	app.Run();

	SDL_Quit();
	return 0;
}
