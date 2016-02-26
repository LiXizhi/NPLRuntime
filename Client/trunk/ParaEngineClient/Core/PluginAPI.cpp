//-----------------------------------------------------------------------------
// Class:	PluginAPI
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.8.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineCore.h"
#include "PluginAPI.h"

using namespace ParaEngine;


#define VERSION_PARAENGINE		0x1
namespace ParaEngine
{
	DWORD GetParaEngineVersion()
	{
		return VERSION_PARAENGINE;
	}

	IParaEngineCore* GetCOREInterface()
	{
		return CParaEngineCore::GetStaticInterface();
	}
}
