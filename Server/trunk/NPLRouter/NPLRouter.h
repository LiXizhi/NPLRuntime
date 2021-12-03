#pragma once
#ifdef WIN32
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#endif

// ParaEngine includes
#include "PluginAPI.h"
#include "INPLRuntime.h"
#include "INPLRuntimeState.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"

namespace ParaEngine
{
	class IParaEngineCore;
}
extern ParaEngine::IParaEngineCore* GetCoreInterface();

#ifdef WIN32

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

#else
	#include <util/LogService.h>
#endif
#include "NPLInterface.hpp"

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

using namespace std;
