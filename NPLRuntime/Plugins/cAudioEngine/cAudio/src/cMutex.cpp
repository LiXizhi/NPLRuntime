// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cMutex.h"

namespace cAudio
{
#if CAUDIO_MAKE_THREAD_SAFE == 1
	cAudioMutex::cAudioMutex() : Initialized(false)
	{
	}

	cAudioMutex::~cAudioMutex()
	{
		if(!Initialized)
			return;
		#ifdef CAUDIO_PLATFORM_WIN
		DeleteCriticalSection(&criticalSection);
		#else
		pthread_mutex_destroy(&Mutex);
		#endif
	}

	void cAudioMutex::lock(void)
	{
		if(!Initialized)
			initialize();

	#ifdef CAUDIO_PLATFORM_WIN
		EnterCriticalSection(&criticalSection);
	#else
		int error = pthread_mutex_lock(&Mutex);
	#endif
	}

	void cAudioMutex::unlock(void)
	{
		if(!Initialized)
			return;
	#ifdef CAUDIO_PLATFORM_WIN
		LeaveCriticalSection(&criticalSection);
	#else
		int error = pthread_mutex_unlock(&Mutex);
	#endif
	}

	void cAudioMutex::initialize(void)
	{
	#ifdef CAUDIO_PLATFORM_WIN
		InitializeCriticalSection(&criticalSection);
	#else
		pthread_mutexattr_t attr;
		int error = pthread_mutexattr_init(&attr);
		error = pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
		error = pthread_mutex_init(&Mutex, &attr);
		error = pthread_mutexattr_destroy(&attr);
	#endif
		Initialized=true;
	}
#endif

};