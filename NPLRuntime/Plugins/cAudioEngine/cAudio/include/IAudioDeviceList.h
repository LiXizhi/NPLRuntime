// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioString.h"

namespace cAudio
{
	enum IDeviceType
	{
		DT_PLAYBACK = 0,
		DT_RECORDING = 2
	};

	class IAudioDeviceList
	{
	public:
		virtual unsigned int getDeviceCount() = 0;
		virtual cAudioString getDeviceName(unsigned int idx) = 0;
		virtual cAudioString getDeviceDescription(unsigned int idx) = 0;
		virtual cAudioString getDefaultDeviceName() = 0;
		virtual bool isSupported() = 0;
	};
}