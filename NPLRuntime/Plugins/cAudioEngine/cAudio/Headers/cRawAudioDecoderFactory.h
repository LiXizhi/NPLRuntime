// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cMutex.h"
#include "cRawDecoder.h"
#include "IAudioDecoderFactory.h"

namespace cAudio
{
	class cRawAudioDecoderFactory : public IAudioDecoderFactory
	{
		public:
			cRawAudioDecoderFactory() {}
			~cRawAudioDecoderFactory() {}

			IAudioDecoder* CreateAudioDecoder(IDataSource* stream)
			{
				return CAUDIO_NEW cRawDecoder(stream, 22050, EAF_16BIT_MONO);
			}

			IAudioDecoder* CreateAudioDecoder(IDataSource* stream, unsigned int frequency = 22050, AudioFormats format = EAF_16BIT_MONO)
			{
				return CAUDIO_NEW cRawDecoder(stream, frequency, format);
			}
	};
};

