// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "ILogReceiver.h"
#include "cAudioDefines.h"

#if CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER == 1
namespace cAudio
{

    class cConsoleLogReceiver : public ILogReceiver
    {
        public:
			virtual bool OnLogMessage(const char* sender, const char* message, LogLevel level, float time);
    };

};
#endif

