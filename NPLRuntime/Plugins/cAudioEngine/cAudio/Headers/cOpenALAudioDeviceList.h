// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IAudioDeviceList.h"
#include "cMemoryOverride.h"

namespace cAudio
{
	class cOpenALAudioDeviceList : public IAudioDeviceList, public cMemoryOverride
	{
	public:
		cOpenALAudioDeviceList(IDeviceType deviceType = DT_PLAYBACK);
		virtual ~cOpenALAudioDeviceList();

		virtual unsigned int getDeviceCount();
		virtual cAudioString getDeviceName(unsigned int idx);
		virtual cAudioString getDeviceDescription(unsigned int idx);
		virtual cAudioString getDefaultDeviceName();
		virtual bool isSupported();

	private:
		cAudioVector<cAudioString>::Type AvailableDevices;
		cAudioString DefaultDevice;
		IDeviceType DeviceType;
	};
}