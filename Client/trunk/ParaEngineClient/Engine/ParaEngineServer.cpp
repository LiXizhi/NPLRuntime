//-----------------------------------------------------------------------------
// Class:	ParaEngineServer Main Entry file
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Dev Studio
// Date:	2009.4.19
// The main server entry point. main(). 
// Note for debugging: please see the macro comments
//-----------------------------------------------------------------------------
// ParaEngineServer.cpp : Defines the entry point for the console application.
//
#include "ParaEngine.h"
#ifdef PARAENGINE_SERVER
#include "util/ParaTime.h"
#include "NPLRuntime.h"
#include "AISimulator.h"
#include "ParaEngineAppImp.h"
#include "FrameRateController.h"
#include "ParaEngineService.h"

#include "util/keyboard.h"

int main(int argc, char **argv)
{
	printf("            ---ParaEngine Server V%d.%d---         \n", 1, 0);
	int exit_code = 0;

	bool bServiceMode = false;

#ifndef WIN32
	bServiceMode = true;
#endif
	std::string sCmdLine;
	
	for(int i=1;i<argc; ++i)
	{
		// for linux, we will check the command line to see if it contains "-d", which will run in service mode. 
		if(strcmp(argv[i], "-d") == 0)
		{
			bServiceMode = true;
		}
		else if(strcmp(argv[i], "-D") == 0)
		{
			bServiceMode = false;
		}
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
	
	ParaEngine::CParaEngineApp myServerApp(sCmdLine.c_str());
	if(bServiceMode)
	{
		// enter as system service
		ParaEngine::CParaEngineService::InitDaemon();
	}

	ParaEngine::CParaEngineService service;

	service.AcceptKeyStroke(!bServiceMode);

	exit_code = service.Run((argc>0) ? sCmdLine.c_str() : NULL, &myServerApp);

	return exit_code;
}

#endif