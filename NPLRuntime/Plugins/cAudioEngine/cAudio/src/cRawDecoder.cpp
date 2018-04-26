// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cRawDecoder.h"

namespace cAudio{

	cRawDecoder::cRawDecoder(IDataSource* stream,  unsigned int frequency, AudioFormats format) : IAudioDecoder(stream), Frequency(frequency), Format(format)
    {

    }

    cRawDecoder::~cRawDecoder()
    {

    }

    AudioFormats cRawDecoder::getFormat()
    {
        return Format;
    }

    int cRawDecoder::getFrequency()
    {
        return Frequency;
    }

    bool cRawDecoder::isSeekingSupported()
    {
        return true;
    }

	bool cRawDecoder::isValid()
	{
		return true;
	}

    int cRawDecoder::readAudioData(void* output, int amount)
    {
        return Stream->read(output,amount);
    }

    bool cRawDecoder::setPosition(int position, bool relative)
    {
        Stream->seek(position,relative);
        return true;
    }

    bool cRawDecoder::seek(float seconds,bool relative)
    {
		int SampleSize = 1;
		if(Format == EAF_8BIT_MONO)
			SampleSize = 1;
		else if(Format == EAF_8BIT_STEREO)
			SampleSize = 2;
		else if(Format == EAF_16BIT_MONO)
			SampleSize = 2;
		else
			SampleSize = 4;

        int amountToSeek = seconds * (float)Frequency * (float)SampleSize;
        return setPosition(amountToSeek, relative);
    }

	float cRawDecoder::getTotalTime()
	{
		int SampleSize = 0;
		if(Format == EAF_8BIT_MONO)
			SampleSize = 1;
		else if(Format == EAF_8BIT_STEREO)
			SampleSize = 2;
		else if(Format == EAF_16BIT_MONO)
			SampleSize = 2;
		else
			SampleSize = 4;
		return (float)Stream->getSize() / ((float)Frequency * (float)SampleSize);
	}

	int cRawDecoder::getTotalSize()
	{
		return Stream->getSize();
	}

	int cRawDecoder::getCompressedSize()
	{
		return Stream->getSize();
	}

	float cRawDecoder::getCurrentTime()
	{
		int SampleSize = 0;
		if(Format == EAF_8BIT_MONO)
			SampleSize = 1;
		else if(Format == EAF_8BIT_STEREO)
			SampleSize = 2;
		else if(Format == EAF_16BIT_MONO)
			SampleSize = 2;
		else
			SampleSize = 4;

		return (float)Stream->getCurrentPos() / ((float)Frequency * (float)SampleSize);
	}

	int cRawDecoder::getCurrentPosition()
	{
		return Stream->getCurrentPos();
	}

	int cRawDecoder::getCurrentCompressedPosition()
	{
		return Stream->getCurrentPos();
	}

	cAudioString cRawDecoder::getType() const
	{
		return cAudioString(_CTEXT("cRawDecoder"));
	}
}
