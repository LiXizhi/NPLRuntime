// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cAudioSource.h"
#include "cAudio.h"
#include "cLogger.h"
#include "cFilter.h"
#include "cEffect.h"
#include "cAudioSleep.h"
#include <string.h>

#if CAUDIO_EFX_ENABLED == 1
#include "cOpenALDeviceContext.h"
#endif

namespace cAudio
{
#if CAUDIO_EFX_ENABLED == 1
    cAudioSource::cAudioSource(IAudioDecoder* decoder, IAudioDeviceContext* context, cEFXFunctions* oALFunctions)
		: Context(context), Source(0), Volume(1.0), Decoder(decoder), Loop(false), Valid(false),
		EFX(oALFunctions), Filter(NULL), EffectSlotsAvailable(0), LastFilterTimeStamp(0)
#else
	cAudioSource::cAudioSource(IAudioDecoder* decoder, IAudioDeviceContext* context)
		: Context(context), Source(0), Volume(1.0), Decoder(decoder), Loop(false), Valid(false)
#endif
    {
		cAudioMutexBasicLock lock(Mutex);

		for(int i=0; i<CAUDIO_SOURCE_NUM_BUFFERS; ++i)
			Buffers[i] = 0;

#if CAUDIO_EFX_ENABLED == 1
		for(int i=0; i<CAUDIO_SOURCE_MAX_EFFECT_SLOTS; ++i)
			Effects[i] = NULL;

		for(int i=0; i<CAUDIO_SOURCE_MAX_EFFECT_SLOTS; ++i)
			LastEffectTimeStamp[i] = 0;
#endif

		if(Decoder)
			Decoder->grab();

		//Generates 3 buffers for the ogg file
		alGenBuffers(CAUDIO_SOURCE_NUM_BUFFERS, Buffers);
		bool state = !checkError();
		if(state)
		{
			//Creates one source to be stored.
			alGenSources(1, &Source);
			state = !checkError();
			setVolume(Volume);
		}

#if CAUDIO_EFX_ENABLED == 1
		Valid = state && (Decoder != NULL) && (Context != NULL) && (EFX != NULL);

		int numSlots = 0;
		ALCdevice* device = alcGetContextsDevice(((cOpenALDeviceContext*)Context)->getOpenALContext());
		alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &numSlots);

