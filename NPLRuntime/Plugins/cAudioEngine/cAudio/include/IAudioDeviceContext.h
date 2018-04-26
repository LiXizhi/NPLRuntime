// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

namespace cAudio
{
	class IAudioManager;
	class IAudioEffects;

	class IAudioDeviceContext
	{
	public:
		virtual bool initialize(const char* deviceName, int outputFrequency, int eaxEffectSlots) = 0;
		virtual void shutDown() = 0; 
		virtual void update() = 0;
		virtual IAudioManager* getAudioManager() const = 0;
		virtual IAudioEffects* getEffects() const = 0;
	};
}