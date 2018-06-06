// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CRAWDECODER_H_INCLUDED
#define CRAWDECODER_H_INCLUDED

#include "../include/IAudioDecoder.h"
#include "../Headers/cMemoryOverride.h"

namespace cAudio
{
    class cRawDecoder : public IAudioDecoder, public cMemoryOverride
    {
        public:

			cRawDecoder(IDataSource* stream, unsigned int frequency, AudioFormats format);
            ~cRawDecoder();

            virtual AudioFormats getFormat();
            virtual int getFrequency();
            virtual bool isSeekingSupported();
			virtual bool isValid();
            virtual int readAudioData(void* output, int amount);
            virtual bool setPosition(int position, bool relative);
            virtual bool seek(float seconds,bool relative);
			virtual float getTotalTime();
			virtual int getTotalSize();
			virtual int getCompressedSize();
			virtual float getCurrentTime();
			virtual int getCurrentPosition();
			virtual int getCurrentCompressedPosition();

        private:
            unsigned int Frequency;
			AudioFormats Format;
    };
};

#endif //! CRAWDECODER_H_INCLUDED