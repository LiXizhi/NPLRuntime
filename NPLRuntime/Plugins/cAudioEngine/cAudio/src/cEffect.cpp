// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cEffect.h"
#include "cLogger.h"
#include "cUtils.h"
#include "cAudio.h"

#if CAUDIO_EFX_ENABLED == 1

namespace cAudio
{
	cEffect::cEffect(cEFXFunctions* oALFunctions) : EFX(oALFunctions), Type(EET_NULL),
		Volume(1.0f), IgnoreAttenuation(false), LastUpdated(0), Valid(false), Filter(NULL),
		Effect(0), EffectSlot(0)
	{
		cAudioMutexBasicLock lock(Mutex);
		if(EFX)
		{
			if(EFX->Supported)
			{
				cAudioMutexBasicLock lock(EFX->Mutex);

				//Create Effect Slot
				EFX->alGenAuxiliaryEffectSlots(1, &EffectSlot);
				bool error = CheckError();
				if(error)
				{
					EffectSlot = 0;
					return;
				}

				//Create the effect object
				EFX->alGenEffects(1, &Effect);
				error = CheckError();
				if(error)
				{
					EFX->alDeleteAuxiliaryEffectSlots(1, &EffectSlot);
					EffectSlot = 0;
					Effect = 0;
					return;
				}

				//Bind the effect to the effect slot
				EFX->alAuxiliaryEffectSloti(EffectSlot, AL_EFFECTSLOT_EFFECT, Effect);
				error = CheckError();
				if(error)
				{
					EFX->alDeleteEffects(1, &Effect);
					EFX->alDeleteAuxiliaryEffectSlots(1, &EffectSlot);
					EffectSlot = 0;
					Effect = 0;
					return;
				}

				//Everything worked, effect is ready to use
				Valid = true;
			}
		}
	}

