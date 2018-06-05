// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IEffect.h"
#include "IFilter.h"
#include "cAudioDefines.h"

#if CAUDIO_EFX_ENABLED == 1

namespace cAudio
{
	//! Interface for Audio Effects in cAudio.
    class IAudioEffects
    {
    public:
		IAudioEffects() {}
        virtual ~IAudioEffects() {}
		
		//! Creates an Audio Effect for use with audio sources.
		/** \return Pointer to the created Effect instance. */
		virtual IEffect* createEffect() = 0;
		
		//! Creates an Audio Filter for use with audio sources.
		/** \return Pointer to the created Filter instance. */
		virtual IFilter* createFilter() = 0;

		//! Returns the max number of effects supported.
		virtual unsigned int getMaxEffectsSupported() const = 0;

		//! Checks to see if the given effect type is supported.
		/**
		\param type: the effect type to be checked.
		\return True if the effect is supported, False if the effect isn't supported. */
		virtual bool isEffectSupported(const EffectTypes& type) const = 0;
		
		//! Checks to see if the given filter type is supported.
		/**
		\param type: the filter type to be checked.
		\return True if the filter is supported, False if the filter isn't supported. */
		virtual bool isFilterSupported(const FilterTypes& type) const = 0;

		//! Adds a preset for the EAX Reverb Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addEAXReverbEffectPreset(const char* name, const sEAXReverbParameters& setting) = 0;
		
		//! Adds a preset for the Reverb Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addReverbEffectPreset(const char* name, const sReverbParameters& setting) = 0;
		
		//! Adds a preset for the Chorus Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addChorusEffectPreset(const char* name, const sChorusParameters& setting) = 0;
		
		//! Adds a preset for the Distortion Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addDistortionEffectPreset(const char* name, const sDistortionParameters& setting) = 0;
		
		//! Adds a preset for the Echo Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addEchoEffectPreset(const char* name, const sEchoParameters& setting) = 0;
		
		//! Adds a preset for the Flanger Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addFlangerEffectPreset(const char* name, const sFlangerParameters& setting) = 0;
		
		//! Adds a preset for the Frequency Shift Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addFrequencyShiftEffectPreset(const char* name, const sFrequencyShiftParameters& setting) = 0;
		
		//! Adds a preset for the Vocal Morpher Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addVocalMorpherEffectPreset(const char* name, const sVocalMorpherParameters& setting) = 0;
		
		//! Adds a preset for the Pitch Shifter Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addPitchShifterEffectPreset(const char* name, const sPitchShifterParameters& setting) = 0;
		
		//! Adds a preset for the Ring Modulator Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addRingModulatorEffectPreset(const char* name, const sRingModulatorParameters& setting) = 0;
		
		//! Adds a preset for the Autowah Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addAutowahEffectPreset(const char* name, const sAutowahParameters& setting) = 0;
		
		//! Adds a preset for the Compressor Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addCompressorEffectPreset(const char* name, const sCompressorParameters& setting) = 0;
		
		//! Adds a preset for the Equalizer Audio Effect type.
		/**
		\param name: Name of the preset.
		\param setting: Instance of the Effect parameter struct for this type of effect.
		\return True on success, False on failure. */
		virtual bool addEqualizerEffectPreset(const char* name, const sEqualizerParameters& setting) = 0;

		//! Returns a previously registered preset for the EAX Reverb Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sEAXReverbParameters getEAXReverbEffectPreset(const char* name) = 0;
		
		//! Returns a previously registered preset for the Reverb Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sReverbParameters getReverbEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Chorus Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sChorusParameters getChorusEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Distortion Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sDistortionParameters getDistortionEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Echo Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sEchoParameters getEchoEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Flanger Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sFlangerParameters getFlangerEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Frequency Shift Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sFrequencyShiftParameters getFrequencyShiftEffectPreset(const char* name) = 0;
		
		//! Returns a previously registered preset for the Vocal Morpher Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sVocalMorpherParameters getVocalMorpherEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Pitch Shifter Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sPitchShifterParameters getPitchShifterEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Ring Modulator Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sRingModulatorParameters getRingModulatorEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Autowah Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sAutowahParameters getAutowahEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Compressor Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sCompressorParameters getCompressorEffectPreset(const char* name) = 0;

		//! Returns a previously registered preset for the Equalizer Effect.
		/**
		\param name: Name of the preset to retrieve.
		\return The preset or the default parameters if the preset could not be found. */
		virtual sEqualizerParameters getEqualizerEffectPreset(const char* name) = 0;

		//! Removes a previously registered effect preset.
		/**
		\param type: Type of effect to remove a preset for.
		\param name: Name of the preset to remove. */
		virtual void removeEffectPreset(const EffectTypes& type, const char* name) = 0;

		//! Returns if a effect preset of a certain name is registered.
		/**
		\param type: Type of the effect.
		\param name: Name of the preset.
		\return True if it exists, false if not. */
		virtual bool isEffectPresetRegistered(const EffectTypes& type, const char* name) = 0;

		//! Removes all effect presets for a specific effect type.
		/**
		\param type: Type of effect to remove presets for, or EET_NULL to remove all of them. */
		virtual void removeAllEffectPresets(const EffectTypes& type) = 0;

    protected:
    private:
    };
};

#endif