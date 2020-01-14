// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef IAUDIODECODERFACTORY_H
#define IAUDIODECODERFACTORY_H

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
		protected:
		private:
	};
};

#endif //! IAUDIODECODERFACTORY_H
