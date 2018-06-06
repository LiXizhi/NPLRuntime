// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CWAVDECODER_H_INCLUDED
#define CWAVDECODER_H_INCLUDED

#include "../include/IAudioDecoder.h"
#include "../include/cAudioDefines.h"
#include "../Headers/cMemoryOverride.h"

#ifdef CAUDIO_COMPILE_WITH_WAV_DECODER

namespace cAudio
{
    class cWavDecoder : public IAudioDecoder, public cMemoryOverride
    {
        public:

            cWavDecoder(IDataSource* stream);
            ~cWavDecoder();

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
            short Channels;
            int SampleRate;
            int ByteRate;
            short BlockAlign;
            short BitsPerSample;
            int DataSize;
			int DataOffset;

			bool Valid;
    };
};

#endif

#endif //! CFLACDECODER_H_INCLUDED