		EffectSlotsAvailable = (numSlots <= CAUDIO_SOURCE_MAX_EFFECT_SLOTS) ? numSlots : CAUDIO_SOURCE_MAX_EFFECT_SLOTS;
#else
		Valid = state && (Decoder != NULL) && (Context != NULL);
#endif
    }

    cAudioSource::~cAudioSource()
    {
		cAudioMutexBasicLock lock(Mutex);

#if CAUDIO_EFX_ENABLED == 1
		for(int i=0; i<CAUDIO_SOURCE_MAX_EFFECT_SLOTS; ++i)
		{
			if(Effects[i])
				Effects[i]->drop();
			Effects[i] = NULL;
		}

		if(Filter)
			Filter->drop();
		Filter = NULL;
#endif
		
		//Stops the audio Source
		alSourceStop(Source);
		empty();
		//Deletes the source
		alDeleteSources(1, &Source);
		//deletes the last filled buffer
		alDeleteBuffers(CAUDIO_SOURCE_NUM_BUFFERS, Buffers);
		checkError();
		getLogger()->logDebug("Audio Source", "Audio source released.");
		signalEvent(ON_RELEASE);

		if(Decoder)
			Decoder->drop();

		unRegisterAllEventHandlers();
    }


	bool cAudioSource::drop()
	{
		--RefCount;
		if (RefCount == 0)
		{
			Context->getAudioManager()->release(this);
			return true;
		}
		return false;
	}

	bool cAudioSource::play()
	{
		cAudioMutexBasicLock lock(Mutex);
		if (!isPaused())
        {
            int queueSize = 0;
			//Purges all buffers from the source
			alSourcei(Source, AL_BUFFER, 0);
			checkError();
            for(int u = 0; u < CAUDIO_SOURCE_NUM_BUFFERS; u++)
            {
                int val = stream(Buffers[u]);

                if(val < 0)
                {
                    return false;
                }
                else if(val > 0)
                    ++queueSize;
            }
            //Stores the sources 3 buffers to be used in the queue
            alSourceQueueBuffers(Source, queueSize, Buffers);
			checkError();
        }
#if CAUDIO_EFX_ENABLED == 1
		updateFilter();
		for(unsigned int i=0; i<CAUDIO_SOURCE_MAX_EFFECT_SLOTS; ++i)
			updateEffect(i);
#endif
        alSourcePlay(Source);
		checkError();
		getLogger()->logDebug("Audio Source", "Source playing.");
		signalEvent(ON_PLAY);
		oldState = AL_PLAYING;
        return true;
    }

	bool cAudioSource::play2d(const bool& toLoop)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcei(Source, AL_SOURCE_RELATIVE, true);
        loop(toLoop);
        bool state = play();
		checkError();
		return state;
    }

	bool cAudioSource::play3d(const cVector3& position, const float& soundstr, const bool& toLoop)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcei(Source, AL_SOURCE_RELATIVE, false);
        setPosition(position);
        setStrength(soundstr);
        loop(toLoop);
        bool state = play();
		checkError();
		return state;
    }

	void cAudioSource::pause()
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcePause(Source);
		checkError();
		getLogger()->logDebug("Audio Source", "Source paused.");
		signalEvent(ON_PAUSE);
		oldState = AL_PAUSED;
    }

	void cAudioSource::stop()
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourceStop(Source);

		//INFO:FIXED EXTREME SLOWDOWN ON IPHONE
		int queued = 0;
		alGetSourcei(Source, AL_BUFFERS_QUEUED, &queued);
		while ( queued-- )
		{
			ALuint buffer;
			alSourceUnqueueBuffers(Source, 1, &buffer);
		}

		//Resets the audio to the beginning
		Decoder->setPosition(0, false);
		checkError();
		getLogger()->logDebug("Audio Source", "Source stopped.");
		signalEvent(ON_STOP);
		oldState = AL_STOPPED;
    }

	void cAudioSource::loop(const bool& loop)
	{
		cAudioMutexBasicLock lock(Mutex);
        Loop = loop;
    }

	bool cAudioSource::seek(const float& seconds, bool relative)
	{
		bool state = false;
		cAudioMutexBasicLock lock(Mutex);
        if(Decoder->isSeekingSupported())
        {
			state = Decoder->seek(seconds, relative);
        }
		return state;
    }

	float cAudioSource::getTotalAudioTime()
	{
		return Decoder->getTotalTime();
	}

	int cAudioSource::getTotalAudioSize()
	{
		return Decoder->getTotalSize();
	}

	int cAudioSource::getCompressedAudioSize()
	{
		return Decoder->getCompressedSize();
	}

	float cAudioSource::getCurrentAudioTime()
	{
		return Decoder->getCurrentTime();
	}

	int cAudioSource::getCurrentAudioPosition()
	{
		return Decoder->getCurrentPosition();
	}

	int cAudioSource::getCurrentCompressedAudioPosition()
	{
		return Decoder->getCurrentCompressedPosition();
	}

	bool cAudioSource::update()
	{
		cAudioMutexBasicLock lock(Mutex);

		int processed = 0;
		bool active = true;
        if(isValid() || isPlaying())
		{
#if CAUDIO_EFX_ENABLED == 1
			updateFilter();
			for(unsigned int i=0; i<CAUDIO_SOURCE_MAX_EFFECT_SLOTS; ++i)
				updateEffect(i);
#endif

			//gets the sound source processed buffers
			alGetSourcei(Source, AL_BUFFERS_PROCESSED, &processed);
            checkError();
            
			//while there is more data refill buffers with audio data.
			while (processed--)
			{
				ALuint buffer;
				alSourceUnqueueBuffers(Source, 1, &buffer);

				if (checkError()) 
				{
					processed++;
					cAudioSleep(1);
					continue;
				}

				// LiXizhi 2014.12.2 fixing a bug where stopped music still get played 1-2 seconds on rewind. 
				// if audio is already stopped, we should not read from the buffer (changing the buffer position)
				if (oldState != AL_STOPPED)
				{
					active = stream(buffer);
					//if more in stream continue playing.
					if (active)
					{
						alSourceQueueBuffers(Source, 1, &buffer);
					}
				}

				if (checkError()) 
				{
					processed++;
					cAudioSleep(1);
					continue;
				}
			}

			signalEvent(ON_UPDATE);
		}

		ALenum state;
		alGetSourcei(Source, AL_SOURCE_STATE, &state);
        checkError();
		if(state == AL_STOPPED && oldState != state)
		{
			//Resets the audio to the beginning
			Decoder->setPosition(0, false);
			getLogger()->logDebug("Audio Source", "Source stopped.");
			signalEvent(ON_STOP);
			oldState = state;
		}

		return active;
    }

	const bool cAudioSource::isValid() const
	{
        return Valid;
	}

	const bool cAudioSource::isPlaying() const
	{
		ALenum state = 0;
        alGetSourcei(Source, AL_SOURCE_STATE, &state);
        checkError();
        return (state == AL_PLAYING);
    }

	const bool cAudioSource::isPaused() const
	{
		ALenum state = 0;
        alGetSourcei(Source, AL_SOURCE_STATE, &state);
        checkError();
        return (state == AL_PAUSED);
    }

	const bool cAudioSource::isStopped() const
	{
		ALenum state = 0;
        alGetSourcei(Source, AL_SOURCE_STATE, &state);
        checkError();
		return (state == AL_STOPPED);
    }

	const bool cAudioSource::isLooping() const
	{
		return Loop;
	}

	void cAudioSource::setPosition(const cVector3& position)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSource3f(Source, AL_POSITION, position.x, position.y, position.z);
		checkError();
    }

	void cAudioSource::setVelocity(const cVector3& velocity)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSource3f(Source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
		checkError();
    }

	void cAudioSource::setDirection(const cVector3& direction)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSource3f(Source, AL_DIRECTION, direction.x, direction.y, direction.z);
		checkError();
    }

	void cAudioSource::setRolloffFactor(const float& rolloff)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_ROLLOFF_FACTOR, rolloff);
		checkError();
    }

	void cAudioSource::setStrength(const float& soundstrength)
	{
		float inverseStrength = 0.0f;
		if(soundstrength > 0.0f)
			inverseStrength = 1.0f / soundstrength;

		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_ROLLOFF_FACTOR, inverseStrength);
		checkError();
    }

	void cAudioSource::setMinDistance(const float& minDistance)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_REFERENCE_DISTANCE, minDistance);
		checkError();
	}

	void cAudioSource::setMaxAttenuationDistance(const float& maxDistance)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_MAX_DISTANCE, maxDistance);
		checkError();
	}

	void cAudioSource::setPitch(const float& pitch)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef (Source, AL_PITCH, pitch);
		checkError();
    }

	void cAudioSource::setVolume(const float& volume)
	{
		cAudioMutexBasicLock lock(Mutex);
		Volume = volume;
        alSourcef(Source, AL_GAIN, Volume * Context->getAudioManager()->getMasterVolume());
		checkError();
    }

	void cAudioSource::setMinVolume(const float& minVolume)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_MIN_GAIN, minVolume);
		checkError();
	}

	void cAudioSource::setMaxVolume(const float& maxVolume)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_MAX_GAIN, maxVolume);
		checkError();
	}

	void cAudioSource::setInnerConeAngle(const float& innerAngle)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_CONE_INNER_ANGLE, innerAngle);
		checkError();
	}

	void cAudioSource::setOuterConeAngle(const float& outerAngle)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_CONE_OUTER_ANGLE, outerAngle);
		checkError();
	}

	void cAudioSource::setOuterConeVolume(const float& outerVolume)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_CONE_OUTER_GAIN, outerVolume);
		checkError();
	}

	void cAudioSource::setDopplerStrength(const float& dstrength)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSourcef(Source, AL_DOPPLER_FACTOR, dstrength);
		checkError();
    }

	void cAudioSource::setDopplerVelocity(const cVector3& dvelocity)
	{
		cAudioMutexBasicLock lock(Mutex);
        alSource3f(Source, AL_DOPPLER_VELOCITY, dvelocity.x, dvelocity.y, dvelocity.z);
		checkError();
    }

	void cAudioSource::move(const cVector3& position)
	{
		cAudioMutexBasicLock lock(Mutex);
		cVector3 oldPos = getPosition();
		cVector3 velocity = position - oldPos;

        alSource3f(Source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
		alSource3f(Source, AL_POSITION, position.x, position.y, position.z);
		checkError();
	}

	const cVector3 cAudioSource::getPosition() const
	{
		cVector3 position;
		alGetSourcefv(Source, AL_POSITION, &position.x);
        checkError();
		return position;
	}

	const cVector3 cAudioSource::getVelocity() const
	{
		cVector3 velocity;
		alGetSourcefv(Source, AL_VELOCITY, &velocity.x);
		return velocity;
	}

	const cVector3 cAudioSource::getDirection() const
	{
		cVector3 direction;
		alGetSourcefv(Source, AL_DIRECTION, &direction.x);
        checkError();
		return direction;
	}

	const float cAudioSource::getRolloffFactor() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_ROLLOFF_FACTOR, &value);
        checkError();
		return value;
	}

	const float cAudioSource::getStrength() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_ROLLOFF_FACTOR, &value);
        checkError();
        
		float inverseStrength = 0.0f;
		if(value > 0.0f)
			inverseStrength = 1.0f / value;

		return inverseStrength;
	}

	const float cAudioSource::getMinDistance() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_REFERENCE_DISTANCE, &value);
        checkError();
		return value;
	}

	const float cAudioSource::getMaxDistance() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_MAX_DISTANCE, &value);
        checkError();
		return value;
	}

	const float cAudioSource::getPitch() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_PITCH, &value);
        checkError();
		return value;
	}

	const float cAudioSource::getVolume() const
	{
		return Volume;
	}

	const float cAudioSource::getMinVolume() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_MIN_GAIN, &value);
        checkError();
		return value;
	}

	const float cAudioSource::getMaxVolume() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_MAX_GAIN, &value);
        checkError();
		return value;
	}

	const float cAudioSource::getInnerConeAngle() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_CONE_INNER_ANGLE, &value);
        checkError();
		return value;
	}

	const float cAudioSource::getOuterConeAngle() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_CONE_OUTER_ANGLE, &value);
        checkError();
		return value;
	}

	const float cAudioSource::getOuterConeVolume() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_CONE_OUTER_GAIN, &value);
        checkError();
		return value;
	}

	const float cAudioSource::getDopplerStrength() const
	{
		float value = 0.0f;
		alGetSourcef(Source, AL_DOPPLER_FACTOR, &value);
        checkError();
		return value;
	}

	const cVector3 cAudioSource::getDopplerVelocity() const
	{
		cVector3 velocity;
		alGetSourcefv(Source, AL_DOPPLER_VELOCITY, &velocity.x);
        checkError();
		return velocity;
	}

