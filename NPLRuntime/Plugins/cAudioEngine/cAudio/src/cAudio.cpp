// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cAudio.h"
#include "cAudioManager.h"
#include "cAudioCapture.h"

#if CAUDIO_COMPILE_WITH_OGG_DECODER == 1
#include "cOggAudioDecoderFactory.h"
#endif

#if CAUDIO_COMPILE_WITH_WAV_DECODER == 1
#include "cWavAudioDecoderFactory.h"
#endif

#include "cRawAudioDecoderFactory.h"

#if CAUDIO_COMPILE_WITH_FILE_SOURCE == 1
#include "cFileSourceFactory.h"
#include "cFileSource.h"
#endif

#include "cPluginManager.h"
#include "cLogger.h"
#include "cConsoleLogReceiver.h"
#include "cFileLogReceiver.h"
#include "cOpenALAudioDeviceList.h"

namespace cAudio
{

	//---------------------------------------------------------------------------------------
	// Audio manager section
	//---------------------------------------------------------------------------------------

#if CAUDIO_COMPILE_WITH_OGG_DECODER == 1
	static cOggAudioDecoderFactory OggDecoderFactory;
#endif
#if CAUDIO_COMPILE_WITH_WAV_DECODER == 1
	static cWavAudioDecoderFactory WavDecoderFactory;
#endif

	static cRawAudioDecoderFactory RawDecoderFactory;

#if CAUDIO_COMPILE_WITH_FILE_SOURCE == 1
	static cFileSourceFactory FileSourceFactory;
#endif

	CAUDIO_API IAudioManager* createAudioManager(bool initializeDefault)
	{
		cAudioManager* manager = CAUDIO_NEW cAudioManager;
		if(manager)
		{
			if(initializeDefault) 
				manager->initialize();

#if CAUDIO_COMPILE_WITH_OGG_DECODER == 1
			manager->registerAudioDecoder(&OggDecoderFactory, "ogg");
#endif
#if CAUDIO_COMPILE_WITH_WAV_DECODER == 1
			manager->registerAudioDecoder(&WavDecoderFactory, "wav");
#endif

			manager->registerAudioDecoder(&RawDecoderFactory, "raw");

#if CAUDIO_COMPILE_WITH_FILE_SOURCE == 1
			manager->registerDataSource(&FileSourceFactory, "FileSystem", 0);
#endif

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
			cAudioVector<IAudioPlugin*>::Type plugins = cPluginManager::Instance()->getPluginList();
			for(unsigned int i = 0; i < plugins.size(); ++i)
			{
				plugins[i]->onCreateAudioManager(manager);
			}
#endif
		}
		return manager;
	}

	CAUDIO_API void destroyAudioManager(IAudioManager* manager)
	{
		if(manager)
		{
#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
			cAudioVector<IAudioPlugin*>::Type plugins = cPluginManager::Instance()->getPluginList();
			for(unsigned int i = 0; i < plugins.size(); ++i)
			{
				plugins[i]->onDestroyAudioManager(manager);
			}
#endif

			manager->unRegisterAllAudioDecoders();
			manager->unRegisterAllDataSources();
			manager->unRegisterAllEventHandlers();
			manager->shutDown();

			CAUDIO_DELETE manager;
			manager = NULL;
		}
	}

	//---------------------------------------------------------------------------------------
	// Audio capture section
	//---------------------------------------------------------------------------------------

	CAUDIO_API IAudioCapture* createAudioCapture(bool initializeDefault)
	{
		cAudioCapture* capture = CAUDIO_NEW cAudioCapture;
		if(capture)
		{
			if(initializeDefault)
				capture->initialize();			

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
			cAudioVector<IAudioPlugin*>::Type plugins = cPluginManager::Instance()->getPluginList();
			for(unsigned int i = 0; i < plugins.size(); ++i)
			{
				plugins[i]->onCreateAudioCapture(capture);
			}
#endif
		}
		return capture;
	}

	CAUDIO_API void destroyAudioCapture(IAudioCapture* capture)
	{
		if(capture)
		{
#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
			cAudioVector<IAudioPlugin*>::Type plugins = cPluginManager::Instance()->getPluginList();
			for(unsigned int i = 0; i < plugins.size(); ++i)
			{
				plugins[i]->onDestoryAudioCapture(capture);
			}
#endif
			CAUDIO_DELETE capture;
			capture = NULL;
		}
	}

	//---------------------------------------------------------------------------------------
	// Logger section
	//---------------------------------------------------------------------------------------

	static cLogger Logger;
	static bool FirstTimeLogInit(false);

#if CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER == 1
	static cConsoleLogReceiver ConsoleLog;
#endif

#if CAUDIO_COMPILE_WITH_FILE_LOG_RECEIVER == 1
	static cFileLogReceiver FileLog;
#endif

	CAUDIO_API ILogger* getLogger()
	{
		if(!FirstTimeLogInit)
		{
			FirstTimeLogInit = true;
#if CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER == 1
			Logger.registerLogReceiver(&ConsoleLog, "Console");
#endif
#if CAUDIO_COMPILE_WITH_FILE_LOG_RECEIVER == 1
			Logger.registerLogReceiver(&FileLog,"File");
#endif
		}
		return &Logger;
	}

	//---------------------------------------------------------------------------------------
	// IAudioDeviceList section
	//---------------------------------------------------------------------------------------
	CAUDIO_API IAudioDeviceList* createAudioDeviceList(IDeviceType deviceType)
	{
		return CAUDIO_NEW cOpenALAudioDeviceList(deviceType);
	}
}
