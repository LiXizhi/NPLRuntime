// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cMutex.h"
#include "cAudioDefines.h"
#include "IThread.h"

#ifdef CAUDIO_PLATFORM_WIN
#include <windows.h>	//Basic windows includes
#include <process.h>
#else
#include <pthread.h>	//Assumed linux system
#endif	

namespace cAudio
{
	class cAudioThread : public IThread
	{
	public:
		cAudioThread(IThreadWorker* pWorker);
		~cAudioThread();

		virtual bool start();
		virtual void join();
		virtual void shutdown();
		virtual bool isRunning();

	protected:
		void updateLoop();

#ifdef CAUDIO_PLATFORM_WIN
		static unsigned int __stdcall threadFunc(void *args);
		HANDLE ThreadHandle;
#else
		static void* threadFunc(void* args);
		pthread_t ThreadHandle;
#endif
		IThreadWorker* Worker;
		unsigned int ThreadID;
		cAudioMutex Mutex;
		bool IsInit;
		bool Loop;
	};
};