// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"

#if CAUDIO_EFX_ENABLED == 1

#include "cMutex.h"
#include "cEFXFunctions.h"
#include "IAudioEffects.h"
#include "cSTLAllocator.h"
#include "cAudioString.h"

namespace cAudio
{
	class cAudioEffects : public IAudioEffects
    {
    public:
		cAudioEffects();
        virtual ~cAudioEffects();

		virtual IEffect* createEffect();
		virtual IFilter* createFilter();

		virtual unsigned int getMaxEffectsSupported() const;
		virtual bool isEffectSupported(const EffectTypes& type) const;
		virtual bool isFilterSupported(const FilterTypes& type) const;

		virtual bool addEAXReverbEffectPreset(const char* name, const sEAXReverbParameters& setting);
		virtual bool addReverbEffectPreset(const char* name, const sReverbParameters& setting);
		virtual bool addChorusEffectPreset(const char* name, const sChorusParameters& setting);
		virtual bool addDistortionEffectPreset(const char* name, const sDistortionParameters& setting);
		virtual bool addEchoEffectPreset(const char* name, const sEchoParameters& setting);
		virtual bool addFlangerEffectPreset(const char* name, const sFlangerParameters& setting);
		virtual bool addFrequencyShiftEffectPreset(const char* name, const sFrequencyShiftParameters& setting);
		virtual bool addVocalMorpherEffectPreset(const char* name, const sVocalMorpherParameters& setting);
		virtual bool addPitchShifterEffectPreset(const char* name, const sPitchShifterParameters& setting);
		virtual bool addRingModulatorEffectPreset(const char* name, const sRingModulatorParameters& setting);
		virtual bool addAutowahEffectPreset(const char* name, const sAutowahParameters& setting);
		virtual bool addCompressorEffectPreset(const char* name, const sCompressorParameters& setting);
		virtual bool addEqualizerEffectPreset(const char* name, const sEqualizerParameters& setting);

		virtual sEAXReverbParameters getEAXReverbEffectPreset(const char* name);
		virtual sReverbParameters getReverbEffectPreset(const char* name);
		virtual sChorusParameters getChorusEffectPreset(const char* name);
		virtual sDistortionParameters getDistortionEffectPreset(const char* name);
		virtual sEchoParameters getEchoEffectPreset(const char* name);
		virtual sFlangerParameters getFlangerEffectPreset(const char* name);
		virtual sFrequencyShiftParameters getFrequencyShiftEffectPreset(const char* name);
		virtual sVocalMorpherParameters getVocalMorpherEffectPreset(const char* name);
		virtual sPitchShifterParameters getPitchShifterEffectPreset(const char* name);
		virtual sRingModulatorParameters getRingModulatorEffectPreset(const char* name);
		virtual sAutowahParameters getAutowahEffectPreset(const char* name);
		virtual sCompressorParameters getCompressorEffectPreset(const char* name);
		virtual sEqualizerParameters getEqualizerEffectPreset(const char* name);

		virtual void removeEffectPreset(const EffectTypes& type, const char* name);
		virtual bool isEffectPresetRegistered(const EffectTypes& type, const char* name);
		virtual void removeAllEffectPresets(const EffectTypes& type);

		cEFXFunctions* getEFXInterface();
		void checkEFXSupportDetails();

    private:
		cAudioMutex Mutex;
		cEFXFunctions EFXInterface;
		unsigned int MaxEffectsSupported;
		bool SupportedEffects[EET_COUNT];
		bool SupportedFilters[EFT_COUNT];

		cAudioMap<cAudioString, sEAXReverbParameters>::Type EAXReverbPresets;
		cAudioMap<cAudioString, sReverbParameters>::Type ReverbPresets;
		cAudioMap<cAudioString, sChorusParameters>::Type ChorusPresets;
		cAudioMap<cAudioString, sDistortionParameters>::Type DistortionPresets;
		cAudioMap<cAudioString, sEchoParameters>::Type EchoPresets;
		cAudioMap<cAudioString, sFlangerParameters>::Type FlangerPresets;
		cAudioMap<cAudioString, sFrequencyShiftParameters>::Type FrequencyShiftPresets;
		cAudioMap<cAudioString, sVocalMorpherParameters>::Type VocalMorpherPresets;
		cAudioMap<cAudioString, sPitchShifterParameters>::Type PitchShifterPresets;
		cAudioMap<cAudioString, sRingModulatorParameters>::Type RingModulatorPresets;
		cAudioMap<cAudioString, sAutowahParameters>::Type AutowahPresets;
		cAudioMap<cAudioString, sCompressorParameters>::Type CompressorPresets;
		cAudioMap<cAudioString, sEqualizerParameters>::Type EqualizerPresets;
    };
};

#endif