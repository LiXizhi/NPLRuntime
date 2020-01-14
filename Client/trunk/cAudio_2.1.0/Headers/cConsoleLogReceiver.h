// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CCONSOLELOGRECEIVER_H_INCLUDED
#define CCONSOLELOGRECEIVER_H_INCLUDED

#include "../include/ILogReceiver.h"
#include "../include/cAudioDefines.h"

#ifdef CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER
namespace cAudio
{

    class cConsoleLogReceiver : public ILogReceiver
    {
        public:
			virtual bool OnLogMessage(const char* sender, const char* message, LogLevel level, float time);
    };

};
#endif

#endif //! CCONSOLELOGRECEIVER_H_INCLUDED

