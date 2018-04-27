// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include <al.h>
#include <alc.h>

#include "IAudioDeviceContext.h"
#include "cMemoryOverride.h"
#include "cMutex.h"
#include "cAudioEffects.h"

namespace cAudio
{
	class cOpenALDeviceContext : public IAudioDeviceContext, public cMemoryOverride
	{
	public:
		cOpenALDeviceContext(IAudioManager* audioManager);
		virtual ~cOpenALDeviceContext();
		virtual bool initialize(const char* deviceName, int outputFrequency, int eaxEffectSlots) override;
		virtual void shutDown() override;
		virtual void update()override;
		virtual IAudioManager* getAudioManager() const override;
		virtual IAudioEffects* getEffects() const override;
		ALCcontext* getOpenALContext() const ;

        virtual void release() override;
	private:
		
		//! Check for OpenAL errors
		bool checkError();

		//! Mutex for thread synchronization
		cAudioMutex Mutex;

		//! An OpenAL context pointer
		ALCcontext* Context;

		//! An OpenAL device pointer
		ALCdevice* Device;

		IAudioManager* AudioManager;

		bool Initialized;

		//! Holds whether EFX is supported
		bool EFXSupported;

#if CAUDIO_EFX_ENABLED == 1
		//! Interface for audio effects
		cAudioEffects initEffects;
#endif
	};
}
