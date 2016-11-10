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

#include "AppDelegate.h"
#include <string>

int main(int argc, char **argv)
{

	int exit_code = 0;

	bool bServiceMode = false;
	bool bInterpreterMode = false;

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
		else if(strcmp(argv[i], "-i") == 0)
		{
			bInterpreterMode = true;
		}
		else if (argv[i])
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

    AppDelegate* app = new AppDelegate();
    app->SetMacCommandLine(sCmdLine);

    app->Run();

    return 0;
}
