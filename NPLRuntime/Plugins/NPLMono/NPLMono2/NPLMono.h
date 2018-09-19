#pragma once
#include <string>
#include <vector>
#include <list>
#include <map>
using namespace std;

namespace ParaEngine
{
	class IParaEngineCore;
}

extern ParaEngine::IParaEngineCore* GetCoreInterface();


#ifdef APP_LOG
#undef APP_LOG
#endif
#define APP_LOG(message) GetCoreInterface()->GetAppInterface()->AppLog(message);

#ifdef OUTPUT_LOG
#undef OUTPUT_LOG
#endif
#define OUTPUT_LOG GetCoreInterface()->GetAppInterface()->WriteToLog