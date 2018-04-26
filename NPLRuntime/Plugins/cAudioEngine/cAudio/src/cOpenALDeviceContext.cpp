// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cOpenALDeviceContext.h"
#include "cAudio.h"
#include "cLogger.h"
#include "cAudioEffects.h"

#if CAUDIO_EFX_ENABLED == 1
#	ifdef CAUDIO_PLATFORM_WIN
#		include <efx.h>
#		include <efx-creative.h>
#		include <xram.h>
#	elif defined(CAUDIO_PLATFORM_LINUX)
#		include <AL/alext.h>
#	endif
#endif

namespace cAudio
{
	cOpenALDeviceContext::cOpenALDeviceContext(IAudioManager* audioManager) : AudioManager(audioManager), Context(NULL), Device(NULL), Initialized(false)
	{

	}

	cOpenALDeviceContext::~cOpenALDeviceContext()
	{

	}

	bool cOpenALDeviceContext::initialize(const char* deviceName, int outputFrequency, int eaxEffectSlots)
	{
		cAudioMutexBasicLock lock(Mutex);

		if(Initialized)
			return false;

		//Stores the context attributes (MAX of 4, with 2 zeros to terminate)
		ALint attribs[6] = { 0 };

		unsigned int currentAttrib = 0;
		if(outputFrequency > 0)
		{
			attribs[currentAttrib++] = ALC_FREQUENCY;
			attribs[currentAttrib++] = outputFrequency;
		}
#if CAUDIO_EFX_ENABLED == 1
		if(eaxEffectSlots > 0)
		{
			attribs[currentAttrib++] = ALC_MAX_AUXILIARY_SENDS;
			attribs[currentAttrib++] = eaxEffectSlots;
		}
#endif

		//Create a new device
		Device = alcOpenDevice(deviceName);
		//Check if device can be created
		if (Device == NULL)
		{
			getLogger()->logError("AudioManager", "Failed to Create OpenAL Device.");
			checkError();
			return false;
		}

		Context = alcCreateContext(Device, attribs);
		if (Context == NULL)
		{
			getLogger()->logError("AudioManager", "Failed to Create OpenAL Context.");
			checkError();
			alcCloseDevice(Device);
			Device = NULL;
			return false;
		}

		if(!alcMakeContextCurrent(Context))
		{
			getLogger()->logError("AudioManager", "Failed to make OpenAL Context current.");
			checkError();
			alcDestroyContext(Context);
			alcCloseDevice(Device);
			Context = NULL;
			Device = NULL;
			return false;
		}

		getLogger()->logInfo("AudioManager", "OpenAL Version: %s", alGetString(AL_VERSION));
		getLogger()->logInfo("AudioManager", "Vendor: %s", alGetString(AL_VENDOR));
		getLogger()->logInfo("AudioManager", "Renderer: %s", alGetString(AL_RENDERER));

#if CAUDIO_EFX_ENABLED == 1
		initEffects.getEFXInterface()->Mutex.lock();
		EFXSupported = initEffects.getEFXInterface()->CheckEFXSupport(Device);
		initEffects.getEFXInterface()->Mutex.unlock();
		initEffects.checkEFXSupportDetails();

		if(EFXSupported)
		{
			int EFXMajorVersion = 0;
			int EFXMinorVersion = 0;
			alcGetIntegerv(Device, ALC_EFX_MAJOR_VERSION, 1, &EFXMajorVersion);
			alcGetIntegerv(Device, ALC_EFX_MINOR_VERSION, 1, &EFXMinorVersion);
			getLogger()->logInfo("AudioManager", "EFX Version: %i.%i", EFXMajorVersion, EFXMinorVersion);
			getLogger()->logInfo("AudioManager", "EFX supported and enabled.");
		}
		else
		{
			getLogger()->logWarning("AudioManager", "EFX is not supported, EFX disabled.");
		}
#endif
		getLogger()->logInfo("AudioManager", "Supported Extensions: %s", alGetString(AL_EXTENSIONS));

		return true;
	}

	void cOpenALDeviceContext::shutDown()
	{
		if(Initialized)
		{
			cAudioMutexBasicLock lock(Mutex);
			//Reset context to null
			alcMakeContextCurrent(NULL);
			//Delete the context
			alcDestroyContext(Context);
			Context = NULL;
			//Close the device
			alcCloseDevice(Device);
			checkError();

			Device = NULL;
			Initialized = false;
		}
	}

	void cOpenALDeviceContext::update()
	{
		
	}

	IAudioManager* cOpenALDeviceContext::getAudioManager() const
	{
		return AudioManager;
	}

	IAudioEffects* cOpenALDeviceContext::getEffects() const
	{
#if CAUDIO_EFX_ENABLED == 1
		if(EFXSupported)
			return (IAudioEffects*)&initEffects;
#endif

		return NULL;
	}

	ALCcontext* cOpenALDeviceContext::getOpenALContext() const
	{
		return Context;
	}

	bool cOpenALDeviceContext::checkError()
	{
		int error = alGetError();
		const char* errorString;

		if (error != AL_NO_ERROR)
		{
			errorString = alGetString(error);
			getLogger()->logError("AudioManager", "OpenAL Error: %s.", errorString);
			return true;
		}

		if(Device)
		{
			error = alcGetError(Device);
			if (error != AL_NO_ERROR)
			{
				errorString = alGetString(error);
				getLogger()->logError("AudioManager", "OpenAL Error: %s.", errorString);
				return true;
			}
		}
		return false;
	}
}