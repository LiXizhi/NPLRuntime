// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"

#if CAUDIO_EFX_ENABLED == 1

#include <al.h>
#include <alc.h>
#include "cMutex.h"
#include "cEFXFunctions.h"
#include "cMemoryOverride.h"
#include "IEffect.h"

namespace cAudio
{
	class cEffect : public IEffect, public cMemoryOverride
	{
	public:
		cEffect(cEFXFunctions* oALFunctions);
		virtual ~cEffect();

		virtual const EffectTypes& getType() const;
		virtual void setType(const EffectTypes& type);

		virtual const sEAXReverbParameters& getEAXReverbParameters() const;
		virtual void setEAXReverbParameters(const sEAXReverbParameters& param);

		virtual const sReverbParameters& getReverbParameters() const;
		virtual void setReverbParameters(const sReverbParameters& param);

		virtual const sChorusParameters& getChorusParameters() const;
		virtual void setChorusParameters(const sChorusParameters& param);

		virtual const sDistortionParameters& getDistortionParameters() const;
		virtual void setDistortionParameters(const sDistortionParameters& param);

		virtual const sEchoParameters& getEchoParameters() const;
		virtual void setEchoParameters(const sEchoParameters& param);

		virtual const sFlangerParameters& getFlangerParameters() const;
		virtual void setFlangerParameters(const sFlangerParameters& param);

		virtual const sFrequencyShiftParameters& getFrequencyShiftParameters() const;
		virtual void setFrequencyShiftParameters(const sFrequencyShiftParameters& param);

		virtual const sVocalMorpherParameters& getVocalMorpherParameters() const;
		virtual void setVocalMorpherParameters(const sVocalMorpherParameters& param);

		virtual const sPitchShifterParameters& getPitchShifterParameters() const;
		virtual void setPitchShifterParameters(const sPitchShifterParameters& param);

		virtual const sRingModulatorParameters& getRingModulatorParameters() const;
		virtual void setRingModulatorParameters(const sRingModulatorParameters& param);

		virtual const sAutowahParameters& getAutowahParameters() const;
		virtual void setAutowahParameters(const sAutowahParameters& param);

		virtual const sCompressorParameters& getCompressorParameters() const;
		virtual void setCompressorParameters(const sCompressorParameters& param);

		virtual const sEqualizerParameters& getEqualizerParameters() const;
		virtual void setEqualizerParameters(const sEqualizerParameters& param);

		virtual float getMasterVolume() const;
		virtual void setMasterVolume(const float& volume);

		virtual bool isIgnoringAttenuation() const;
		virtual void ignoreAttenuation(const bool& ignore);

		virtual IFilter* getFilter() const;
		virtual void attachFilter(IFilter* filter);
		virtual void removeFilter();

		virtual unsigned int getLastUpdated() const;
		virtual bool isValid() const;

		ALuint getOpenALEffect() const;
		ALuint getOpenALEffectSlot() const;

	private:
		cEFXFunctions* EFX;
		//Mutex for thread syncronization
		cAudioMutex Mutex;

		EffectTypes Type;
		float Volume;
		bool IgnoreAttenuation;
		unsigned int LastUpdated;

		bool Valid;

		IFilter* Filter;

		ALuint Effect;
		ALuint EffectSlot;

		bool UpdateEffect();
		bool CheckError();
		ALenum ConvertEffectEnum(EffectTypes type);
		ALenum ConvertFrequencyShiftEnum(sFrequencyShiftParameters::ShiftDirection shift);
		ALenum ConvertVocalMorphPhonemeEnum(sVocalMorpherParameters::MorpherPhoneme phoneme);

		//Storage for all the possible settings for effects
		sEAXReverbParameters		EAXReverbParam;
		sReverbParameters			ReverbParam;
		sChorusParameters			ChorusParam;
		sDistortionParameters		DistortionParam;
		sEchoParameters				EchoParam;
		sFlangerParameters			FlangerParam;
		sFrequencyShiftParameters	FrequencyShiftParam;
		sVocalMorpherParameters		VocalMorpherParam;
		sPitchShifterParameters		PitchShifterParam;
		sRingModulatorParameters	RingModulatorParam;
		sAutowahParameters			AutowahParam;
		sCompressorParameters		CompressorParam;
		sEqualizerParameters		EqualizerParam;
	};
};


#endif