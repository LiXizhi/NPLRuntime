// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cMutex.h"
#include "cWavDecoder.h"
#include "IAudioDecoderFactory.h"

#if CAUDIO_COMPILE_WITH_WAV_DECODER == 1

namespace cAudio
{
	class cWavAudioDecoderFactory : public IAudioDecoderFactory
	{
		public:
			cWavAudioDecoderFactory() {}
			~cWavAudioDecoderFactory() {}

			IAudioDecoder* CreateAudioDecoder(IDataSource* stream)
			{      
				return CAUDIO_NEW cWavDecoder(stream);
			}
	};
};

#endif