#if CAUDIO_EFX_ENABLED == 1
	unsigned int cAudioSource::getNumEffectSlotsAvailable() const
	{
		return EffectSlotsAvailable;
	}

	bool cAudioSource::attachEffect(unsigned int slot, IEffect* effect)
	{
		cAudioMutexBasicLock lock(Mutex);
		if(slot < EffectSlotsAvailable)
		{
			Effects[slot] = effect;

			if(Effects[slot])
				Effects[slot]->grab();

			updateEffect(slot);
			return true;
		}
		return false;
	}

	void cAudioSource::removeEffect(unsigned int slot)
	{
		cAudioMutexBasicLock lock(Mutex);
		if(slot < EffectSlotsAvailable)
		{
			if(Effects[slot])
				Effects[slot]->drop();

			Effects[slot] = NULL;
			LastEffectTimeStamp[slot] = 0;
			updateEffect(slot, true);
		}
	}

	bool cAudioSource::attachFilter(IFilter* filter)
	{
		cAudioMutexBasicLock lock(Mutex);
		Filter = filter;

		if(Filter)
			Filter->grab();

		updateFilter();
		return true;
	}

	void cAudioSource::removeFilter()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(Filter)
			Filter->drop();
		Filter = NULL;
		LastFilterTimeStamp = 0;
		updateFilter(true);
	}
