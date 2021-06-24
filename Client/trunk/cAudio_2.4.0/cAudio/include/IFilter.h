// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IRefCounted.h"
#include "cAudioDefines.h"

#if CAUDIO_EFX_ENABLED == 1

namespace cAudio
{
	//! Enum of all available filter types in cAudio.
	enum FilterTypes
	{
		EFT_NULL,
		EFT_LOWPASS,
		EFT_HIGHPASS,
		EFT_BANDPASS,
		EFT_COUNT
	};

	//! Interface for a filter in cAudio.
	class IFilter : public IRefCounted
	{
	public:
		IFilter() {  }
		virtual ~IFilter() {  }

		//! Returns the type of the filter.
		virtual const FilterTypes& getType() const = 0;

		//! Sets the type of this filter.
		/**
		\param type: Type of filter to set this filter up as. */
		virtual void setType(const FilterTypes& type) = 0;

		//! Returns the master volume of this filter.
		virtual float getVolume() const = 0;

		//! Sets the master volume of this filter.
		/** This volume scales all audio that goes through it by this amount.
		\param volume: New volume amount. Range: 0.0f to 1.0. */
		virtual void setVolume(const float& volume) = 0;

		//! Returns the volume for low frequencies.
		virtual float getLowFrequencyVolume() const = 0;

		//! Sets the volume for low frequencies.
		/** This volume scales lower frequency audio that goes through it by this amount.
		\param volume: New volume amount. Range: 0.0f to 1.0. */
		virtual void setLowFrequencyVolume(const float& volumeLF) = 0;

		//! Returns the volume for high frequencies.
		virtual float getHighFrequencyVolume() const = 0;

		//! Sets the volume for high frequencies.
		/** This volume scales higher frequency audio that goes through it by this amount.
		\param volume: New volume amount. Range: 0.0f to 1.0. */
		virtual void setHighFrequencyVolume(const float& volumeHF) = 0;

		//! Returns a timestamp indicating the last time settings on this filter were changed.  Used internally by the engine to update attached sources or effects.
		virtual unsigned int getLastUpdated() const = 0;
		
		//! Returns if this filter is ready to be used or if it has encountered a fatal error.
		virtual bool isValid() const = 0;
	};
};

#endif
