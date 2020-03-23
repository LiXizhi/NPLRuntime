// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CWAVAUDIODECODERFACOTRY_H_INCLUDED
#define CWAVAUDIODECODERFACOTRY_H_INCLUDED

#include "../include/IAudioDecoderFactory.h"
#include "cWavDecoder.h"
#include "../Headers/cMutex.h"

#ifdef CAUDIO_COMPILE_WITH_WAV_DECODER

namespace cAudio
{

class cWavAudioDecoderFactory : public IAudioDecoderFactory
{
    public:
        cWavAudioDecoderFactory() {}
        ~cWavAudioDecoderFactory() {}

        IAudioDecoder* CreateAudioDecoder(IDataSource* stream)
        {
			Mutex.lock();
            IAudioDecoder* decoder = CAUDIO_NEW cWavDecoder(stream);
			Mutex.unlock();
			return decoder;
        }
    protected:
		cAudioMutex Mutex;
    private:
};

};

#endif

#endif //! CWAVAUDIODECODERFACOTRY_H_INCLUDED
