// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include <stdarg.h>
#include "cMutex.h"
#include "ILogger.h"
#include "cSTLAllocator.h"
#include "cAudioString.h"

namespace cAudio
{
	class cLogger : public ILogger
    {
    public:
		cLogger();
		virtual ~cLogger() { }

		virtual void logCritical( const char* sender, const char *msg, ... );
		virtual void logError( const char* sender, const char *msg, ... );
		virtual void logWarning( const char* sender, const char *msg, ... );
		virtual void logInfo( const char* sender, const char *msg, ... );
		virtual void logDebug( const char* sender, const char *msg, ... );

		virtual const LogLevel& getLogLevel() const { return MinLogLevel; }
		virtual void setLogLevel( const LogLevel& logLevel );

		virtual bool registerLogReceiver(ILogReceiver* receiver, const char* name);
		virtual void unRegisterLogReceiver(const char* name);
		virtual bool isLogReceiverRegistered(const char* name);
		virtual ILogReceiver* getLogReceiver(const char* name);

	protected:
		void broadcastMessage( LogLevel level, const char* sender, const char* msg, va_list args );

		cAudioMutex Mutex;
		unsigned long StartTime;
		char TempTextBuf[2048];
		LogLevel MinLogLevel;
		cAudioMap<cAudioString, ILogReceiver*>::Type Receivers;
		typedef cAudioMap<cAudioString, ILogReceiver*>::Type::iterator ReceiversIterator;
	private:
    };
};
