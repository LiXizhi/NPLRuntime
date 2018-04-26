// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cConsoleLogReceiver.h"

#if CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER == 1
#include <iostream>

namespace cAudio
{
    bool cConsoleLogReceiver::OnLogMessage(const char* sender, const char* message, LogLevel level, float time)
	{
		//std::cout << time << " " << sender << ": [" << LogLevelStrings[level] << "] " << message << std::endl;
		std::cout << "[" << LogLevelStrings[level] << "] " << message << std::endl;
		return true;
	}
};

#endif