#endif

    void cAudioSource::empty()
    {
        int queued = 0;
        alGetSourcei(Source, AL_BUFFERS_QUEUED, &queued);
        checkError();
        
        while (queued--)
        {
            ALuint buffer;
            alSourceUnqueueBuffers(Source, 1, &buffer);
			checkError();
        }
    }

	bool cAudioSource::checkError() const
    {
        int error = alGetError();
        if (error != AL_NO_ERROR)
        {
			const char* errorString = alGetString(error);
			if(error == AL_OUT_OF_MEMORY)
				getLogger()->logCritical("Audio Source", "OpenAL Error: %s.", errorString);
			else
				getLogger()->logError("Audio Source", "OpenAL Error: %s.", errorString);
			return true;
        }
		return false;
    }

    bool cAudioSource::stream(ALuint buffer)
    {
        if(Decoder)
        {
	        //stores the calculated data into buffer that is passed to output.
			size_t totalread = 0;
			unsigned int errorcount = 0;
	        char tempbuffer[CAUDIO_SOURCE_BUFFER_SIZE];
			int nActualreadFailCount = 0;
			while( totalread < CAUDIO_SOURCE_BUFFER_SIZE )
			{
				char tempbuffer2[CAUDIO_SOURCE_BUFFER_SIZE];
				int actualread = Decoder->readAudioData(tempbuffer2, CAUDIO_SOURCE_BUFFER_SIZE-totalread);
				if(actualread > 0)
				{
					memcpy(tempbuffer+totalread,tempbuffer2,actualread);
					totalread += actualread;
				}
				if(actualread < 0)
				{
					++errorcount;
					getLogger()->logDebug("Audio Source", "Decoder returned an error: %i (%i of 3)", actualread, errorcount);
					if(errorcount >= 3)
					{
						stop();
						break;
					}
				}
				if (actualread == 0 && ++nActualreadFailCount>2)
				{
					if(isLooping())
					{
						//If we are to loop, set to the beginning and reload from the start
						Decoder->setPosition(0, false);
						getLogger()->logDebug("Audio Source", "Buffer looping.");
						break;
					}
					else
						break;
				}
			}

	        //Second check, in case looping is not enabled, we will return false for end of stream
	        if(totalread == 0)
	       	{
	       		return false;
	        }
			getLogger()->logDebug("Audio Source", "Buffered %i bytes of data into buffer %i at %i hz.", totalread, buffer, Decoder->getFrequency());
            alBufferData(buffer, convertAudioFormatEnum(Decoder->getFormat()), tempbuffer, totalread, Decoder->getFrequency());
			checkError();
            return true;
        }
		return false;
    }

	ALenum cAudioSource::convertAudioFormatEnum(AudioFormats format)
	{
		switch(format)
		{
		case EAF_8BIT_MONO:
			return AL_FORMAT_MONO8;
		case EAF_16BIT_MONO:
			return AL_FORMAT_MONO16;
		case EAF_8BIT_STEREO:
			return AL_FORMAT_STEREO8;
		case EAF_16BIT_STEREO:
			return AL_FORMAT_STEREO16;
		default:
			return AL_FORMAT_MONO8;
		};
	}

