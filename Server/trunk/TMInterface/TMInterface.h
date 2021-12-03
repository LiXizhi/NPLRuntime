#pragma once
#ifdef WIN32
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#endif
#include <map>
#include <string>
#include <vector>
#include <list>
#include <stdio.h>
#include <stdarg.h> 
#include <algorithm> 
#include <math.h>
#include <limits>
#include <stack>
#include <iostream>
#include <boost/thread/mutex.hpp>

#include "PluginAPI.h"
#include "INPLRuntimeState.h"
#include "NPLInterface.hpp"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"

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

#define SERVICE_LOG(logger, level, message) APP_LOG(message)
#define SERVICE_LOG1(logger, message, ...) OUTPUT_LOG(message, ## __VA_ARGS__);

using namespace std;