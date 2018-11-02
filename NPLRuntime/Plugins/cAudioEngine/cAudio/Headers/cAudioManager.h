// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cListener.h"
#include "cMutex.h"
#include "cAudioEffects.h"
#include "cMemoryOverride.h"
#include "IAudioDecoderFactory.h"
#include "IAudioManager.h"
#include "cSTLAllocator.h"
#include "cAudioString.h"
#include "IThread.h"
#include "IAudioDeviceContext.h"

namespace cAudio
{
	class IAudioSource;

	class cAudioManager : public IAudioManager, public cMemoryOverride, public IThreadWorker
	{
	public:
		enum Events{
			ON_INIT,
			ON_UPDATE,
			ON_RELEASE,
			ON_SOURCECREATE,
			ON_DECODERREGISTER,
			ON_DATASOURCEREGISTER,
		};

		cAudioManager() : AudioThread(NULL), AudioContext(NULL), Initialized(false), MasterVolume(1.0f) { }
		virtual ~cAudioManager();

		virtual bool initialize(const char* deviceName = 0x0, int outputFrequency = -1, int eaxEffectSlots = 4);      
		virtual void shutDown();     
		virtual void update();
		virtual IAudioSource* getSoundByName(const char* name);       
		virtual void releaseAllSources();
		virtual void release(IAudioSource* source);

		virtual IAudioSource* play2D(const char* filename, bool playLooped = false, bool startPaused = false);
		virtual IAudioSource* play3D(const char* filename, cVector3 position, bool playLooped = false, bool startPaused = false);

		virtual void setMasterVolume(float vol);
		virtual float getMasterVolume() const;
		virtual void stopAllSounds();

		virtual IAudioSource* create(const char* name, const char* filename, bool stream = false);
		virtual IAudioSource* createFromMemory(const char* name, const char* data, size_t length, const char* extension);
		virtual IAudioSource* createFromRaw(const char* name, const char* data, size_t length, unsigned int frequency, AudioFormats format);
		virtual IAudioSource* createFromAudioBuffer(const char* name, AudioCaptureBuffer* pBiffer, unsigned int frequency, AudioFormats format);

		virtual bool registerAudioDecoder(IAudioDecoderFactory* factory, const char* extension);
		virtual void unRegisterAudioDecoder(const char* extension);
		virtual bool isAudioDecoderRegistered(const char* extension);
		virtual IAudioDecoderFactory* getAudioDecoderFactory(const char* extension);
		virtual void unRegisterAllAudioDecoders();

		virtual bool registerDataSource(IDataSourceFactory* factory, const char* name, int priority);
		virtual void unRegisterDataSource(const char* name);
		virtual bool isDataSourceRegistered(const char* name);
		virtual IDataSourceFactory* getDataSourceFactory(const char* name);
		virtual void unRegisterAllDataSources();

		virtual void registerEventHandler(IManagerEventHandler* handler);
		virtual void unRegisterEventHandler(IManagerEventHandler* handler) ;
		virtual void unRegisterAllEventHandlers();

		virtual IListener* getListener() { return &initlistener; }

		virtual bool isUpdateThreadRunning();

#if CAUDIO_EFX_ENABLED == 1
		virtual IAudioEffects* getEffects();
#endif

	protected:
		virtual void run();

		IAudioSource* createAudioSource(IAudioDecoder* decoder, const cAudioString& audioName, const cAudioString& dataSource);

	private:
		//! Mutex for thread synchronization
		cAudioMutex Mutex;

		//! Whether the manager is currently initialized and ready to go.
		bool Initialized;

		//! Our update thread
		IThread* AudioThread;

		IAudioDeviceContext* AudioContext;

		float MasterVolume;

		//! Holds an index for fast searching of audio sources by name
		cAudioMap<cAudioString, IAudioSource*>::Type audioIndex;
		typedef cAudioMap<cAudioString, IAudioSource*>::Type::iterator audioIndexIterator;
		//! Holds all managed audio sources
		cAudioVector<IAudioSource*>::Type audioSources;
		//! Holds audio sources which gets deleted from the audioManager
		cAudioVector<IAudioSource*>::Type managedAudioSources;
		//! Holds audio sources which gets deleted from the audioManager
		cAudioVector<IAudioSource*>::Type managedAudioSourcesDelBuffer;
		//! Decoder map that holds all decoders by file extension
		cAudioMap<cAudioString, IAudioDecoderFactory*>::Type decodermap;
		typedef cAudioMap<cAudioString, IAudioDecoderFactory*>::Type::iterator decodermapIterator;
		//! Archive map that holds all datasource types
		cAudioMap<cAudioString, IDataSourceFactory*>::Type datasourcemap;
		typedef cAudioMap<cAudioString, IDataSourceFactory*>::Type::iterator datasourcemapIterator;
		cAudioVector< std::pair<int, cAudioString> >::Type dataSourcePriorityList;

		//! The listener object        
		cListener initlistener;

		//! Signals a event to all event handlers
		void signalEvent(Events sevent);
		
		//! List of all attached event handlers
		cAudioList<IManagerEventHandler*>::Type eventHandlerList;
	};
}

