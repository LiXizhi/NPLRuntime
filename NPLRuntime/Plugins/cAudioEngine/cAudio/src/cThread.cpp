// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cThread.h"
#include "cAudioSleep.h"

namespace cAudio
{
	cAudioThread::cAudioThread(IThreadWorker* pWorker) : ThreadHandle(0), Worker(pWorker), ThreadID(0), IsInit(false), Loop(true)
	{
	}

	cAudioThread::~cAudioThread()
	{
		if(IsInit)
			shutdown();
	}

	bool cAudioThread::start()
	{
        cAudioMutexBasicLock lock(Mutex);
        
		if(IsInit) 
            return IsInit;
        
        IsInit = true;
        
#ifdef CAUDIO_PLATFORM_WIN
		ThreadHandle = reinterpret_cast<HANDLE>(_beginthreadex(0, 0, threadFunc, this, 0, &ThreadID));
		if(ThreadHandle == 0)
			CloseHandle( ThreadHandle );
#else
		pthread_create( &ThreadHandle, 0, threadFunc, this );
#endif
        IsInit = ThreadHandle != 0;
		return IsInit;
	}
	void cAudioThread::join()
	{
        cAudioMutexBasicLock lock(Mutex);
		if(IsInit)
		{
			Loop = false;
#ifdef CAUDIO_PLATFORM_WIN
			WaitForSingleObject(ThreadHandle, INFINITE);
#else
			pthread_join(ThreadHandle, NULL);
#endif
		}
	}
	void cAudioThread::shutdown()
	{
        cAudioMutexBasicLock lock(Mutex);
		if(IsInit)
		{
			IsInit = false;
#ifdef CAUDIO_PLATFORM_WIN
			_endthread();
#else
			pthread_exit(0);
#endif			
		}
	}
	void cAudioThread::updateLoop()
	{
		if(IsInit && Worker)
		{
			while (Loop)
			{
				Worker->run();
			}
		}
	}

	bool cAudioThread::isRunning()
	{
		return Loop && IsInit;
	}

#ifdef CAUDIO_PLATFORM_WIN
	unsigned int __stdcall cAudioThread::threadFunc(void *args)
	{
		cAudioThread* pThread = reinterpret_cast<cAudioThread*>(args);
		pThread->updateLoop();
		return 0;
	}
#else
	void* cAudioThread::threadFunc(void* args)
	{
		cAudioThread* pThread = reinterpret_cast<cAudioThread*>(args);
        pThread->updateLoop();
		return 0;
	}
#endif
};