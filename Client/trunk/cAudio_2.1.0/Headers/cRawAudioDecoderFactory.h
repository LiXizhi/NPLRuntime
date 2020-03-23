// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CRAWAUDIODECODERFACOTRY_H_INCLUDED
#define CRAWAUDIODECODERFACOTRY_H_INCLUDED

#include "../include/IAudioDecoderFactory.h"
#include "cRawDecoder.h"
#include "../Headers/cMutex.h"

namespace cAudio
{

class cRawAudioDecoderFactory : public IAudioDecoderFactory
{
    public:
        cRawAudioDecoderFactory() {}
        ~cRawAudioDecoderFactory() {}

		IAudioDecoder* CreateAudioDecoder(IDataSource* stream)
        {
			Mutex.lock();
            IAudioDecoder* decoder = CAUDIO_NEW cRawDecoder(stream, 22050, EAF_16BIT_MONO);
			Mutex.unlock();
			return decoder;
        }

		IAudioDecoder* CreateAudioDecoder(IDataSource* stream, unsigned int frequency = 22050, AudioFormats format = EAF_16BIT_MONO)
        {
			Mutex.lock();
            IAudioDecoder* decoder = CAUDIO_NEW cRawDecoder(stream, frequency, format);
			Mutex.unlock();
			return decoder;
        }
    protected:
		cAudioMutex Mutex;
    private:
};

};

#endif //! CRAWAUDIODECODERFACOTRY_H_INCLUDED
