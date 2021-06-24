// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cMemoryOverride.h"
#include "IAudioDecoder.h"
#include "cAudioDefines.h"

#if CAUDIO_COMPILE_WITH_WAV_DECODER == 1

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
			virtual cAudioString getType() const;

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

