// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include <list>
#include <string>
#include <vector>
#include <al.h>
#include <alc.h>

#include "cMutex.h"
#include "cEFXFunctions.h"
#include "cMemoryOverride.h"
#include "IAudioSource.h"
#include "cVector3.h"
#include "ILogger.h"
#include "cSTLAllocator.h"
#include "IAudioDeviceContext.h"

namespace cAudio
{

    class cAudioSource : public IAudioSource, public cMemoryOverride
    {
    public:

		enum Events{
			ON_UPDATE,
			ON_RELEASE,
			ON_PLAY,
			ON_PAUSE,
			ON_STOP,
		};

#if CAUDIO_EFX_ENABLED == 1
		cAudioSource(IAudioDecoder* decoder, IAudioDeviceContext* context, cEFXFunctions* oALFunctions);
#else
		cAudioSource(IAudioDecoder* decoder, IAudioDeviceContext* context);
#endif
		~cAudioSource();

		virtual bool play();
		virtual bool play2d(const bool& toLoop = false);
		virtual bool play3d(const cVector3& position, const float& soundstr = 1.0 , const bool& toLoop = false);
		
		virtual void pause();    
		virtual void stop();
		virtual void loop(const bool& toLoop);
		virtual bool seek(const float& seconds, bool relative = false);

		virtual float getTotalAudioTime();
		virtual int getTotalAudioSize();
		virtual int getCompressedAudioSize();

		virtual float getCurrentAudioTime();
		virtual int getCurrentAudioPosition();
		virtual int getCurrentCompressedAudioPosition();

		virtual bool update();

		virtual const bool isValid() const;
		virtual const bool isPlaying() const;
		virtual const bool isPaused() const;
		virtual const bool isStopped() const;
		virtual const bool isLooping() const;
      
		virtual void setPosition(const cVector3& position);
		virtual void setVelocity(const cVector3& velocity);
		virtual void setDirection(const cVector3& direction);

		virtual void setRolloffFactor(const float& rolloff);
		virtual void setStrength(const float& soundstrength);
		virtual void setMinDistance(const float& minDistance);
		virtual void setMaxAttenuationDistance(const float& maxDistance);

		virtual void setPitch(const float& pitch);
		virtual void setVolume(const float& volume);
		virtual void setMinVolume(const float& minVolume);
		virtual void setMaxVolume(const float& maxVolume);

		virtual void setInnerConeAngle(const float& innerAngle);
		virtual void setOuterConeAngle(const float& outerAngle);
		virtual void setOuterConeVolume(const float& outerVolume);

		virtual void setDopplerStrength(const float& dstrength); 
		virtual void setDopplerVelocity(const cVector3& dvelocity);

		virtual void move(const cVector3& position);

		virtual const cVector3 getPosition() const;
		virtual const cVector3 getVelocity() const;
		virtual const cVector3 getDirection() const;

		virtual const float getRolloffFactor() const;
		virtual const float getStrength() const;
		virtual const float getMinDistance() const;
		virtual const float getMaxDistance() const;

		virtual const float getPitch() const;
		virtual const float getVolume() const;
		virtual const float getMinVolume() const;
		virtual const float getMaxVolume() const;

		virtual const float getInnerConeAngle() const;
		virtual const float getOuterConeAngle() const;
		virtual const float getOuterConeVolume() const;

		virtual const float getDopplerStrength() const;
		virtual const cVector3 getDopplerVelocity() const;

		virtual void registerEventHandler(ISourceEventHandler* handler);
		virtual void unRegisterEventHandler(ISourceEventHandler* handler);
		virtual void unRegisterAllEventHandlers();

		virtual bool drop(); //! Override the default behavior

#if CAUDIO_EFX_ENABLED == 1
		virtual unsigned int getNumEffectSlotsAvailable() const;
		virtual bool attachEffect(unsigned int slot, IEffect* effect);
		virtual void removeEffect(unsigned int slot);

		virtual bool attachFilter(IFilter* filter);
		virtual void removeFilter();
#endif

	private:
		//! Mutex for thread synchronization
		cAudioMutex Mutex;
		//! Empties the current working buffer queue
		void empty();
		//! Checks for OpenAL errors and reports them
		bool checkError() const;
		//! Streams audio data from the decoder into a buffer
		bool stream(ALuint buffer);
		//! Signals a event to all event handlers
		void signalEvent(Events sevent);
		//! Converts our audio format enum to OpenAL's
		ALenum convertAudioFormatEnum(AudioFormats format);

		//! The context that owns this source
		IAudioDeviceContext* Context;
		
		//! Holds the current volume
		float Volume;

		//! Internal audio buffers
		ALuint Buffers[CAUDIO_SOURCE_NUM_BUFFERS]; 
		//! OpenAL source
		ALuint Source; 

		ALenum oldState;
		//! cAudio decoder being used to stream data
		IAudioDecoder* Decoder;

		//! Stores whether the source is to loop the audio stream
		bool Loop;
		//! Stores whether the source is ready to be used
		bool Valid;

		//! List of registered event handlers
		cAudioList<ISourceEventHandler*>::Type eventHandlerList;

#if CAUDIO_EFX_ENABLED == 1
		//! Holds pointers to all the EFX related functions
		cEFXFunctions* EFX;
		//! Updates the attached filter
		void updateFilter(bool remove = false);
		//! Updates the effect attached to a specific slot
		void updateEffect(unsigned int slot, bool remove = false);

		//! Stores the effects attached to this source
		IEffect* Effects[CAUDIO_SOURCE_MAX_EFFECT_SLOTS];
		//! Stores the last updated time stamps for the attached effects
		unsigned int LastEffectTimeStamp[CAUDIO_SOURCE_MAX_EFFECT_SLOTS];

		//! Stores the attached direct feed filter
		IFilter* Filter;
		//! Stores the last updated time stamp for the attached filter
		unsigned int LastFilterTimeStamp;

		//! Number of effects supported by the OpenAL Context
		unsigned int EffectSlotsAvailable;
#endif
    };
};