	cEffect::~cEffect()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(EFX && EFX->Supported)
		{
			cAudioMutexBasicLock lock(EFX->Mutex);
			EFX->alAuxiliaryEffectSloti(EffectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);

			if(Effect)
			{
				EFX->alDeleteEffects(1, &Effect);
			}

			if(EffectSlot)
			{
				EFX->alDeleteAuxiliaryEffectSlots(1, &EffectSlot);
			}
		}
	}

	const EffectTypes& cEffect::getType() const
	{
		return Type;
	}

	void cEffect::setType(const EffectTypes& type)
	{
		cAudioMutexBasicLock lock(Mutex);
		Type = type;
		UpdateEffect();
	}

	const sEAXReverbParameters& cEffect::getEAXReverbParameters() const
	{
		return EAXReverbParam;
	}

	void cEffect::setEAXReverbParameters(const sEAXReverbParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		EAXReverbParam = param;
		UpdateEffect();
	}

	const sReverbParameters& cEffect::getReverbParameters() const
	{
		return ReverbParam;
	}

	void cEffect::setReverbParameters(const sReverbParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		ReverbParam = param;
		UpdateEffect();
	}

	const sChorusParameters& cEffect::getChorusParameters() const
	{
		return ChorusParam;
	}

	void cEffect::setChorusParameters(const sChorusParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		ChorusParam = param;
		UpdateEffect();
	}

	const sDistortionParameters& cEffect::getDistortionParameters() const
	{
		return DistortionParam;
	}

	void cEffect::setDistortionParameters(const sDistortionParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		DistortionParam = param;
		UpdateEffect();
	}

	const sEchoParameters& cEffect::getEchoParameters() const
	{
		return EchoParam;
	}

	void cEffect::setEchoParameters(const sEchoParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		EchoParam = param;
		UpdateEffect();
	}

	const sFlangerParameters& cEffect::getFlangerParameters() const
	{
		return FlangerParam;
	}

	void cEffect::setFlangerParameters(const sFlangerParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		FlangerParam = param;
		UpdateEffect();
	}

	const sFrequencyShiftParameters& cEffect::getFrequencyShiftParameters() const
	{
		return FrequencyShiftParam;
	}

	void cEffect::setFrequencyShiftParameters(const sFrequencyShiftParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		FrequencyShiftParam = param;
		UpdateEffect();
	}

	const sVocalMorpherParameters& cEffect::getVocalMorpherParameters() const
	{
		return VocalMorpherParam;
	}

	void cEffect::setVocalMorpherParameters(const sVocalMorpherParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		VocalMorpherParam = param;
		UpdateEffect();
	}

	const sPitchShifterParameters& cEffect::getPitchShifterParameters() const
	{
		return PitchShifterParam;
	}

	void cEffect::setPitchShifterParameters(const sPitchShifterParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		PitchShifterParam = param;
		UpdateEffect();
	}

	const sRingModulatorParameters& cEffect::getRingModulatorParameters() const
	{
		return RingModulatorParam;
	}

	void cEffect::setRingModulatorParameters(const sRingModulatorParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		RingModulatorParam = param;
		UpdateEffect();
	}

	const sAutowahParameters& cEffect::getAutowahParameters() const
	{
		return AutowahParam;
	}

	void cEffect::setAutowahParameters(const sAutowahParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		AutowahParam = param;
		UpdateEffect();
	}

	const sCompressorParameters& cEffect::getCompressorParameters() const
	{
		return CompressorParam;
	}

	void cEffect::setCompressorParameters(const sCompressorParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		CompressorParam = param;
		UpdateEffect();
	}

	const sEqualizerParameters& cEffect::getEqualizerParameters() const
	{
		return EqualizerParam;
	}

	void cEffect::setEqualizerParameters(const sEqualizerParameters& param)
	{
		cAudioMutexBasicLock lock(Mutex);
		EqualizerParam = param;
		UpdateEffect();
	}

	float cEffect::getMasterVolume() const
	{
		return Volume;
	}

	void cEffect::setMasterVolume(const float& volume)
	{
		cAudioMutexBasicLock lock(Mutex);
		Volume = volume;
		UpdateEffect();
	}

	bool cEffect::isIgnoringAttenuation() const
	{
		return IgnoreAttenuation;
	}

	void cEffect::ignoreAttenuation(const bool& ignore)
	{
		cAudioMutexBasicLock lock(Mutex);
		IgnoreAttenuation = ignore;
		UpdateEffect();
	}

	IFilter* cEffect::getFilter() const
	{
		return Filter;
	}

	void cEffect::attachFilter(IFilter* filter)
	{
		cAudioMutexBasicLock lock(Mutex);
		Filter = filter;

		if(Filter)
			Filter->grab();
		UpdateEffect();
	}

	void cEffect::removeFilter()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(Filter)
			Filter->drop();
		Filter = NULL;
	}

	unsigned int cEffect::getLastUpdated() const
	{
		return LastUpdated;
	}

	bool cEffect::isValid() const
	{
		return Valid;
	}

	ALuint cEffect::getOpenALEffect() const
	{
		return Effect;
	}

	ALuint cEffect::getOpenALEffectSlot() const
	{
		return EffectSlot;
	}

	bool cEffect::UpdateEffect()
	{
		++LastUpdated;

		ALenum alEffectType = ConvertEffectEnum(Type);

		if(EFX && EFX->Supported && EffectSlot && Effect)
		{
			cAudioMutexBasicLock lock(EFX->Mutex);
			EFX->alEffecti(Effect, AL_EFFECT_TYPE, alEffectType);
			Valid = !CheckError();
			if(!Valid)
				return false;

			EFX->alAuxiliaryEffectSlotf(EffectSlot, AL_EFFECTSLOT_GAIN, Volume);
			Valid = !CheckError();
			if(!Valid)
				return false;

			EFX->alAuxiliaryEffectSloti(EffectSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, IgnoreAttenuation ? AL_TRUE : AL_FALSE);
			Valid = !CheckError();
			if(!Valid)
				return false;

			if(Type == EET_EAX_REVERB)
			{
				EFX->alEffectf(Effect, AL_EAXREVERB_DENSITY, EAXReverbParam.Density);
				EFX->alEffectf(Effect, AL_EAXREVERB_DIFFUSION, EAXReverbParam.Diffusion);
				EFX->alEffectf(Effect, AL_EAXREVERB_GAIN, EAXReverbParam.Gain);
				EFX->alEffectf(Effect, AL_EAXREVERB_GAINHF, EAXReverbParam.GainHF);
				EFX->alEffectf(Effect, AL_EAXREVERB_GAINLF, EAXReverbParam.GainLF);
				EFX->alEffectf(Effect, AL_EAXREVERB_DECAY_TIME, EAXReverbParam.DecayTime);
				EFX->alEffectf(Effect, AL_EAXREVERB_DECAY_HFRATIO, EAXReverbParam.DecayHFRatio);
				EFX->alEffectf(Effect, AL_EAXREVERB_DECAY_LFRATIO, EAXReverbParam.DecayLFRatio);
				EFX->alEffectf(Effect, AL_EAXREVERB_REFLECTIONS_GAIN, EAXReverbParam.ReflectionsGain);
				EFX->alEffectf(Effect, AL_EAXREVERB_REFLECTIONS_DELAY, EAXReverbParam.ReflectionsDelay);
				EFX->alEffectfv(Effect, AL_EAXREVERB_REFLECTIONS_PAN, &EAXReverbParam.ReflectionsPan.x);
				EFX->alEffectf(Effect, AL_EAXREVERB_LATE_REVERB_GAIN, EAXReverbParam.LateReverbGain);
				EFX->alEffectf(Effect, AL_EAXREVERB_LATE_REVERB_DELAY, EAXReverbParam.LateReverbDelay);
				EFX->alEffectfv(Effect, AL_EAXREVERB_LATE_REVERB_PAN, &EAXReverbParam.LateReverbPan.x);
				EFX->alEffectf(Effect, AL_EAXREVERB_ECHO_TIME, EAXReverbParam.EchoTime);
				EFX->alEffectf(Effect, AL_EAXREVERB_ECHO_DEPTH, EAXReverbParam.EchoDepth);
				EFX->alEffectf(Effect, AL_EAXREVERB_MODULATION_TIME, EAXReverbParam.ModulationTime);
				EFX->alEffectf(Effect, AL_EAXREVERB_MODULATION_DEPTH, EAXReverbParam.ModulationDepth);
				EFX->alEffectf(Effect, AL_EAXREVERB_HFREFERENCE, EAXReverbParam.HFReference);
				EFX->alEffectf(Effect, AL_EAXREVERB_LFREFERENCE, EAXReverbParam.LFReference);
				EFX->alEffectf(Effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, EAXReverbParam.RoomRolloffFactor);
				EFX->alEffectf(Effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, EAXReverbParam.AirAbsorptionGainHF);
				EFX->alEffecti(Effect, AL_EAXREVERB_DECAY_HFLIMIT, EAXReverbParam.DecayHFLimit ? AL_TRUE : AL_FALSE);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_REVERB)
			{
				EFX->alEffectf(Effect, AL_REVERB_DENSITY, ReverbParam.Density);
				EFX->alEffectf(Effect, AL_REVERB_DIFFUSION, ReverbParam.Diffusion);
				EFX->alEffectf(Effect, AL_REVERB_GAIN, ReverbParam.Gain);
				EFX->alEffectf(Effect, AL_REVERB_GAINHF, ReverbParam.GainHF);
				EFX->alEffectf(Effect, AL_REVERB_DECAY_TIME, ReverbParam.DecayTime);
				EFX->alEffectf(Effect, AL_REVERB_DECAY_HFRATIO, ReverbParam.DecayHFRatio);
				EFX->alEffectf(Effect, AL_REVERB_REFLECTIONS_GAIN, ReverbParam.ReflectionsGain);
				EFX->alEffectf(Effect, AL_REVERB_REFLECTIONS_DELAY, ReverbParam.ReflectionsDelay);
				EFX->alEffectf(Effect, AL_REVERB_LATE_REVERB_GAIN, ReverbParam.LateReverbGain);
				EFX->alEffectf(Effect, AL_REVERB_LATE_REVERB_DELAY, ReverbParam.LateReverbDelay);
				EFX->alEffectf(Effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, ReverbParam.RoomRolloffFactor);
				EFX->alEffectf(Effect, AL_REVERB_AIR_ABSORPTION_GAINHF, ReverbParam.AirAbsorptionGainHF);
				EFX->alEffecti(Effect, AL_REVERB_DECAY_HFLIMIT, ReverbParam.DecayHFLimit ? AL_TRUE : AL_FALSE);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_CHORUS)
			{
				ALenum waveform = AL_CHORUS_DEFAULT_WAVEFORM;
				switch(ChorusParam.Waveform)
				{
				case sChorusParameters::ECW_SINUSOID:
					waveform = AL_CHORUS_WAVEFORM_SINUSOID;
					break;
				case sChorusParameters::ECW_TRIANGLE:
					waveform = AL_CHORUS_WAVEFORM_TRIANGLE;
					break;
				default:
					waveform = AL_CHORUS_DEFAULT_WAVEFORM;
					break;
				};
				EFX->alEffecti(Effect, AL_CHORUS_WAVEFORM, waveform);
				EFX->alEffecti(Effect, AL_CHORUS_PHASE, ChorusParam.Phase);
				EFX->alEffectf(Effect, AL_CHORUS_RATE, ChorusParam.Rate);
				EFX->alEffectf(Effect, AL_CHORUS_DEPTH, ChorusParam.Depth);
				EFX->alEffectf(Effect, AL_CHORUS_FEEDBACK, ChorusParam.Feedback);
				EFX->alEffectf(Effect, AL_CHORUS_DELAY, ChorusParam.Delay);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_DISTORTION)
			{
				EFX->alEffectf(Effect, AL_DISTORTION_EDGE, DistortionParam.Edge);
				EFX->alEffectf(Effect, AL_DISTORTION_GAIN, DistortionParam.Gain);
				EFX->alEffectf(Effect, AL_DISTORTION_LOWPASS_CUTOFF, DistortionParam.LowpassCutoff);
				EFX->alEffectf(Effect, AL_DISTORTION_EQCENTER, DistortionParam.EqCenter);
				EFX->alEffectf(Effect, AL_DISTORTION_EQBANDWIDTH, DistortionParam.EqBandwidth);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_ECHO)
			{
				EFX->alEffectf(Effect, AL_ECHO_DELAY, EchoParam.Delay);
				EFX->alEffectf(Effect, AL_ECHO_LRDELAY, EchoParam.LRDelay);
				EFX->alEffectf(Effect, AL_ECHO_DAMPING, EchoParam.Damping);
				EFX->alEffectf(Effect, AL_ECHO_FEEDBACK, EchoParam.Feedback);
				EFX->alEffectf(Effect, AL_ECHO_SPREAD, EchoParam.Spread);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_FLANGER)
			{
				ALenum waveform = AL_FLANGER_DEFAULT_WAVEFORM;
				switch(FlangerParam.Waveform)
				{
				case sFlangerParameters::EFW_SINUSOID:
					waveform = AL_FLANGER_WAVEFORM_SINUSOID;
					break;
				case sFlangerParameters::EFW_TRIANGLE:
					waveform = AL_FLANGER_WAVEFORM_SINUSOID;
					break;
				default:
					waveform = AL_FLANGER_DEFAULT_WAVEFORM;
					break;
				};
				EFX->alEffecti(Effect, AL_FLANGER_WAVEFORM, waveform);
				EFX->alEffecti(Effect, AL_FLANGER_PHASE, FlangerParam.Phase);
				EFX->alEffectf(Effect, AL_FLANGER_RATE, FlangerParam.Rate);
				EFX->alEffectf(Effect, AL_FLANGER_DEPTH, FlangerParam.Depth);
				EFX->alEffectf(Effect, AL_FLANGER_FEEDBACK, FlangerParam.Feedback);
				EFX->alEffectf(Effect, AL_FLANGER_DELAY, FlangerParam.Delay);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_FREQUENCY_SHIFTER)
			{
				ALenum shiftleft = ConvertFrequencyShiftEnum(FrequencyShiftParam.Left);
				ALenum shiftright = ConvertFrequencyShiftEnum(FrequencyShiftParam.Right);
				EFX->alEffectf(Effect, AL_FREQUENCY_SHIFTER_FREQUENCY, FrequencyShiftParam.Frequency);
				EFX->alEffecti(Effect, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, shiftleft);
				EFX->alEffecti(Effect, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, shiftright);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_VOCAL_MORPHER)
			{
				ALenum pA = ConvertVocalMorphPhonemeEnum(VocalMorpherParam.PhonemeA);
				ALenum pB = ConvertVocalMorphPhonemeEnum(VocalMorpherParam.PhonemeB);

				ALenum waveform = AL_VOCAL_MORPHER_DEFAULT_WAVEFORM;
				switch(VocalMorpherParam.Waveform)
				{
				case sVocalMorpherParameters::EMW_SINUSOID:
					waveform = AL_VOCAL_MORPHER_WAVEFORM_SINUSOID;
					break;
				case sVocalMorpherParameters::EMW_TRIANGLE:
					waveform = AL_VOCAL_MORPHER_WAVEFORM_TRIANGLE;
					break;
				case sVocalMorpherParameters::EMW_SAW:
					waveform = AL_VOCAL_MORPHER_WAVEFORM_SAWTOOTH;
					break;
				default:
					waveform = AL_VOCAL_MORPHER_DEFAULT_WAVEFORM;
					break;
				};

				EFX->alEffecti(Effect, AL_VOCAL_MORPHER_PHONEMEA, pA);
				EFX->alEffecti(Effect, AL_VOCAL_MORPHER_PHONEMEB, pB);
				EFX->alEffecti(Effect, AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING, VocalMorpherParam.PhonemeACoarseTune);
				EFX->alEffecti(Effect, AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING, VocalMorpherParam.PhonemeBCoarseTune);
				EFX->alEffecti(Effect, AL_VOCAL_MORPHER_WAVEFORM, waveform);
				EFX->alEffectf(Effect, AL_VOCAL_MORPHER_RATE, VocalMorpherParam.Rate);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_PITCH_SHIFTER)
			{
				EFX->alEffecti(Effect, AL_PITCH_SHIFTER_COARSE_TUNE, PitchShifterParam.CoarseTune);
				EFX->alEffecti(Effect, AL_PITCH_SHIFTER_FINE_TUNE, PitchShifterParam.FineTune);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_RING_MODULATOR)
			{
				ALenum waveform = AL_RING_MODULATOR_DEFAULT_WAVEFORM;
				switch(RingModulatorParam.Waveform)
				{
				case sRingModulatorParameters::EMW_SINUSOID:
					waveform = AL_RING_MODULATOR_SINUSOID;
					break;
				case sRingModulatorParameters::EMW_SAW:
					waveform = AL_RING_MODULATOR_SAWTOOTH;
					break;
				case sRingModulatorParameters::EMW_SQUARE:
					waveform = AL_RING_MODULATOR_SQUARE;
					break;
				default:
					waveform = AL_RING_MODULATOR_DEFAULT_WAVEFORM;
					break;
				};

				EFX->alEffectf(Effect, AL_RING_MODULATOR_FREQUENCY, RingModulatorParam.Frequency);
				EFX->alEffectf(Effect, AL_RING_MODULATOR_HIGHPASS_CUTOFF, RingModulatorParam.HighPassCutoff);
				EFX->alEffecti(Effect, AL_RING_MODULATOR_WAVEFORM, waveform);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_AUTOWAH)
			{
				EFX->alEffectf(Effect, AL_AUTOWAH_ATTACK_TIME, AutowahParam.AttackTime);
				EFX->alEffectf(Effect, AL_AUTOWAH_RELEASE_TIME, AutowahParam.ReleaseTime);
				EFX->alEffectf(Effect, AL_AUTOWAH_RESONANCE, AutowahParam.Resonance);
				EFX->alEffectf(Effect, AL_AUTOWAH_PEAK_GAIN, AutowahParam.PeakGain);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_COMPRESSOR)
			{
				EFX->alEffecti(Effect, AL_COMPRESSOR_ONOFF, CompressorParam.Active ? AL_TRUE : AL_FALSE);
				Valid = !CheckError();
				return Valid;
			}
			else if(Type == EET_EQUALIZER)
			{
				EFX->alEffectf(Effect, AL_EQUALIZER_LOW_GAIN, EqualizerParam.LowGain);
				EFX->alEffectf(Effect, AL_EQUALIZER_LOW_CUTOFF, EqualizerParam.LowCutoff);
				EFX->alEffectf(Effect, AL_EQUALIZER_MID1_GAIN, EqualizerParam.Mid1Gain);
				EFX->alEffectf(Effect, AL_EQUALIZER_MID1_CENTER, EqualizerParam.Mid1Center);
				EFX->alEffectf(Effect, AL_EQUALIZER_MID1_WIDTH, EqualizerParam.Mid1Width);
				EFX->alEffectf(Effect, AL_EQUALIZER_MID2_GAIN, EqualizerParam.Mid2Gain);
				EFX->alEffectf(Effect, AL_EQUALIZER_MID2_CENTER, EqualizerParam.Mid2Center);
				EFX->alEffectf(Effect, AL_EQUALIZER_MID2_WIDTH, EqualizerParam.Mid2Width);
				EFX->alEffectf(Effect, AL_EQUALIZER_HIGH_GAIN, EqualizerParam.HighGain);
				EFX->alEffectf(Effect, AL_EQUALIZER_HIGH_CUTOFF, EqualizerParam.HighCutoff);
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

	bool cEffect::CheckError()
	{
		int error = alGetError();
		const char* errorString;

        if (error != AL_NO_ERROR)
        {
			errorString = alGetString(error);
			if(error == AL_OUT_OF_MEMORY)
				getLogger()->logCritical("Audio Effect", "OpenAL Error: %s.", errorString);
			else
				getLogger()->logError("Audio Effect", "OpenAL Error: %s.", errorString);
			return true;
        }
		return false;
	}

	ALenum cEffect::ConvertEffectEnum(EffectTypes type)
	{
		switch(type)
		{
		case EET_NULL:
			return AL_EFFECT_NULL;
		case EET_EAX_REVERB:
			return AL_EFFECT_EAXREVERB;
		case EET_REVERB:
			return AL_EFFECT_REVERB;
		case EET_CHORUS:
			return AL_EFFECT_CHORUS;
		case EET_DISTORTION:
			return AL_EFFECT_DISTORTION;
		case EET_ECHO:
			return AL_EFFECT_ECHO;
		case EET_FLANGER:
			return AL_EFFECT_FLANGER;
		case EET_FREQUENCY_SHIFTER:
			return AL_EFFECT_FREQUENCY_SHIFTER;
		case EET_VOCAL_MORPHER:
			return AL_EFFECT_VOCAL_MORPHER;
		case EET_PITCH_SHIFTER:
			return AL_EFFECT_PITCH_SHIFTER;
		case EET_RING_MODULATOR:
			return AL_EFFECT_RING_MODULATOR;
		case EET_AUTOWAH:
			return AL_EFFECT_AUTOWAH;
		case EET_COMPRESSOR:
			return AL_EFFECT_COMPRESSOR;
		case EET_EQUALIZER:
			return AL_EFFECT_EQUALIZER;
		default:
			return AL_EFFECT_NULL;
		};
	}

	ALenum cEffect::ConvertFrequencyShiftEnum(sFrequencyShiftParameters::ShiftDirection shift)
	{
		switch(shift)
		{
		case sFrequencyShiftParameters::ESD_DOWN:
			return AL_FREQUENCY_SHIFTER_DIRECTION_DOWN;
		case sFrequencyShiftParameters::ESD_UP:
			return AL_FREQUENCY_SHIFTER_DIRECTION_UP;
		case sFrequencyShiftParameters::ESD_OFF:
			return AL_FREQUENCY_SHIFTER_DIRECTION_OFF;
		default:
			return AL_FREQUENCY_SHIFTER_DIRECTION_DOWN;
		};
	}

	ALenum cEffect::ConvertVocalMorphPhonemeEnum(sVocalMorpherParameters::MorpherPhoneme phoneme)
	{
		switch(phoneme)
		{
		case sVocalMorpherParameters::EMP_A:
			return AL_VOCAL_MORPHER_PHONEME_A;
		case sVocalMorpherParameters::EMP_E:
			return AL_VOCAL_MORPHER_PHONEME_E;
		case sVocalMorpherParameters::EMP_I:
			return AL_VOCAL_MORPHER_PHONEME_I;
		case sVocalMorpherParameters::EMP_O:
			return AL_VOCAL_MORPHER_PHONEME_O;
		case sVocalMorpherParameters::EMP_U:
			return AL_VOCAL_MORPHER_PHONEME_U;
		case sVocalMorpherParameters::EMP_AA:
			return AL_VOCAL_MORPHER_PHONEME_AA;
		case sVocalMorpherParameters::EMP_AE:
			return AL_VOCAL_MORPHER_PHONEME_AE;
		case sVocalMorpherParameters::EMP_AH:
			return AL_VOCAL_MORPHER_PHONEME_AH;
		case sVocalMorpherParameters::EMP_AO:
			return AL_VOCAL_MORPHER_PHONEME_AO;
		case sVocalMorpherParameters::EMP_EH:
			return AL_VOCAL_MORPHER_PHONEME_EH;
		case sVocalMorpherParameters::EMP_ER:
			return AL_VOCAL_MORPHER_PHONEME_ER;
		case sVocalMorpherParameters::EMP_IH:
			return AL_VOCAL_MORPHER_PHONEME_IH;
		case sVocalMorpherParameters::EMP_IY:
			return AL_VOCAL_MORPHER_PHONEME_IY;
		case sVocalMorpherParameters::EMP_UH:
			return AL_VOCAL_MORPHER_PHONEME_UH;
		case sVocalMorpherParameters::EMP_UW:
			return AL_VOCAL_MORPHER_PHONEME_UW;
		case sVocalMorpherParameters::EMP_B:
			return AL_VOCAL_MORPHER_PHONEME_B;
		case sVocalMorpherParameters::EMP_D:
			return AL_VOCAL_MORPHER_PHONEME_D;
		case sVocalMorpherParameters::EMP_F:
			return AL_VOCAL_MORPHER_PHONEME_F;
		case sVocalMorpherParameters::EMP_G:
			return AL_VOCAL_MORPHER_PHONEME_G;
		case sVocalMorpherParameters::EMP_J:
			return AL_VOCAL_MORPHER_PHONEME_J;
		case sVocalMorpherParameters::EMP_K:
			return AL_VOCAL_MORPHER_PHONEME_K;
		case sVocalMorpherParameters::EMP_L:
			return AL_VOCAL_MORPHER_PHONEME_L;
		case sVocalMorpherParameters::EMP_M:
			return AL_VOCAL_MORPHER_PHONEME_M;
		case sVocalMorpherParameters::EMP_N:
			return AL_VOCAL_MORPHER_PHONEME_N;
		case sVocalMorpherParameters::EMP_P:
			return AL_VOCAL_MORPHER_PHONEME_P;
		case sVocalMorpherParameters::EMP_R:
			return AL_VOCAL_MORPHER_PHONEME_R;
		case sVocalMorpherParameters::EMP_S:
			return AL_VOCAL_MORPHER_PHONEME_S;
		case sVocalMorpherParameters::EMP_T:
			return AL_VOCAL_MORPHER_PHONEME_T;
		case sVocalMorpherParameters::EMP_V:
			return AL_VOCAL_MORPHER_PHONEME_V;
		case sVocalMorpherParameters::EMP_Z:
			return AL_VOCAL_MORPHER_PHONEME_Z;
		default:
			return AL_VOCAL_MORPHER_DEFAULT_PHONEMEA;
		};
	}
};

#endif