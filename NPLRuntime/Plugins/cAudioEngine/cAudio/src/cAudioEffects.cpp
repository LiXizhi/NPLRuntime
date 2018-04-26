// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cAudioEffects.h"
#include "cFilter.h"
#include "cEffect.h"
#include "cUtils.h"

#if CAUDIO_EFX_ENABLED == 1

//Number of effect slots to try to create
#define EFFECT_SLOTS_TO_TEST 256

namespace cAudio
{
	cAudioEffects::cAudioEffects() : MaxEffectsSupported(0)
	{
		for(int i=0; i<EET_COUNT; ++i)
			SupportedEffects[i] = false;

		for(int i=0; i<EFT_COUNT; ++i)
			SupportedFilters[i] = false;
	}

	cAudioEffects::~cAudioEffects()
	{

	}

	IEffect* cAudioEffects::createEffect()
	{
		cAudioMutexBasicLock lock(Mutex);
		IEffect* effect = CAUDIO_NEW cEffect(&EFXInterface);

		if(effect && effect->isValid())
			return effect;

		return NULL;
	}

	IFilter* cAudioEffects::createFilter()
	{
		cAudioMutexBasicLock lock(Mutex);
		IFilter* filter = CAUDIO_NEW cFilter(&EFXInterface);

		if(filter && filter->isValid())
			return filter;

		return NULL;
	}

	unsigned int cAudioEffects::getMaxEffectsSupported() const
	{
		return MaxEffectsSupported;
	}

	bool cAudioEffects::isEffectSupported(const EffectTypes& type) const
	{
		return SupportedEffects[type];
	}

	bool cAudioEffects::isFilterSupported(const FilterTypes& type) const
	{
		return SupportedFilters[type];
	}

	bool cAudioEffects::addEAXReverbEffectPreset(const char* name, const sEAXReverbParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		EAXReverbPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addReverbEffectPreset(const char* name, const sReverbParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		ReverbPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addChorusEffectPreset(const char* name, const sChorusParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		ChorusPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addDistortionEffectPreset(const char* name, const sDistortionParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		DistortionPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addEchoEffectPreset(const char* name, const sEchoParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		EchoPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addFlangerEffectPreset(const char* name, const sFlangerParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		FlangerPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addFrequencyShiftEffectPreset(const char* name, const sFrequencyShiftParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		FrequencyShiftPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addVocalMorpherEffectPreset(const char* name, const sVocalMorpherParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		VocalMorpherPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addPitchShifterEffectPreset(const char* name, const sPitchShifterParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		PitchShifterPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addRingModulatorEffectPreset(const char* name, const sRingModulatorParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		RingModulatorPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addAutowahEffectPreset(const char* name, const sAutowahParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		AutowahPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addCompressorEffectPreset(const char* name, const sCompressorParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		CompressorPresets[safeName] = setting;
		return true;
	}

	bool cAudioEffects::addEqualizerEffectPreset(const char* name, const sEqualizerParameters& setting)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		EqualizerPresets[safeName] = setting;
		return true;
	}

	sEAXReverbParameters cAudioEffects::getEAXReverbEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sEAXReverbParameters>::Type::iterator it = EAXReverbPresets.find(safeName);
		if(it != EAXReverbPresets.end())
		{
			return it->second;
		}
		return sEAXReverbParameters();
	}

	sReverbParameters cAudioEffects::getReverbEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sReverbParameters>::Type::iterator it = ReverbPresets.find(safeName);
		if(it != ReverbPresets.end())
		{
			return it->second;
		}
		return sReverbParameters();
	}

	sChorusParameters cAudioEffects::getChorusEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sChorusParameters>::Type::iterator it = ChorusPresets.find(safeName);
		if(it != ChorusPresets.end())
		{
			return it->second;
		}
		return sChorusParameters();
	}

	sDistortionParameters cAudioEffects::getDistortionEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sDistortionParameters>::Type::iterator it = DistortionPresets.find(safeName);
		if(it != DistortionPresets.end())
		{
			return it->second;
		}
		return sDistortionParameters();
	}

	sEchoParameters cAudioEffects::getEchoEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sEchoParameters>::Type::iterator it = EchoPresets.find(safeName);
		if(it != EchoPresets.end())
		{
			return it->second;
		}
		return sEchoParameters();
	}

	sFlangerParameters cAudioEffects::getFlangerEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sFlangerParameters>::Type::iterator it = FlangerPresets.find(safeName);
		if(it != FlangerPresets.end())
		{
			return it->second;
		}
		return sFlangerParameters();
	}

	sFrequencyShiftParameters cAudioEffects::getFrequencyShiftEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sFrequencyShiftParameters>::Type::iterator it = FrequencyShiftPresets.find(safeName);
		if(it != FrequencyShiftPresets.end())
		{
			return it->second;
		}
		return sFrequencyShiftParameters();
	}

	sVocalMorpherParameters cAudioEffects::getVocalMorpherEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sVocalMorpherParameters>::Type::iterator it = VocalMorpherPresets.find(safeName);
		if(it != VocalMorpherPresets.end())
		{
			return it->second;
		}
		return sVocalMorpherParameters();
	}

	sPitchShifterParameters cAudioEffects::getPitchShifterEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sPitchShifterParameters>::Type::iterator it = PitchShifterPresets.find(safeName);
		if(it != PitchShifterPresets.end())
		{
			return it->second;
		}
		return sPitchShifterParameters();
	}

	sRingModulatorParameters cAudioEffects::getRingModulatorEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sRingModulatorParameters>::Type::iterator it = RingModulatorPresets.find(safeName);
		if(it != RingModulatorPresets.end())
		{
			return it->second;
		}
		return sRingModulatorParameters();
	}

