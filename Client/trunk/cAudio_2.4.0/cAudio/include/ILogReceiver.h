// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

namespace cAudio
{
	//! Enum of all supported log levels in cAudio.
	enum LogLevel
	{
		ELL_DEBUG,
		ELL_INFO,
		ELL_WARNING,
		ELL_ERROR,
		ELL_CRITICAL,
		ELL_COUNT
	};

	//! Contains strings for each log level to make them easier to print to a stream.
	const char* const LogLevelStrings[] =
	{
		"Debug",
		"Information",
		"Warning",
		"Error",
		"Critical",
		0
	};

	//! Interface for receiving log messages and relaying them to some kind of output device or stream.
	class ILogReceiver
	{
	public:
		ILogReceiver() { }
		~ILogReceiver() { }

		//! Called on every logged message that is greater than or equal to the minimum log level.
		/**
		\param sender: The class/component sending the message.
		\param message: The log message itself.
		\param level: Log level of the message. 
		\param time: Time in seconds that the message was sent.  This time has millisecond accuracy. */
		virtual bool OnLogMessage(const char* sender, const char* message, LogLevel level, float time) = 0;
	};
};

