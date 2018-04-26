// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IAudioDecoder.h"

namespace cAudio
{
	//! Interface for factories that create Audio Decoders for cAudio
	class IAudioDecoderFactory
	{
		public:
			IAudioDecoderFactory() {}
			virtual ~IAudioDecoderFactory() {}

			//! Returns an audio decoder.
			/**
			\param stream: Data Source to attach to this decoder.
			\return A pointer to a decoder instance, or NULL on failure to allocate. */
			virtual IAudioDecoder* CreateAudioDecoder(IDataSource* stream) = 0;
	};
};
