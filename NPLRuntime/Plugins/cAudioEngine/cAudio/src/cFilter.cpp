// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cFilter.h"
#include "cLogger.h"
#include "cUtils.h"
#include "cAudio.h"

#if CAUDIO_EFX_ENABLED == 1

namespace cAudio
{
	cFilter::cFilter(cEFXFunctions* oALFunctions) : EFX(oALFunctions), Type(EFT_NULL), 
		Volume(1.0f), LowFreqVolume(1.0f), HighFreqVolume(1.0f), LastUpdated(0),
		Valid(false), Filter(0)
	{
		cAudioMutexBasicLock lock(Mutex);
		if(EFX)
			Valid = EFX->Supported;

		if(Valid)
		{
			cAudioMutexBasicLock lock(EFX->Mutex);
			EFX->alGenFilters(1, &Filter);
			Valid = !CheckError();
			if(!Valid)
				Filter = 0;
		}
	}

	cFilter::~cFilter()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(Filter && EFX && EFX->Supported)
		{
			cAudioMutexBasicLock lock(EFX->Mutex);
			EFX->alDeleteFilters(1, &Filter);
		}
	}

	const FilterTypes& cFilter::getType() const
	{
		return Type;
	}

	void cFilter::setType(const FilterTypes& type)
	{
		cAudioMutexBasicLock lock(Mutex);
		Type = type;
		UpdateFilter();
	}

	float cFilter::getVolume() const
	{
		return Volume;
	}

	void cFilter::setVolume(const float& volume)
	{
		cAudioMutexBasicLock lock(Mutex);
		Volume = volume;
		UpdateFilter();
	}

	float cFilter::getLowFrequencyVolume() const
	{
		return LowFreqVolume;
	}

	void cFilter::setLowFrequencyVolume(const float& volumeLF)
	{
		cAudioMutexBasicLock lock(Mutex);
		LowFreqVolume = volumeLF;
		UpdateFilter();
	}

	float cFilter::getHighFrequencyVolume() const
	{
		return HighFreqVolume;
	}

	void cFilter::setHighFrequencyVolume(const float& volumeHF)
	{
		cAudioMutexBasicLock lock(Mutex);
		HighFreqVolume = volumeHF;
		UpdateFilter();
	}

	unsigned int cFilter::getLastUpdated() const
	{
		return LastUpdated;
	}

	bool cFilter::isValid() const
	{
		return Valid;
	}

	ALuint cFilter::getOpenALFilter() const
	{
		return Filter;
	}

	bool cFilter::UpdateFilter()
	{
		++LastUpdated;

		ALenum alFilterType = ConvertFilterEnum(Type);

		if(Filter && EFX && EFX->Supported)
		{
			cAudioMutexBasicLock lock(EFX->Mutex);
			EFX->alFilteri(Filter, AL_FILTER_TYPE, alFilterType);
			Valid = !CheckError();
			if(!Valid)
				return false;

			if(Type == EFT_LOWPASS)
			{
				EFX->alFilterf(Filter, AL_LOWPASS_GAIN, Volume);
				EFX->alFilterf(Filter, AL_LOWPASS_GAINHF, HighFreqVolume);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EFT_HIGHPASS)
			{
				EFX->alFilterf(Filter, AL_HIGHPASS_GAIN, Volume);
				EFX->alFilterf(Filter, AL_HIGHPASS_GAINLF, LowFreqVolume);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EFT_BANDPASS)
			{
				EFX->alFilterf(Filter, AL_BANDPASS_GAIN, Volume);
				EFX->alFilterf(Filter, AL_BANDPASS_GAINLF, LowFreqVolume);
				EFX->alFilterf(Filter, AL_BANDPASS_GAINHF, HighFreqVolume);
				Valid = !CheckError();
				return Valid;
			}
			else
			{
				return true;
			}
		}
		return false;
	}

	bool cFilter::CheckError()
	{
		int error = alGetError();
		const char* errorString;

        if (error != AL_NO_ERROR)
        {
			errorString = alGetString(error);
			if(error == AL_OUT_OF_MEMORY)
				getLogger()->logCritical("Audio Filter", "OpenAL Error: %s.", errorString);
			else
				getLogger()->logError("Audio Filter", "OpenAL Error: %s.", errorString);
			return true;
        }
		return false;
	}

	ALenum cFilter::ConvertFilterEnum(FilterTypes type)
	{
		switch(type)
		{
		case EFT_NULL:
			return AL_FILTER_NULL;
		case EFT_LOWPASS:
			return AL_FILTER_LOWPASS;
		case EFT_HIGHPASS:
			return AL_FILTER_HIGHPASS;
		case EFT_BANDPASS:
			return AL_FILTER_BANDPASS;
		default:
			return AL_FILTER_NULL;
		};
	}
};

#endif