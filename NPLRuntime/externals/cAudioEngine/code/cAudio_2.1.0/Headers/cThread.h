// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CAUDIOTHREAD_H
#define CAUDIOTHREAD_H

#include "../include/cAudioDefines.h"

//Helper defines in order to make sure the function is declared right for use as a thread
#ifdef CAUDIO_PLATFORM_WIN
#define CAUDIO_DECLARE_THREAD_FUNCTION(functionName) unsigned __stdcall functionName( void* arguments )
#else
#define CAUDIO_DECLARE_THREAD_FUNCTION(functionName) void* functionName( void* arguments )
#endif

namespace cAudio
{
	class cAudioThread
	{
	public:
	//	Really basic function to spawn a single detached thread
	/** \param start_address The function you want to call
	//	\param arg Any arguments to pass to the function
	//	\return 0 if successful, otherwise an error */
#ifdef CAUDIO_PLATFORM_WIN
	static int SpawnThread( unsigned __stdcall start_address( void* ), void *arg);
#else
	static int SpawnThread( void* start_address( void* ), void *arg);
#endif
	};
};

#endif //! CAUDIOTHREAD_H