// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "ILogReceiver.h"
#include "cAudioDefines.h"

namespace cAudio
{
	//! Interface for all logging operations in cAudio.
	class ILogger
    {
    public:
		ILogger() { }
		virtual ~ILogger() { }

		//! Used to log a critical error message to the logging system.
		/** 
		\param sender: Name of the class/component sending the message. 
		\param msg: The message to send. */
		virtual void logCritical( const char* sender, const char *msg, ... ) = 0;

		//! Used to log an error message to the logging system.
		/** 
		\param sender: Name of the class/component sending the message. 
		\param msg: The message to send. */
		virtual void logError( const char* sender, const char *msg, ... ) = 0;

		//! Used to log a warning to the logging system.
		/** 
		\param sender: Name of the class/component sending the message. 
		\param msg: The message to send. */
		virtual void logWarning( const char* sender, const char *msg, ... ) = 0;

		//! Used to log an informational message to the logging system.
		/** 
		\param sender: Name of the class/component sending the message. 
		\param msg: The message to send. */
		virtual void logInfo( const char* sender, const char *msg, ... ) = 0;

		//! Used to log a debug message to the logging system.
		/** 
		\param sender: Name of the class/component sending the message. 
		\param msg: The message to send. */
		virtual void logDebug( const char* sender, const char *msg, ... ) = 0;

		//! Returns the minimum log level that will be sent to the log receivers.
		virtual const LogLevel& getLogLevel() const = 0;

		//! Sets the minimum log level that the engine will send to log receivers.
		/** Primarily used to prevent too much verbose information from being sent to disk/console.
		\param logLevel: LogLevel to set as the new minimum. Anything equal to or greater than this level will be logged. */
		virtual void setLogLevel( const LogLevel& logLevel ) = 0;

		//! Register Log Receiver.
		/** Note: Any class registered will become owned by the internal thread.
		If threading is enabled, you MUST make the receiver threadsafe if you plan to access it in your application while it is registered. 
		\param receiver: Pointer to your implementation of ILogReceiver.
		\param name: Name of the log receiver. 
		\return True on success, False on failure. */
		virtual bool registerLogReceiver(ILogReceiver* receiver, const char* name) = 0;

		//! Unregister a Log Receiver.
		/** Will NOT delete any user added receiver, you must do that yourself.
		\param name: Name of the log receiver to remove. */
		virtual void unRegisterLogReceiver(const char* name) = 0;

		//! Returns whether an log receiver is currently registered.
		/**
		\param name: Name of the log receiver to check for. */
		virtual bool isLogReceiverRegistered(const char* name) = 0;

		//! Returns a registered log receiver.
		/**
		\param name: Name of the log receiver to return.
		\return Pointer to the found log receiver or NULL if it could not be found. */
		virtual ILogReceiver* getLogReceiver(const char* name) = 0;
    };
};