	sAutowahParameters cAudioEffects::getAutowahEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sAutowahParameters>::Type::iterator it = AutowahPresets.find(safeName);
		if(it != AutowahPresets.end())
		{
			return it->second;
		}
		return sAutowahParameters();
	}

	sCompressorParameters cAudioEffects::getCompressorEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sCompressorParameters>::Type::iterator it = CompressorPresets.find(safeName);
		if(it != CompressorPresets.end())
		{
			return it->second;
		}
		return sCompressorParameters();
	}

	sEqualizerParameters cAudioEffects::getEqualizerEffectPreset(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		cAudioMap<cAudioString, sEqualizerParameters>::Type::iterator it = EqualizerPresets.find(safeName);
		if(it != EqualizerPresets.end())
		{
			return it->second;
		}
		return sEqualizerParameters();
	}

	void cAudioEffects::removeEffectPreset(const EffectTypes& type, const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		switch(type)
		{
		case EET_EAX_REVERB:
			EAXReverbPresets.erase(safeName);
			break;
		case EET_REVERB:
			ReverbPresets.erase(safeName);
			break;
		case EET_CHORUS:
			ChorusPresets.erase(safeName);
			break;
		case EET_DISTORTION:
			DistortionPresets.erase(safeName);
			break;
		case EET_ECHO:
			EchoPresets.erase(safeName);
			break;
		case EET_FLANGER:
			FlangerPresets.erase(safeName);
			break;
		case EET_FREQUENCY_SHIFTER:
			FrequencyShiftPresets.erase(safeName);
			break;
		case EET_VOCAL_MORPHER:
			VocalMorpherPresets.erase(safeName);
			break;
		case EET_PITCH_SHIFTER:
			PitchShifterPresets.erase(safeName);
			break;
		case EET_RING_MODULATOR:
			RingModulatorPresets.erase(safeName);
			break;
		case EET_AUTOWAH:
			AutowahPresets.erase(safeName);
			break;
		case EET_COMPRESSOR:
			CompressorPresets.erase(safeName);
			break;
		case EET_EQUALIZER:
			EqualizerPresets.erase(safeName);
			break;
		default:
			break;
		}
	}

	bool cAudioEffects::isEffectPresetRegistered(const EffectTypes& type, const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = fromUTF8(name);
		switch(type)
		{
		case EET_EAX_REVERB:
			return (EAXReverbPresets.find(safeName) != EAXReverbPresets.end());
			break;
		case EET_REVERB:
			return (ReverbPresets.find(safeName) != ReverbPresets.end());
			break;
		case EET_CHORUS:
			return (ChorusPresets.find(safeName) != ChorusPresets.end());
			break;
		case EET_DISTORTION:
			return (DistortionPresets.find(safeName) != DistortionPresets.end());
			break;
		case EET_ECHO:
			return (EchoPresets.find(safeName) != EchoPresets.end());
			break;
		case EET_FLANGER:
			return (FlangerPresets.find(safeName) != FlangerPresets.end());
			break;
		case EET_FREQUENCY_SHIFTER:
			return (FrequencyShiftPresets.find(safeName) != FrequencyShiftPresets.end());
			break;
		case EET_VOCAL_MORPHER:
			return (VocalMorpherPresets.find(safeName) != VocalMorpherPresets.end());
			break;
		case EET_PITCH_SHIFTER:
			return (PitchShifterPresets.find(safeName) != PitchShifterPresets.end());
			break;
		case EET_RING_MODULATOR:
			return (RingModulatorPresets.find(safeName) != RingModulatorPresets.end());
			break;
		case EET_AUTOWAH:
			return (AutowahPresets.find(safeName) != AutowahPresets.end());
			break;
		case EET_COMPRESSOR:
			return (CompressorPresets.find(safeName) != CompressorPresets.end());
			break;
		case EET_EQUALIZER:
			return (EqualizerPresets.find(safeName) != EqualizerPresets.end());
			break;
		default:
			return false;
			break;
		}
	}

	void cAudioEffects::removeAllEffectPresets(const EffectTypes& type)
	{
		switch(type)
		{
		case EET_EAX_REVERB:
			EAXReverbPresets.clear();
			break;
		case EET_REVERB:
			ReverbPresets.clear();
			break;
		case EET_CHORUS:
			ChorusPresets.clear();
			break;
		case EET_DISTORTION:
			DistortionPresets.clear();
			break;
		case EET_ECHO:
			EchoPresets.clear();
			break;
		case EET_FLANGER:
			FlangerPresets.clear();
			break;
		case EET_FREQUENCY_SHIFTER:
			FrequencyShiftPresets.clear();
			break;
		case EET_VOCAL_MORPHER:
			VocalMorpherPresets.clear();
			break;
		case EET_PITCH_SHIFTER:
			PitchShifterPresets.clear();
			break;
		case EET_RING_MODULATOR:
			RingModulatorPresets.clear();
			break;
		case EET_AUTOWAH:
			AutowahPresets.clear();
			break;
		case EET_COMPRESSOR:
			CompressorPresets.clear();
			break;
		case EET_EQUALIZER:
			EqualizerPresets.clear();
			break;
		default:
			EAXReverbPresets.clear();
			ReverbPresets.clear();
			ChorusPresets.clear();
			DistortionPresets.clear();
			EchoPresets.clear();
			FlangerPresets.clear();
			FrequencyShiftPresets.clear();
			VocalMorpherPresets.clear();
			PitchShifterPresets.clear();
			RingModulatorPresets.clear();
			AutowahPresets.clear();
			CompressorPresets.clear();
			EqualizerPresets.clear();
			break;
		}
	}

	cEFXFunctions* cAudioEffects::getEFXInterface()
	{
		return &EFXInterface;
	}

	void cAudioEffects::checkEFXSupportDetails()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(EFXInterface.Supported)
		{
			cAudioMutexBasicLock lock(EFXInterface.Mutex);

			//Count the number of effect slots this device supports (limits the max number of effects available)
			unsigned int count = 0;
			ALuint effectSlots[EFFECT_SLOTS_TO_TEST];
			for(count=0; count<EFFECT_SLOTS_TO_TEST; ++count)
			{
				EFXInterface.alGenAuxiliaryEffectSlots(1, &effectSlots[count]);
				if(alGetError() != AL_NO_ERROR)
					break;
			}

			MaxEffectsSupported = count;

			//Check what effects are supported
			ALuint Effect;
			EFXInterface.alGenEffects(1, &Effect);
			if (alGetError() == AL_NO_ERROR)
			{
				SupportedEffects[EET_NULL] = true;

				// Try setting Effect Type to known Effects
				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
				SupportedEffects[EET_REVERB] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
				SupportedEffects[EET_EAX_REVERB] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_CHORUS);
				SupportedEffects[EET_CHORUS] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_DISTORTION);
				SupportedEffects[EET_DISTORTION] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_ECHO);
				SupportedEffects[EET_ECHO] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_FLANGER);
				SupportedEffects[EET_FLANGER] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_FREQUENCY_SHIFTER);
				SupportedEffects[EET_FREQUENCY_SHIFTER] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_VOCAL_MORPHER);
				SupportedEffects[EET_VOCAL_MORPHER] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_PITCH_SHIFTER);
				SupportedEffects[EET_PITCH_SHIFTER] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_RING_MODULATOR);
				SupportedEffects[EET_RING_MODULATOR] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_AUTOWAH);
				SupportedEffects[EET_AUTOWAH] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_COMPRESSOR);
				SupportedEffects[EET_COMPRESSOR] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alEffecti(Effect, AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER);
				SupportedEffects[EET_EQUALIZER] = (alGetError() == AL_NO_ERROR);
			}

			// Delete Effect
			EFXInterface.alDeleteEffects(1, &Effect);

			// Generate a Filter to use to determine what Filter Types are supported
			ALuint Filter;
			EFXInterface.alGenFilters(1, &Filter);
			if (alGetError() == AL_NO_ERROR)
			{
				SupportedFilters[EFT_NULL] = true;

				// Try setting the Filter type to known Filters
				EFXInterface.alFilteri(Filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
				SupportedFilters[EFT_LOWPASS] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alFilteri(Filter, AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
				SupportedFilters[EFT_HIGHPASS] = (alGetError() == AL_NO_ERROR);

				EFXInterface.alFilteri(Filter, AL_FILTER_TYPE, AL_FILTER_BANDPASS);
				SupportedFilters[EFT_BANDPASS] = (alGetError() == AL_NO_ERROR);
			}

			// Delete Filter
			EFXInterface.alDeleteFilters(1, &Filter);

			//Cleanup the slots we created
			while(count > 0)
			{
				EFXInterface.alDeleteAuxiliaryEffectSlots(1, &effectSlots[--count]);
			}
		}
	}

};

#endif