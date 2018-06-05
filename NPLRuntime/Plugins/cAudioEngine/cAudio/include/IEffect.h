// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IRefCounted.h"
#include "IFilter.h"
#include "IEffectParameters.h"
#include "cAudioDefines.h"

#if CAUDIO_EFX_ENABLED == 1

namespace cAudio
{
	//! Enum of all available effect types in cAudio.
	enum EffectTypes
	{
		EET_NULL,
		EET_EAX_REVERB,
		EET_REVERB,
		EET_CHORUS,
		EET_DISTORTION,
		EET_ECHO,
		EET_FLANGER,
		EET_FREQUENCY_SHIFTER,
		EET_VOCAL_MORPHER,
		EET_PITCH_SHIFTER,
		EET_RING_MODULATOR,
		EET_AUTOWAH,
		EET_COMPRESSOR,
		EET_EQUALIZER,
		EET_COUNT
	};

	//! Interface for a single effect in cAudio.
	class IEffect : public IRefCounted
	{
	public:
		IEffect() {  }
		virtual ~IEffect() {  }

		//! Returns the current type this effect object is set to.
		virtual const EffectTypes& getType() const = 0;

		//! Sets the type of this effect object.
		/** 
		\param type: Type of effect to switch to. */
		virtual void setType(const EffectTypes& type) = 0;

		//! Returns the current parameters for the EAX Reverb Effect.
		virtual const sEAXReverbParameters& getEAXReverbParameters() const = 0;

		//! Sets the parameters for the EAX Reverb Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setEAXReverbParameters(const sEAXReverbParameters& param) = 0;

		//! Returns the current parameters for the Reverb Effect.
		virtual const sReverbParameters& getReverbParameters() const = 0;

		//! Sets the parameters for the Reverb Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setReverbParameters(const sReverbParameters& param) = 0;

		//! Returns the current parameters for the Chorus Effect.
		virtual const sChorusParameters& getChorusParameters() const = 0;

		//! Sets the parameters for the Chorus Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setChorusParameters(const sChorusParameters& param) = 0;

		//! Returns the current parameters for the Distortion Effect.
		virtual const sDistortionParameters& getDistortionParameters() const = 0;

		//! Sets the parameters for the Distortion Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setDistortionParameters(const sDistortionParameters& param) = 0;

		//! Returns the current parameters for the Echo Effect.
		virtual const sEchoParameters& getEchoParameters() const = 0;

		//! Sets the parameters for the Echo Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setEchoParameters(const sEchoParameters& param) = 0;

		//! Returns the current parameters for the Flanger Effect.
		virtual const sFlangerParameters& getFlangerParameters() const = 0;

		//! Sets the parameters for the Flanger Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setFlangerParameters(const sFlangerParameters& param) = 0;

		//! Returns the current parameters for the Frequency Shift Effect.
		virtual const sFrequencyShiftParameters& getFrequencyShiftParameters() const = 0;

		//! Sets the parameters for the Frequency Shift Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setFrequencyShiftParameters(const sFrequencyShiftParameters& param) = 0;

		//! Returns the current parameters for the Vocal Morpher Effect.
		virtual const sVocalMorpherParameters& getVocalMorpherParameters() const = 0;

		//! Sets the parameters for the Vocal Morpher Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setVocalMorpherParameters(const sVocalMorpherParameters& param) = 0;

		//! Returns the current parameters for the Pitch Shifter Effect.
		virtual const sPitchShifterParameters& getPitchShifterParameters() const = 0;

		//! Sets the parameters for the Pitch Shifter Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setPitchShifterParameters(const sPitchShifterParameters& param) = 0;

		//! Returns the current parameters for the Ring Modulator Effect.
		virtual const sRingModulatorParameters& getRingModulatorParameters() const = 0;

		//! Sets the parameters for the Ring Modulator Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setRingModulatorParameters(const sRingModulatorParameters& param) = 0;

		//! Returns the current parameters for the Autowah Effect.
		virtual const sAutowahParameters& getAutowahParameters() const = 0;

		//! Sets the parameters for the Autowah Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setAutowahParameters(const sAutowahParameters& param) = 0;

		//! Returns the current parameters for the Compressor Effect.
		virtual const sCompressorParameters& getCompressorParameters() const = 0;

		//! Sets the parameters for the Compressor Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setCompressorParameters(const sCompressorParameters& param) = 0;

		//! Returns the current parameters for the Equalizer Effect.
		virtual const sEqualizerParameters& getEqualizerParameters() const = 0;

		//! Sets the parameters for the Equalizer Effect.
		/**
		\param param: Parameter struct to set this effect to.*/
		virtual void setEqualizerParameters(const sEqualizerParameters& param) = 0;

		//! Returns the master volume for this effect.
		/** This volume scales the amount of effect audible from all attached sources.
		\return Currently set volume. */
		virtual float getMasterVolume() const = 0;

		//! Sets the master volume for this effect.
		/** This volume scales the amount of effect audible from all attached sources.
		\param volume: Volume to set the master volume to.  1.0f equal no volume change.  Range: 0.0f to 1.0.*/
		virtual void setMasterVolume(const float& volume) = 0;

		//! Returns if the effect for each attached source is attenuated by distance.
		virtual bool isIgnoringAttenuation() const = 0;

		//! Sets whether the effect for each attached source is attenuated by distance.
		/** If set to true, can cause some interesting and non-realistic effects, so be careful with it.
		\param ignore: Whether to ignore attenuation. */
		virtual void ignoreAttenuation(const bool& ignore) = 0;

		//! Returns the attached filter to this audio effect.
		virtual IFilter* getFilter() const = 0;

		//! Attaches a filter to this effect.
		/**
		\param filter: A Pointer to the filter to attach. */
		virtual void attachFilter(IFilter* filter) = 0;

		//! Removes the currently attached filter.
		virtual void removeFilter() = 0;

		//! Returns a timestamp indicating the last time settings on this effect were changed.  Used internally by the engine to update attached sources.
		virtual unsigned int getLastUpdated() const = 0;
		
		//! Returns if this effect is ready to be used or if it has encountered a fatal error.
		virtual bool isValid() const = 0;
	};
};

#endif
