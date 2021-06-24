// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"

#if CAUDIO_MAKE_THREAD_SAFE == 1
	#ifdef CAUDIO_PLATFORM_WIN
	#include <windows.h>	//Basic windows include
	#else
	#include <pthread.h>	//Assumed linux system
	#endif
#endif

namespace cAudio
{
#if CAUDIO_MAKE_THREAD_SAFE == 1
	//! Basic mutex class used for internal thread locking
	class cAudioMutex
	{
	public:
		cAudioMutex();
		~cAudioMutex();

		void lock();
		void unlock();
	private:
		void initialize();
		#ifdef CAUDIO_PLATFORM_WIN
		CRITICAL_SECTION criticalSection;
		#else
		pthread_mutex_t Mutex;
		#endif
		bool Initialized;
	};
#else
	//Dud class to disable the mutex
	class cAudioMutex
	{
	public:
		cAudioMutex() {}
		~cAudioMutex() {}
		void lock() {}
		void unlock() {}
	};
#endif

	class cAudioMutexBasicLock
	{
	public:
		cAudioMutexBasicLock(cAudioMutex& mutex) : Mutex(&mutex)
		{
			Mutex->lock();
		}
		~cAudioMutexBasicLock()
		{
			Mutex->unlock();
		}
	protected:
		cAudioMutex* Mutex;
	};
};