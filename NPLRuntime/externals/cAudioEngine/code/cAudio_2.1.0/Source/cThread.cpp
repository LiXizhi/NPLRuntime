// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "../Headers/cThread.h"

#ifdef CAUDIO_PLATFORM_WIN
#include <windows.h>	//Basic windows includes
#include <process.h>
#else
#include <pthread.h>	//Assumed linux system
#endif	

namespace cAudio
{
	#ifdef CAUDIO_PLATFORM_WIN
	int cAudioThread::SpawnThread( unsigned __stdcall start_address( void* ), void *arg)
	{
		HANDLE threadHandle;
		unsigned threadID = 0;

		threadHandle = (HANDLE) _beginthreadex(NULL,0,start_address,arg,0,&threadID);

		int state = (threadHandle==0) ? 1 : 0;

		if(state == 0)
			CloseHandle( threadHandle );

		return state;
	}
	#else
	int cAudioThread::SpawnThread( void* start_address( void* ), void *arg)
	{
		pthread_t threadHandle;

		pthread_attr_t attr;
		pthread_attr_init( &attr );
		pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );

		return pthread_create( &threadHandle, &attr, start_address, arg );
	}
	#endif
};