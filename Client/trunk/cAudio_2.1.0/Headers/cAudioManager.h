// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CAUDIOMANAGER_H_INCLUDED
#define CAUDIOMANAGER_H_INCLUDED

#include "cAudioSource.h"
#include "../include/IAudioDecoderFactory.h"
#include "cListener.h"
#include "../include/IAudioManager.h"
#include "../Headers/cMutex.h"
#include "../Headers/cAudioEffects.h"
#include "../Headers/cMemoryOverride.h"
#include "../Headers/cSTLAllocator.h"

#include <AL/al.h>
#include <AL/alc.h>

#ifdef CAUDIO_PLATFORM_LINUX
#include <AL/alext.h>
#endif

namespace cAudio
{
	class IAudioSource;

    class cAudioManager : public IAudioManager, public cMemoryOverride
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

		cAudioManager() : Device(NULL), Context(NULL), EFXSupported(false), Initialized(false) { }
		virtual ~cAudioManager() { }

		virtual bool initialize(const char* deviceName = 0x0, int outputFrequency = -1, int eaxEffectSlots = 4);      
		virtual void shutDown();     
		virtual void update();
		virtual IAudioSource* getSoundByName(const char* name);       
		virtual void releaseAllSources();
		virtual void release(IAudioSource* source);

		virtual const char* getAvailableDeviceName(unsigned int index);
		virtual unsigned int getAvailableDeviceCount();
		virtual const char* getDefaultDeviceName();

		virtual IAudioSource* create(const char* name, const char* filename, bool stream = false);
		virtual IAudioSource* createFromMemory(const char* name, const char* data, size_t length, const char* extension);
		virtual IAudioSource* createFromRaw(const char* name, const char* data, size_t length, unsigned int frequency, AudioFormats format);
      
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

		//! Grabs a list of available devices, as well as the default system one
		void getAvailableDevices();

		virtual IListener* getListener() { return &initlistener; }

#ifdef CAUDIO_EFX_ENABLED
		virtual IAudioEffects* getEffects() { return &initEffects; }
#endif

	private:
		//! Mutex for thread syncronization
		cAudioMutex Mutex;

		//! An OpenAL context pointer
		ALCcontext* Context;
		//! An OpenAL device pointer
		ALCdevice* Device;

		//! Holds whether EFX is supported
		bool EFXSupported;

		//! Whether the manager is currently initialized and ready to go.
		bool Initialized;

		//! Holds an index for fast searching of audio sources by name
		cAudioMap<cAudioString, IAudioSource*>::Type audioIndex;
		typedef cAudioMap<cAudioString, IAudioSource*>::Type::iterator audioIndexIterator;
		//! Holds all managed audio sources
		cAudioVector<IAudioSource*>::Type audioSources;
		//! Decoder map that holds all decoders by file extension
		cAudioMap<cAudioString, IAudioDecoderFactory*>::Type decodermap;
		typedef cAudioMap<cAudioString, IAudioDecoderFactory*>::Type::iterator decodermapIterator;
		//! Archive map that holds all datasource types
		cAudioMap<cAudioString, IDataSourceFactory*>::Type datasourcemap;
		typedef cAudioMap<cAudioString, IDataSourceFactory*>::Type::iterator datasourcemapIterator;
		cAudioVector< std::pair<int, cAudioString> >::Type dataSourcePriorityList;

		//! The listener object        
		cListener initlistener;
#ifdef CAUDIO_EFX_ENABLED
		//! Interface for audio effects
		cAudioEffects initEffects;
#endif
		//! Check for OpenAL errors
		bool checkError();

		cAudioVector<cAudioString>::Type AvailableDevices;
		cAudioString DefaultDevice;

		//! Signals a event to all event handlers
		void signalEvent(Events sevent);
		
		//! List of all attached event handlers
		cAudioList<IManagerEventHandler*>::Type eventHandlerList;
    };
}

#endif //! CAUDIOMANAGER_H_INCLUDED
