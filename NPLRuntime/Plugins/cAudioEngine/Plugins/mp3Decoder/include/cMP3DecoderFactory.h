#ifndef CMP3DECODERFACTORY_H
#define CMP3DECODERFACTORY_H

#include "IAudioDecoderFactory.h"
#include "cMP3Decoder.h"
#include "../Headers/cMutex.h"

using namespace cAudio;

class cMP3DecoderFactory : public IAudioDecoderFactory
{
	public:
		cMP3DecoderFactory() { }
		~cMP3DecoderFactory() { }

        IAudioDecoder* CreateAudioDecoder(IDataSource* stream)
		{
			IAudioDecoder* decoder = new cMP3Decoder(stream);
            return decoder;
		}
};

#endif //! CMP3DECODERFACTORY_H