#if CAUDIO_EFX_ENABLED == 1
	void cAudioSource::updateFilter(bool remove)
	{
		if(!remove)
		{
			if(Filter && Filter->isValid())
			{
				if(LastFilterTimeStamp != Filter->getLastUpdated())
				{
					LastFilterTimeStamp = Filter->getLastUpdated();
					cFilter* theFilter = static_cast<cFilter*>(Filter);
					if(theFilter)
					{
						alSourcei(Source, AL_DIRECT_FILTER, theFilter->getOpenALFilter());
						checkError();
						return;
					}
				}
				return;
			}
		}
		alSourcei(Source, AL_DIRECT_FILTER, AL_FILTER_NULL);
		checkError();
	}

	void cAudioSource::updateEffect(unsigned int slot, bool remove)
	{
		if(slot < EffectSlotsAvailable)
		{
			if(!remove)
			{
				if(Effects[slot] && Effects[slot]->isValid())
				{
					if(LastEffectTimeStamp[slot] != Effects[slot]->getLastUpdated())
					{
						LastEffectTimeStamp[slot] = Effects[slot]->getLastUpdated();
						cEffect* theEffect = static_cast<cEffect*>(Effects[slot]);
						if(theEffect)
						{
							ALuint filterID = AL_FILTER_NULL;
							cFilter* theFilter = static_cast<cFilter*>(theEffect->getFilter());
							if(theFilter)
							{
								filterID = theFilter->getOpenALFilter();
							}
							alSource3i(Source, AL_AUXILIARY_SEND_FILTER, theEffect->getOpenALEffectSlot(), slot, filterID);
							checkError();
							return;
						}
					}
					return;
				}
			}
			alSource3i(Source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, slot, AL_FILTER_NULL);
			checkError();
		}
	}
#endif

	void cAudioSource::registerEventHandler(ISourceEventHandler* handler)
	{
		if(handler)
		{
		    cAudioMutexBasicLock lock(Mutex);
			eventHandlerList.push_back(handler);
		}
	}

	void cAudioSource::unRegisterEventHandler(ISourceEventHandler* handler)
	{
		if(handler)
		{
		    cAudioMutexBasicLock lock(Mutex);
			eventHandlerList.remove(handler);
		}
	}

	void cAudioSource::unRegisterAllEventHandlers()
	{
	    cAudioMutexBasicLock lock(Mutex);
		eventHandlerList.clear();
	}

	void cAudioSource::signalEvent(Events sevent)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioList<ISourceEventHandler*>::Type::iterator it = eventHandlerList.begin();

		if(it != eventHandlerList.end()){

			switch(sevent){

				case ON_UPDATE:

					for( ; it != eventHandlerList.end(); it++){
						(*it)->onUpdate();
					}

					break;

				case ON_RELEASE:

					for( ; it != eventHandlerList.end(); it++){
						(*it)->onRelease();
					}

					break;

				case ON_PLAY:

					for( ; it != eventHandlerList.end(); it++){
						(*it)->onPlay();
					}


					break;

				case ON_PAUSE:

					for( ; it != eventHandlerList.end(); it++){
						(*it)->onPause();
					}

					break;

				case ON_STOP:

					for( ; it != eventHandlerList.end(); it++){
						(*it)->onStop();
					}

					break;
			}
		}
	}
}
