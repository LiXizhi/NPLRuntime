// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include <time.h>
#include <stdio.h>
#include "cLogger.h"
#include "cConsoleLogReceiver.h"
#include "cFileLogReceiver.h"
#include "cUtils.h"

namespace cAudio
{
	cLogger::cLogger() : StartTime(0), MinLogLevel(ELL_INFO)
	{
		StartTime = clock();
	}

	void cLogger::logCritical( const char* sender, const char *msg, ... )
	{
		if(ELL_CRITICAL >= MinLogLevel)
		{
			Mutex.lock();
			va_list args;
			va_start( args, msg );
			broadcastMessage( ELL_CRITICAL, sender, msg, args );
			va_end( args );
			Mutex.unlock();
		}
	}
	void cLogger::logError( const char* sender, const char *msg, ... )
	{
		if(ELL_ERROR >= MinLogLevel)
		{
			Mutex.lock();
			va_list args;
			va_start( args, msg );
			broadcastMessage( ELL_ERROR, sender, msg, args );
			va_end( args );
			Mutex.unlock();
		}
	}
	void cLogger::logWarning( const char* sender, const char *msg, ... )
	{
		if(ELL_WARNING >= MinLogLevel)
		{
			Mutex.lock();
			va_list args;
			va_start( args, msg );
			broadcastMessage( ELL_WARNING, sender, msg, args );
			va_end( args );
			Mutex.unlock();
		}
	}
	void cLogger::logInfo( const char* sender, const char *msg, ... )
	{
		if(ELL_INFO >= MinLogLevel)
		{
			Mutex.lock();
			va_list args;
			va_start( args, msg );
			broadcastMessage( ELL_INFO, sender, msg, args );
			va_end( args );
			Mutex.unlock();
		}
	}
	void cLogger::logDebug( const char* sender, const char *msg, ... )
	{
		if(ELL_DEBUG >= MinLogLevel)
		{
			Mutex.lock();
			va_list args;
			va_start( args, msg );
			broadcastMessage( ELL_DEBUG, sender, msg, args );
			va_end( args );
			Mutex.unlock();
		}
	}
	void cLogger::setLogLevel( const LogLevel& logLevel )
	{
		Mutex.lock();
		MinLogLevel = logLevel;
		Mutex.unlock();
	}
	void cLogger::broadcastMessage( LogLevel level, const char* sender, const char* msg, va_list args )
	{
		float messageTime = (clock() - StartTime) / (float)CLOCKS_PER_SEC;
		vsnprintf( TempTextBuf, 2048, msg, args );

		ReceiversIterator it = Receivers.begin();
        for (it = Receivers.begin(); it != Receivers.end(); it++)
        {
            it->second->OnLogMessage(sender, TempTextBuf, level, messageTime);
        }
	}
	bool cLogger::registerLogReceiver(ILogReceiver* receiver, const char* name)
    {
		Mutex.lock();
		cAudioString logName = fromUTF8(name);
        Receivers[logName] = receiver;
		Mutex.unlock();
		return true;
    }

	void cLogger::unRegisterLogReceiver(const char* name)
	{
		Mutex.lock();
		cAudioString logName = fromUTF8(name);
		ReceiversIterator it = Receivers.find(logName);
		if(it != Receivers.end())
		{
			Receivers.erase(it);
		}
		Mutex.unlock();
	}

	bool cLogger::isLogReceiverRegistered(const char* name)
	{
		Mutex.lock();
		cAudioString logName = fromUTF8(name);
		ReceiversIterator it = Receivers.find(logName);
		bool result = (it != Receivers.end());
		Mutex.unlock();
		return result;
	}

	ILogReceiver* cLogger::getLogReceiver(const char* name)
	{
		Mutex.lock();
		cAudioString logName = fromUTF8(name);
		ReceiversIterator it = Receivers.find(logName);
		if(it != Receivers.end())
		{
			Mutex.unlock();
			return it->second;
		}
		Mutex.unlock();
		return NULL;
	}
};
