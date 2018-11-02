// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"

#if CAUDIO_EFX_ENABLED == 1

#include "IFilter.h"
#include "cMutex.h"
#include "cEFXFunctions.h"
#include "cMemoryOverride.h"

#include <al.h>
#include <alc.h>

namespace cAudio
{
	class cFilter : public IFilter, public cMemoryOverride
	{
	public:
		cFilter(cEFXFunctions* oALFunctions);
		~cFilter();

		virtual const FilterTypes& getType() const;
		virtual void setType(const FilterTypes& type);

		virtual float getVolume() const;
		virtual void setVolume(const float& volume);

		virtual float getLowFrequencyVolume() const;
		virtual void setLowFrequencyVolume(const float& volumeLF);

		virtual float getHighFrequencyVolume() const;
		virtual void setHighFrequencyVolume(const float& volumeHF);

		virtual unsigned int getLastUpdated() const;
		virtual bool isValid() const;

		ALuint getOpenALFilter() const;

	protected:
		cEFXFunctions* EFX;

		//Mutex for thread syncronization
		cAudioMutex Mutex;

		FilterTypes Type;
		float Volume;
		float LowFreqVolume;
		float HighFreqVolume;
		unsigned int LastUpdated;

		bool Valid;

		ALuint Filter;

		bool UpdateFilter();
		bool CheckError();
		ALenum ConvertFilterEnum(FilterTypes type);
	};
};

#endif
