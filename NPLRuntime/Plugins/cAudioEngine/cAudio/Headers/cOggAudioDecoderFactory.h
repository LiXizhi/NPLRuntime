// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IAudioDecoderFactory.h"
#include "cOggDecoder.h"
#include "cMutex.h"

#if CAUDIO_COMPILE_WITH_OGG_DECODER == 1

namespace cAudio
{
	class cOggAudioDecoderFactory : public IAudioDecoderFactory
	{
		public:
			cOggAudioDecoderFactory() {}
			~cOggAudioDecoderFactory() {}

			IAudioDecoder* CreateAudioDecoder(IDataSource* stream)
			{
				return CAUDIO_NEW cOggDecoder(stream);
			}
	};
};

#endif
